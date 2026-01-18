// DlgSaveNPCPath.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSaveNPCPath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveNPCPath dialog

CDlgSaveNPCPath::CDlgSaveNPCPath(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgSaveNPCPath::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSaveNPCPath)
	m_NewFileName = _T("");
	//}}AFX_DATA_INIT
}

void CDlgSaveNPCPath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSaveNPCPath)
	DDX_Control(pDX, IDC_LIST_NPCPATHFILENAME, m_SavedFileList);
	DDX_Text(pDX, IDC_EDIT_NPCPATHFILENAME, m_NewFileName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSaveNPCPath, CDialog)
//{{AFX_MSG_MAP(CDlgSaveNPCPath)
ON_LBN_SELCHANGE(IDC_LIST_NPCPATHFILENAME, OnSelchangeListNpcpathfilename)
ON_LBN_DBLCLK(IDC_LIST_NPCPATHFILENAME, OnDblclkListNpcpathfilename)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveNPCPath message handlers

void CDlgSaveNPCPath::OnSelchangeListNpcpathfilename()
{
	int CurrSel = m_SavedFileList.GetCurSel();
	m_SavedFileList.GetText(CurrSel, m_NewFileName);
	UpdateData(FALSE);
}

void CDlgSaveNPCPath::OnDblclkListNpcpathfilename()
{
	OnOK();
}

BOOL CDlgSaveNPCPath::OnInitDialog()
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
	m_SavedFileList.Dir(DDL_READONLY, "npcpath\\*.npi");

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
