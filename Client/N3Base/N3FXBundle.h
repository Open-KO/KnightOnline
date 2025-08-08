// N3FXBundle.h: interface for the CN3FXPartParticles class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXBUNDLE_H__
#define __N3FXBUNDLE_H__

#include "N3BaseFileAccess.h"
#include "N3FXDef.h"
#include "N3FXPartBase.h"

class CN3FXBundle : public CN3BaseFileAccess  
{
public:
	static constexpr int SUPPORTED_BUNDLE_VERSION = 2;

	// 기본 정보들...
	
	int					m_iVersion;		//번들버전
	std::string			m_strName;		//번들이름
	int					m_iMoveType;	//움직이는 형태..
	float				m_fVelocity;
	bool				m_bStatic;		//위치를 고정시킬것인가....

	float				m_fLife0;		//번들의 수명..

	FXPARTWITHSTARTTIME*	m_pPart[MAX_FX_PART];
	
	// 변하는 정보들...
	float				m_fLife;		//현재 나이..
	uint32_t				m_dwState;		//현재 번들의 상태

	__Vector3			m_vPos;			//현재 번들의 위치.
	__Vector3			m_vPrePos;		//로딩 또는 Duplicate 하기전에 임시로 위치 지정
	__Vector3			m_vDestPos;		//현재 번들의 목표위치. 
	__Vector3			m_vDir;			//현재 번들의 방향.
	
	int					m_iSourceID;	//이 효과를 쓰는 주체의 아이디..(게임에서..)
	int					m_iTargetID;	//이 효과를 받는 대상의 아이디..(게임에서..)
	int					m_iTargetJoint;	//이 효과를 받는 대상의 부위..(게임에서..)
	int					m_iSourceJoint;

	bool				m_bDependScale;
	//__Vector3			m_vTargetScale;
	float				m_fTargetScale;

//	int					m_iNumStep;		//몇단계나 존재하는지...^^
//	int					m_iCurrStep;	//현재 진행중인 시퀀스
	static float		m_fEffectSndDist;

	class CN3SndObj*	m_pSndObj;

protected:
	bool	CheckAllPartsDead();		//모든 파트들이 죽었는지 검사..

public:
//기본적으로 필요한 함수들...
	virtual void	Trigger(int iSourceID=0, int iTargetID = 0, int iTargetJoint = 0, int iSndID = -1);
	virtual bool	Tick();
	virtual void	Duplicate(CN3FXBundle* pDestBundle);

	void	Init();
	void	Stop(bool immediately=false);
	
	void	Render();
	uint32_t	GetState() { return m_dwState; }

	virtual bool	Load(HANDLE hFile); // 핸들에서 읽어오기..
	bool	Save(HANDLE hFile);	// 저장하기..

//	int				GetPartCount() { return m_pPartList.size(); }
	CN3FXPartBase*	GetPart(int i);
	float			GetPartSTime(int i);
	void			SetPartSTime(int i, float time);
	static void		SetEffectSndDistance(float fDist) { m_fEffectSndDist = fDist; }
	static float	GetEffectSndDistance() { return m_fEffectSndDist; }

//생성자 소멸자..
	CN3FXBundle();	
	virtual ~CN3FXBundle();

#ifdef _N3TOOL	//툴에서만 쓰는 함수들...
public:
	CN3FXPartBase*	SetPart(const char* pFileName);
	bool	DecodeScriptFile(const char* lpPathName);
#endif // end of _N3TOOL
};

#endif // #ifndef __N3FXBUNDLE_H__
