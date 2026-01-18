// DlgBar.cpp : implementation file
//

#include "stdafx.h"
#include "UIE.h"
#include "DlgBar.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBar dialog

CDlgBar::CDlgBar(CWnd* pParent /*=nullptr*/) : CDialog(CDlgBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgBar)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBar)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgBar, CDialog)
//{{AFX_MSG_MAP(CDlgBar)
ON_BN_CLICKED(IDC_BUTTON_BASEPATH, OnButtonBasepath)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBar message handlers

void CDlgBar::OnButtonBasepath()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (nullptr == pFrm)
		return;

	CWnd* pWnd = GetDlgItem(IDC_EDIT_BASEPATH);
	CString strPath;
	if (pWnd)
		pWnd->GetWindowText(strPath);
	char szFolder[_MAX_PATH] = "";
	char szTitle[]           = "Select a base path:"; //"리소스 base경로를 선택해주세요.";

	BROWSEINFO bi;
	LPCITEMIDLIST lpidl;
	bi.hwndOwner      = pFrm->m_hWnd;
	bi.pidlRoot       = nullptr;
	bi.pszDisplayName = szFolder;
	bi.lpszTitle      = szTitle;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS;
	bi.lpfn           = nullptr;
	bi.lParam         = 0;

	lpidl             = SHBrowseForFolder(&bi);
	if (lpidl && SHGetPathFromIDList(lpidl, szFolder))
	{
		pFrm->SetBasePath(szFolder);

		CWinApp* pApp = AfxGetApp();
		ASSERT(pApp);
		pApp->WriteProfileString("Work", "Path", szFolder);
	}
}
