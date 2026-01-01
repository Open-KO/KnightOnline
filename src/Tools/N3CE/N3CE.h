// N3CE.h : main header file for the N3CE application
//

#if !defined(AFX_N3CE_H__1E0C5C82_CAFE_44FC_AD84_DDCDED50FA12__INCLUDED_)
#define AFX_N3CE_H__1E0C5C82_CAFE_44FC_AD84_DDCDED50FA12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CN3CEApp:
// See N3CE.cpp for the implementation of this class
//

class CN3CEApp : public CWinApp
{
public:
	CN3CEApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3CEApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CN3CEApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3CE_H__1E0C5C82_CAFE_44FC_AD84_DDCDED50FA12__INCLUDED_)
