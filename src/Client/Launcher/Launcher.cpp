// Launcher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Launcher.h"
#include "LauncherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp

BEGIN_MESSAGE_MAP(CLauncherApp, CWinApp)
//{{AFX_MSG_MAP(CLauncherApp)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG
ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp construction

CLauncherApp::CLauncherApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLauncherApp object

CLauncherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp initialization

BOOL CLauncherApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	//	SetDialogBkColor(RGB(255, 0, 255), RGB(0, 255, 0));

	CLauncherDlg dlg;
	m_pMainWnd        = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.

	return FALSE;
}

CWnd* CLauncherApp::GetMainWnd()
{
	// TODO: Add your specialized code here and/or call the base class

	return CWinApp::GetMainWnd();
}

void CLauncherApp::DoWaitCursor(int nCode)
{
	// TODO: Add your specialized code here and/or call the base class

	CWinApp::DoWaitCursor(nCode);
}
