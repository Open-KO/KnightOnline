// N3FXPartMesh.h: interface for the CN3FXPartMesh class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTMESH_H__
#define __N3FXPARTMESH_H__

#include "N3FXPartBase.h"

class CN3FXShape;

class CN3FXPartMesh : public CN3FXPartBase
{
public:
	static constexpr int SUPPORTED_PART_VERSION = 5;

	CN3FXShape*	m_pShape;
	CN3FXShape*	m_pRefShape;

	uint32_t		m_dwCurrColor;	//
	
	char		m_cTextureMoveDir;	//텍스쳐 이동 방향..1:up 2:down, 3:left, 4:right
	float		m_fu;				//텍스쳐 이동 속도
	float		m_fv;				//텍스쳐 이동 속도

	__Vector3	m_vUnitScale;
	__Vector3	m_vScaleVel;
	__Vector3	m_vCurrScaleVel;
	__Vector3	m_vScaleAccel;
	__Vector3	m_vDir;

	bool		m_bTexLoop;
	float		m_fMeshFPS;

protected:
	bool	IsDead();

public:
	void	Init();				//	각종 변수들을 처음 로딩한 상태로 초기화...
	void	Start();			//	파트 구동 시작.
	void	Stop();				//	파트 구동 멈춤..
	bool	Tick();				//	ticktick...
	void	Render();			//	화면에 뿌리기..
	bool	Load(HANDLE hFile);	//	게임파일 불러오기.
	bool	Save(HANDLE hFile);	//	게임파일 저장오기.
	void	Duplicate(CN3FXPartMesh* pSrc);
		
public:
	void	Rotate();
	void	Move();
	void	Scaling();
	void	MoveTexUV();

	int		NumPart();
	int		NumVertices(int Part);
	LPDIRECT3DVERTEXBUFFER9 GetVB(int Part);

	CN3FXPartMesh();
	virtual ~CN3FXPartMesh();

#ifdef _N3TOOL
	bool	ParseScript(char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3);
#endif // end of _N3TOOL

};

#endif // #ifndef __N3FXPARTMESH_H__
