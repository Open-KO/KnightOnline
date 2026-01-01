// N3ME.h : main header file for the N3ME application
//

#if !defined(AFX_N3ME_H__D671E217_CF08_471A_B577_0055D4F66932__INCLUDED_)
#define AFX_N3ME_H__D671E217_CF08_471A_B577_0055D4F66932__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CN3MEApp:
// See N3ME.cpp for the implementation of this class
//

class CN3MEApp : public CWinApp
{
public:
	CN3MEApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3MEApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CN3MEApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3ME_H__D671E217_CF08_471A_B577_0055D4F66932__INCLUDED_)
