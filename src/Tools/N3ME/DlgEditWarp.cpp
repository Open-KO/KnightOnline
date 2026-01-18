// DlgEditWarp.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgEditWarp.h"
#include "WarpMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditWarp dialog

CDlgEditWarp::CDlgEditWarp(CWnd* pParent /*=nullptr*/) : CDialog(CDlgEditWarp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditWarp)
	m_fRadius     = 0.0f;
	m_fPosX       = 0.0f;
	m_fPosY       = 0.0f;
	m_fPosZ       = 0.0f;
	m_strName     = _T("");
	m_iMoney      = 0;
	m_sID         = 0;
	m_sZoneID     = 0;
	m_AgreeState  = _T("");
	//}}AFX_DATA_INIT

	m_pRefWarpMgr = nullptr;
}

void CDlgEditWarp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditWarp)
	DDX_Control(pDX, IDC_LIST_WARP, m_ListWarp);
	DDX_Text(pDX, IDC_EDIT_RAD, m_fRadius);
	DDX_Text(pDX, IDC_EDIT_POS_X, m_fPosX);
	DDX_Text(pDX, IDC_EDIT_POS_Y, m_fPosY);
	DDX_Text(pDX, IDC_EDIT_POS_Z, m_fPosZ);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDV_MaxChars(pDX, m_strName, 32);
	DDX_Text(pDX, IDC_EDIT_MONEY, m_iMoney);
	DDX_Text(pDX, IDC_EDIT_ID, m_sID);
	DDV_MinMaxInt(pDX, m_sID, 0, 9999);
	DDX_Text(pDX, IDC_EDIT_ZONE, m_sZoneID);
	DDX_Text(pDX, IDC_EDIT_AGREE, m_AgreeState);
	DDV_MaxChars(pDX, m_AgreeState, 256);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgEditWarp, CDialog)
//{{AFX_MSG_MAP(CDlgEditWarp)
ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
ON_BN_CLICKED(IDC_BTN_EDIT, OnBtnEdit)
ON_BN_CLICKED(IDC_BTN_UPDATE, OnBtnUpdate)
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_BTN_EXPORT, OnBtnExport)
ON_BN_CLICKED(IDC_BTN_IMPORT, OnBtnImport)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditWarp message handlers

void CDlgEditWarp::OnBtnDelete()
{
	int idx = m_ListWarp.GetCurSel();
	if (idx < 0)
		return;

	WARPINFO* pWI = (WARPINFO*) m_ListWarp.GetItemDataPtr(idx);
	m_pRefWarpMgr->DelInfo(pWI);

	m_ListWarp.DeleteString(idx);
}

void CDlgEditWarp::OnBtnEdit()
{
	int idx = m_ListWarp.GetCurSel();
	if (idx < 0)
		return;

	WARPINFO* pWI = (WARPINFO*) m_ListWarp.GetItemDataPtr(idx);

	m_tmpInfo     = (*pWI);
	SetEditor();

	OnBtnDelete();
}

void CDlgEditWarp::OnBtnUpdate()
{
	GetEditor();
	WARPINFO* pWI = new WARPINFO;
	(*pWI)        = m_tmpInfo;

	int idx       = m_ListWarp.AddString(pWI->szName);
	m_ListWarp.SetItemDataPtr(idx, pWI);

	m_pRefWarpMgr->AddInfo(pWI);
}

BOOL CDlgEditWarp::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ListWarp.ResetContent();
	SetEditor();
	//ResetAll();

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEditWarp::ResetAll()
{
	m_ListWarp.ResetContent();

	std::list<WARPINFO*>::iterator it, ite;

	ite = m_pRefWarpMgr->m_ListWarpInfo.end();
	for (it = m_pRefWarpMgr->m_ListWarpInfo.begin(); it != ite; it++)
	{
		WARPINFO* pWI = (*it);
		int idx       = m_ListWarp.AddString(pWI->szName);
		m_ListWarp.SetItemDataPtr(idx, pWI);
	}
}

void CDlgEditWarp::SetEditor()
{
	m_fRadius = m_tmpInfo.fRadius;
	m_fPosX   = m_tmpInfo.fX;
	m_fPosY   = m_tmpInfo.fY;
	m_fPosZ   = m_tmpInfo.fZ;

	m_strName.Format(m_tmpInfo.szName);
	m_iMoney  = m_tmpInfo.dwMoney;
	m_sID     = m_tmpInfo.sID;
	m_sZoneID = m_tmpInfo.sZoneID;
	m_AgreeState.Format(m_tmpInfo.szAgree);

	UpdateData(FALSE);
}

void CDlgEditWarp::GetEditor()
{
	UpdateData(TRUE);

	m_tmpInfo.fRadius = m_fRadius;
	m_tmpInfo.fX      = m_fPosX;
	m_tmpInfo.fY      = m_fPosY;
	m_tmpInfo.fZ      = m_fPosZ;

	sprintf(m_tmpInfo.szName, (LPCTSTR) m_strName);
	m_tmpInfo.dwMoney = m_iMoney;
	m_tmpInfo.sID     = m_sID;
	m_tmpInfo.sZoneID = m_sZoneID;
	sprintf(m_tmpInfo.szAgree, (LPCTSTR) m_AgreeState);
}

void CDlgEditWarp::OnClose()
{
	CDialog::OnClose();
	m_pRefWarpMgr->SetActive(false);
}

void CDlgEditWarp::OnOK()
{
	CDialog::OnOK();
	m_pRefWarpMgr->SetActive(false);
}

void CDlgEditWarp::OnBtnExport()
{
	CFileDialog dlg(FALSE, "wap", "Noname", OFN_EXPLORER | OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
		"Warp Info파일(*.wap)|*.wap||");

	if (dlg.DoModal() == IDOK)
	{
		CString str = dlg.GetPathName();
		m_pRefWarpMgr->SaveToFile((LPCTSTR) str);
	}
}

void CDlgEditWarp::OnBtnImport()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wap", nullptr, dwFlags, "Warp Info파일(*.wap)|*.wap||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetPathName();
	m_pRefWarpMgr->LoadFromFile((LPCTSTR) str);
}
