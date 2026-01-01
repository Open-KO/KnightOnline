// DlgRiverProperty.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgRiverProperty.h"

#include "RiverMng.h"
#include "RiverMesh.h"
#include <N3Base/N3Texture.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgRiverProperty dialog
CDlgRiverProperty::CDlgRiverProperty(CRiverMng* pRiverMng, CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgRiverProperty::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgRiverProperty)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pRiverMng = pRiverMng;
	m_IsModalDialog = FALSE;
}


void CDlgRiverProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRiverProperty)
	DDX_Control(pDX, IDC_LP_RIVER, m_LPRiver);
	DDX_Control(pDX, IDC_COMBO_RIBER, m_ComGoRiver);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgRiverProperty, CDialog)
	//{{AFX_MSG_MAP(CDlgRiverProperty)
	ON_BN_CLICKED(IDC_BUTTON_EXTRUDE, OnButtonExtrude)
	ON_BN_CLICKED(IDC_BUTTON_RECALC_UV, OnButtonRecalcUv)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_RIVER, OnButtonDeleteRiver)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_VERTEX, OnButtonDeleteVertex)
	ON_BN_CLICKED(IDC_BUTTON_RECALCVERTEX, OnButtonRecalcvertex)
	ON_CBN_SELCHANGE(IDC_COMBO_RIBER,OnCobGORiver)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRiverProperty message handlers

BOOL CDlgRiverProperty::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ComGoRiver.ResetContent();
	int idx = m_ComGoRiver.AddString("NONE");
	m_ComGoRiver.SetItemData(idx, 0);

	m_LPRiver.AddPropItem("River ID", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("Alpha factor(hex)", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("Speed1", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("meter / u", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("meter / v", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("Texture File", "", PIT_FILE, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA|");
	m_LPRiver.AddPropItem("Animation Texture FPS", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("Speed2", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("meter / u2", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("meter / v2", "", PIT_EDIT, "");
	m_LPRiver.AddPropItem("Animation Texture File", "", PIT_FILE, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA|");
	
	int nW = 100;
	m_LPRiver.SetDividerWidth(nW);

	if (m_IsModalDialog)
	{
		GetDlgItem(IDC_BUTTON_EXTRUDE)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DELETE_VERTEX)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DELETE_RIVER)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_RECALC_UV)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_RECALCVERTEX)->ShowWindow(FALSE);
		GetDlgItem(IDC_COMBO_RIBER)->ShowWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDOK)->ShowWindow(FALSE);
		GetDlgItem(IDCANCEL)->ShowWindow(FALSE);
	}

	UpdateInfo();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgRiverProperty::UpdateInfo()
{
	ASSERT(m_pRiverMng);
	CRiverMesh* pSelRiver = m_pRiverMng->GetSelRiver();
	if (pSelRiver == nullptr)
	{
		m_LPRiver.ShowWindow(FALSE);
		if (m_IsModalDialog==FALSE)
		{
			GetDlgItem(IDC_BUTTON_EXTRUDE)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_DELETE_VERTEX)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_DELETE_RIVER)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_RECALC_UV)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_RECALCVERTEX)->ShowWindow(FALSE);
			GetDlgItem(IDC_COMBO_RIBER)->ShowWindow(TRUE);
		}
		GetDlgItem(IDC_NOTSELECTED)->ShowWindow(TRUE);
		return;
	}
	GetDlgItem(IDC_NOTSELECTED)->ShowWindow(FALSE);
	m_LPRiver.ShowWindow(TRUE);
	if (m_IsModalDialog==FALSE)
	{
		GetDlgItem(IDC_BUTTON_EXTRUDE)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DELETE_VERTEX)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DELETE_RIVER)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_RECALC_UV)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_RECALCVERTEX)->ShowWindow(TRUE);
		GetDlgItem(IDC_COMBO_RIBER)->ShowWindow(TRUE);
	}


	CPropertyItem* pItem;
	pItem = m_LPRiver.GetPropItem("River ID");
	if(pItem) pItem->m_curValue.Format("%d", pSelRiver->GetRiverID());
	pItem = m_LPRiver.GetPropItem("Alpha factor(hex)");
	if(pItem) pItem->m_curValue.Format("0x%08x", pSelRiver->GetAlphaFactor());
	pItem = m_LPRiver.GetPropItem("Speed1");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetSpeed1());
	pItem = m_LPRiver.GetPropItem("meter / u");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetMeterPerU());
	pItem = m_LPRiver.GetPropItem("meter / v");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetMeterPerV());

	pItem = m_LPRiver.GetPropItem("Texture File");
	if(pItem)
	{
		CN3Texture* pTex = pSelRiver->TexGet();
		if(pTex) pItem->m_curValue = pTex->FileName().c_str();
		else pItem->m_curValue = "";
	}

	pItem = m_LPRiver.GetPropItem("Animation Texture FPS");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetAnimTexFPS());
	pItem = m_LPRiver.GetPropItem("Speed2");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetSpeed2());
	pItem = m_LPRiver.GetPropItem("meter / u2");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetMeterPerU2());
	pItem = m_LPRiver.GetPropItem("meter / v2");
	if(pItem) pItem->m_curValue.Format("%f", pSelRiver->GetMeterPerV2());

	pItem = m_LPRiver.GetPropItem("Animation Texture File");
	if(pItem)
	{
		int iCount = pSelRiver->GetAnimTexCount();
		CN3Texture* pTex = pSelRiver->AnimTexGet(iCount-1);
		if(pTex) pItem->m_curValue = pTex->FileName().c_str();
		else pItem->m_curValue = "";
	}

	int iRiverID = pSelRiver->GetRiverID();
	RiverComboFinding(iRiverID);
}

void CDlgRiverProperty::RiverComboFinding(int iRiverID)
{
	for(int i=0;i<m_ComGoRiver.GetCount(); i++)
	{
		unsigned char cId = (int)m_ComGoRiver.GetItemData(i);
		if(iRiverID==cId)
		{
			m_ComGoRiver.SetCurSel(i);
			break;
		}
	}
}

void CDlgRiverProperty::RiverComboSetting()
{
	m_ComGoRiver.ResetContent();
	ASSERT(m_pRiverMng);

	int idx;
	int iRiverSize = m_pRiverMng->GetDrawRiverNum();
	if(iRiverSize == 0) 
	{
		idx = m_ComGoRiver.AddString("NONE");
		m_ComGoRiver.SetItemData(idx, 0);

		return;
	}

	idx = m_ComGoRiver.AddString("Select");
	m_ComGoRiver.SetItemData(idx, 0);


	int iRiverID;
	char szRiverID[10];
	it_RiverMesh it = m_pRiverMng->GetDrawRiver();
	for(int i =0;i<iRiverSize; ++i,++it)
	{
		CRiverMesh* pRM = *it;
		if(pRM)
		{
			iRiverID = pRM->GetRiverID();
			sprintf(szRiverID,"%d",iRiverID);
			idx = m_ComGoRiver.AddString(szRiverID);
			m_ComGoRiver.SetItemData(idx, (DWORD_PTR) iRiverID);
		}
	}

}

void CDlgRiverProperty::OnCobGORiver()
{
	int idx = m_ComGoRiver.GetCurSel();
	int iFindRiver = (unsigned char)m_ComGoRiver.GetItemData(idx);	

	m_pRiverMng->GoRiver(iFindRiver);
}

BOOL CDlgRiverProperty::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(m_pRiverMng);
	CRiverMesh* pSelRiver = m_pRiverMng->GetSelRiver();
	if((void*)wParam == &m_LPRiver)
	{
		if (pSelRiver)
		{
			CPropertyItem* pItem = (CPropertyItem*)lParam;
			if (pItem->m_propName == "River ID")
			{
				if ( m_pRiverMng->SetRiverID(pSelRiver, (int)atoi(pItem->m_curValue)) == FALSE )
				{
					MessageBox("존재하는 ID입니다.");
					pItem->m_curValue.Format("%d", pSelRiver->GetRiverID());
				}
			}
			else if (pItem->m_propName == "Alpha factor(hex)") pSelRiver->SetAlphaFactor(strtoul(pItem->m_curValue, nullptr, 16));
			else if (pItem->m_propName == "Speed1") pSelRiver->SetSpeed1((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "meter / u") pSelRiver->SetMeterPerU((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "meter / v") pSelRiver->SetMeterPerV((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "Texture File")
			{
				CN3Base tmp;	tmp.m_szName = pItem->m_curValue;		// 상대경로로 바꾸기
				if (pSelRiver->SetTextureName(tmp.m_szName.c_str()) == FALSE)
				{
					CString strMsg;
					strMsg.Format("Cannot get \"%s\"Texture, check file and directory",	pItem->m_curValue);
					MessageBox(strMsg);
					pItem->m_curValue = "";
				}
				else pItem->m_curValue = tmp.m_szName.c_str();
			}
			else if (pItem->m_propName == "Animation Texture FPS") pSelRiver->SetAnimTexFPS((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "Speed2") pSelRiver->SetSpeed2((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "meter / u2") pSelRiver->SetMeterPerU2((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "meter / v2") pSelRiver->SetMeterPerV2((float)atof(pItem->m_curValue));
			else if (pItem->m_propName == "Animation Texture File")
			{
				// 에니메이션 되는 텍스쳐 지정 (갯수는 파일 이름으로부터 알아낸다. 따라서 맨 마지막번호파일을 지정해야함)

				CN3Base tmp;	tmp.m_szName = pItem->m_curValue;		// 상대경로로 바꾸기
				// 화일 이름 분리
				char szDir[_MAX_DIR];
				char szFName[_MAX_FNAME];
				char szExt[_MAX_EXT];
				_splitpath(tmp.m_szName.c_str(), nullptr, szDir, szFName, szExt);
				int iCount = atoi(szFName+lstrlen(szFName)-2) + 1;	// 파일 이름의 끝에 두자리를 숫자로 변환
				CString strFName = szDir; strFName += szFName;
				strFName = strFName.Left(strFName.GetLength()-2);

				if (pSelRiver->SetAnimTextureName(strFName, szExt, iCount ) == FALSE)
				{
					CString strMsg;
					strMsg.Format("Cannot get \"%s\"Texture, check file and directory",	pItem->m_curValue);
					MessageBox(strMsg);
					pItem->m_curValue = "";
				}
				else pItem->m_curValue = tmp.m_szName.c_str();
			}
		}
		return TRUE;
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CDlgRiverProperty::OnOK() 
{
	if (m_IsModalDialog)
	{
		CRiverMesh* pSelRiver = m_pRiverMng->GetSelRiver();
		ASSERT(pSelRiver);
		if (pSelRiver && pSelRiver->TexGet())
		{
			if (m_pRiverMng->GetRiverMesh(pSelRiver->GetRiverID()) == nullptr) CDialog::OnOK();
			else MessageBox("중복되는 아이디 입니다.");
		}
		else
		{
			MessageBox("Texture를 지정하지 않았습니다.");
		}
	}
}

void CDlgRiverProperty::OnCancel() 
{
	if (m_IsModalDialog) CDialog::OnCancel();
}

void CDlgRiverProperty::OnButtonExtrude() 
{
	if (m_IsModalDialog) return;

	m_pRiverMng->ExtrudeRiverEdge();
}

void CDlgRiverProperty::OnButtonRecalcUv() 
{
	if (m_IsModalDialog) return;
	m_pRiverMng->ReCalcUV();
}

void CDlgRiverProperty::OnButtonDeleteRiver() 
{
	if (m_IsModalDialog) return;
	CRiverMesh* pSelRiver = m_pRiverMng->GetSelRiver();
	if (pSelRiver)
	{
		if (MessageBox("선택된 강을 지우시겠습니까?", "Remove river", MB_YESNO|MB_DEFBUTTON2) == IDNO) return;
		m_pRiverMng->RemoveRiverMesh(pSelRiver->GetRiverID());
	}
}

void CDlgRiverProperty::OnButtonDeleteVertex() 
{
	if (m_IsModalDialog) return;
	m_pRiverMng->DeleteSelectedVertex();
}

void CDlgRiverProperty::OnButtonRecalcvertex() 
{
	if (m_IsModalDialog) return;
	m_pRiverMng->ReCalcSelectedVertex();
}
