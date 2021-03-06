/********************************************************************************
*																				*
*					GSocket：完成端口通讯模块（IOCP Socket）					*
*								——GProcessThread								*
*																				*
*				Copyright © 2009  代码客（卢益贵）  版权所有					*
*						未经许可，不得用于任何项目开发							*
*  QQ:48092788  E-Mail:gcode@qq.com  源码博客：http://blog.csdn.net/guestcode	*
*																				*
*					GSN:34674B4D-1F63-11D3-B64C-11C04F79498E					*
********************************************************************************/

#include "stdafx.h"

#include "GLog.h"
#include "GPerIoData.h"
#include "GMemory.h"
#include "GThread.h"
#include "GWorkerThread.h"
#include "GSocketInside.h"
#include "GSocket.h"

#if(_USE_PROCESS_THREAD)
/*********************************************************************************
                   变量定义和初始化
*********************************************************************************/ 
HANDLE								hGProcThrdThreadCompletionPort = 0;
PGTHREAD							pGProcThrdThreadAddr = NULL;
DWORD								dwGProcThrdThreadNumber = 0;
BOOL								bGProcThrdThreadIsActive = FALSE;

/*********************************************************************************
                   处理线程
*********************************************************************************/

void GProcesserThreadProc(PGTHREAD pThread)
{
	DWORD dwBytes;
	PGIOCP_COMPLETION_KEY pCompletionKey;
	LPOVERLAPPED pOverlapped;

	for(;;)
	{
		#if(_RUN_INFO)
		pThread->dwState = GTHREAD_STATE_SLEEP;
		#endif

		GetQueuedCompletionStatus(hGProcThrdThreadCompletionPort, &dwBytes, (PULONG_PTR)&pCompletionKey, &pOverlapped, INFINITE);		

		if(pThread->bIsShutdown)
			break;

		#if(_RUN_INFO)
		pThread->dwState = GTHREAD_STATE_WORKING1;
		#endif

		if(!(dwBytes & 0x80000000))
		{
			#if(_RUN_INFO)
			pThread->dwState = GTHREAD_STATE_WORKING1;
			#endif
			pCompletionKey->pfnOnIocpOper(dwBytes & (~0x80000000), pCompletionKey, pOverlapped);
		}else
		{
			#if(_RUN_INFO)
			pThread->dwState = GTHREAD_STATE_WORKING2;
			#endif

			pCompletionKey->pfnOnIocpError(pCompletionKey, pOverlapped);
		}

		#if(_RUN_INFO)
		pThread->dwRunCount++;
		#endif
	}
}

/*********************************************************************************
                   处理线程功能函数
*********************************************************************************/
DWORD GProcThrd_GetThreadNumber(void)
{
	return(dwGProcThrdThreadNumber);
}

void GProcThrd_SetThreadNumber(DWORD dwNumber)
{
	if(bGProcThrdThreadIsActive)
		return;
	dwGProcThrdThreadNumber = dwNumber;
}

void GProcThrd_Create(void)
{
	if(bGProcThrdThreadIsActive)
		return;

	DWORD dwNP = GWkrThrd_GetNumberOfProcessors();
	
	if(!dwGProcThrdThreadNumber)
		dwGProcThrdThreadNumber = dwNP * 2 + 2;

	if(dwGProcThrdThreadNumber < dwNP)
		dwGProcThrdThreadNumber = dwNP;

	hGProcThrdThreadCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, dwGProcThrdThreadNumber);
	if(NULL == hGProcThrdThreadCompletionPort)
	{
		GLog_Write("GProcThrd_Create：完成端口句柄创建失败");
		return;
	}
	
	pGProcThrdThreadAddr = (PGTHREAD)GMem_Alloc(dwGProcThrdThreadNumber * sizeof(GTHREAD));
	if(!pGProcThrdThreadAddr)
	{
		dwGProcThrdThreadNumber = 0;
		CloseHandle(hGProcThrdThreadCompletionPort);
		GLog_Write("GProcThrd_Create：从GMem分配工作者内存失败");
		return;
	}

	DWORD i;
 	PGTHREAD pThread;
	
	pThread = pGProcThrdThreadAddr;
	for(i = 0; i < dwGProcThrdThreadNumber; i++)
	{
		GThrd_CreateThread(pThread, GTHREAD_TYPE_IOCP, "处理者", &GProcesserThreadProc);
		if(!pThread->dwThreadId)
		{
			GLog_Write("GProcThrd_Create：创建处理者线程失败");
			return;
		}
		pThread = pThread + 1;
	}
	bGProcThrdThreadIsActive = TRUE;		
}

void GProcThrd_Destroy(void)
{
	bGProcThrdThreadIsActive = FALSE;
	if(!pGProcThrdThreadAddr)
		return;

	PGTHREAD pThread;

	DWORD i;

	pThread = pGProcThrdThreadAddr;
	for(i = 0; i < dwGProcThrdThreadNumber; i++)
	{
		pThread->bIsShutdown = TRUE;
		pThread = pThread + 1;
	}
	
	for(i = 0; i < dwGProcThrdThreadNumber; i++)
		PostQueuedCompletionStatus(hGProcThrdThreadCompletionPort, 0, 0, NULL);
	
	pThread = pGProcThrdThreadAddr;
	for(i = 0; i < dwGProcThrdThreadNumber; i++)
	{
		GThrd_DestroyThread(pThread);
		pThread = pThread + 1;
	}

	GMem_Free(pGProcThrdThreadAddr);
	CloseHandle(hGProcThrdThreadCompletionPort);
	hGProcThrdThreadCompletionPort = 0;
	dwGProcThrdThreadNumber = 0;
	pGProcThrdThreadAddr = NULL;
}
  
#endif