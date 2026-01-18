// DlgLight.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgLight.h"
#include <N3Base/N3Light.h>
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLight dialog

CDlgLight::CDlgLight(CWnd* pParent /*=nullptr*/) : CDialog(CDlgLight::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLight)
	m_strPathName     = _T("");
	m_strLightObjName = _T("");
	//}}AFX_DATA_INIT

	m_pRefLightObjMgr = nullptr;
	m_pSelLO          = nullptr;
}

void CDlgLight::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLight)
	DDX_Control(pDX, IDC_STC_COLOR, m_stcColor);
	DDX_Control(pDX, IDC_SLD_ATTEN, m_sld_Atten);
	DDX_Control(pDX, IDC_SLD_RANGE, m_sld_Range);
	DDX_Control(pDX, IDC_LIST_LIGHTS, m_ListLO);
	DDX_Text(pDX, IDC_PATHNAME, m_strPathName);
	DDX_Text(pDX, IDC_EDT_NAME, m_strLightObjName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgLight, CDialog)
//{{AFX_MSG_MAP(CDlgLight)
ON_BN_CLICKED(IDC_BTN_EXPORT, OnBtnExport)
ON_BN_CLICKED(IDC_BTN_IMPORT, OnBtnImport)
ON_BN_CLICKED(IDC_BTN_UPLOAD, OnBtnUpload)
ON_BN_CLICKED(IDC_BTN_DOWNLOAD, OnBtnDownload)
ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
ON_LBN_SELCHANGE(IDC_LIST_LIGHTS, OnSelchangeListLights)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLD_ATTEN, OnReleasedcaptureSldAtten)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLD_RANGE, OnReleasedcaptureSldRange)
ON_WM_PAINT()
ON_WM_LBUTTONUP()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLight message handlers

BOOL CDlgLight::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_sld_Atten.SetRange(0, 100, TRUE);
	m_sld_Atten.SetPos(100);

	m_sld_Range.SetRange(0, 500, TRUE);
	m_sld_Range.SetPos(0);

	m_ListLO.ResetContent();
	m_strPathName.Empty();
	m_strLightObjName.Empty();

	m_pLOColor = (COLORREF) (0x00ffffff);
	m_pSelLO   = nullptr;

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLight::OnOK()
{
	return;
	//CDialog::OnOK();
}

void CDlgLight::OnBtnExport()
{
	if (m_strPathName.IsEmpty())
	{
		CFileDialog dlg(FALSE, "LightObjs", "Noname",
			OFN_EXPLORER | OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
			"Light Obejct Data File(*.LightObjs)|*.LightObjs||");

		if (dlg.DoModal() == IDOK)
		{
			m_strPathName = dlg.GetPathName();
			m_pRefLightObjMgr->SaveToFile((LPCTSTR) m_strPathName);
			UpdateData(FALSE);
		}
	}
	else
		m_pRefLightObjMgr->SaveToFile((LPCTSTR) m_strPathName);
}

void CDlgLight::OnBtnImport()
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "LightObjs", nullptr, dwFlags,
		"Light Obejct Data File(*.LightObjs)|*.LightObjs||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	m_pRefLightObjMgr->LoadFromFile((LPCTSTR) dlg.GetPathName());

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->Invalidate();
}

void CDlgLight::OnBtnUpload()
{
	UpdateData(TRUE);

	if (m_strLightObjName.IsEmpty())
	{
		MessageBox("Light Object객체의 이름이 지정되어 있지 않습니다.", "-.-;");
		return;
	}

	float fRange = (float) m_sld_Range.GetPos() / 10.0f;

	float fAtten = (100.0f - (float) m_sld_Atten.GetPos()) / 100.0f;

	__ColorValue crLgt;
	crLgt.a = 0.0f;
	crLgt.r = (float) ((m_pLOColor << 24) >> 24) / 255.0f;
	crLgt.g = (float) ((m_pLOColor << 16) >> 24) / 255.0f;
	crLgt.b = (float) ((m_pLOColor << 8) >> 24) / 255.0f;

	int idx = m_ListLO.AddString(m_strLightObjName);
	m_ListLO.SetItemDataPtr(idx, m_pRefLightObjMgr->m_pCurrLO);

	m_pRefLightObjMgr->UpLoad(m_strLightObjName.GetString(), fRange, fAtten, crLgt);

	LPLIGHTOBJ pLO = m_pRefLightObjMgr->m_pCurrLO;

	int Atten      = (int) (pLO->pRefLight->m_Data.Attenuation0 * 100.0f);
	m_sld_Atten.SetPos(Atten);

	int Range = (int) (pLO->pRefLight->m_Data.Range * 10.0f / 4.0f);
	m_sld_Range.SetPos(Range);

	m_pLOColor = 0x00000000;
	DWORD R, G, B;
	R           = (DWORD) (pLO->pRefLight->m_Data.Diffuse.r * 255.0f);
	m_pLOColor += R;
	G           = (DWORD) (pLO->pRefLight->m_Data.Diffuse.g * 255.0f);
	m_pLOColor += (G << 8);
	B           = (DWORD) (pLO->pRefLight->m_Data.Diffuse.b * 255.0f);
	m_pLOColor += (B << 16);

	m_strLightObjName.Format(pLO->szName);

	UpdateData(FALSE);

	m_pRefLightObjMgr->ChangeSelLights();
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->Invalidate();
}

void CDlgLight::OnBtnDownload()
{
	int idx = m_ListLO.GetCurSel();
	if (idx < 0)
		return;
	m_pSelLO = (LPLIGHTOBJ) m_ListLO.GetItemDataPtr(idx);

	m_pRefLightObjMgr->DownLoad();

	m_ListLO.DeleteString(idx);
	idx = m_ListLO.GetCurSel();
	if (idx < 0)
		m_pSelLO = nullptr;
	else
		m_pSelLO = (LPLIGHTOBJ) m_ListLO.GetItemDataPtr(idx);

	LPLIGHTOBJ pLO = m_pRefLightObjMgr->m_pCurrLO;

	int Atten      = (int) (pLO->pRefLight->m_Data.Attenuation0 * 100.0f);
	m_sld_Atten.SetPos(Atten);

	int Range = (int) (pLO->pRefLight->m_Data.Range * 10.0f / 4.0f);
	m_sld_Range.SetPos(Range);

	m_pLOColor = 0x00000000;
	DWORD R, G, B;
	R           = (DWORD) (pLO->pRefLight->m_Data.Diffuse.r * 255.0f);
	m_pLOColor += R;
	G           = (DWORD) (pLO->pRefLight->m_Data.Diffuse.g * 255.0f);
	m_pLOColor += (G << 8);
	B           = (DWORD) (pLO->pRefLight->m_Data.Diffuse.b * 255.0f);
	m_pLOColor += (B << 16);

	m_strLightObjName.Format(pLO->szName);

	UpdateData(FALSE);
	Invalidate();

	m_pRefLightObjMgr->ChangeSelLights();
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->Invalidate();
}

void CDlgLight::OnBtnDelete()
{
	if (!m_pSelLO)
		return;
	m_pRefLightObjMgr->DeleteLO(m_pSelLO);

	int idx = m_ListLO.GetCurSel();
	if (idx < 0)
		return;
	m_ListLO.DeleteString(idx);
	idx = m_ListLO.GetCurSel();
	if (idx < 0)
		m_pSelLO = nullptr;
	else
		m_pSelLO = (LPLIGHTOBJ) m_ListLO.GetItemDataPtr(idx);

	Invalidate();

	m_pRefLightObjMgr->ChangeSelLights();
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->Invalidate();
}

void CDlgLight::OnSelchangeListLights()
{
	int idx = m_ListLO.GetCurSel();
	if (idx < 0)
		return;

	m_pSelLO = (LPLIGHTOBJ) m_ListLO.GetItemDataPtr(idx);

	m_pRefLightObjMgr->ChangeSelLights();

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->Invalidate();
}

void CDlgLight::OnReleasedcaptureSldAtten(NMHDR* pNMHDR, LRESULT* pResult)
{
	RefreshCurrLight();
	*pResult = 0;
}

void CDlgLight::OnReleasedcaptureSldRange(NMHDR* pNMHDR, LRESULT* pResult)
{
	RefreshCurrLight();
	*pResult = 0;
}

void CDlgLight::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CDC* pDC = m_stcColor.GetDC();
	CRect rt;
	m_stcColor.GetClientRect(&rt);
	pDC->FillSolidRect(&rt, m_pLOColor);
	m_stcColor.ReleaseDC(pDC);
}

void CDlgLight::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd *pColorWnd, *pMouseWnd;
	pColorWnd = (CWnd*) &m_stcColor;
	pMouseWnd = ChildWindowFromPoint(point);

	CRect rt;
	if (pMouseWnd == pColorWnd)
	{
		CColorDialog dlg;
		if (dlg.DoModal() == IDOK)
			m_pLOColor = dlg.GetColor();
		Invalidate();

		RefreshCurrLight();
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgLight::RefreshCurrLight()
{
	float fRange = (float) m_sld_Range.GetPos() / 10.0f;
	float fAtten = (100.0f - (float) m_sld_Atten.GetPos()) / 100.0f;
	__ColorValue crLgt;
	crLgt.a = 0.0f;
	crLgt.r = (float) ((m_pLOColor << 24) >> 24) / 255.0f;
	crLgt.g = (float) ((m_pLOColor << 16) >> 24) / 255.0f;
	crLgt.b = (float) ((m_pLOColor << 8) >> 24) / 255.0f;

	m_pRefLightObjMgr->RefreshCurrLights(fRange, fAtten, crLgt);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->Invalidate();
}

void CDlgLight::InsertLOList(LPLIGHTOBJ pLO)
{
	int idx = m_ListLO.AddString(&(pLO->szName[0]));
	m_ListLO.SetItemDataPtr(idx, (LPLIGHTOBJ) pLO);
}

void CDlgLight::Reset()
{
	OnInitDialog();
}
