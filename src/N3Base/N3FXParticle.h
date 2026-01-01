// N3FXPartParticles.h: interface for the CN3FXPartParticles class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXPARTICLE_H__
#define __N3FXPARTICLE_H__

#include "N3Base.h"
//#include "N3FXDef.h"
class CN3FXPartParticles;

class CN3FXParticle : public CN3Base  
{
public:
	CN3FXPartParticles*	m_pRefParent;
	__VertexXyzColorT1*	m_pVB;

	int			m_iID;
	float		m_fLife;
	float		m_fCurrLife;
	
	int			m_iNumTex;
	int			m_iTexIdx;
	
	__Vector3	m_vCreatePoint;	//생성기준위치..
	__Vector3	m_vAxis;		//기준방향..(회전축)
	__Vector3	m_vVelocity;	//진행방향..
	__Vector3	m_vAccel;		//

	float		m_fDropVel;		//
	float		m_fDropY;		//
	float		m_fRot;			//
	__Vector3	m_vLcPos;		//로컬...
	__Vector3	m_vWdPos;		//	
	uint32_t		m_dwColor;		//
	float		m_fSize;		//원래크기
	//float		m_fCurrSize;	//현재

public:
	bool Tick();
	
//생성자 소멸자..
	CN3FXParticle();	
	virtual ~CN3FXParticle();
};

#endif // #ifndef __N3FXPARTICLE_H__