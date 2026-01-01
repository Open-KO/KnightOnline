#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTblEditorApp:
// See TblEditor.cpp for the implementation of this class
//

class CShellManager;
class CTblEditorApp : public CWinApp
{
public:
	CTblEditorApp();
	~CTblEditorApp() override;

// Overrides
public:
	BOOL InitInstance() override;
	BOOL ProcessMessageFilter(int code, LPMSG lpMsg) override;

// Implementation

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();

protected:
	CShellManager* m_pShellManager;
	HACCEL m_hAccelTable;
};

extern CTblEditorApp theApp;
