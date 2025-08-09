// DlgEditScript.cpp : implementation file
//

#include "stdafx.h"
#include "N3FXE.h"
#include "MainFrm.h"
#include "DlgEditScript.h"
#include "DlgNewFileName.h"
#include <N3Base/N3FXBundle.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditScript dialog


CDlgEditScript::CDlgEditScript(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgEditScript::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditScript)
	m_fVelocity = 0.0f;
	m_bDependScale = FALSE;
	m_strPathName = _T("");
	m_fStartTime8 = 0.0f;
	m_fStartTime9 = 0.0f;
	m_fStartTime10 = 0.0f;
	m_fStartTime11 = 0.0f;
	m_fStartTime12 = 0.0f;
	m_fStartTime13 = 0.0f;
	m_fStartTime14 = 0.0f;
	m_fStartTime15 = 0.0f;
	m_bStaticPos = FALSE;
	//}}AFX_DATA_INIT

	m_pPartName[0] = &m_CBPartName0;
	m_pPartName[1] = &m_CBPartName1;
	m_pPartName[2] = &m_CBPartName2;
	m_pPartName[3] = &m_CBPartName3;
	m_pPartName[4] = &m_CBPartName4;
	m_pPartName[5] = &m_CBPartName5;
	m_pPartName[6] = &m_CBPartName6;
	m_pPartName[7] = &m_CBPartName7;
	m_pPartName[8] = &m_CBPartName8;
	m_pPartName[9] = &m_CBPartName9;
	m_pPartName[10] = &m_CBPartName10;
	m_pPartName[11] = &m_CBPartName11;
	m_pPartName[12] = &m_CBPartName12;
	m_pPartName[13] = &m_CBPartName13;
	m_pPartName[14] = &m_CBPartName14;
	m_pPartName[15] = &m_CBPartName15;

	m_pPartStartTime[0] = &m_fStartTime0;
	m_pPartStartTime[1] = &m_fStartTime1;
	m_pPartStartTime[2] = &m_fStartTime2;
	m_pPartStartTime[3] = &m_fStartTime3;
	m_pPartStartTime[4] = &m_fStartTime4;
	m_pPartStartTime[5] = &m_fStartTime5;
	m_pPartStartTime[6] = &m_fStartTime6;
	m_pPartStartTime[7] = &m_fStartTime7;
	m_pPartStartTime[8] = &m_fStartTime8;
	m_pPartStartTime[9] = &m_fStartTime9;
	m_pPartStartTime[10] = &m_fStartTime10;
	m_pPartStartTime[11] = &m_fStartTime11;
	m_pPartStartTime[12] = &m_fStartTime12;
	m_pPartStartTime[13] = &m_fStartTime13;
	m_pPartStartTime[14] = &m_fStartTime14;
	m_pPartStartTime[15] = &m_fStartTime15;


	m_pRefFrm = NULL;
	m_pFXBundle = NULL;
}


void CDlgEditScript::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditScript)
	DDX_Control(pDX, IDC_COMBO_PART_16, m_CBPartName15);
	DDX_Control(pDX, IDC_COMBO_PART_15, m_CBPartName14);
	DDX_Control(pDX, IDC_COMBO_PART_14, m_CBPartName13);
	DDX_Control(pDX, IDC_COMBO_PART_13, m_CBPartName12);
	DDX_Control(pDX, IDC_COMBO_PART_12, m_CBPartName11);
	DDX_Control(pDX, IDC_COMBO_PART_11, m_CBPartName10);
	DDX_Control(pDX, IDC_COMBO_PART_10, m_CBPartName9);
	DDX_Control(pDX, IDC_COMBO_PART_9, m_CBPartName8);
	DDX_Text(pDX, IDC_EDIT_START_TIME_1, m_fStartTime0);
	DDV_MinMaxFloat(pDX, m_fStartTime0, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_2, m_fStartTime1);
	DDV_MinMaxFloat(pDX, m_fStartTime1, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_3, m_fStartTime2);
	DDV_MinMaxFloat(pDX, m_fStartTime2, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_4, m_fStartTime3);
	DDV_MinMaxFloat(pDX, m_fStartTime3, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_5, m_fStartTime4);
	DDV_MinMaxFloat(pDX, m_fStartTime4, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_6, m_fStartTime5);
	DDV_MinMaxFloat(pDX, m_fStartTime5, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_7, m_fStartTime6);
	DDV_MinMaxFloat(pDX, m_fStartTime6, -1.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_8, m_fStartTime7);
	DDV_MinMaxFloat(pDX, m_fStartTime7, -1.f, 100.f);
	DDX_Control(pDX, IDC_COMBO_PART_1, m_CBPartName0);
	DDX_Control(pDX, IDC_COMBO_PART_2, m_CBPartName1);
	DDX_Control(pDX, IDC_COMBO_PART_3, m_CBPartName2);
	DDX_Control(pDX, IDC_COMBO_PART_4, m_CBPartName3);
	DDX_Control(pDX, IDC_COMBO_PART_5, m_CBPartName4);
	DDX_Control(pDX, IDC_COMBO_PART_6, m_CBPartName5);
	DDX_Control(pDX, IDC_COMBO_PART_7, m_CBPartName6);
	DDX_Control(pDX, IDC_COMBO_PART_8, m_CBPartName7);
	DDX_Text(pDX, IDC_EDIT_VELOCITY, m_fVelocity);
	DDV_MinMaxFloat(pDX, m_fVelocity, -100.f, 1000.f);
	DDX_Check(pDX, IDC_CHECK_DEPEND_SCALE, m_bDependScale);
	DDX_Text(pDX, IDC_SOURCE_NAME, m_strPathName);
	DDV_MaxChars(pDX, m_strPathName, 256);
	DDX_Text(pDX, IDC_EDIT_START_TIME_9, m_fStartTime8);
	DDV_MinMaxFloat(pDX, m_fStartTime8, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_10, m_fStartTime9);
	DDV_MinMaxFloat(pDX, m_fStartTime9, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_11, m_fStartTime10);
	DDV_MinMaxFloat(pDX, m_fStartTime10, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_12, m_fStartTime11);
	DDV_MinMaxFloat(pDX, m_fStartTime11, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_13, m_fStartTime12);
	DDV_MinMaxFloat(pDX, m_fStartTime12, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_14, m_fStartTime13);
	DDV_MinMaxFloat(pDX, m_fStartTime13, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_15, m_fStartTime14);
	DDV_MinMaxFloat(pDX, m_fStartTime14, 0.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_START_TIME_16, m_fStartTime15);
	DDV_MinMaxFloat(pDX, m_fStartTime15, 0.f, 10000.f);
	DDX_Check(pDX, IDC_CHECK_STATIC_POS, m_bStaticPos);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditScript, CDialog)
	//{{AFX_MSG_MAP(CDlgEditScript)
	ON_BN_CLICKED(IDC_BTN_PLAY, OnBtnPlay)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_PART_1, OnSelchangeComboPart1)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_2, OnSelchangeComboPart2)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_3, OnSelchangeComboPart3)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_4, OnSelchangeComboPart4)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_5, OnSelchangeComboPart5)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_6, OnSelchangeComboPart6)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_7, OnSelchangeComboPart7)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_8, OnSelchangeComboPart8)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_1, OnChangeEditStartTime1)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_2, OnChangeEditStartTime2)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_3, OnChangeEditStartTime3)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_4, OnChangeEditStartTime4)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_5, OnChangeEditStartTime5)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_6, OnChangeEditStartTime6)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_7, OnChangeEditStartTime7)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_8, OnChangeEditStartTime8)
	ON_BN_CLICKED(IDC_BTN_SAVE_AS, OnBtnSaveAs)
	ON_BN_CLICKED(IDC_BTN_REFRESH, OnBtnRefresh)
	ON_EN_CHANGE(IDC_EDIT_VELOCITY, OnChangeEditVelocity)
	ON_BN_CLICKED(IDC_CHECK_DEPEND_SCALE, OnCheckDependScale)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT_START_TIME_9, OnChangeEditStartTime9)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_10, OnChangeEditStartTime10)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_11, OnChangeEditStartTime11)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_12, OnChangeEditStartTime12)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_13, OnChangeEditStartTime13)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_14, OnChangeEditStartTime14)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_15, OnChangeEditStartTime15)
	ON_EN_CHANGE(IDC_EDIT_START_TIME_16, OnChangeEditStartTime16)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_9, OnSelchangeComboPart9)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_10, OnSelchangeComboPart10)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_11, OnSelchangeComboPart11)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_12, OnSelchangeComboPart12)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_13, OnSelchangeComboPart13)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_14, OnSelchangeComboPart14)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_15, OnSelchangeComboPart15)
	ON_CBN_SELCHANGE(IDC_COMBO_PART_16, OnSelchangeComboPart16)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_1, OnDropdownComboPart1)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_2, OnDropdownComboPart2)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_3, OnDropdownComboPart3)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_4, OnDropdownComboPart4)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_5, OnDropdownComboPart5)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_6, OnDropdownComboPart6)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_7, OnDropdownComboPart7)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_8, OnDropdownComboPart8)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_9, OnDropdownComboPart9)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_10, OnDropdownComboPart10)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_11, OnDropdownComboPart11)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_12, OnDropdownComboPart12)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_13, OnDropdownComboPart13)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_14, OnDropdownComboPart14)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_15, OnDropdownComboPart15)
	ON_CBN_DROPDOWN(IDC_COMBO_PART_16, OnDropdownComboPart16)
	ON_BN_CLICKED(IDC_CHECK_STATIC_POS, OnCheckStaticPos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditScript message handlers

void CDlgEditScript::OnBtnPlay() 
{
	if((!m_pFXBundle) || (!m_pRefFrm)) return;
	m_pRefFrm->m_pCurrFX = m_pFXBundle;

	m_pFXBundle->Stop();
	m_pFXBundle->Init();
	m_pFXBundle->Trigger();
}

void CDlgEditScript::OnBtnSave() 
{
	if(!m_pFXBundle) return;

	if(m_strPathName == _T(""))
	{
		OnBtnSaveAs();
		return;
	}

	FILE* file = fopen(m_strPathName, "w");

	fprintf(file,"<N3FXBUNDLE>\n");
	//fprintf(file,"<NAME> %s\n", m_pFXBundle->m_strName.c_str());

	for(int i=0;i<MAX_FX_PART_TOOL;i++)
	{
		int SelIdx = m_pPartName[i]->GetCurSel();
		if(SelIdx<=0) continue;

		char szBuff[_MAX_PATH];

		m_pPartName[i]->GetLBText(SelIdx, szBuff);	

		fprintf(file,"<PART> fx/%s %3.2f\n", szBuff, (*m_pPartStartTime[i]));
	}

	if(m_bDependScale) fprintf(file, "<Depend_Scale> true\n");
	else fprintf(file, "<Depend_Scale> false\n");

	if(m_bStaticPos) fprintf(file, "<Static_Pos> true\n");
	else fprintf(file, "<Static_Pos> false\n");

	fprintf(file,"<Velocity> %5.4f\n", m_fVelocity);
	
	fclose(file);

	LoadBundle(m_strPathName);
	SaveGameDataPartnBundle();	
}

void CDlgEditScript::SaveGameDataPartnBundle()
{
	if(!m_pFXBundle) return;
	m_pFXBundle->SaveToFile();
}

void CDlgEditScript::OnBtnStop() 
{ 
	if((!m_pFXBundle) || (!m_pRefFrm)) return;

	m_pFXBundle->Stop();
}

//strPathName은 FullPath...
bool CDlgEditScript::LoadBundle(CString &strPathName)
{
	if(m_pFXBundle)
	{
		delete m_pFXBundle;
		m_pFXBundle = NULL;
	}

	///////////////////////////////////////////////////
	// Dialog초기화..
	for(int i=0;i<MAX_FX_PART_TOOL;i++)
	{
		(*m_pPartStartTime[i]) = 0.0f;		
		m_pPartName[i]->SetCurSel(0);
	}
	m_fVelocity = 0.0f;
	m_bDependScale = FALSE;
	//
	///////////////////////////////////////////////////

	m_strPathName = strPathName;

	m_pFXBundle = new CN3FXBundle;
	m_pRefFrm->m_pCurrFX = m_pFXBundle;
	if(m_pFXBundle->DecodeScriptFile((LPCTSTR)strPathName))	//번들 스크립트 읽기 성공했으면...
	{
		//
		//	set part editor 
		//
		for(int i=0;i<MAX_FX_PART_TOOL;i++)
		{
			CN3FXPartBase* pPart = m_pFXBundle->GetPart(i);
			if(pPart)
			{
				std::string szPartFullPath;
				szPartFullPath = pPart->PathGet() + pPart->FileName();

				char szScriptFullPath[_MAX_PATH];
				char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
				_splitpath(szPartFullPath.c_str(), szDrive, szDir, szFName, szExt);
				_makepath(szScriptFullPath, szDrive, szDir, szFName, "n3fxpart");

				//콤보박스 셋팅..
				char szComboName[_MAX_PATH];
				_splitpath(szScriptFullPath, szDrive, szDir, szFName, szExt);
				_makepath(szComboName, NULL, NULL, szFName, szExt);

				int ComboCount = m_pPartName[i]->GetCount();
				for(int j=0;j<ComboCount;j++)
				{
					char szComboNameTarget[_MAX_PATH];
					m_pPartName[i]->GetLBText(j, szComboNameTarget);
					if(lstrcmpi(szComboNameTarget, szComboName)==0)
					{
						m_pPartName[i]->SetCurSel(j);
					}
				}
				//옆의 시간 에디트박스도 셋팅..^^
				(*m_pPartStartTime[i]) = m_pFXBundle->GetPartSTime(i);				
			}
		}
		m_fVelocity = m_pFXBundle->m_fVelocity;		
		m_bDependScale = m_pFXBundle->m_bDependScale;
		m_bStaticPos = m_pFXBundle->m_bStatic;
		ReloadCombo();
		UpdateData(FALSE);
		return true;
	}

	delete m_pFXBundle;
	m_pFXBundle = NULL;

	return false;
}

bool CDlgEditScript::NewBundle()
{
	if(m_pFXBundle)
	{
		delete m_pFXBundle;
		m_pFXBundle = NULL;
	}

	///////////////////////////////////////////////////
	// Dialog초기화..
	OnInitDialog();
	/*
	for(int i=0;i<MAX_FX_PART_TOOL;i++)
	{
		(*m_pPartStartTime[i]) = 0.0f;		
		m_pPartName[i]->SetCurSel(0);
	}
	m_fVelocity = 0.0f;
	m_bDependScale = FALSE;
	*/
	///////////////////////////////////////////////////

	m_pFXBundle = new CN3FXBundle;
	m_pRefFrm->m_pCurrFX = m_pFXBundle;

	UpdateData(FALSE);
	return true;
}

void CDlgEditScript::OnClose() 
{
	CDialog::OnClose();
	m_pRefFrm->DestroyBundle(this);
}

BOOL CDlgEditScript::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for(int i=0;i<MAX_FX_PART_TOOL;i++)
	{
		(*m_pPartStartTime[i]) = 0.0f;

		m_pPartName[i]->ResetContent();		
		m_pPartName[i]->Clear();
		m_pPartName[i]->InsertString(0,"NONE");
		m_pPartName[i]->SetCurSel(0);

		//	m_CBPartName들 채우기..

		CString strPath;
		CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
		strPath = pBaseFileAccess->PathGet().c_str();
		delete pBaseFileAccess;
		strPath +="fx\\";

		char szCurrPath[_MAX_PATH];
		GetCurrentDirectory(_MAX_PATH, szCurrPath);
		SetCurrentDirectory((LPCTSTR)strPath);
		m_pPartName[i]->Dir(DDL_READONLY, "*.n3fxpart");
		SetCurrentDirectory(szCurrPath);
	}

	m_fVelocity = 0.0f;		
	m_bDependScale = FALSE;
	m_bStaticPos = FALSE;
	m_strPathName.Empty();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEditScript::RefreshParts()
{
	if(!m_pFXBundle) return;

	for(int i=0;i<MAX_FX_PART_TOOL;i++)
	{
		if(m_pFXBundle->m_pPart[i])
		{
			if(m_pFXBundle->m_pPart[i]->pPart) delete m_pFXBundle->m_pPart[i]->pPart;
			delete m_pFXBundle->m_pPart[i];
		}

		m_pFXBundle->m_pPart[i] = new FXPARTWITHSTARTTIME;
		
		int idx = m_pPartName[i]->GetCurSel();
		if(idx>0)
		{
			CString strPath;
			CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
			strPath = pBaseFileAccess->PathGet().c_str();
			delete pBaseFileAccess;
			strPath +="fx\\";
			
			char szBuff[_MAX_PATH];
			m_pPartName[i]->GetLBText(idx, szBuff);
			strPath += szBuff;

			m_pFXBundle->m_pPart[i]->pPart = m_pFXBundle->SetPart((LPCTSTR)strPath);
			m_pFXBundle->m_pPart[i]->fStartTime = (*m_pPartStartTime[i]);			
		}
	}
	m_pFXBundle->Init();

	ReloadCombo();
}

void CDlgEditScript::ReloadCombo()
{
	CString strOldName;
	int idx;
	for(int i=0;i<MAX_FX_PART_TOOL;i++)
	{
		idx = m_pPartName[i]->GetCurSel();
		m_pPartName[i]->GetLBText(idx, strOldName);

		m_pPartName[i]->ResetContent();		
		m_pPartName[i]->Clear();
		m_pPartName[i]->InsertString(0,"NONE");
		m_pPartName[i]->SetCurSel(0);

		//	m_CBPartName들 채우기..
		CString strPath;
		CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
		strPath = pBaseFileAccess->PathGet().c_str();
		delete pBaseFileAccess;
		strPath +="fx\\";

		char szCurrPath[_MAX_PATH];
		GetCurrentDirectory(_MAX_PATH, szCurrPath);
		SetCurrentDirectory((LPCTSTR)strPath);
		m_pPartName[i]->Dir(DDL_READONLY, "*.n3fxpart");
		
		int count = m_pPartName[i]->GetCount();
		for(int j=0;j<count;j++)
		{
			CString DestName;
			m_pPartName[i]->GetLBText(j, DestName);
			if(lstrcmpi((LPCTSTR)DestName, (LPCTSTR)strOldName)==0)
			{
				m_pPartName[i]->SetCurSel(j);
			}
		}
		SetCurrentDirectory(szCurrPath);
	}
}

void CDlgEditScript::OnSelchangeComboPart1() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart2() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart3() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart4() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart5() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart6() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart7() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart8() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart9() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart10() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart11() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart12() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart13() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart14() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart15() { RefreshParts(); }
void CDlgEditScript::OnSelchangeComboPart16() { RefreshParts(); }

void CDlgEditScript::OnChangeEditStartTime1() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime2() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime3() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime4() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime5() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime6() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime7() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime8() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime9() {	UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime10() { UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime11() { UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime12() { UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime13() { UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime14() { UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime15() { UpdateData(TRUE); RefreshParts(); }
void CDlgEditScript::OnChangeEditStartTime16() { UpdateData(TRUE); RefreshParts(); }

void CDlgEditScript::OnDropdownComboPart1() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart2() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart3() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart4() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart5() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart6() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart7() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart8() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart9() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart10() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart11() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart12() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart13() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart14() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart15() { ReloadCombo(); }
void CDlgEditScript::OnDropdownComboPart16() { ReloadCombo(); }

void CDlgEditScript::OnBtnSaveAs() 
{
	CDlgNewFileName dlg;
	dlg.m_strExt = ".N3FXBundle";
	if(dlg.DoModal()==IDOK)
	{
		CString PathName = "fx\\";
		PathName += dlg.m_strNewFileName;
		PathName += dlg.m_strExt;
		CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
		pBaseFileAccess->FileNameSet((LPCTSTR)PathName);

		m_strPathName.Empty();
		m_strPathName = pBaseFileAccess->PathGet().c_str();
		m_strPathName += pBaseFileAccess->FileName().c_str();

		delete pBaseFileAccess;

		char szGameFileName[_MAX_PATH];
		char szExt[_MAX_EXT];
		char szDir[_MAX_DIR];
		char szDrive[_MAX_DRIVE];
		sprintf(szGameFileName, m_pFXBundle->FileName().c_str());
		_splitpath(szGameFileName, szDrive, szDir, NULL, szExt);
		sprintf(szGameFileName,"%s%s%s%s",szDrive, szDir, dlg.m_strNewFileName.GetString(),szExt);		
		m_pFXBundle->FileNameSet(szGameFileName);

		OnBtnSave();
	}
}


void CDlgEditScript::OnBtnRefresh() { RefreshParts(); }


void CDlgEditScript::OnChangeEditVelocity() 
{
	UpdateData(TRUE);
	m_pFXBundle->m_fVelocity = m_fVelocity;	
}


void CDlgEditScript::OnCheckDependScale() 
{
	UpdateData(TRUE);
	if(m_bDependScale==TRUE) m_pFXBundle->m_bDependScale = true;
	else m_pFXBundle->m_bDependScale = false;
}


void CDlgEditScript::OnOK()
{
	return;
	//CDialog::OnOK();
}

void CDlgEditScript::OnDestroy() 
{
	OnBtnStop();

	if(m_pRefFrm->m_pCurrFX == m_pFXBundle) m_pRefFrm->m_pCurrFX = NULL;
	if(m_pFXBundle)
	{
		delete m_pFXBundle;
		m_pFXBundle = NULL;
	}

	CDialog::OnDestroy();	
}



void CDlgEditScript::OnCheckStaticPos() 
{
	UpdateData(TRUE);
	if(m_bStaticPos==TRUE) m_pFXBundle->m_bStatic = true;
	else m_pFXBundle->m_bStatic = false;	
}
