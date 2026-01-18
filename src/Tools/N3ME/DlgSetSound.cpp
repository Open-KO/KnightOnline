// DlgSetSound.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSetSound.h"
#include "SoundMgr.h"
#include "SoundCell.h"
#include "MapMng.h"
#include "DlgAddSoundGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetSound dialog

CDlgSetSound::CDlgSetSound(CWnd* pParent /*=nullptr*/) : CDialog(CDlgSetSound::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSetSound)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgSetSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetSound)
	DDX_Control(pDX, IDC_LIST_SOUNDGROUP, m_ListSoundGroup);
	DDX_Control(pDX, IDC_LIST_SOUNDINFO, m_ListSoundInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSetSound, CDialog)
//{{AFX_MSG_MAP(CDlgSetSound)
ON_BN_CLICKED(IDC_BTN_ADD_SOUNDGROUP, OnBtnAddSoundgroup)
ON_BN_CLICKED(IDC_BTN_DELETE_SOUNDGROUP, OnBtnDeleteSoundgroup)
ON_BN_CLICKED(IDC_BTN_INPUT_INFO, OnBtnInputInfo)
ON_WM_DESTROY()
ON_LBN_SELCHANGE(IDC_LIST_SOUNDGROUP, OnSelchangeListSoundgroup)
ON_LBN_SELCHANGE(IDC_LIST_SOUNDINFO, OnSelchangeListSoundinfo)
ON_BN_CLICKED(IDC_BTN_INPUT_INFO_DEL, OnBtnInputInfoDel)
ON_BN_CLICKED(IDC_BTN_INPUT_INFO_EDIT, OnBtnInputInfoEdit)
ON_BN_CLICKED(IDC_BTN_MODIFY_SOUNDGROUP, OnBtnModifySoundgroup)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetSound message handlers

LPSOUNDINFO CDlgSetSound::GetSoundGroup(DWORD dwID)
{
	int cnt = m_ListSoundGroup.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		LPSOUNDINFO pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
		if (pSoundInfo && pSoundInfo->dwID == dwID)
			return pSoundInfo;
	}
	return nullptr;
}

void CDlgSetSound::OnBtnAddSoundgroup()
{
	CDlgAddSoundGroup dlg;
	if (dlg.DoModal() == IDOK)
	{
		LPSOUNDINFO pSoundGroup = new SOUNDINFO;
		(*pSoundGroup)          = dlg.m_SndInfo;
		pSoundGroup->dwID       = MakeIdx();

		AddSoundGroup(pSoundGroup->szName, pSoundGroup);
	}
}

void CDlgSetSound::OnBtnModifySoundgroup()
{
	int idx = m_ListSoundGroup.GetCurSel();
	if (idx < 0)
		return;

	LPSOUNDINFO pSoundGroup    = new SOUNDINFO;
	LPSOUNDINFO pTmpSoundGroup = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(idx);
	(*pSoundGroup)             = (*pTmpSoundGroup);
	delete pTmpSoundGroup;

	DWORD dwID = pSoundGroup->dwID;

	CDlgAddSoundGroup dlg;
	dlg.SetSndInfo(pSoundGroup);
	if (dlg.DoModal() == IDOK)
	{
		(*pSoundGroup)    = dlg.m_SndInfo;
		pSoundGroup->dwID = dwID;

		m_ListSoundGroup.DeleteString(idx);
		m_ListSoundGroup.InsertString(idx, pSoundGroup->szName);
		m_ListSoundGroup.SetItemDataPtr(idx, pSoundGroup);
	}
}

int CDlgSetSound::MakeIdx()
{
	if (m_iIdx_Min > 0)
		return 0;
	if (m_iIdx_Min > m_iIdx_Max)
		return 0;

	LPSOUNDINFO pSoundInfo = nullptr;

	for (int i = m_iIdx_Min; i <= m_iIdx_Max; i++)
	{
		int cnt = m_ListSoundGroup.GetCount();
		int j;
		for (j = 0; j < cnt; j++)
		{
			pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(j);
			if (pSoundInfo->dwID == i)
				break;
		}
		if (j == cnt)
		{
			return i;
		}
	}

	return (m_iIdx_Max + 1);
}

void CDlgSetSound::OnBtnDeleteSoundgroup()
{
	int idx = m_ListSoundGroup.GetCurSel();
	if (idx < 0)
		return;

	LPSOUNDINFO pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(idx);
	int SndInfoID          = pSoundInfo->dwID;

	delete pSoundInfo;
	m_ListSoundGroup.DeleteString(idx);

	if (SndInfoID == m_iIdx_Min)
	{
		m_iIdx_Min = m_iIdx_Max;

		int cnt    = m_ListSoundGroup.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
			if (m_iIdx_Min > static_cast<int>(pSoundInfo->dwID))
				m_iIdx_Min = static_cast<int>(pSoundInfo->dwID);
		}
	}

	if (SndInfoID == m_iIdx_Max)
	{
		m_iIdx_Max = m_iIdx_Min;

		int cnt    = m_ListSoundGroup.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
			if (m_iIdx_Max < static_cast<int>(pSoundInfo->dwID))
				m_iIdx_Max = static_cast<int>(pSoundInfo->dwID);
		}
	}
	OnSelchangeListSoundgroup();
}

void CDlgSetSound::AddSoundGroup(char* szTitle, LPSOUNDINFO pSoundInfo)
{
	int idx = m_ListSoundGroup.AddString(szTitle);
	m_ListSoundGroup.SetItemDataPtr(idx, pSoundInfo);

	int iSndID = static_cast<int>(pSoundInfo->dwID);

	if (iSndID < m_iIdx_Min)
		m_iIdx_Min = iSndID;

	if (iSndID > m_iIdx_Max)
		m_iIdx_Max = iSndID;
}

void CDlgSetSound::OnBtnInputInfo()
{
	int idx = m_ListSoundGroup.GetCurSel();
	if (idx < 0)
		return;

	if (m_pRefSoundMgr->m_pCurrSound->m_Rect.left == 0
		&& m_pRefSoundMgr->m_pCurrSound->m_Rect.right == 0
		&& m_pRefSoundMgr->m_pCurrSound->m_Rect.top == 0
		&& m_pRefSoundMgr->m_pCurrSound->m_Rect.bottom == 0)
		return;

	LPSOUNDINFO pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(idx);
	if (!pSoundInfo)
		return;

	m_pRefSoundMgr->m_pCurrSound->m_dwSoundGroupID = pSoundInfo->dwID;

	AddSoundInfo(m_pRefSoundMgr->m_pCurrSound);

	m_pRefSoundMgr->UpdateSound();
	m_pRefSoundMgr->m_pRefMapMng->Invalidate();
}

void CDlgSetSound::AddSoundInfo(CSoundCell* pSound)
{
	int cnt = m_ListSoundGroup.GetCount();
	int idx = -1;
	for (int i = 0; i < cnt; i++)
	{
		LPSOUNDINFO pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
		if (pSoundInfo && pSoundInfo->dwID == pSound->m_dwSoundGroupID)
		{
			idx = i;
			break;
		}
	}

	if (idx >= 0)
	{
		char szBuff1[256];
		m_ListSoundGroup.GetText(idx, szBuff1);

		char szBuff2[256];
		sprintf(szBuff2, "(%d,%d)->(%d,%d) : %s", pSound->m_Rect.left, pSound->m_Rect.top,
			pSound->m_Rect.right, pSound->m_Rect.bottom, szBuff1);

		idx = m_ListSoundInfo.AddString(szBuff2);
		m_ListSoundInfo.SetItemDataPtr(idx, pSound);
	}
}

BOOL CDlgSetSound::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_BGM_NAME1, "none");
	SetDlgItemText(IDC_BGM_NAME2, "none");
	SetDlgItemText(IDC_BGM_NAME3, "none");
	SetDlgItemText(IDC_BGM_NAME4, "none");
	SetDlgItemText(IDC_BGM_REGENTIME1, "0");
	SetDlgItemText(IDC_BGM_REGENTIME2, "0");
	SetDlgItemText(IDC_BGM_REGENTIME3, "0");
	SetDlgItemText(IDC_BGM_REGENTIME4, "0");

	SetDlgItemText(IDC_BGE_NAME1, "none");
	SetDlgItemText(IDC_BGE_NAME2, "none");
	SetDlgItemText(IDC_BGE_NAME3, "none");
	SetDlgItemText(IDC_BGE_NAME4, "none");
	SetDlgItemText(IDC_BGE_REGENTIME1, "0");
	SetDlgItemText(IDC_BGE_REGENTIME2, "0");
	SetDlgItemText(IDC_BGE_REGENTIME3, "0");
	SetDlgItemText(IDC_BGE_REGENTIME4, "0");

	m_pRefSoundMgr = nullptr;
	m_pSelSound    = nullptr;

	m_ListSoundGroup.ResetContent();
	m_ListSoundInfo.ResetContent();

	m_iIdx_Min = 500;
	m_iIdx_Max = 0;

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetSound::OnDestroy()
{
	int cnt = m_ListSoundGroup.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		LPSOUNDINFO pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
		delete pSoundInfo;
	}

	CDialog::OnDestroy();
}

void CDlgSetSound::OnSelchangeListSoundgroup()
{
	int idx = m_ListSoundGroup.GetCurSel();
	if (idx < 0)
	{
		SetDlgItemText(IDC_BGM_NAME1, "none");
		SetDlgItemText(IDC_BGM_NAME2, "none");
		SetDlgItemText(IDC_BGM_NAME3, "none");
		SetDlgItemText(IDC_BGM_NAME4, "none");

		SetDlgItemText(IDC_BGM_REGENTIME1, "0");
		SetDlgItemText(IDC_BGM_REGENTIME2, "0");
		SetDlgItemText(IDC_BGM_REGENTIME3, "0");
		SetDlgItemText(IDC_BGM_REGENTIME4, "0");

		SetDlgItemText(IDC_BGE_NAME1, "none");
		SetDlgItemText(IDC_BGE_NAME2, "none");
		SetDlgItemText(IDC_BGE_NAME3, "none");
		SetDlgItemText(IDC_BGE_NAME4, "none");

		SetDlgItemText(IDC_BGE_REGENTIME1, "0");
		SetDlgItemText(IDC_BGE_REGENTIME2, "0");
		SetDlgItemText(IDC_BGE_REGENTIME3, "0");
		SetDlgItemText(IDC_BGE_REGENTIME4, "0");
		return;
	}

	LPSOUNDINFO pSoundInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(idx);

	SetDlgItemText(IDC_BGM_NAME1, pSoundInfo->szBGM[0]);
	SetDlgItemText(IDC_BGM_NAME2, pSoundInfo->szBGM[1]);
	SetDlgItemText(IDC_BGM_NAME3, pSoundInfo->szBGM[2]);
	SetDlgItemText(IDC_BGM_NAME4, pSoundInfo->szBGM[3]);

	char szBuff[10];
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGMRegenTime[0]);
	SetDlgItemText(IDC_BGM_REGENTIME1, szBuff);
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGMRegenTime[1]);
	SetDlgItemText(IDC_BGM_REGENTIME2, szBuff);
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGMRegenTime[2]);
	SetDlgItemText(IDC_BGM_REGENTIME3, szBuff);
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGMRegenTime[3]);
	SetDlgItemText(IDC_BGM_REGENTIME4, szBuff);

	SetDlgItemText(IDC_BGE_NAME1, pSoundInfo->szBGE[0]);
	SetDlgItemText(IDC_BGE_NAME2, pSoundInfo->szBGE[1]);
	SetDlgItemText(IDC_BGE_NAME3, pSoundInfo->szBGE[2]);
	SetDlgItemText(IDC_BGE_NAME4, pSoundInfo->szBGE[3]);

	sprintf(szBuff, "%4.2f", pSoundInfo->fBGERegenTime[0]);
	SetDlgItemText(IDC_BGE_REGENTIME1, szBuff);
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGERegenTime[0]);
	SetDlgItemText(IDC_BGE_REGENTIME2, szBuff);
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGERegenTime[0]);
	SetDlgItemText(IDC_BGE_REGENTIME3, szBuff);
	sprintf(szBuff, "%4.2f", pSoundInfo->fBGERegenTime[0]);
	SetDlgItemText(IDC_BGE_REGENTIME4, szBuff);
}

void CDlgSetSound::OnSelchangeListSoundinfo()
{
	int idx = m_ListSoundInfo.GetCurSel();
	if (idx < 0)
	{
		m_pSelSound = nullptr;
		m_pRefSoundMgr->m_pRefMapMng->Invalidate();
		return;
	}

	m_pSelSound = (CSoundCell*) m_ListSoundInfo.GetItemDataPtr(idx);
	m_pRefSoundMgr->m_pRefMapMng->Invalidate();
}

void CDlgSetSound::OnBtnInputInfoDel()
{
	int idx = m_ListSoundInfo.GetCurSel();
	if (idx >= 0)
	{
		m_pSelSound = (CSoundCell*) m_ListSoundInfo.GetItemDataPtr(idx);
		m_pRefSoundMgr->DelSound(m_pSelSound);
		m_pSelSound = nullptr;
		m_ListSoundInfo.DeleteString(idx);
	}

	OnSelchangeListSoundinfo();
}

void CDlgSetSound::OnBtnInputInfoEdit()
{
	int idx = m_ListSoundInfo.GetCurSel();
	if (idx >= 0)
	{
		m_pSelSound = (CSoundCell*) m_ListSoundInfo.GetItemDataPtr(idx);
		m_pRefSoundMgr->SetCurrSound(m_pSelSound);
		m_pSelSound = nullptr;
		m_ListSoundInfo.DeleteString(idx);
	}

	OnSelchangeListSoundinfo();
}

bool CDlgSetSound::LoadSoundGroup(File& file)
{
	int cnt = 0;
	file.Read(&cnt, sizeof(int));

	for (int i = 0; i < cnt; i++)
	{
		LPSOUNDINFO pSndInfo = new SOUNDINFO;
		file.Read(pSndInfo, sizeof(SOUNDINFO));
		AddSoundGroup(pSndInfo->szName, pSndInfo);
	}
	return true;
}

bool CDlgSetSound::SaveSoundGroup(File& file)
{
	int cnt = m_ListSoundGroup.GetCount();
	file.Write(&cnt, sizeof(int));

	LPSOUNDINFO pSndInfo;
	for (int i = 0; i < cnt; i++)
	{
		pSndInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
		file.Write(pSndInfo, sizeof(SOUNDINFO));
	}
	return true;
}

void CDlgSetSound::Clear()
{
	int cnt = 0;
	cnt     = m_ListSoundGroup.GetCount();
	LPSOUNDINFO pSndInfo;
	for (int i = 0; i < cnt; i++)
	{
		pSndInfo = (LPSOUNDINFO) m_ListSoundGroup.GetItemDataPtr(i);
		delete pSndInfo;
	}
	m_ListSoundGroup.ResetContent();

	m_ListSoundInfo.ResetContent();
}

