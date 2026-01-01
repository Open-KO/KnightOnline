// SkyViewer.h : main header file for the SKYVIEWER application
//

#if !defined(AFX_SKYVIEWER_H__836D22A7_96E6_46B7_A0D7_CF10AD65C812__INCLUDED_)
#define AFX_SKYVIEWER_H__836D22A7_96E6_46B7_A0D7_CF10AD65C812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerApp:
// See SkyViewer.cpp for the implementation of this class
//

class CSkyViewerApp : public CWinApp
{
public:
	CSkyViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkyViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CSkyViewerApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYVIEWER_H__836D22A7_96E6_46B7_A0D7_CF10AD65C812__INCLUDED_)
