// DlgInputAttr.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgInputAttr.h"
#include "MainFrm.h"
#include "DTexGroupMng.h"
#include "DTexGroup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgInputAttr dialog
CDlgInputAttr::CDlgInputAttr(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgInputAttr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgInputAttr)
	//}}AFX_DATA_INIT
}


void CDlgInputAttr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgInputAttr)
	DDX_Control(pDX, IDC_LB_GROUP, m_LBGroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgInputAttr, CDialog)
	//{{AFX_MSG_MAP(CDlgInputAttr)
	ON_LBN_DBLCLK(IDC_LB_GROUP, OnDblclkLbGroup)
	ON_LBN_SELCHANGE(IDC_LB_GROUP, OnSelchangeLbGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInputAttr message handlers

BOOL CDlgInputAttr::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetWindowPos(nullptr, m_Point.x, m_Point.y, 0, 0, SWP_NOSIZE);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

	auto it = pDTexGroupMng->m_Groups.begin();
	int iSize = static_cast<int>(pDTexGroupMng->m_Groups.size());
	for (int i = 0; i < iSize; i++, it++)
	{
		CDTexGroup* pDTG = *it;
		m_LBGroup.InsertString(i, pDTG->m_Name);
		m_LBGroup.SetItemData(i, (DWORD_PTR) pDTG->m_ID);
	}

	m_LBGroup.SetCurSel(0);
	m_SelectedGroupID = static_cast<int>(m_LBGroup.GetItemData(0));
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInputAttr::OnDblclkLbGroup() 
{
	CDialog::OnOK();
}

void CDlgInputAttr::OnSelchangeLbGroup() 
{
	int CurSel = m_LBGroup.GetCurSel();
	m_SelectedGroupID = static_cast<int>(m_LBGroup.GetItemData(CurSel));
}
