// DlgLoadEvt.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgLoadEvt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadEvt dialog

CDlgLoadEvt::CDlgLoadEvt(CWnd* pParent /*=nullptr*/) : CDialog(CDlgLoadEvt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLoadEvt)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgLoadEvt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLoadEvt)
	DDX_Control(pDX, IDC_LIST_SAVED_EVT_FILE, m_EvtFileList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgLoadEvt, CDialog)
//{{AFX_MSG_MAP(CDlgLoadEvt)
ON_LBN_DBLCLK(IDC_LIST_SAVED_EVT_FILE, OnDblclkListSavedEvtFile)
ON_LBN_SELCHANGE(IDC_LIST_SAVED_EVT_FILE, OnSelchangeListSavedEvtFile)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadEvt message handlers

BOOL CDlgLoadEvt::OnInitDialog()
{
	CDialog::OnInitDialog();

	char szOldPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szOldPath);

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	char szModuleFilePath[_MAX_PATH];
	GetModuleFileName(nullptr, szModuleFilePath, _MAX_PATH);

	char szNewPath[_MAX_PATH];
	_splitpath(szModuleFilePath, szDrive, szDir, nullptr, nullptr);
	_makepath(szNewPath, szDrive, szDir, nullptr, nullptr);
	SetCurrentDirectory(szNewPath);
	m_EvtFileList.Dir(DDL_READONLY, "Event\\*.evt");

	int count = m_EvtFileList.GetCount();

	CString str;
	for (int i = 0; i < count; i++)
	{
		m_EvtFileList.GetText(0, str);

		char szFileName[MAX_PATH];
		char szExt[_MAX_EXT];
		_splitpath((LPCTSTR) str, nullptr, nullptr, szFileName, szExt);

		//str.Format("%s%s",szFileName,szExt);
		str.Format("%s", szFileName);
		m_EvtFileList.InsertString(count, str);
		m_EvtFileList.DeleteString(0);
	}

	SetCurrentDirectory(szOldPath);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLoadEvt::OnDblclkListSavedEvtFile()
{
	OnOK();
}

void CDlgLoadEvt::OnSelchangeListSavedEvtFile()
{
	int CurrSel = m_EvtFileList.GetCurSel();
	m_EvtFileList.GetText(CurrSel, m_SelFileName);
}
