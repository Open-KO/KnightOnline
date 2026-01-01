// FormViewProperty.cpp : implementation file
//

#include "stdafx.h"
#include "SkyViewer.h"
#include "MainFrm.h"

#include "SkyViewerDoc.h"
#include "SkyViewerView.h"
#include "FormViewProperty.h"

#include <N3Base/N3Texture.h>
#include <N3Base/N3Sun.h>
#include <N3Base/N3Cloud.h>
#include <N3Base/N3SkyMng.h>
#include <N3Base/N3GlobalEffectMng.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty

IMPLEMENT_DYNCREATE(CFormViewProperty, CFormView)

CFormViewProperty::CFormViewProperty()
	: CFormView(CFormViewProperty::IDD)
{
	//{{AFX_DATA_INIT(CFormViewProperty)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bUpdatingNow = FALSE;
	m_bPlayingNow = FALSE;
}

CFormViewProperty::~CFormViewProperty()
{
}

void CFormViewProperty::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormViewProperty)
	DDX_Control(pDX, IDC_SLD_TIME, m_SldTime);
	DDX_Control(pDX, IDC_LIST_CLOUD_TEXTURES, m_ListCloudTextures);
	DDX_Control(pDX, IDC_LIST_SUN_TEXTURES, m_ListSunTextures);
	DDX_Control(pDX, IDC_LIST_DAY_CHANGE, m_ListDayChanges);
	DDX_Control(pDX, IDC_LIST_PROPERTY, m_LPProperty);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFormViewProperty, CFormView)
	//{{AFX_MSG_MAP(CFormViewProperty)
	ON_LBN_SELCHANGE(IDC_LIST_DAY_CHANGE, OnSelchangeListDayChange)
	ON_BN_CLICKED(IDC_B_ADD, OnBAdd)
	ON_BN_CLICKED(IDC_B_INSERT, OnBInsert)
	ON_BN_CLICKED(IDC_B_DELETE, OnBDelete)
	ON_BN_CLICKED(IDC_B_MOVE_UP, OnBMoveUp)
	ON_BN_CLICKED(IDC_B_MOVE_DOWN, OnBMoveDown)
	ON_EN_CHANGE(IDC_E_NAME, OnChangeEName)
	ON_BN_CLICKED(IDC_C_PLAY, OnCPlay)
	ON_WM_TIMER()
	ON_LBN_DBLCLK(IDC_LIST_CLOUD_TEXTURES, OnDblclkListCloudTextures)
	ON_LBN_DBLCLK(IDC_LIST_SUN_TEXTURES, OnDblclkListSunTextures)
	ON_BN_CLICKED(IDC_B_BROWSE_MOON_TEXTURE, OnBBrowseMoonTexture)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_B_RAINY, OnBRainy)
	ON_BN_CLICKED(IDC_B_SNOWS, OnBSnows)
	ON_BN_CLICKED(IDC_B_CLEAN, OnBClean)
	ON_BN_CLICKED(IDC_APPLY_DATE, OnApplyDate)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty diagnostics

#ifdef _DEBUG
void CFormViewProperty::AssertValid() const
{
	CFormView::AssertValid();
}

void CFormViewProperty::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CSkyViewerDoc* CFormViewProperty::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSkyViewerDoc)));
	return (CSkyViewerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty message handlers

void CFormViewProperty::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	CSkyViewerDoc* pDoc = GetDocument();
	m_SldTime.SetRange(0, 24 * 60);
	
//enum		eSKY_DAYCHANGE {	SDC_SKYCOLOR=0,		// 하늘색
//								SDC_FOGCOLOR,		// 안개색
//								SDC_STARCOUNT,		// 별 수
//								SDC_MOONPHASE,		// 달 모양
//								SDC_SUNCOLOR,		// 해 색
//								SDC_GLOWCOLOR,		// 해의 glow색
//								SDC_FLARECOLOR,		// 해의 flare색
//								SDC_CLOUD1COLOR,	// 구름층 1의 색
//								SDC_CLOUD2COLOR,	// 구름층 2의 색
//								SDC_CLOUDTEX,		// 구름 텍스쳐 바꾸기
//								NUM_SKYDAYCHANGE,
//								
//								SDC_UNKNOWN = 0xffffffff
//};

	m_LPProperty.ResetContent();
	m_LPProperty.AddPropItem("이름", "", PIT_EDIT, "");
	m_LPProperty.AddPropItem("종류", "", PIT_COMBO, "하늘색|안개색|별수|달모양|해의 색|해의 Glow 색|해의 Flare 색|구름층1의 색|구름층2의 색|구름텍스처 바꾸기|라이트0|라이트1|라이트2");
	m_LPProperty.AddPropItem("인수1", "", PIT_EDIT, "");
	m_LPProperty.AddPropItem("색1", "", PIT_COLOR, "");
	m_LPProperty.AddPropItem("인수2", "", PIT_EDIT, "");
	m_LPProperty.AddPropItem("색2", "", PIT_COLOR, "");
	m_LPProperty.AddPropItem("시간", "", PIT_EDIT, "0시0분");
	m_LPProperty.AddPropItem("지연시간", "", PIT_EDIT, "0초");

	// 시간을 가져와 세팅..
	SYSTEMTIME Time;
	::GetLocalTime(&Time);
	pDoc->m_Sky.SetGameTime(Time.wYear, Time.wMonth, Time.wDay, Time.wHour, Time.wMinute);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);

	this->UpdateAllInfo();
}

BOOL CFormViewProperty::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	if((void*)wParam == &m_LPProperty)
	{
		CSkyViewerDoc* pDoc = GetDocument();

		int iIndex = m_ListDayChanges.GetCurSel();
		__SKY_DAYCHANGE* pSDC = pDoc->m_Sky.DayChangeGet(iIndex);
		if(nullptr == pSDC) return FALSE;
		CPropertyItem* pItem = (CPropertyItem*)lParam;

		if(pItem->m_propName == "이름") pSDC->szName = pItem->m_curValue;
		else if(pItem->m_propName == "시간")
		{
			int iHour = 0, iMin = 0;
			sscanf(pItem->m_curValue, "%d시%d분", &iHour, &iMin);
			pSDC->dwWhen = CONVERT_SEC(iHour,iMin,0);
		}
		else if(pItem->m_propName == "종류")	pSDC->eSkyDayChange = (eSKY_DAYCHANGE)(pItem->m_crColor);
		else if(pItem->m_propName == "인수1")	pSDC->dwParam1 = atoi(pItem->m_curValue);
		else if(pItem->m_propName == "색1")		pSDC->dwParam1 = ::_RGB_To_D3DCOLOR(pItem->m_crColor, 255u);
		else if(pItem->m_propName == "인수2")	pSDC->dwParam2 = atoi(pItem->m_curValue);
		else if(pItem->m_propName == "색2")		pSDC->dwParam2 = ::_RGB_To_D3DCOLOR(pItem->m_crColor, 255u);
		else if(pItem->m_propName == "지연시간") pSDC->fHowLong = (float)(atof(pItem->m_curValue));

		int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0;
		pDoc->m_Sky.GetGameTime(&iYear, &iMonth, &iDay, &iHour, &iMin);
		pDoc->m_Sky.SetGameTime(iYear, iMonth, iDay, iHour, iMin);
		
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);


		return TRUE;
	}
	
	return CFormView::OnNotify(wParam, lParam, pResult);
}

void CFormViewProperty::OnSelchangeListDayChange() 
{
	this->UpdateInfo();
}

void CFormViewProperty::UpdateTime()
{
	int iYear, iMonth, iDay, iHour = 0, iMin = 0;
	CSkyViewerDoc* pDoc = GetDocument();
	pDoc->m_Sky.GetGameTime(&iYear, &iMonth, &iDay, &iHour, &iMin);
	
	// 시간 업데이트..
	SetDlgItemInt(IDC_E_YEAR, iYear);
	SetDlgItemInt(IDC_E_MONTH, iMonth);
	SetDlgItemInt(IDC_E_DAY, iDay);
	SetDlgItemInt(IDC_E_HOUR, iHour);
	SetDlgItemInt(IDC_E_MIN, iMin);
}

void CFormViewProperty::UpdateInfo()
{
	if(TRUE == m_bUpdatingNow) return;
	m_bUpdatingNow = TRUE;

	CSkyViewerDoc* pDoc = GetDocument();

	int iIndex = m_ListDayChanges.GetCurSel();
	__SKY_DAYCHANGE* pSDC = pDoc->m_Sky.DayChangeGet(iIndex);

	CPropertyItem* pItem0 = m_LPProperty.GetPropItem("이름");
	CPropertyItem* pItem1 = m_LPProperty.GetPropItem("시간");
	CPropertyItem* pItem2 = m_LPProperty.GetPropItem("종류");
	
	CPropertyItem* pItem3 = m_LPProperty.GetPropItem("인수1");
	CPropertyItem* pItem4 = m_LPProperty.GetPropItem("색1");
	CPropertyItem* pItem5 = m_LPProperty.GetPropItem("인수2");
	CPropertyItem* pItem6 = m_LPProperty.GetPropItem("색2");
	
	CPropertyItem* pItem7 = m_LPProperty.GetPropItem("지연시간");

	if(nullptr == pSDC)
	{
		SetDlgItemInt(IDC_E_NUMBER, iIndex);
		SetDlgItemText(IDC_E_NAME, "");

		if(pItem0) pItem0->m_curValue = "";
		if(pItem1) pItem1->m_curValue = "0시0분";
		if(pItem2) pItem2->m_crColor = 0;
		
		if(pItem3) pItem3->m_curValue = "";
		if(pItem4) pItem4->m_crColor = 0;
		if(pItem5) pItem5->m_curValue = "";
		if(pItem6) pItem6->m_crColor = 0;
		
		if(pItem7) pItem7->m_curValue = "";
	}
	else
	{
		SetDlgItemInt(IDC_E_NUMBER, iIndex);
		SetDlgItemText(IDC_E_NAME, pSDC->szName.c_str());

		if(pItem0) pItem0->m_curValue = pSDC->szName.c_str();
		if(pItem1)
		{
			int iHour = pSDC->dwWhen / (3600), iMin = (pSDC->dwWhen % (3600))/60;
			pItem1->m_curValue.Format("%d시%d분", iHour, iMin);
		}
		if(pItem2) pItem2->m_crColor = pSDC->eSkyDayChange;
		
		if(pItem3) pItem3->m_curValue.Format("%d", pSDC->dwParam1);
		if(pItem4) pItem4->m_crColor = ::_D3DCOLOR_To_RGB(pSDC->dwParam1);
		if(pItem5) pItem5->m_curValue.Format("%d", pSDC->dwParam2);
		if(pItem6) pItem6->m_crColor = ::_D3DCOLOR_To_RGB(pSDC->dwParam2);
		
		if(pItem7) pItem7->m_curValue.Format("%.2f", pSDC->fHowLong);
	}

	this->UpdateTime();

	m_LPProperty.Invalidate();
	m_bUpdatingNow = FALSE;
}

void CFormViewProperty::UpdateAllInfo()
{
	CSkyViewerDoc* pDoc = GetDocument();

	if(pDoc->m_Sky.MoonTextureGet()) SetDlgItemText(IDC_E_MOON_TEXTURE, pDoc->m_Sky.MoonTextureGet()->FileName().c_str());
	else SetDlgItemText(IDC_E_MOON_TEXTURE, "");

	int i = 0;

	m_ListSunTextures.ResetContent();
	for(i = 0; i < NUM_SUNPART; i++)
	{
		if(pDoc->m_Sky.SunTextureGet(i)) m_ListSunTextures.AddString(pDoc->m_Sky.SunTextureGet(i)->FileName().c_str());
		else m_ListSunTextures.AddString("");
	}
	
	m_ListCloudTextures.ResetContent();
	for(i = 0; i < NUM_CLOUD; i++)
	{
		m_ListCloudTextures.AddString(pDoc->m_Sky.CloudTextureFileName(i));
	}

	int iIndexPrev = m_ListDayChanges.GetCurSel();
	if(iIndexPrev < 0) iIndexPrev = 0;

	m_ListDayChanges.ResetContent();
	int iSDCC = pDoc->m_Sky.DayChangeCount();
	for(i = 0; i < iSDCC; i++)
	{
		__SKY_DAYCHANGE* pSDC = pDoc->m_Sky.DayChangeGet(i);
		m_ListDayChanges.AddString(pSDC->szName.c_str());
	}

	this->UpdateInfo();
}

void CFormViewProperty::OnBAdd() 
{
	CSkyViewerDoc* pDoc = GetDocument();
	pDoc->m_Sky.DayChangeAdd();
	this->UpdateAllInfo();
}

void CFormViewProperty::OnBInsert() 
{
	int iIndex = m_ListDayChanges.GetCurSel();

	CSkyViewerDoc* pDoc = GetDocument();
	pDoc->m_Sky.DayChangeInsert(iIndex);
	this->UpdateAllInfo();
}

void CFormViewProperty::OnBDelete() 
{
	int iIndex = m_ListDayChanges.GetCurSel();

	CSkyViewerDoc* pDoc = GetDocument();
	pDoc->m_Sky.DayChangeDelete(iIndex);
	this->UpdateAllInfo();
}

void CFormViewProperty::OnBMoveUp() 
{
}

void CFormViewProperty::OnBMoveDown() 
{
	// TODO: Add your control notification handler code here
	
}

void CFormViewProperty::OnChangeEName() 
{
	if(m_bUpdatingNow) return;


	int iIndex = m_ListDayChanges.GetCurSel();
	CSkyViewerDoc* pDoc = GetDocument();
	__SKY_DAYCHANGE* pSDC = pDoc->m_Sky.DayChangeGet(iIndex);
	if(nullptr == pSDC) return;

	CString szName;
	GetDlgItemText(IDC_E_NAME, szName);
	pSDC->szName = szName;

	m_ListDayChanges.DeleteString(iIndex);
	m_ListDayChanges.InsertString(iIndex, szName);
	m_ListDayChanges.SetCurSel(iIndex);
}

void CFormViewProperty::OnCPlay() 
{
	m_bPlayingNow = !m_bPlayingNow;
	if(m_bPlayingNow) SetTimer(0, 20, nullptr);
	else
	{
		SetDlgItemText(IDC_C_PLAY, "Play");
		KillTimer(0);
	}
}

void CFormViewProperty::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 0) // Play..
	{
		this->UpdateTime();

		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);
	}
	
	CFormView::OnTimer(nIDEvent);
}

void CFormViewProperty::OnDblclkListCloudTextures() 
{
	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(TRUE, "DXT", nullptr, dwFlags, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;
	
	int iIndex = m_ListCloudTextures.GetCurSel();

	FileName = dlg.GetPathName();
	CSkyViewerDoc* pDoc = GetDocument();
	CN3Texture* pTex = pDoc->m_Sky.CloudTextureSet(iIndex, FileName);

	m_ListCloudTextures.DeleteString(iIndex);
	m_ListCloudTextures.InsertString(iIndex, pTex->FileName().c_str());
	m_ListCloudTextures.SetCurSel(iIndex);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewProperty::OnDblclkListSunTextures() 
{
	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(TRUE, "DXT", nullptr, dwFlags, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;
	
	int iIndex = m_ListSunTextures.GetCurSel();

	FileName = dlg.GetPathName();
	CSkyViewerDoc* pDoc = GetDocument();
	CN3Texture* pTex = pDoc->m_Sky.SunTextureSet(iIndex, FileName);

	m_ListSunTextures.DeleteString(iIndex);
	m_ListSunTextures.InsertString(iIndex, pTex->FileName().c_str());
	m_ListSunTextures.SetCurSel(iIndex);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewProperty::OnBBrowseMoonTexture() 
{
	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(TRUE, "DXT", nullptr, dwFlags, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;
	
	FileName = dlg.GetPathName();
	CSkyViewerDoc* pDoc = GetDocument();
	CN3Texture* pTex = pDoc->m_Sky.MoonTextureSet(FileName);

	SetDlgItemText(IDC_E_MOON_TEXTURE, pTex->FileName().c_str());

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewProperty::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == (CScrollBar*)(&(m_SldTime)))
	{
		int iHour = m_SldTime.GetPos() / 60;
		int iMin = m_SldTime.GetPos() % 60;
		int iYear = 0, iMonth = 0, iDay = 0;

		CSkyViewerDoc* pDoc = GetDocument();
		pDoc->m_Sky.GetGameTime(&iYear, &iMonth, &iDay, nullptr, nullptr);
		pDoc->m_Sky.SetGameTime(iYear, iMonth, iDay, iHour, iMin);

		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);

		this->UpdateTime();
	}
	
	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CFormViewProperty::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	this->UpdateAllInfo();
}

void CFormViewProperty::OnBRainy() 
{
	CSkyViewerDoc* pDoc = GetDocument();
	
	int iPercentage = GetDlgItemInt(IDC_E_PERCENTAGE);
	if(iPercentage < 0) iPercentage = 0;
	if(iPercentage > 100) iPercentage = 100;
	pDoc->m_Sky.SetWeather(CN3SkyMng::SW_CLEAR, 0);
	pDoc->m_Sky.SetWeather(CN3SkyMng::SW_RAINY, iPercentage);
	pDoc->m_Weather.WeatherSetRainy(iPercentage);
}

void CFormViewProperty::OnBSnows() 
{
	CSkyViewerDoc* pDoc = GetDocument();
	
	int iPercentage = GetDlgItemInt(IDC_E_PERCENTAGE);
	if(iPercentage < 0) iPercentage = 0;
	if(iPercentage > 100) iPercentage = 100;
	pDoc->m_Sky.SetWeather(CN3SkyMng::SW_CLEAR, 0);
	pDoc->m_Sky.SetWeather(CN3SkyMng::SW_SNOW, iPercentage);
	pDoc->m_Weather.WeatherSetSnow(iPercentage);
}

void CFormViewProperty::OnBClean() 
{
	CSkyViewerDoc* pDoc = GetDocument();
	
	int iPercentage = GetDlgItemInt(IDC_E_PERCENTAGE);
	if(iPercentage < 0) iPercentage = 0;
	if(iPercentage > 100) iPercentage = 100;
	pDoc->m_Sky.SetWeather(CN3SkyMng::SW_RAINY, 0);
	pDoc->m_Sky.SetWeather(CN3SkyMng::SW_CLEAR, iPercentage);
	pDoc->m_Weather.WeatherSetClean();
}

void CFormViewProperty::OnApplyDate() 
{
	int iYear = GetDlgItemInt(IDC_E_YEAR);
	int iMonth = GetDlgItemInt(IDC_E_MONTH);
	int iDay = GetDlgItemInt(IDC_E_DAY);
	int iHour = GetDlgItemInt(IDC_E_HOUR);;
	int iMin = GetDlgItemInt(IDC_E_MIN);;

	CSkyViewerDoc* pDoc = GetDocument();
	pDoc->m_Sky.SetGameTime(iYear, iMonth, iDay, iHour, iMin);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetViewRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewProperty::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	if(nullptr == m_LPProperty.GetSafeHwnd()) return;

	CRect rc, rc2, rcLP;

	m_LPProperty.GetWindowRect(&rc);
	rcLP = rc; this->ScreenToClient(&rcLP);
	int y = cy - (rcLP.Height() + 5);
	m_LPProperty.SetWindowPos(nullptr, rcLP.left, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	m_ListDayChanges.GetWindowRect(&rc);
	rc2 = rc; this->ScreenToClient(&rc2);
	int cy2 = cy - rc2.top - rcLP.Height() - 3;
	m_ListDayChanges.SetWindowPos(nullptr, 0, 0, rc.Width(), cy2, SWP_NOMOVE | SWP_NOZORDER);

	m_SldTime.GetWindowRect(&rc);
	rc2 = rc; this->ScreenToClient(&rc2);
	cy2 = cy - rc2.top - 5;
	m_SldTime.SetWindowPos(nullptr, 0, 0, rc.Width(), cy2, SWP_NOMOVE | SWP_NOZORDER);
}
