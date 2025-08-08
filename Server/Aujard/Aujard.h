// Aujard.h : main header file for the AUJARD application
//

#if !defined(AFX_AUJARD_H__8E466E6E_163E_4622_8610_501586363AD4__INCLUDED_)
#define AFX_AUJARD_H__8E466E6E_163E_4622_8610_501586363AD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAujardApp:
// See Aujard.cpp for the implementation of this class
//

class CAujardApp : public CWinApp
{
public:
	CAujardApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAujardApp)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAujardApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUJARD_H__8E466E6E_163E_4622_8610_501586363AD4__INCLUDED_)
