// DlgMakeWall.cpp : implementation file
//
#include "stdafx.h"
#include "n3me.h"
#include "DlgMakeWall.h"
#include "Wall.h"
#include "WallMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMakeWall dialog


CDlgMakeWall::CDlgMakeWall(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgMakeWall::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMakeWall)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pRefWallMgr = nullptr;
	m_pSelWall = nullptr;
}


void CDlgMakeWall::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMakeWall)
	DDX_Control(pDX, IDC_LIST_WALL, m_WallList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMakeWall, CDialog)
	//{{AFX_MSG_MAP(CDlgMakeWall)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_BN_CLICKED(IDC_BTN_WALL_EDIT, OnBtnWallEdit)
	ON_BN_CLICKED(IDC_BTN_WALL_RECORD, OnBtnWallRecord)
	ON_LBN_SELCHANGE(IDC_LIST_WALL, OnSelchangeListWall)
	ON_LBN_DBLCLK(IDC_LIST_WALL, OnDblclkListWall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMakeWall message handlers

void CDlgMakeWall::OnBtnDelete() 
{
	int idx = m_WallList.GetCurSel();
	CWall* pWall = (CWall*)m_WallList.GetItemDataPtr(idx);
	m_WallList.DeleteString(idx);

	if(pWall) m_pRefWallMgr->DelWall(pWall);
	m_pSelWall = nullptr;
}

void CDlgMakeWall::OnBtnWallEdit() 
{
	int idx = m_WallList.GetCurSel();
	if(idx<0) return;

	CString str;
	m_WallList.GetText(idx, str);

	SetDlgItemText(IDC_EDIT_WALL, (LPCTSTR)str);

	CWall* pWall = (CWall*)m_WallList.GetItemDataPtr(idx);

	m_WallList.DeleteString(idx);

	if(pWall)
	{
		m_pRefWallMgr->SetCurrWall(pWall);
	}
}

void CDlgMakeWall::OnBtnWallRecord() 
{
	CWall* pWall = m_pRefWallMgr->m_pCurrWall;
	int idx = 0;
	if(pWall)
	{
		CString CurrName;
		GetDlgItemText( IDC_EDIT_WALL, CurrName);
		strcpy(pWall->m_Name, (LPCTSTR)CurrName);

		idx = m_WallList.AddString((LPCTSTR)CurrName);
		
		m_WallList.SetItemDataPtr(idx, pWall);
		m_pRefWallMgr->UpdateWall();
	}
	int count = m_WallList.GetCount();
	CString str;
	str.Format("Wall%04d", count);
	SetDlgItemText(IDC_EDIT_WALL, (LPCTSTR)str);
	//UpdateData(FALSE);
}

void CDlgMakeWall::OnSelchangeListWall() 
{
	int idx = m_WallList.GetCurSel();
	m_pSelWall = (CWall*)m_WallList.GetItemDataPtr(idx);
}

BOOL CDlgMakeWall::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_WallList.ResetContent();

	CString str;
	str.Format("Wall0000");
	SetDlgItemText(IDC_EDIT_WALL, (LPCTSTR)str);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMakeWall::OnDblclkListWall() 
{
	int idx = m_WallList.GetCurSel();
	m_pSelWall = (CWall*)m_WallList.GetItemDataPtr(idx);

	m_pRefWallMgr->SetFocus(m_pSelWall);	
}

void CDlgMakeWall::ResetAll()
{
	m_WallList.ResetContent();

	CString str;
	str.Format("Wall0000");
	SetDlgItemText(IDC_EDIT_WALL, (LPCTSTR)str);

	std::list<CWall*>::iterator itWall;

	for(itWall = m_pRefWallMgr->m_pWalls.begin(); itWall != m_pRefWallMgr->m_pWalls.end(); itWall++)
	{
		int idx = m_WallList.AddString((*itWall)->m_Name);
		m_WallList.SetItemDataPtr(idx, (*itWall));
	}
}
