// ItemManager.h : main header file for the ITEMMANAGER application
//

#if !defined(AFX_ITEMMANAGER_H__9B641F5C_3768_400F_B6D3_5BC6392010E8__INCLUDED_)
#define AFX_ITEMMANAGER_H__9B641F5C_3768_400F_B6D3_5BC6392010E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CItemManagerApp:
// See ItemManager.cpp for the implementation of this class
//

class CItemManagerApp : public CWinApp
{
public:
	CItemManagerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CItemManagerApp)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CItemManagerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITEMMANAGER_H__9B641F5C_3768_400F_B6D3_5BC6392010E8__INCLUDED_)
