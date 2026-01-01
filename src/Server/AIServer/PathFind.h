#ifndef SERVER_AISERVER_PATHFIND_H
#define SERVER_AISERVER_PATHFIND_H

#pragma once

#include <MathUtils/GeometricStructs.h>

class _PathNode
{
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

class STACK
{
public:
	_PathNode* NodePtr;
	STACK* NextStackPtr;
};

class AIServerApp;
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
	_PathNode*		m_pOpen;
	_PathNode*		m_pClosed;
	STACK*			m_pStack;
//	int**			m_pMap;
	int*			m_pMap;
	_SIZE			m_vMapSize;

	AIServerApp*	m_pMain;
};

#endif // SERVER_AISERVER_PATHFIND_H
