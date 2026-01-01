// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "N3FXE.h"

#include "N3FXEView.h"
#include "MainFrm.h"
#include "DlgEditScript.h"
#include "DlgEditPartMesh.h"
#include "DlgEditPartBillBoard.h"
#include "DlgEditPartParticle.h"
#include "DlgEditPartGround.h"
#include "DlgNewPart.h"
#include "DlgEditFxg.h"
#include <N3Base/N3FXDef.h>
#include <N3Base/N3FXBundle.h>
#include "Ground.h"


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
	ON_COMMAND(ID_FILE_LOAD_PART, OnFileLoadPart)
	ON_COMMAND(ID_FILE_LOAD_BUNDLE, OnFileLoadBundle)
	ON_COMMAND(ID_VIEW_MODEL, OnViewModel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL, OnUpdateViewModel)
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)
	ON_COMMAND(ID_VIEW_AXIS, OnViewAxis)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AXIS, OnUpdateViewAxis)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_VIEW_BGCOLOR, OnViewBgcolor)
	ON_COMMAND(ID_FILE_NEW_BUNDLE, OnFileNewBundle)
	ON_COMMAND(ID_VIEW_FLOOR, OnViewFloor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FLOOR, OnUpdateViewFloor)
	ON_COMMAND(ID_FILE_OPEN_FLOOR_TEX, OnFileOpenFloorTex)
	ON_COMMAND(ID_FILE_NEW_GROUP, OnFileNewGroup)
	ON_COMMAND(ID_FILE_LOAD_GROUP, OnFileLoadGroup)
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
	m_pCurrFX = nullptr;
	m_pEditWndList.clear();
	m_pEditGroup.clear();

	m_bRenderChr = true;
	m_bRenderGrid = true;
	m_bRenderAxis = true;
	m_bRenderFloor = true;
	m_dwBGColor = 0xff404040;

	m_pGround = nullptr;
}

CMainFrame::~CMainFrame()
{
	m_pCurrFX = nullptr;

	std::list<CDlgEditScript*>::iterator it;
	it = m_pEditWndList.begin();
	while(it!=m_pEditWndList.end())
	{
		(*it)->DestroyWindow();
		delete (*it);
		it++;		
	}
	m_pEditWndList.clear();

	std::list<CDlgEditPartMesh*>::iterator itPartMesh;
	itPartMesh = m_pEditPartMesh.begin();
	while(itPartMesh!=m_pEditPartMesh.end())
	{
		(*itPartMesh)->DestroyWindow();
		delete (*itPartMesh);
		itPartMesh++;		
	}
	m_pEditPartMesh.clear();

	std::list<CDlgEditPartBillBoard*>::iterator itPartBillBoard;
	itPartBillBoard = m_pEditPartBillBoard.begin();
	while(itPartBillBoard!=m_pEditPartBillBoard.end())
	{
		(*itPartBillBoard)->DestroyWindow();
		delete (*itPartBillBoard);
		itPartBillBoard++;		
	}
	m_pEditPartBillBoard.clear();

	std::list<CDlgEditPartParticle*>::iterator itPartParticle;
	itPartParticle = m_pEditPartParticle.begin();
	while(itPartParticle!=m_pEditPartParticle.end())
	{
		(*itPartParticle)->DestroyWindow();
		delete (*itPartParticle);
		itPartParticle++;		
	}
	m_pEditPartParticle.clear();

	std::list<CDlgEditPartGround*>::iterator itPartGround;
	itPartGround = m_pEditPartGround.begin();
	while(itPartGround!=m_pEditPartGround.end())
	{
		(*itPartGround)->DestroyWindow();
		delete (*itPartGround);
		itPartGround++;		
	}
	m_pEditPartGround.clear();

	std::list<CDlgEditFxg*>::iterator itGroup;
	itGroup = m_pEditGroup.begin();
	while(itGroup!=m_pEditGroup.end())
	{
		(*itGroup)->DestroyWindow();
		delete (*itGroup);
		itGroup++;		
	}
	m_pEditGroup.clear();

	if(m_pGround) delete m_pGround;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
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

	// Engine 생성
	if(m_Eng.Init(TRUE, m_hWnd, 64, 64, 0, TRUE) == false) return -1;
	m_Eng.GridCreate(200, 200); // 그리드 만들기..

	// 기본 카메라 & Light 만들기..
	m_Camera.EyePosSet(0, 5, -15);
	//D3DCOLORVALUE crLgt = { 1.0f, 1.0f, 1.0f, 1.0f };
	//m_Light.m_Data.InitDirection(0, __Vector3(-1, -1, -1), crLgt);
	//m_Light.m_Data.InitPoint(0, __Vector3(0, 0, 0), crLgt);

	__ColorValue crLgt;
	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.8f;
	//CN3Light* pLightGlobal = new CN3Light(); // 전체를 비출 라이트..
	m_Light[0].m_Data.InitDirection(0, { 0, -1, 0 }, crLgt);
	m_Light[1].m_Data.InitDirection(1, { 0, 1, 0 }, crLgt);
	
	//
	m_Chr.LoadFromFile("object\\obj_baba.n3shape");

	m_pGround = new CGround;
	
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

bool CMainFrame::IsDuplicated(CString& PathName)
{
	std::list<CDlgEditPartMesh*>::iterator itPartMesh;
	for(itPartMesh=m_pEditPartMesh.begin(); itPartMesh!=m_pEditPartMesh.end();itPartMesh++)
	{
		if((lstrcmpi((LPCTSTR)PathName, (LPCTSTR)(*itPartMesh)->m_strPathName)==0) ) 
		{			
			(*itPartMesh)->ShowWindow(TRUE);
			return true;
		}
	}

	std::list<CDlgEditPartBillBoard*>::iterator itPartBillBoard;
	for(itPartBillBoard=m_pEditPartBillBoard.begin(); itPartBillBoard!=m_pEditPartBillBoard.end(); itPartBillBoard++)
	{
		if((lstrcmpi((LPCTSTR)PathName, (LPCTSTR)(*itPartBillBoard)->m_strPathName)==0) ) 
		{			
			(*itPartBillBoard)->ShowWindow(TRUE);
			return true;
		}
	}

	std::list<CDlgEditPartParticle*>::iterator itPartParticle;
	for(itPartParticle=m_pEditPartParticle.begin(); itPartParticle!=m_pEditPartParticle.end(); itPartParticle++)
	{
		if((lstrcmpi((LPCTSTR)PathName, (LPCTSTR)(*itPartParticle)->m_strPathName)==0) ) 
		{			
			(*itPartParticle)->ShowWindow(TRUE);
			return true;
		}
	}

	std::list<CDlgEditPartGround*>::iterator itPartGround;
	for(itPartGround=m_pEditPartGround.begin(); itPartGround!=m_pEditPartGround.end(); itPartGround++)
	{
		if((lstrcmpi((LPCTSTR)PathName, (LPCTSTR)(*itPartGround)->m_strPathName)==0) ) 
		{			
			(*itPartGround)->ShowWindow(TRUE);
			return true;
		}
	}

	return false;
}

int CMainFrame::GetPartType(CString& PathName)
{
	int PartType = FX_PART_TYPE_NONE;

	FILE* stream = fopen((LPCTSTR)PathName, "r");
	if(!stream) return 0;
	
	char szLine[512] = "", szCommand[80] = "", szBuf[4][80] = { "", "", "", ""};
	char* pResult = fgets(szLine, 512, stream);
	sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

	if(lstrcmpi(szCommand, "<n3fxPart>"))
	{
		fclose(stream);
		return FX_PART_TYPE_NONE;
	}

	while(!feof(stream))
	{
		char* pResult = fgets(szLine, 512, stream);
		if(pResult == nullptr) continue;

		ZeroMemory(szCommand,80);
		ZeroMemory(szBuf[0],80);
		ZeroMemory(szBuf[1],80);
		ZeroMemory(szBuf[2],80);
		ZeroMemory(szBuf[3],80);
		sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

		if(lstrcmpi(szCommand, "<type>")==0)
		{
			if(lstrcmpi(szBuf[0], "particle")==0) PartType = FX_PART_TYPE_PARTICLE;
			else if(lstrcmpi(szBuf[0], "board")==0) PartType = FX_PART_TYPE_BOARD;
			else if(lstrcmpi(szBuf[0], "mesh")==0) PartType = FX_PART_TYPE_MESH;
			else if(lstrcmpi(szBuf[0], "ground")==0) PartType = FX_PART_TYPE_BOTTOMBOARD;			
			//^^v 더 넣을꺼 있으면 넣어라..

			fclose(stream);
			return PartType;
		}		
	}
	fclose(stream);
	return PartType;
}


void CMainFrame::OnFileLoadPart() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, "N3FXPart", nullptr, dwFlags, "N3FXPart File(*.n3fxpart)|*.n3fxpart||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;

	POSITION pos = dlg.GetStartPosition();

	while(pos != nullptr)
	{
		CString PathName = dlg.GetNextPathName(pos);

		if(IsDuplicated(PathName)) return;

		int iPartType = GetPartType(PathName);

		switch(iPartType)
		{
		case FX_PART_TYPE_MESH:
			{
				CDlgEditPartMesh* pEditPartWnd = new CDlgEditPartMesh;
				pEditPartWnd->Create(IDD_DLG_PART_MESH);
				pEditPartWnd->m_pRefFrm = this;									
									
				if(pEditPartWnd->LoadPartScript((LPCTSTR)PathName))
				{
					pEditPartWnd->ShowWindow(TRUE);
					m_pEditPartMesh.push_back(pEditPartWnd);
				}
				else
				{
					MessageBox("n3fxpart파일을 여는데 실패했습니다...-.-;;","ERR01",MB_OK);
					pEditPartWnd->DestroyWindow();
					delete pEditPartWnd;
				}
				break;
			}
		case FX_PART_TYPE_BOARD:
			{
				CDlgEditPartBillBoard* pEditPartWnd = new CDlgEditPartBillBoard;
				pEditPartWnd->Create(IDD_DLG_PART_BILLBOARD);
				pEditPartWnd->m_pRefFrm = this;									
									
				if(pEditPartWnd->LoadPartScript((LPCTSTR)PathName))
				{
					pEditPartWnd->ShowWindow(TRUE);
					m_pEditPartBillBoard.push_back(pEditPartWnd);
				}
				else
				{
					MessageBox("n3fxpart파일을 여는데 실패했습니다...-.-;;","ERR01",MB_OK);
					pEditPartWnd->DestroyWindow();
					delete pEditPartWnd;
				}
				break;
			}
		case FX_PART_TYPE_PARTICLE:
			{
				CDlgEditPartParticle* pEditPartWnd = new CDlgEditPartParticle;
				pEditPartWnd->Create(IDD_DLG_PART_PARTICLE);
				pEditPartWnd->m_pRefFrm = this;									
									
				if(pEditPartWnd->LoadPartScript((LPCTSTR)PathName))
				{
					pEditPartWnd->ShowWindow(TRUE);
					m_pEditPartParticle.push_back(pEditPartWnd);
				}
				else
				{
					MessageBox("n3fxpart파일을 여는데 실패했습니다...-.-;;","ERR01",MB_OK);
					pEditPartWnd->DestroyWindow();
					delete pEditPartWnd;
				}
				break;
			}
		case FX_PART_TYPE_BOTTOMBOARD:
			{
				CDlgEditPartGround* pEditPartWnd = new CDlgEditPartGround;
				pEditPartWnd->Create(IDD_DLG_PART_GROUND);
				pEditPartWnd->m_pRefFrm = this;									
									
				if(pEditPartWnd->LoadPartScript((LPCTSTR)PathName))
				{
					pEditPartWnd->ShowWindow(TRUE);
					m_pEditPartGround.push_back(pEditPartWnd);
				}
				else
				{
					MessageBox("n3fxpart파일을 여는데 실패했습니다...-.-;;","ERR01",MB_OK);
					pEditPartWnd->DestroyWindow();
					delete pEditPartWnd;
				}
				break;
			}
		}
	}
}

void CMainFrame::OnFileLoadBundle() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, "N3FXBundle", nullptr, dwFlags, "N3FXBundle File(*.n3fxbundle)|*.n3fxbundle||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	POSITION pos = dlg.GetStartPosition();

	while(pos != nullptr)
	{
		CString PathName = dlg.GetNextPathName(pos);

		//스크립트 에디트 창 만들고, 스크립트 정보들 읽고, 셋팅..
		CDlgEditScript*	pEditWnd = new CDlgEditScript;
		pEditWnd->Create(IDD_DLG_SCRIPTEDITOR, nullptr);
		pEditWnd->m_pRefFrm = this;

		// load 해서 성공하면 그대로 하고 실패하면 다 지워버려..
		if(pEditWnd->LoadBundle(PathName))
		{
			pEditWnd->ShowWindow(TRUE);
			m_pEditWndList.push_back(pEditWnd);			
		}
		else
		{
			MessageBox("n3fxbundle파일을 여는데 실패했습니다.","-.-;;",MB_OK);
			pEditWnd->DestroyWindow();
			delete pEditWnd;
		}
	}
}

void CMainFrame::TickRender()
{
	m_Eng.Clear(m_dwBGColor);
	m_Eng.s_lpD3DDev->BeginScene();
	__Matrix44 mtxScale;
	__Matrix44 mtxWorld;
	mtxWorld.Identity();
	m_Eng.s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());

	mtxScale.Scale(1,1,1);
	if(m_bRenderGrid) m_Eng.RenderGrid(mtxScale);
	if(m_bRenderAxis) m_Eng.RenderAxis(true);

	m_Camera.Tick();
	m_Camera.Apply();
	m_Light[0].Tick();
	m_Light[0].Apply();

	m_Light[1].Tick();
	m_Light[1].Apply();

//	__Matrix44 mtxChrRot;
//	mtxChrRot.Identity();
//	mtxChrRot.RotationY(__PI);
//	__Quaternion qt(mtxChrRot);
	
//	m_Chr.RotSet(qt);

	m_Chr.Tick();
	//m_Chr.PosSet(0.0f, -0.2f, 0.0f);
	if(m_bRenderChr)
	{
		m_Eng.s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		m_Chr.Render();
	}

	mtxWorld.Identity();
	m_Eng.s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());

	if(m_bRenderFloor) m_pGround->Render();
	
	//온갖 renderstate설정...	
	if(m_pCurrFX)
	{
		DWORD dwLgt; 
		m_Eng.s_lpD3DDev->GetRenderState( D3DRS_LIGHTING, &dwLgt );
		m_Eng.s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, FALSE );
		
		m_pCurrFX->Tick();
		m_pCurrFX->Render();		
		
		m_Eng.s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, dwLgt );		
	}

	CN3Base::s_AlphaMgr.Render(); // 이걸 반드시 해줘야 알파블렌딩 버퍼 오버플로우가 안난다..
	m_Eng.s_lpD3DDev->EndScene();
	m_Eng.Present(GetActiveView()->m_hWnd);

	// 프레임 표시
	DWORD dwTick = GetTickCount();
	static DWORD dwTickPrev = dwTick;
	static CString szFPS;
	if(dwTick > dwTickPrev + 100) 
	{
		szFPS.Format("FPS : %6.2f", m_Eng.s_fFrmPerSec);
		dwTickPrev = dwTick;
	}

	m_wndDlgBar.SetDlgItemText(IDC_FPS, (LPCTSTR)szFPS);
}


void CMainFrame::OnViewModel() { m_bRenderChr = !m_bRenderChr; }
void CMainFrame::OnUpdateViewModel(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_bRenderChr); }
void CMainFrame::OnViewGrid() {	m_bRenderGrid = !m_bRenderGrid;	}
void CMainFrame::OnUpdateViewGrid(CCmdUI* pCmdUI) {	pCmdUI->SetCheck(m_bRenderGrid); }
void CMainFrame::OnViewAxis() {	m_bRenderAxis = !m_bRenderAxis;	}
void CMainFrame::OnUpdateViewAxis(CCmdUI* pCmdUI) {	pCmdUI->SetCheck(m_bRenderAxis); }
void CMainFrame::OnViewFloor() { m_bRenderFloor = !m_bRenderFloor; }
void CMainFrame::OnUpdateViewFloor(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_bRenderFloor); }

void CMainFrame::DestroyGroup(CDlgEditFxg* pGroup)
{
	std::list<CDlgEditFxg*>::iterator it;
	it = m_pEditGroup.begin();

	while(it!=m_pEditGroup.end())
	{
		if((*it)==pGroup)
		{
			it = m_pEditGroup.erase(it);
			pGroup->DestroyWindow();
			delete pGroup;
			continue;			
		}
		it++;
	}
}

void CMainFrame::DestroyBundle(CDlgEditScript* pBundle)
{
	std::list<CDlgEditScript*>::iterator it;
	it = m_pEditWndList.begin();

	while(it!=m_pEditWndList.end())
	{
		if((*it)==pBundle)
		{
			it = m_pEditWndList.erase(it);
			pBundle->DestroyWindow();
			delete pBundle;
			continue;			
		}
		it++;
	}
}

void CMainFrame::DestroyPartMesh(CDlgEditPartMesh* pPartMesh)
{
	std::list<CDlgEditPartMesh*>::iterator it;
	it = m_pEditPartMesh.begin();

	while(it!=m_pEditPartMesh.end())
	{
		if((*it)==pPartMesh)
		{
			it = m_pEditPartMesh.erase(it);
			pPartMesh->DestroyWindow();
			delete pPartMesh;
			continue;			
		}
		it++;
	}
}

void CMainFrame::DestroyPartBillBoard(CDlgEditPartBillBoard* pPartBillBoard)
{
	std::list<CDlgEditPartBillBoard*>::iterator it;
	it = m_pEditPartBillBoard.begin();

	while(it!=m_pEditPartBillBoard.end())
	{
		if((*it)==pPartBillBoard)
		{
			it = m_pEditPartBillBoard.erase(it);
			pPartBillBoard->DestroyWindow();
			delete pPartBillBoard;
			continue;			
		}
		it++;
	}
}

void CMainFrame::DestroyPartParticle(CDlgEditPartParticle* pPartParticle)
{
	std::list<CDlgEditPartParticle*>::iterator it;
	it = m_pEditPartParticle.begin();

	while(it!=m_pEditPartParticle.end())
	{
		if((*it)==pPartParticle)
		{
			it = m_pEditPartParticle.erase(it);
			pPartParticle->DestroyWindow();
			delete pPartParticle;
			continue;			
		}
		it++;
	}
}

void CMainFrame::DestroyPartGround(CDlgEditPartGround* pPartGround)
{
	std::list<CDlgEditPartGround*>::iterator it;
	it = m_pEditPartGround.begin();

	while(it!=m_pEditPartGround.end())
	{
		if((*it)==pPartGround)
		{
			it = m_pEditPartGround.erase(it);
			pPartGround->DestroyWindow();
			delete pPartGround;
			continue;			
		}
		it++;
	}
}

void CMainFrame::OnFileNew() 
{
	CDlgNewPart Dlg;
	if(Dlg.DoModal()==IDOK)
	{
		int PartType = Dlg.m_iPartType + 1;

		switch(PartType)
		{
		case FX_PART_TYPE_MESH:
			{
				CDlgEditPartMesh* pEditPartWnd = new CDlgEditPartMesh;
				pEditPartWnd->Create(IDD_DLG_PART_MESH);
				pEditPartWnd->m_pRefFrm = this;

				pEditPartWnd->ShowWindow(TRUE);
				m_pEditPartMesh.push_back(pEditPartWnd);
				break;
			}
		case FX_PART_TYPE_BOARD:
			{
				CDlgEditPartBillBoard* pEditPartWnd = new CDlgEditPartBillBoard;
				pEditPartWnd->Create(IDD_DLG_PART_BILLBOARD);
				pEditPartWnd->m_pRefFrm = this;
				
				pEditPartWnd->ShowWindow(TRUE);
				m_pEditPartBillBoard.push_back(pEditPartWnd);
				break;
			}
		case FX_PART_TYPE_PARTICLE:
			{
				CDlgEditPartParticle* pEditPartWnd = new CDlgEditPartParticle;
				pEditPartWnd->Create(IDD_DLG_PART_PARTICLE);
				pEditPartWnd->m_pRefFrm = this;									
				
				pEditPartWnd->ShowWindow(TRUE);
				m_pEditPartParticle.push_back(pEditPartWnd);
				break;
			}
		case FX_PART_TYPE_BOTTOMBOARD:
			{
				CDlgEditPartGround* pEditPartWnd = new CDlgEditPartGround;
				pEditPartWnd->Create(IDD_DLG_PART_GROUND);
				pEditPartWnd->m_pRefFrm = this;									
				
				pEditPartWnd->ShowWindow(TRUE);
				m_pEditPartGround.push_back(pEditPartWnd);
				break;
			}
		}
	}	
}

void CMainFrame::OnViewBgcolor() 
{
	CColorDialog dlg;
	if(dlg.DoModal()==IDOK)
	{
		COLORREF color = dlg.GetColor();
		DWORD r,g,b;
		r = (color<<24)>>8;
		g = ((color>>8)<<24)>>16;
		b = ((color>>16)<<24)>>24;
		m_dwBGColor = 0xff000000;
		m_dwBGColor += r;
		m_dwBGColor += g;
		m_dwBGColor += b;
	}
}

void CMainFrame::OnFileNewBundle() 
{
	//스크립트 에디트 창 만들고, 스크립트 정보들 읽고, 셋팅..
	CDlgEditScript*	pEditWnd = new CDlgEditScript;
	pEditWnd->Create(IDD_DLG_SCRIPTEDITOR, nullptr);
	pEditWnd->m_pRefFrm = this;

	// load 해서 성공하면 그대로 하고 실패하면 다 지워버려..
	if(pEditWnd->NewBundle())
	{
		pEditWnd->ShowWindow(TRUE);
		m_pEditWndList.push_back(pEditWnd);			
	}
	else
	{
		MessageBox("새 번들을 만드는데 실패했습니다.","ERR;;",MB_OK);
		pEditWnd->DestroyWindow();
		delete pEditWnd;
	}		
}


void CMainFrame::OnFileOpenFloorTex() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, nullptr, nullptr, dwFlags, "tga(*.tga),bmp(*.bmp),dxt(*.dxt)|*.tga;*.bmp;*.dxt||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	m_pGround->SetTex((LPCTSTR)dlg.GetPathName());
	
}

void CMainFrame::OnFileNewGroup() 
{
	//스크립트 에디트 창 만들고, 스크립트 정보들 읽고, 셋팅..
	CDlgEditFxg* pEditWnd = new CDlgEditFxg;
	pEditWnd->Create(IDD_DLG_GROUP, nullptr);
	pEditWnd->m_pRefFrm = this;

	pEditWnd->ShowWindow(TRUE);
	m_pEditGroup.push_back(pEditWnd);
}

void CMainFrame::OnFileLoadGroup() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, "N3FXGroup", nullptr, dwFlags, "N3FXGroup File(*.n3fxgroup)|*.n3fxgroup||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	POSITION pos = dlg.GetStartPosition();

	while(pos != nullptr)
	{
		CString PathName = dlg.GetNextPathName(pos);

		//스크립트 에디트 창 만들고, 스크립트 정보들 읽고, 셋팅..
		CDlgEditFxg* pEditWnd = new CDlgEditFxg;
		pEditWnd->Create(IDD_DLG_GROUP, nullptr);
		pEditWnd->m_pRefFrm = this;

		// load 해서 성공하면 그대로 하고 실패하면 다 지워버려..
		if(pEditWnd->LoadScript((LPCTSTR)PathName))
		{
			pEditWnd->ShowWindow(TRUE);
			m_pEditGroup.push_back(pEditWnd);			
		}
		else
		{
			MessageBox("n3fxgroup파일을 여는데 실패했습니다.","-.-;;",MB_OK);
			pEditWnd->DestroyWindow();
			delete pEditWnd;
		}
	}
}
