/********************************************************************************
*																				*
*					GSocket：完成端口通讯模块（IOCP Socket）					*
*								——GSocket										*
*																				*
*				Copyright © 2009  代码客（卢益贵）  版权所有					*
*						未经许可，不得用于任何项目开发							*
*  QQ:48092788  E-Mail:gcode@qq.com  源码博客：http://blog.csdn.net/guestcode	*
*																				*
*					GSN:34674B4D-1F63-11D3-B64C-11C04F79498E					*
********************************************************************************/

#pragma once

/*********************************************************************************
                   条件编译
*********************************************************************************/
#define _GSOCKET_FUNC_TCP_SERVER				1
#define _GSOCKET_FUNC_TCP_CLIENT				1
#define _GSOCKET_FUNC_UDP_SERVER				0
#define _GSOCKET_FUNC_UDP_CLIENT				0

#define _USE_INTERLOCKED_IN_LIST				0
#define _USE_SINGLY_LINKED_LIST_IN_IODATA_POOL	1

#define _REUSED_SOCKET							1
#define _USE_GPROTOCOL							0
#define _USE_PROCESS_THREAD						1
#define _RUN_INFO								1

/*********************************************************************************
                   GSocket定义
*********************************************************************************/
typedef BOOL(*PFN_ON_GSOCK_TRAVERSAL)(const DWORD dwParam, const DWORD dwIndex, const DWORD dwContext);

/*********************************************************************************
                  GThread
*********************************************************************************/
typedef enum _GTHREAD_TYPE
{
GTHREAD_TYPE_IOCP,
GTHREAD_TYPE_TCP_SERVER_SERVICE,
GTHREAD_TYPE_TCP_CLIENT_SERVICE,
GTHREAD_TYPE_UDP_SERVER_SERVICE,
GTHREAD_TYPE_UDP_CLIENT_SERVICE,
GTHREAD_TYPE_PROCESS
}GTHREAD_TYPE;

DWORD GThrd_GetThreadId(DWORD dwWorkerId);
DWORD GThrd_GetRunCount(DWORD dwWorkerId);
DWORD GThrd_GetState(DWORD dwThreadContext);
GTHREAD_TYPE GThrd_GetType(DWORD dwThreadContext);
char* GThrd_GetName(DWORD dwWorkerId);
void GThrd_TraversalThread(DWORD dwParam, PFN_ON_GSOCK_TRAVERSAL pfnOnProc);
DWORD GThrd_GetThreadCount(void);

void* GThrd_GetData(void);
void GThrd_SetData(DWORD dwWorkerId, void * pData);

typedef	void(*PFN_ON_GTHREAD_EVENT)(const DWORD dwThreadContext);
void GThrd_SetEventProc(PFN_ON_GTHREAD_EVENT pfnOnBeginProc, PFN_ON_GTHREAD_EVENT pfnOnEndProc);

/*********************************************************************************
                   GProcThrdessThread
*********************************************************************************/
#if(_USE_PROCESS_THREAD)
DWORD GProcThrd_GetThreadNumber(void);
void GProcThrd_SetThreadNumber(DWORD dwNumber);
#endif

/*********************************************************************************
                   GWkrThrd
*********************************************************************************/
DWORD GWkrThrd_GetNumberOfProcessors(void);
DWORD GWkrThrd_GetWorkerThreadNumberDef(void);
DWORD GWkrThrd_GetWorkerThreadNumber(void);
void GWkrThrd_SetWorkerThreadNumber(DWORD dwNumber);
DWORD GWkrThrd_GetConcurrentThreadNumber(void);
void GWkrThrd_SetConcurrentThreadNumber(DWORD dwNumber);

/*********************************************************************************
                   GHndData
*********************************************************************************/
typedef	void(*PFN_ON_GHND_EVENT)(const DWORD dwClientContext);
typedef	void(*PFN_ON_GHND_DATA_EVENT)(const DWORD dwClientContext, const char *pBuf, const DWORD Bytes);

typedef enum _GHANDLE_TYPE
{
GHND_TYPE_TCP_SVR_LISTEN,
GHND_TYPE_TCP_SVR_CLIENT,
GHND_TYPE_TCP_CLT_CLIENT,
GHND_TYPE_UDP_SVR_LISTEN,
GHND_TYPE_UDP_SVR_CLIENT,
GHND_TYPE_UDP_CLT_CLIENT,
}GHND_TYPE;


typedef enum _GHANDLE_STATE
{
GHND_STATE_DISCONNECT,
GHND_STATE_CONNECTED,
GHND_STATE_ACCEPTING,
GHND_STATE_CONNECTING,
GHND_STATE_LISTENING
}GHND_STATE;

DWORD GHndDat_GetMemBytes(void);
DWORD GHndDat_GetSize(void);
DWORD GHndDat_GetTotal(void);
void GHndDat_SetTotal(DWORD dwTotal);
DWORD GHndDat_GetUsed(void);
GHND_TYPE GHndDat_GetType(DWORD dwClientContext);
GHND_STATE GHndDat_GetState(DWORD dwClientContext);
DWORD GHndDat_GetAddr(DWORD dwClientContext);
DWORD GHndDat_GetPort(DWORD dwClientContext);
DWORD GHndDat_GetTickCountAcitve(DWORD dwClientContext);
DWORD GHndDat_GetOwner(DWORD dwClientContext);

void* GHndDat_GetData(DWORD dwClientContext);
void GHndDat_SetData(DWORD dwClientContext, void* pData);

void GHndDat_SetProcOnHndCreate(PFN_ON_GHND_EVENT pfnOnProc);
void GHndDat_SetProcOnHndDestroy(PFN_ON_GHND_EVENT pfnOnProc);

/*********************************************************************************
                   GIoData
*********************************************************************************/
typedef char* PGIO_BUF;

DWORD GIoDat_GetGBufSize(void);
void GIoDat_SetGBufSize(DWORD dwSize);
DWORD GIoDat_GetMemBytes(void);
DWORD GIoDat_GetTotal(void);
void GIoDat_SetTotal(DWORD dwTotal);
DWORD GIoDat_GetUsed(void);

PGIO_BUF GIoDat_AllocGBuf(void);
void GIoDat_FreeGBuf(PGIO_BUF pGIoBuf);

/*********************************************************************************
                   GTcpClient
*********************************************************************************/

DWORD GTcpClt_GetClientCount(void);
void GTcpClt_TraversalClient(DWORD dwParam, DWORD dwFromIndex, DWORD dwCount, PFN_ON_GSOCK_TRAVERSAL pfnOnProc);

void GTcpClt_PostBroadcastBuf(char* pBuf, DWORD dwBytes);
DWORD GTcpClt_CreateClient(char *pszRemoteIp, DWORD dwRemotePort, char *pszLocalIp, void* pOwner);
void GTcpClt_DestroyClient(DWORD dwClientContext);
void GTcpClt_DisconnectClient(DWORD dwClientContext);

/*********************************************************************************
                   GTcpServer
*********************************************************************************/
DWORD GTcpSvr_GetClientCount(void);
DWORD GTcpSvr_GetListenerCount(void);
DWORD GTcpSvr_GetPendingAcceptCount(void);

DWORD GTcpSvr_GetListenerConnectCount(DWORD dwListenerId);
void GTcpSvr_TraversalListener(DWORD dwParam, PFN_ON_GSOCK_TRAVERSAL pfnOnProc);
void GTcpSvr_TraversalClient(DWORD dwParam, DWORD dwFromIndex, DWORD dwCount, PFN_ON_GSOCK_TRAVERSAL pfnOnProc);

DWORD GTcpSvr_CreateListen(char *lpszLocalIp, DWORD dwLocalPort, void* pOwner);
void GTcpSvr_CloseClient(DWORD dwClientContext);

/*********************************************************************************
                   GCommProt
*********************************************************************************/
BOOL GCommProt_PostSendGBuf(DWORD dwClientContext, PGIO_BUF pGBuf, DWORD dwBytes);
BOOL GCommProt_PostSendBuf(DWORD dwClientContext, char* pBuf, DWORD dwBytes);

/*********************************************************************************
                   GSocket
*********************************************************************************/
DWORD GSock_GetTimeIdleOvertime(void);
void GSock_SetTimeIdleOvertime(DWORD dwTime);
DWORD GSock_GetTimeHeartbeat(void);
void GSock_SetTimeHeartbeat(DWORD dwTime);
#if(_GSOCKET_FUNC_TCP_SERVER)
DWORD GSock_GetTimeAcceptOvertime(void);
void GSock_SetTimeAcceptOvertime(DWORD dwTime);
#endif//#if(_GSOCKET_FUNC_TCP_SERVER)
#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
DWORD GSock_GetMaxNumberConnection(void);
void GSock_SetMaxNumberConnection(DWORD dwNumber);
#endif//#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
#if(_GSOCKET_FUNC_TCP_SERVER)
DWORD GSock_GetNumberPostAccept(void);
void GSock_SetNumberPostAccept(DWORD dwNumber);
#endif//#if(_GSOCKET_FUNC_TCP_SERVER)
DWORD GSock_GetNumberPostRecv(void);
void GSock_SetNumberPostRecv(DWORD dwNumber);
#if(_GSOCKET_FUNC_TCP_SERVER)
BOOL GSock_IsZeroByteAccept(void);
void GSock_SetZeroByteAccept(BOOL bIsZeroBuffer);
#endif//#if(_GSOCKET_FUNC_TCP_SERVER
BOOL GSock_IsZeroByteRecvr(void);
void GSock_SetZeroByteRecv(BOOL bIsZeroBuffer);

#if(_GSOCKET_FUNC_TCP_SERVER)
void GSock_SetOnConnectProcTcpSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc);
#endif

#if(_GSOCKET_FUNC_UDP_SERVER)
void GSock_SetOnConnectProcUdpSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc);
#endif

#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
void GSock_SetOnDisconnectProcSvr(PFN_ON_GHND_EVENT pfnOnProc);
void GSock_SetOnReceiveProcSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc);
void GSock_SetOnSendedProcSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc);
void GSock_SetOnSendErrorProcSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc);
void GSock_SetOnConnectionOverflowProc(PFN_ON_GHND_EVENT pfnOnProc);
#endif//#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)

#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)
void GSock_SetOnConnectProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc);
void GSock_SetOnDisconnectProcClt(PFN_ON_GHND_EVENT pfnOnProc);
void GSock_SetOnReceiveProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc);
void GSock_SetOnSendedProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc);
void GSock_SetOnSendErrorProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc);
#endif//#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)

#if(_GSOCKET_FUNC_TCP_CLIENT)
void GSock_SetOnConnectErrorProc(PFN_ON_GHND_EVENT pfnOnProc);
void GSock_SetOnCreateClientProc(PFN_ON_GHND_EVENT pfnOnProc);
void GSock_SetOnDestroyClientProc(PFN_ON_GHND_EVENT pfnOnProc);
#endif//#if(_GSOCKET_FUNC_TCP_CLIENT)

void GSock_SetOnIdleOvertimeProc(PFN_ON_GHND_EVENT pfnOnProc);
void GSock_SetOnHeartbeatProc(PFN_ON_GHND_EVENT pfnOnProc);

void GSock_CloseClient(DWORD dwClientContext);

void* GSock_WSAGetExtensionFunctionPointer(SOCKET sSocket, GUID ExFuncGuid);
void GSock_GetLocalNetIp(char *szIp, int nSize, BOOL bIsIntnetIp);
void GSock_AddrToIp(char *szIp, int nSize, DWORD dwAddr);

BOOL GSock_IsActive(void);
void GSock_SetSocketSendBufSize(int nSize);
void GSock_SetSocketRecvBufSize(int nSize);
void GSock_SetMtuNodelay(BOOL bNodelay);
void GSock_StartService(void);
void GSock_StopService(void);

char* GSock_GetVersion(void);
DWORD GSock_GetClientCount(void);

