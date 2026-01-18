// DlgSaveEvt.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSaveEvt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveEvt dialog

CDlgSaveEvt::CDlgSaveEvt(CWnd* pParent /*=nullptr*/) : CDialog(CDlgSaveEvt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSaveEvt)
	m_FileName = _T("");
	//}}AFX_DATA_INIT
}

void CDlgSaveEvt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSaveEvt)
	DDX_Control(pDX, IDC_LIST_SAVED_EVT_FILE, m_SavedFileList);
	DDX_Text(pDX, IDC_EDIT_NEW_EVT_FILE, m_FileName);
	DDV_MaxChars(pDX, m_FileName, 255);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSaveEvt, CDialog)
//{{AFX_MSG_MAP(CDlgSaveEvt)
ON_LBN_SELCHANGE(IDC_LIST_SAVED_EVT_FILE, OnSelchangeListSavedEvtFile)
ON_LBN_DBLCLK(IDC_LIST_SAVED_EVT_FILE, OnDblclkListSavedEvtFile)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveEvt message handlers

void CDlgSaveEvt::OnSelchangeListSavedEvtFile()
{
	int CurrSel = m_SavedFileList.GetCurSel();
	m_SavedFileList.GetText(CurrSel, m_FileName);
	UpdateData(FALSE);
}

void CDlgSaveEvt::OnDblclkListSavedEvtFile()
{
	OnOK();
}

BOOL CDlgSaveEvt::OnInitDialog()
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
	m_SavedFileList.Dir(DDL_READONLY, "event\\*.evt");

	int count = m_SavedFileList.GetCount();

	CString str;
	for (int i = 0; i < count; i++)
	{
		m_SavedFileList.GetText(0, str);

		char szFileName[_MAX_PATH];
		char szExt[_MAX_EXT];
		_splitpath((LPCTSTR) str, nullptr, nullptr, szFileName, szExt);

		//str.Format("%s%s",szFileName,szExt);
		str.Format("%s", szFileName);
		m_SavedFileList.InsertString(count, str);
		m_SavedFileList.DeleteString(0);
	}

	SetCurrentDirectory(szOldPath);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}
