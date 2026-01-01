// DlgBar.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "MainFrm.h"
#include "MapMng.h"
#include "LyTerrain.h"
#include "DlgBar.h"
#include <N3Base/N3TransformCollision.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBar dialog


CDlgBar::CDlgBar()
{
	//{{AFX_DATA_INIT(CDlgBar)
	m_iZoneID = 0;
	//}}AFX_DATA_INIT

		CSliderCtrl	m_TileRegion_Slider;
	
//	m_FP_Slider.SetRange(100,4200);
//	m_FP_Slider.SetPos(100);
	
	m_bUpdateingNow = FALSE;
}


void CDlgBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBar)
	DDX_Control(pDX, IDC_SLIDER_LIGHT, m_sld_Light);
	DDX_Control(pDX, IDC_SLIDER_TILEREGION, m_TileRegion_Slider);
	DDX_Control(pDX, IDC_SLIDER_FARPLANE, m_FP_Slider);
	DDX_Text(pDX, IDC_E_ZONEID, m_iZoneID);
	DDV_MinMaxInt(pDX, m_iZoneID, 0, 9999);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgBar, CDialog)
	//{{AFX_MSG_MAP(CDlgBar)
	ON_BN_CLICKED(IDC_B_SET_RESOURCE_PATH, OnBSetResourcePath)
	ON_EN_CHANGE(IDC_E_TRANSFORM_X, OnChangeETransformX)
	ON_EN_CHANGE(IDC_E_TRANSFORM_Y, OnChangeETransformY)
	ON_EN_CHANGE(IDC_E_TRANSFORM_Z, OnChangeETransformZ)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FARPLANE, OnReleasedcaptureSliderFarplane)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_TILEREGION, OnReleasedcaptureSliderTileregion)
	ON_EN_CHANGE(IDC_E_ZONEID, OnChangeEZoneid)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_LIGHT, OnReleasedcaptureSliderLight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBar message handlers

void CDlgBar::OnBSetResourcePath() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->OnResourcePathSet(); // Project ¼¼ÆÃ..
}

void CDlgBar::OnChangeETransformX() 
{
	this->UpdateDataFromControl();
}

void CDlgBar::OnChangeETransformY() 
{
	this->UpdateDataFromControl();
}

void CDlgBar::OnChangeETransformZ() 
{
	this->UpdateDataFromControl();
}

void CDlgBar::UpdateInfo()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	if(nullptr == pFrm->GetMapMng()) return;

	int nSelCount = pFrm->GetMapMng()->GetSelOutputObjCount();
	if (nSelCount <= 0) return;

	m_bUpdateingNow = TRUE;
	CN3TransformCollision* pTrans = (CN3TransformCollision*)pFrm->GetMapMng()->GetSelOutputObj(0);
	int nCM = pFrm->GetMapMng()->GetCursorMode();
	if(nSelCount == 1 && nullptr != pTrans && (CM_POS == nCM || CM_SCALE == nCM))
	{
		__Vector3 v;
		
		if(nCM == CM_POS)
		{
			v = pTrans->Pos();
			SetDlgItemText(IDC_STATIC_TRANSFORM, "Position");
		}
		else if(nCM == CM_SCALE)
		{
			v = pTrans->Scale();
			SetDlgItemText(IDC_STATIC_TRANSFORM, "Scale");
		}

		CString szVal = "";
		szVal.Format("%f", v.x); SetDlgItemText(IDC_E_TRANSFORM_X, szVal);
		szVal.Format("%f", v.y); SetDlgItemText(IDC_E_TRANSFORM_Y, szVal);
		szVal.Format("%f", v.z); SetDlgItemText(IDC_E_TRANSFORM_Z, szVal);
	}
	else
	{
		SetDlgItemText(IDC_E_TRANSFORM_X, "");
		SetDlgItemText(IDC_E_TRANSFORM_Y, "");
		SetDlgItemText(IDC_E_TRANSFORM_Z, "");
	}

	m_bUpdateingNow = FALSE;
}

void CDlgBar::UpdateDataFromControl()
{
	if(TRUE == m_bUpdateingNow) return;
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	if(nullptr == pFrm->GetMapMng()) return;

	int nSelCount = pFrm->GetMapMng()->GetSelOutputObjCount();
	CN3Transform* pTrans = pFrm->GetMapMng()->GetSelOutputObj(0);
	int nCM = pFrm->GetMapMng()->GetCursorMode();
	if(nSelCount != 1 || nullptr == pTrans && CM_POS != nCM && CM_SCALE != nCM)
	{
		return;
	}
	else
	{
		__Vector3 v;
		CString szVal = "";
		GetDlgItemText(IDC_E_TRANSFORM_X, szVal); v.x = (float)atof(szVal);
		GetDlgItemText(IDC_E_TRANSFORM_Y, szVal); v.y = (float)atof(szVal);
		GetDlgItemText(IDC_E_TRANSFORM_Z, szVal); v.z = (float)atof(szVal);
		
		if(nCM == CM_POS)
		{
			pTrans->PosSet(v);
		}
		else if(nCM == CM_SCALE)
		{
			pTrans->ScaleSet(v);
		}

		pFrm->GetActiveView()->InvalidateRect(nullptr, FALSE);
	}

	m_bUpdateingNow = FALSE;
}

void CDlgBar::OnReleasedcaptureSliderFarplane(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;
	if(pFrm->GetMapMng())
	{
		pFrm->GetMapMng()->UpDateFP();
	}
	
	*pResult = 0;
}

void CDlgBar::OnReleasedcaptureSliderTileregion(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;
	if(pFrm->GetMapMng())
	{
		if(pFrm->GetMapMng()->GetTerrain())
			pFrm->GetMapMng()->GetTerrain()->UpDateDistLimit();
	}
	
	*pResult = 0;
}

BOOL CDlgBar::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_FP_Slider.SetRange(128,5120);
	m_FP_Slider.SetPos(512);

	m_TileRegion_Slider.SetRange(10,100);
	m_TileRegion_Slider.SetPos(30);

	m_sld_Light.SetRange(1,100);
	m_sld_Light.SetPos(70);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgBar::OnChangeEZoneid() 
{
	UpdateData();

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(pFrm->GetMapMng())
	{
		pFrm->GetMapMng()->SetZoneID(m_iZoneID);
	}
}

void CDlgBar::OnReleasedcaptureSliderLight(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;
	if(pFrm->GetMapMng())
	{
		int val = m_sld_Light.GetPos();
		float lgt = (float)val / 100.0f;
		pFrm->GetMapMng()->SetLight(lgt);
	}
	
	*pResult = 0;
}
