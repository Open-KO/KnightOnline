// DlgPercent.cpp : implementation file
//

#include "stdafx.h"
#include "n3fxe.h"
#include "DlgPercent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPercent dialog

CDlgPercent::CDlgPercent(CWnd* pParent /*=nullptr*/) : CDialog(CDlgPercent::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPercent)
	m_fPercent = 0.0f;
	//}}AFX_DATA_INIT
}

void CDlgPercent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPercent)
	DDX_Control(pDX, IDC_SLIDER1, m_Slider);
	DDX_Text(pDX, IDC_EDT_PERCENT, m_fPercent);
	DDV_MinMaxFloat(pDX, m_fPercent, 0.f, 100.f);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgPercent, CDialog)
//{{AFX_MSG_MAP(CDlgPercent)
ON_WM_HSCROLL()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPercent message handlers

void CDlgPercent::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	m_iRealValue = m_Slider.GetPos();
	m_fPercent   = (float) m_iRealValue / 2.55f;
	UpdateData(FALSE);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CDlgPercent::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_iRealValue = 0;
	m_Slider.SetRange(0, 255);
	m_Slider.SetPos(0);
	m_Slider.SetTicFreq(51);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}
