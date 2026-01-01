// DlgDelGroup.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgDelGroup.h"
#include "MainFrm.h"
#include "DTexGroupMng.h"
#include "DTexGroup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDelGroup dialog
CDlgDelGroup::CDlgDelGroup(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgDelGroup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDelGroup)
	//}}AFX_DATA_INIT
	m_SelGroupID = -1;
}


void CDlgDelGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDelGroup)
	DDX_Control(pDX, IDC_LIST_DELGROUP, m_GroupList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDelGroup, CDialog)
	//{{AFX_MSG_MAP(CDlgDelGroup)
	ON_LBN_SELCHANGE(IDC_LIST_DELGROUP, OnSelchangeListDelgroup)
	ON_LBN_DBLCLK(IDC_LIST_DELGROUP, OnDblclkListDelgroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDelGroup message handlers

BOOL CDlgDelGroup::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

	auto it = pDTexGroupMng->m_Groups.begin();
	int iSize = static_cast<int>(pDTexGroupMng->m_Groups.size());
	for (int i = 0; i < iSize; i++, it++)
	{
		CDTexGroup* pDTG = *it;
		m_GroupList.InsertString(i, pDTG->m_Name);
		m_GroupList.SetItemData(i, (DWORD_PTR) pDTG->m_ID);
	}
	m_GroupList.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDelGroup::OnSelchangeListDelgroup() 
{
	int CurrSel = m_GroupList.GetCurSel();
	m_SelGroupID = (int)m_GroupList.GetItemData(CurrSel);
}

void CDlgDelGroup::OnDblclkListDelgroup() 
{
	OnOK();	
}
