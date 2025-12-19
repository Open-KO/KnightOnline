// N3Board2DGrass.h: interface for the GrassBoard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRASSBOARD_H__D53F0EC4_B777_49CD_BEE8_071AD4A1680E__INCLUDED_)
#define AFX_GRASSBOARD_H__D53F0EC4_B777_49CD_BEE8_071AD4A1680E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Transform.h>

const uint32_t BOARD_X				= 0;
const uint32_t BOARD_Y				= 1;
const uint32_t BOARD_Z				= 2;
const uint32_t BOARD_XYZ			= 3;

class CN3Terrain;

class CGrassBoard : public CN3Transform
{
protected:
	typedef struct Grass_Info
	{
		__Matrix44	mtxWorld;
		__Vector3	vPos;	//	풀의 위치(지도상의)
		uint32_t		dwAlpColor;	//	풀의 알파
		int			iTexIndex;	//	풀의 인덱스
	}Grass_Info;
//	__VertexXyzColorT1 m_vRects[4];
	Grass_Info		m_sGrassInfo[20];

	int				m_nTexIndex;
	uint8_t	m_ucTexIndex;	//	그림인덱스
	uint8_t	m_ucTexNum;	//	풀의 갯수

//	float			m_fLeftPo;
//	float			m_fTopPo;
	__Vector3		m_vCenterPo;

public:
	uint32_t m_dwBoardType; // Board Type

	float	m_fBrightmin;	//	unit full bright lengs
	float	m_fBrightmax;	//	unit shadow lengs	

	BOOL	m_bCamOut;	//	카메라의 범위를 벋어나 랜더 할 필요 없음(TRUE)

	BOOL	m_bGroundInfo;	//	현재 셀이 풀을 그릴수 있는지 확인

public:
//	void TexSelectNum(int Texindex,uint8_t TexOrgIndex) {m_nTexIndex = Texindex, m_usTexIndex = TexOrgIndex;}
//	BOOL ThisTexIsHave(uint8_t TexIndex) { return TexIndex & m_usTexIndex;}

	void Init(__Vector3 vPos, uint32_t dwBoardType);
	void Tick(CN3Terrain* pTerrain);
	void Render(CN3Texture** ppTex);

	void ReCalcMatrix() override;

	bool Load(File& file) override;
	void LoadFromFile(int iTexIndex,uint8_t ucTexOrgIndex,__Vector3 vPos);

	void SetBrightInit(float min,float max) {m_fBrightmin = min,m_fBrightmax = max - min;};
	uint32_t SetBrightLevel(float Level);
	
	void SetInfo(__Vector3 vBoardPosion,uint16_t usData);
//	uint16_t GetLeft() {return m_vCenterPo.x;}
//	uint16_t GetTop()  {return m_vCenterPo.z;}
	__Vector3	GetPo() {return m_vCenterPo;}

#ifdef _N3TOOL
	bool Save(File& file) override;
#endif // end of #ifdef _N3TOOL

	void Release() override;

	CGrassBoard();
	~CGrassBoard() override;

protected:
	void FindGrassIndex(const uint8_t uCGrassMngOrder, int* pnInputGrass, int& nGrassTotNum);
};

#endif // !defined(AFX_GRASSBOARD_H__D53F0EC4_B777_49CD_BEE8_071AD4A1680E__INCLUDED_)
