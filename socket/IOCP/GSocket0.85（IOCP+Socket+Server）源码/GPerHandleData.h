/********************************************************************************
*																				*
*					GSocket：完成端口通讯模块（IOCP Socket）					*
*								——GPerHandleData								*
*																				*
*				Copyright © 2009  代码客（卢益贵）  版权所有					*
*						未经许可，不得用于任何项目开发							*
*  QQ:48092788  E-Mail:gcode@qq.com  源码博客：http://blog.csdn.net/guestcode	*
*																				*
*					GSN:34674B4D-1F63-11D3-B64C-11C04F79498E					*
********************************************************************************/

#pragma once

#include "GSocket.h"

typedef struct _GHND_DATA
{
void*						pfnOnIocpOper;
void*						pfnOnIocpError;
SOCKET						Socket;
_GHND_DATA*					pNext;
_GHND_DATA*					pPrior;
GHND_TYPE					htType;
GHND_STATE					hsState;
DWORD						dwAddrFimily;
DWORD						dwAddr;
DWORD						dwPort;
DWORD						dwTickCountAcitve;
void*						pOwner;
void*						pData;
}GHND_DATA, *PGHND_DATA;

typedef	BOOL(*PFN_ON_GHND_CREATE)(PGHND_DATA pHndData);
typedef	void(*PFN_ON_GHND_DESTROY)(PGHND_DATA pHndData);

PGHND_DATA GHndDat_Alloc(void);
void GHndDat_Free(PGHND_DATA pHndData);

extern BOOL bGHndDataIsActive;
void GHndDat_Create(PFN_ON_GHND_CREATE pfnOnCreate);
void GHndDat_Destroy(PFN_ON_GHND_DESTROY pfnOnDestroy);


