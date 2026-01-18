// RiverMng.h: interface for the CRiverMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RIVERMNG_H__3A365AC2_8F3E_4E3A_98B4_953352C11BA3__INCLUDED_)
#define AFX_RIVERMNG_H__3A365AC2_8F3E_4E3A_98B4_953352C11BA3__INCLUDED_

#pragma once

#include <N3Base/N3BaseFileAccess.h>
#include "VtxPosDummy.h"
#include <afxtempl.h>

enum
{
	RCM_NONE   = 0,
	RCM_SELECT = 1,
	RCM_CREATE = 2
};

class CRiverMesh;
class CMainFrame;
class CDlgRiverProperty;

#include <list>
typedef std::list<CRiverMesh*>::iterator it_RiverMesh;

class CRiverMng : public CN3BaseFileAccess
{
public:
	CRiverMng(CMainFrame* pMainFrm);
	~CRiverMng() override;

	class __VertexRiver
	{
	public:
		float x, y, z;
		float nx, ny, nz;
		D3DCOLOR color;
		float u, v, u2, v2;
		void Set(float sx, float sy, float sz, float snx, float sny, float snz, D3DCOLOR scolor,
			float su, float sv, float su2, float sv2)
		{
			x = sx, y = sy, z = sz;
			nx = snx, ny = sny, nz = snz;
			color = scolor;
			u = su, v = sv;
			u2 = su2, v2 = sv2;
		}
	};

	// Attributes
public:
	CRiverMesh* GetSelRiver() const
	{
		return m_pSelRiver;
	}

	int GetRCursorMode() const
	{
		return m_RCursorMode;
	}

	int GetDrawRiverNum() const
	{
		return static_cast<int>(m_RiverMeshes.size());
	}

	it_RiverMesh GetDrawRiver();

protected:
	std::list<CRiverMesh*> m_RiverMeshes;                    // River Mesh list
	CRiverMesh* m_pSelRiver;                                 // 선택된 RiverMesh
	CTypedPtrArray<CPtrArray, __VertexXyzT2*> m_SelVtxArray; // 선택된 점들
	CVtxPosDummy m_VtxPosDummy;                              // 점을 움직일수 있는 dummy object
	BOOL m_bEditMode;                                        // 강물 편집모드인가?
	int m_RCursorMode;                 // 강을 추가?하는 것인가 점을 선택하는 것인가?

	CMainFrame* m_pMainFrm;            // mainframe pointer
	CDlgRiverProperty* m_pDlgProperty; // property dialog

	RECT m_rcSelDrag;                  // 드래그 영역
	__VertexXyzColor m_CreateLine[2];  // 강 처음 생성할때 보이는 선

									   // Operations
public:
	void Tick();
	void Render();
	void Release() override;
	bool Load(File& file) override;
	bool Save(File& file) override;

	void MakeGameFiles(File& file, float fSize);           // Game file로 저장
	CRiverMesh* CreateNewRiverMesh(__Vector3& vPos1, __Vector3& vPos2, __Vector3& vPos3,
		__Vector3& vPos4);                                 // 새로운 강을 추가한다.
	void RemoveRiverMesh(int iRiverID);                    // 선택된 강을 삭제한다.
	BOOL MouseMsgFilter(LPMSG pMsg);                       // Mouse 메세지 처리
	BOOL SetRiverID(CRiverMesh* pRiverMesh, int iRiverID); // 강의 ID를 설정
	void SetEditMode(BOOL bEditMode);                      // 강 EditMode설정
	void ExtrudeRiverEdge();
	CRiverMesh* GetRiverMesh(int iRiverID);                // RiverMesh 얻기
	void DeleteSelectedVertex();                           // 선택된 점들 지우기
	void ReCalcUV();                                       // 선택된 강의 UV좌표 다시 계산.
	void ReCalcSelectedVertex();

	void GoRiver(int iRiverID);                            // 선택된 강으로 갑니다.

protected:
	void SetSelRiver(CRiverMesh* pRiverMesh);              // RiverMesh 선택하기
	void SelectVtxByDragRect(RECT* pRect, BOOL bAdd);
};

#endif // !defined(AFX_RIVERMNG_H__3A365AC2_8F3E_4E3A_98B4_953352C11BA3__INCLUDED_)
