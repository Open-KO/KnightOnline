// MapMng.cpp: implementation of the CMapMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "MapMng.h"

#include "MainFrm.h"
#include "N3MEView.h"
#include "LyTerrainDef.h"
#include "LyTerrain.h"
//#include "DlgSceneGraph.h"
#include "DlgShapeList.h"
#include "DlgBase.h"
#include "BrushDlg.h"
#include "DlgTerrainSize.h"
#include "NPCPathMgr.h"
#include "WallMgr.h"
#include "WarpMgr.h"
#include "RegenUser.h"
#include "EventMgr.h"
#include "DlgUnusedFiles.h"
#include "ProgressBar.h"
#include "SoundMgr.h"
#include "LightObjMgr.h"

#include <N3Base/N3Camera.h>
#include <N3Base/N3Chr.h>
#include <N3Base/N3EngTool.h>
#include <N3Base/N3Shape.h>
#include <N3Base/N3ShapeMgr.h>
#include <N3Base/N3Scene.h>
#include <N3Base/N3PMeshInstance.h>
#include <N3Base/Pick.h>

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// 생성자
CMapMng::CMapMng(CMainFrame* pMainFrm) : m_RiverMng(pMainFrm), m_PondMng(pMainFrm)
{
	ASSERT(pMainFrm->GetSafeHwnd());

	m_pMainFrm               = pMainFrm;
	m_pTerrain               = nullptr;

	// Path 지정하기
	m_pSceneSource           = new CN3Scene;
	m_pSceneSource->m_szName = "SourceList";
	m_pSceneSource->FileNameSet("SourceList.N3Scene");

	m_pSceneOutput   = new CN3Scene;

	//m_pDlgSourceList = new CDlgSceneGraph(m_pMainFrm, OBJ_SHAPE);
	m_pDlgSourceList = new CDlgShapeList(m_pMainFrm);
	m_pDlgSourceList->Create(IDD_SHAPE_LIST, m_pMainFrm);
	m_pDlgSourceList->SetWindowText("Object List");
	m_pDlgSourceList->m_pMapMng = this;

	//m_pDlgOutputList = new CDlgSceneGraph(m_pMainFrm, OBJ_SHAPE);
	m_pDlgOutputList            = new CDlgShapeList(m_pMainFrm);
	m_pDlgOutputList->Create(IDD_SHAPE_LIST, m_pMainFrm);
	m_pDlgOutputList->SetWindowText("Output List");
	m_pDlgOutputList->m_IsSourceObj = FALSE;
	m_pDlgOutputList->m_pMapMng     = this;

	m_pDlgBase                      = new CDlgBase(this);
	m_pDlgBase->Create(IDD_BASE, m_pMainFrm);

	m_pBrushDlg = new CBrushDlg;
	m_pBrushDlg->Create(IDD_BRUSH, m_pMainFrm);

	m_FillMode       = D3DFILL_SOLID;
	m_ShadeMode      = D3DSHADE_GOURAUD;

	m_bViewWireFrame = true;

	m_pDlgBase->UpdateInfo();

	m_pNPCPath                   = new CNPCPathMgr;
	m_pNPCPath->m_pRefMapMng     = this;

	m_pWall                      = new CWallMgr;
	m_pWall->m_pRefMapMng        = this;

	m_pEventMgr                  = new CEventMgr;
	m_pEventMgr->m_pRefMapMng    = this;

	m_pRegenUser                 = new CRegenUser;
	m_pRegenUser->m_pRefMapMng   = this;

	m_pWarpMgr                   = new CWarpMgr;
	m_pWarpMgr->m_pRefMapMng     = this;

	m_pSoundMgr                  = new CSoundMgr;
	m_pSoundMgr->m_pRefMapMng    = this;

	m_pLightObjMgr               = new CLightObjMgr;
	m_pLightObjMgr->m_pRefMapMng = this;

	m_iZoneID                    = 0;
	m_bHideObj                   = false;

	m_bLoadingComplete           = false;

	// 윈도 배치
	CRect rc(0, 0, 980, 1000);
	pMainFrm->MoveWindow(&rc);
	m_pDlgSourceList->MoveWindow(rc.right, rc.top, 300, 500);
	m_pDlgOutputList->MoveWindow(rc.right, 500, 300, 500);

	Release();
}

// 파괴자
CMapMng::~CMapMng()
{
	if (m_pSceneSource)
	{
		delete m_pSceneSource;
		m_pSceneSource = nullptr;
	}
	if (m_pDlgSourceList)
	{
		m_pDlgSourceList->DestroyWindow();
		delete m_pDlgSourceList;
		m_pDlgSourceList = nullptr;
	}
	if (m_pDlgOutputList)
	{
		m_pDlgOutputList->DestroyWindow();
		delete m_pDlgOutputList;
		m_pDlgOutputList = nullptr;
	}
	if (m_pDlgBase)
	{
		m_pDlgBase->DestroyWindow();
		delete m_pDlgBase;
		m_pDlgBase = nullptr;
	}
	if (m_pBrushDlg)
	{
		m_pBrushDlg->DestroyWindow();
		delete m_pBrushDlg;
		m_pBrushDlg = nullptr;
	}
	if (m_pSceneOutput)
	{
		delete m_pSceneOutput;
		m_pSceneOutput = nullptr;
	}
	if (m_pNPCPath)
	{
		delete m_pNPCPath;
		m_pNPCPath = nullptr;
	}
	if (m_pWall)
	{
		delete m_pWall;
		m_pWall = nullptr;
	}
	if (m_pEventMgr)
	{
		delete m_pEventMgr;
		m_pEventMgr = nullptr;
	}
	if (m_pRegenUser)
	{
		delete m_pRegenUser;
		m_pRegenUser = nullptr;
	}
	if (m_pWarpMgr)
	{
		delete m_pWarpMgr;
		m_pWarpMgr = nullptr;
	}
	if (m_pSoundMgr)
	{
		delete m_pSoundMgr;
		m_pSoundMgr = nullptr;
	}
	if (m_pLightObjMgr)
	{
		delete m_pLightObjMgr;
		m_pLightObjMgr = nullptr;
	}
}

// 초기화
void CMapMng::Release()
{
	if (m_pTerrain)
	{
		delete m_pTerrain;
		m_pTerrain = nullptr;
	}
	if (m_pSceneOutput)
	{
		m_pSceneOutput->Release();
		m_pSceneOutput->DefaultCameraAdd();
		m_pSceneOutput->m_bDisableDefaultLight = true;
		//m_pSceneOutput->DefaultLightAdd();
		for (int i = 0; i < NUM_UNIT_LIGHT; i++)
		{
			__ColorValue crLgt;
			crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.7f;

			CN3Light* pLight = new CN3Light;
			if (i == 0)
				pLight->m_Data.InitDirection(0, { 0, -1, 0 }, crLgt);
			else
				pLight->m_Data.InitDirection(1, { 0, 1, 0 }, crLgt);
			pLight->m_Data.bOn = true;
			m_pSceneOutput->LightAdd(pLight);
		}

		m_pSceneOutput->CameraGetActive()->m_Data.fNP = 1.0f;
		m_pSceneOutput->CameraGetActive()->m_Data.fFP = m_pMainFrm->GetFP();
	}

	m_pSelSourceObj = nullptr;
	m_SelOutputObjArray.RemoveAll();
	m_SelOutputObjBack.RemoveAll();

	m_eSelObjBackState   = eEDIT_NOT;
	m_bRenderSelObj      = FALSE;
	m_bRenderAxisAndGrid = FALSE;
	m_CursorMode         = CM_SELECT;
	m_pDummy             = nullptr;
	m_rcSelDrag.left = m_rcSelDrag.top = m_rcSelDrag.right = m_rcSelDrag.bottom = 0;

	m_RiverMng.Release();
	m_PondMng.Release();
}

//FarPlane 갱신...
void CMapMng::UpDateFP()
{
	m_pSceneOutput->CameraGetActive()->m_Data.fFP = m_pMainFrm->GetFP();
	Invalidate();
}

// m_pSceneSource 캐릭터와 오브젝트를 추가하는 함수
void CMapMng::LoadSourceObjects()
{
	ASSERT(m_pSceneSource && m_pDlgSourceList);

	m_pSceneSource->Release();

	WIN32_FIND_DATA FindFileData;

	// source\Chr 폴더의 모든 캐릭터 추가
	CString szChrPath;
	szChrPath.Format("%sChr\\", CN3Base::s_szPath.c_str());
	SetCurrentDirectory(szChrPath); // szFolder\Chr 폴더로 경로를 바꾸고..
	HANDLE hFind = FindFirstFile("*.N3Chr", &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		AddChr(m_pSceneSource, std::string(szChrPath + FindFileData.cFileName), FALSE);
		while (FindNextFile(hFind, &FindFileData))
		{
			AddChr(m_pSceneSource, std::string(szChrPath + FindFileData.cFileName), FALSE);
		}
		FindClose(hFind);
	}

	// source\Data 폴더의 모든 shape 추가
	CString szShapePath;
	szShapePath.Format("%sObject\\", CN3Base::s_szPath.c_str());
	SetCurrentDirectory(szShapePath);                  // szFolder\Mesh 폴더로 경로를 바꾸고..
	hFind = FindFirstFile("*.N3Shape", &FindFileData); // 파일 찾기.

	if (hFind != INVALID_HANDLE_VALUE)
	{
		AddShape(m_pSceneSource, std::string(szShapePath + FindFileData.cFileName), FALSE);
		while (FindNextFile(hFind, &FindFileData))
		{
			AddShape(m_pSceneSource, std::string(szShapePath + FindFileData.cFileName), FALSE);
		}
		FindClose(hFind);
	}

	m_pSceneSource->Tick();                       // Object 초기화
	m_pDlgSourceList->UpdateTree(m_pSceneSource); // 목록 갱신
}

// 지정한 Scene에 캐릭터 추가하는 함수
CN3Transform* CMapMng::AddChr(
	CN3Scene* pDestScene, const std::string& szFN, BOOL bGenerateChainNumber)
{
	CN3Chr* pChr = new CN3Chr;
	if (false == pChr->LoadFromFile(szFN)) // 부르기가 실패하면..
	{
		delete pChr;
		return nullptr;
	}

	if (bGenerateChainNumber)
	{
		int nCC          = pDestScene->ChrCount();
		int nChainNumber = 0;
		char szCompare[_MAX_PATH];
		for (int i = 0; i < nCC; i++)
		{
			lstrcpy(szCompare, pDestScene->ChrGet(i)->m_szName.c_str());
			int nL = lstrlen(szCompare);
			if (nL < 5)
				continue;

			szCompare[nL - 5] = '\0';        // 뒤에 붙는 언더바와 네자리 번호는 뺀다..
			if (pChr->m_szName == szCompare) // 이름이 같으면..
			{
				nChainNumber = atoi(&(szCompare[nL - 4])) + 1;
			}
		}

		char szName[_MAX_PATH];
		wsprintf(szName, "%s_%.4d", pChr->m_szName.c_str(), nChainNumber);
		pChr->m_szName = szName; // .. 이름을 짓는다..
	}

	pDestScene->ChrAdd(pChr);
	return pChr;
}

// 지정한 Scene에 Shape 추가하는 함수
CN3Transform* CMapMng::AddShape(
	CN3Scene* pDestScene, const std::string& szFN, BOOL bGenerateChainNumber)
{
	CN3Shape* pShape = new CN3Shape;
	if (false == pShape->LoadFromFile(szFN)) // 부르기가 실패하면..
	{
		delete pShape;
		return nullptr;
	}

	/*	if(bGenerateChainNumber)
	{
		int nSC = pDestScene->ShapeCount();
		int nChainNumber = 0;
		char szCompare[_MAX_PATH];
		for(int i = 0; i < nSC; i++)
		{
			lstrcpy(szCompare, pDestScene->ShapeGet(i)->Name());
			int nL = lstrlen(szCompare);
			if(nL < 5) continue;

			szCompare[nL-5] = nullptr; // 뒤에 붙는 언더바와 네자리 번호는 뺀다..
			if(0 == lstrcmpi(pShape->Name(), szCompare)) // 이름이 같으면..
			{
				nChainNumber = atoi(&(szCompare[nL-4])) + 1;
			}
		}

		char szName[_MAX_PATH];
		wsprintf(szName, "%s_%.4d", pShape->Name(), nChainNumber);
		pShape->m_szName = szName; // .. 이름을 짓는다..
		
		char szFileName2[_MAX_PATH];
		char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
		_splitpath(szFileName, szDrive, szDir, szFName, szExt);
		_makepath(szFileName2, szDrive, szDir, szName, szExt);
		pShape->FileNameSet(szFileName2); // 파일 이름 짓기...
	}
*/
	pDestScene->ShapeAdd(pShape); // 추가 하고
	return pShape;
}

// pObj를 m_pSceneOutput에 추가한다.
CN3Transform* CMapMng::AddObjectToOutputScene(CN3Transform* pObj)
{
	if (pObj == nullptr)
		return nullptr;
	ASSERT(m_pSceneOutput && pObj->m_szName.size());

	// m_pSceneOutput에 넣기
	CN3Transform* pDestObj;
	if (pObj->Type() & OBJ_CHARACTER)
	{
		pDestObj = AddChr(m_pSceneOutput, pObj->FileName(), TRUE);
	}
	else if (pObj->Type() & OBJ_SHAPE)
	{
		pDestObj = AddShape(m_pSceneOutput, pObj->FileName(), TRUE);
	}

	if (pDestObj)
	{
		pDestObj->PosSet(pObj->Pos());
		m_pDlgOutputList->UpdateTree(m_pSceneOutput);
		return pDestObj;
	}
	return nullptr;
}

// 선택한 객체를 지움
void CMapMng::DeleteSelObjectFromOutputScene()
{
	// 강물 편집일 경우 선택 점 지우기
	if (GetCursorMode() == CM_EDIT_RIVER)
	{
		m_RiverMng.DeleteSelectedVertex();
		return;
	}

	if (GetCursorMode() == CM_EDIT_POND)
	{
		// m_PondMng.DeleteSelectedVertex();
		return;
	}

	int iSize = static_cast<int>(m_SelOutputObjArray.GetSize());
	if (iSize == 0)
		return;

	for (int i = 0; i < iSize; i++)
	{
		CN3Transform* pSelObj = m_SelOutputObjArray.GetAt(i);
		if (pSelObj == nullptr)
			continue;

		DWORD dwType = pSelObj->Type();
		if (dwType & OBJ_CHARACTER)
			m_pSceneOutput->ChrDelete((CN3Chr*) pSelObj);
		else if (dwType & OBJ_SHAPE)
			m_pSceneOutput->ShapeDelete((CN3Shape*) pSelObj);
	}

	m_SelOutputObjArray.RemoveAll();

	if (m_pDummy != nullptr)
		m_pDummy->SetSelObj(nullptr);

	m_pDlgOutputList->UpdateTree(m_pSceneOutput);
	Invalidate();
}

void CMapMng::SavePartition(float x, float z, float width)
{
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(FALSE, "n3m", nullptr, dwFlags, "N3ME 파일(*.n3m)|*.n3m||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString lpszPathName = dlg.GetPathName();

	// 파일 이름
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath((LPCTSTR) lpszPathName, szDrive, szDir, szFName, szExt);

	//n3m만들기..^^
	char szN3M[_MAX_PATH];
	_makepath(szN3M, szDrive, szDir, szFName, "n3m");

	FileWriter file;
	if (!file.Create(szN3M))
	{
		MessageBox(::GetActiveWindow(), lpszPathName,
			"Fail to open map data file for save, Pleas retry.", MB_OK);
		return;
	}

	char comment[80] = { "이파일 여는 사람 바보..^^" };
	file.Write(&comment, sizeof(comment));
	file.Close();

	// 지형
	if (m_pTerrain)
	{
		char szTerrain[_MAX_PATH];
		_makepath(szTerrain, szDrive, szDir, szFName, "trn");
		m_pTerrain->SaveToFilePartition(szTerrain, x, z, width);
	}

	// sdt파일 저장(shape data text)
	char szSceneText[_MAX_PATH];
	_makepath(szSceneText, szDrive, szDir, szFName, "sdt");
	SaveObjectPostDataPartition(szSceneText, x, z, width);

	/*
	// warp 정보 load..
	char szWarp[_MAX_PATH];
	_makepath(szWarp, szDrive, szDir, szFName, "wap");
	m_pWarpMgr->SaveToFile(szWarp);

	//이벤트 정보 저장..
	//char szEvent[_MAX_PATH];
	//_makepath(szEvent, szDrive, szDir, szFName, "evt");
	//m_pEventMgr->SaveToFile(szEvent);	
	//*/
}

void CMapMng::SaveToFile(LPCTSTR lpszPathName)
{
	if (lstrlen(lpszPathName) == 0)
		return;

	// 파일 이름
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPathName, szDrive, szDir, szFName, szExt);

	//n3m만들기..^^
	char szN3M[_MAX_PATH];
	_makepath(szN3M, szDrive, szDir, szFName, "n3m");

	FileWriter file;
	if (!file.Create(szN3M))
	{
		MessageBox(::GetActiveWindow(), lpszPathName,
			"Fail to open map data file for save, Pleas retry.", MB_OK);
		return;
	}

	char comment[80] = { "이파일 여는 사람 바보..^^" };
	file.Write(&comment, sizeof(comment));
	file.Close();

	// 지형
	if (m_pTerrain)
	{
		char szTerrain[_MAX_PATH];
		_makepath(szTerrain, szDrive, szDir, szFName, "trn");
		m_pTerrain->SaveToFile(szTerrain);
	}

	// sdt파일 저장(shape data text)
	char szSceneText[_MAX_PATH];
	_makepath(szSceneText, szDrive, szDir, szFName, "sdt");
	SaveObjectPostData(szSceneText);

	// 강물 편집 정보 저장..
	char szRiver[_MAX_PATH];
	_makepath(szRiver, szDrive, szDir, szFName, "rvr");
	m_RiverMng.SaveToFile(szRiver);

	// 연못 편집 정보 저장..
	char szPond[_MAX_PATH];
	_makepath(szPond, szDrive, szDir, szFName, "pvr");
	m_PondMng.SaveToFile(szPond);

	//벽 정보 저장..
	char szWall[_MAX_PATH];
	_makepath(szWall, szDrive, szDir, szFName, "wal");
	m_pWall->SaveToFile(szWall);

	// warp 정보 load..
	char szWarp[_MAX_PATH];
	_makepath(szWarp, szDrive, szDir, szFName, "wap");
	m_pWarpMgr->SaveToFile(szWarp);

	// sound 정보 Load..
	char szSound[_MAX_PATH];
	_makepath(szSound, szDrive, szDir, szFName, "tsd");
	m_pSoundMgr->SaveToFile(szSound);

	// light object 정보..
	char szLightObj[_MAX_PATH];
	_makepath(szLightObj, szDrive, szDir, szFName, "tld");
	m_pLightObjMgr->SaveToFile(szLightObj);

	//이벤트 정보 저장..
	//char szEvent[_MAX_PATH];
	//_makepath(szEvent, szDrive, szDir, szFName, "evt");
	//m_pEventMgr->SaveToFile(szEvent);
}

void CMapMng::LoadFromFile(LPCTSTR lpszPathName)
{
	if (lstrlen(lpszPathName) == 0)
		return;

	// 파일 이름
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_PATH], szExt[_MAX_EXT];
	_splitpath(lpszPathName, szDrive, szDir, szFName, szExt);

	// 지형
	delete m_pTerrain;
	m_pTerrain = new CLyTerrain;
	m_pTerrain->Init();
	char szTerrain[_MAX_PATH];
	_makepath(szTerrain, szDrive, szDir, szFName, "trn");
	m_pTerrain->LoadFromFile(szTerrain);

	// Scene
	ASSERT(m_pSceneOutput);
	char szSceneText[_MAX_PATH];
	_makepath(szSceneText, szDrive, szDir, szFName, "sdt");
	LoadObjectPostData(szSceneText);

	// river
	char szRiver[_MAX_PATH];
	_makepath(szRiver, szDrive, szDir, szFName, "rvr");
	m_RiverMng.LoadFromFile(szRiver);

	// Pond
	char szPond[_MAX_PATH];
	_makepath(szPond, szDrive, szDir, szFName, "pvr");
	m_PondMng.LoadFromFile(szPond);

	//벽 정보 load..
	char szWall[_MAX_PATH];
	_makepath(szWall, szDrive, szDir, szFName, "wal");
	m_pWall->LoadFromFile(szWall);

	// warp 정보 load..
	char szWarp[_MAX_PATH];
	_makepath(szWarp, szDrive, szDir, szFName, "wap");
	m_pWarpMgr->LoadFromFile(szWarp);

	// sound 정보 Load..
	char szSound[_MAX_PATH];
	_makepath(szSound, szDrive, szDir, szFName, "tsd");
	m_pSoundMgr->LoadFromFile(szSound);

	// light object 정보..
	char szLightObj[_MAX_PATH];
	_makepath(szLightObj, szDrive, szDir, szFName, "tld");
	m_pLightObjMgr->LoadFromFile(szLightObj);

	//이벤트 정보..
	//char szEvent[_MAX_PATH];
	//_makepath(szEvent, szDrive, szDir, szFName, "evt");
	//m_pEventMgr->LoadFromFile(szEvent);
	//m_pEventMgr->SetActive(true);
	//m_pEventMgr->SetActive(false);

	m_pDlgOutputList->UpdateTree(m_pSceneOutput);
}

void CMapMng::Tick()
{
	m_pSceneOutput->Tick();
	if (m_pDummy)
		m_pDummy->Tick();
	m_RiverMng.Tick();
	m_PondMng.Tick();
	if (m_pTerrain)
		m_pTerrain->Tick();
}

void CMapMng::Render()
{
	// 로딩이 아직 안 끝났다...
	if (!m_bLoadingComplete)
		return;

	CN3EngTool* pEng = m_pMainFrm->m_pEng;
	if (pEng == nullptr)
		return;

	pEng->s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, m_FillMode);
	pEng->s_lpD3DDev->SetRenderState(D3DRS_SHADEMODE, m_ShadeMode);

	if (m_bRenderAxisAndGrid) // 축과 그리드 그리기...
	{
		pEng->RenderAxis();

		__Matrix44 mtxWorld;
		mtxWorld.Scale(32.0f, 32.0f, 32.0f);

		if (m_SelOutputObjArray.GetSize() > 0)
		{
			CN3Transform* pSelObj = m_SelOutputObjArray.GetAt(0);

			// 선택된 객체가 있으면..
			if (pSelObj != nullptr)
				mtxWorld.PosSet(0, pSelObj->Pos().y, 0); // 높이를 올린다.
		}

		pEng->RenderGrid(mtxWorld);                      // 그리드 그리기...
	}

	if (m_pTerrain)
		m_pTerrain->Render();
	if (m_pSceneOutput && !m_bHideObj)
		m_pSceneOutput->Render();

	int iSize = static_cast<int>(m_SelOutputObjArray.GetSize());
	for (int i = 0; i < iSize; i++)
	{
		CN3Transform* pSelObj = m_SelOutputObjArray.GetAt(i);
		if (pSelObj == nullptr)
			continue;

		if (pSelObj->Type() & OBJ_SHAPE)
		{
			((CN3Shape*) pSelObj)->RenderSelected(m_bViewWireFrame);
			((CN3Shape*) pSelObj)->RenderCollisionMesh(); // 충돌 메시 그리기...
			((CN3Shape*) pSelObj)->RenderClimbMesh();     // 올라가는 메시 그리기..
		}
	}

	if (m_CursorMode == CM_OBJECT_BRUSH)
	{
		if (m_pSelSourceObj && m_bRenderSelObj)
		{
			m_bRenderSelObj = FALSE;

			DWORD dwType    = m_pSelSourceObj->Type();
			if (dwType & OBJ_CHARACTER)
				((CN3Chr*) m_pSelSourceObj)->Render();
			else if (dwType & OBJ_SHAPE)
				((CN3Shape*) m_pSelSourceObj)->Render();
		}
	}
	if (m_pDummy)
		m_pDummy->Render();

	if (m_pNPCPath != nullptr && m_pNPCPath->m_bActive)
		m_pNPCPath->Render();

	if (m_pWall != nullptr && m_pWall->m_bActive)
		m_pWall->Render();

	if (m_pEventMgr != nullptr && m_pEventMgr->m_bActive)
		m_pEventMgr->Render();

	if (m_pRegenUser != nullptr && m_pRegenUser->m_bActive)
		m_pRegenUser->Render();

	if (m_pSoundMgr != nullptr && m_pSoundMgr->m_bActive)
		m_pSoundMgr->Render();

	if (m_pLightObjMgr != nullptr && m_pLightObjMgr->m_bActive)
		m_pLightObjMgr->Render();

	// 풀심기 테스트
	if (m_SowSeedMng.bActive)
		m_SowSeedMng.Render(s_lpD3DDev);

	m_RiverMng.Render();
	m_PondMng.Render();
	//	RenderGrid(256.0f, 4096);
	RenderDragRect(&m_rcSelDrag);
}

void CMapMng::RenderDragRect(RECT* rc)
{
	ASSERT(rc);
	if (rc->top == rc->bottom && rc->left == rc->right)
		return;
	__VertexTransformedColor Vertices[5];
	DWORD color = 0xff00ffff;
	Vertices[0].Set((float) rc->left, (float) rc->top, 0.0f, 0.1f, color);
	Vertices[1].Set((float) rc->right, (float) rc->top, 0.0f, 0.1f, color);
	Vertices[2].Set((float) rc->right, (float) rc->bottom, 0.0f, 0.1f, color);
	Vertices[3].Set((float) rc->left, (float) rc->bottom, 0.0f, 0.1f, color);
	Vertices[4]               = Vertices[0];

	CN3Eng* pEng              = m_pMainFrm->m_pEng;
	LPDIRECT3DDEVICE9 pD3DDev = pEng->s_lpD3DDev;

	HRESULT hr;
	hr = pD3DDev->SetTexture(0, nullptr);
	hr = pD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	hr = pD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
	hr = pD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, Vertices, sizeof(__VertexTransformedColor));
}

void CMapMng::FocusSelObj()
{
	ASSERT(m_pSceneOutput);
	CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
	int iSize          = static_cast<int>(m_SelOutputObjArray.GetSize());
	if (iSize <= 0 || pCamera == nullptr)
		return;

	__Vector3 vMin, vMax;
	vMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
	vMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	CN3Transform* pN3;
	CN3TransformCollision* pSelObj;
	for (int i = 0; i < iSize; ++i)
	{
		pN3               = m_SelOutputObjArray.GetAt(i);
		pSelObj           = (CN3TransformCollision*) pN3;

		__Vector3 vTmpMin = pSelObj->Min();
		__Vector3 vTmpMax = pSelObj->Max();

		if (vTmpMin.x < vMin.x)
			vMin.x = vTmpMin.x;
		if (vTmpMin.y < vMin.y)
			vMin.y = vTmpMin.y;
		if (vTmpMin.z < vMin.z)
			vMin.z = vTmpMin.z;
		if (vTmpMax.x > vMax.x)
			vMax.x = vTmpMax.x;
		if (vTmpMax.y > vMax.y)
			vMax.y = vTmpMax.y;
		if (vTmpMax.z > vMax.z)
			vMax.z = vTmpMax.z;
	}

	// 물체 크기에 맞춰 카메라 거리 조절
	if (vMin.x != FLT_MAX && vMax.x != -FLT_MAX)
	{
		__Vector3 vDir = pCamera->Dir();
		__Vector3 vAt  = vMin + ((vMax - vMin) / 2);
		pCamera->AtPosSet(vAt);
		pCamera->EyePosSet(vAt - vDir * (vMax - vMin).Magnitude());
	}
	//	else {ASSERT(0);}
	//	{	// 물체 크기를 알 수 없으므로 지금 거리 유지
	//		CN3TransformCollision* pSelObj = m_SelOutputObjArray.GetAt(0);
	//		pCamera->m_vRot = pSelObj->m_vPos;
	//		__Vector3 vDiff = pSelObj->m_vPos - pCamera->m_vRot;
	//		pCamera->m_vPos += vDiff;
	//	}
	Invalidate();
}

void CMapMng::FocusAt(__Vector3 v)
{
	ASSERT(m_pSceneOutput);
	CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();

	__Vector3 vDir     = pCamera->Dir();
	__Vector3 vAt      = v;

	pCamera->AtPosSet(vAt);
	pCamera->EyePosSet(vAt - vDir * 10);

	Invalidate();
}

void CMapMng::FocusAll()
{
	ASSERT(m_pSceneOutput);
	CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
	if (m_pTerrain == nullptr || pCamera == nullptr)
		return;
	SIZE size      = m_pTerrain->GetPatchNum(1.0f);
	__Vector3 vDir = pCamera->Dir();
	__Vector3 vAt;
	vAt.Set(size.cx / 2.0f, 0, size.cy / 2.0f);
	pCamera->AtPosSet(vAt);
	pCamera->EyePosSet(
		vAt
		- vDir * ((float) std::max(size.cx, size.cy)) / tanf(pCamera->m_Data.fFOV / 2.0f) / 2.0f);

	Invalidate();
}

/*
BOOL CMapMng::CameraMove(LPMSG pMsg)
{
	static int iButtonDownCount = 0;
	if (pMsg->message == WM_MOUSEWHEEL)
	{
		//short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		short zDelta = (short)((pMsg->wParam>>16)&0x0000ffff);
		CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
			
		if(pCamera)
		{
			float fD = (pCamera->AtPos() - pCamera->EyePos()).Magnitude();
			pCamera->MoveStraight(fD * zDelta * 0.001f);
			pCamera->Apply();
			return TRUE;
		}
	}
	else if (pMsg->message == WM_KEYDOWN)
	{
		CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
		switch(pMsg->wParam)
		{
		case VK_UP:
			if (pCamera) {pCamera->Move(pCamera->Dir()*10.0f); pCamera->Apply(); return TRUE;}
			break;
		case VK_DOWN:
			if (pCamera) {pCamera->Move(pCamera->Dir()*(-10.0f)); pCamera->Apply(); return TRUE;}
			break;
		case VK_RIGHT:
			if (pCamera) {pCamera->Rotate(0, -0.1f); pCamera->Apply(); return TRUE;}
			break;
		case VK_LEFT:
			if (pCamera) {pCamera->Rotate(0, 0.1f); pCamera->Apply(); return TRUE;}
			break;
		case '1':
			{
				static BOOL bSet = FALSE;
				if (pCamera)
				{
					static __Vector3 vEye(0, 0, 0);
					static __Vector3 vAt(0, 0, 1);
					static __Vector3 vUp(0, 1, 0);
					if (GetAsyncKeyState(VK_CONTROL) & 0xff00)
					{
						vEye = pCamera->EyePos();	vAt = pCamera->AtPos();	vUp = pCamera->UpVector();
						pCamera->Apply();
						bSet = TRUE;
						return FALSE;
					}
					else if (bSet)
					{
						pCamera->EyePosSet(vEye); pCamera->AtPosSet(vAt);	pCamera->UpVectorSet(vUp);
						pCamera->Apply();
						return TRUE;
					}
					else return FALSE;
				}

			}
			break;
		case '2':	// '1'과 같은 내용임 (함수로 만들지 않은 이유는 static변수 때문에..)
			{	static BOOL bSet = FALSE;
				if (pCamera) { static __Vector3 vEye(0, 0, 0);	static __Vector3 vAt(0, 0, 1); static __Vector3 vUp(0, 1, 0); 
					if (GetAsyncKeyState(VK_CONTROL) & 0xff00) { vEye = pCamera->EyePos();	vAt = pCamera->AtPos();	vUp = pCamera->UpVector(); pCamera->Apply(); bSet = TRUE; return FALSE; }
					else if (bSet) { pCamera->EyePosSet(vEye); pCamera->AtPosSet(vAt);	pCamera->UpVectorSet(vUp); pCamera->Apply(); return TRUE;}
					else return FALSE;}
			} break;
		case '3':	// '1'과 같은 내용임 (함수로 만들지 않은 이유는 static변수 때문에..)
			{	static BOOL bSet = FALSE;
				if (pCamera) { static __Vector3 vEye(0, 0, 0);	static __Vector3 vAt(0, 0, 1); static __Vector3 vUp(0, 1, 0);
					if (GetAsyncKeyState(VK_CONTROL) & 0xff00) { vEye = pCamera->EyePos();	vAt = pCamera->AtPos();	vUp = pCamera->UpVector(); pCamera->Apply(); bSet = TRUE; return FALSE; }
					else if (bSet) { pCamera->EyePosSet(vEye); pCamera->AtPosSet(vAt);	pCamera->UpVectorSet(vUp); pCamera->Apply(); return TRUE;}
					else return FALSE;}
			} break;
		case '4':	// '1'과 같은 내용임 (함수로 만들지 않은 이유는 static변수 때문에..)
			{	static BOOL bSet = FALSE;
				if (pCamera) { static __Vector3 vEye(0, 0, 0);	static __Vector3 vAt(0, 0, 1); static __Vector3 vUp(0, 1, 0);
					if (GetAsyncKeyState(VK_CONTROL) & 0xff00) { vEye = pCamera->EyePos();	vAt = pCamera->AtPos();	vUp = pCamera->UpVector(); pCamera->Apply(); bSet = TRUE; return FALSE; }
					else if (bSet) { pCamera->EyePosSet(vEye); pCamera->AtPosSet(vAt);	pCamera->UpVectorSet(vUp); pCamera->Apply(); return TRUE;}
					else return FALSE;}
			} break;
		case '5':	// '1'과 같은 내용임 (함수로 만들지 않은 이유는 static변수 때문에..)
			{	static BOOL bSet = FALSE;
				if (pCamera) { static __Vector3 vEye(0, 0, 0);	static __Vector3 vAt(0, 0, 1); static __Vector3 vUp(0, 1, 0);
					if (GetAsyncKeyState(VK_CONTROL) & 0xff00) { vEye = pCamera->EyePos();	vAt = pCamera->AtPos();	vUp = pCamera->UpVector(); pCamera->Apply(); bSet = TRUE; return FALSE; }
					else if (bSet) { pCamera->EyePosSet(vEye); pCamera->AtPosSet(vAt);	pCamera->UpVectorSet(vUp); pCamera->Apply(); return TRUE;}
					else return FALSE;}
			} break;
		}
	}
	else if (iButtonDownCount == 0 && !(GetAsyncKeyState(VK_MENU) & 0xff00) ) return FALSE;	// alt 키가 안눌렸을경우는 카메라 움직임이 아니다.

	static CPoint ptPrev;
	switch(pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (iButtonDownCount == 0)
		{
			SetCapture(pMsg->hwnd);
			ptPrev.x = short(LOWORD(pMsg->lParam)); ptPrev.y = short(HIWORD(pMsg->lParam));
		}
		++iButtonDownCount;
		return TRUE;
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		--iButtonDownCount;
		if (iButtonDownCount <= 0) ReleaseCapture();
		return TRUE;
		break;
	case WM_MOUSEMOVE:
		break;
	default:	// 마우스 메세지가 아닐경우 카메라 움직임이 아니다.
		return FALSE;
	}

	DWORD nFlags = pMsg->wParam;
	CPoint point(short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)));
	CPoint ptDelta = point - ptPrev;
	ptPrev = point;
	
	CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
	if (pCamera == nullptr) return FALSE;

	if(	(nFlags & MK_LBUTTON) && 
		(nFlags & MK_MBUTTON) ) // Alt + LB + MB
	{
		float fZoom = -(float)(ptDelta.x)/1000.0f;
		pCamera->Zoom(fZoom);
		pCamera->Apply();
		SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_ZOOM)));
		return TRUE;
	}
	else if((nFlags & MK_LBUTTON) ) // Alt + LB
	{
		float fRX = (float)(ptDelta.y)/200.0f;
		float fRY = -(float)(ptDelta.x)/200.0f;
		pCamera->LookAround(fRX, fRY);
		pCamera->Apply();
		SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_ROTATE)));
		return TRUE;
	}
	else if((nFlags & MK_RBUTTON) ) // Alt + RB
	{
		float fRX = (float)(ptDelta.y)/200.0f;
		float fRY = (float)(ptDelta.x)/200.0f;
		pCamera->Rotate(fRX, -fRY);
		pCamera->Apply();
		return TRUE;
	}
	else if((nFlags & MK_MBUTTON) ) // Alt + MB
	{
		__Vector3 vDelta = pCamera->EyePos() - pCamera->AtPos();
		float fDelta = vDelta.Magnitude() * 0.002f / pCamera->m_Data.fFOV;
		__Vector3 vD1;
		vD1.Set(-(float)(ptDelta.x), (float)(ptDelta.y), 0 );

		vD1 *= fDelta;

		pCamera->MovePlane(vD1.x, -vD1.y);
		pCamera->Apply();
		SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_MOVE)));
		return TRUE;
	}
	return FALSE;
}
*/

BOOL CMapMng::MouseMsgFilter(LPMSG pMsg)
{
	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN:
		{
			DWORD_PTR nFlags = pMsg->wParam;
			POINT point      = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
			if (m_CursorMode == CM_FLAT_TERRAIN && m_pBrushDlg->m_rdoFlatMode == 0)
			{
				__Vector3 vPos;
				if (m_pTerrain && m_pTerrain->Pick(point.x, point.y, &vPos))
				{
					m_pBrushDlg->SetFlatHeight(vPos.y);
				}
			}
		}
		break;
		case WM_KEYDOWN:
		{
			switch (pMsg->wParam)
			{
				case 0x20: //space bar...
				{
					CPoint point = ((CN3MEView*) m_pMainFrm->GetActiveView())->m_CurrMousePos;
					if (!m_pTerrain)
						break;

					__Vector3 vec;

					if (!m_pTerrain->Pick(point.x, point.y, &vec, nullptr))
						break;

					CStatusBar* pBar = GetStatusBar();
					CString str;
					str.Format("X:%4.2f Y:%4.2f Z:%4.2f", vec.x, vec.y, vec.z);
					if (pBar)
						pBar->SetPaneText(0, str);
					Invalidate();
				}
			}
		}
	}

	static BOOL bSelectDrag = FALSE;
	if (bSelectDrag == FALSE &&                    // Object drag select 모드가 아니고
		RCM_SELECT != m_RiverMng.GetRCursorMode()) // 강편집의 drag select모드가 아닐때
	{
		CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
		if (pCamera && pCamera->MoveByWindowMessage(pMsg))
			return TRUE; // 카메라 이동 메세지 거르기
	}
	if (bSelectDrag == FALSE && PCM_SELECT != m_PondMng.GetPCursorMode())
	{
		CN3Camera* pCamera = m_pSceneOutput->CameraGetActive();
		if (pCamera && pCamera->MoveByWindowMessage(pMsg))
			return TRUE;
	}

	if (m_pTerrain && m_pTerrain->MouseMsgFilter(pMsg))
		return TRUE; // 지형 편집 메세지 거르기
	if (m_RiverMng.MouseMsgFilter(pMsg))
		return TRUE; // 강 편집 메세지 거르기
	if (m_PondMng.MouseMsgFilter(pMsg))
		return TRUE; // 연못 편집 메시지 거르기
	if (m_pNPCPath && m_pNPCPath->MouseMsgFilter(pMsg))
		return TRUE; // NPC 길만들기 메세지 거르기
	if (m_pWall && m_pWall->MouseMsgFilter(pMsg))
		return TRUE;
	if (m_pEventMgr && m_pEventMgr->MouseMsgFilter(pMsg))
		return TRUE;
	if (m_pRegenUser && m_pRegenUser->MouseMsgFilter(pMsg))
		return TRUE;
	if (m_pSoundMgr && m_pSoundMgr->MouseMsgFilter(pMsg))
		return TRUE;
	if (m_pLightObjMgr && m_pLightObjMgr->MouseMsgFilter(pMsg))
		return TRUE;

	// Dummy Cube움직이는 메세지 거르기
	if (m_pDummy && m_pDummy->MouseMsgFilter(pMsg))
		return TRUE;

	{
		HWND hWnd = GetFocus();
		if (hWnd == m_pMainFrm->GetActiveView()->GetSafeHwnd())
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0xff00)
			{
				if (GetAsyncKeyState(0x43) & 0xff00) //	'c'
				{
					SetEditState(eEDIT_COPY);
				}
				else if (GetAsyncKeyState(0x56) & 0xff00) //	'v'
				{
					SetEditState(eEDIT_PASTE);
				}
			}
		}
	}

	// 나머지 객체 선택 및 배치
	switch (pMsg->message)
	{
		case WM_MOUSEMOVE:
		{
			DWORD_PTR nFlags = pMsg->wParam;
			POINT point      = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
			if (bSelectDrag)
			{
				m_rcSelDrag.right  = point.x;
				m_rcSelDrag.bottom = point.y;
				return TRUE;
			}
			else if (m_CursorMode == CM_OBJECT_BRUSH && m_pSelSourceObj)
			{
				__Vector3 vPos;
				if (m_pTerrain && m_pTerrain->Pick(point.x, point.y, &vPos))
				{
					m_pSelSourceObj->PosSet(vPos);
					m_pSelSourceObj->Tick();
					m_bRenderSelObj  = TRUE;
					CStatusBar* pBar = GetStatusBar();
					CString str;
					str.Format("X:%f Y:%f Z:%f", vPos.x, vPos.y, vPos.z);
					if (pBar)
						pBar->SetPaneText(0, str);
					return TRUE;
				}
			}
			else if (m_CursorMode == CM_ROTATE && m_pTerrain)
			{
				return TRUE;
				/*
				__Vector3 vPos;
				CN3Transform* pDestObj = nullptr;

				int j,iSize = m_SelOutputObjArray.GetSize();
				
				for(j=0;j<iSize;++j)
				{
					pDestObj = m_SelOutputObjArray.GetAt(j);
					if(pDestObj==nullptr) continue;

					vPos = pDestObj->Pos();
					vPos.y = m_pTerrain->GetHeight(vPos.x,vPos.z);
					pDestObj->PosSet(vPos);
				}
				*/
			}
		}
		break;
		case WM_LBUTTONDOWN: // 객체 선택
		{
			POINT point = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
			if (m_CursorMode == CM_OBJECT_BRUSH && m_pSelSourceObj)
			{
				__Vector3 vPos;
				if (m_pTerrain && m_pTerrain->Pick(point.x, point.y, &vPos))
				{
					m_pSelSourceObj->PosSet(vPos);
					m_bRenderSelObj = FALSE;

					AddObjectToOutputScene(m_pSelSourceObj);
					return TRUE;
				}
			}
			else if (m_CursorMode == CM_SELECT || m_CursorMode == CM_POS
					 || m_CursorMode == CM_ROTATE || m_CursorMode == CM_SCALE)
			{
				SetCapture(pMsg->hwnd);
				bSelectDrag       = TRUE;
				m_rcSelDrag.right = m_rcSelDrag.left = point.x;
				m_rcSelDrag.bottom = m_rcSelDrag.top = point.y;
				return TRUE;
			}
		}
		break;
		case WM_LBUTTONUP: // 객체 선택
		{
			if (bSelectDrag)
			{
				POINT point = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
				if (abs(m_rcSelDrag.left - point.x) < 4
					&& abs(m_rcSelDrag.top - point.y) < 4) // 클릭한걸로 간주
				{
					int nPart         = -1;
					CN3Base* pBaseSel = Pick(point, &nPart);

					SelectObject(
						pBaseSel, FALSE, (GetAsyncKeyState(VK_SHIFT) & 0xff00) ? TRUE : FALSE);
					if (m_pDlgBase && pBaseSel && pBaseSel->Type() & OBJ_SHAPE) // 파트 선택..
					{
						m_pDlgBase->m_CBShapePart.SetCurSel(nPart);
					}
				}
				else
				{ // 드레그 한 것
					if (m_rcSelDrag.left > point.x)
					{
						m_rcSelDrag.right = m_rcSelDrag.left;
						m_rcSelDrag.left  = point.x;
					}
					else
						m_rcSelDrag.right = point.x;
					if (m_rcSelDrag.top > point.y)
					{
						m_rcSelDrag.bottom = m_rcSelDrag.top;
						m_rcSelDrag.top    = point.y;
					}
					else
						m_rcSelDrag.bottom = point.y;
					SelectObjectByDragRect(
						&m_rcSelDrag, (GetAsyncKeyState(VK_SHIFT) & 0xff00) ? TRUE : FALSE);
				}
				ReleaseCapture();
				bSelectDrag      = FALSE;
				m_rcSelDrag.left = m_rcSelDrag.top = m_rcSelDrag.right = m_rcSelDrag.bottom = 0;

				m_pMainFrm->UpdateTransformInfo(); // 위치, 회전값등을 업데이트한다.

				return TRUE;
			}
		}
		break;
		case WM_RBUTTONUP: // 객체 선택 해제
		{
			if (bSelectDrag)
			{
				ReleaseCapture();
				bSelectDrag      = FALSE;
				m_rcSelDrag.left = m_rcSelDrag.top = m_rcSelDrag.right = m_rcSelDrag.bottom = 0;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

CN3Base* CMapMng::Pick(POINT point, int* pnPart) // Object Picking...
{
	static __Sort sort[MAX_SCENE_SHAPE];
	int nSortCount = 0;
	memset(sort, 0, sizeof(sort));

	CPick pick;
	pick.SetPickXY(point.x, point.y);
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();

	int nSC          = m_pSceneOutput->ShapeCount();
	CN3Shape* pShape = nullptr;
	//	CN3PMesh* pPMesh;
	__Vector3 vDist;
	int i;
	for (i = 0; i < nSC; i++)
	{
		pShape = m_pSceneOutput->ShapeGet(i);
		if (pShape == nullptr)
			continue;

		//		int nPC = pShape->PartCount();
		//		for(int j = 0; j < nPC; j++)
		//		{
		//			CN3SPart* pPart = pShape->Part(j);
		//			if (pPart == nullptr) continue;
		//			pPMesh = pPart->Mesh();
		//			if(pPMesh == nullptr) continue;
		//
		vDist                     = pShape->s_CameraData.vEye - pShape->Pos();

		sort[nSortCount].pObj     = pShape;
		sort[nSortCount].fCamDist = vDist.Magnitude();
		sort[nSortCount].vMin     = pShape->Min();
		sort[nSortCount].vMax     = pShape->Max();

		nSortCount++;
		//		}
	}
	/*
	CN3Chr* pChr;
	CN3Mesh* pMesh;
	int nCC = m_pSceneOutput->ChrCount();
	for(i = 0; i < nCC; i++)
	{
		pChr = m_pSceneOutput->ChrGet(i);
		_ASSERT(pChr);

		int nPC = pChr->PartCount();
		for(int j = 0; j < nPC; j++)
		{
			if(pChr->MeshGet(0, j) == nullptr) continue;

			pMesh = pChr->MeshBoxGet(j);
			if(pMesh == nullptr) continue;

			vDist = pChr->s_CameraData.vEye - pChr->m_vPos;

			sort[nSortCount].pObj = pChr;
			sort[nSortCount].fCamDist = vDist.Magnitude();
			sort[nSortCount].vMin = pMesh->Min() * pChr->m_Matrix;
			sort[nSortCount].vMax = pMesh->Max() * pChr->m_Matrix;

			nSortCount++;
		}
	}
*/
	qsort(sort, nSortCount, sizeof(__Sort), SortByCameraDistance);

	int nPart = -1;
	for (i = 0; i < nSortCount; i++)
	{
		//		bIntersect = pick.PickByBox(sort[i].vMin, sort[i].vMax, vI);
		nPart = sort[i].pObj->CheckCollisionPrecisely(true, point.x, point.y);
		if (nPart >= 0)
		{
			if (pnPart)
				*pnPart = nPart;
			return sort[i].pObj;
		}
	}
	return nullptr;
}

void CMapMng::SelectObject(CN3Base* pObj, BOOL IsSourceObj, BOOL bAdd)
{
	// source 선택
	if (IsSourceObj)
	{
		if (pObj != nullptr && pObj->Type() & (OBJ_CHARACTER | OBJ_SHAPE))
			m_pSelSourceObj = (CN3Transform*) pObj;
		else
			m_pSelSourceObj = nullptr;
	}
	// 이미 배치된 객체 선택
	else
	{
		CN3Transform* pSelObj = nullptr;
		if (pObj != nullptr && pObj->Type() & (OBJ_CHARACTER | OBJ_SHAPE))
		{
			pSelObj = (CN3Transform*) pObj;

			// 추가
			if (bAdd)
			{
				BOOL bAleadySelected = FALSE;
				int iSize            = static_cast<int>(m_SelOutputObjArray.GetSize());
				int i                = 0;
				for (; i < iSize; i++)
				{
					if (m_SelOutputObjArray.GetAt(i) == pSelObj)
					{
						bAleadySelected = TRUE;
						break;
					}
				}

				if (bAleadySelected)
					m_SelOutputObjArray.RemoveAt(i);          // 이미 있으므로 선택목록에서 제거
				else
					m_SelOutputObjArray.InsertAt(0, pSelObj); // 추가
			}
			// 새로 선택
			else
			{
				m_SelOutputObjArray.RemoveAll();
				m_SelOutputObjArray.Add(pSelObj);
			}
		}
		// 잘못 선택했거나 캐릭터나 shape를 선택하지 않았다.
		else
		{
			if (!bAdd)
				m_SelOutputObjArray.RemoveAll();
		}

		OnSelChanged();
	}

	// 한개를 선택했다면..
	if (m_SelOutputObjArray.GetSize() == 1)
		m_pMainFrm->UpdateTransformInfo();
}

void CMapMng::OnSelChanged()
{
	CN3Transform* pSelObj = nullptr;

	int iCount            = static_cast<int>(m_SelOutputObjArray.GetSize());
	if (iCount > 0)
		pSelObj = m_SelOutputObjArray.GetAt(0);

	if (pSelObj != nullptr)
	{
		if (m_pDlgBase != nullptr && m_pDlgBase->IsWindowVisible())
		{
			m_pDlgBase->UpdateInfo();
			m_pDlgBase->UpdateWindow();
		}

		if (iCount == 1 && m_pDlgOutputList != nullptr
			&& m_pDlgOutputList->GetSafeHwnd() != nullptr)
			//m_pDlgOutputList->SelectObject(TVI_ROOT, pSelObj);
			m_pDlgOutputList->SelectObject(pSelObj);
	}

	if (m_pDummy != nullptr)
	{
		m_pDummy->SetSelObj(pSelObj);

		for (int i = 1; i < iCount; i++)
			m_pDummy->AddSelObj(m_SelOutputObjArray.GetAt(i));
	}

	Invalidate();
}

void CMapMng::SelectObjectByDragRect(RECT* pRect, BOOL bAdd)
{
	if (pRect == nullptr || m_pSceneOutput == nullptr)
		return;

	if (!bAdd)
		m_SelOutputObjArray.RemoveAll();

	CN3Eng* pEng              = m_pMainFrm->m_pEng;
	LPDIRECT3DDEVICE9 pD3DDev = pEng->s_lpD3DDev;

	__Matrix44 matView, matProj, matVP;
	pD3DDev->GetTransform(D3DTS_VIEW, matView.toD3D());
	pD3DDev->GetTransform(D3DTS_PROJECTION, matProj.toD3D());
	matVP           = matView * matProj;

	D3DVIEWPORT9 vp = pEng->s_CameraData.vp;

	int iSC         = m_pSceneOutput->ShapeCount();
	for (int i = 0; i < iSC; i++)
	{
		CN3TransformCollision* pObj = m_pSceneOutput->ShapeGet(i);
		__Vector4 v;
		v.Transform(pObj->Pos(), matVP);
		float fScreenZ = (v.z / v.w);
		if (fScreenZ < 1.0 && fScreenZ > 0.0)
		{
			float fScreenX = ((v.x / v.w) + 1.0f) * (vp.Width) / 2.0f;
			float fScreenY = (1.0f - (v.y / v.w)) * (vp.Height) / 2.0f;
			if (fScreenX >= pRect->left && fScreenX <= pRect->right && fScreenY >= pRect->top
				&& fScreenY <= pRect->bottom)
			{
				BOOL bAleadySelected = FALSE;
				int iSize            = static_cast<int>(m_SelOutputObjArray.GetSize());
				int j                = 0;
				for (; j < iSize; j++)
				{
					if (m_SelOutputObjArray.GetAt(j) == pObj)
					{
						bAleadySelected = TRUE;
						break;
					}
				}

				if (bAleadySelected)
					m_SelOutputObjArray.RemoveAt(j);       // 이미 있으므로 선택목록에서 제거
				else
					m_SelOutputObjArray.InsertAt(0, pObj); // 추가
			}
		}
	}

	OnSelChanged();
}

int CMapMng::SortByCameraDistance(const void* pArg1, const void* pArg2)
{
	__Sort* pObj1 = (__Sort*) pArg1;
	__Sort* pObj2 = (__Sort*) pArg2;

	if (pObj1->fCamDist < pObj2->fCamDist)
		return -1;
	else if (pObj1->fCamDist > pObj2->fCamDist)
		return 1;
	else
		return 0;
}

void CMapMng::MakeGameFiles(LPCTSTR lpszPathName, float fSize)
{
	if (nullptr == m_pTerrain || nullptr == m_pSceneOutput)
		return;
	if (lstrlen(lpszPathName) == 0)
		return;

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPathName, szDrive, szDir, szFName, szExt);

	// 파일 저장.
	FileWriter file;
	if (!file.Create(lpszPathName))
	{
		MessageBox(::GetActiveWindow(), lpszPathName,
			"Fail to open map game data file for save, Pleas retry.", MB_OK);
		return;
	}

	char comment[80] = { "이파일 여는 사람 바보..^^" };
	file.Write(&m_iZoneID, sizeof(int));
	file.Write(&comment, sizeof(comment));
	file.Close();

	// 지형정보 저장
	char szTerrain[_MAX_PATH] = "";
	_makepath(szTerrain, szDrive, szDir, szFName, ".gtd");
	m_pTerrain->FileNameSet(szTerrain);

	FileWriter terrainGameFile;
	if (!terrainGameFile.Create(szTerrain))
	{
		MessageBox(::GetActiveWindow(), szTerrain, "Failed to open terrain file for save", MB_OK);
	}
	else
	{
		m_pTerrain->m_szName = szFName; // 이름을 지정한다.. 이 이름대로 저장된다.
		m_pTerrain->SaveGameData(terrainGameFile);
		char szColorMapName[_MAX_PATH];
		_makepath(szColorMapName, szDrive, szDir, szFName, ".tct");
		m_pTerrain->MakeGameColorMap(szColorMapName);

		m_RiverMng.MakeGameFiles(terrainGameFile, fSize);
		m_PondMng.MakeGameFiles(terrainGameFile, fSize);
		//m_pSoundMgr->SaveGameData(terrainGameFile);
		terrainGameFile.Close();

		char szLightMapName[_MAX_PATH];
		_makepath(szLightMapName, szDrive, szDir, szFName, ".tlt");
		m_pTerrain->MakeGameLightMap(szLightMapName);
	}

	// Shape Manager 만들고 저장...
	//	if(true == m_pSceneOutput->CheckOverlappedShapesAndReport()) // 이름이나 위치 중복 확인..
	//	{
	//		int idYesNo = ::MessageBox(::GetActiveWindow(), "중복된 오브젝트들을 삭제 하시겠습니까?", "중복된 오브젝트 처리", MB_YESNO);
	//		if(IDYES == idYesNo)
	//		{
	//			m_pSceneOutput->DeleteOverlappedShapes();
	//			m_pDlgOutputList->UpdateTree(m_pSceneOutput); // 아웃풋 리스트 체크..
	//			m_SelOutputObjArray.RemoveAll(); // 셀렉션 초기화..
	//		}
	//	}

	CN3ShapeMgr ShapeMgr;
	ShapeMgr.Create((m_pTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE,
		(m_pTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE);
	int nSC = m_pSceneOutput->ShapeCount();
	for (int i = 0; i < nSC; i++)
	{
		ShapeMgr.Add(m_pSceneOutput->ShapeGet(i)); // Shape 추가.
	}

	if (m_pWall)
		m_pWall->AddWall2Coll(&ShapeMgr);
	ShapeMgr.GenerateCollisionData(); // 충돌 메시 데이터를 생성한다...
	char szObjPosting[_MAX_PATH] = "";
	_makepath(szObjPosting, szDrive, szDir, szFName,
		".opd");                      // "Object Posting Data" - Shape Manager file 이름을 정하고..
	ShapeMgr.SaveToFile(szObjPosting);

	//이벤트 저장..
	char szEventName[_MAX_PATH] = "";
	_makepath(szEventName, szDrive, szDir, szFName, ".gev"); //
	if (!m_pEventMgr->MakeGameFile(szEventName, m_pTerrain->m_iHeightMapSize))
	{
		MessageBox(::GetActiveWindow(), lpszPathName, "Fail to make game event file...", MB_OK);
	}

	char szLightName[_MAX_PATH] = "";
	_makepath(szLightName, szDrive, szDir, szFName, ".glo"); //
	if (!m_pLightObjMgr->MakeGameFile(szLightName))
	{
		MessageBox(
			::GetActiveWindow(), lpszPathName, "Fail to make game light info file...", MB_OK);
	}
}

void CMapMng::MakeTerrainMovableAttr(CN3ShapeMgr* pShapeMgr)
{
	m_pTerrain->MakeMoveTable(m_pEventMgr->m_ppEvent); //움직임 속성 셋팅...
	pShapeMgr->MakeMoveTable(m_pEventMgr->m_ppEvent);  //움직임 속성 셋팅...
}

void CMapMng::MakeServerDataFiles(LPCTSTR lpszPathName)
{
	if (nullptr == m_pEventMgr->m_ppEvent)
	{
		m_pEventMgr->SetActive(true);
		m_pEventMgr->SetActive(false);
	}

	FileWriter file;

	if (!file.Create(lpszPathName))
	{
		m_pMainFrm->MessageBox("file write error");
		return;
	}

	//terrain 저장..
	m_pTerrain->SaveServerData(file);
	//

	// Shape Manager 만들고 저장...
	//	if(true == m_pSceneOutput->CheckOverlappedShapesAndReport()) // 이름이나 위치 중복 확인..
	//	{
	//		int idYesNo = ::MessageBox(::GetActiveWindow(), "중복된 오브젝트들을 삭제 하시겠습니까?", "중복된 오브젝트 처리", MB_YESNO);
	//		if(IDYES == idYesNo)
	//		{
	//			m_pSceneOutput->DeleteOverlappedShapes();
	//			m_pDlgOutputList->UpdateTree(m_pSceneOutput); // 아웃풋 리스트 체크..
	//			m_SelOutputObjArray.RemoveAll(); // 셀렉션 초기화..
	//		}
	//	}

	CN3ShapeMgr ShapeMgr;
	ShapeMgr.Create((m_pTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE,
		(m_pTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE);
	int nSC = m_pSceneOutput->ShapeCount();
	int i;
	for (i = 0; i < nSC; i++)
		ShapeMgr.Add(m_pSceneOutput->ShapeGet(i)); // Shape 추가.

	if (m_pWall)
		m_pWall->AddWall2Coll(&ShapeMgr);
	ShapeMgr.GenerateCollisionData(); // 충돌 메시 데이터를 생성한다..
	MakeTerrainMovableAttr(&ShapeMgr);
	ShapeMgr.SaveCollisionData(file); // 충돌 데이터만 저장...

	// Object Event 저장.
	//
	int iEventObjectCount = 0; // 먼저 갯수를 세고..
	for (i = 0; i < nSC; i++)
	{
		CN3Shape* pShape = m_pSceneOutput->ShapeGet(i);
		if (pShape->m_iEventID || pShape->m_iEventType || pShape->m_iNPC_ID
			|| pShape->m_iNPC_Status) // 이벤트가 있으면
			iEventObjectCount++;
	}

	file.Write(&iEventObjectCount, 4);
	for (i = 0; i < nSC; i++)
	{
		CN3Shape* pShape = m_pSceneOutput->ShapeGet(i);
		short sEvent     = 0;
		__Vector3 vPos;
		if (pShape->m_iEventID || pShape->m_iEventType || pShape->m_iNPC_ID
			|| pShape->m_iNPC_Status) // 이벤트가 있으면
		{
			file.Write(&(pShape->m_iBelong), 4);
			sEvent = (short) (pShape->m_iEventID);
			file.Write(&sEvent, 2);
			sEvent = (short) (pShape->m_iEventType);
			file.Write(&sEvent, 2);
			sEvent = (short) (pShape->m_iNPC_ID);
			file.Write(&sEvent, 2);
			sEvent = (short) (pShape->m_iNPC_Status);
			file.Write(&sEvent, 2);

			vPos = pShape->Pos();
			file.Write(&(vPos.x), 4);
			file.Write(&(vPos.y), 4);
			file.Write(&(vPos.z), 4);
		}
	}

	//MapTile속성 저장...
	//
	m_pEventMgr->MakeEventArray();
	/*
	for(int z=0; z<m_pTerrain->m_iHeightMapSize; z++)
	{
		for(int x=0; x<m_pTerrain->m_iHeightMapSize; x++)
		{
			file.Write(&(m_pEventMgr->m_ppEvent[x][z]), sizeof(short));
		}		
	}
	*/
	for (int x = 0; x < m_pTerrain->m_iHeightMapSize; x++)
		file.Write(m_pEventMgr->m_ppEvent[x], sizeof(short) * m_pTerrain->m_iHeightMapSize);

	m_pRegenUser->SaveServerData(file);
	m_pWarpMgr->SaveServerData(file);

	// 이벤트 정보..
	// 파일 이름
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_PATH], szExt[_MAX_EXT];
	_splitpath(lpszPathName, szDrive, szDir, szFName, szExt);

	//이벤트 정보 저장..
	char szEvent[_MAX_PATH];
	char szEvtFName[_MAX_PATH];
	sprintf(szEvtFName, "%s_Event", szFName);
	_makepath(szEvent, szDrive, szDir, szEvtFName, "txt");
	m_pEventMgr->SaveInfoTextFile(
		szEvent); //서버에서 쓰는 이벤트 아이디 조건들이 들어 있는 텍스트 파일..

	// 텍스트파일로 함 뽑아보자..
	FILE* stream = fopen("c:\\move.txt", "w");
	if (stream != nullptr)
	{
		for (int z = m_pTerrain->m_iHeightMapSize - 1; z >= 0; z--)
		{
			for (int x = 0; x < m_pTerrain->m_iHeightMapSize; x++)
			{
				int v = m_pEventMgr->m_ppEvent[x][z];
				fprintf(stream, "%d ", v);
			}
			fprintf(stream, "\n");
		}
		fclose(stream);
	}

	//뽑았다.

	/*	
	char szCollisionFN[512];
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPathName, szDrive, szDir, szFName, szExt);
	_makepath(szCollisionFN, szDrive,szDir, szFName, ".scd"); // 다른 이름으로 저장..
	CFile colFile;
	colFile.Open(szCollisionFN, CFile::modeCreate | CFile::modeWrite);
	ShapeMgr.SaveCollisionData((HANDLE)file.m_hFile); // 충돌 데이터만 저장...
	colFile.Close();
*/
}

BOOL CMapMng::GetObjectMinMax(CN3Transform* pObj, __Vector3& vMin, __Vector3& vMax)
{
	if (pObj == nullptr)
		return FALSE;
	DWORD dwType = pObj->Type();
	/*
	if ( dwType & OBJ_CHARACTER)
	{
		CN3Chr* pChr = (CN3Chr*)pObj;
		CN3Mesh* pMesh;
		vMin.Set(FLT_MAX,FLT_MAX,FLT_MAX); vMax.Set(-FLT_MAX,-FLT_MAX,-FLT_MAX);
		int nPC = pChr->PartCount();
		for(int i = 0; i < nPC; i++)
		{
			if(pChr->MeshGet(0, i) == nullptr) continue;

			pMesh = pChr->MeshBoxGet(i);
			if(pMesh == nullptr) continue;

			__Vector3 vMeshMin, vMeshMax;
			vMeshMin = pMesh->Min() * pChr->m_Matrix;
			vMeshMax = pMesh->Max() * pChr->m_Matrix;

			if (vMin.x>vMeshMin.x) vMin.x = vMeshMin.x;
			if (vMin.y>vMeshMin.y) vMin.y = vMeshMin.y;
			if (vMin.z>vMeshMin.z) vMin.z = vMeshMin.z;
			if (vMax.x<vMeshMax.x) vMax.x = vMeshMax.x;
			if (vMax.y<vMeshMax.y) vMax.y = vMeshMax.y;
			if (vMax.z<vMeshMax.z) vMax.z = vMeshMax.z;
		}
	}
	else*/
	if (dwType & OBJ_SHAPE)
	{
		CN3Shape* pShape = (CN3Shape*) pObj;
		CN3PMesh* pPMesh;
		vMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
		vMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		int nPC = pShape->PartCount();
		for (int i = 0; i < nPC; i++)
		{
			CN3SPart* pPart = pShape->Part(i);
			if (pPart == nullptr)
				continue;
			pPMesh = pPart->Mesh();
			if (pPMesh == nullptr)
				continue;

			__Vector3 vMeshMin, vMeshMax;
			vMeshMin = pPMesh->Min() * pShape->m_Matrix;
			vMeshMax = pPMesh->Max() * pShape->m_Matrix;

			if (vMin.x > vMeshMin.x)
				vMin.x = vMeshMin.x;
			if (vMin.y > vMeshMin.y)
				vMin.y = vMeshMin.y;
			if (vMin.z > vMeshMin.z)
				vMin.z = vMeshMin.z;
			if (vMax.x < vMeshMax.x)
				vMax.x = vMeshMax.x;
			if (vMax.y < vMeshMax.y)
				vMax.y = vMeshMax.y;
			if (vMax.z < vMeshMax.z)
				vMax.z = vMeshMax.z;
		}
	}
	else
		return FALSE;

	return TRUE;
}

void CMapMng::RenderObjectToWindow(CN3TransformCollision* pObj, HWND hWnd)
{
	if (pObj == nullptr || hWnd == nullptr)
		return;
	DWORD dwType = pObj->Type();
	if (!(dwType & OBJ_CHARACTER || dwType & OBJ_SHAPE))
		return;
	ASSERT(m_pMainFrm);

	CN3Eng* pEng              = m_pMainFrm->m_pEng;
	LPDIRECT3DDEVICE9 pD3DDev = pEng->s_lpD3DDev;

	// Save CameraData
	__CameraData CameraDataBackUp;
	CopyMemory(&CameraDataBackUp, &CN3Base::s_CameraData, sizeof(CameraDataBackUp));

	// clear & Set Viewport - tigger
	CRect rc;
	::GetClientRect(hWnd, &rc);
	pEng->Clear(0xff606060, rc);
	pEng->SetViewPort(rc);

	// begin
	pD3DDev->BeginScene();

	// Object의 위치 및 크기 파악
	__Vector3 vDir(-1, -1, 3);
	vDir.Normalize();
	__Vector3 vMin = pObj->Min();
	__Vector3 vMax = pObj->Max();

	__Vector3 vEye, vAt, vUp(0, 1, 0);
	vAt    = vMin + (vMax - vMin) / 2.0f;
	vAt.y -= (vMax - vMin).Magnitude() / 8.0f;
	vEye   = vAt - vDir * ((vMax - vMin).Magnitude() * 0.7f);
	//	pEng->s_CameraData.vEye = vEye;	// Shape의 LOD설정 때문에..

	// back up
	__Matrix44 mtxOldView, mtxOldProj;
	pD3DDev->GetTransform(D3DTS_VIEW, mtxOldView.toD3D());
	pD3DDev->GetTransform(D3DTS_PROJECTION, mtxOldProj.toD3D());
	DWORD dwLighting;
	pD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	// camera frustum 세팅..(Apply함수 내부에서 transform을 바꾸기때문에 이 위치에 넣어야 한다.)
	//	pEng->s_CameraData.vAt = vAt;
	//	TempCamera.s_CameraData = pEng->s_CameraData;
	CN3Camera TempCamera;
	TempCamera.EyePosSet(vEye);
	TempCamera.AtPosSet(vAt);
	TempCamera.UpVectorSet(vUp);
	TempCamera.Tick();
	TempCamera.Apply(); // 임시카메라에 데이터를 넣고 frustum 정보를 계산..

	// View Matrix 및 Projection Matrix Setting
	//	__Matrix44 viewmtx;
	//	viewmtx.LookAtLH(vEye, vAt, vUp);
	//	pD3DDev->SetTransform(D3DTS_VIEW, viewmtx.toD3D());
	//	__Matrix44 prjmtx;
	//	prjmtx.PerspectiveFovLH(DegreesToRadians(54.0f), pEng->s_CameraData.fAspect, 0.01f, 1000.0f);
	//	pD3DDev->SetTransform(D3DTS_PROJECTION, prjmtx.toD3D());

	// Set Render State
	pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 그리기
	if (dwType & OBJ_CHARACTER)
	{
		CN3Chr* pChr = (CN3Chr*) pObj;
		pChr->Tick(-1000);
		pChr->Render();
	}
	else if (dwType & OBJ_SHAPE)
	{
		CN3Shape* pShape = (CN3Shape*) pObj;
		pShape->Tick(-1000);
		pShape->Render();
	}

	CN3Base::s_AlphaMgr.Render(); // Alpha primitive 그리기...
	pD3DDev->EndScene();          // end
	pEng->Present(hWnd);          // present

	// restore (이전 상태로 되돌려주지 않으면 지형에서 picking이 제대로 되지 않는다)
	pD3DDev->SetTransform(D3DTS_VIEW, mtxOldView.toD3D());
	pD3DDev->SetTransform(D3DTS_PROJECTION, mtxOldProj.toD3D());
	CopyMemory(&CN3Base::s_CameraData, &CameraDataBackUp, sizeof(CameraDataBackUp));
	pD3DDev->SetViewport(&CN3Base::s_CameraData.vp);
	pD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
}

void CMapMng::SetCursorMode(int iMode)
{
	if (m_CursorMode == iMode)
		return;
	if (nullptr == m_pBrushDlg)
	{
		MessageBox(
			::GetActiveWindow(), "Please, Activate brush dialog", "NULL brush dialog", MB_OK);
		return;
	}

	if (m_pDummy)
		m_pDummy->SetSelObj(nullptr);

	m_pBrushDlg->SetTerrain(nullptr); // Brush 창을 보이지 않게한다.
	if (m_pTerrain)
		m_pTerrain->SetEditMode(TEM_NOT);
	if (m_pNPCPath)
		m_pNPCPath->SetActive(false);
	if (m_pWall)
		m_pWall->SetActive(false);
	if (m_pEventMgr)
		m_pEventMgr->SetActive(false);
	if (m_pRegenUser)
		m_pRegenUser->SetActive(false);
	if (m_pSoundMgr)
		m_pSoundMgr->SetActive(false);
	if (m_pLightObjMgr)
		m_pLightObjMgr->SetActive(false);

	m_RiverMng.SetEditMode(FALSE);
	m_PondMng.SetEditMode(FALSE);
	// 풀심기
	m_SowSeedMng.SetActive(FALSE);

	switch (iMode)
	{
		case CM_SELECT:
			m_pDummy = nullptr;
			break;
		case CM_OBJECT_BRUSH:
			m_pDummy = nullptr;
			break;
		case CM_POS:
			m_pDummy = &m_PosDummy;
			m_pMainFrm->UpdateTransformInfo(); // 위치, 회전값등을 업데이트한다.
			break;
		case CM_ROTATE:
			m_pDummy = &m_RotDummy;
			break;
		case CM_SCALE:
			m_pDummy = &m_ScaleDummy;
			m_pMainFrm->UpdateTransformInfo(); // 위치, 회전값등을 업데이트한다.
			break;
		case CM_EDIT_TERRAIN:
			m_pBrushDlg->SetTerrain(m_pTerrain);
			if (m_pTerrain)
				m_pTerrain->SetEditMode(TEM_HEIGHTEN);
			break;
		case CM_SMOOTH_TERRAIN:
			m_pBrushDlg->SetTerrain(m_pTerrain);
			if (m_pTerrain)
				m_pTerrain->SetEditMode(TEM_SMOOTH);
			break;
		case CM_FLAT_TERRAIN:
			m_pBrushDlg->SetTerrain(m_pTerrain);
			if (m_pTerrain)
				m_pTerrain->SetEditMode(TEM_FLAT);
			break;
		case CM_SET_DTEX:
			if (m_pTerrain)
				m_pTerrain->SetEditMode(TEM_DTEX);
			break;
		case CM_EDIT_RIVER:
			m_RiverMng.SetEditMode(TRUE);
			break;
		case CM_EDIT_POND:
			m_PondMng.SetEditMode(TRUE);
			break;
		case CM_EDIT_LIGHTMAP:
			if (m_pTerrain)
				m_pTerrain->SetEditMode(TEM_LIGHTMAP);
			break;
		case CM_MAKE_NPCPATH:
			if (m_pTerrain && m_pNPCPath)
			{
				if (m_pEventMgr && m_pNPCPath->m_pppRefEvent == nullptr)
				{
					m_pEventMgr->SetActive(true);
					m_pEventMgr->SetActive(false);

					CN3ShapeMgr ShapeMgr;
					ShapeMgr.Create((m_pTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE,
						(m_pTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE);
					int nSC = m_pSceneOutput->ShapeCount();
					for (int i = 0; i < nSC; i++)
					{
						ShapeMgr.Add(m_pSceneOutput->ShapeGet(i)); // Shape 추가.
					}
					if (m_pWall)
						m_pWall->AddWall2Coll(&ShapeMgr);
					ShapeMgr.GenerateCollisionData(); // 충돌 메시 데이터를 생성한다..

					MakeTerrainMovableAttr(&ShapeMgr);
					m_pNPCPath->m_pppRefEvent = m_pEventMgr->m_ppEvent;
				}
				m_pNPCPath->SetActive(true);
			}
			break;
		case CM_MAKE_WALL:
			if (m_pTerrain && m_pWall)
			{
				m_pWall->SetActive(true);
			}
			break;
		case CM_EDIT_EVENT:
			if (m_pTerrain && m_pEventMgr)
			{
				m_pEventMgr->SetActive(true);
			}
			break;
		case CM_EDIT_REGENUSER:
			if (m_pTerrain && m_pRegenUser)
			{
				m_pRegenUser->SetActive(true);
			}
			break;
		case CM_EDIT_SOUND:
			if (m_pTerrain && m_pSoundMgr)
			{
				m_pSoundMgr->SetActive(true);
			}
			break;
			// 풀심기
		case CM_EDIT_SEED:
		{
			m_SowSeedMng.SetActive(TRUE);
		}
		break;
		case CM_EDIT_LIGHT:
		{
			m_pLightObjMgr->SetActive(TRUE);
		}
		break;

		default:
			ASSERT(0);
			break;
	}

	m_CursorMode = iMode;

	if (m_pDummy != nullptr)
	{
		int iSize = static_cast<int>(m_SelOutputObjArray.GetSize());
		if (iSize > 0)
		{
			m_pDummy->SetSelObj(m_SelOutputObjArray.GetAt(0));
			for (int i = 1; i < iSize; i++)
				m_pDummy->AddSelObj(m_SelOutputObjArray.GetAt(i));
		}
		else
		{
			m_pDummy->SetSelObj(nullptr);
		}

		m_pDummy->m_pTerrainRef = m_pTerrain; // 더미에 지형 포인터를 넣어준다.
	}

	m_pMainFrm->GetActiveView()->Invalidate(FALSE);
}

void CMapMng::Invalidate()
{
	if (m_pMainFrm)
	{
		CView* pView = m_pMainFrm->GetActiveView();
		if (pView)
			pView->Invalidate(FALSE);
	}
}

// 선택한 객체를 지형에 붙인다.(Y값만 조정)
void CMapMng::DropSelObjToTerrain()
{
	if (m_pTerrain == nullptr)
		return;

	int iSize = static_cast<int>(m_SelOutputObjArray.GetSize());
	for (int i = 0; i < iSize; i++)
	{
		CN3Transform* pSelObj = m_SelOutputObjArray.GetAt(i);
		ASSERT(pSelObj);

		__Vector3 vPos = pSelObj->Pos();
		vPos.y         = m_pTerrain->GetHeight(vPos.x, vPos.z);
		pSelObj->PosSet(vPos);

		if (i == 0 && m_pDummy)
			m_pDummy->PosSet(vPos);
	}

	Invalidate();
}

void CMapMng::ImportTerrain(const char* szMeshFN)
{
	CDlgTerrainSize dlg;
	dlg.m_fSize = 1024.0f * 4.0f;
	if (dlg.DoModal() != IDOK)
		return;

	HCURSOR hLoadCur, hCurrentCur;
	hCurrentCur = GetCursor();
	hLoadCur    = AfxGetApp()->LoadCursor(IDC_LOAD);
	SetCursor(hLoadCur);

	if (m_pTerrain)
		m_pTerrain->Release();
	else
		m_pTerrain = new CLyTerrain;

	m_pTerrain->Import(szMeshFN, dlg.m_fSize);

	m_pEventMgr->SetActive(true);
	m_pEventMgr->SetActive(false);

	SetCursor(hCurrentCur);

	m_bLoadingComplete = true;
}

void CMapMng::ImportTerrainHeight(const char* szMeshFN)
{
	HCURSOR hLoadCur, hCurrentCur;
	hCurrentCur = GetCursor();
	hLoadCur    = AfxGetApp()->LoadCursor(IDC_LOAD);
	SetCursor(hLoadCur);

	if (!m_pTerrain)
		return;
	else
	{
		m_pTerrain->ImportHeight(szMeshFN);
	}

	SetCursor(hCurrentCur);

	m_bLoadingComplete = true;
}

void CMapMng::RenderGrid(
	float fGridSize, float fMaxDistance) // fGridSize크기로 fMaxDistance거리까지 격자를 그려준다
{
	if (m_pMainFrm == nullptr)
		return;
	CN3Eng* pEng = m_pMainFrm->m_pEng;
	if (pEng == nullptr)
		return;
	LPDIRECT3DDEVICE9 pD3DDev     = pEng->s_lpD3DDev;

	static float fPrevGridSize    = 0;
	static float fPrevMaxDistance = 0;
	static __VertexXyzColor pVertices[100 * 4];
	static int iVC = 0;

	if (fGridSize != fPrevGridSize || fMaxDistance != fPrevMaxDistance)
	{
		fPrevGridSize    = fGridSize;
		fPrevMaxDistance = fMaxDistance;
		int i, iCount = (int) (fMaxDistance / fGridSize);
		const int color = 0xff00ff00;
		iVC             = 0;
		for (i = 0; i < iCount; ++i)
		{
			pVertices[iVC++].Set(i * fGridSize, 0, 0, color);
			pVertices[iVC++].Set(i * fGridSize, 0, fMaxDistance, color);
		}
		for (i = 0; i < iCount; ++i)
		{
			pVertices[iVC++].Set(0, 0, i * fGridSize, color);
			pVertices[iVC++].Set(fMaxDistance, 0, i * fGridSize, color);
		}
	}

	// Backup state
	DWORD dwLighting, dwZEnable;
	pD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
	pD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);

	// Set Render State
	pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);

	__Matrix44 matWorld;
	matWorld.Identity();
	pD3DDev->SetTransform(D3DTS_WORLD, matWorld.toD3D());

	pD3DDev->SetTexture(0, nullptr);

	// render
	pD3DDev->SetFVF(FVF_XYZCOLOR);
	pD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, iVC / 2, pVertices, sizeof(__VertexXyzColor));

	// restore
	pD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
	pD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
}

void CMapMng::SaveObjectPostData(LPCTSTR lpszFileName)
{
	/*
	//////////////////////////////////
	// OldData
	if (m_pSceneOutput == nullptr) return;

	FILE* stream = fopen(lpszFileName, "w");
	if (stream == nullptr)
	{
		m_pMainFrm->MessageBox("파일을 만들수 없습니다.");
		return;
	}

	// 폴더 이름을 분리하고..
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszFileName, szDrive, szDir, szFName, szExt);

	int i, iSC = m_pSceneOutput->ShapeCount();
	fprintf(stream, "Shape Count : %d\n", iSC);

	for(i=0; i<iSC; ++i)
	{
		CN3Shape* pShape = m_pSceneOutput->ShapeGet(i);
		ASSERT(pShape);
		if (pShape == nullptr) continue;

		char szSFN[MAX_PATH];
		_makepath(szSFN, szDrive, szDir, pShape->Name(), ".n3shape");
		pShape->SaveToFile(szSFN); // Shape 정보 binary file로 저장..

		fprintf(stream, "%s\n", pShape->Name()); // 텍스트에 Shape 파일 이름을 쓴다..
	}
	fclose(stream);
	// OldData
	//////////////////////////////////
*/

	if (m_pSceneOutput == nullptr)
		return;

	FILE* stream = fopen(lpszFileName, "w");
	if (stream == nullptr)
	{
		m_pMainFrm->MessageBox("파일을 만들수 없습니다.");
		return;
	}

	// 폴더 이름을 분리하고..
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszFileName, szDrive, szDir, szFName, szExt); // 파일 이름과 확장자만 갖고..

	int i, iSC = m_pSceneOutput->ShapeCount();
	fprintf(stream, "Shape Post Count : %d\n", iSC);

	char szSFN[MAX_PATH] = "";
	for (i = 0; i < iSC; ++i)
	{
		CN3Shape* pShape = m_pSceneOutput->ShapeGet(i);

		_splitpath(pShape->FileName().c_str(), nullptr, nullptr, szFName,
			szExt);                                         // 파일 이름과 확장자만 갖고..
		_makepath(szSFN, nullptr, nullptr, szFName, szExt); // 파일 이름을 다시 만든다.

		__Vector3 vPos     = pShape->Pos();
		__Vector3 vScale   = pShape->Scale();
		__Quaternion qtRot = pShape->Rot();
		int iSPC           = pShape->PartCount();
		fprintf(stream,
			"FileName[ %s ] PartCount[ %d ] Position[ %f %f %f] Rotation[ %f %f %f %f ] Scale[ %f "
			"%f %f ] Belong [ %d ] Attribute [ %d %d %d %d ]\n",
			szSFN, iSPC, vPos.x, vPos.y, vPos.z, qtRot.x, qtRot.y, qtRot.z, qtRot.w, vScale.x,
			vScale.y, vScale.z, pShape->m_iBelong, pShape->m_iEventID, pShape->m_iEventType,
			pShape->m_iNPC_ID, pShape->m_iNPC_Status);
		for (int j = 0; j < iSPC; j++)
		{
			CN3SPart* pPart = pShape->Part(j);
			fprintf(stream, "\tPart - DiffuseARGB[ %f %f %f %f ] AmbientARGB[ %f %f %f %f ]\n",
				pPart->m_Mtl.Diffuse.a, pPart->m_Mtl.Diffuse.r, pPart->m_Mtl.Diffuse.g,
				pPart->m_Mtl.Diffuse.b, pPart->m_Mtl.Ambient.a, pPart->m_Mtl.Ambient.r,
				pPart->m_Mtl.Ambient.g, pPart->m_Mtl.Ambient.b);
		}
	}

	fclose(stream);
}

void CMapMng::SaveObjectPostDataPartition(LPCTSTR lpszFileName, float psx, float psz, float width)
{
	if (m_pSceneOutput == nullptr)
		return;

	FILE* stream = fopen(lpszFileName, "w");
	if (stream == nullptr)
	{
		m_pMainFrm->MessageBox("파일을 만들수 없습니다.");
		return;
	}

	// 폴더 이름을 분리하고..
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszFileName, szDrive, szDir, szFName, szExt); // 파일 이름과 확장자만 갖고..

	float sx = (int) (psx / TERRAIN_CELL_SIZE) * TERRAIN_CELL_SIZE;
	float sz = (int) (psz / TERRAIN_CELL_SIZE) * TERRAIN_CELL_SIZE;
	float ex = sx + width;
	float ez = sz + width;

	int i, iSC = m_pSceneOutput->ShapeCount();

	std::list<int> ShapeList;
	ShapeList.clear();

	CProgressBar ProgressBar;
	ProgressBar.Create("Pick out Objects.", 50, iSC);

	for (i = 0; i < iSC; ++i)
	{
		ProgressBar.StepIt();
		CN3Shape* pShape = m_pSceneOutput->ShapeGet(i);
		if (!pShape)
			continue;

		if (pShape->Pos().x > ex || pShape->Pos().x < sx || pShape->Pos().z > ez
			|| pShape->Pos().z < sz)
			continue;

		ShapeList.push_back(i);
	}

	int RealCount = static_cast<int>(ShapeList.size());
	fprintf(stream, "Shape Post Count : %d\n", RealCount);

	char szSFN[MAX_PATH] = "";

	std::list<int>::iterator it, ite;
	ite = ShapeList.end();

	ProgressBar.Create("Pick out Objects.", 50, RealCount);
	for (it = ShapeList.begin(); it != ite; it++)
	{
		ProgressBar.StepIt();
		int idx          = (*it);
		CN3Shape* pShape = m_pSceneOutput->ShapeGet(idx);

		_splitpath(pShape->FileName().c_str(), nullptr, nullptr, szFName,
			szExt);                                         // 파일 이름과 확장자만 갖고..
		_makepath(szSFN, nullptr, nullptr, szFName, szExt); // 파일 이름을 다시 만든다.

		__Vector3 vPos      = pShape->Pos();
		vPos.x             -= sx;
		vPos.z             -= sz;

		__Vector3 vScale    = pShape->Scale();
		__Quaternion qtRot  = pShape->Rot();
		int iSPC            = pShape->PartCount();
		fprintf(stream,
			"FileName[ %s ] PartCount[ %d ] Position[ %f %f %f] Rotation[ %f %f %f %f ] Scale[ %f "
			"%f %f ] Belong [ %d ] Attribute [ %d %d %d %d ]\n",
			szSFN, iSPC, vPos.x, vPos.y, vPos.z, qtRot.x, qtRot.y, qtRot.z, qtRot.w, vScale.x,
			vScale.y, vScale.z, pShape->m_iBelong, pShape->m_iEventID, pShape->m_iEventType,
			pShape->m_iNPC_ID, pShape->m_iNPC_Status);
		for (int j = 0; j < iSPC; j++)
		{
			CN3SPart* pPart = pShape->Part(j);
			fprintf(stream, "\tPart - DiffuseARGB[ %f %f %f %f ] AmbientARGB[ %f %f %f %f ]\n",
				pPart->m_Mtl.Diffuse.a, pPart->m_Mtl.Diffuse.r, pPart->m_Mtl.Diffuse.g,
				pPart->m_Mtl.Diffuse.b, pPart->m_Mtl.Ambient.a, pPart->m_Mtl.Ambient.r,
				pPart->m_Mtl.Ambient.g, pPart->m_Mtl.Ambient.b);
		}
	}

	fclose(stream);
}

void CMapMng::LoadObjectPostData(LPCTSTR lpszFileName)
{
	if (m_pSceneOutput == nullptr)
		return;

	// Scene 에 있는 오브젝트들 Release...
	m_pSceneOutput->ShapeRelease();
	m_pSceneOutput->ChrRelease();

	FILE* stream = fopen(lpszFileName, "r");
	if (stream == nullptr)
	{
		m_pMainFrm->MessageBox("지정한 텍스트 파일을 찾을 수 없습니다.");
		return;
	}

	// 폴더 이름을 분리하고..
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszFileName, szDrive, szDir, szFName, szExt);

	char szFirstLine[256];
	fgets(szFirstLine, 256, stream);           // 첫째 줄을 읽고..
	if (strstr(szFirstLine, "Shape Count : ")) // 문자열이 있으면 예전 데이터이다..
	{
		int iSC = 0, result = 0;
		sscanf(szFirstLine, "Shape Count : %d\n", &iSC);
		for (int i = 0; i < iSC; ++i)
		{
			char szDestName[_MAX_PATH];
			result = fscanf(stream, "%s\n", szDestName); // 파일 이름을 읽고..
			if (result == EOF)
				break;

			char szSFN[MAX_PATH];
			_makepath(szSFN, szDrive, szDir, szDestName, ".n3shape");

			CN3Shape* pShape = new CN3Shape;
			if (false == pShape->LoadFromFile(szSFN))
			{
				delete pShape;
				pShape = nullptr;
				continue;                               // Shape 정보 binary file로 읽기..
			}

			szDestName[lstrlen(szDestName) - 5] = '\0'; // _0000 문자열을 뺀다..
			_makepath(szSFN, nullptr, nullptr, szDestName, ".n3shape");
			pShape->m_szName = szDestName;
			pShape->FileNameSet(std::string(szSFN));    // 다시 파일 이름 설정..

			m_pSceneOutput->ShapeAdd(pShape);
		}
	}
	else // 새로 만든 데이터이다..
	{
		int i = 0, iSC = 0;
		sscanf(szFirstLine, "Shape Post Count : %d\n", &iSC);

		char szSFN[MAX_PATH] = "", szSFN2[MAX_PATH] = "";
		char szLine[1024] = "";
		for (i = 0; i < iSC; ++i)
		{
			CN3Shape* pShape = new CN3Shape();
			m_pSceneOutput->ShapeAdd(pShape); // 추가..

			int iSPC = 0, iBelong = 0, iEventID = 0, iEventType = 0, iNPC_ID = 0, iNPC_Status = 0;
			__Vector3 vPos(0, 0, 0);
			__Vector3 vScale(1, 1, 1);
			__Quaternion qtRot;
			qtRot.Identity();

			fgets(szLine, 1024, stream);
			sscanf(szLine,
				"FileName[ %s ] PartCount[ %d ] Position[ %f %f %f] Rotation[ %f %f %f %f ] Scale[ "
				"%f %f %f ] Belong [ %d ] Attribute [ %d %d %d %d ]\n",
				szSFN, &iSPC, &(vPos.x), &(vPos.y), &(vPos.z), &(qtRot.x), &(qtRot.y), &(qtRot.z),
				&(qtRot.w), &(vScale.x), &(vScale.y), &(vScale.z), &(iBelong), &(iEventID),
				&(iEventType), &(iNPC_ID), &(iNPC_Status));
			// 텍스트에 Shape 파일 이름을 쓴다..
			wsprintf(szSFN2, "Object\\%s", szSFN);
			pShape->LoadFromFile(szSFN2); // 파일에서 읽고..
			for (int j = 0; j < iSPC; j++)
			{
				fgets(szLine, 1024, stream);

				CN3SPart* pPart = pShape->Part(j);
				if (pPart)
				{
					sscanf(szLine,
						"\tPart - DiffuseARGB[ %f %f %f %f ] AmbientARGB[ %f %f %f %f ]\n",
						&(pPart->m_Mtl.Diffuse.a), &(pPart->m_Mtl.Diffuse.r),
						&(pPart->m_Mtl.Diffuse.g), &(pPart->m_Mtl.Diffuse.b),
						&(pPart->m_Mtl.Ambient.a), &(pPart->m_Mtl.Ambient.r),
						&(pPart->m_Mtl.Ambient.g), &(pPart->m_Mtl.Ambient.b));
				}
			}

			pShape->PosSet(vPos);
			pShape->RotSet(qtRot);
			pShape->ScaleSet(vScale);
			pShape->ReCalcMatrix(); // 행렬 다시 계산..

			pShape->m_iBelong     = iBelong;
			pShape->m_iEventID    = iEventID;
			pShape->m_iEventType  = iEventType;
			pShape->m_iNPC_ID     = iNPC_ID;
			pShape->m_iNPC_Status = iNPC_Status;
		}
	}

	fclose(stream);
}

CStatusBar* CMapMng::GetStatusBar()
{
	CFrameWnd* pFrame = (CFrameWnd*) AfxGetMainWnd();
	if (pFrame == nullptr || pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)) == FALSE)
		return nullptr;

	CStatusBar* pBar = (CStatusBar*) pFrame->GetMessageBar();
	if (pBar == nullptr || pBar->IsKindOf(RUNTIME_CLASS(CStatusBar)) == FALSE)
		return nullptr;

	return pBar;
}

void CMapMng::UpdateAll()
{
	if (m_pDlgOutputList)
		m_pDlgOutputList->UpdateTree(m_pSceneOutput);
	if (m_pDlgSourceList)
		m_pDlgSourceList->UpdateTree(m_pSceneSource);
}

void CMapMng::ImportPostDataFromScene(const char* szFileName)
{
	FileReader file;
	if (!file.OpenExisting(szFileName))
		return;

	int nCameraActive;
	float fFrmCur, fFrmStart, fFrmEnd;
	file.Read(&nCameraActive, 4);
	file.Read(&fFrmCur, 4);   // Animation Frame;
	file.Read(&fFrmStart, 4); // 전체 프레임.
	file.Read(&fFrmEnd, 4);   // 전체 프레임.

	int i = 0, nL = 0;
	char szName[512] = "";

	int nCC          = 0;
	file.Read(&nCC, 4); // 카메라..
	for (i = 0; i < nCC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL] = '\0';
	}

	int nLC = 0;
	file.Read(&nLC, 4); // 카메라..
	for (i = 0; i < nLC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL] = '\0';
	}

	int nSC = 0;
	file.Read(&nSC, 4); // Shapes..
	for (i = 0; i < nSC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL] = '\0';

		// Import...
		AddShape(m_pSceneOutput, szName, TRUE);
	}

	int nChrC = 0;
	file.Read(&nChrC, 4); // 캐릭터
	for (i = 0; i < nChrC; i++)
	{
		file.Read(&nL, 4);
		if (nL <= 0)
			continue;

		file.Read(szName, nL);
		szName[nL] = '\0';
	}

	m_pDlgOutputList->UpdateTree(m_pSceneOutput); // 트리 업데이트...
}

void CMapMng::ImportShape(const char* szFullPath)
{
	// Import...
	AddShape(m_pSceneOutput, szFullPath, FALSE);
}

void CMapMng::DeleteUnusedFiles()
{
	typedef std::map<std::string, CN3BaseFileAccess*> mapBase;
	typedef mapBase::value_type valBase;
	typedef mapBase::iterator it_Base;

	std::vector<std::string> invalidFNs;
	std::vector<std::string> unusedFNs;
	std::string szFN;

	//  일단 몽땅 다 맵에 넣는다..
	mapBase mBases;
	int iSC          = m_pSceneOutput->ShapeCount();

	CN3Shape* pShape = nullptr;
	CN3VMesh* pVMesh = nullptr;
	CN3SPart* pPart  = nullptr;
	CN3PMesh* pPMesh = nullptr;
	CN3Texture* pTex = nullptr;
	int i;
	for (i = 0; i < iSC; i++)
	{
		pShape = m_pSceneOutput->ShapeGet(i);
		if (nullptr == pShape)
			continue;

		szFN = CN3Base::PathGet() + pShape->FileName();
		CharLower(&(szFN[0]));
		mBases.insert(valBase(szFN, pShape));

		pVMesh = pShape->CollisionMesh();
		if (pVMesh)
		{
			szFN = CN3Base::PathGet() + pVMesh->FileName();
			CharLower(&(szFN[0]));
			mBases.insert(valBase(szFN, pVMesh));
		}
		else
		{
			invalidFNs.push_back("NULL VMesh : " + pShape->FileName());
		}

		int iSPC = pShape->PartCount();
		for (int j = 0; j < iSPC; j++)
		{
			pPart = pShape->Part(j);
			if (nullptr == pPart)
			{
				CString szErr;
				szErr.Format("NULL Part : %s - %d번째 Part", pShape->FileName().c_str(), j);
				invalidFNs.push_back(szErr.operator LPCTSTR());
				continue;
			}

			pPMesh = pPart->Mesh();
			if (pPMesh)
			{
				szFN = CN3Base::PathGet() + pPMesh->FileName();
				CharLower(&(szFN[0]));
				mBases.insert(valBase(szFN, pPMesh));
			}
			else
			{
				CString szErr;
				szErr.Format("NULL PMesh : %s - %d번째 Part", pShape->FileName().c_str(), j);
				invalidFNs.push_back(szErr.operator LPCTSTR());
			}

			int iSTC = pPart->TexCount();
			for (int k = 0; k < iSTC; k++)
			{
				pTex = pPart->Tex(k);
				if (pTex)
				{
					szFN = CN3Base::PathGet() + pTex->FileName();
					CharLower(&(szFN[0]));
					mBases.insert(valBase(szFN, pTex));
				}
				else
				{
					CString szErr;
					szErr.Format("NULL Texture : %s - %d번째 Part, %d번째 Texture",
						pShape->FileName().c_str(), j, k);
					invalidFNs.push_back(szErr.operator LPCTSTR());
					continue;
				}
			}
		}
	}

	// 파일을 찾고..
	std::string szPath = CN3Base::PathGet() + "object\\";
	::SetCurrentDirectory(szPath.c_str());
	CFileFind ff;

	BOOL bFind;
	CString szFNTmp;
	CString szFNTmp2;

	for (ff.FindFile(); bFind = ff.FindNextFile();)
	{
		szFNTmp  = ff.GetFilePath();
		szFNTmp2 = ff.GetFileName();

		if (szFNTmp2 == "." || szFNTmp2 == "..")
			continue;

		szFNTmp.MakeLower();

		szFN       = szFNTmp;
		it_Base it = mBases.find(szFN);
		if (it != mBases.end())
			continue; // 찾았으면 쓴거다..

		unusedFNs.push_back(szFN);
	}

	if (!bFind)
	{
		szFNTmp  = ff.GetFilePath();
		szFNTmp2 = ff.GetFileName();

		szFNTmp.MakeLower();

		szFN       = szFNTmp;
		it_Base it = mBases.find(szFN);
		if (it == mBases.end())
			unusedFNs.push_back(szFN);
	}

	// 파일 지우기 대화상자 띄우기..
	CDlgUnusedFiles dlg;
	for (const std::string& filename : unusedFNs)
		dlg.m_FileNames.Add(filename.c_str());

	for (const std::string& filename : invalidFNs)
		dlg.m_InvalidFileNames.Add(filename.c_str());

	dlg.DoModal();

	// 모두 업데이트..
	m_pSelSourceObj = nullptr;          // 이렇게 해주어야 뻑이 안난다.
	m_SelOutputObjArray.RemoveAll();

	LoadSourceObjects();                // Source Object 를 다시 읽고..
	UpdateAll();                        // 몽땅 업데이트...
}

void CMapMng::DeleteOverlappedObjects() // 위치가 겹친 젝트를 찾는다.
{
	std::vector<CN3Shape*> OverlappedObjects;
	int iSC = m_pSceneOutput->ShapeCount();

	CProgressBar ProgressBar;
	ProgressBar.Create("Delete overlapped objects..", 50, iSC);
	ProgressBar.SetPos(1);
	int iPosProgress = 0;

	CN3Shape *pObj1, *pObj2;
	int i;
	for (i = 0; i < iSC; i++)
	{
		pObj1 = m_pSceneOutput->ShapeGet(i);
		for (int j = i + 1; j < iSC; j++)
		{
			pObj2 = m_pSceneOutput->ShapeGet(j);
			if (pObj1->Pos() == pObj2->Pos() || (pObj1->Pos() - pObj2->Pos()).Magnitude() < 0.3f)
			{
				OverlappedObjects.push_back(pObj1);
				break;
			}
		}

		iPosProgress++;
		ProgressBar.SetPos(iPosProgress);
	}

	for (CN3Shape* pShape : OverlappedObjects)
		m_pSceneOutput->ShapeDelete(pShape); // 겹친거 지우기..

	// 업데이트...
	m_pSelSourceObj = nullptr;                        // 이렇게 해주어야 뻑이 안난다.
	m_SelOutputObjArray.RemoveAll();
	if (m_pDlgOutputList)
		m_pDlgOutputList->UpdateTree(m_pSceneOutput); // 몽땅 업데이트...
}

void CMapMng::DeleteSelectedSourceObjects()
{
	if (m_pSelSourceObj == nullptr || !(m_pSelSourceObj->Type() & OBJ_SHAPE))
		return;

	std::vector<CN3Shape*> SameObjects;
	int iSC = m_pSceneOutput->ShapeCount();
	for (int i = 0; i < iSC; i++)
	{
		CN3Shape* pObj = m_pSceneOutput->ShapeGet(i);
		if (pObj->FileName() == m_pSelSourceObj->FileName())
			SameObjects.push_back(pObj);
	}

	for (CN3Shape* pShape : SameObjects)
		m_pSceneOutput->ShapeDelete(pShape); // 겹친거 지우기..

	// 업데이트...
	m_pSceneSource->ShapeDelete((CN3Shape*) m_pSelSourceObj);
	m_pSelSourceObj = nullptr; // 이렇게 해주어야 뻑이 안난다.
	m_SelOutputObjArray.RemoveAll();
	UpdateAll();               // 몽땅 업데이트...
}

CN3Camera* CMapMng::CameraGet()
{
	if (m_pSceneOutput)
		return m_pSceneOutput->CameraGetActive();
	return nullptr;
}

void CMapMng::SetLight(float fLgt)
{
	if (m_pSceneOutput)
	{
		for (int i = 0; i < NUM_UNIT_LIGHT; i++)
		{
			CN3Light* pLight = m_pSceneOutput->LightGet(i);
			if (pLight)
			{
				__ColorValue crLgt;
				crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = fLgt;

				if (i == 0)
					pLight->m_Data.InitDirection(0, { 0, -1, 0 }, crLgt);
				else
					pLight->m_Data.InitDirection(1, { 0, 1, 0 }, crLgt);
			}
		}
		Invalidate();
	}
}

void CMapMng::ActWarpMgr()
{
	m_pWarpMgr->SetActive(true);
}

bool CMapMng::IsWarpMgrActive()
{
	return m_pWarpMgr->IsActive();
}

void CMapMng::SetEditState(ENUM_EDIT_STATE eEditStat)
{
	static __Vector3 vOldPos;
	static DWORD dwChkTime = 0;
	DWORD dwNowTick        = GetTickCount();
	if (dwNowTick - dwChkTime < 1000)
		return;
	dwChkTime = dwNowTick;

	switch (eEditStat)
	{
		default:
		case eEDIT_NOT:
		{
			//			m_SelOutputObjBack.RemoveAll();
			m_eSelObjBackState = eEDIT_NOT;
		}
		break;

		case eEDIT_COPY:
		{
			//	기존백업한거 지우고
			m_SelOutputObjBack.RemoveAll();
			vOldPos.Zero();

			//	선택한 것들을 백업하고
			CN3Transform* pDestObj = nullptr;
			int iSize              = static_cast<int>(m_SelOutputObjArray.GetSize());
			for (int j = 0; j < iSize; j++)
			{
				pDestObj = m_SelOutputObjArray.GetAt(j);
				if (pDestObj == nullptr)
					continue;

				m_SelOutputObjBack.InsertAt(0, pDestObj);

				if (vOldPos.Magnitude() == 0.0f)
					vOldPos = pDestObj->Pos();
			}
			m_eSelObjBackState = eEDIT_COPY;
		}
		break;

		case eEDIT_PASTE:
		{
			if (m_eSelObjBackState == eEDIT_COPY)
			{
				CN3Transform *pDestObj = nullptr, *pNewObj = nullptr;
				__Vector3 vNewPos, vObjPos;

				int iSize = static_cast<int>(m_SelOutputObjBack.GetSize());
				if (iSize == 0)
					return;

				//	찍을 새로운위치를 입력
				ASSERT(m_pTerrain);
				CPoint point = ((CN3MEView*) m_pMainFrm->GetActiveView())->m_CurrMousePos;
				m_pTerrain->Pick(point.x, point.y, &vNewPos);

				if (vOldPos.Magnitude() == 0.0f || vNewPos.Magnitude() == 0.0f)
					return;

				// 새로 이사갈 백터를 구함
				vNewPos -= vOldPos;

				//	더미의 새로운 좌표 입력
				if (m_pDummy != nullptr)
					m_pDummy->PosSet(m_pDummy->Pos() + vNewPos);

				m_SelOutputObjArray.RemoveAll();            // 기존 선택된 정보를 지우고
				for (int j = 0; j < iSize; j++)
				{
					pDestObj = m_SelOutputObjBack.GetAt(j); // 백업된 데이터를 찾는다
					if (pDestObj == nullptr)
						continue;

					pNewObj = AddObjectToOutputScene(
						pDestObj); // 주소의 정보로만 새로운 데이터를 만든다
					if (pNewObj == nullptr)
						continue;

					m_SelOutputObjArray.InsertAt(0, pNewObj); // 기존 정보에 새로 입력한다

					pNewObj->ScaleSet(pDestObj->Scale());     // 크기 입력
					pNewObj->RotSet(pDestObj->Rot());         // 회전각 입력

					vObjPos   = pDestObj->Pos() + vNewPos;
					vObjPos.y = m_pTerrain->GetHeight(vObjPos.x, vObjPos.z);
					pNewObj->PosSet(vObjPos); // 새로운 위치점 입력
				}

				OnSelChanged();               // 데이터입력등 새로운데이터로 갱신
			}
		}
		break;
	}
}
