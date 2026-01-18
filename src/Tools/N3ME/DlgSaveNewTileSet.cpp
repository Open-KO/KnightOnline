// DlgSaveNewTileSet.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSaveNewTileSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveNewTileSet dialog

CDlgSaveNewTileSet::CDlgSaveNewTileSet(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgSaveNewTileSet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSaveNewTileSet)
	m_NewTileSetName = _T("");
	//}}AFX_DATA_INIT
}

void CDlgSaveNewTileSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSaveNewTileSet)
	DDX_Control(pDX, IDC_LIST_TILESET, m_ListTileSet);
	DDX_Text(pDX, IDC_EDIT_NEWTILESETNAME, m_NewTileSetName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSaveNewTileSet, CDialog)
//{{AFX_MSG_MAP(CDlgSaveNewTileSet)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveNewTileSet message handlers

BOOL CDlgSaveNewTileSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	char szOldPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szOldPath);

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	char szModuleFilePath[_MAX_PATH];
	GetModuleFileName(nullptr, szModuleFilePath, _MAX_PATH);

	char szNewPath[_MAX_PATH];
	_splitpath(szModuleFilePath, szDrive, szDir, nullptr, nullptr);
	_makepath(szNewPath, szDrive, szDir, nullptr, nullptr);
	SetCurrentDirectory(szNewPath);
	m_ListTileSet.Dir(DDL_READONLY, "dtex\\*.dtx");

	int count = m_ListTileSet.GetCount();

	CString str;
	for (int i = 0; i < count; i++)
	{
		m_ListTileSet.GetText(0, str);

		char szFileName[MAX_PATH];
		_splitpath((LPCTSTR) str, nullptr, nullptr, szFileName, nullptr);

		str.Format(szFileName);
		m_ListTileSet.InsertString(count, str);
		m_ListTileSet.DeleteString(0);
	}

	SetCurrentDirectory(szOldPath);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}
