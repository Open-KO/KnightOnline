// DlgFormat.cpp : implementation file
//

#include "stdafx.h"
#include "n3texviewer.h"
#include "DlgFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFormat dialog


CDlgFormat::CDlgFormat(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgFormat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFormat)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_Fmt = D3DFMT_UNKNOWN;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bMipMap = FALSE;
}


void CDlgFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFormat)
	DDX_Control(pDX, IDC_CB_FORMAT, m_CBFormat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFormat, CDialog)
	//{{AFX_MSG_MAP(CDlgFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFormat message handlers

BOOL CDlgFormat::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int n = 0;
	m_CBFormat.ResetContent();
	n = m_CBFormat.AddString("DXT1"); m_CBFormat.SetItemData(n, D3DFMT_DXT1);
	n = m_CBFormat.AddString("DXT2"); m_CBFormat.SetItemData(n, D3DFMT_DXT2);
	n = m_CBFormat.AddString("DXT3"); m_CBFormat.SetItemData(n, D3DFMT_DXT3);
	n = m_CBFormat.AddString("DXT4"); m_CBFormat.SetItemData(n, D3DFMT_DXT4);
	n = m_CBFormat.AddString("DXT5"); m_CBFormat.SetItemData(n, D3DFMT_DXT5);
	n = m_CBFormat.AddString("A1R5G5B5"); m_CBFormat.SetItemData(n, D3DFMT_A1R5G5B5);
	n = m_CBFormat.AddString("A4R4G4B4"); m_CBFormat.SetItemData(n, D3DFMT_A4R4G4B4);
	n = m_CBFormat.AddString("R8G8B8"); m_CBFormat.SetItemData(n, D3DFMT_R8G8B8);
	n = m_CBFormat.AddString("A8R8G8B8"); m_CBFormat.SetItemData(n, D3DFMT_A8R8G8B8);
	n = m_CBFormat.AddString("X8R8G8B8"); m_CBFormat.SetItemData(n, D3DFMT_X8R8G8B8);
	m_CBFormat.SetCurSel(0);
	
	if(D3DFMT_DXT1 == m_Fmt) m_CBFormat.SetCurSel(0);
	else if(D3DFMT_DXT2 == m_Fmt) m_CBFormat.SetCurSel(1);
	else if(D3DFMT_DXT3 == m_Fmt) m_CBFormat.SetCurSel(2);
	else if(D3DFMT_DXT4 == m_Fmt) m_CBFormat.SetCurSel(3);
	else if(D3DFMT_DXT5 == m_Fmt) m_CBFormat.SetCurSel(4);
	else if(D3DFMT_A1R5G5B5 == m_Fmt) m_CBFormat.SetCurSel(5);
	else if(D3DFMT_A4R4G4B4 == m_Fmt) m_CBFormat.SetCurSel(6);
	else if(D3DFMT_R8G8B8 == m_Fmt) m_CBFormat.SetCurSel(7);
	else if(D3DFMT_A8R8G8B8 == m_Fmt) m_CBFormat.SetCurSel(8);
	else if(D3DFMT_X8R8G8B8 == m_Fmt) m_CBFormat.SetCurSel(9);

	SetDlgItemInt(IDC_E_WIDTH, m_nWidth);
	SetDlgItemInt(IDC_E_HEIGHT, m_nHeight);
	CheckDlgButton(IDC_C_GENERATE_MIPMAP, m_bMipMap);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFormat::OnOK() 
{
	m_Fmt = D3DFMT_UNKNOWN;
	int nSel = m_CBFormat.GetCurSel();
	if(0 == nSel) m_Fmt = D3DFMT_DXT1;
	else if(1 == nSel) m_Fmt = D3DFMT_DXT2;
	else if(2 == nSel) m_Fmt = D3DFMT_DXT3;
	else if(3 == nSel) m_Fmt = D3DFMT_DXT4;
	else if(4 == nSel) m_Fmt = D3DFMT_DXT5;
	else if(5 == nSel) m_Fmt = D3DFMT_A1R5G5B5;
	else if(6 == nSel) m_Fmt = D3DFMT_A4R4G4B4;
	else if(7 == nSel) m_Fmt = D3DFMT_R8G8B8;
	else if(8 == nSel) m_Fmt = D3DFMT_A8R8G8B8;
	else if(9 == nSel) m_Fmt = D3DFMT_X8R8G8B8;

	m_nWidth = GetDlgItemInt(IDC_E_WIDTH);
	m_nHeight = GetDlgItemInt(IDC_E_HEIGHT);
	m_bMipMap = IsDlgButtonChecked(IDC_C_GENERATE_MIPMAP);
	
	if(D3DFMT_UNKNOWN != m_Fmt && m_nWidth / 2 * 2 == m_nWidth && m_nHeight / 2 * 2 == m_nHeight)
	{
		CDialog::OnOK();
	}
	else
	{
		MessageBox("잘못된 포맷이거나 텍스처의 크기가 2의 승수가 아닙니다.");
	}
}
