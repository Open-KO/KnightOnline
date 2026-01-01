// DlgNewPart.cpp : implementation file
//

#include "stdafx.h"
#include "N3FXE.h"
#include "DlgNewPart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgNewPart dialog


CDlgNewPart::CDlgNewPart(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgNewPart::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgNewPart)
	m_iPartType = 0;
	//}}AFX_DATA_INIT
}


void CDlgNewPart::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgNewPart)
	DDX_Radio(pDX, IDC_RADIO1, m_iPartType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgNewPart, CDialog)
	//{{AFX_MSG_MAP(CDlgNewPart)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgNewPart message handlers

void CDlgNewPart::OnOK() 
{
	UpdateData(TRUE);	
	CDialog::OnOK();
}
