// DlgPMeshCreateOption.cpp : implementation file
//

#include "stdafx.h"
#include "DlgPMeshCreateOption.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPMeshCreateOption dialog

CDlgPMeshCreateOption::CDlgPMeshCreateOption(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgPMeshCreateOption::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPMeshCreateOption)
	m_bTriangleLoss = TRUE;
	m_fWeight       = 4.0f;
	m_iLoss         = 1;
	m_iTriangle     = 0;
	//}}AFX_DATA_INIT
}

void CDlgPMeshCreateOption::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPMeshCreateOption)
	DDX_Check(pDX, IDC_CHECK_TRIANGLE_LOSS, m_bTriangleLoss);
	DDX_Text(pDX, IDC_EDIT_WEIGHT, m_fWeight);
	DDX_Radio(pDX, IDC_RADIO_MOST_LOSS, m_iLoss);
	DDX_Radio(pDX, IDC_RADIO_AREA, m_iTriangle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgPMeshCreateOption, CDialog)
//{{AFX_MSG_MAP(CDlgPMeshCreateOption)
ON_BN_CLICKED(IDC_CHECK_TRIANGLE_LOSS, OnCheckTriangleLoss)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPMeshCreateOption message handlers

BOOL CDlgPMeshCreateOption::OnInitDialog()
{
	CDialog::OnInitDialog();

	OnCheckTriangleLoss();
	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPMeshCreateOption::OnCheckTriangleLoss()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_RADIO_AREA)->EnableWindow(m_bTriangleLoss);
	GetDlgItem(IDC_RADIO_SIDE)->EnableWindow(m_bTriangleLoss);
	GetDlgItem(IDC_EDIT_WEIGHT)->EnableWindow(m_bTriangleLoss);
	GetDlgItem(IDC_RADIO_AREA)->EnableWindow(m_bTriangleLoss);
}
