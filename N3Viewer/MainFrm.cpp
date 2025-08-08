// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "N3Viewer.h"

#include "N3ViewerDoc.h"
#include <N3Base/N3Scene.h>

#include "MainFrm.h"
#include "DlgBrowsePath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_PMESH, OnUpdateEditPmesh)
	ON_COMMAND(ID_EDIT_PMESH, OnEditPmesh)
	ON_COMMAND(ID_PROJECT_SET, OnProjectSet)
	ON_COMMAND(ID_ADD_LOD, OnAddLod)
	ON_COMMAND(ID_PLAY, OnPlay)
	ON_WM_TIMER()
	ON_COMMAND(ID_TOOL_FIX_PROGRESSIVE_MESH, OnToolFixProgressiveMesh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_eMode = MODE_NORMAL;

	m_wndSplitter.m_hWnd = NULL;
	m_wndSplitterLeft.m_hWnd = NULL;

	m_IsPlaying = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Engine 생성
//	m_Eng.InitEnv();
	if(!m_Eng.Init(TRUE, m_hWnd, 64, 64, 0, TRUE)) return -1;
	m_Eng.GridCreate(1000, 1000); // 그리드 만들기..

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	CRect rc; GetClientRect(rc);
	int nW1 = rc.Width() / 4;

	if(m_wndSplitter.CreateStatic(this, 1, 2) == FALSE) return FALSE;
	if(m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CN3ViewerView), CSize(rc.Width() - nW1, rc.Height()), pContext) == FALSE) return FALSE;

	if(m_wndSplitterLeft.CreateStatic(&m_wndSplitter, 2, 1) == FALSE) return FALSE;
	if(m_wndSplitterLeft.CreateView(0, 0, RUNTIME_CLASS(CViewSceneTree), CSize(nW1, rc.Height()/2), pContext) == FALSE) return FALSE;
	if(m_wndSplitterLeft.CreateView(1, 0, RUNTIME_CLASS(CViewProperty), CSize(nW1, rc.Height()/2), pContext) == FALSE) return FALSE;

//	m_wndSplitter.SetColumnInfo(0, nW1, 0);
//	if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CN3ViewerView), CSize(rc.Width() - nW1, rc.Height()), pContext) ||
//		!m_wndSplitterLeft.CreateView(0, 0, RUNTIME_CLASS(CViewSceneTree), CSize(nW1, rc.Height()/2), pContext) || 
//		!m_wndSplitterLeft.CreateView(1, 0, RUNTIME_CLASS(CViewProperty), CSize(nW1, rc.Height()/2), pContext) )
//	{
//		m_wndSplitter.DestroyWindow();
//		m_wndSplitterLeft.DestroyWindow();
//		return FALSE;
//	}

	m_wndSplitter.SetColumnInfo(0, nW1, nW1/2);

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	cs.x = GetSystemMetrics(SM_CXSCREEN) / 6;
	cs.y = GetSystemMetrics(SM_CYSCREEN) / 6;
	cs.cx = GetSystemMetrics(SM_CXSCREEN) * 2 / 3;
	cs.cy = GetSystemMetrics(SM_CYSCREEN) * 2 / 3;

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


CN3ViewerView* CMainFrame::GetView()
{
	return (CN3ViewerView*)(m_wndSplitter.GetPane(0,1));
}

CViewSceneTree* CMainFrame::GetViewSceneTree()
{
	return (CViewSceneTree*)(m_wndSplitterLeft.GetPane(0,0));
}

CViewProperty* CMainFrame::GetViewProperty()
{
	return (CViewProperty*)(m_wndSplitterLeft.GetPane(1,0));
}

void CMainFrame::OnEditPmesh() 
{
	CN3ViewerDoc* pDoc = (CN3ViewerDoc*)(this->GetActiveDocument());
	if(NULL == pDoc) return;

	if(pDoc->m_pSelectedObj && (pDoc->m_pSelectedObj->Type() & OBJ_SHAPE) && MODE_NORMAL == m_eMode)
	{
		m_eMode = MODE_EDIT_PMESH;
		m_DlgPMeshEdit.m_pShapeRef = (CN3Shape*)pDoc->m_pSelectedObj;
		if(m_DlgPMeshEdit.GetSafeHwnd() == NULL) m_DlgPMeshEdit.Create(IDD_PMESH_EDIT, this);
		m_DlgPMeshEdit.ShowWindow(SW_SHOW);
	}
	else
	{
		m_DlgPMeshEdit.DestroyWindow();
		m_eMode = MODE_NORMAL;
	}
	
	this->GetView()->InvalidateRect(NULL, FALSE);
}

void CMainFrame::OnUpdateEditPmesh(CCmdUI* pCmdUI) 
{
	if(MODE_NORMAL == m_eMode) pCmdUI->SetText("Edit Progressive Mesh");
	else pCmdUI->SetText("End <Edit Progressive Mesh>");
}

void CMainFrame::OnProjectSet() 
{
	CDlgBrowsePath dlg;
	if(IDCANCEL == dlg.DoModal()) return;
	
	CString szPath = dlg.GetPath();

	CT2A szPathA(szPath);
	CN3Base::PathSet(szPathA.m_psz);
}

void CMainFrame::OnAddLod() 
{
	if(m_DlgPMeshEdit.GetSafeHwnd() != NULL)
		m_DlgPMeshEdit.LOD_Add();
}

void CMainFrame::OnPlay() 
{
	m_IsPlaying = !m_IsPlaying;
	if(m_IsPlaying) this->SetTimer(100, 10, NULL);
	else this->KillTimer(100);
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	if(100 == nIDEvent)
	{
		this->GetView()->InvalidateRect(NULL, FALSE);
	}
	
	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnToolFixProgressiveMesh() 
{
	CString szExt;
	CString szFilter = "N3 Progressive Mesh File(*.N3PMesh)|*.N3PMesh||";

	CString FileName;
	char* pBuff = new char[2048000]; memset(pBuff, 0, 2048000);
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, szExt, NULL, dwFlags, szFilter, NULL);
	dlg.m_ofn.nMaxFile = 2048000;
	dlg.m_ofn.lpstrFile = pBuff;
	if(dlg.DoModal() == IDCANCEL)
	{
		delete [] pBuff;
		return;
	}

	CFile file;
	file.Open("프로그레시브 메쉬 처리 안된 리스트.txt", CFile::modeWrite | CFile::modeCreate);

	CN3PMesh PM;
	POSITION pos = dlg.GetStartPosition();
	for(int i = 0; pos != NULL; i++)
	{
		FileName = dlg.GetNextPathName(pos);
		
		PM.Release();
		PM.LoadFromFile(std::string(FileName));
		PM.SaveToFile(std::string(FileName));

		if(PM.LODCtrlCount() <= 0 || PM.CollapsesCount() <= 0)
		{
			CString szWarning;
			szWarning.Format("LOD 처리 안됨 : %s\r\n", PM.FileName().c_str());
			file.Write(szWarning, szWarning.GetLength());
		}
	}

	MessageBox("프로그레시브 처리가 안된 파일리스트는 \"프로그레시브 메쉬 처리 안된 리스트.txt\" 파일에 기록되었습니다.");
	file.Close();

	delete [] pBuff;
}
