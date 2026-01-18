// PondMng.h: interface for the CPondMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PONDMNG_H__B71BF3BD_5F4F_4E5B_9F9F_B8BD45E2685B__INCLUDED_)
#define AFX_PONDMNG_H__B71BF3BD_5F4F_4E5B_9F9F_B8BD45E2685B__INCLUDED_

#pragma once

#include <N3Base/N3BaseFileAccess.h>
#include "VtxPosDummy.h"
#include <afxtempl.h>
#include "PondMesh.h"

enum
{
	PCM_NONE   = 0,
	PCM_SELECT = 1,
	PCM_CREATE = 2,
	PCM_SELECTING
};

//class CPondMesh;
class CMainFrame;
class CDlgPondProperty;

#include <list>
typedef std::list<CPondMesh*>::iterator it_PondMesh;
typedef std::list<__Vector3*>::iterator it_SelVtxBak;

class CPondMng : public CN3BaseFileAccess
{
public:
	CPondMng(CMainFrame* pMainFrm);
	~CPondMng() override;

	struct __VertexPond
	{
	public:
		float x, y, z;
		float nx, ny, nz;
		D3DCOLOR color;
		float u, v, u2, v2;

		void Set(__VertexXyzT2& __Vtx, float snx, float sny, float snz, DWORD dwColor)
		{
			x = __Vtx.x, y = __Vtx.y, z = __Vtx.z;
			u = __Vtx.tu, v = __Vtx.tv;
			u2 = __Vtx.tu2, v2 = __Vtx.tv2;

			nx = snx, ny = sny, nz = snz;
			color = dwColor;
		}
	};

	// Attributes
public:
	CPondMesh* GetSelPond();
	int GetDrawPondNum() const
	{
		return static_cast<int>(m_PondMeshes.size());
	}

	it_PondMesh GetDrawPond();

	int GetPCursorMode() const
	{
		return m_PCursorMode;
	}

protected:
	std::list<CPondMesh*> m_PondMeshes;                      // Pond Mesh list
	std::list<CPondMesh*> m_pSelPonds;                       // 선택된 PondMesh
	CTypedPtrArray<CPtrArray, __VertexXyzT2*> m_SelVtxArray; // 선택된 점들
	std::list<__Vector3*> m_SelVtxBakArray;
	CVtxPosDummy m_VtxPosDummy;                              // 점을 움직일수 있는 dummy object
	BOOL m_bEditMode;                                        // 연못물 편집모드인가?
	BOOL m_bChooseGroup;                                     // 연못물 그룹으로 편집하는가
	BOOL m_bChooseEditPond;                                  // 현재 쓰는 연못내에서만 편집할것인가
	BOOL m_bMovePond;                                        // 선택한 연못을 통째로 움직일 것인가
	int m_PCursorMode;                // 연못을 추가?하는 것인가 점을 선택하는 것인가?

	CMainFrame* m_pMainFrm;           // mainframe pointer
	CDlgPondProperty* m_pDlgProperty; // property dialog

	RECT m_rcSelDrag;                 // 드래그 영역
	__VertexXyzColor m_CreateLine[5]; // 연못 처음 생성할때 보이는 선

	__Vector3 m_vPondsCenter;         //	연못(들)의 중간점

	BOOL m_bShift;

	// Operations
public:
	void Tick();
	void Render();
	void Release() override;
	bool Load(File& file) override;
	bool Save(File& file) override;

	BOOL MouseMsgFilter(LPMSG pMsg);             // Mouse 메세지 처리
	void MakeGameFiles(File& file, float fSize); // Game file로 저장

	CPondMesh* CreateNewPondMesh(); // 시작점과 끝난점(사각형)으로 새로운 연못을 추가한다.

	BOOL SetPondID(CPondMesh* pPondMesh, int iPondID); // 연못의 ID를 설정
	void SetEditMode(BOOL bEditMode);                  // 연못 EditMode설정
	CPondMesh* GetPondMesh(int iPondID);               // PondMesh 얻기

	void MainInvalidate();                             // 화면 강재로 다시 찍기

	//	CDlgPondProperty에서 쓰인 함수들
	void RemovePondMesh(int iPondID); // 선택된 연못을 삭제한다.
	void GoPond(int iPondID);         // 선택된 연못으로 갑니다.
	void StationPond();               // 선택된 연못의 점들을 다시 배치
	void MovePond();                  // 선택된 연못을 통째 옮기려 한다

	BOOL GetChooseGroup()
	{
		return m_bChooseGroup;
	}
	BOOL GetChooseEditPond()
	{
		return m_bChooseEditPond;
	}
	void ChooseGroupPond()
	{
		m_bChooseGroup ^= 1;
	}
	void ChooseEditPond()
	{
		m_bChooseEditPond ^= 1;
	}

	//	나중에 지울지도
	void ReCalcSelectedVertex(); // 지정된 스케일에 따라 다시 정렬
	void ReCalcUV();             // 선택된 연못의 UV좌표 다시 계산.
protected:
	void SetSelPond(CPondMesh* pPondMesh, BOOL bChooseGroup = FALSE); // PondMesh 선택하기
	BOOL SelectVtxByDragRect(RECT* pRect, BOOL bAdd, BOOL bSelectPond = FALSE);
	void ClearSelectRcAllPond(); // 모든 연못의 선택한 점들의 임시영역 초기화 하기

	void SelPondRelease();
	void SelPondDelete(CPondMesh* pPondMesh);
	void SetSelPonds(CPondMesh* pPondMesh);
	void SetRotatePonds(float fMove);

	void SetVtxCenter();

	void VtxBackupRelease();
	void SetVtxBackup();
	void ReSetVtxBackup();

	void ReSetDrawRect(__Vector3 vStrPos, __Vector3 vEndPos); //	연못을 그리기위한 영역 재정리
	void InputDummyMovePos(__Vector3 vMovePos); //	더미가 움직인 만큼 선택한 연못에 입력
};

#endif // !defined(AFX_PONDMNG_H__B71BF3BD_5F4F_4E5B_9F9F_B8BD45E2685B__INCLUDED_)
