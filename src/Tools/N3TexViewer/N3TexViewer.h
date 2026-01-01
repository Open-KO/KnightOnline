// N3TexViewer.h : main header file for the N3TEXVIEWER application
//

#if !defined(AFX_N3TEXVIEWER_H__BD91BC46_CB3C_4E31_96A6_6A3A22E1730B__INCLUDED_)
#define AFX_N3TEXVIEWER_H__BD91BC46_CB3C_4E31_96A6_6A3A22E1730B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerApp:
// See N3TexViewer.cpp for the implementation of this class
//

class CN3TexViewerApp : public CWinApp
{
public:
	CN3TexViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3TexViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CN3TexViewerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3TEXVIEWER_H__BD91BC46_CB3C_4E31_96A6_6A3A22E1730B__INCLUDED_)
