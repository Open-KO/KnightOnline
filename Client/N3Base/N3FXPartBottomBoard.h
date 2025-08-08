// N3FXPartBottomBoard.h: interface for the CN3FXPartBottomBoard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTBOTTOMBOARD_H__
#define __N3FXPARTBOTTOMBOARD_H__

#include "N3BaseFileAccess.h"
#include "N3FXDef.h"
#include "N3FXPartBase.h"

//
//	바닥에 그리는 판때기..
//	triangle fan으로 그리고 삼각형 여덟개로 그린다..
//

class CN3FXPartBottomBoard : public CN3FXPartBase  
{
public:
	static constexpr int SUPPORTED_PART_VERSION = 1;

	float				m_fSizeX;			//	보드의 크기
	float				m_fSizeZ;

	bool				m_bTexLoop;
	int					m_iTexIdx;

	uint32_t				m_dwCurrColor;

	__VertexXyzColorT1*	m_pVB;

	float				m_fScaleVelX;
	float				m_fScaleVelZ;
	float				m_fScaleAccelX;
	float				m_fScaleAccelZ;

	float				m_fCurrScaleVelX;
	float				m_fCurrScaleVelZ;

	float				m_fCurrSizeX;			//	보드의 크기
	float				m_fCurrSizeZ;

	float				m_fGap;
	
protected:
	__Vector3*			m_vUnit;

protected:
	void	CreateVB();
	bool	IsDead();

public:
	void	Init();									//	각종 변수들을 처음 로딩한 상태로 초기화...
	void	Start();								//	파트 구동 시작.
	void	Stop();									//	파트 구동 멈춤..
	bool	Tick();									//	ticktick...
	void	Render();								//	화면에 뿌리기..
	bool	Load(HANDLE hFile);						//	게임파일 불러오기.
	bool	Save(HANDLE hFile);						//	게임파일 저장오기.
	void	Duplicate(CN3FXPartBottomBoard* pSrc);

public:
	CN3FXPartBottomBoard();
	virtual ~CN3FXPartBottomBoard();

#ifdef _N3TOOL
	bool	ParseScript(char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3);
#endif // end of _N3TOOL
};

#endif // #ifndef __N3FXPARTBOTTOMBOARD_H__
