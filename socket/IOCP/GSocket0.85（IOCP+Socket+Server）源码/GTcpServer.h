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

#pragma once

#if(_GSOCKET_FUNC_TCP_SERVER)

extern BOOL bGTcpSvrIsActive;

void GTcpSvr_CloseClients(void);
void GTcpSvr_EndThread(void);
void GTcpSvr_CloseListeners(void);
void GTcpSvr_CloseListenerEvents(void);
void GTcpSvr_FreeListener(void);

void GTcpSvr_Create(void);
void GTcpSvr_Destroy(void);

#endif//#if(_GSOCKET_FUNC_TCP_SERVER)
