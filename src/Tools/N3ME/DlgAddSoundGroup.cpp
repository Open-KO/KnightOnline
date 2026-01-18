// DlgAddSoundGroup.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgAddSoundGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAddSoundGroup dialog

CDlgAddSoundGroup::CDlgAddSoundGroup(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgAddSoundGroup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAddSoundGroup)
	m_fBGE_Regen_1 = 0.0f;
	m_fBGE_Regen_2 = 0.0f;
	m_fBGE_Regen_3 = 0.0f;
	m_fBGE_Regen_4 = 0.0f;
	m_fBGM_Regen_1 = 0.0f;
	m_fBGM_Regen_2 = 0.0f;
	m_fBGM_Regen_3 = 0.0f;
	m_fBGM_Regen_4 = 0.0f;
	//}}AFX_DATA_INIT

	m_SndInfo.dwID = 0;
	memset(m_SndInfo.szName, 0, 256);
	for (int i = 0; i < 4; i++)
	{
		m_SndInfo.fBGERegenTime[i] = 0.0f;
		m_SndInfo.fBGMRegenTime[i] = 0.0f;
		sprintf(m_SndInfo.szBGE[i], "none");
		sprintf(m_SndInfo.szBGM[i], "none");
	}
}

void CDlgAddSoundGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAddSoundGroup)
	DDX_Text(pDX, IDC_EDT_BGE_REGEN1, m_fBGE_Regen_1);
	DDX_Text(pDX, IDC_EDT_BGE_REGEN2, m_fBGE_Regen_2);
	DDX_Text(pDX, IDC_EDT_BGE_REGEN3, m_fBGE_Regen_3);
	DDX_Text(pDX, IDC_EDT_BGE_REGEN4, m_fBGE_Regen_4);
	DDX_Text(pDX, IDC_EDT_BGM_REGEN1, m_fBGM_Regen_1);
	DDX_Text(pDX, IDC_EDT_BGM_REGEN2, m_fBGM_Regen_2);
	DDX_Text(pDX, IDC_EDT_BGM_REGEN3, m_fBGM_Regen_3);
	DDX_Text(pDX, IDC_EDT_BGM_REGEN4, m_fBGM_Regen_4);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgAddSoundGroup, CDialog)
//{{AFX_MSG_MAP(CDlgAddSoundGroup)
ON_BN_CLICKED(IDC_BTN_BGE_NAME1, OnBtnBgeName1)
ON_BN_CLICKED(IDC_BTN_BGE_NAME2, OnBtnBgeName2)
ON_BN_CLICKED(IDC_BTN_BGE_NAME3, OnBtnBgeName3)
ON_BN_CLICKED(IDC_BTN_BGE_NAME4, OnBtnBgeName4)
ON_BN_CLICKED(IDC_BTN_BGM_NAME1, OnBtnBgmName1)
ON_BN_CLICKED(IDC_BTN_BGM_NAME2, OnBtnBgmName2)
ON_BN_CLICKED(IDC_BTN_BGM_NAME3, OnBtnBgmName3)
ON_BN_CLICKED(IDC_BTN_BGM_NAME4, OnBtnBgmName4)
ON_BN_CLICKED(IDC_BTN_DEL_BGE_NAME1, OnBtnDelBgeName1)
ON_BN_CLICKED(IDC_BTN_DEL_BGE_NAME2, OnBtnDelBgeName2)
ON_BN_CLICKED(IDC_BTN_DEL_BGE_NAME3, OnBtnDelBgeName3)
ON_BN_CLICKED(IDC_BTN_DEL_BGE_NAME4, OnBtnDelBgeName4)
ON_BN_CLICKED(IDC_BTN_DEL_BGM_NAME1, OnBtnDelBgmName1)
ON_BN_CLICKED(IDC_BTN_DEL_BGM_NAME2, OnBtnDelBgmName2)
ON_BN_CLICKED(IDC_BTN_DEL_BGM_NAME3, OnBtnDelBgmName3)
ON_BN_CLICKED(IDC_BTN_DEL_BGM_NAME4, OnBtnDelBgmName4)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAddSoundGroup message handlers

void CDlgAddSoundGroup::SetSndInfo(LPSOUNDINFO pSndInfo)
{
	memcpy(&m_SndInfo, pSndInfo, sizeof(SOUNDINFO));
}

BOOL CDlgAddSoundGroup::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_BGE_NAME1, m_SndInfo.szBGE[0]);
	SetDlgItemText(IDC_BGE_NAME2, m_SndInfo.szBGE[1]);
	SetDlgItemText(IDC_BGE_NAME3, m_SndInfo.szBGE[2]);
	SetDlgItemText(IDC_BGE_NAME4, m_SndInfo.szBGE[3]);

	m_fBGE_Regen_1 = m_SndInfo.fBGERegenTime[0];
	m_fBGE_Regen_2 = m_SndInfo.fBGERegenTime[1];
	m_fBGE_Regen_3 = m_SndInfo.fBGERegenTime[2];
	m_fBGE_Regen_4 = m_SndInfo.fBGERegenTime[3];

	SetDlgItemText(IDC_BGM_NAME1, m_SndInfo.szBGM[0]);
	SetDlgItemText(IDC_BGM_NAME2, m_SndInfo.szBGM[1]);
	SetDlgItemText(IDC_BGM_NAME3, m_SndInfo.szBGM[2]);
	SetDlgItemText(IDC_BGM_NAME4, m_SndInfo.szBGM[3]);

	m_fBGM_Regen_1 = m_SndInfo.fBGMRegenTime[0];
	m_fBGM_Regen_2 = m_SndInfo.fBGMRegenTime[1];
	m_fBGM_Regen_3 = m_SndInfo.fBGMRegenTime[2];
	m_fBGM_Regen_4 = m_SndInfo.fBGMRegenTime[3];

	SetDlgItemText(IDC_EDT_GROUPNAME, m_SndInfo.szName);

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAddSoundGroup::OnBtnBgeName1()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGE_NAME1, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgeName2()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGE_NAME2, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgeName3()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGE_NAME3, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgeName4()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGE_NAME4, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgmName1()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGM_NAME1, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgmName2()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGM_NAME2, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgmName3()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGM_NAME3, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnBtnBgmName4()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave파일(*.wav)|*.wav||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str = dlg.GetFileName();

	SetDlgItemText(IDC_BGM_NAME4, (LPCTSTR) str);
}

void CDlgAddSoundGroup::OnOK()
{
	UpdateData(TRUE);

	GetDlgItemText(IDC_EDT_GROUPNAME, m_SndInfo.szName, 256);
	if (m_SndInfo.szName[0] == 0)
	{
		AfxMessageBox("묶음이름을 입력하셔야 해요.\n앞머리에 띄어쓰기 하면 안되는거 알죠?^^");
		return;
	}

	GetDlgItemText(IDC_BGM_NAME1, m_SndInfo.szBGM[0], 256);
	GetDlgItemText(IDC_BGM_NAME2, m_SndInfo.szBGM[1], 256);
	GetDlgItemText(IDC_BGM_NAME3, m_SndInfo.szBGM[2], 256);
	GetDlgItemText(IDC_BGM_NAME4, m_SndInfo.szBGM[3], 256);

	GetDlgItemText(IDC_BGE_NAME1, m_SndInfo.szBGE[0], 256);
	GetDlgItemText(IDC_BGE_NAME2, m_SndInfo.szBGE[1], 256);
	GetDlgItemText(IDC_BGE_NAME3, m_SndInfo.szBGE[2], 256);
	GetDlgItemText(IDC_BGE_NAME4, m_SndInfo.szBGE[3], 256);

	m_SndInfo.fBGMRegenTime[0] = m_fBGM_Regen_1;
	m_SndInfo.fBGMRegenTime[1] = m_fBGM_Regen_2;
	m_SndInfo.fBGMRegenTime[2] = m_fBGM_Regen_3;
	m_SndInfo.fBGMRegenTime[3] = m_fBGM_Regen_4;

	m_SndInfo.fBGERegenTime[0] = m_fBGE_Regen_1;
	m_SndInfo.fBGERegenTime[1] = m_fBGE_Regen_2;
	m_SndInfo.fBGERegenTime[2] = m_fBGE_Regen_3;
	m_SndInfo.fBGERegenTime[3] = m_fBGE_Regen_4;

	CDialog::OnOK();
}

void CDlgAddSoundGroup::OnBtnDelBgeName1()
{
	SetDlgItemText(IDC_BGE_NAME1, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgeName2()
{
	SetDlgItemText(IDC_BGE_NAME2, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgeName3()
{
	SetDlgItemText(IDC_BGE_NAME3, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgeName4()
{
	SetDlgItemText(IDC_BGE_NAME4, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgmName1()
{
	SetDlgItemText(IDC_BGM_NAME1, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgmName2()
{
	SetDlgItemText(IDC_BGM_NAME2, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgmName3()
{
	SetDlgItemText(IDC_BGM_NAME3, "none");
}

void CDlgAddSoundGroup::OnBtnDelBgmName4()
{
	SetDlgItemText(IDC_BGM_NAME4, "none");
}
