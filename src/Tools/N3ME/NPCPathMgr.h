// NPCPathMgr.h: interface for the CNPCPathMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCPATHMGR_H__927D9E56_84BF_42B3_A588_B2552510C22B__INCLUDED_)
#define AFX_NPCPATHMGR_H__927D9E56_84BF_42B3_A588_B2552510C22B__INCLUDED_

#pragma once

#include <N3Base/N3Base.h>
#include <list>

const int MAX_NPCPATH    = 31;
const float MAX_LEN_PATH = 40.0f;

class CNPCPath;
class CDlgMakeNPCPath;
class CMapMng;

class CNPCPathMgr : public CN3Base
{
protected:
	__Vector3 m_BaseCube[8];

	__VertexXyzColor m_CubeVB[36];
	__VertexXyzColor m_LineVB[2];
	__VertexXyzColor m_StartRectVB[24];
	__VertexXyzColor m_ActRectVB[24];

	__Vector3 m_LTStartVertex;
	__Vector3 m_RBStartVertex;

	__Vector3 m_LTActVertex;
	__Vector3 m_RBActVertex;

public:
	CMapMng* m_pRefMapMng;           // 지형 참조 포인터..
	std::list<CNPCPath*> m_pPaths;   // npc path들...
	CDlgMakeNPCPath* m_pDlgMakePath; // path make dialog..
	bool m_bActive;                  // 이기능이 활성화 되어 있는지...1:활성화, 0:비활성화..
	CNPCPath* m_pCurrPath;           // 현재 만들고 있는 길..or 만들려고 준비한 버퍼..

	short** m_pppRefEvent;
	bool m_bRenderMovableRegion;

protected:
	void RenderMovableRegion();
	void SetLTRB(__Vector3* pLT, __Vector3* pRB);
	void MakeRectVB(__VertexXyzColor* pVB, __Vector3 lt, __Vector3 rb, DWORD color);
	void MakeLine(__Vector3 sv, __Vector3 ev);
	void MakeCube(__Vector3 cv, D3DCOLOR color);

public:
	int GetSize() const
	{
		return static_cast<int>(m_pPaths.size());
	}

	void TransPos(float x, float z);
	void SetCurrPath(CNPCPath* pPath);
	void DelPath(CNPCPath* pPath);
	CNPCPath* GetpPath(int idx);
	void LoadFromFile(const char* FileName);
	void SaveToFile(const char* FileName);
	void MakeServerDataFile(const char* FullFileName);

	void Render();
	void UpdatePath();
	void SetActive(bool active);
	BOOL MouseMsgFilter(LPMSG pMsg);

	CNPCPathMgr(CMapMng* pMapMng);
	CNPCPathMgr();
	virtual ~CNPCPathMgr();
};

#endif // !defined(AFX_NPCPATHMGR_H__927D9E56_84BF_42B3_A588_B2552510C22B__INCLUDED_)
