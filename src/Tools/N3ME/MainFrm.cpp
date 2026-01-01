// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "N3ME.h"

#include "MainFrm.h"
#include "LyTerrain.h"
#include "DTex.h"
#include "DlgDTexGroupView.h"
#include "DTexMng.h"
#include "DTexGroupMng.h"
#include "DlgSetDTex.h"
#include "MapMng.h"
#include "DlgBrowsePath.h"
#include "DlgSowSeed.h"
#include "DlgSaveDivision.h"

#include <N3Base/N3EngTool.h>
#include <N3Base/N3Shape.h>
#include <N3Base/N3Camera.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_CURSOR_SELECT, OnCursorSelect)
	ON_COMMAND(ID_CURSOR_OBJ_BRUSH, OnCursorObjBrush)
	ON_COMMAND(ID_CURSOR_POS, OnCursorPos)
	ON_COMMAND(ID_CURSOR_ROTATE, OnCursorRotate)
	ON_COMMAND(ID_CURSOR_SCALE, OnCursorScale)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_SELECT, OnUpdateCursorSelect)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_OBJ_BRUSH, OnUpdateCursorObjBrush)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_POS, OnUpdateCursorPos)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_ROTATE, OnUpdateCursorRotate)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_SCALE, OnUpdateCursorScale)
	ON_COMMAND(ID_VIEW_OUTPUTOBJECT, OnViewOutputobject)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTOBJECT, OnUpdateViewOutputobject)
	ON_COMMAND(ID_VIEW_SOURCEOBJECT, OnViewSourceobject)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCEOBJECT, OnUpdateViewSourceobject)
	ON_COMMAND(ID_VIEW_SOLID, OnViewSolid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOLID, OnUpdateViewSolid)
	ON_COMMAND(ID_VIEW_WIREFRAME, OnViewWireframe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAME, OnUpdateViewWireframe)
	ON_COMMAND(ID_EDIT_PROPERTY, OnEditProperty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTY, OnUpdateEditProperty)
	ON_COMMAND(ID_CURSOR_EDIT_TERRAIN, OnCursorEditTerrain)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_EDIT_TERRAIN, OnUpdateCursorEditTerrain)
	ON_COMMAND(ID_TIP_FOCUS_SELOBJ, OnTipFocusSelobj)
	ON_UPDATE_COMMAND_UI(ID_TIP_FOCUS_SELOBJ, OnUpdateTipFocusSelobj)
	ON_COMMAND(ID_TIP_FOCUS_ALL, OnTipFocusAll)
	ON_UPDATE_COMMAND_UI(ID_TIP_FOCUS_ALL, OnUpdateTipFocusAll)
	ON_COMMAND(ID_TIP_DROP, OnTipDrop)
	ON_UPDATE_COMMAND_UI(ID_TIP_DROP, OnUpdateTipDrop)
	ON_COMMAND(ID_TIP_DELETE_OBJ, OnTipDeleteObj)
	ON_UPDATE_COMMAND_UI(ID_TIP_DELETE_OBJ, OnUpdateTipDeleteObj)
	ON_COMMAND(ID_CURSOR_SMOOTH_TERRAIN, OnCursorSmoothTerrain)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_SMOOTH_TERRAIN, OnUpdateCursorSmoothTerrain)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_COMMAND(ID_EDIT_DTEX, OnEditDtex)
	ON_COMMAND(ID_VIEW_DTEX, OnViewDtex)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DTEX, OnUpdateViewDtex)
	ON_COMMAND(ID_CURSOR_DTEX, OnCursorDtex)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_DTEX, OnUpdateCursorDtex)
	ON_COMMAND(ID_FILE_SERVER_DATA, OnFileServerData)
	ON_COMMAND(ID_CURSOR_EDIT_RIVER, OnCursorEditRiver)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_EDIT_RIVER, OnUpdateCursorEditRiver)
	ON_COMMAND(ID_CURSOR_EDIT_POND, OnCursorEditPond)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_EDIT_POND, OnUpdateCursorEditPond)
	ON_COMMAND(ID_RESOURCE_PATH_SET, OnResourcePathSet)
	ON_COMMAND(ID_FILE_COLORMAP_LOAD, OnFileColormapLoad)
	ON_COMMAND(ID_FILE_COLORMAP_SAVE, OnFileColormapSave)
	ON_COMMAND(ID_FILE_GAMETILETEX_SAVE, OnFileGametiletexSave)
	ON_COMMAND(ID_TIP_GENERATE_COLORMAP, OnTipGenerateColormap)
	ON_COMMAND(ID_TIP_GENERATE_COLORMAP_ALL, OnTipGenerateColormapAll)
	ON_UPDATE_COMMAND_UI(ID_TIP_GENERATE_COLORMAP, OnUpdateTipGenerateColormap)
	ON_COMMAND(ID_TIP_TILING_ALL, OnTipTilingAll)
	ON_UPDATE_COMMAND_UI(ID_TIP_TILING_ALL, OnUpdateTipTilingAll)
	ON_COMMAND(ID_TIP_DEFAULT_MATERIAL, OnTipDefaultMaterial)
	ON_COMMAND(ID_TIP_REMOVE_ALPHAFLAG, OnTipRemoveAlphaflag)
	ON_UPDATE_COMMAND_UI(ID_TIP_DEFAULT_MATERIAL, OnUpdateTipDefaultMaterial)
	ON_UPDATE_COMMAND_UI(ID_TIP_REMOVE_ALPHAFLAG, OnUpdateTipRemoveAlphaflag)
	ON_COMMAND(ID_CURSOR_EDIT_LIGHTMAP, OnCursorEditLightmap)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_EDIT_LIGHTMAP, OnUpdateCursorEditLightmap)
	ON_COMMAND(ID_CURSOR_MAKE_NPCPATH, OnCursorMakeNpcpath)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_MAKE_NPCPATH, OnUpdateCursorMakeNpcpath)
	ON_COMMAND(ID_VIEW_SELECTED_OBJECT_WIREFRAME, OnViewSelectedObjectWireframe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECTED_OBJECT_WIREFRAME, OnUpdateViewSelectedObjectWireframe)
	ON_COMMAND(ID_TIP_SOW_SEED, OnTipSowSeed)
	ON_COMMAND(ID_VIEW_HIDE_OBJ, OnViewHideObj)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDE_OBJ, OnUpdateViewHideObj)
	ON_COMMAND(ID_VIEW_AXIS_AND_GRID, OnViewAxisAndGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AXIS_AND_GRID, OnUpdateViewAxisAndGrid)
	ON_COMMAND(ID_CURSOR_MAKE_WALL, OnCursorMakeWall)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_MAKE_WALL, OnUpdateCursorMakeWall)
	ON_COMMAND(ID_CURSOR_EDIT_EVENT, OnCursorEditEvent)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_EDIT_EVENT, OnUpdateCursorEditEvent)
	ON_COMMAND(ID_FILE_IMPORT_HEIGHT, OnFileImportHeight)
	ON_COMMAND(ID_TIP_DELETE_UNUSED_FILES, OnTipDeleteUnusedFiles)
	ON_COMMAND(ID_TIP_DELETE_OVERLAPPED_OBJECTS, OnTipDeleteOverlappedObjects)
	ON_COMMAND(ID_TIP_DELETE_SELECTED_SOURCE_OBJECTS, OnTipDeleteSelectedSourceObjects)
	ON_COMMAND(ID_VIEW_ORTH, OnViewOrth)
	ON_COMMAND(ID_CURSOR_FLAT_TERRAIN, OnCursorFlatTerrain)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_FLAT_TERRAIN, OnUpdateCursorFlatTerrain)
	ON_COMMAND(ID_VIEW_TOGGLE_FOG, OnViewToggleFog)
	ON_COMMAND(ID_CURSOR_REGEN_USER, OnCursorRegenUser)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_REGEN_USER, OnUpdateCursorRegenUser)
	ON_COMMAND(ID_TIP_SET_WARP_INFO, OnTipSetWarpInfo)
	ON_UPDATE_COMMAND_UI(ID_TIP_SET_WARP_INFO, OnUpdateTipSetWarpInfo)
	ON_COMMAND(ID_FILE_SAVE_DIVIDE, OnFileSaveDivide)
	ON_COMMAND(ID_FILE_IMPORT_HEIGHT_BMP, OnFileImportHeightBmp)
	ON_COMMAND(ID_FILE_EXPORT_HEIGHT_BMP, OnFileExportHeightBmp)
	ON_COMMAND(ID_TIP_SET_HEIGHT_SCALE, OnTipSetHeightScale)
	ON_COMMAND(ID_CURSOR_SOUND, OnCursorSound)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_SOUND, OnUpdateCursorSound)
	ON_COMMAND(ID_SELECT_MAP_EDIT, OnSelectMapEdit)
	ON_COMMAND(ID_CURSOR_LIGHTOBJ, OnCursorLightobj)
	ON_UPDATE_COMMAND_UI(ID_CURSOR_LIGHTOBJ, OnUpdateCursorLightobj)
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
	m_pEng = nullptr;
	m_pMapMng = nullptr;
	m_pDTexMng = nullptr;
	m_pDTexGroupMng = nullptr;
	m_DTexInfoFileName.Empty();
	m_pDlgSowSeed = nullptr;
	ZeroMemory(m_SeedFileName, MAX_PATH);
}

CMainFrame::~CMainFrame()
{
	if(m_pDlgSowSeed)
	{
		m_pDlgSowSeed->Release();
		delete m_pDlgSowSeed;
		m_pDlgSowSeed = nullptr;
	}
	std::list<LPSEEDGROUP>::iterator sgit;
	for(sgit = m_SeedGroupList.begin(); sgit != m_SeedGroupList.end(); sgit++)
	{
		delete (*sgit);
	}
	m_SeedGroupList.clear();	
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	EnableDocking(CBRS_ALIGN_ANY);
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndDlgBar.Create(IDR_MAINFRAME, this))
//	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
//		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) ||
		!m_wndReBar.AddBar(&m_wndDlgBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	// 경로 설정..
	char szPathCur[256] = "";
	GetCurrentDirectory(256, szPathCur);
	CN3Base::PathSet(szPathCur);
	
	// 엔진 초기화
	m_pEng = new CN3EngTool();
	//m_pEng->InitEnv();
	m_pEng->Init(TRUE, m_hWnd, 32, 32, 0, TRUE);
	m_pEng->GridCreate(1000, 1000);

	//m_pEng->CreateExtraDevice(2, 2);

 	m_pMapMng = new CMapMng(this);

	m_pDTexGroupMng = new CDTexGroupMng;
	m_pDTexGroupMng->Init(this);

	m_pDTexMng = new CDTexMng;
	m_pDTexMng->Init(this);

	// 기본 리소스 읽기..
	m_pMapMng->LoadSourceObjects();
	m_pDTexGroupMng->Init(this);
	//m_pDTexGroupMng->LoadFromFile();
	m_pDTexMng->Init(this);
	//m_pDTexMng->LoadFromFile();

	// 툴바 다이얼로그 정보..
	m_wndDlgBar.SetDlgItemText(IDC_E_PATH, m_pEng->PathGet().c_str());

	//씨앗 뿌리기 관련 클래스 초기화..
	m_pDlgSowSeed = new CDlgSowSeed;
	m_pDlgSowSeed->Create(IDD_SOW_SEED,this);
	m_pDlgSowSeed->ShowWindow(FALSE);
	m_pDlgSowSeed->m_pRefFrm = this;

	m_pDlgMapView = new CDlgMapView;
	m_pDlgMapView->Create(IDD_ALL_MAP_VIEW,this);
	m_pDlgMapView->ShowWindow(FALSE);
	m_pDlgMapView->m_pFrame = this;
	m_pDlgMapView->m_pD3DDevice = m_pMapMng->s_lpD3DDev;
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

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


void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();

	delete m_pDTexGroupMng;
	m_pDTexGroupMng = nullptr;

	delete m_pDTexMng;
	m_pDTexMng = nullptr;

	delete m_pMapMng;
	m_pMapMng = nullptr;

	delete m_pDlgSowSeed;
	m_pDlgSowSeed = nullptr;

	delete m_pEng;
	m_pEng = nullptr;
}

void CMainFrame::OnFileExport() 
{
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(FALSE, "gmd", nullptr, dwFlags, "Game Map Data file(*.gmd)|*.gmd||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	strcpy(m_SeedFileName,dlg.GetFileTitle());
	m_pMapMng->MakeGameFiles(dlg.GetPathName());
}

void CMainFrame::OnFileServerData() 
{
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(FALSE, "smd", nullptr, dwFlags, "Server Map Data file(*.smd)|*.smd||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	m_pMapMng->MakeServerDataFiles(dlg.GetPathName());	
}


void CMainFrame::OnCursorSelect() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_SELECT);}
void CMainFrame::OnCursorObjBrush() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_OBJECT_BRUSH);}
void CMainFrame::OnCursorPos() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_POS);	}
void CMainFrame::OnCursorRotate()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_ROTATE);}
void CMainFrame::OnCursorScale()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_SCALE);}
void CMainFrame::OnCursorEditTerrain() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_TERRAIN);}
void CMainFrame::OnCursorSmoothTerrain() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_SMOOTH_TERRAIN);}
void CMainFrame::OnCursorFlatTerrain() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_FLAT_TERRAIN);}
void CMainFrame::OnCursorDtex() 
{
	ASSERT(m_pMapMng);
/*
	LPDIRECT3DSURFACE9 pBackBuff;
	HRESULT hr;
	hr = m_pEng->s_lpD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuff);
	D3DSURFACE_DESC desc;
	hr = pBackBuff->GetDesc(&desc);
	if(desc.Format!=D3DFMT_X8R8G8B8)
	{
		MessageBox("화면색상 모드가 32bit모드이어야 합니다..","경고!!");
	}
*/	
	m_pMapMng->SetCursorMode(CM_SET_DTEX);
//	this->OnViewDtex(); // 화면에 타일 텍스처 그룹 다이얼로그를 열어준다..
}
void CMainFrame::OnCursorEditRiver() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_RIVER);}
void CMainFrame::OnCursorEditPond() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_POND);}
void CMainFrame::OnCursorEditLightmap() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_LIGHTMAP);}
void CMainFrame::OnCursorMakeNpcpath()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_MAKE_NPCPATH);}
void CMainFrame::OnCursorMakeWall()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_MAKE_WALL);}
void CMainFrame::OnCursorEditEvent()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_EVENT);}
void CMainFrame::OnCursorRegenUser() 
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_REGENUSER);}
void CMainFrame::OnCursorSound()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_SOUND);}

void CMainFrame::OnUpdateCursorSelect(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_SELECT);}
void CMainFrame::OnUpdateCursorObjBrush(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_OBJECT_BRUSH);}
void CMainFrame::OnUpdateCursorPos(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_POS);}
void CMainFrame::OnUpdateCursorRotate(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_ROTATE);}
void CMainFrame::OnUpdateCursorScale(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_SCALE);}
void CMainFrame::OnUpdateCursorEditTerrain(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);
	if (m_pMapMng->GetTerrain()) pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_TERRAIN);
	else pCmdUI->Enable(FALSE);
}
void CMainFrame::OnUpdateCursorSmoothTerrain(CCmdUI* pCmdUI) 
{	
	ASSERT(m_pMapMng);
	if (m_pMapMng->GetTerrain()) pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_SMOOTH_TERRAIN);
	else pCmdUI->Enable(FALSE);
}
void CMainFrame::OnUpdateCursorFlatTerrain(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);
	if (m_pMapMng->GetTerrain()) pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_FLAT_TERRAIN);
	else pCmdUI->Enable(FALSE);
}
void CMainFrame::OnUpdateCursorDtex(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_SET_DTEX);}
void CMainFrame::OnUpdateCursorEditRiver(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_RIVER);}
void CMainFrame::OnUpdateCursorEditPond(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_POND);}
void CMainFrame::OnUpdateCursorEditLightmap(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_LIGHTMAP);}
void CMainFrame::OnUpdateCursorMakeNpcpath(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_MAKE_NPCPATH);}
void CMainFrame::OnUpdateCursorMakeWall(CCmdUI* pCmdUI)
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_MAKE_WALL);}
void CMainFrame::OnUpdateCursorEditEvent(CCmdUI* pCmdUI)
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_EVENT);}
void CMainFrame::OnUpdateCursorRegenUser(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_REGENUSER);}
void CMainFrame::OnUpdateCursorSound(CCmdUI* pCmdUI)
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_SOUND);}

void CMainFrame::OnViewOutputobject() 
{
	ASSERT(m_pMapMng);	CWnd* pWnd = m_pMapMng->GetDlgOutputList();
	if (pWnd) pWnd->ShowWindow(!pWnd->IsWindowVisible());
}
void CMainFrame::OnViewSourceobject() 
{
	ASSERT(m_pMapMng);	CWnd* pWnd = m_pMapMng->GetDlgSourceList();
	if (pWnd) pWnd->ShowWindow(!pWnd->IsWindowVisible());
}
void CMainFrame::OnUpdateViewOutputobject(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	CWnd* pWnd = m_pMapMng->GetDlgOutputList();
	if (pWnd) pCmdUI->SetCheck(pWnd->IsWindowVisible());
}
void CMainFrame::OnUpdateViewSourceobject(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	CWnd* pWnd = m_pMapMng->GetDlgSourceList();
	if (pWnd) pCmdUI->SetCheck(pWnd->IsWindowVisible());
}

void CMainFrame::OnEditProperty() 
{
	ASSERT(m_pMapMng);	CWnd* pWnd = m_pMapMng->GetDlgBase();
	if (pWnd) pWnd->ShowWindow(!pWnd->IsWindowVisible());
}
void CMainFrame::OnUpdateEditProperty(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	CWnd* pWnd = m_pMapMng->GetDlgBase();
	if (pWnd) pCmdUI->SetCheck(pWnd->IsWindowVisible());
}

void CMainFrame::OnViewSolid() 
{
	ASSERT(m_pMapMng);	m_pMapMng->m_FillMode = D3DFILL_SOLID;
	//m_pMapMng->m_ShadeMode = D3DSHADE_GOURAUD;
}
void CMainFrame::OnViewWireframe() 
{
	ASSERT(m_pMapMng);	m_pMapMng->m_FillMode = D3DFILL_WIREFRAME;
}
void CMainFrame::OnUpdateViewSolid(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->m_FillMode == D3DFILL_SOLID);
}
void CMainFrame::OnUpdateViewWireframe(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->m_FillMode == D3DFILL_WIREFRAME);
}
void CMainFrame::OnViewHideObj() 
{
	ASSERT(m_pMapMng);
	m_pMapMng->m_bHideObj = !(m_pMapMng->m_bHideObj);
	Invalidate(FALSE);
}

void CMainFrame::OnUpdateViewHideObj(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);

	if(m_pMapMng->m_bHideObj) pCmdUI->SetText("오브젝트 보이기\tH");
	if(!m_pMapMng->m_bHideObj) pCmdUI->SetText("오브젝트 감추기\tH");	
}

void CMainFrame::OnTipFocusSelobj() 
{
	ASSERT(m_pMapMng);	m_pMapMng->FocusSelObj();
}
void CMainFrame::OnTipFocusAll() 
{
	ASSERT(m_pMapMng);	m_pMapMng->FocusAll();
}
void CMainFrame::OnTipDrop() 
{
	ASSERT(m_pMapMng);	m_pMapMng->DropSelObjToTerrain();
}
void CMainFrame::OnTipDeleteObj() 
{
	ASSERT(m_pMapMng);	m_pMapMng->DeleteSelObjectFromOutputScene();
}
void CMainFrame::OnTipGenerateColormap() 
{
	ASSERT(m_pMapMng && m_pMapMng->GetTerrain());	m_pMapMng->GetTerrain()->GeneraterColorMap(false);
}
void CMainFrame::OnTipGenerateColormapAll() 
{
	ASSERT(m_pMapMng && m_pMapMng->GetTerrain());	m_pMapMng->GetTerrain()->GeneraterColorMap(true);
}
void CMainFrame::OnTipTilingAll() 
{
	ASSERT(m_pMapMng && m_pMapMng->GetTerrain());	m_pMapMng->GetTerrain()->TilingAll();
}

void CMainFrame::OnUpdateTipFocusSelobj(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable(m_pMapMng->GetSelOutputObjCount() ? TRUE:FALSE);
}
void CMainFrame::OnUpdateTipFocusAll(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable(m_pMapMng->GetTerrain() ? TRUE:FALSE);
}
void CMainFrame::OnUpdateTipDrop(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable( (m_pMapMng->GetTerrain() && m_pMapMng->GetSelOutputObjCount()) ? TRUE:FALSE);
}
void CMainFrame::OnUpdateTipDeleteObj(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable( m_pMapMng->GetSelOutputObjCount() ? TRUE:FALSE);
}
void CMainFrame::OnUpdateTipGenerateColormap(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable( m_pMapMng->GetTerrain() ? TRUE:FALSE);	
}
void CMainFrame::OnUpdateTipTilingAll(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable( m_pMapMng->GetTerrain() ? TRUE:FALSE);	
}


void CMainFrame::OnFileImport() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "N3VMesh", nullptr, dwFlags, "N3 Progressive Mesh file(*.N3VMesh)|*.N3VMesh||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	m_pMapMng->ImportTerrain(dlg.GetPathName());
}

void CMainFrame::OnFileImportHeight() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "N3VMesh", nullptr, dwFlags, "N3 Progressive Mesh file(*.N3VMesh)|*.N3VMesh||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	m_pMapMng->ImportTerrainHeight(dlg.GetPathName());	
}

void CMainFrame::OnEditDtex() 
{
	CDlgSetDTex dlg;
	dlg.DoModal();
}

void CMainFrame::OnViewDtex() 
{
	ASSERT(m_pDTexMng);
	CDlgDTexGroupView* pDlg = m_pDTexGroupMng->m_pGroupView;
	if(pDlg) pDlg->ShowWindow(!pDlg->IsWindowVisible());	
}

void CMainFrame::OnUpdateViewDtex(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);
	CDlgDTexGroupView* pDlg = m_pDTexGroupMng->m_pGroupView;
	if(pDlg) pCmdUI->SetCheck(pDlg->IsWindowVisible());
}

void CMainFrame::OnResourcePathSet() 
{
	ASSERT(m_pMapMng);

	CDlgBrowsePath dlg;
	if(IDCANCEL == dlg.DoModal()) return;
	
	CString szPath = dlg.GetPath();
	CT2A szPathA(szPath);
	CN3Base::PathSet(szPathA.m_psz); // 경로 설정..
	m_wndDlgBar.SetDlgItemText(IDC_E_PATH, szPath);
	
	// 기본 리소스 읽기..
	m_pMapMng->LoadSourceObjects();
	
}

void CMainFrame::OnFileColormapLoad() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "BMP", nullptr, dwFlags, "24Bit Bitmap File(*.BMP)|*.BMP||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	ASSERT(m_pMapMng);
	CLyTerrain* pTerrain = m_pMapMng->GetTerrain();
	if(pTerrain) pTerrain->ColorMapImport((LPCTSTR)dlg.GetPathName());
}

void CMainFrame::OnFileColormapSave() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(FALSE, "BMP", nullptr, dwFlags, "24Bit Bitmap File(*.BMP)|*.BMP||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	ASSERT(m_pMapMng);
	CLyTerrain* pTerrain = m_pMapMng->GetTerrain();
	if(pTerrain) pTerrain->ColorMapExport((LPCTSTR)dlg.GetPathName());
}

void CMainFrame::OnFileGametiletexSave() 
{
	if(m_pDTexMng)
	{
		m_pDTexMng->SaveGameTile();
	}
}

void CMainFrame::UpdateTransformInfo()
{
	m_wndDlgBar.UpdateInfo();
}

void CMainFrame::OnTipDefaultMaterial() // 기본재질로 만들기.
{
	ASSERT(m_pMapMng);

	int nSel = m_pMapMng->GetSelOutputObjCount();
	for(int i = 0; i < nSel; i++)
	{
		CN3TransformCollision* pObj = (CN3TransformCollision*)m_pMapMng->GetSelOutputObj(i);
		if(pObj->Type() & OBJ_SHAPE)
		{
			CN3Shape* pShape = (CN3Shape*)pObj;
			pShape->MakeDefaultMaterial(); // 기본재질로 만들기.
		}
	}
}

void CMainFrame::OnTipRemoveAlphaflag() // 알파 블렌딩 플래그 제거..
{
	ASSERT(m_pMapMng);

	int nSel = m_pMapMng->GetSelOutputObjCount();
	for(int i = 0; i < nSel; i++)
	{
		CN3TransformCollision* pObj = (CN3TransformCollision*)m_pMapMng->GetSelOutputObj(i);
		if(pObj->Type() & OBJ_SHAPE)
		{
			CN3Shape* pShape = (CN3Shape*)pObj;
			pShape->RemoveRenderFlags(RF_ALPHABLENDING); // 알파 블렌딩 플래그 제거..
		}
	}
}

void CMainFrame::OnUpdateTipDefaultMaterial(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable(m_pMapMng->GetSelOutputObjCount() ? TRUE:FALSE);
}

void CMainFrame::OnUpdateTipRemoveAlphaflag(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);	pCmdUI->Enable(m_pMapMng->GetSelOutputObjCount() ? TRUE:FALSE);
}

void CMainFrame::LoadDTexSet(CString FileName)
{
	if(m_DTexInfoFileName==FileName) return;

	m_DTexInfoFileName = FileName;

	m_pDTexGroupMng->LoadFromFile(FileName);
	m_pDTexMng->LoadFromFile(FileName);
}



void CMainFrame::OnViewSelectedObjectWireframe() 
{
	if(m_pMapMng)
	{
		m_pMapMng->m_bViewWireFrame = !m_pMapMng->m_bViewWireFrame;
	}
}

void CMainFrame::OnUpdateViewSelectedObjectWireframe(CCmdUI* pCmdUI) 
{
	if(m_pMapMng) pCmdUI->SetCheck(m_pMapMng->m_bViewWireFrame);
}

void CMainFrame::OnTipSowSeed() 
{
	if(!m_pDlgSowSeed->m_hWnd) m_pDlgSowSeed->Create(IDD_SOW_SEED);
	m_pDlgSowSeed->ShowWindow(TRUE);

	// 풀심기 
	if( m_pMapMng)
	{
		m_pMapMng->SetCursorMode(CM_EDIT_SEED);
		if( m_pMapMng->GetTerrain() != nullptr)
			m_pMapMng->GetTerrain()->SetEditMode(TEM_BRUSH_SHOW);

		m_pMapMng->m_SowSeedMng.SetActive(TRUE);
	}

}



void CMainFrame::OnViewAxisAndGrid() 
{
	if(m_pMapMng) m_pMapMng->m_bRenderAxisAndGrid = !m_pMapMng->m_bRenderAxisAndGrid;
}

void CMainFrame::OnUpdateViewAxisAndGrid(CCmdUI* pCmdUI) 
{
	if(m_pMapMng) pCmdUI->SetCheck(m_pMapMng->m_bRenderAxisAndGrid);
}

void CMainFrame::OnTipDeleteUnusedFiles() 
{
	if(m_pMapMng) m_pMapMng->DeleteUnusedFiles();
}

void CMainFrame::OnTipDeleteOverlappedObjects() 
{
	if(m_pMapMng) m_pMapMng->DeleteOverlappedObjects();
}

void CMainFrame::OnTipDeleteSelectedSourceObjects() 
{
	if(m_pMapMng) m_pMapMng->DeleteSelectedSourceObjects();
}

void CMainFrame::OnViewOrth() 
{
	if(m_pMapMng)
	{
		CN3Camera* pCamera = m_pMapMng->CameraGet();
		if(pCamera)
		{
			pCamera->m_bOrtho = !pCamera->m_bOrtho;
			pCamera->m_Data.fNP = 0.5f;
			pCamera->m_Data.fFP = 4096.0f;
			pCamera->EyePosSet(0, 2048, 0);
			pCamera->AtPosSet(0, 0, 0);
			pCamera->UpVectorSet(0, 0, 1);
			if(this->GetActiveView()) this->GetActiveView()->InvalidateRect(nullptr, FALSE);
		}
	}
}

void CMainFrame::OnViewToggleFog() 
{
	if(m_pMapMng)
	{
		CN3Camera* pCamera = m_pMapMng->CameraGet();
		if(pCamera)
		{
			pCamera->m_bFogUse = !pCamera->m_bFogUse;
			if(this->GetActiveView()) this->GetActiveView()->InvalidateRect(nullptr, FALSE);
		}
	}
}

void CMainFrame::OnTipSetWarpInfo() 
{
	if(m_pMapMng) m_pMapMng->ActWarpMgr();
}

void CMainFrame::OnUpdateTipSetWarpInfo(CCmdUI* pCmdUI) 
{
	ASSERT(m_pMapMng);
	pCmdUI->SetRadio(m_pMapMng->IsWarpMgrActive());
}

void CMainFrame::OnFileSaveDivide() 
{
	if(!m_pMapMng) return;
	if(!(m_pMapMng->GetTerrain())) return;

	int size = m_pMapMng->GetTerrain()->m_iHeightMapSize-1;

	CDlgSaveDivision dlg;
	dlg.m_pTerrain = m_pMapMng->GetTerrain();
	dlg.m_pMapMng = m_pMapMng;
	
	dlg.DoModal();			
}

void CMainFrame::OnFileImportHeightBmp() 
{
	if(!m_pMapMng) return;
	if(!(m_pMapMng->GetTerrain())) return;

	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "bmp", nullptr, dwFlags, "Height file(*.bmp)|*.bmp||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	m_pMapMng->GetTerrain()->ImportHeightBMP((LPCTSTR)dlg.GetPathName());
}

void CMainFrame::OnFileExportHeightBmp() 
{
	if(!m_pMapMng) return;
	if(!(m_pMapMng->GetTerrain())) return;

	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;	
	CFileDialog dlg(FALSE, "bmp", nullptr, dwFlags, "Height BMP file(*.bmp)|*.bmp||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	m_pMapMng->GetTerrain()->ExportHeightBMP((LPCTSTR)dlg.GetPathName());
}

void CMainFrame::OnTipSetHeightScale() 
{
	// TODO: Add your command handler code here
	
}




void CMainFrame::OnSelectMapEdit() 
{
	// TODO: Add your command handler code here
	m_pDlgMapView->ShowWindow(TRUE);
	m_pDlgMapView->UpData();
	m_pDlgMapView->Select_Map_Edit = TRUE;

}

void CMainFrame::OnCursorLightobj()
{	ASSERT(m_pMapMng);	m_pMapMng->SetCursorMode(CM_EDIT_LIGHT); }

void CMainFrame::OnUpdateCursorLightobj(CCmdUI* pCmdUI) 
{	ASSERT(m_pMapMng);	pCmdUI->SetRadio(m_pMapMng->GetCursorMode() == CM_EDIT_LIGHT);	}
