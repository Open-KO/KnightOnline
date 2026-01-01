// Launcher.h : main header file for the LAUNCHER application
//

#if !defined(AFX_LAUNCHER_H__E8289BE5_E48B_4B72_AA38_5C6AFAA024EC__INCLUDED_)
#define AFX_LAUNCHER_H__E8289BE5_E48B_4B72_AA38_5C6AFAA024EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp:
// See Launcher.cpp for the implementation of this class
//

class CLauncherApp : public CWinApp
{
public:
	CLauncherApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLauncherApp)
	public:
	virtual BOOL InitInstance();
	virtual CWnd* GetMainWnd();
	virtual void DoWaitCursor(int nCode);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLauncherApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAUNCHER_H__E8289BE5_E48B_4B72_AA38_5C6AFAA024EC__INCLUDED_)
