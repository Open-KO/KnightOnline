// PathFind.h: interface for the CPathFind class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHFIND_H__395FDD6E_C35A_43A2_BBB2_FCDCD17E8CE8__INCLUDED_)
#define AFX_PATHFIND_H__395FDD6E_C35A_43A2_BBB2_FCDCD17E8CE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class _PathNode {
public:
	int f;
	int h;
	int g;
	int x;
	int y;
	_PathNode* Parent;
	_PathNode* Child[8];
	_PathNode* NextNode;
};

class STACK {
public:
	_PathNode* NodePtr;
	STACK* NextStackPtr;
};

class CServerDlg;

class CPathFind
{
public:
	bool IsBlankMap(int x, int y);
	void SetMap(int x, int y, int* pMap);
	void PropagateDown(_PathNode* old);
	void Insert(_PathNode* node);
	_PathNode* CheckOpen(int x, int y);
	_PathNode* CheckClosed(int x, int y);
	void FindChildPathSub(_PathNode* node, int x, int y, int dx, int dy, int arg);
	void FindChildPath(_PathNode* node, int dx, int dy);
	void ClearData();
	_PathNode* ReturnBestNode();
	_PathNode* FindPath(int start_x, int start_y, int dest_x, int dest_y);
	CPathFind();
	virtual ~CPathFind();

	void Push(_PathNode* node);
	_PathNode* Pop();

protected:
	LONG		m_lMapUse;
	_PathNode*	m_pOpen, *m_pClosed;
	STACK*		m_pStack;
//	int**		m_pMap;
	int*		m_pMap;
	CSize		m_vMapSize;

	CServerDlg*	m_pMain;
};

#endif // !defined(AFX_PATHFIND_H__395FDD6E_C35A_43A2_BBB2_FCDCD17E8CE8__INCLUDED_)
