// MapMng.h: interface for the CMapMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPMNG_H__995D141A_97C5_4CE7_B100_D729B35E9959__INCLUDED_)
#define AFX_MAPMNG_H__995D141A_97C5_4CE7_B100_D729B35E9959__INCLUDED_

#pragma once

// Cursor Mode
enum
{
	CM_SELECT         = 1,
	CM_OBJECT_BRUSH   = 2,
	CM_POS            = 3,
	CM_ROTATE         = 4,
	CM_SCALE          = 5,
	CM_EDIT_TERRAIN   = 6,
	CM_SMOOTH_TERRAIN = 7,
	CM_SET_DTEX       = 8,
	CM_EDIT_RIVER     = 9,
	CM_EDIT_LIGHTMAP  = 10,
	CM_MAKE_NPCPATH   = 11,
	CM_MAKE_WALL      = 12,
	CM_EDIT_EVENT     = 13,
	CM_EDIT_POND      = 14,
	CM_FLAT_TERRAIN   = 15,
	CM_EDIT_REGENUSER = 16,
	CM_EDIT_SOUND     = 17,
	CM_EDIT_SEED      = 18,
	CM_EDIT_LIGHT     = 19
};

const int NUM_UNIT_LIGHT = 2;

#include <afxtempl.h>
#include <N3Base/N3Base.h>
#include "PosDummy.h"
#include "RotDummy.h"
#include "ScaleDummy.h"
#include "RiverMng.h"
#include "PondMng.h"
#include "LyTerrain.h"
#include "RegenUser.h"
#include "SowSeedMng.h"

class CMainFrame;
class CN3Scene;
class CN3Camera;
class CN3Light;
//class CDlgSceneGraph;
class CDlgShapeList;
class CBrushDlg;
class CN3Transform;
class CN3TransformCollision;
class CDlgBase;
class CN3Base;
class CNPCPathMgr;
class CWallMgr;
class CN3ShapeMgr;
class CEventMgr;
class CRegenUser;
class CWarpMgr;
class CSoundMgr;
class CLightObjMgr;

class CMapMng : public CN3Base
{
public:
	CMapMng(CMainFrame* pMainFrm);
	virtual ~CMapMng();

	// Attributes
public:
	CN3Transform* GetSelOutputObj(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= m_SelOutputObjArray.GetSize())
			return nullptr;

		return m_SelOutputObjArray.GetAt(iIndex);
	}

	int GetSelOutputObjCount() const
	{
		return static_cast<int>(m_SelOutputObjArray.GetSize());
	}

	CLyTerrain* GetTerrain() const
	{
		return m_pTerrain;
	}

	CWnd* GetDlgSourceList() const
	{
		return (CWnd*) m_pDlgSourceList;
	}

	CWnd* GetDlgOutputList() const
	{
		return (CWnd*) m_pDlgOutputList;
	}

	CWnd* GetDlgBase() const
	{
		return (CWnd*) m_pDlgBase;
	}

	CN3Scene* GetSceneOutput()
	{
		return m_pSceneOutput;
	}

	D3DSHADEMODE m_ShadeMode;
	D3DFILLMODE m_FillMode;
	bool m_bViewWireFrame;           // 선택된 오브젝트를 와이어 프레임으로 보게 한다..
	bool m_bRenderAxisAndGrid;       // 축과 그리드를 그릴지 안그릴지 결정..

	int m_iZoneID;                   // 존 아이디...
	bool m_bHideObj;
	bool m_bLoadingComplete;         // 이게 참이어야만 렌더링을 한다..

	CMainFrame* m_pMainFrm;          // Main 윈도우
	CSowSeedMng m_SowSeedMng;
	CDlgShapeList* m_pDlgSourceList; // Object 목록을 보여줄 다이알로그
	CN3Transform* GetSelSourceObj()
	{
		return m_pSelSourceObj;
	}

protected:
	enum ENUM_EDIT_STATE
	{
		eEDIT_NOT = 0,
		eEDIT_COPY,
		eEDIT_PASTE,
		eEDIT_NUM
	};

	CLightObjMgr* m_pLightObjMgr;
	CSoundMgr* m_pSoundMgr;
	CWarpMgr* m_pWarpMgr;
	CRegenUser* m_pRegenUser;
	CEventMgr* m_pEventMgr;   //존 이벤트 관리...
	CWallMgr* m_pWall;        //존 외각 가지 못하는 벽관리..
	CNPCPathMgr* m_pNPCPath;  // NPC 길 만드는 클래스..
	CLyTerrain* m_pTerrain;   // 지형
	CN3Scene* m_pSceneSource; // source object 목록에 보여줄 것들을 담은 Scene
	CN3Scene* m_pSceneOutput; // 맵에 배치한 Object를 포함한 Scene

	//CDlgSceneGraph*		m_pDlgSourceList;			// Object 목록을 보여줄 다이알로그
	//CDlgSceneGraph*		m_pDlgOutputList;			// 맵에 배치한 Object를 보여줄 다이알로그
	CDlgShapeList* m_pDlgOutputList; // 맵에 배치한 Object를 보여줄 다이알로그
	CDlgBase* m_pDlgBase;            // 객체 등록정보 편집 대화상자..
	CBrushDlg* m_pBrushDlg;          // 지형 편집 브러쉬 옵션지정해주는 대화상자

	CN3Transform* m_pSelSourceObj;   // source object 목록에서 선택한 것
	//	CTypedPtrArray<CPtrArray, CN3TransformCollision*> m_SelOutputObjArray;	// 맵에 배치한 Object 중 선택한 것들
	CTypedPtrArray<CPtrArray, CN3Transform*>
		m_SelOutputObjArray;            // 맵에 배치한 Object 중 선택한 것들

	ENUM_EDIT_STATE m_eSelObjBackState; //	임시복사(ctrl+c)의 상태표시
	CTypedPtrArray<CPtrArray, CN3Transform*>
		m_SelOutputObjBack;             // 맵에 배치한 Object 중 선택한 것들 가지고 있음

	int m_CursorMode;                   // 마우스의 기능 모드 구분
	BOOL m_bRenderSelObj;               // source object 목록에서 선택된 것을 그릴지 안그릴지 결정

	CTransDummy* m_pDummy;              // 물체를 이동 회전 확대/축소 하는 기능을 담당하는 클래스
	CPosDummy m_PosDummy;               // 이동
	CRotDummy m_RotDummy;               // 회전
	CScaleDummy m_ScaleDummy;           // 확대/축소
	CRiverMng m_RiverMng;               // 강물 정보관리
	CPondMng m_PondMng;                 // 연못 정보관리

	RECT m_rcSelDrag;                   // 드래그 영역

	struct __Sort                       // Object picking할때 정렬하는 구조체
	{
		CN3TransformCollision* pObj;
		float fCamDist;
		__Vector3 vMin, vMax;
	};
	// Operations
public:
	void SetLight(float fLgt);
	void ActWarpMgr();
	bool IsWarpMgrActive();
	void DeleteSelectedSourceObjects();
	void DeleteOverlappedObjects();
	void DeleteUnusedFiles();
	void SetZoneID(int id)
	{
		m_iZoneID = id;
		if (m_pTerrain)
			m_pTerrain->m_iZoneID = id;
	} //지형 존 아이디 셋팅.
	void ImportShape(const char* szFullPath);
	void MakeTerrainMovableAttr(
		CN3ShapeMgr* pShapeMgr); //지형에서 갈수 있는 타일과 갈 수 없는 타일을 정리해라..
	void ImportPostDataFromScene(const char* szFileName); // Scene 에서 오브젝트 배치된걸 불러온다..
	void UpDateFP();
	void Tick();
	void Render();
	void Release();
	void SavePartition(float x, float z, float width);
	void SaveToFile(LPCTSTR lpszPathName);          // Map 파일 저장
	void LoadFromFile(LPCTSTR lpszPathName);        // Map 파일 불러오기
	BOOL MouseMsgFilter(LPMSG pMsg);                // 마우스의 기능
	void ImportTerrain(const char* szMeshFN);       // VMesh 파일에서 지형 데이터 읽어오기
	void ImportTerrainHeight(const char* szMeshFN); // VMesh 파일에서 지형의 높이값만 읽어오기..
	void MakeGameFiles(LPCTSTR lpszPathName, float fSize = 128.0f);        // 게임 데이터로 변환하기
	void MakeServerDataFiles(LPCTSTR lpszPathName);
	void SelectObject(CN3Base* pObj, BOOL IsSourceObj, BOOL bAdd = FALSE); // 객체를 선택한다.
	void RenderObjectToWindow(
		CN3TransformCollision* pObj, HWND hWnd); // 특정 윈도우에 Object를 그려준다.
	void SetCursorMode(int iMode);               // 마우스커서의 이용방법을 바꾼다.
	int GetCursorMode() const
	{
		return m_CursorMode;
	} // 마우스 커서의 이용방법을 얻는다.
	void Invalidate();                             // View 화면 갱신
	void FocusSelObj();                            // 선택된 객체로 포터스를 옮긴다.
	void FocusAll();                               // 전체 맵을 볼수 있도록 한다.
	void FocusAt(__Vector3 v);                     // 지정된 점으로 포커스 옮겨.
	void DropSelObjToTerrain();                    // 선택한 객체를 지형에 붙인다.(Y값만 조정)
	void DeleteSelObjectFromOutputScene();         // OutputScene에서 선택한 객체를 지운다.
	void RenderDragRect(RECT* rc);                 // 드래그 영역을 그린다.
	void UpdateAll();                              // source 와 output dialog를 update시킨다.

	void LoadObjectPostData(LPCTSTR lpszFileName); // Shape 배치정보를 text파일에서 읽어온다.
	void SaveObjectPostData(LPCTSTR lpszFileName); // Shape 배치정보를 text파일로 저장한다.
	void SaveObjectPostDataPartition(LPCTSTR lpszFileName, float psx, float psz, float width);
	void RenderGrid(float fGridSize, float fMaxDistance); // 맵에 일정간격으로Grid를 그려준다
	void OnSelChanged();                                  // 선택한 객체가 바뀌었을때 해줄것들
	void SelectObjectByDragRect(RECT* pRect, BOOL bAdd);  // 드래그 해서 객체 선택하기
	void LoadSourceObjects();                             // Folder 에서 Source Object 를 읽어온다.
	CN3Transform* AddChr(CN3Scene* pDestScene, const std::string& szFN,
		BOOL bGenerateChainNumber);                       // 특정Scene에 캐릭터 객체를 복사해 추가
	CN3Transform* AddShape(CN3Scene* pDestScene, const std::string& szFN,
		BOOL bGenerateChainNumber);                       // 특정Scene에 Shape 객체를 복사해 추가
	CN3Transform* AddObjectToOutputScene(CN3Transform*
			pObj); // 소스목록에서 선택한 Object를 넣으면 OutputScene으로 복사해서 넣어준다.
	CN3Camera* CameraGet();

	CN3Base* Pick(POINT point, int* pnPart);   // 객체 picking 함수
	static int SortByCameraDistance(
		const void* pArg1, const void* pArg2); // 카메라 거리에 따른 정렬함수 qsort에 이용

	//	BOOL	CameraMove(LPMSG pMsg);		// 카메라 이동에 관한 메세지 처리
	BOOL GetObjectMinMax(
		CN3Transform* pObj, __Vector3& vMin, __Vector3& vMax); // 객체의 최대최소점 찾기

	CStatusBar* GetStatusBar();

	void SetEditState(ENUM_EDIT_STATE eEditStat); //	드로우해서 선택한 오브젝트를 edit
};

#endif // !defined(AFX_MAPMNG_H__995D141A_97C5_4CE7_B100_D729B35E9959__INCLUDED_)

