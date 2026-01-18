// DlgInputGroup.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgInputGroup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgInputGroup dialog
CDlgInputGroup::CDlgInputGroup(CWnd* pParent /*=nullptr*/) : CDialog(CDlgInputGroup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgInputGroup)
	m_GroupName = _T("");
	//}}AFX_DATA_INIT
}

void CDlgInputGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgInputGroup)
	DDX_Text(pDX, IDC_EDIT_INPUTGROUP, m_GroupName);
	DDV_MaxChars(pDX, m_GroupName, 40);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgInputGroup, CDialog)
//{{AFX_MSG_MAP(CDlgInputGroup)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInputGroup message handlers
