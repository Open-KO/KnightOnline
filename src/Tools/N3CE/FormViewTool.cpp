// FormViewTool.cpp : implementation file
//

#include "stdafx.h"
#include "n3ce.h"
#include "FormViewTool.h"

#include "N3CEDoc.h"
#include "N3CEView.h"
#include "FormViewAnimation.h"
#include "MainFrm.h"
#include <N3Base/N3Chr.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFormViewTool

IMPLEMENT_DYNCREATE(CFormViewTool, CFormView)

CFormViewTool::CFormViewTool()
	: CFormView(CFormViewTool::IDD)
{
	//{{AFX_DATA_INIT(CFormViewTool)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CFormViewTool::~CFormViewTool()
{
}

void CFormViewTool::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormViewTool)
	DDX_Control(pDX, IDC_LIST_SOUND1, m_ListSound1);
	DDX_Control(pDX, IDC_LIST_SOUND0, m_ListSound0);
	DDX_Control(pDX, IDC_CB_LOD, m_CBLOD);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFormViewTool, CFormView)
	//{{AFX_MSG_MAP(CFormViewTool)
	ON_BN_CLICKED(IDC_B_MAKE_SMOOTH, OnBMakeSmooth)
	ON_CBN_SELCHANGE(IDC_CB_LOD, OnSelchangeCbLod)
	ON_BN_CLICKED(IDC_B_ADD_TEST_SOUND0, OnBAddTestSound0)
	ON_BN_CLICKED(IDC_B_DELETE_TEST_SOUND0, OnBDeleteTestSound0)
	ON_BN_CLICKED(IDC_B_ADD_TEST_SOUND1, OnBAddTestSound1)
	ON_BN_CLICKED(IDC_B_DELETE_TEST_SOUND1, OnBDeleteTestSound1)
	ON_LBN_SELCHANGE(IDC_LIST_SOUND0, OnSelchangeListSound0)
	ON_LBN_SELCHANGE(IDC_LIST_SOUND1, OnSelchangeListSound1)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_B_APPLY_OFFSET_FRAME, OnBApplyOffsetFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormViewTool diagnostics

#ifdef _DEBUG
void CFormViewTool::AssertValid() const
{
	CFormView::AssertValid();
}

void CFormViewTool::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFormViewTool message handlers

void CFormViewTool::UpdateAllInfo()
{

}

void CFormViewTool::OnBMakeSmooth() 
{
	CN3Chr* pChr = ((CN3CEDoc*)GetDocument())->m_Scene.ChrGet(0);
	if(nullptr == pChr) return;

	int nPC = pChr->PartCount();
	for(int i = 0; i < nPC; i++)
	{
		CN3CPart* pPart = pChr->Part(i);
		if(nullptr == pPart) continue;

		for(int j = 0; j < MAX_CHR_LOD; j++)
		{
			CN3Skin* pSkin = pPart->Skin(j);
			if(nullptr == pSkin) continue;

			pSkin->ReGenerateSmoothNormal(); // 법선 벡터를 만들고.. // 부드럽게 게산..
		}
	}

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE); // 렌더링..
}

void CFormViewTool::OnInitialUpdate() 
{
//	if(m_CBLOD.m_hWnd)
//	{
//		m_CBLOD.ResetContent();
//		for(int i = 0; i < MAX_CHR_LOD; i++)
//		{
//			CString szLOD;
//			szLOD.Format("LOD : %d", i);
//			m_CBLOD.AddString(szLOD);
//		}
//		m_CBLOD.SetCurSel(0);
//	}

	CFormView::OnInitialUpdate();
}

void CFormViewTool::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if(m_CBLOD.m_hWnd) 
	{
		int nSel = m_CBLOD.GetCurSel();
		if(nSel < 0) m_CBLOD.SetCurSel(0);
	}
	this->UpdateAllInfo();
}

void CFormViewTool::OnSelchangeCbLod() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE); // 렌더링..
}

void CFormViewTool::OnBAddTestSound0() 
{
	char szBuff[10240] = "";
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave files(*.wav)|*.wav||", nullptr);
	char szCurPath[256]; GetCurrentDirectory(256, szCurPath);
	dlg.m_ofn.lpstrInitialDir = szCurPath;
	dlg.m_ofn.nMaxFile = 10240;
	dlg.m_ofn.lpstrFile = szBuff;

	if(dlg.DoModal() == IDCANCEL) return;

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	POSITION pos = dlg.GetStartPosition();
	CString FileName;
	while(pos != nullptr)
	{
		FileName = dlg.GetNextPathName(pos);
		m_ListSound0.AddString(FileName);
	}
}

void CFormViewTool::OnBDeleteTestSound0() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	
	int iSel = m_ListSound0.GetCurSel();
	if(iSel >= 0)
	{
		CString szFN;
		m_ListSound0.GetText(iSel, szFN);
		std::string szFN2(szFN);
		pFrm->m_SndMgr.ReleaseObj(&(pFrm->m_pSndObj0));

		m_ListSound0.DeleteString(iSel);
		m_ListSound0.SetCurSel(iSel);
	}
}

void CFormViewTool::OnSelchangeListSound0() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	
	int iSel = m_ListSound0.GetCurSel();
	if(iSel >= 0)
	{
		CString szFN;
		m_ListSound0.GetText(iSel, szFN);
		std::string szFN2(szFN);
		pFrm->m_SndMgr.ReleaseObj(&(pFrm->m_pSndObj0));
		pFrm->m_pSndObj0 = (pFrm->m_SndMgr.CreateObj(szFN2, SNDTYPE_3D));
	}
}

void CFormViewTool::OnBAddTestSound1() 
{
	char szBuff[10240] = "";
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, "wav", nullptr, dwFlags, "Wave files(*.wav)|*.wav||", nullptr);
	char szCurPath[256]; GetCurrentDirectory(256, szCurPath);
	dlg.m_ofn.lpstrInitialDir = szCurPath;
	dlg.m_ofn.nMaxFile = 10240;
	dlg.m_ofn.lpstrFile = szBuff;

	if(dlg.DoModal() == IDCANCEL) return;

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	POSITION pos = dlg.GetStartPosition();
	CString FileName;
	while(pos != nullptr)
	{
		FileName = dlg.GetNextPathName(pos);
		m_ListSound1.AddString(FileName);
	}
}

void CFormViewTool::OnBDeleteTestSound1() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	
	int iSel = m_ListSound1.GetCurSel();
	if(iSel >= 0)
	{
		CString szFN;
		m_ListSound1.GetText(iSel, szFN);
		std::string szFN2(szFN);
		pFrm->m_SndMgr.ReleaseObj(&(pFrm->m_pSndObj0));

		m_ListSound0.DeleteString(iSel);
		m_ListSound0.SetCurSel(iSel);
	}
}

void CFormViewTool::OnSelchangeListSound1() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	
	int iSel = m_ListSound1.GetCurSel();
	if(iSel >= 0)
	{
		CString szFN;
		m_ListSound1.GetText(iSel, szFN);
		std::string szFN2(szFN);
		pFrm->m_SndMgr.ReleaseObj(&(pFrm->m_pSndObj1));
		pFrm->m_pSndObj1 = pFrm->m_SndMgr.CreateObj(szFN2, SNDTYPE_3D);
	}
}

void CFormViewTool::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	
	if(m_ListSound0.GetSafeHwnd() != nullptr)
	{
		CRect rcOrg, rcClient;
		m_ListSound0.GetWindowRect(rcOrg); // 그래프 컨트롤의 원래 화면 좌표를 
		this->GetClientRect(rcClient); // 
		this->ClientToScreen(rcClient); // 화면 좌표로 바꾸고..

		int cx2 = rcClient.right - rcOrg.left - 5;
		int cy2 = rcOrg.Height();

		m_ListSound0.SetWindowPos(nullptr, 0, 0, cx2, cy2, SWP_NOMOVE | SWP_NOZORDER);
	}

	if(m_ListSound1.GetSafeHwnd() != nullptr)
	{
		CRect rcOrg, rcClient;
		m_ListSound1.GetWindowRect(rcOrg); // 그래프 컨트롤의 원래 화면 좌표를 
		this->GetClientRect(rcClient); // 
		this->ClientToScreen(rcClient); // 화면 좌표로 바꾸고..

		int cx2 = rcClient.right - rcOrg.left - 5;
		int cy2 = rcOrg.Height();

		m_ListSound1.SetWindowPos(nullptr, 0, 0, cx2, cy2, SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CFormViewTool::OnBApplyOffsetFrame() 
{
	CString str;
	GetDlgItemText(IDC_E_OFFSET, str);
	float fOffset = (float)atof(str);
	if(0 == fOffset) return;

	CN3CEDoc* pDoc = (CN3CEDoc*)(GetDocument());
	CN3Chr* pChr = pDoc->m_Scene.ChrGet(0);
	if(nullptr == pChr) return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if(nullptr == pAniCtrl || pAniCtrl->Count() <= 0) return;

	int iCount = pAniCtrl->Count();
	for(int i = 0; i < iCount; i++)
	{
		__AnimData* pData = pAniCtrl->DataGet(i);
		if(pData) pData->Offset(fOffset);
	}

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetPaneAnimation()->UpdateInfo();
}

