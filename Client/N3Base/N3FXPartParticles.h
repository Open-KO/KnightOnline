// N3FXParticle.h: interface for the CN3FXParticle class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTPARTICLES_H__
#define __N3FXPARTPARTICLES_H__

#pragma warning(disable : 4786)

#include <list>
#include "N3FXPartBase.h"

class CN3FXParticle;
class CN3FXShape;

class CN3FXPartParticles : public CN3FXPartBase  
{
public:
	static constexpr int SUPPORTED_PART_VERSION = 5;

	//related whole particle system...
	__VertexXyzColorT1			m_vUnit[NUM_VERTEX_PARTICLE];
	//uint16_t						m_wUnitIB[6];

	__VertexXyzColorT1*			m_pVB;
	//uint16_t*						m_pIB;

	__Matrix44					m_mtxVI;				//	inverse view mtx..

	int							m_iNumParticle;			//	파티클의 총 수..(maximum)
	int							m_iNumLodParticle;		//	거리에 따른 파티클 갯수
	//float						m_fParticleSize;		//	파티클의 크기
	std::pair<float, float>		m_pair_fParticleSize;	//	파티클 크기의 범위
	std::pair<float, float>		m_pair_fParticleLife;	//	파티클 생명의 범위
	
	std::list<CN3FXParticle*>	m_pVBList_Alive;		//	살아있는 파티클을 담고 있는 버퍼..
	std::list<CN3FXParticle*>	m_pVBList_Dead;			//	죽은 파티클을 담고 있는 버퍼..

	float						m_fCreateDelay;			//	파티클 생성 시간 간격.
	float						m_CurrCreateDelay;		//	현재 까지 create delay time..
	int							m_iNumCreate;			//	한번 생성시 만들어지는 파티클 수.

	__Vector3					m_MinCreateRange;		//	파티클 생성 자유도 범위..min..
	__Vector3					m_MaxCreateRange;		//	파티클 생성 자유도 범위..max..

	//emitter...
	uint32_t						m_dwEmitType;			//	발사형태..(spread, gather)..
	PARTICLEEMITCONDITION		m_uEmitCon;				//	발사형태에 따른 필요 데이타..
	__Vector3					m_vEmitterDir;
	__Vector3					m_vPrevShapePos;
	__Vector3					m_vShapePos;

	//particle..
	__Vector3	m_vPtEmitDir;		//emitter에서 파티클 분사방향..
	float		m_fPtVelocity;		//파티클 분사속도..
	float		m_fPtAccel;			//분사 가속도..
	float		m_fPtRotVelocity;	//파티클 회전 속도..
	float		m_fPtGravity;		//중력가속도..

	//related particle color....
	uint32_t		m_dwChangeColor[NUM_KEY_COLOR];
	bool		m_bChangeColor;

	//related animation key
	CN3FXShape*	m_pShape;
	CN3FXShape*	m_pRefShape;
	float		m_fMeshFPS;
	bool		m_bAnimKey;

	//particle local rotate......2002.10.21.
	float		m_fTexRotateVelocity;		//	파티클 텍스쳐를 로컬 Z축으로 회전시켜서 빙글빙글 도는 것처럼 보이게...(속도)
	float		m_fScaleVelX;
	float		m_fScaleVelY;

protected:
	void	Rotate();
	void	Scaling();
	void	Move();

	void	InitVB();
	void	CreateParticles();
	void	CreateParticles_Spread();
	void	CreateParticles_Gather();

	std::list<CN3FXParticle*>::iterator	DestroyParticle(std::list<CN3FXParticle*>::iterator it);

	bool	IsDead();

	bool RotateQuaternion(__Vector3 vSrcDir, __Vector3 vDestDir, __Quaternion* pQt);
	float CameraDist(__Vector3 v1, __Vector3 v2, __Vector3 v3);

	//////////////////////////////////////////////
	//m_pVBList_Alive를 소트하기위함이야..
	void	PSort();
	void	PMerge(std::list<CN3FXParticle*>& l1, std::list<CN3FXParticle*>& l2);
	bool	PComp(CN3FXParticle* pP1, CN3FXParticle* pP2);
	//
	//////////////////////////////////////////////
	
public:
	void	Init();				//	각종 변수들을 처음 로딩한 상태로 초기화...
	void	Start();			//	파트 구동 시작.
	void	Stop();				//	파트 구동 멈춤..
	bool	Tick();				//	ticktick...
	void	Render();			//	화면에 뿌리기..
	bool	Load(HANDLE hFile);	//	게임파일 불러오기.
	bool	Save(HANDLE hFile);	//	게임파일 저장오기.
	void	Duplicate(CN3FXPartParticles* pSrc);

	bool	GetColor(int key, uint32_t& color);

	CN3FXPartParticles();	
	virtual ~CN3FXPartParticles();	

#ifdef _N3TOOL
	bool	ParseScript(char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3);

	bool	m_bChangeColorKey[NUM_KEY_COLOR];
	bool	m_bChangeAlphaKey[NUM_KEY_COLOR];
#endif // end of _N3TOOL
};

#endif // #ifndef __N3FXPARTPARTICLES_H__


