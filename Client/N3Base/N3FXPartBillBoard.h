// N3FXPartBillBoard.h: interface for the CN3FXPartBillBoard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTBILLBOARD_H__
#define __N3FXPARTBILLBOARD_H__

#include "N3FXPartBase.h"

class CN3FXPartBillBoard : public CN3FXPartBase  
{
public:
	static constexpr int SUPPORTED_PART_VERSION = 5;

	int					m_iNum;				//	보드의 갯수.
	float				m_fSizeX;			//	보드의 크기
	float				m_fSizeY;
	bool				m_bTexLoop;
	
	int					m_iTexIdx;
	uint32_t				m_dwCurrColor;

	float				m_fRadius;

	__VertexXyzColorT1*	m_pVB;

	bool				m_bRoateOnlyY;

	float				m_fScaleVelX;
	float				m_fScaleVelY;
	float				m_fScaleAccelX;
	float				m_fScaleAccelY;

	float				m_fCurrScaleVelX;
	float				m_fCurrScaleVelY;

	float				m_fCurrSizeX;			//	보드의 크기
	float				m_fCurrSizeY;

	__Matrix44			m_mtxRot;
	float				m_fRotBillBoardX;
	float				m_fRotBillBoardY;
	float				m_fRotBillBoardZ;

protected:
	__Vector3			m_vUnit[4];

protected:
	void	CreateVB();
	bool	IsDead();
	__Vector3 Rotate2AbsolutePos(__Vector3 vRelativePos);
	float	CameraDist();

public:
	void	Init();				//	각종 변수들을 처음 로딩한 상태로 초기화...
	void	Start();			//	파트 구동 시작.
	void	Stop();				//	파트 구동 멈춤..
	bool	Tick();				//	ticktick...
	void	Render();			//	화면에 뿌리기..
	bool	Load(HANDLE hFile);	//	게임파일 불러오기.
	bool	Save(HANDLE hFile);	//	게임파일 저장오기.
	void	Duplicate(CN3FXPartBillBoard* pSrc);

	void	SetScale(float size) { m_fSizeX = m_fCurrSizeX = size; m_fSizeY = m_fCurrSizeY = size; }
	void	SetRadius(float rad) { m_fRadius = rad; }
	
public:
	CN3FXPartBillBoard();
	virtual ~CN3FXPartBillBoard();

#ifdef _N3TOOL
	bool	ParseScript(char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3);
#endif // end of _N3TOOL
};

#endif // !defined(AFX_N3FXPARTBILLBOARD_H__3449DE4C_B687_459A_BF2C_A1FB98895B17__INCLUDED_)
