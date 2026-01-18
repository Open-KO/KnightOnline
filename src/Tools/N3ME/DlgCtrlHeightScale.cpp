// DlgCtrlHeightScale.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgCtrlHeightScale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlHeightScale dialog

CDlgCtrlHeightScale::CDlgCtrlHeightScale(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgCtrlHeightScale::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlHeightScale)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgCtrlHeightScale::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlHeightScale)
	DDX_Control(pDX, IDC_SLIDER1, m_sldr_Scale);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgCtrlHeightScale, CDialog)
//{{AFX_MSG_MAP(CDlgCtrlHeightScale)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlHeightScale message handlers
