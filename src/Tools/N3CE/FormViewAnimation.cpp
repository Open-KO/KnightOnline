// FormViewAnimation.cpp : implementation file
//

#include "stdafx.h"
#include "n3ce.h"
#include "N3CEDoc.h"
#include "N3CEView.h"
#include "MainFrm.h"
#include "FormViewAnimation.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

const float FRAME_PRECISION = 10.0f; // 프레임 표시 정밀도

/////////////////////////////////////////////////////////////////////////////
// CFormViewAnimation

IMPLEMENT_DYNCREATE(CFormViewAnimation, CFormView)

CFormViewAnimation::CFormViewAnimation() : CFormView(CFormViewAnimation::IDD)
{
	//{{AFX_DATA_INIT(CFormViewAnimation)
	//}}AFX_DATA_INIT

	m_bUpdatingNow = FALSE;
}

CFormViewAnimation::~CFormViewAnimation()
{
}

void CFormViewAnimation::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormViewAnimation)
	DDX_Control(pDX, IDC_SLD_SCENE_FRAME, m_SldSceneFrm);
	DDX_Control(pDX, IDC_SLIDER_FRM, m_SldFrm);
	DDX_Control(pDX, IDC_C_PLAY, m_CPlay);
	DDX_Control(pDX, IDC_LIST_ANIMATION0, m_ListAnim0);
	DDX_Control(pDX, IDC_LIST_ANIMATION1, m_ListAnim1);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFormViewAnimation, CFormView)
//{{AFX_MSG_MAP(CFormViewAnimation)
ON_BN_CLICKED(IDC_B_SET_FRAME_START, OnBSetFrameStart)
ON_BN_CLICKED(IDC_B_SET_FRAME_END, OnBSetFrameEnd)
ON_BN_CLICKED(IDC_B_SET_FRAME_PLUG_TRACE_START, OnBSetFramePlugTraceStart)
ON_BN_CLICKED(IDC_B_SET_FRAME_PLUG_TRACE_END, OnBSetFramePlugTraceEnd)
ON_BN_CLICKED(IDC_B_SET_FRAME_SOUND0, OnBSetFrameSound0)
ON_BN_CLICKED(IDC_B_SET_FRAME_SOUND1, OnBSetFrameSound1)
ON_BN_CLICKED(IDC_C_PLAY, OnCPlay)
ON_LBN_SELCHANGE(IDC_LIST_ANIMATION0, OnSelchangeListAnimation0)
ON_LBN_SELCHANGE(IDC_LIST_ANIMATION1, OnSelchangeListAnimation1)
ON_BN_CLICKED(IDC_B_ADD, OnBAdd)
ON_BN_CLICKED(IDC_B_INSERT, OnBInsert)
ON_BN_CLICKED(IDC_B_DELETE, OnBDelete)
ON_BN_CLICKED(IDC_B_LOAD, OnBLoad)
ON_BN_CLICKED(IDC_B_SAVE, OnBSave)
ON_BN_CLICKED(IDC_B_MOVE_UP, OnBMoveUp)
ON_BN_CLICKED(IDC_B_MOVE_DOWN, OnBMoveDown)
ON_EN_CHANGE(IDC_E_SCENE_FRM_START, OnChangeESceneFrmStart)
ON_EN_CHANGE(IDC_E_SCENE_FRM_END, OnChangeESceneFrmEnd)
ON_EN_CHANGE(IDC_E_ANI_NAME, OnChangeEAniName)
ON_EN_CHANGE(IDC_E_FRAME_START, OnChangeEFrameStart)
ON_EN_CHANGE(IDC_E_FRAME_END, OnChangeEFrameEnd)
ON_EN_CHANGE(IDC_E_FRAME_PER_SEC, OnChangeEFramePerSec)
ON_EN_CHANGE(IDC_E_FRAME_PLUG_TRACE_START, OnChangeEFramePlugTraceStart)
ON_EN_CHANGE(IDC_E_FRAME_PLUG_TRACE_END, OnChangeEFramePlugTraceEnd)
ON_EN_CHANGE(IDC_E_FRAME_SOUND0, OnChangeEFrameSound0)
ON_EN_CHANGE(IDC_E_FRAME_SOUND1, OnChangeEFrameSound1)
ON_EN_CHANGE(IDC_E_FRAME_BLEND0, OnChangeEFrameBlend0)
ON_BN_CLICKED(IDC_C_DELAYED_LOOPING, OnCDelayedLooping)
ON_WM_HSCROLL()
ON_WM_SIZE()
ON_WM_TIMER()
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_B_SET_FRAME_STRIKE0, OnBSetFrameStrike0)
ON_BN_CLICKED(IDC_B_SET_FRAME_STRIKE1, OnBSetFrameStrike1)
ON_EN_CHANGE(IDC_E_FRAME_STRIKE0, OnChangeEFrameStrike0)
ON_EN_CHANGE(IDC_E_FRAME_STRIKE1, OnChangeEFrameStrike1)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_ADD, OnEditAnimationDataAdd)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_INSERT, OnEditAnimationDataInsert)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_DELETE, OnEditAnimationDataDelete)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_COPY, OnEditAnimationDataCopy)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_PASTE, OnEditAnimationDataPaste)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_MOVE_UP, OnEditAnimationDataMoveUp)
ON_COMMAND(ID_EDIT_ANIMATION_DATA_MOVE_DOWN, OnEditAnimationDataMoveDown)
ON_LBN_DBLCLK(IDC_LIST_ANIMATION0, OnDblclkListAnimation0)
ON_BN_CLICKED(IDC_B_NEW, OnBNew)
ON_BN_CLICKED(IDC_B_SAVE_AS, OnBSaveAs)
ON_BN_CLICKED(
	IDC_B_CALCULATE_DELAY_TIME_WITH_UPPER_ANIMATION, OnBCalculateDelayTimeWithUpperAnimation)
ON_WM_PAINT()
ON_WM_MOUSEMOVE()
ON_BN_CLICKED(
	IDC_B_CALCULATE_DELAY_TIME_WITH_LOWER_ANIMATION, OnBCalculateDelayTimeWithLowerAnimation)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormViewAnimation diagnostics

#ifdef _DEBUG
void CFormViewAnimation::AssertValid() const
{
	CFormView::AssertValid();
}

void CFormViewAnimation::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CN3CEDoc* CFormViewAnimation::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3CEDoc)));
	return (CN3CEDoc*) m_pDocument;
}
#endif //_DEBUG

void CFormViewAnimation::OnBAdd()
{
	this->OnEditAnimationDataAdd();
}

void CFormViewAnimation::OnBInsert()
{
	this->OnEditAnimationDataInsert();
}

void CFormViewAnimation::OnBDelete()
{
	this->OnEditAnimationDataDelete();
}

void CFormViewAnimation::OnBSetFrameStart()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni             = m_ListAnim0.GetCurSel();
	__AnimData* pAniData = pAniCtrl->DataGet(iAni);
	if (nullptr == pAniData)
		return;

	pAniData->fFrmStart = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnBSetFrameEnd()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni             = m_ListAnim0.GetCurSel();
	__AnimData* pAniData = pAniCtrl->DataGet(iAni);
	if (nullptr == pAniData)
		return;

	pAniData->fFrmEnd = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::UpdateFrameSliderAndOther()
{
	CN3Chr* pChr         = GetDocument()->m_Scene.ChrGet(0);
	int iAni             = pChr->AniIndexCur();
	__AnimData* pAniData = pChr->AniDataCur();
	if (pAniData) // 프레임 슬라이더 업데이트..
	{
		float fFrmOld = m_SldFrm.GetCurrentFrame();
		m_SldFrm.SetAnimationData(*pAniData);
		m_SldFrm.SetCurrentFrame(fFrmOld);

		if (pAniData->fFrmPerSec > 0)
		{
			float fTime = (pAniData->fFrmEnd - pAniData->fFrmStart) / pAniData->fFrmPerSec;
			//			if(1 == pAniData->iBlendFlags) fTime += pAniData->fTimeBlend;
			CString szTime;
			szTime.Format("시간 : %.2f초", fTime);
			SetDlgItemText(IDC_STATIC_ANIMATION_TIME_SUM, szTime);
		}
		else
		{
			SetDlgItemText(IDC_STATIC_ANIMATION_TIME_SUM, "시간 : ??초");
		}
	}
	else
	{
		SetDlgItemText(IDC_STATIC_ANIMATION_TIME_SUM, "시간 : ??초");
	}
}

void CFormViewAnimation::UpdateInfo()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;
	if (TRUE == m_bUpdatingNow)
		return;

	m_bUpdatingNow = TRUE;

	int iAni       = m_ListAnim0.GetCurSel();
	float fFrmCur  = GetDocument()->m_Scene.m_fFrmCur;
	SetDlgItemInt(IDC_E_NUMBER, iAni);

	__AnimData* pAniData = pAniCtrl->DataGet(iAni);

	CString szTmp        = "";
	if (pAniData)
	{
		SetDlgItemText(IDC_E_ANI_NAME, pAniData->szName.c_str());

		szTmp.Format("%.1f", pAniData->fFrmStart);
		SetDlgItemText(IDC_E_FRAME_START, szTmp);
		szTmp.Format("%.1f", pAniData->fFrmEnd);
		SetDlgItemText(IDC_E_FRAME_END, szTmp);
		szTmp.Format("%.1f", pAniData->fFrmPerSec);
		SetDlgItemText(IDC_E_FRAME_PER_SEC, szTmp);

		szTmp.Format("%.1f", pAniData->fFrmPlugTraceStart);
		SetDlgItemText(IDC_E_FRAME_PLUG_TRACE_START, szTmp);
		szTmp.Format("%.1f", pAniData->fFrmPlugTraceEnd);
		SetDlgItemText(IDC_E_FRAME_PLUG_TRACE_END, szTmp);

		szTmp.Format("%.1f", pAniData->fFrmSound0);
		SetDlgItemText(IDC_E_FRAME_SOUND0, szTmp);
		szTmp.Format("%.1f", pAniData->fFrmSound1);
		SetDlgItemText(IDC_E_FRAME_SOUND1, szTmp);

		szTmp.Format("%.2f", pAniData->fTimeBlend);
		SetDlgItemText(IDC_E_FRAME_BLEND0, szTmp);

		CheckDlgButton(IDC_C_DELAYED_LOOPING,
			pAniData->iBlendFlags); // 루핑할때 블렌딩 시간만큼 지연시키면서 루핑한다..

		szTmp.Format("%.2f", pAniData->fFrmStrike0);
		SetDlgItemText(IDC_E_FRAME_STRIKE0, szTmp);
		szTmp.Format("%.2f", pAniData->fFrmStrike1);
		SetDlgItemText(IDC_E_FRAME_STRIKE1, szTmp);

		float fFrmOld = m_SldFrm.GetCurrentFrame();
		szTmp.Format("%f", fFrmCur);
		SetDlgItemText(IDC_E_FRAME_CUR, szTmp);
		m_SldFrm.SetAnimationData(*pAniData);
		m_SldFrm.SetCurrentFrame(fFrmOld);

		if (pAniData->fFrmPerSec > 0)
		{
			float fTime = (pAniData->fFrmEnd - pAniData->fFrmStart) / pAniData->fFrmPerSec;
			//			if(1 == pAniData->iBlendFlags) fTime += pAniData->fTimeBlend;
			CString szTime;
			szTime.Format("시간 : %.2f초", fTime);
			SetDlgItemText(IDC_STATIC_ANIMATION_TIME_SUM, szTime);
		}
	}
	else
	{
		SetDlgItemText(IDC_E_ANI_NAME, "NULL!!!!");
		SetDlgItemText(IDC_E_FRAME_START, szTmp);
		SetDlgItemText(IDC_E_FRAME_END, szTmp);
		SetDlgItemText(IDC_E_FRAME_PER_SEC, szTmp);

		SetDlgItemText(IDC_STATIC_ANIMATION_TIME_SUM, "시간 : ??초");
	}

	m_bUpdatingNow = FALSE;
}

void CFormViewAnimation::UpdateAllInfo()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;
	if (m_ListAnim0.GetSafeHwnd() == nullptr)
		return;

	int iCount    = pAniCtrl->Count();

	int nPrevSel0 = m_ListAnim0.GetCurSel();
	int nPrevSel1 = m_ListAnim1.GetCurSel();

	if (nPrevSel0 < 0)
		nPrevSel0 = 0;
	else if (nPrevSel0 >= iCount)
		nPrevSel0 = iCount - 1;
	if (nPrevSel1 < 0)
		nPrevSel1 = 0;
	else if (nPrevSel1 >= iCount)
		nPrevSel1 = iCount - 1;

	m_ListAnim0.ResetContent();
	m_ListAnim1.ResetContent();
	for (int i = 0; i < iCount; i++)
	{
		CString szTmp = pAniCtrl->DataGet(i)->szName.c_str();
		m_ListAnim0.AddString(szTmp);
		m_ListAnim1.AddString(szTmp);
	}
	m_ListAnim0.SetCurSel(nPrevSel0);
	m_ListAnim1.SetCurSel(nPrevSel1);

	float fFrmS = GetDocument()->m_Scene.m_fFrmStart;
	float fFrmE = GetDocument()->m_Scene.m_fFrmEnd;
	SetDlgItemInt(IDC_E_SCENE_FRM_START, (int) fFrmS);
	SetDlgItemInt(IDC_E_SCENE_FRM_END, (int) fFrmE);
	m_SldSceneFrm.SetRange((int) (fFrmS * FRAME_PRECISION), (int) (fFrmE * FRAME_PRECISION));

	SetDlgItemText(IDC_E_ANI_FILE_NAME, pAniCtrl->FileName().c_str());

	this->UpdateInfo();
}

void CFormViewAnimation::GetData()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;
	if (TRUE == m_bUpdatingNow)
		return;

	int iAni             = m_ListAnim0.GetCurSel();
	__AnimData* pAniData = pAniCtrl->DataGet(iAni);
	if (nullptr == pAniData)
		return;

	CString szTmp = "";
	GetDlgItemText(IDC_E_ANI_NAME, szTmp);
	pAniData->szName = szTmp;
	GetDlgItemText(IDC_E_FRAME_START, szTmp);
	pAniData->fFrmStart = (float) atof(szTmp);
	GetDlgItemText(IDC_E_FRAME_END, szTmp);
	pAniData->fFrmEnd = (float) atof(szTmp);
	GetDlgItemText(IDC_E_FRAME_PER_SEC, szTmp);
	pAniData->fFrmPerSec = (float) atof(szTmp);

	GetDlgItemText(IDC_E_FRAME_PLUG_TRACE_START, szTmp);
	pAniData->fFrmPlugTraceStart = (float) atof(szTmp);
	GetDlgItemText(IDC_E_FRAME_PLUG_TRACE_END, szTmp);
	pAniData->fFrmPlugTraceEnd = (float) atof(szTmp);

	GetDlgItemText(IDC_E_FRAME_SOUND0, szTmp);
	pAniData->fFrmSound0 = (float) atof(szTmp);
	GetDlgItemText(IDC_E_FRAME_SOUND1, szTmp);
	pAniData->fFrmSound1 = (float) atof(szTmp);

	GetDlgItemText(IDC_E_FRAME_BLEND0, szTmp);
	pAniData->fTimeBlend = (float) atof(szTmp);

	GetDlgItemText(IDC_E_FRAME_STRIKE0, szTmp);
	pAniData->fFrmStrike0 = (float) atof(szTmp);
	GetDlgItemText(IDC_E_FRAME_STRIKE1, szTmp);
	pAniData->fFrmStrike1 = (float) atof(szTmp);

	if (IsDlgButtonChecked(IDC_C_DELAYED_LOOPING))
		pAniData->iBlendFlags = 1; // 루핑할때 블렌딩 시간만큼 지연시키면서 루핑한다..
	else
		pAniData->iBlendFlags = 0;
}

void CFormViewAnimation::OnOK()
{
	this->GetData();
}

void CFormViewAnimation::OnCPlay()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->GetPaneRender()->m_DequeAnimation.clear();

	if (m_CPlay.GetCheck())
	{
		CN3Chr* pChr                         = GetDocument()->m_Scene.ChrGet(0);
		pFrm->GetPaneRender()->m_bPlayingNow = true;

		SetTimer(0, 10, nullptr);
	}
	else
	{
		pFrm->GetPaneRender()->m_bPlayingNow = false;

		KillTimer(0);
	}
}

void CFormViewAnimation::OnTimer(UINT_PTR nIDEvent)
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);

	if (0 == nIDEvent && pChr)
	{
		float fFrm                       = pChr->FrmCur();
		GetDocument()->m_Scene.m_fFrmCur = fFrm;

		m_SldFrm.SetCurrentFrame(fFrm);

		CString szFrm;
		szFrm.Format("%5.2f", fFrm);
		SetDlgItemText(IDC_E_FRAME_CUR, szFrm);

		CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
		pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE); // 렌더링..
	}

	CFormView::OnTimer(nIDEvent);
}

void CFormViewAnimation::OnChangeEFrameStart()
{
	CWnd* pWnd = GetDlgItem(IDC_E_FRAME_START);

	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFrameEnd()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFramePerSec()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnClose()
{
	CFormView::OnClose();
	this->DestroyWindow();
}

void CFormViewAnimation::OnSelchangeListAnimation0()
{
	this->UpdateInfo();

	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni             = this->m_ListAnim0.GetCurSel();
	__AnimData* pAniData = pAniCtrl->DataGet(iAni);
	if (nullptr == pAniData)
		return;

	// Playe 중에는 큐에 쌓인다..
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEView* pView = pFrm->GetPaneRender();
	if (pView->m_bPlayingNow)
	{
		pView->m_DequeAnimation.push_back(iAni);
	}
	else
	{
		pChr->AniCurSet(iAni); // 에니메이션 세팅..
	}

	::SetFocus(GetDlgItem(IDC_E_ANI_NAME)->m_hWnd);

	GetDocument()->m_Scene.m_fFrmCur = pAniData->fFrmStart; // 프레임 맞추고..
	pView->InvalidateRect(nullptr, FALSE);
}

void CFormViewAnimation::OnSelchangeListAnimation1()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	int iAni = this->m_ListAnim1.GetCurSel();
	CString szBlend;
	pChr->AniUpperSet(iAni);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewAnimation::OnChangeEAniName()
{
	int nSel = m_ListAnim0.GetCurSel();
	if (nSel < 0)
		return;

	this->GetData();

	CString str;
	GetDlgItemText(IDC_E_ANI_NAME, str);

	m_ListAnim0.DeleteString(nSel);
	m_ListAnim0.InsertString(nSel, str);
	m_ListAnim0.SetCurSel(nSel);

	m_ListAnim1.DeleteString(nSel);
	m_ListAnim1.InsertString(nSel, str);
	m_ListAnim1.SetCurSel(nSel);
}

void CFormViewAnimation::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	float fFrm = GetDocument()->m_Scene.m_fFrmCur;
	if ((void*) pScrollBar == (void*) &m_SldFrm)
	{
		fFrm = m_SldFrm.GetCurrentFrame();
		m_SldSceneFrm.SetPos(fFrm * FRAME_PRECISION); // Scene 슬라이더도 움직여준다.
	}
	else if ((void*) pScrollBar == (void*) &m_SldSceneFrm)
	{
		fFrm = m_SldSceneFrm.GetPos() / FRAME_PRECISION;
	}

	CString szTmp;
	szTmp.Format("%f", fFrm);
	SetDlgItemText(IDC_E_FRAME_CUR, szTmp);

	GetDocument()->m_Scene.m_fFrmCur = fFrm;
	CMainFrame* pFrm                 = (CMainFrame*) AfxGetMainWnd();
	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFormViewAnimation::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (m_SldSceneFrm.GetSafeHwnd() != nullptr)
	{
		CRect rcOrg, rcClient;
		m_SldSceneFrm.GetWindowRect(rcOrg); // 그래프 컨트롤의 원래 화면 좌표를
		this->GetClientRect(rcClient);      //
		this->ClientToScreen(rcClient);     // 화면 좌표로 바꾸고..

		int cx2 = rcClient.right - rcOrg.left - 5;
		int cy2 = rcOrg.Height();

		m_SldSceneFrm.SetWindowPos(nullptr, 0, 0, cx2, cy2, SWP_NOMOVE | SWP_NOZORDER);
	}

	//	if(m_SldFrm.GetSafeHwnd() != nullptr)
	//	{
	//		CRect rcOrg, rcClient;
	//		m_SldFrm.GetWindowRect(rcOrg); // 그래프 컨트롤의 원래 화면 좌표를
	//		this->GetClientRect(rcClient); //
	//		this->ClientToScreen(rcClient); // 화면 좌표로 바꾸고..
	//
	//		int cx2 = rcClient.right - rcOrg.left - 5;
	//		int cy2 = rcOrg.Height();
	//
	//		m_SldFrm.SetWindowPos(nullptr, 0, 0, cx2, cy2, SWP_NOMOVE | SWP_NOZORDER);
	//	}

	if (m_ListAnim0.GetSafeHwnd() != nullptr)
	{
		CRect rcOrg, rcClient;
		m_ListAnim0.GetWindowRect(rcOrg); // 그래프 컨트롤의 원래 화면 좌표를
		this->GetClientRect(rcClient);    //
		this->ClientToScreen(rcClient);   // 화면 좌표로 바꾸고..

		int cx2 = rcOrg.Width();
		int cy2 = rcClient.bottom - rcOrg.top - 5;

		m_ListAnim0.SetWindowPos(nullptr, 0, 0, cx2, cy2, SWP_NOMOVE | SWP_NOZORDER);
	}

	if (m_ListAnim1.GetSafeHwnd() != nullptr)
	{
		CRect rcOrg, rcClient;
		m_ListAnim1.GetWindowRect(rcOrg); // 그래프 컨트롤의 원래 화면 좌표를
		this->GetClientRect(rcClient);    //
		this->ClientToScreen(rcClient);   // 화면 좌표로 바꾸고..

		int cx2 = rcOrg.Width();
		int cy2 = rcClient.bottom - rcOrg.top - 5;

		m_ListAnim1.SetWindowPos(nullptr, 0, 0, cx2, cy2, SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CFormViewAnimation::OnBNew()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	pAniCtrl->Release();
	this->UpdateAllInfo();
}

void CFormViewAnimation::OnBLoad()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	CFileDialog dlg(
		TRUE, "N3Anim", nullptr, dwFlags, "Animation Data(*.N3Anim)|*.n3Anim||", nullptr);
	char szCurPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szCurPath);
	dlg.m_ofn.lpstrInitialDir = szCurPath;
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName = dlg.GetPathName();

	pAniCtrl->LoadFromFile(std::string(FileName));

	this->UpdateAllInfo();
}

void CFormViewAnimation::OnBSave()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	pAniCtrl->SaveToFile();
}

void CFormViewAnimation::OnBSaveAs()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	CFileDialog dlg(
		FALSE, "N3Anim", nullptr, dwFlags, "Animation Data(*.N3Anim)|*.n3Anim||", nullptr);
	char szCurPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szCurPath);
	dlg.m_ofn.lpstrInitialDir = szCurPath;
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName = dlg.GetPathName();

	pAniCtrl->SaveToFile(std::string(FileName));

	this->UpdateAllInfo();
}

void CFormViewAnimation::OnChangeESceneFrmStart()
{
	GetDocument()->m_Scene.m_fFrmStart = (float) (GetDlgItemInt(IDC_E_SCENE_FRM_START));
	m_SldSceneFrm.SetRange((int) (GetDocument()->m_Scene.m_fFrmStart * FRAME_PRECISION),
		(int) (GetDocument()->m_Scene.m_fFrmEnd * FRAME_PRECISION));
}

void CFormViewAnimation::OnChangeESceneFrmEnd()
{
	GetDocument()->m_Scene.m_fFrmEnd = (float) (GetDlgItemInt(IDC_E_SCENE_FRM_END));
	m_SldSceneFrm.SetRange((int) (GetDocument()->m_Scene.m_fFrmStart * FRAME_PRECISION),
		(int) (GetDocument()->m_Scene.m_fFrmEnd * FRAME_PRECISION));
}

void CFormViewAnimation::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	this->UpdateAllInfo();

	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (pChr && m_ListAnim0.GetSafeHwnd())
	{
		int iAni = m_ListAnim0.GetCurSel();
		pChr->AniCurSet(iAni); // 에니메이션 세팅..
	}
}

void CFormViewAnimation::OnBSetFramePlugTraceStart()
{
	__AnimData* pAniData = this->GetCurAniData();
	if (nullptr == pAniData)
		return;

	pAniData->fFrmPlugTraceStart = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnBSetFramePlugTraceEnd()
{
	__AnimData* pAniData = this->GetCurAniData();
	if (nullptr == pAniData)
		return;

	pAniData->fFrmPlugTraceEnd = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnBSetFrameSound0()
{
	__AnimData* pAniData = this->GetCurAniData();
	if (nullptr == pAniData)
		return;

	pAniData->fFrmSound0 = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnBSetFrameSound1()
{
	__AnimData* pAniData = this->GetCurAniData();
	if (nullptr == pAniData)
		return;

	pAniData->fFrmSound1 = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnBSetFrameStrike0()
{
	__AnimData* pAniData = this->GetCurAniData();
	if (nullptr == pAniData)
		return;

	pAniData->fFrmStrike0 = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnBSetFrameStrike1()
{
	__AnimData* pAniData = this->GetCurAniData();
	if (nullptr == pAniData)
		return;

	pAniData->fFrmStrike1 = GetDocument()->m_Scene.m_fFrmCur;
	this->UpdateInfo();
}

void CFormViewAnimation::OnChangeEFramePlugTraceStart()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFramePlugTraceEnd()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFrameSound0()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFrameSound1()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFrameBlend0()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFrameStrike0()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnChangeEFrameStrike1()
{
	this->GetData();
	this->UpdateFrameSliderAndOther();
}

void CFormViewAnimation::OnCDelayedLooping()
{
	this->UpdateFrameSliderAndOther();
	this->GetData();
}

void CFormViewAnimation::OnBMoveUp()
{
	this->OnEditAnimationDataMoveUp();
}

void CFormViewAnimation::OnBMoveDown()
{
	this->OnEditAnimationDataMoveDown();
}

void CFormViewAnimation::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
}

__AnimData* CFormViewAnimation::GetCurAniData()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return nullptr;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return nullptr;

	int iAni             = m_ListAnim0.GetCurSel();
	int iCount           = pAniCtrl->Count();
	__AnimData* pAniData = nullptr;
	if (iAni >= 0 && iAni < iCount)
		pAniData = pAniCtrl->DataGet(iAni);
	return pAniData;
}

void CFormViewAnimation::OnEditAnimationDataAdd()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	pAniCtrl->Add();
	int iAniCount = pAniCtrl->Count();
	this->UpdateAllInfo();

	m_ListAnim0.SetCurSel(iAniCount - 1);
	::SetFocus(GetDlgItem(IDC_E_ANI_NAME)->m_hWnd);
}

void CFormViewAnimation::OnEditAnimationDataInsert()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni = m_ListAnim0.GetCurSel();
	pAniCtrl->Insert(iAni);
	this->UpdateAllInfo();

	m_ListAnim0.SetCurSel(iAni);
	::SetFocus(GetDlgItem(IDC_E_ANI_NAME)->m_hWnd);
}

void CFormViewAnimation::OnEditAnimationDataDelete()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni = m_ListAnim0.GetCurSel();
	pAniCtrl->Delete(iAni);

	this->UpdateAllInfo();
}

void CFormViewAnimation::OnEditAnimationDataCopy()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni             = m_ListAnim0.GetCurSel();
	__AnimData* pAniData = pAniCtrl->DataGet(iAni);

	if (pAniData)
		m_AnimDataTemp = *pAniData;
}

void CFormViewAnimation::OnEditAnimationDataPaste()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni             = m_ListAnim0.GetCurSel();
	__AnimData* pAniData = pAniCtrl->DataGet(iAni);

	if (pAniData)
		*pAniData = m_AnimDataTemp;
	this->UpdateAllInfo();
}

void CFormViewAnimation::OnEditAnimationDataMoveUp()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl || pAniCtrl->Count() <= 0)
		return;
	int iAni         = m_ListAnim0.GetCurSel();
	int iAniToChange = iAni - 1;
	pAniCtrl->Swap(iAni, iAniToChange); // 위거랑 아래꺼랑 바꾼다.

	m_ListAnim0.SetCurSel(iAniToChange);
	this->UpdateAllInfo();
}

void CFormViewAnimation::OnEditAnimationDataMoveDown()
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl || pAniCtrl->Count() <= 0)
		return;
	int iAni         = m_ListAnim0.GetCurSel();
	int iAniToChange = iAni + 1;
	pAniCtrl->Swap(iAni, iAniToChange); // 위거랑 아래꺼랑 바꾼다.

	m_ListAnim0.SetCurSel(iAniToChange);
	this->UpdateAllInfo();
}

void CFormViewAnimation::OnDblclkListAnimation0()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CMenu* pMenu     = pFrm->GetMenu();
	CMenu* pSM       = pMenu->GetSubMenu(1); // 편집 메뉴
	pSM              = pSM->GetSubMenu(3);   // 에니메이션 편집 메뉴..
}

void CFormViewAnimation::OnBCalculateDelayTimeWithUpperAnimation()
{
	this->CalculateDelayTimeAndUpdate(-1);
}

void CFormViewAnimation::OnBCalculateDelayTimeWithLowerAnimation()
{
	this->CalculateDelayTimeAndUpdate(1);
}

void CFormViewAnimation::CalculateDelayTimeAndUpdate(int iAnimationIndexOffset)
{
	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;

	int iAni              = m_ListAnim0.GetCurSel();
	int iCount            = pAniCtrl->Count();
	__AnimData* pAniData  = pAniCtrl->DataGet(iAni);
	__AnimData* pAniData2 = pAniCtrl->DataGet(iAni + iAnimationIndexOffset);

	if (nullptr == pAniData || nullptr == pAniData2)
		return;

	float fAttackCount = 1.0f;
	if (IsDlgButtonChecked(IDC_C_DOUBLE_ATTACK))
		fAttackCount = 2.0f;

	if (pAniData->fFrmPerSec > 0 && pAniData2->fFrmPerSec > 0)
	{
		float fTime  = (pAniData->fFrmEnd - pAniData->fFrmStart) / pAniData->fFrmPerSec;
		float fTime2 = (pAniData2->fFrmEnd - pAniData2->fFrmStart) / pAniData2->fFrmPerSec;

		if (pAniData->iBlendFlags & 1)
			fTime += pAniData->fTimeBlend;
		if (pAniData2->iBlendFlags & 1)
			fTime2 += pAniData2->fTimeBlend;

		float fTDelta = pAniData->fTimeBlend + (fTime2 * fAttackCount) - fTime;
		if (fTDelta > 0)
		{
			pAniData->fTimeBlend = fTDelta;
			this->UpdateInfo();
		}
		else
		{
			MessageBox("두 에니메이션의 값 차이가 음수가 됩니다.");
		}
	}
	else
	{
		MessageBox("Animation Data 의 재생 속도가 0 입니다.");
	}
}
