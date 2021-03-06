/********************************************************************************
*																				*
*					GSocket：完成端口通讯模块（IOCP Socket）					*
*								——GWorkerThread								*
*																				*
*				Copyright © 2009  代码客（卢益贵）  版权所有					*
*						未经许可，不得用于任何项目开发							*
*  QQ:48092788  E-Mail:gcode@qq.com  源码博客：http://blog.csdn.net/guestcode	*
*																				*
*					GSN:34674B4D-1F63-11D3-B64C-11C04F79498E					*
********************************************************************************/

#pragma once

typedef	void(*PFN_ON_GIOCP_ERROR)(void* pCompletionKey, void* pOverlapped);
typedef	void(*PFN_ON_GIOCP_OPER)(DWORD dwBytes, void* pCompletionKey, void* pOverlapped);

typedef struct _COMPLETION_KEY
{
PFN_ON_GIOCP_OPER			pfnOnIocpOper;
PFN_ON_GIOCP_ERROR			pfnOnIocpError;
}GIOCP_COMPLETION_KEY, *PGIOCP_COMPLETION_KEY;


extern HANDLE hGWkrThrdCompletionPort;
extern BOOL bGWkrThrdIsActive;
void GWkrThrd_Create(void);
void GWkrThrd_Destroy(void);

