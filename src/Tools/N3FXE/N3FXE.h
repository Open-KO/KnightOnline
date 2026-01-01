// N3FXE.h : main header file for the N3FXE application
//

#if !defined(AFX_N3FXE_H__9D64675E_A5BC_43E2_A97D_B9266331170F__INCLUDED_)
#define AFX_N3FXE_H__9D64675E_A5BC_43E2_A97D_B9266331170F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CN3FXEApp:
// See N3FXE.cpp for the implementation of this class
//

class CN3FXEApp : public CWinApp
{
public:
	CN3FXEApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3FXEApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CN3FXEApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3FXE_H__9D64675E_A5BC_43E2_A97D_B9266331170F__INCLUDED_)
