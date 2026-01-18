// QTNode.h: interface for the CQTNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QTNODE_H__4B02CBDB_FCB9_4475_BF2F_7701DB658B28__INCLUDED_)
#define AFX_QTNODE_H__4B02CBDB_FCB9_4475_BF2F_7701DB658B28__INCLUDED_

#pragma once

// 쿼드트리 노드들의 상태속성..
enum eQTNODESTATE
{
	QTNODESTATE_CLIPPED = 0,
	QTNODESTATE_NOTRENDER,
	QTNODESTATE_RENDER,
	QTNODESTATE_UNKNOWN
};

enum eDIR
{
	DIR_LEFT = 0,
	DIR_TOP,
	DIR_RIGHT,
	DIR_BOTTOM,
	DIR_NUM
};

class CLyTerrain;

class CQTNode
{
protected:
	int m_CenterX;        //중심점의 높이필드배열상의 좌표.
	int m_CenterZ;        //중심점의 높이필드배열상의 좌표.

	int m_Level;          //쿼드트리상에서 깊이값..
	eQTNODESTATE m_State; //

	CQTNode* m_pFriend[DIR_NUM];
	CQTNode* m_pChild[DIR_NUM];

	CLyTerrain* m_pRefTerrain;

public:
	float m_MinY; //최저점
	float m_MaxY; //최고점
	int GetCenterX()
	{
		return m_CenterX;
	}
	int GetCenterZ()
	{
		return m_CenterZ;
	}

private:
	bool IsRender(POINT* pEye);
	inline int Distance(int sx, int sz, int tx, int tz);
	void RenderMaxLevel();
	void RenderNormalLevel();

public:
	int GetMaxLevel(eDIR dir);
	void Render();
	void Tick();
	void ClearMinMaxY();
	void SetWholeClipped();
	void SetMinMaxY();
	void SetCenterPoint(int x, int z);
	void Init(int level = 0, CLyTerrain* pTerrain = nullptr);
	void Release();
	void LinkFriend();

	CQTNode();
	virtual ~CQTNode();
};

#endif // !defined(AFX_QTNODE_H__4B02CBDB_FCB9_4475_BF2F_7701DB658B28__INCLUDED_)
