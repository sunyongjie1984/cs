/********************************************************************************
*																				*
*					GSocket：完成端口通讯模块（IOCP Socket）					*
*								——GTcpServer									*
*																				*
*				Copyright © 2009  代码客（卢益贵）  版权所有					*
*						未经许可，不得用于任何项目开发							*
*  QQ:48092788  E-Mail:gcode@qq.com  源码博客：http://blog.csdn.net/guestcode	*
*																				*
*					GSN:34674B4D-1F63-11D3-B64C-11C04F79498E					*
********************************************************************************/

#include "stdafx.h"
#include <winsock2.h>
#include <mswsock.h>

#include "GLog.h"
#include "GMemory.h"
#include "GWorkerThread.h"
#include "GPerIoData.h"
#include "GPerHandleData.h"
#include "GSocketInside.h"
#include "GThread.h"
#include "GSocket.h"

#if(_GSOCKET_FUNC_TCP_SERVER)

#define GTcpSvr_AcceptEx(pListener, pClient, pIoData) (pfnGSockAcceptEx(pListener->Socket, pClient->Socket, pIoData->cData, dwGSockAcceptBytes, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, LPWSAOVERLAPPED(pIoData)))
#define GTcpSvr_GetAcceptExSockAddrs(pListener, Buf, Bytes, Addr, Len) (pfnGSockGetAcceptExSockAddrs(Buf, Bytes, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (PSOCKADDR *)&Addr, (int*)&Len, (PSOCKADDR *)&Addr, (int*)&Len))
#define GTcpSvr_DisconnectEx(pClient) (pfnGSockDisconnectEx(pClient->Socket, NULL, TF_REUSE_SOCKET, 0))

/*********************************************************************************
                   变量定义和初始化
*********************************************************************************/
#if(_USE_INTERLOCKED_IN_LIST)
BOOL						bGTcpSvrPendingAcceptLock = FALSE;
#else
CRITICAL_SECTION			GTcpSvrPendingAcceptCS;
#endif
PGHND_DATA					pGTcpSvrPendingAcceptHead = NULL;
DWORD						dwGTcpSvrPendingAcceptCount = 0;

#if(_USE_INTERLOCKED_IN_LIST)
BOOL						bGTcpSvrClientLock = FALSE;
#else
CRITICAL_SECTION			GTcpSvrClientCS;
#endif
PGHND_DATA					pGTcpSvrClientHead = NULL;
DWORD						dwGTcpSvrClientCount = NULL;

CRITICAL_SECTION			GTcpSvrListenerCS;
DWORD						dwGTcpSvrListenerCount = 0;
PGHND_DATA					pGTcpSvrListeners[WSA_MAXIMUM_WAIT_EVENTS];
HANDLE						hGTcpSvrListenerEvents[WSA_MAXIMUM_WAIT_EVENTS];
PGHND_DATA*					pGTcpSvrOvertimeClient = NULL;
DWORD						pGTcpSvrOvertimeCount = 0;
GTHREAD						GTcpSvrServiceThreadData;

BOOL						bGTcpSvrIsActive = FALSE;

/*********************************************************************************
                内核锁
*********************************************************************************/
#if(_USE_INTERLOCKED_IN_LIST)
void GTcpSvr_LockClientList(void)
{
	for(;;)
	{
		if(FALSE == GSock_InterlockedSet(bGTcpSvrClientLock, TRUE, FALSE))
			return;
	}
}

void GTcpSvr_UnlockClientList(void)
{
	GSock_InterlockedSet(bGTcpSvrClientLock, FALSE, TRUE);
}

void GTcpSvr_LockPendingAcceptList(void)
{
	for(;;)
	{
		if(FALSE == GSock_InterlockedSet(bGTcpSvrPendingAcceptLock, TRUE, FALSE))
			return;
	}
}

void GTcpSvr_UnlockPendingAcceptList(void)
{
	GSock_InterlockedSet(bGTcpSvrPendingAcceptLock, FALSE, TRUE);
}
#endif

/*********************************************************************************
                变量设置
*********************************************************************************/
DWORD GTcpSvr_GetClientCount(void)
{
	return(dwGTcpSvrClientCount);
}

DWORD GTcpSvr_GetListenerCount(void)
{
	return(dwGTcpSvrListenerCount);
}

DWORD GTcpSvr_GetPendingAcceptCount(void)
{
	return(dwGTcpSvrPendingAcceptCount);
}

/*********************************************************************************
                Listener参数获取
*********************************************************************************/
DWORD GTcpSvr_GetListenerConnectCount(DWORD dwListenerId)
{
	return((DWORD)PGHND_DATA(dwListenerId)->pData);
}

void GTcpSvr_TraversalListener(DWORD dwParam, PFN_ON_GSOCK_TRAVERSAL pfnOnProc)
{
	if(!bGTcpSvrIsActive)
		return;

	EnterCriticalSection(&GTcpSvrListenerCS);

	DWORD i;

	for(i = 0; i < dwGTcpSvrListenerCount; i++)
	{
		if(!pfnOnProc(dwParam, i, DWORD(pGTcpSvrListeners[i])))
			break;
	}

	LeaveCriticalSection(&GTcpSvrListenerCS);
}

/*********************************************************************************
                服务操作
*********************************************************************************/
void GTcpSvr_InsertClientList(PGHND_DATA pClient)
{
	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_LockClientList();
	#else
	EnterCriticalSection(&GTcpSvrClientCS);
	#endif

	pClient->pPrior = NULL;
	pClient->pNext = pGTcpSvrClientHead;
	if(pGTcpSvrClientHead)
		pGTcpSvrClientHead->pPrior = pClient;
	pGTcpSvrClientHead = pClient;
	dwGTcpSvrClientCount++;
	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_UnlockClientList();
	#else
	LeaveCriticalSection(&GTcpSvrClientCS);
	#endif
}

void GTcpSvr_DeleteClientList(PGHND_DATA pClient)
{
	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_LockClientList();
	#else
	EnterCriticalSection(&GTcpSvrClientCS);
	#endif

	if(pClient == pGTcpSvrClientHead)
	{
		pGTcpSvrClientHead = pGTcpSvrClientHead->pNext;
		if(pGTcpSvrClientHead)
			pGTcpSvrClientHead->pPrior = NULL;
	}else
	{
		pClient->pPrior->pNext = pClient->pNext;
		if(pClient->pNext)
			pClient->pNext->pPrior = pClient->pPrior;
	}
	dwGTcpSvrClientCount--;

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_UnlockClientList();
	#else
	LeaveCriticalSection(&GTcpSvrClientCS);
	#endif
}

void GTcpSvr_InsertPendingAcceptList(PGHND_DATA pClient)
{
	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_LockPendingAcceptList();
	#else
	EnterCriticalSection(&GTcpSvrPendingAcceptCS);
	#endif

	pClient->pNext = pGTcpSvrPendingAcceptHead;
	if(pGTcpSvrPendingAcceptHead)
		pGTcpSvrPendingAcceptHead->pPrior = pClient;
	pClient->pPrior = NULL;
	pGTcpSvrPendingAcceptHead = pClient;
	dwGTcpSvrPendingAcceptCount++;

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_UnlockPendingAcceptList();
	#else
	LeaveCriticalSection(&GTcpSvrPendingAcceptCS);
	#endif
}
void GTcpSvr_DeletePendingAcceptList(PGHND_DATA pClient)
{
	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_LockPendingAcceptList();
	#else
	EnterCriticalSection(&GTcpSvrPendingAcceptCS);
	#endif

	if(pClient == pGTcpSvrPendingAcceptHead)
	{
		pGTcpSvrPendingAcceptHead = pGTcpSvrPendingAcceptHead->pNext;
		if(pGTcpSvrPendingAcceptHead)
			pGTcpSvrPendingAcceptHead->pPrior = NULL;
	}else
	{
		pClient->pPrior->pNext = pClient->pNext;
		if(pClient->pNext)
			pClient->pNext->pPrior = pClient->pPrior;
	}
	dwGTcpSvrPendingAcceptCount--;

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_UnlockPendingAcceptList();
	#else
	LeaveCriticalSection(&GTcpSvrPendingAcceptCS);
	#endif
}

void GTcpSvr_TraversalClient(DWORD dwParam, DWORD dwFromIndex, DWORD dwCount, PFN_ON_GSOCK_TRAVERSAL pfnOnProc)
{
	if(!bGTcpSvrIsActive)
		return;

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_LockClientList();
	#else
	EnterCriticalSection(&GTcpSvrClientCS);
	#endif

	PGHND_DATA pClient;
	DWORD dwIndex;

	dwIndex = 0;
	dwCount = dwFromIndex + dwCount;
	pClient = pGTcpSvrClientHead;
	while(pClient)
	{		
		if(dwIndex >= dwFromIndex)
		{
			if((dwIndex >= dwCount) || (!pfnOnProc(dwParam, dwIndex, (DWORD)pClient)))
				break;
		}
		dwIndex++;
		pClient = pClient->pNext;
	}

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_UnlockClientList();
	#else
	LeaveCriticalSection(&GTcpSvrClientCS);
	#endif
}

void GTcpSvr_OnReadWriteError(PGHND_DATA pClient, PGIO_DATA pIoData)
{
	if(GIO_WRITE_COMPLETED == pIoData->OperType)
		pfnOnGSockSendErrorSvr((DWORD)pClient, pIoData->cData, pIoData->WSABuf.len);
	GIoDat_Free(pIoData);

	if(GHND_STATE_CONNECTED != GSock_InterlockedSet(pClient->hsState, GHND_STATE_DISCONNECT, GHND_STATE_CONNECTED))
		return;

	GTcpSvr_DeleteClientList(pClient);
	GSock_InterlockedDec(PGHND_DATA(pClient->pOwner)->pData);
	pfnOnGSockDisconnectSvr((DWORD)pClient);

	if(GIO_CONNECTION_OVERFLOW == pIoData->OperType)
	{
		pfnOnGSockConnectionOverflow((DWORD)pClient);
		GSock_UninitTcpHndData(pClient);
		#if(_REUSED_SOCKET)
		GSock_InitTcpHndData(pClient);
		#endif
		GHndDat_Free(pClient);
	}else
	if(GIO_IDLE_OVERTIME == pIoData->OperType)
	{
		pfnOnGSockIdleOvertime((DWORD)pClient);
		GSock_UninitTcpHndData(pClient);
		#if(_REUSED_SOCKET)
		GSock_InitTcpHndData(pClient);
		#endif
		GHndDat_Free(pClient);
	}else
	if(GIO_CLOSE == pIoData->OperType)
	{
		GSock_UninitTcpHndData(pClient);
		#if(_REUSED_SOCKET)
		GSock_InitTcpHndData(pClient);
		#endif
		GHndDat_Free(pClient);
	}else
	{
		#if(_REUSED_SOCKET)
		GTcpSvr_DisconnectEx(pClient);
		#else
		GSock_UninitTcpHndData(pClient);
		#endif
		GHndDat_Free(pClient);
	}
	
}

void GTcpSvr_OnReadWrite(DWORD dwBytes, PGHND_DATA pClient, PGIO_DATA pIoData)
{
	if(!dwBytes)
	{
		if(bGSockIsZeroByteRecv && (GHND_STATE_CONNECTED == pClient->hsState) && (GIO_READ_COMPLETED == pIoData->OperType))
		{			
			GIoDat_ResetIoDataOnRead(pIoData);
			pIoData->OperType = GIO_ZERO_READ_COMPLETED;
			pIoData->WSABuf.len = dwGBufSize;
			dwBytes = 0;
			DWORD dwFlag = 0;
			if((SOCKET_ERROR != WSARecv(pClient->Socket, &(pIoData->WSABuf), 1, &dwBytes, &dwFlag, LPWSAOVERLAPPED(pIoData), NULL)) ||
				(ERROR_IO_PENDING == WSAGetLastError()))
			{
				return;
			}
		}
		GTcpSvr_OnReadWriteError(pClient, pIoData);
		return;
	}
	
	if(GIO_WRITE_COMPLETED == pIoData->OperType)
	{
		pfnOnGSockSendedSvr((DWORD)pClient, pIoData->cData, dwBytes);
		GIoDat_Free(pIoData);
	}else
	{
		pClient->dwTickCountAcitve = GetTickCount();
		#if(_USE_GPROTOCOL)
		if(GCommProt_ProcessReceive(pClient, pIoData->cData, dwBytes, pfnOnGSockReceiveSvr))
		{
			pIoData = GIoDat_Alloc();
			if(!pIoData)
			{
				GLog_Write("GTcpSvr_OnReadWrite：IoData分配失败，无法再投递接收");
				return;
			}
		}
		#else
		pfnOnGSockReceiveSvr((DWORD)pClient, pIoData->cData, dwBytes);
		#endif

		GIoDat_ResetIoDataOnRead(pIoData);
		pIoData->OperType = GIO_READ_COMPLETED;
		pIoData->WSABuf.len = dwGSockRecvBytes;
		dwBytes = 0;
		DWORD dwFlag = 0;
		if((SOCKET_ERROR == WSARecv(pClient->Socket, &(pIoData->WSABuf), 1, &dwBytes, &dwFlag, LPWSAOVERLAPPED(pIoData), NULL)) &&	
			(ERROR_IO_PENDING != WSAGetLastError()))
		{
			GTcpSvr_OnReadWriteError(pClient, pIoData);
		}
	}
}

BOOL GTcpSvr_PostAccept(PGHND_DATA pListener, DWORD dwCount)
{
	int nCount;
	PGHND_DATA pClient;
	PGIO_DATA pIoData;

	nCount = 0;
	
	while(dwCount && (dwGTcpSvrClientCount + dwGTcpSvrPendingAcceptCount < dwGSockMaxNumberConnection))
	{
		pClient = GHndDat_Alloc();
		if(!pClient)
		{
			GLog_Write("GTcpSvr_PostAccept：分配HndData失败，无法投递接受");
			return(nCount);
		}
		#if(!_REUSED_SOCKET)
		GSock_InitTcpHndData(pClient);
		#endif
		pIoData = GIoDat_Alloc();
		if(!pIoData)
		{
			#if(!_REUSED_SOCKET)
			GSock_UninitTcpHndData(pClient);
			#endif
			GHndDat_Free(pClient);
			GLog_Write("GTcpSvr_PostAccept：分配IoData失败，无法投递接受");
			return(nCount);
		}

		pClient->pfnOnIocpOper = &GTcpSvr_OnReadWrite;
		pClient->pfnOnIocpError = &GTcpSvr_OnReadWriteError;
		pClient->htType = GHND_TYPE_TCP_SVR_CLIENT;
		pClient->hsState = GHND_STATE_ACCEPTING;
		pClient->pOwner = pListener;
		pClient->pData = NULL;

		pIoData->OperType = GIO_CONNECTED;
		pIoData->pOwner = pClient;
		pIoData->WSABuf.len = dwGSockRecvBytes;

		GTcpSvr_InsertPendingAcceptList(pClient);
		if((!GTcpSvr_AcceptEx(pListener, pClient, pIoData)) && (ERROR_IO_PENDING != WSAGetLastError()))
		{
			GTcpSvr_DeletePendingAcceptList(pClient);
			GIoDat_Free(pIoData);
			#if(!_REUSED_SOCKET)
			GSock_UninitTcpHndData(pClient);
			#endif
			GHndDat_Free(pClient);
			GLog_Write("GTcpSvr_PostAccept：执行pfnGTcpSvrAcceptEx失败，无法投递接受");
			return(nCount);
		}
		dwCount--;
		nCount++;
	}//for(i = 0; i < dwCount; i++)

	return(TRUE);
}

void GTcpSvr_OnAcceptError(PGHND_DATA pListener, PGIO_DATA pIoData)
{
	PGHND_DATA pClient = PGHND_DATA(pIoData->pOwner);
	GIoDat_Free(pIoData);

	if(GHND_STATE_ACCEPTING != GSock_InterlockedSet(pClient->hsState, GHND_STATE_DISCONNECT, GHND_STATE_ACCEPTING))
		return;

	GTcpSvr_DeletePendingAcceptList(pClient);
	if(GIO_CLOSE == pIoData->OperType)
	{
		GSock_UninitTcpHndData(pClient);
		GHndDat_Free(pClient);
	}else
	{
		#if(_REUSED_SOCKET)
		GTcpSvr_DisconnectEx(pClient);
		#else
		GSock_UninitTcpHndData(pClient);
		#endif
		GHndDat_Free(pClient);
	}
}

void GTcpSvr_OnAccept(DWORD dwBytes, PGHND_DATA pListener, PGIO_DATA pIoData)
{
	if((!dwBytes) && (dwGSockAcceptBytes))
	{
		GTcpSvr_OnAcceptError(pListener, pIoData);
		return;
	}

	PGHND_DATA pClient;
	PSOCKADDR_IN pAddr;
	int nLen;

	pClient = PGHND_DATA(pIoData->pOwner);
	GTcpSvr_DeletePendingAcceptList(pClient);
	setsockopt(pClient->Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&(pListener->Socket), sizeof(pListener->Socket));
	BOOL bDontLinger = FALSE;
	setsockopt(pClient->Socket, SOL_SOCKET, SO_DONTLINGER, (const char *) &bDontLinger, sizeof(BOOL));

	GTcpSvr_GetAcceptExSockAddrs(pListener, pIoData->cData, dwGSockAcceptBytes, pAddr, nLen);
	pClient->dwAddr = pAddr->sin_addr.S_un.S_addr;
	pClient->dwPort = htons(pAddr->sin_port);
	pClient->hsState = GHND_STATE_CONNECTED;
	pClient->dwTickCountAcitve = GetTickCount();

	GTcpSvr_InsertClientList(pClient);
	GSock_InterlockedAdd(PGHND_DATA(pClient->pOwner)->pData);	
	#if(_USE_GPROTOCOL)
	if(GCommProt_ProcessReceive(pClient, pIoData->cData, dwBytes, pfnOnGSockConnectTcpSvr))
	{
		pIoData = GIoDat_Alloc();
		if(!pIoData)
		{
			GLog_Write("GTcpSvr_OnAccept：IoData分配失败，连接后无法再投递接收");
			return;
		}
	}
	#else
	pfnOnGSockConnectTcpSvr((DWORD)pClient, pIoData->cData, dwBytes);
	#endif
	
	ZeroMemory(pIoData, sizeof(WSAOVERLAPPED));
	DWORD dwCount = dwGSockNumberPostRecv;
	for(;;)
	{
		GIoDat_ResetIoDataOnRead(pIoData);
		pIoData->OperType = GIO_READ_COMPLETED;
		pIoData->WSABuf.len = dwGSockRecvBytes;		
		dwBytes = 0;
		DWORD dwFlag = 0;
		if((SOCKET_ERROR == WSARecv(pClient->Socket, &(pIoData->WSABuf), 1, &dwBytes, &dwFlag, LPWSAOVERLAPPED(pIoData), NULL)) &&
			(ERROR_IO_PENDING != WSAGetLastError()))
		{
			GTcpSvr_OnReadWriteError(pClient, pIoData);
			break;
		}
		dwCount--;
		if(!dwCount)
			break;
		pIoData = GIoDat_Alloc();
		if(!pIoData)
		{
			GLog_Write("GTcpSvr_OnAccept：申请IoData失败，连接后无法投递接收");
			break;
		}		
		pIoData->pOwner = pClient;
	}
	if(dwGTcpSvrClientCount >= dwGSockMaxNumberConnection)
	{
		pfnOnGSockConnectionOverflow((DWORD)pClient);
		void GTcpSvr_DoCloseClient(PGHND_DATA pClient, PGHND_DATA pIoDataOwner, GIO_OPER_TYPE OperType);
		GTcpSvr_DoCloseClient(pClient, pClient, GIO_CLOSE);
		GLog_Write("GTcpSvr_OnAccept：连接超过最大值");
	}
}

/*********************************************************************************
                   监听者
*********************************************************************************/
DWORD GTcpSvr_CreateListen(char *lpszLocalIp, DWORD dwLocalPort, void* pOwner)
{
	if((!bGTcpSvrIsActive) || (WSA_MAXIMUM_WAIT_EVENTS <= dwGTcpSvrListenerCount))
		return(0);

	SOCKADDR_IN Addr;
	PGHND_DATA pListener;

	pListener = GHndDat_Alloc();
	if(!pListener)
	{
		GLog_Write("GTcpSvr_CreateListen：分配HndData失败，无法创建一个监听者");
		return(0);
	}
	#if(!_REUSED_SOCKET)
	GSock_InitTcpHndData(pListener);
	#endif

	Addr.sin_family = AF_INET;
	Addr.sin_addr.S_un.S_addr = inet_addr(lpszLocalIp);
	Addr.sin_port = htons((u_short)dwLocalPort);	
	if((SOCKET_ERROR == bind(pListener->Socket, (PSOCKADDR)&Addr, sizeof(SOCKADDR_IN))) ||
		(SOCKET_ERROR == listen(pListener->Socket, SOMAXCONN)))
	{
		#if(!_REUSED_SOCKET)
		GSock_UninitTcpHndData(pListener);
		#endif
		GHndDat_Free(pListener);
		GLog_Write("GTcpSvr_CreateListen：绑定或监听操作失败，无法创建一个监听者");
		return(0);
	}
	pListener->pfnOnIocpOper = &GTcpSvr_OnAccept;
	pListener->pfnOnIocpError = &GTcpSvr_OnAcceptError;
	pListener->dwAddr = Addr.sin_addr.S_un.S_addr;
	pListener->dwPort = dwLocalPort;
	pListener->htType = GHND_TYPE_TCP_SVR_LISTEN;
	pListener->hsState = GHND_STATE_LISTENING;
	pListener->pOwner = pOwner;
	pListener->pData = NULL;

	if(!GTcpSvr_PostAccept(pListener, dwGSockNumberPostAccept))
	{
		#if(!_REUSED_SOCKET)
		GSock_UninitTcpHndData(pListener);
		#endif
		GHndDat_Free(pListener);
		GLog_Write("GTcpSvr_CreateListen：投递接受失败“GTcpSvr_PostAccept”，无法创建一个监听者");
		return(0);
	}

	EnterCriticalSection(&GTcpSvrListenerCS);
	pGTcpSvrListeners[dwGTcpSvrListenerCount] = pListener;
	hGTcpSvrListenerEvents[dwGTcpSvrListenerCount] = CreateEvent(NULL, FALSE, FALSE, NULL);
	WSAEventSelect(pListener->Socket, hGTcpSvrListenerEvents[dwGTcpSvrListenerCount], FD_ACCEPT);
	dwGTcpSvrListenerCount++;
	LeaveCriticalSection(&GTcpSvrListenerCS);

	return(DWORD(pListener));
}

void GTcpSvr_CloseListeners(void)
{
	DWORD i;

	GLog_Write("GTcpSvr_CloseListeners：正在关闭监听者");
	for(i = 0; i < dwGTcpSvrListenerCount; i++)
		closesocket(pGTcpSvrListeners[i]->Socket);
	GLog_Write("GTcpSvr_CloseListeners：成功关闭监听者");
}

void GTcpSvr_CloseListenerEvents(void)
{
	DWORD i;
	for(i = 0; i < dwGTcpSvrListenerCount; i++)
		CloseHandle(hGTcpSvrListenerEvents[i]);
}

void GTcpSvr_FreeListener(void)
{
	DWORD i;

	for(i = 0; i < dwGTcpSvrListenerCount; i++)
		GHndDat_Free(pGTcpSvrListeners[i]);
}

/*********************************************************************************
                   服务管理
*********************************************************************************/
void GTcpSvr_DoCloseClient(PGHND_DATA pClient, PGHND_DATA pIoDataOwner, GIO_OPER_TYPE OperType)
{
	PGIO_DATA pIoData = GIoDat_Alloc();
	if(pIoData)
	{
		pIoData->OperType = OperType;
		pIoData->WSABuf.len = 0;
		pIoData->pOwner = pIoDataOwner;
		PostQueuedCompletionStatus(hGWkrThrdCompletionPort, 0, (DWORD)pClient, (LPOVERLAPPED)pIoData);
	}else
		GLog_Write("GTcpSvr_DoCloseClient：申请IoData失败，无法关闭连接");
}

void GTcpSvr_CloseClient(DWORD dwClientContext)
{
	GTcpSvr_DoCloseClient((PGHND_DATA)dwClientContext, (PGHND_DATA)dwClientContext, GIO_CLOSE);
}

void GTcpSvrServiceThread(PGTHREAD pThread)
{
	DWORD dwResult;
	PGHND_DATA pClient;
	INT nResult;
	INT nLong;
	INT nLongBytes = sizeof(INT);
	DWORD dwTickCount;

	GLog_Write("GTcpSvrServiceThread：伺服线程开始");
	for(;;)
	{
		#if(_RUN_INFO)
		pThread->dwState = GTHREAD_STATE_SLEEP;
		#endif

		if(!dwGTcpSvrListenerCount)
		{
			Sleep(1000);
			dwResult = WSA_WAIT_TIMEOUT;
		}else
			dwResult = WSAWaitForMultipleEvents(dwGTcpSvrListenerCount, hGTcpSvrListenerEvents, FALSE, 1000, FALSE);
		if(pThread->bIsShutdown)
			break;

		#if(_RUN_INFO)
		pThread->dwState = GTHREAD_STATE_WORKING1;
		pThread->dwRunCount++;
		#endif

		if(WSA_WAIT_TIMEOUT != dwResult)
		{	
			dwResult -= WSA_WAIT_EVENT_0;
			WSAResetEvent(hGTcpSvrListenerEvents[dwResult]);
			if((dwGTcpSvrClientCount + dwGTcpSvrPendingAcceptCount >= dwGSockMaxNumberConnection) || (!GTcpSvr_PostAccept(pGTcpSvrListeners[dwResult], dwGSockNumberPostAccept)))
			{
				SOCKADDR_IN Addr;
				int nLen;
				nLen = sizeof(SOCKADDR_IN);
				closesocket(WSAAccept(pGTcpSvrListeners[dwResult]->Socket, (SOCKADDR*)&Addr, &nLen, NULL, 0));
				if(dwGTcpSvrClientCount >= dwGSockMaxNumberConnection)
					GLog_Write("GTcpSvrServiceThread：超出连接限制");
				else
					GLog_Write("GTcpSvrServiceThread：投递接受“GTcpSvr_PostAccept”失败");
			}
			continue;
		}

		if(dwGSockAcceptBytes)
		{
			#if(_USE_INTERLOCKED_IN_LIST)
			GTcpSvr_LockPendingAcceptList();
			#else
			EnterCriticalSection(&GTcpSvrPendingAcceptCS);
			#endif

			#if(_RUN_INFO)
			pThread->dwState = GTHREAD_STATE_WORKING2;
			#endif

			pClient = pGTcpSvrPendingAcceptHead;
			pGTcpSvrOvertimeCount = 0;
			while(pClient)
			{
				nResult = getsockopt(pClient->Socket, SOL_SOCKET, SO_CONNECT_TIME, (char *)&nLong, (PINT)&nLongBytes);
				if((SOCKET_ERROR != nResult) && (0xFFFFFFFF != nLong) && (dwGSockTimeAcceptOvertime < (DWORD)nLong))
				{
					pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount] = pClient;
					pGTcpSvrOvertimeCount++;
				}
				pClient = pClient->pNext;
			}

			#if(_USE_INTERLOCKED_IN_LIST)
			GTcpSvr_UnlockPendingAcceptList();
			#else
			LeaveCriticalSection(&GTcpSvrPendingAcceptCS);
			#endif

			#if(_RUN_INFO)
			pThread->dwState = GTHREAD_STATE_WORKING3;
			#endif

			while(pGTcpSvrOvertimeCount)
			{
				pGTcpSvrOvertimeCount--;
				pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount]->dwTickCountAcitve = dwTickCount;
				GTcpSvr_DoCloseClient(PGHND_DATA(pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount]->pOwner), pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount], GIO_CLOSE);
			}
		}//if(dwGSockAcceptBytes)

		if(dwGSockTimeIdleOvertime)
		{
			#if(_USE_INTERLOCKED_IN_LIST)
			GTcpSvr_LockClientList();
			#else
			EnterCriticalSection(&GTcpSvrClientCS);
			#endif

			#if(_RUN_INFO)
			pThread->dwState = GTHREAD_STATE_WORKING4;
			#endif

			pClient = pGTcpSvrClientHead;
			pGTcpSvrOvertimeCount = 0;
			dwTickCount = GetTickCount();
			while(pClient)
			{//QueryPerformanceCounter
				if(dwTickCount >= pClient->dwTickCountAcitve)
					nLong = dwTickCount - pClient->dwTickCountAcitve;
				else
					nLong = 0xFFFFFFFF - pClient->dwTickCountAcitve + dwTickCount;
				if((DWORD)nLong > dwGSockTimeIdleOvertime)
				{
					pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount] = pClient;
					pGTcpSvrOvertimeCount++;
				}
				pClient = pClient->pNext;
			}

			#if(_USE_INTERLOCKED_IN_LIST)
			GTcpSvr_UnlockClientList();
			#else
			LeaveCriticalSection(&GTcpSvrClientCS);
			#endif
			
			#if(_RUN_INFO)
			pThread->dwState = GTHREAD_STATE_WORKING5;
			#endif

			while(pGTcpSvrOvertimeCount)
			{
				pGTcpSvrOvertimeCount--;
				pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount]->dwTickCountAcitve = dwTickCount;
				GTcpSvr_DoCloseClient(pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount], pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount], GIO_IDLE_OVERTIME);
			}
		}//if(dwGSockTimeIdleOvertime)
	}//for(;;)
}

void GTcpSvr_Create(void)
{
	if(bGTcpSvrIsActive)
		return;

	pGTcpSvrOvertimeClient = (PGHND_DATA*)GMem_Alloc(dwGSockMaxNumberConnection * sizeof(SOCKET*));
	if(!pGTcpSvrOvertimeClient)
	{
		GLog_Write("GTcpSvr_Create：从GMem分配超时句柄数据失败");
		return;
	}	

	pGTcpSvrPendingAcceptHead = NULL;
	dwGTcpSvrPendingAcceptCount = 0;
	#if(!_USE_INTERLOCKED_IN_LIST)
	InitializeCriticalSection(&GTcpSvrPendingAcceptCS);
	#endif	

	pGTcpSvrClientHead = NULL;
	dwGTcpSvrClientCount = 0;
	#if(!_USE_INTERLOCKED_IN_LIST)
	InitializeCriticalSection(&GTcpSvrClientCS);
	#endif

	InitializeCriticalSection(&GTcpSvrListenerCS);
	dwGTcpSvrListenerCount = 0;
	pGTcpSvrOvertimeCount = 0;
	GThrd_CreateThread(&GTcpSvrServiceThreadData, GTHREAD_TYPE_TCP_SERVER_SERVICE, "TcpSvr伺服", &GTcpSvrServiceThread);
	if(!GTcpSvrServiceThreadData.dwThreadId)
	{
		#if(!_USE_INTERLOCKED_IN_LIST)
		DeleteCriticalSection(&GTcpSvrPendingAcceptCS);
		DeleteCriticalSection(&GTcpSvrClientCS);
		#endif
		DeleteCriticalSection(&GTcpSvrListenerCS);
		GLog_Write("GTcpSvr_Create：创建服务线程“GTcpSvrServiceThread”失败");
		return;
	}

	bGTcpSvrIsActive = TRUE;
}

void GTcpSvr_CloseClients(void)
{
	PGHND_DATA pHndData;

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_LockClientList();
	#else
	EnterCriticalSection(&GTcpSvrClientCS);
	#endif

	pGTcpSvrOvertimeCount = 0;
	pHndData = pGTcpSvrClientHead;
	while(pHndData)
	{
		pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount] = pHndData;
		pGTcpSvrOvertimeCount++;
		pHndData = pHndData->pNext;
	}

	#if(_USE_INTERLOCKED_IN_LIST)
	GTcpSvr_UnlockClientList();
	#else
	LeaveCriticalSection(&GTcpSvrClientCS);
	#endif

	GLog_Write("GTcpSvr_CloseClients：正在关闭客户连接");
	while(pGTcpSvrOvertimeCount)
	{
		pGTcpSvrOvertimeCount--;
		closesocket(pGTcpSvrOvertimeClient[pGTcpSvrOvertimeCount]->Socket);
	}
	GLog_Write("GTcpSvr_CloseClients：成功关闭客户连接");
}

void GTcpSvr_EndThread(void)
{
	if(!GTcpSvrServiceThreadData.bIsShutdown)
	{
		GTcpSvrServiceThreadData.bIsShutdown = TRUE;
		GLog_Write("GTcpSvr_EndThread：正在关闭服务线程");
		GThrd_DestroyThread(&GTcpSvrServiceThreadData);
		GLog_Write("GTcpSvr_EndThread：成功关闭服务线程");
	}
}

void GTcpSvr_Destroy(void)
{
	bGTcpSvrIsActive = FALSE;
	if(!pGTcpSvrOvertimeClient)
		return;

	GTcpSvr_FreeListener();

	pGTcpSvrPendingAcceptHead = NULL;
	dwGTcpSvrPendingAcceptCount = 0;
	#if(!_USE_INTERLOCKED_IN_LIST)
	DeleteCriticalSection(&GTcpSvrPendingAcceptCS);
	#endif

	pGTcpSvrClientHead = NULL;
	dwGTcpSvrClientCount = 0;
	#if(!_USE_INTERLOCKED_IN_LIST)
	DeleteCriticalSection(&GTcpSvrClientCS);
	#endif

	pGTcpSvrOvertimeCount = 0;
	GMem_Free(pGTcpSvrOvertimeClient);
	pGTcpSvrOvertimeClient = NULL;
	dwGTcpSvrListenerCount = 0;
	DeleteCriticalSection(&GTcpSvrListenerCS);
}

#endif//#if(_GSOCKET_FUNC_TCP_SERVER)