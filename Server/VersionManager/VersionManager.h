// VersionManager.h : main header file for the VERSIONMANAGER application
//

#if !defined(AFX_VERSIONMANAGER_H__0D350B04_57C4_481C_AF95_CD1337B02391__INCLUDED_)
#define AFX_VERSIONMANAGER_H__0D350B04_57C4_481C_AF95_CD1337B02391__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVersionManagerApp:
// See VersionManager.cpp for the implementation of this class
//

class CVersionManagerApp : public CWinApp
{
public:
	CVersionManagerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVersionManagerApp)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVersionManagerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERSIONMANAGER_H__0D350B04_57C4_481C_AF95_CD1337B02391__INCLUDED_)
