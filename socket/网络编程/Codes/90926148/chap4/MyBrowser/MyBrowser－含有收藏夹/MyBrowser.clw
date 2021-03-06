; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMyBrowserView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "MyBrowser.h"
LastPage=0

ClassCount=5
Class1=CMyBrowserApp
Class2=CMyBrowserDoc
Class3=CMyBrowserView
Class4=CMainFrame

ResourceCount=2
Resource1=IDR_MAINFRAME
Class5=CAboutDlg
Resource2=IDD_ABOUTBOX

[CLS:CMyBrowserApp]
Type=0
HeaderFile=MyBrowser.h
ImplementationFile=MyBrowser.cpp
Filter=N

[CLS:CMyBrowserDoc]
Type=0
HeaderFile=MyBrowserDoc.h
ImplementationFile=MyBrowserDoc.cpp
Filter=N

[CLS:CMyBrowserView]
Type=0
HeaderFile=MyBrowserView.h
ImplementationFile=MyBrowserView.cpp
Filter=C
BaseClass=CHtmlView
VirtualFilter=7VWC
LastObject=ID_START_PAGE


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T




[CLS:CAboutDlg]
Type=0
HeaderFile=MyBrowser.cpp
ImplementationFile=MyBrowser.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_SETUP
Command7=ID_FILE_MRU_FILE1
Command8=ID_APP_EXIT
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_VIEW_TOOLBAR
Command14=ID_VIEW_STATUS_BAR
Command15=ID_GO_FORWARD
Command16=ID_GO_BACK
Command17=ID_GO_REFRESH
Command18=ID_GO_STOP
Command19=ID_GO_SERACH
Command20=ID_START_PAGE
Command21=IDC_FAVORITE
Command22=ID_APP_ABOUT
CommandCount=22

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

