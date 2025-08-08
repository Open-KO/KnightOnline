// Ebenezer.h : main header file for the EBENEZER application
//

#if !defined(AFX_EBENEZER_H__CCD753A4_B614_486A_895D_330D83D9775F__INCLUDED_)
#define AFX_EBENEZER_H__CCD753A4_B614_486A_895D_330D83D9775F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEbenezerApp:
// See Ebenezer.cpp for the implementation of this class
//

class CEbenezerApp : public CWinApp
{
public:
	CEbenezerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEbenezerApp)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEbenezerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBENEZER_H__CCD753A4_B614_486A_895D_330D83D9775F__INCLUDED_)
