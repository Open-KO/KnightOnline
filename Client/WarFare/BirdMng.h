// BirdMng.h: interface for the CBirdMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BIRDMNG_H__5307F307_1E9C_469B_BD46_52A408740383__INCLUDED_)
#define AFX_BIRDMNG_H__5307F307_1E9C_469B_BD46_52A408740383__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Base.h>

typedef struct __TABLE_BIRD
{
	uint32_t dwID; // zone ID
	char* pszTerrainFN; // Terrain FileName - Terrain, Tile Map, ColorMap....
	char* pszColorMapFN;
	char* pszObjectPostDataFN; // Object 위치 정보 - 이안에 충돌 체크 데이터도 들어 있다.
} TABLE_BIRD;


class CBird;

class CBirdMng : public CN3Base  
{
public:
	CBirdMng();
	virtual ~CBirdMng();

// Attributes
protected:
	CBird*			m_pBird;
	int				m_iBirdCount;

// Operations
public:
	virtual void	Release();
	void			Tick();
	void			Render();
	void			LoadFromFile(const std::string& szFN);
protected:

};

#endif // !defined(AFX_BIRDMNG_H__5307F307_1E9C_469B_BD46_52A408740383__INCLUDED_)
