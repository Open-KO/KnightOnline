// DlgAddDTex.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgAddDTex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAddDTex dialog

CDlgAddDTex::CDlgAddDTex(CWnd* pParent /*=nullptr*/) : CDialog(CDlgAddDTex::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAddDTex)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgAddDTex::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAddDTex)
	DDX_Control(pDX, IDC_ADDDTEXLIST, m_TexList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgAddDTex, CDialog)
//{{AFX_MSG_MAP(CDlgAddDTex)
ON_LBN_SELCHANGE(IDC_ADDDTEXLIST, OnSelchangeAdddtexlist)
ON_LBN_DBLCLK(IDC_ADDDTEXLIST, OnDblclkAdddtexlist)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAddDTex message handlers

void CDlgAddDTex::OnSelchangeAdddtexlist()
{
	int CurrSel = m_TexList.GetCurSel();
	//m_TexList.SetCurSel(CurrSel);
	m_TexList.GetText(CurrSel, m_TexName);
}

void CDlgAddDTex::OnDblclkAdddtexlist()
{
	OnOK();
}

BOOL CDlgAddDTex::OnInitDialog()
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
	m_TexList.Dir(DDL_READONLY, "dtex\\*.bmp");
	SetCurrentDirectory(szOldPath);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}
