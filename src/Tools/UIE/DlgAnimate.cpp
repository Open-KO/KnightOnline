// DlgAnimate.cpp : implementation file
//

#include "stdafx.h"
#include "UIE.h"
#include "DlgAnimate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAnimate dialog


CDlgAnimate::CDlgAnimate(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgAnimate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAnimate)
	m_iCount = 0;
	//}}AFX_DATA_INIT
}


void CDlgAnimate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAnimate)
	DDX_Text(pDX, IDC_EDIT_COUNT, m_iCount);
	DDV_MinMaxInt(pDX, m_iCount, 0, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAnimate, CDialog)
	//{{AFX_MSG_MAP(CDlgAnimate)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAnimate message handlers
