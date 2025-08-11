// Wall.cpp: implementation of the CWall class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "Wall.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWall::CWall()
{
	ZeroMemory(m_Name, 80);
	m_Wall.clear();
}

CWall::~CWall()
{
	m_Wall.clear();
}

void CWall::AddVertex(__Vector3 Vertex)
{
	m_Wall.push_back(Vertex);
}

void CWall::DelPrevVertex()
{
	if(m_Wall.size()>0) m_Wall.pop_back();
}

bool CWall::GetVertex(int idx, __Vector3* pPos)
{
	if (idx < 0
		|| idx >= static_cast<int>(m_Wall.size()))
		return false;

	std::list<__Vector3>::iterator it;
	it = m_Wall.begin();

	for(int i=0; i<idx; i++)
	{
		it++;
	}
	(*pPos) = (*it);
	return true;
}

void CWall::Load(HANDLE hFile)
{
	DWORD dwRWC;
	ReadFile(hFile, m_Name, 80, &dwRWC, NULL);

	int size;
	ReadFile(hFile, &size, sizeof(int), &dwRWC, NULL);

	__Vector3 Vertex;
	m_Wall.clear();
	for(int i=0;i<size;i++)
	{
		ReadFile(hFile, &Vertex, sizeof(__Vector3), &dwRWC, NULL);
		m_Wall.push_back(Vertex);
	}
}

void CWall::Save(HANDLE hFile)
{
	DWORD dwRWC;
	WriteFile(hFile, m_Name, 80, &dwRWC, NULL);

	int size = m_Wall.size();
	WriteFile(hFile, &size, sizeof(int), &dwRWC, NULL);

	std::list<__Vector3>::iterator itVertex;

	__Vector3 Vertex;
	for(itVertex = m_Wall.begin(); itVertex != m_Wall.end(); itVertex++)
	{
		Vertex = (*itVertex);
		WriteFile(hFile, &Vertex, sizeof(__Vector3), &dwRWC, NULL);
	}
}
