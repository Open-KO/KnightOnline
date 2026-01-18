// DlgSetLightMap.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSetLightMap.h"
#include "MainFrm.h"
#include <N3Base/N3EngTool.h>
#include "LyTerrainDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetLightMap dialog

CDlgSetLightMap::CDlgSetLightMap(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgSetLightMap::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSetLightMap)
	m_rdo_MoreLight = 0;
	//}}AFX_DATA_INIT
}

void CDlgSetLightMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetLightMap)
	DDX_Control(pDX, IDC_LM_SLD_OPACITY, m_sld_Opacity);
	DDX_Control(pDX, IDC_STATIC_SELECTEDCOLOR, m_ColorView);
	DDX_Control(pDX, IDC_LM_SLD_PEN_SIZE, m_sld_PenSize);
	DDX_Control(pDX, IDC_LM_SLD_HARD, m_sld_Hard);
	DDX_Radio(pDX, IDC_RADIO1, m_rdo_MoreLight);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSetLightMap, CDialog)
//{{AFX_MSG_MAP(CDlgSetLightMap)
ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
ON_BN_CLICKED(IDC_BTN_SELECT_COLOR, OnBtnSelectColor)
ON_WM_PAINT()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetLightMap message handlers

BOOL CDlgSetLightMap::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_sld_PenSize.SetRange(1, (LIGHTMAP_TEX_SIZE - 2));
	m_sld_Hard.SetRange(1, 100);
	m_sld_Opacity.SetRange(1, 100);

	m_sld_PenSize.SetPos((LIGHTMAP_TEX_SIZE - 2) / 2);
	m_sld_Hard.SetPos(70);
	m_sld_Opacity.SetPos(100);

	m_rdo_MoreLight = 0;

	m_SelDXColor = m_SelDCColor = 0xff000000;
	m_ColorWidth                = 70;
	m_ColorHeight               = 30;

	m_ColorView.SetWindowPos(
		nullptr, 0, 0, m_ColorWidth, m_ColorHeight, SWP_DRAWFRAME | SWP_NOMOVE);

	Invalidate(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

DWORD CDlgSetLightMap::GetColor()
{
	return m_SelDXColor;
	//	DWORD SrcColor = m_sld_Color.GetPos();
	//	DWORD Color = SrcColor + (SrcColor<<4) + (SrcColor<<8) + (SrcColor<<12) + (SrcColor<<16) + (SrcColor<<20) + (SrcColor<<24) + (SrcColor<<28);
	//	return Color;
}

int CDlgSetLightMap::GetHardness()
{
	int Hard = m_sld_Hard.GetPos();
	return Hard;
}

float CDlgSetLightMap::GetAlpha()
{
	int Opacity  = m_sld_Opacity.GetPos();
	float fAlpha = (float) Opacity / 100.0f;
	return fAlpha;
}

int CDlgSetLightMap::GetPenSize()
{
	int PenSize = m_sld_PenSize.GetPos();
	return PenSize;
}

void CDlgSetLightMap::OnRadio1()
{
	m_rdo_MoreLight = 0;
}

void CDlgSetLightMap::OnRadio2()
{
	m_rdo_MoreLight = 1;
}

bool CDlgSetLightMap::MoreLight()
{
	if (m_rdo_MoreLight)
		return true;

	return false;
}

void CDlgSetLightMap::OnBtnSelectColor()
{
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		COLORREF color = dlg.GetColor();
		DWORD b        = (color << 8) >> 24;
		DWORD g        = ((color >> 8) << 24) >> 16;
		DWORD r        = (color << 24) >> 8;

		m_SelDXColor   = 0xff000000 + b + g + r;
		m_SelDCColor   = color;

		Invalidate(FALSE);
	}
}

void CDlgSetLightMap::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CDC* pDC = m_ColorView.GetDC();

	pDC->FillSolidRect(0, 0, m_ColorWidth, m_ColorHeight, m_SelDCColor);

	ReleaseDC(pDC);
}
