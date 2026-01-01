// DlgReplace.cpp : implementation file
//

#include "stdafx.h"
#include "uie.h"
#include "DlgReplace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgReplace dialog


CDlgReplace::CDlgReplace(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgReplace::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgReplace)
	m_strFind = _T("");
	m_strReplace = _T("");
	//}}AFX_DATA_INIT
}


void CDlgReplace::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReplace)
	DDX_Text(pDX, IDC_EDIT_FIND, m_strFind);
	DDX_Text(pDX, IDC_EDIT_REPLACE, m_strReplace);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgReplace, CDialog)
	//{{AFX_MSG_MAP(CDlgReplace)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReplace message handlers
