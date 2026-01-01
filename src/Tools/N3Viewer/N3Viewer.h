// N3Viewer.h : main header file for the N3VIEWER application
//

#if !defined(AFX_N3VIEWER_H__6AE8CCFC_D85F_4339_A6ED_9E2CD70ACBBE__INCLUDED_)
#define AFX_N3VIEWER_H__6AE8CCFC_D85F_4339_A6ED_9E2CD70ACBBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerApp:
// See N3Viewer.cpp for the implementation of this class
//

class CN3ViewerApp : public CWinApp
{
public:
	CN3ViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3ViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CN3ViewerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3VIEWER_H__6AE8CCFC_D85F_4339_A6ED_9E2CD70ACBBE__INCLUDED_)
