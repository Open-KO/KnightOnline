// UIE.h : main header file for the UIE application
//

#if !defined(AFX_UIE_H__83F51B13_FAF0_4DD2_8383_7BE9C8C4AD44__INCLUDED_)
#define AFX_UIE_H__83F51B13_FAF0_4DD2_8383_7BE9C8C4AD44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CUIEApp:
// See UIE.cpp for the implementation of this class
//

class CUIEApp : public CWinApp
{
public:
	CUIEApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIEApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CUIEApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

// global function
BOOL SelectTexture(char* pszBuff);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIE_H__83F51B13_FAF0_4DD2_8383_7BE9C8C4AD44__INCLUDED_)
