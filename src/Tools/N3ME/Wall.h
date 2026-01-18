// Wall.h: interface for the CWall class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WALL_H__77875B11_2DB7_42DB_8C20_95A1AEDC17B8__INCLUDED_)
#define AFX_WALL_H__77875B11_2DB7_42DB_8C20_95A1AEDC17B8__INCLUDED_

#pragma once

#include <N3Base/N3Base.h>
#include <list>

class CWall : public CN3Base
{
public:
	char m_Name[80];
	std::list<__Vector3> m_Wall;

public:
	int GetSize()
	{
		return static_cast<int>(m_Wall.size());
	}

	void Load(File& file);
	void Save(File& file);
	bool GetVertex(int idx, __Vector3* pPos); // idx째 점 가져오기..
	void AddVertex(__Vector3 Vertex);         //path더하기.
	void DelPrevVertex();

	CWall();
	~CWall() override;
};

#endif // !defined(AFX_WALL_H__77875B11_2DB7_42DB_8C20_95A1AEDC17B8__INCLUDED_)
