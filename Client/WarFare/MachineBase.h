// MachineBase.h: interface for the CMachineBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MACHINEBASE_H__CA5C8B1C_9A41_465B_875E_979A9BCD0483__INCLUDED_)
#define AFX_MACHINEBASE_H__CA5C8B1C_9A41_465B_875E_979A9BCD0483__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Base.h>

const uint32_t MS_STOP		= 0x00000000;
const uint32_t MS_FORWARD	= 0x00000001;
const uint32_t MS_BACKWARD = 0x00000002;
const uint32_t MS_TURNRIGHT= 0x00000004;
const uint32_t MS_TURNLEFT = 0x00000008;

const uint32_t MACHINE_TYPE_CATAPULT = 0x21;

class CN3Texture;
class CN3PMeshInstance;

#include <N3Base/N3Shape.h>

class CMachineBase : public CN3Shape
{
public:
	CMachineBase();
	virtual ~CMachineBase();

// Structure
public:
	enum {WHEEL_FL=0, WHEEL_FR=1, WHEEL_BL=2, WHEEL_BR=3, NUM_WHEEL=4};
	struct __Wheel
	{
		// 바퀴 정보
//		__Vector3			m_vPivot;	// Local 축
		CN3SPart*				pPart;	// Shape의 바퀴 Part포인터

		// 바퀴 정보 입력받은 후 계산해주는 값들.
		float				fRadius;	// 바퀴의 반지름
		float				fRotateRatio;// machine이 1.0f(rad)회전할때 바퀴가 돌아가는 각도(rad) 정도

		// 게임 진행시 바뀌는 정보.
		float				fRadian;	// 현재 바퀴의 돌아간 각도. (x축으로 돌린다.)
	};

	std::string 	m_szID;				// id string

protected:
	__Wheel			m_Wheel[NUM_WHEEL];
	float			m_fSpeed;			// 초당 이동속도(전후진)
	float			m_fRotateSpeed;		// 초당 회전 각속도
	//__Vector3		m_vDir;				// machine의 진행방향
	float			m_fDirRadian;		// 진행방향 각도(z양의 방향이 0.0f);
	uint32_t			m_dwMachineState;
	BOOL*			m_bSkipCalcPartMtx;		// 파트 메트릭스 계산 건너뛰기(part갯수만큼 할당됨)
	__Vector3		m_vBalancePoint[4];		// 균형을 잡기위한 4점의 위치(전후좌우) 로컬좌표임.

	float			m_fFireRadian;		// 발사각
	float			m_fFireSpeed;		// 발사속도(세기)

// Attributes
public:

	void			SetFireRadian(float fFireRadian) {m_fFireRadian = fFireRadian;}
	void			SetFireSpeed(float fFireSpeed) {m_fFireSpeed = fFireSpeed;}
	void			SetDirRadian(float fDirRadian) {m_fDirRadian = fDirRadian; ReCalcMatrix();}
	uint32_t			GetMachineState() const {return m_dwMachineState;}

// Operations
public:
	virtual void	Release();
	virtual void	Render();
	virtual void	ReCalcMatrix();
	virtual void	Tick(float fFrm);
	virtual void	LoadMachine(FILE* stream);

protected:
	CN3SPart*		GetPartByPMeshName(const std::string& szName);
	virtual void	ReCalcMatrix4AnimatedPart();
// 머신 동작 관련
public:
	void Stop() {m_dwMachineState = MS_STOP;}
	void ToggleMoveForward(BOOL bOn) {if (bOn) m_dwMachineState |= MS_FORWARD; else m_dwMachineState &= (~MS_FORWARD);}
	void ToggleMoveBackward(BOOL bOn) {if (bOn) m_dwMachineState |= MS_BACKWARD; else m_dwMachineState &= (~MS_BACKWARD);}
	void ToggleTurnRight(BOOL bOn) {if (bOn) m_dwMachineState |= MS_TURNRIGHT; else m_dwMachineState &= (~MS_TURNRIGHT);}
	void ToggleTurnLeft(BOOL bOn) {if (bOn) m_dwMachineState |= MS_TURNLEFT; else m_dwMachineState &= (~MS_TURNLEFT);}

	virtual void Fire();		// 발사
};

#endif // !defined(AFX_MACHINEBASE_H__CA5C8B1C_9A41_465B_875E_979A9BCD0483__INCLUDED_)
