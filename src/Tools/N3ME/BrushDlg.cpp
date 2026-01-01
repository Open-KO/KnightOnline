// BrushDlg.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "BrushDlg.h"
#include "LyTerrain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrushDlg dialog
CBrushDlg::CBrushDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CBrushDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBrushDlg)
	m_fFalloff = 0.0f;
	m_iSize = 0;
	m_iShape = 1;
	m_bFlat = FALSE;
	m_rdoFlatMode = -1;
	//}}AFX_DATA_INIT
	m_pTerrain = nullptr;
}


void CBrushDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrushDlg)
	DDX_Control(pDX, IDC_SLIDER_SIZE, m_sldSize);
	DDX_Control(pDX, IDC_SLIDER_FALLOFF, m_sldFalloff);
	DDX_Text(pDX, IDC_EDIT_FALLOFF, m_fFalloff);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_iSize);
	DDX_CBIndex(pDX, IDC_SHAPE, m_iShape);
	DDX_Check(pDX, IDC_CHECK_FLAT, m_bFlat);
	DDX_Radio(pDX, IDC_RDO_GET_HEIGHT, m_rdoFlatMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBrushDlg, CDialog)
	//{{AFX_MSG_MAP(CBrushDlg)
	ON_CBN_SELCHANGE(IDC_SHAPE, OnSelchangeShape)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_FALLOFF, OnCustomdrawSliderFalloff)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SIZE, OnCustomdrawSliderSize)
	ON_BN_CLICKED(IDC_CHECK_FLAT, OnCheckFlat)
	ON_BN_CLICKED(IDC_RDO_FLATEN, OnRdoFlaten)
	ON_BN_CLICKED(IDC_RDO_GET_HEIGHT, OnRdoGetHeight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrushDlg message handlers

void CBrushDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	//CDialog::OnCancel();
}

void CBrushDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}

void CBrushDlg::OnSelchangeShape() 
{
	SetTerrainBrush();
	
}

void CBrushDlg::OnCustomdrawSliderFalloff(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int iMin, iMax;	m_sldFalloff.GetRange(iMin, iMax);
	m_fFalloff = m_sldFalloff.GetPos()*(1.0f/iMax);
	UpdateData(FALSE);
	SetTerrainBrush();
	
	*pResult = 0;
}

void CBrushDlg::OnCustomdrawSliderSize(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_iSize = m_sldSize.GetPos();
	UpdateData(FALSE);
	SetTerrainBrush();
	
	*pResult = 0;
}

BOOL CBrushDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_rdoFlatMode = 0;
	
	m_sldFalloff.SetRange(0, 100);
	m_sldSize.SetRange(1, 21);

	if(m_pTerrain) m_pTerrain->m_bBrushFlat = FALSE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBrushDlg::SetTerrain(CLyTerrain* pTerrain)
{
	m_pTerrain = pTerrain;
	ShowWindow(m_pTerrain ? TRUE : FALSE);

	if(m_pTerrain) m_pTerrain->m_bBrushFlat = m_bFlat;
}

void CBrushDlg::SetTerrainBrush()
{
	if (m_pTerrain)
	{
		UpdateData(TRUE);
		m_pTerrain->UpdateBrushIntensityMap(m_iShape+1, m_iSize, m_fFalloff);
	}
}

void CBrushDlg::OnCheckFlat() 
{
	UpdateData(TRUE);

	if(m_pTerrain) m_pTerrain->m_bBrushFlat = m_bFlat;
}

void CBrushDlg::OnRdoFlaten() 
{
	m_rdoFlatMode = 1;
	if(m_pTerrain) m_pTerrain->m_bFlaten = true;	
}

void CBrushDlg::OnRdoGetHeight() 
{
	m_rdoFlatMode = 0;
	if(m_pTerrain) m_pTerrain->m_bFlaten = false;	
}

void CBrushDlg::SetFlatHeight(float height)
{
	if(m_pTerrain) m_pTerrain->m_fFlatHeight = height;

	char szHeight[10];
	sprintf(szHeight,"%f",height);
	SetDlgItemText(IDC_FLAT_HEIGHT, szHeight);
}
