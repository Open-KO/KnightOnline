// DlgLoadTileSet.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgLoadTileSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadTileSet dialog


CDlgLoadTileSet::CDlgLoadTileSet(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgLoadTileSet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLoadTileSet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgLoadTileSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLoadTileSet)
	DDX_Control(pDX, IDC_LIST_LOAD_TILESET, m_ListTileSet);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLoadTileSet, CDialog)
	//{{AFX_MSG_MAP(CDlgLoadTileSet)
	ON_LBN_SELCHANGE(IDC_LIST_LOAD_TILESET, OnSelchangeListLoadTileset)
	ON_LBN_DBLCLK(IDC_LIST_LOAD_TILESET, OnDblclkListLoadTileset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadTileSet message handlers

BOOL CDlgLoadTileSet::OnInitDialog() 
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
	for(int i=0;i<count;i++)
	{
		m_ListTileSet.GetText(0, str);

		char szFileName[MAX_PATH];
		_splitpath((LPCTSTR)str, nullptr, nullptr, szFileName, nullptr);

		str.Format(szFileName);
		m_ListTileSet.InsertString(count, str);
		m_ListTileSet.DeleteString(0);
	}

	SetCurrentDirectory(szOldPath);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLoadTileSet::OnSelchangeListLoadTileset() 
{
	int CurrSel = m_ListTileSet.GetCurSel();
	m_ListTileSet.GetText(CurrSel, m_SelFileName);// TODO: Add your control notification handler code here

	char szFileName[MAX_PATH];
	_splitpath((LPCTSTR)m_SelFileName, nullptr, nullptr, szFileName, nullptr);

	m_SelFileName.Format(szFileName);
}

void CDlgLoadTileSet::OnDblclkListLoadTileset() 
{
	OnOK();	
}
