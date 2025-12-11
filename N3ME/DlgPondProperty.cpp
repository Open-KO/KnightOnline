// DlgPondProperty.cpp: implementation of the CDlgPondProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "DlgPondProperty.h"

#include "PondMng.h"
#include "PondMesh.h"
#include <N3Base/N3Texture.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgPondProperty::CDlgPondProperty(CPondMng* pPondMng, CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgPondProperty::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPondProperty)
		// NOTE: the ClassWizard will add member initialization here
	m_fWidthLength = 0.0f;
	m_fHeightLength = 0.0f;
	//}}AFX_DATA_INIT
	m_pPondMng = pPondMng;
	m_IsModalDialog = FALSE;
}

void CDlgPondProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPondProperty)
	DDX_Control(pDX, IDC_LP_POND, m_LPPond);
	DDX_Control(pDX, IDC_COMBO_GOPOND, m_ComGoPond);
	DDX_Text(pDX, IDC_WIDTHLENGTH, m_fWidthLength);
	DDX_Text(pDX, IDC_HEIGHTLENGTH, m_fHeightLength);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPondProperty, CDialog)
	//{{AFX_MSG_MAP(CDlgPondProperty)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_POND, OnButtonDeletePond)
	ON_BN_CLICKED(IDC_BUTTON_EDITPOND, OnButtonEditPond)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_POND, OnButtonMovePond)
	ON_BN_CLICKED(IDC_BUTTON_STATION_POND, OnButtonStationPond)
	ON_BN_CLICKED(IDC_BU_RECALCUV,OnButtonRecalUV)
	ON_BN_CLICKED(IDC_GROUP,OnButtonGroup)
	ON_CBN_SELCHANGE(IDC_COMBO_GOPOND,OnCobGOPond)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPondProperty message handlers

BOOL CDlgPondProperty::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ComGoPond.ResetContent();
	int idx = m_ComGoPond.AddString("NONE");
	m_ComGoPond.SetItemData(idx, 0);
	
	m_LPPond.AddPropItem("Pond ID", "", PIT_EDIT, "");

	m_LPPond.AddPropItem("WaterHeight(float)", "", PIT_EDIT, "");
	m_LPPond.AddPropItem("WaterScaleWidth(int)", "", PIT_EDIT, "");
	m_LPPond.AddPropItem("WaterScaleHeight(int)", "", PIT_EDIT, "");

	m_LPPond.AddPropItem("Texture File", "", PIT_FILE, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA|");
	m_LPPond.AddPropItem("Alpha factor(hex)", "", PIT_EDIT, "");

	m_LPPond.AddPropItem("Water tu(float)", "", PIT_EDIT, "");
	m_LPPond.AddPropItem("Water tv(float)", "", PIT_EDIT, "");

	m_LPPond.AddPropItem("left(float)", "", PIT_EDIT, "");
	m_LPPond.AddPropItem("top(float)", "", PIT_EDIT, "");
	m_LPPond.AddPropItem("right(float)", "", PIT_EDIT, "");
	m_LPPond.AddPropItem("bottom(float)", "", PIT_EDIT, "");
	
	int nW = 150;
	m_LPPond.SetDividerWidth(nW);

	if (m_IsModalDialog)
	{
		GetDlgItem(IDC_BUTTON_DELETE_POND)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EDITPOND)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_MOVE_POND)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_STATION_POND)->ShowWindow(FALSE);
		GetDlgItem(IDC_WIDTHLENGTH)->ShowWindow(FALSE);
		GetDlgItem(IDC_HEIGHTLENGTH)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATICNAME1)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATICNAME2)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC3)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC4)->ShowWindow(FALSE);
		GetDlgItem(IDC_BU_RECALCUV)->ShowWindow(FALSE);
		GetDlgItem(IDC_GROUP)->ShowWindow(FALSE);
		GetDlgItem(IDC_COMBO_GOPOND)->ShowWindow(FALSE);

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

void CDlgPondProperty::UpdateInfo()
{
	ASSERT(m_pPondMng);
	CPondMesh* pSelPond = m_pPondMng->GetSelPond();
	if (pSelPond == nullptr)
	{
		m_LPPond.ShowWindow(FALSE);
		if (m_IsModalDialog==FALSE)
		{
			GetDlgItem(IDC_BUTTON_DELETE_POND)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_EDITPOND)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_MOVE_POND)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_STATION_POND)->ShowWindow(FALSE);
			GetDlgItem(IDC_WIDTHLENGTH)->ShowWindow(FALSE);
			GetDlgItem(IDC_HEIGHTLENGTH)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATICNAME1)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATICNAME2)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC3)->ShowWindow(FALSE);
			GetDlgItem(IDC_BU_RECALCUV)->ShowWindow(FALSE);
			GetDlgItem(IDC_GROUP)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC4)->ShowWindow(TRUE);
			GetDlgItem(IDC_COMBO_GOPOND)->ShowWindow(TRUE);
		}
		return;
	}
	m_LPPond.ShowWindow(FALSE);
//	m_LPPond.ShowWindow(TRUE);
	if (m_IsModalDialog==FALSE)
	{
		GetDlgItem(IDC_BUTTON_DELETE_POND)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EDITPOND)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_MOVE_POND)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STATION_POND)->ShowWindow(TRUE);
		GetDlgItem(IDC_WIDTHLENGTH)->ShowWindow(TRUE);
		GetDlgItem(IDC_HEIGHTLENGTH)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATICNAME1)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATICNAME2)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC3)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC4)->ShowWindow(TRUE);
		GetDlgItem(IDC_BU_RECALCUV)->ShowWindow(TRUE);
		GetDlgItem(IDC_GROUP)->ShowWindow(TRUE);
		GetDlgItem(IDC_COMBO_GOPOND)->ShowWindow(TRUE);

		if(m_pPondMng->GetChooseEditPond()==TRUE)
			 GetDlgItem(IDC_BUTTON_EDITPOND)->SetWindowText("Pond Lock Edit");
		else GetDlgItem(IDC_BUTTON_EDITPOND)->SetWindowText("Pond UnLock Edit");

		if(pSelPond->GetChangUVState()==TRUE)	
			 GetDlgItem(IDC_BU_RECALCUV)->SetWindowText("점으로");
		else GetDlgItem(IDC_BU_RECALCUV)->SetWindowText("평편하게");

		if(m_pPondMng->GetChooseGroup()==TRUE)
			 GetDlgItem(IDC_GROUP)->SetWindowText("Gup On");
		else GetDlgItem(IDC_GROUP)->SetWindowText("Gup Off");
	}


	CPropertyItem* pItem;
	pItem = m_LPPond.GetPropItem("Pond ID");
	if(pItem) pItem->m_curValue.Format("%d", pSelPond->GetPondID());
	pItem = m_LPPond.GetPropItem("WaterHeight(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetWaterHeight());
	pItem = m_LPPond.GetPropItem("WaterScaleWidth(int)");
	if(pItem) pItem->m_curValue.Format("%d", pSelPond->GetWaterScaleWidht());
	pItem = m_LPPond.GetPropItem("WaterScaleHeight(int)");
	if(pItem) pItem->m_curValue.Format("%d", pSelPond->GetWaterScaleHeight());
	pItem = m_LPPond.GetPropItem("Alpha factor(hex)");
	if(pItem) pItem->m_curValue.Format("0x%08x", pSelPond->GetAlphaFactor());
	pItem = m_LPPond.GetPropItem("Water tu(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetTV());
	pItem = m_LPPond.GetPropItem("Water tv(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetTV());

	pItem = m_LPPond.GetPropItem("left(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetLeft());
	pItem = m_LPPond.GetPropItem("top(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetTop());
	pItem = m_LPPond.GetPropItem("right(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetRight());
	pItem = m_LPPond.GetPropItem("bottom(float)");
	if(pItem) pItem->m_curValue.Format("%f", pSelPond->GetBottom());

	pItem = m_LPPond.GetPropItem("Texture File");
	if(pItem)
	{
		CN3Texture* pTex = pSelPond->TexGet();
		if(pTex) pItem->m_curValue = pTex->FileName().c_str();
		else pItem->m_curValue = "";
	}

	m_LPPond.ShowWindow(TRUE);

	UpdateWaterLength(pSelPond);

	int iPondID = pSelPond->GetPondID();
	PondComboFinding(iPondID);
}

void CDlgPondProperty::PondComboFinding(int iPondID)
{
	for(int i=0;i<m_ComGoPond.GetCount(); i++)
	{
		unsigned char cId = (int)m_ComGoPond.GetItemData(i);
		if(iPondID==cId)
		{
			m_ComGoPond.SetCurSel(i);
			break;
		}
	}
}

void CDlgPondProperty::PondComboSetting()
{
	m_ComGoPond.ResetContent();
	ASSERT(m_pPondMng);

	int idx;
	int iPondSize = m_pPondMng->GetDrawPondNum();
	if(iPondSize == 0) 
	{
		idx = m_ComGoPond.AddString("NONE");
		m_ComGoPond.SetItemData(idx, 0);

		return;
	}

	idx = m_ComGoPond.AddString("Select");
	m_ComGoPond.SetItemData(idx, 0);


	int iPondID;
	char szPondID[10];
	it_PondMesh it = m_pPondMng->GetDrawPond();
	for(int i =0;i<iPondSize; ++i,++it)
	{
		CPondMesh* pRM = *it;
		if(pRM)
		{
			iPondID = pRM->GetPondID();
			sprintf(szPondID,"%d",iPondID);
			idx = m_ComGoPond.AddString(szPondID);
			m_ComGoPond.SetItemData(idx, iPondID);
		}
	}

}

void CDlgPondProperty::OnCobGOPond()
{
	int idx = m_ComGoPond.GetCurSel();
	int iFindPond = (unsigned char)m_ComGoPond.GetItemData(idx);	

	m_pPondMng->GoPond(iFindPond);
}

void CDlgPondProperty::UpdateWaterLength(CPondMesh* pSelPond)
{
	if(pSelPond==nullptr) return;

	m_fWidthLength = pSelPond->GetWaterWidthLength();
	m_fHeightLength = pSelPond->GetWaterHeightLength();
	UpdateData(FALSE);
}

BOOL CDlgPondProperty::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(m_pPondMng);
	CPondMesh* pSelPond = m_pPondMng->GetSelPond();
	if((void*)wParam == &m_LPPond)
	{
		if (pSelPond)
		{
			CPropertyItem* pItem = (CPropertyItem*)lParam;
			if (pItem->m_propName == "Pond ID")
			{
				if ( m_pPondMng->SetPondID(pSelPond, (int)atoi(pItem->m_curValue)) == FALSE )
				{
					pItem->m_curValue.Format("%d", pSelPond->GetPondID());
					UpdateData(FALSE);
					return TRUE;
				}
			}
			else if (pItem->m_propName == "WaterHeight(float)")		{pSelPond->SetWaterHeight((float)atof(pItem->m_curValue)); }
			else if (pItem->m_propName == "Alpha factor(hex)")		{pSelPond->SetAlphaFactor(strtoul(pItem->m_curValue, nullptr, 16)); }
			else if (pItem->m_propName == "Water tu(float)")		{pSelPond->SetTU((float)atof(pItem->m_curValue)); }
			else if (pItem->m_propName == "Water tv(float)")		{pSelPond->SetTV((float)atof(pItem->m_curValue)); }
			else if (pItem->m_propName == "left(float)")			{pSelPond->SetLeft((float)atof(pItem->m_curValue));		UpdateWaterLength(pSelPond);}
			else if (pItem->m_propName == "top(float)")				{pSelPond->SetTop((float)atof(pItem->m_curValue));		UpdateWaterLength(pSelPond);}
			else if (pItem->m_propName == "right(float)")			{pSelPond->SetRight((float)atof(pItem->m_curValue));	UpdateWaterLength(pSelPond);}
			else if (pItem->m_propName == "bottom(float)")			{pSelPond->SetBottom((float)atof(pItem->m_curValue));	UpdateWaterLength(pSelPond);}
			else if (pItem->m_propName == "WaterScaleWidth(int)")	
			{
				int iWidht = (int)atoi(pItem->m_curValue);
				if(MAX_PONDMESH_VERTEX< iWidht*pSelPond->GetWaterScaleHeight())
				{
					MessageBox("만들수있는 한계를 넘었습니다.");
					pItem->m_curValue.Format("%d", pSelPond->GetWaterScaleWidht());
					return TRUE;
				}
				else
				{
					pSelPond->SetWaterScaleWidht(iWidht);
					UpdateWaterLength(pSelPond);
				}				
			}
			else if (pItem->m_propName == "WaterScaleHeight(int)")	
			{
				int iHeight = (int)atoi(pItem->m_curValue);
				if(MAX_PONDMESH_VERTEX< iHeight*pSelPond->GetWaterScaleWidht())
				{
					MessageBox("만들수있는 한계를 넘었습니다.");
					pItem->m_curValue.Format("%d", pSelPond->GetWaterScaleHeight());
					return TRUE;
				}
				else
				{
					pSelPond->SetWaterScaleHeight(iHeight); 
					UpdateWaterLength(pSelPond);
				}
			}
			else if (pItem->m_propName == "Texture File")
			{
				CN3Base tmp;
				tmp.m_szName = pItem->m_curValue;		// 상대경로로 바꾸기
				if (pSelPond->SetTextureName(tmp.m_szName.c_str()) == FALSE)
				{
					CString strMsg;
					strMsg.Format("Cannot get \"%s\"Texture, check file and directory",	pItem->m_curValue);
					MessageBox(strMsg);
					pItem->m_curValue = "";
				}
				else pItem->m_curValue = tmp.m_szName.c_str();
				return TRUE;
			}	
			m_pPondMng->MainInvalidate();
		}
		return TRUE;
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CDlgPondProperty::OnOK() 
{
	if (m_IsModalDialog)
	{
		CPondMesh* pSelPond = m_pPondMng->GetSelPond();
		ASSERT(pSelPond);
		if (pSelPond && pSelPond->TexGet())
		{
			if (m_pPondMng->GetPondMesh(pSelPond->GetPondID()) == nullptr) CDialog::OnOK();
			else MessageBox("중복되는 아이디 입니다.");
		}
		else
		{
			MessageBox("Texture를 지정하지 않았습니다.");
		}
	}
}

void CDlgPondProperty::OnCancel() 
{
	if (m_IsModalDialog) CDialog::OnCancel();
}

void CDlgPondProperty::OnButtonEditPond()
{
	if (m_IsModalDialog) return;

	m_pPondMng->ChooseEditPond();
	if(m_pPondMng->GetChooseEditPond()==TRUE)
		GetDlgItem(IDC_BUTTON_EDITPOND)->SetWindowText("Pond Lock Edit");
	else GetDlgItem(IDC_BUTTON_EDITPOND)->SetWindowText("Pond UnLock Edit");
}

//void CDlgPondProperty::OnButtonExtrude() 
//{
//	if (m_IsModalDialog) return;
//
//	m_pPondMng->ExtrudePondEdge();
//}

void CDlgPondProperty::OnButtonRecalUV() 
{
	if (m_IsModalDialog) return;
	if (m_pPondMng == nullptr) return;

	CPondMesh* pSelPond = m_pPondMng->GetSelPond();
	if(pSelPond==nullptr) return;

	pSelPond->SetChangUVState();
	if(pSelPond->GetChangUVState()==TRUE)
	{
		GetDlgItem(IDC_BU_RECALCUV)->SetWindowText("점으로");
		pSelPond->ReCalcUV();
	}
	else
	{
		GetDlgItem(IDC_BU_RECALCUV)->SetWindowText("평편하게");
		pSelPond->ReCalcVexUV();
	}

	m_pPondMng->MainInvalidate();
}

void CDlgPondProperty::OnButtonDeletePond() 
{
	if (m_IsModalDialog) return;
	CPondMesh* pSelPond = m_pPondMng->GetSelPond();
	if (pSelPond)
	{
		if (MessageBox("선택된 연못을 지우시겠습니까?", "Remove pond", MB_YESNO|MB_DEFBUTTON2) == IDNO) return;
		m_pPondMng->RemovePondMesh(pSelPond->GetPondID());

		UpdateInfo();
	}
}

void CDlgPondProperty::OnButtonMovePond()
{
	if (m_IsModalDialog) return;
	m_pPondMng->MovePond();
}

void CDlgPondProperty::OnButtonStationPond()
{
	if (m_IsModalDialog) return;
	m_pPondMng->StationPond();

	CPondMesh* pSelPond = m_pPondMng->GetSelPond();
	UpdateWaterLength(pSelPond);
}

//void CDlgPondProperty::OnButtonDeleteVertex() 
//{
//	if (m_IsModalDialog) return;
//	m_pPondMng->DeleteSelectedVertex();
//}

//void CDlgPondProperty::OnButtonRecalcvertex() 
//{
//	if (m_IsModalDialog) return;
//	m_pPondMng->ReCalcSelectedVertex();
//}

void CDlgPondProperty::OnButtonGroup()
{
	if (m_IsModalDialog) return;

	m_pPondMng->ChooseGroupPond();
	if(m_pPondMng->GetChooseGroup()==TRUE)
		GetDlgItem(IDC_GROUP)->SetWindowText("Gup On");
	else GetDlgItem(IDC_GROUP)->SetWindowText("Gup Off");
}
