#include "pch.h"
#include "PathFind.h"
#include "AIServerApp.h"

#include <cmath>

namespace AIServer
{

constexpr int LEVEL_TWO_FIND_CROSS    = 11;
constexpr int LEVEL_TWO_FIND_DIAGONAL = 10;

CPathFind::CPathFind()
{
	m_pStack   = new STACK();
	m_pOpen    = nullptr;
	m_pClosed  = nullptr;
	m_pMap     = nullptr;
	m_vMapSize = { 0, 0 };
	m_pMain    = AIServerApp::instance();
}

CPathFind::~CPathFind()
{
	ClearData();
	delete m_pStack;
}

void CPathFind::ClearData()
{
	_PathNode *t_node1 = nullptr, *t_node2 = nullptr;

	if (m_pOpen != nullptr)
	{
		t_node1 = m_pOpen->NextNode;
		while (t_node1 != nullptr)
		{
			t_node2 = t_node1->NextNode;
			delete t_node1;
			t_node1 = t_node2;
		}

		delete m_pOpen;
		m_pOpen = nullptr;
	}

	if (m_pClosed != nullptr)
	{
		t_node1 = m_pClosed->NextNode;
		while (t_node1 != nullptr)
		{
			t_node2 = t_node1->NextNode;
			delete t_node1;
			t_node1 = t_node2;
		}

		delete m_pClosed;
		m_pClosed = nullptr;
	}
}

void CPathFind::SetMap(int x, int y, int* pMap)
{
	m_vMapSize.cx = x;
	m_vMapSize.cy = y;
	m_pMap        = pMap;
}

_PathNode* CPathFind::FindPath(int start_x, int start_y, int dest_x, int dest_y)
{
	_PathNode *t_node = nullptr, *r_node = nullptr;

	//	if(!m_pMap) return nullptr;

	ClearData();

	m_pOpen   = new _PathNode();
	m_pClosed = new _PathNode();

	if (m_pOpen == nullptr || m_pClosed == nullptr)
		return nullptr;

	t_node = new _PathNode();
	if (t_node == nullptr)
		return nullptr;

	t_node->g = 0;
	t_node->h = (int) sqrt(
		(start_x - dest_x) * (start_x - dest_x) + (start_y - dest_y) * (start_y - dest_y));
	//	t_node->h = (int)max( start_x-dest_x, start_y-dest_y );
	t_node->f  = t_node->g + t_node->h;
	t_node->x  = start_x;
	t_node->y  = start_y;

	//	int maxtry = (X 이동폭 * 최대 X구간 ) + (Y 이동폭 * 최대 Y구간) + 1;
	int maxtry = abs(start_x - dest_x) * m_vMapSize.cx + abs(start_y - dest_y) * m_vMapSize.cy + 1;
	int count  = 0;

	m_pOpen->NextNode = t_node;
	while (1)
	{
		if (count > maxtry * 2)
		{
			//			BREAKPOINT();
			//TRACE(_T("패스파인드 중도포기...%d\n"), count);
			return nullptr;
		}

		count  += 1;

		r_node  = (_PathNode*) ReturnBestNode();
		if (r_node == nullptr)
			break;

		if (r_node->x == dest_x && r_node->y == dest_y)
			break;

		FindChildPath(r_node, dest_x, dest_y);
	}

	return r_node;
}

_PathNode* CPathFind::ReturnBestNode()
{
	_PathNode* tmp = nullptr;

	if (m_pOpen->NextNode == nullptr)
		return nullptr;

	tmp                 = m_pOpen->NextNode; // point to first node on m_pOpen
	m_pOpen->NextNode   = tmp->NextNode;     // Make m_pOpen point to nextnode or nullptr.

	tmp->NextNode       = m_pClosed->NextNode;
	m_pClosed->NextNode = tmp;

	return (tmp);
}

void CPathFind::FindChildPath(_PathNode* node, int dx, int dy)
{
	int x = 0, y = 0;

	// UpperLeft
	x = node->x - 1;
	y = node->y - 1;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_CROSS);

	// Upper
	x = node->x;
	y = node->y - 1;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_DIAGONAL);

	// UpperRight
	x = node->x + 1;
	y = node->y - 1;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_CROSS);

	// Right
	x = node->x + 1;
	y = node->y;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_DIAGONAL);

	// LowerRight
	x = node->x + 1;
	y = node->y + 1;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_CROSS);

	// Lower
	x = node->x;
	y = node->y + 1;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_DIAGONAL);

	// LowerLeft
	x = node->x - 1;
	y = node->y + 1;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_CROSS);

	// Left
	x = node->x - 1;
	y = node->y;
	if (IsBlankMap(x, y))
		FindChildPathSub(node, x, y, dx, dy, LEVEL_TWO_FIND_DIAGONAL);
}

void CPathFind::FindChildPathSub(_PathNode* node, int x, int y, int dx, int dy, int arg)
{
	int g = 0, c = 0;
	_PathNode *old_node = nullptr, *t_node = nullptr;

	g        = node->g + arg;

	old_node = CheckOpen(x, y);
	if (old_node != nullptr)
	{
		for (c = 0; c < 8; c++)
		{
			if (node->Child[c] == nullptr)
				break;
		}

		node->Child[c] = old_node;

		if (g < old_node->g)
		{
			old_node->Parent = node;
			old_node->g      = g;
			old_node->f      = g + old_node->h;
		}

		return;
	}

	old_node = CheckClosed(x, y);
	if (old_node != nullptr)
	{
		for (c = 0; c < 8; c++)
		{
			if (node->Child[c] == nullptr)
				break;
		}

		node->Child[c] = old_node;

		if (g < old_node->g)
		{
			old_node->Parent = node;
			old_node->g      = g;
			old_node->f      = g + old_node->h;
			PropagateDown(old_node);
		}

		return;
	}

	t_node = new _PathNode();
	if (t_node == nullptr)
		return;

	t_node->Parent = node;
	t_node->g      = g;
	//		t_node->h = (int)sqrt((x-dx)*(x-dx) + (y-dy)*(y-dy));
	t_node->h      = (int) std::max(x - dx, y - dy);
	t_node->f      = g + t_node->h;
	t_node->x      = x;
	t_node->y      = y;
	Insert(t_node);

	for (c = 0; c < 8; c++)
	{
		if (node->Child[c] == nullptr)
			break;
	}

	node->Child[c] = t_node;
}

_PathNode* CPathFind::CheckOpen(int x, int y)
{
	_PathNode* tmp = m_pOpen->NextNode;
	while (tmp != nullptr)
	{
		if (tmp->x == x && tmp->y == y)
			return tmp;

		tmp = tmp->NextNode;
	}

	return nullptr;
}

_PathNode* CPathFind::CheckClosed(int x, int y)
{
	_PathNode* tmp = m_pClosed->NextNode;
	while (tmp != nullptr)
	{
		if (tmp->x == x && tmp->y == y)
			return tmp;

		tmp = tmp->NextNode;
	}

	return nullptr;
}

void CPathFind::Insert(_PathNode* node)
{
	_PathNode *tmp1 = nullptr, *tmp2 = nullptr;
	int f = 0;

	if (m_pOpen->NextNode == nullptr)
	{
		m_pOpen->NextNode = node;
		return;
	}

	f    = node->f;
	tmp1 = m_pOpen;
	tmp2 = m_pOpen->NextNode;

	while (tmp2 != nullptr && tmp2->f < f)
	{
		tmp1 = tmp2;
		tmp2 = tmp2->NextNode;
	}

	node->NextNode = tmp2;
	tmp1->NextNode = node;
}

void CPathFind::PropagateDown(_PathNode* old)
{
	int c = 0, g = 0;
	_PathNode *child = nullptr, *parent = nullptr;

	g = old->g;
	for (c = 0; c < 8; c++)
	{
		child = old->Child[c];
		if (child == nullptr)
			break;

		if ((g + 1) < child->g)
		{
			child->g      = g + 1;
			child->f      = child->g + child->h;
			child->Parent = old;
			Push(child);
		}
	}

	while (m_pStack->NextStackPtr != nullptr)
	{
		parent = Pop();
		for (c = 0; c < 8; c++)
		{
			child = parent->Child[c];
			if (child == nullptr)
				break;

			if (parent->g + 1 < child->g)
			{
				child->g      = parent->g + 1;
				child->f      = parent->g + parent->h;
				child->Parent = parent;
				Push(child);
			}
		}
	}
}

void CPathFind::Push(_PathNode* node)
{
	if (m_pStack == nullptr)
		return;

	STACK* tmp = new STACK();
	if (tmp == nullptr)
		return;

	tmp->NodePtr           = node;
	tmp->NextStackPtr      = m_pStack->NextStackPtr;
	m_pStack->NextStackPtr = tmp;
}

_PathNode* CPathFind::Pop()
{
	STACK* t_stack         = m_pStack->NextStackPtr;
	_PathNode* t_node      = t_stack->NodePtr;

	m_pStack->NextStackPtr = t_stack->NextStackPtr;
	delete t_stack;

	return t_node;
}

bool CPathFind::IsBlankMap(int x, int y)
{
	if (x < 0 || y < 0 || x >= m_vMapSize.cx || y >= m_vMapSize.cy)
		return false;

	//bool bRet = true;
	//if(m_pMain->m_pMap->m_pMap[x][y].m_bMove > 0) bRet = false;
	//if
	return m_pMap[x * m_vMapSize.cy + y] == 0;
	//return bRet;
}

} // namespace AIServer
