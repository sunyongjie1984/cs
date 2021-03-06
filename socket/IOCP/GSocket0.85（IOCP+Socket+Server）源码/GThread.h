/********************************************************************************
*																				*
*					GSocket：完成端口通讯模块（IOCP Socket）					*
*								——GThread										*
*																				*
*				Copyright © 2009  代码客（卢益贵）  版权所有					*
*						未经许可，不得用于任何项目开发							*
*  QQ:48092788  E-Mail:gcode@qq.com  源码博客：http://blog.csdn.net/guestcode	*
*																				*
*					GSN:34674B4D-1F63-11D3-B64C-11C04F79498E					*
********************************************************************************/

#pragma once

#include "GSocket.h"

typedef struct _GTHREAD
{
_GTHREAD*			pNext;
void*				pfnThreadProc;
char*				pThreadName;
DWORD				dwThreadId;
HANDLE				hFinished;

GTHREAD_TYPE		ttType;
DWORD				dwRunCount;
DWORD				dwState;
BOOL				bIsShutdown;
void*				pData;
}GTHREAD, *PGTHREAD;

typedef enum _GTHREAD_STATE
{
GTHREAD_STATE_SLEEP,
GTHREAD_STATE_WORKING1,
GTHREAD_STATE_WORKING2,
GTHREAD_STATE_WORKING3,
GTHREAD_STATE_WORKING4,
GTHREAD_STATE_WORKING5,
}GTHREAD_STATE;

typedef	void(*PFN_ON_GTHREAD_PROC)(PGTHREAD pThread);

void GThrd_CreateThread(PGTHREAD pThread, GTHREAD_TYPE ttType, char* szThreadName, PFN_ON_GTHREAD_PROC pfnThreadProc);
void GThrd_DestroyThread(PGTHREAD pThread);

void GThrd_Create(void);
void GThrd_Destroy(void);


