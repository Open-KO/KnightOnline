// DlgLoadNPCPath.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgLoadNPCPath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadNPCPath dialog


CDlgLoadNPCPath::CDlgLoadNPCPath(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgLoadNPCPath::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLoadNPCPath)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgLoadNPCPath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLoadNPCPath)
	DDX_Control(pDX, IDC_LIST_NPCPATH, m_NPCPathFileList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLoadNPCPath, CDialog)
	//{{AFX_MSG_MAP(CDlgLoadNPCPath)
	ON_LBN_SELCHANGE(IDC_LIST_NPCPATH, OnSelchangeListNpcpath)
	ON_LBN_DBLCLK(IDC_LIST_NPCPATH, OnDblclkListNpcpath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadNPCPath message handlers

void CDlgLoadNPCPath::OnSelchangeListNpcpath() 
{
	int CurrSel = m_NPCPathFileList.GetCurSel();
	m_NPCPathFileList.GetText(CurrSel, m_SelFileName);// TODO: Add your control notification handler code here
}

void CDlgLoadNPCPath::OnDblclkListNpcpath() 
{
	OnOK();
}

BOOL CDlgLoadNPCPath::OnInitDialog() 
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
	m_NPCPathFileList.Dir(DDL_READONLY, "NPCPath\\*.npi");

	int count = m_NPCPathFileList.GetCount();

	CString str;
	for(int i=0;i<count;i++)
	{
		m_NPCPathFileList.GetText(0, str);

		char szFileName[MAX_PATH];
		char szExt[_MAX_EXT];
		_splitpath((LPCTSTR)str, nullptr, nullptr, szFileName, szExt);

		//str.Format("%s%s",szFileName,szExt);
		str.Format("%s",szFileName);
		m_NPCPathFileList.InsertString(count, str);
		m_NPCPathFileList.DeleteString(0);
	}

	SetCurrentDirectory(szOldPath);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
