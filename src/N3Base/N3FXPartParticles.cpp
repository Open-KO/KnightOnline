// N3FXPartBase.cpp: implementation of the CN3FXPartBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXPartParticles.h"
#include "N3FXBundle.h"
#include "N3FXParticle.h"
#include "N3FXDef.h"
#include "N3FXShape.h"

#include <shared/StringUtils.h>

CN3FXPartParticles::CN3FXPartParticles()
{
	m_iVersion                  = SUPPORTED_PART_VERSION; //3이하는 다 무시해버려..

	m_iNumParticle              = 0;
	m_iNumLodParticle           = 0;
	m_pair_fParticleSize.first  = 0.0f;
	m_pair_fParticleSize.second = 0.0f;
	m_pair_fParticleLife.first  = 0.0f;
	m_pair_fParticleLife.second = 0.0f;

	m_CurrCreateDelay           = 0.0f;
	m_fCreateDelay              = 0.01f;

	m_iNumCreate                = 1;

	m_MinCreateRange.Set(0, 0, 0);
	m_MaxCreateRange.Set(0, 0, 0);

	//m_vUnit[0].Set(-0.5f, 1.5f, 0.0f, 0xffffffff, 0.0f, -1.0f);
	//m_vUnit[1].Set(1.5f, -0.5f, 0.0f, 0xffffffff, 2.0f, 1.0f);
	//m_vUnit[2].Set(-0.5f, -0.5f, 0.0f, 0xffffffff, 0.0f, 1.0f);

	m_vUnit[0].Set(-0.5f, 0.5f, 0.0f, 0xffffffff, 0.0f, 0.0f);
	m_vUnit[1].Set(0.5f, 0.5f, 0.0f, 0xffffffff, 1.0f, 0.0f);
	m_vUnit[2].Set(0.5f, -0.5f, 0.0f, 0xffffffff, 1.0f, 1.0f);
	m_vUnit[3].Set(-0.5f, -0.5f, 0.0f, 0xffffffff, 0.0f, 1.0f);

	//m_wUnitIB[0] = 0;	m_wUnitIB[1] = 1;	m_wUnitIB[2] = 3;
	//m_wUnitIB[3] = 3;	m_wUnitIB[4] = 1;	m_wUnitIB[5] = 2;

	//emitter...
	m_dwEmitType = FX_PART_PARTICLE_EMIT_TYPE_NORMAL;
	memset(&m_uEmitCon, 0, sizeof(m_uEmitCon));

	//particle..
	m_vPtEmitDir.Set(0.0f, 0.0f, -1.0f);
	m_fPtVelocity = m_fPtAccel = m_fPtRotVelocity = m_fPtGravity = 0.0f;

	for (int i = 0; i < NUM_KEY_COLOR; i++)
		m_dwChangeColor[i] = 0xffffffff;
	m_bChangeColor = false;

	m_pRefShape = m_pShape = nullptr;
	m_fMeshFPS             = 30.0f;

	m_vPrevShapePos.Set(0.0f, 0.0f, 0.0f);

	m_bAnimKey           = false;

	m_pVB                = nullptr;
	//m_pIB = nullptr;

	m_fTexRotateVelocity = 0.0f;
	m_fScaleVelX = m_fScaleVelY = 0.0f;

#ifdef _N3TOOL
	for (int i = 0; i < NUM_KEY_COLOR; i++)
	{
		m_bChangeColorKey[i] = false;
		m_bChangeAlphaKey[i] = false;
	}
#endif // end of _N3TOOL

	m_vEmitterDir = {};
	m_vShapePos   = {};

	m_mtxVI.Identity();

	m_bDistanceNumFix           = false;
	m_bParticleYAxisFix         = false;
	m_bParticle_Not_Rotate      = false;
	m_vParticle_Not_Rotate_Axis = {};
	m_fPtRangeMin               = 0.0f;
	m_fPtRangeMax               = 0.0f;
}

CN3FXPartParticles::~CN3FXPartParticles()
{
	CN3FXPartBase::~CN3FXPartBase();

	std::list<CN3FXParticle*>::iterator it;

	for (auto itr = m_pVBList_Alive.begin(); itr != m_pVBList_Alive.end(); ++itr)
	{
		CN3FXParticle* pParticle = (*itr);
		if (pParticle)
			delete pParticle;
	}
	m_pVBList_Alive.clear();

	for (auto itr = m_pVBList_Dead.begin(); itr != m_pVBList_Dead.end(); ++itr)
	{
		CN3FXParticle* pParticle = (*itr);
		if (pParticle)
			delete pParticle;
	}
	m_pVBList_Dead.clear();

	CN3Base::s_MngFXShape.Delete(&m_pRefShape);
	if (m_pShape)
	{
		m_pShape->Release();
		delete m_pShape;
		m_pShape = nullptr;
	}

	if (m_pVB)
	{
		delete[] m_pVB;
		m_pVB = nullptr;
	}
	//if(m_pIB)
	//{
	//	delete[] m_pIB;
	//	m_pIB = nullptr;
	//}
}

//
//
//
#ifdef _N3TOOL
bool CN3FXPartParticles::ParseScript(
	char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3)
{
	if (CN3FXPartBase::ParseScript(szCommand, szBuff0, szBuff1, szBuff2, szBuff3))
		return true;

	//	파티클 수.
	if (lstrcmpi(szCommand, "<particle_count>") == 0)
	{
		m_iNumParticle = atoi(szBuff0);
		if (m_iNumParticle > 0)
			InitVB();
		return true;
	}

	//	파티클 크기.
	if (lstrcmpi(szCommand, "<particle_size>") == 0)
	{
		m_pair_fParticleSize.first = m_pair_fParticleSize.second = static_cast<float>(
			atof(szBuff0));
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_size_range>") == 0)
	{
		m_pair_fParticleSize.first  = static_cast<float>(atof(szBuff0));
		m_pair_fParticleSize.second = static_cast<float>(atof(szBuff1));
		return true;
	}

	//	파티클 생명.
	if (lstrcmpi(szCommand, "<particle_life>") == 0)
	{
		m_pair_fParticleLife.first  = static_cast<float>(atof(szBuff0));
		m_pair_fParticleLife.second = static_cast<float>(atof(szBuff1));
		return true;
	}

	//	파티클 시작오차..min
	if (lstrcmpi(szCommand, "<start_range_min>") == 0)
	{
		m_MinCreateRange.Set(static_cast<float>(atof(szBuff0)), static_cast<float>(atof(szBuff1)),
			static_cast<float>(atof(szBuff2)));
		return true;
	}

	//	파티클 시작오차..max
	if (lstrcmpi(szCommand, "<start_range_max>") == 0)
	{
		m_MaxCreateRange.Set(static_cast<float>(atof(szBuff0)), static_cast<float>(atof(szBuff1)),
			static_cast<float>(atof(szBuff2)));
		return true;
	}

	//	파티클 한번에 생성 갯수
	if (lstrcmpi(szCommand, "<create_count>") == 0)
	{
		m_iNumCreate = atoi(szBuff0);
		return true;
	}

	//	파티클 한번에 생성 시간 범위
	if (lstrcmpi(szCommand, "<create_delay>") == 0)
	{
		m_CurrCreateDelay = m_fCreateDelay = static_cast<float>(atof(szBuff0));
		return true;
	}

	//	시작하는 방법.
	if (lstrcmpi(szCommand, "<emit_type>") == 0)
	{
		if (lstrcmpi(szBuff0, "spread") == 0)
		{
			m_dwEmitType          = FX_PART_PARTICLE_EMIT_TYPE_SPREAD;
			m_uEmitCon.fEmitAngle = static_cast<float>(atof(szBuff1));
		}
		else if (lstrcmpi(szBuff0, "gather") == 0)
		{
			m_dwEmitType              = FX_PART_PARTICLE_EMIT_TYPE_GATHER;
			m_uEmitCon.vGatherPoint.x = static_cast<float>(atof(szBuff1));
			m_uEmitCon.vGatherPoint.y = static_cast<float>(atof(szBuff2));
			m_uEmitCon.vGatherPoint.z = static_cast<float>(atof(szBuff3));
		}
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_direction>") == 0)
	{
		m_vPtEmitDir.Set(static_cast<float>(atof(szBuff0)), static_cast<float>(atof(szBuff1)),
			static_cast<float>(atof(szBuff2)));
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_velocity>") == 0)
	{
		m_fPtVelocity = static_cast<float>(atof(szBuff0));
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_acceleration>") == 0)
	{
		m_fPtAccel = static_cast<float>(atof(szBuff0));
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_rotation_velocity>") == 0)
	{
		float Degree     = static_cast<float>(atof(szBuff0));
		m_fPtRotVelocity = DegreesToRadians(Degree);
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_gravity>") == 0)
	{
		m_fPtGravity = static_cast<float>(atof(szBuff0));
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_color>") == 0)
	{
		int seq        = atoi(szBuff0);
		uint32_t color = atoi(szBuff1);
		if (seq >= 0 && seq < NUM_KEY_COLOR)
			m_dwChangeColor[seq] = color;
		return true;
	}

	if (lstrcmpi(szCommand, "<change_color>") == 0)
	{
		if (lstrcmpi(szBuff0, "true") == 0)
			m_bChangeColor = true;
		else if (lstrcmpi(szBuff0, "false") == 0)
			m_bChangeColor = false;
		return true;
	}

	if (lstrcmpi(szCommand, "<color_key>") == 0)
	{
		int seq = atoi(szBuff0);
		if (seq >= 0 && seq < NUM_KEY_COLOR)
			m_bChangeColorKey[seq] = true;
		return true;
	}

	if (lstrcmpi(szCommand, "<alpha_key>") == 0)
	{
		int seq = atoi(szBuff0);
		if (seq >= 0 && seq < NUM_KEY_COLOR)
			m_bChangeAlphaKey[seq] = true;
		return true;
	}

	if (lstrcmpi(szCommand, "<shape_name>") == 0)
	{
		if (lstrcmpi(szBuff0, "") == 0)
			return false;

		char szPath[MAX_PATH] {};
		strcpy_safe(szPath, szBuff0);

		m_pShape    = new CN3FXShape();
		m_pRefShape = s_MngFXShape.Get(szPath);
		m_pShape->Duplicate(m_pRefShape);
		m_vCurrPos = m_pShape->CenterPos();
		return true;
	}

	if (lstrcmpi(szCommand, "<shape_fps>") == 0)
	{
		m_fMeshFPS = static_cast<float>(atof(szBuff0));
		return true;
	}

	if (lstrcmpi(szCommand, "<shape_apply>") == 0)
	{
		if (lstrcmpi(szBuff0, "true") == 0)
			m_bAnimKey = true;
		else if (lstrcmpi(szBuff0, "false") == 0)
			m_bAnimKey = false;

		return true;
	}

	if (lstrcmpi(szCommand, "<texture_rotation_speed>") == 0)
	{
		m_fTexRotateVelocity = static_cast<float>(atof(szBuff0));
		return true;
	}

	if (lstrcmpi(szCommand, "<particle_scale_velocity>") == 0)
	{
		m_fScaleVelX = static_cast<float>(atof(szBuff0));
		m_fScaleVelY = static_cast<float>(atof(szBuff1));
		return true;
	}

	return false;
}
#endif // end of _N3TOOL

//
//	init...
//
void CN3FXPartParticles::Init()
{
	CN3FXPartBase::Init();
	if (m_pShape)
	{
		m_vShapePos     = m_vPos;
		m_vPrevShapePos = m_vCurrPos = m_vShapePos + m_pShape->CenterPos();
	}
	else
		m_vCurrPos = m_vPos;

	if (m_iVersion < 3 && m_pShape)
	{
		m_bAnimKey = true;
	}

	for (auto itr = m_pVBList_Alive.begin(); itr != m_pVBList_Alive.end(); ++itr)
		m_pVBList_Dead.push_back(*itr);

	m_pVBList_Alive.clear();

	m_CurrCreateDelay = m_fCreateDelay;

	int i             = 0;
	for (auto itr = m_pVBList_Dead.begin(); itr != m_pVBList_Dead.end(); ++itr)
	{
		CN3FXParticle* pParticle = *itr;

		pParticle->m_iID         = i++ * NUM_VERTEX_PARTICLE;
		pParticle->m_dwColor     = 0xffffffff;
		pParticle->m_fCurrLife   = 0.0f;
		pParticle->m_fDropVel    = 0.0f;
		pParticle->m_fDropY      = 0.0f;
		pParticle->m_fLife       = m_pair_fParticleLife.first
							 + ((m_pair_fParticleLife.second - m_pair_fParticleLife.first)
								 * (float) (rand() % 100) / 100.0f);
		pParticle->m_fSize = m_pair_fParticleSize.first
							 + ((m_pair_fParticleSize.second - m_pair_fParticleSize.first)
								 * (float) (rand() % 100) / 100.0f);
		pParticle->m_iNumTex    = m_iNumTex;
		pParticle->m_pRefParent = this;
		pParticle->m_iTexIdx    = 0;
	}
}

//
//
//
bool CN3FXPartParticles::Load(File& file)
{
	if (!CN3FXPartBase::Load(file))
		return false;

	if (m_iVersion < 3)
		return false;

	file.Read(&m_iNumParticle, sizeof(int));
	if (m_iNumParticle > 0)
		InitVB();

	if (m_iVersion < 4)
	{
		float ParticleSize = 0.0f;
		file.Read(&ParticleSize, sizeof(float));
		m_pair_fParticleSize.first = m_pair_fParticleSize.second = ParticleSize;
	}
	else
	{
		file.Read(&m_pair_fParticleSize.first, sizeof(float));
		file.Read(&m_pair_fParticleSize.second, sizeof(float));
	}

	file.Read(&(m_pair_fParticleLife.first), sizeof(float));
	file.Read(&(m_pair_fParticleLife.second), sizeof(float));

	file.Read(&m_MinCreateRange, sizeof(__Vector3));
	file.Read(&m_MaxCreateRange, sizeof(__Vector3));

	file.Read(&m_fCreateDelay, sizeof(float));
	file.Read(&m_iNumCreate, sizeof(int));

	file.Read(&m_dwEmitType, sizeof(uint32_t));

	if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
	{
		file.Read(&m_uEmitCon.fEmitAngle, sizeof(float));
	}
	else if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_GATHER)
	{
		file.Read(&m_uEmitCon.vGatherPoint.x, sizeof(float));
		file.Read(&m_uEmitCon.vGatherPoint.y, sizeof(float));
		file.Read(&m_uEmitCon.vGatherPoint.z, sizeof(float));
	}

	file.Read(&m_vPtEmitDir, sizeof(__Vector3));
	file.Read(&m_fPtVelocity, sizeof(float));
	file.Read(&m_fPtAccel, sizeof(float));
	file.Read(&m_fPtRotVelocity, sizeof(float));
	file.Read(&m_fPtGravity, sizeof(float));

	file.Read(&m_bChangeColor, sizeof(bool));
	if (m_bChangeColor)
	{
		int iNumKeyColor = 0;
		file.Read(&iNumKeyColor, sizeof(int));
		file.Read(&m_dwChangeColor, sizeof(uint32_t) * iNumKeyColor);
	}

	file.Read(&m_bAnimKey, sizeof(bool));
	if (m_bAnimKey)
	{
		file.Read(&m_fMeshFPS, sizeof(float));

		char szShapeFileName[_MAX_PATH] {};
		file.Read(szShapeFileName, _MAX_PATH);

		delete m_pShape;
		m_pShape    = new CN3FXShape();

		m_pRefShape = s_MngFXShape.Get(szShapeFileName);
		m_pShape->Duplicate(m_pRefShape);
	}

	if (m_iVersion >= 5)
	{
		file.Read(&m_fTexRotateVelocity, sizeof(float));
		file.Read(&m_fScaleVelX, sizeof(float));
		file.Read(&m_fScaleVelY, sizeof(float));
	}

	// TODO: implement m_bDistanceNumFix
	if (m_iVersion >= 6)
		file.Read(&m_bDistanceNumFix, sizeof(bool));

	// TODO: implement m_bParticleYAxisFix
	if (m_iVersion >= 7)
		file.Read(&m_bParticleYAxisFix, sizeof(bool));

	// TODO: implement m_bParticle_Not_Rot, m_vParticle_Not_Rotate_Axis
	if (m_iVersion >= 8)
	{
		file.Read(&m_bParticle_Not_Rotate, sizeof(bool));
		file.Read(&m_vParticle_Not_Rotate_Axis, sizeof(__Vector3));
	}

	// TODO: implement m_fPtRangeMin, m_fPtRangeMax
	if (m_iVersion >= 9)
	{
		file.Read(&m_fPtRangeMin, sizeof(float));
		file.Read(&m_fPtRangeMax, sizeof(float));
	}

	if (m_iVersion >= 10)
		file.Seek(5, SEEK_CUR);

	if (m_iVersion >= 11)
		file.Seek(12, SEEK_CUR);

	// NOTE: This should ideally just be an assertion, but we'll continue to allow it to run
	// and otherwise be broken for now.
#if defined(_DEBUG)
	if (m_iVersion > SUPPORTED_PART_VERSION)
	{
		TRACE("!!! WARNING: CN3FXPartParticles::Load({}) encountered version {} (base version {}). "
			  "Needs support!",
			m_pRefBundle != nullptr ? m_pRefBundle->FileName().c_str() : "<unknown>", m_iVersion,
			m_iBaseVersion);
	}
#endif

	Init();

	return true;
}

//
//
//
bool CN3FXPartParticles::Save(File& file)
{
	if (!CN3FXPartBase::Save(file))
		return false;

	file.Write(&m_iNumParticle, sizeof(int));

	file.Write(&(m_pair_fParticleSize.first), sizeof(float));
	file.Write(&(m_pair_fParticleSize.second), sizeof(float));

	file.Write(&(m_pair_fParticleLife.first), sizeof(float));
	file.Write(&(m_pair_fParticleLife.second), sizeof(float));

	file.Write(&m_MinCreateRange, sizeof(__Vector3));
	file.Write(&m_MaxCreateRange, sizeof(__Vector3));

	file.Write(&m_fCreateDelay, sizeof(float));
	file.Write(&m_iNumCreate, sizeof(int));

	file.Write(&m_dwEmitType, sizeof(uint32_t));

	if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
	{
		file.Write(&(m_uEmitCon.fEmitAngle), sizeof(float));
	}
	else if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_GATHER)
	{
		file.Write(&(m_uEmitCon.vGatherPoint.x), sizeof(float));
		file.Write(&(m_uEmitCon.vGatherPoint.y), sizeof(float));
		file.Write(&(m_uEmitCon.vGatherPoint.z), sizeof(float));
	}

	file.Write(&m_vPtEmitDir, sizeof(__Vector3));
	file.Write(&m_fPtVelocity, sizeof(float));
	file.Write(&m_fPtAccel, sizeof(float));
	file.Write(&m_fPtRotVelocity, sizeof(float));
	file.Write(&m_fPtGravity, sizeof(float));

	file.Write(&m_bChangeColor, sizeof(bool));
	if (m_bChangeColor)
	{
		int iNumKeyColor = NUM_KEY_COLOR;
		file.Write(&iNumKeyColor, sizeof(int));
		file.Write(&m_dwChangeColor, sizeof(uint32_t) * NUM_KEY_COLOR);
	}

	file.Write(&m_bAnimKey, sizeof(bool));
	if (m_bAnimKey)
	{
		file.Write(&m_fMeshFPS, sizeof(float));

		char szShapeFileName[_MAX_PATH] {};
		strcpy_safe(szShapeFileName, m_pRefShape->FileName().c_str());
		file.Write(szShapeFileName, _MAX_PATH);
	}

	file.Write(&m_fTexRotateVelocity, sizeof(float));
	file.Write(&m_fScaleVelX, sizeof(float));
	file.Write(&m_fScaleVelY, sizeof(float));

	return true;
}

//
//
//
void CN3FXPartParticles::Start()
{
	CN3FXPartBase::Start();
}

//
//
//
void CN3FXPartParticles::Stop()
{
	CN3FXPartBase::Stop();
}

//
//
//
bool CN3FXPartParticles::Tick()
{
	if (!CN3FXPartBase::Tick())
		return false;

	if (m_pRefBundle == nullptr)
		return false;

#ifndef _N3TOOL
	float fDist = (s_CameraData.vEye - m_pRefBundle->m_vPos).Magnitude();

	if (fDist > 30.0f)
		m_iNumLodParticle = (int) (m_iNumParticle / 3.0f);
	else
	{
		m_iNumLodParticle = (int) (m_iNumParticle * 1 / 3.0f
								   + (m_iNumParticle * 2 / 3.0f) * ((30.0f - fDist) / 30.0f));
	}
#else
	m_iNumLodParticle = m_iNumParticle;
#endif

	m_mtxVI = s_CameraData.mtxViewInverse;
	m_mtxVI.PosSet(0, 0, 0);

	m_CurrCreateDelay += CN3Base::s_fSecPerFrm;

	if (m_bAnimKey && m_pShape)
	{
		//frm 계산..
		float fFrm  = m_fCurrLife * m_fMeshFPS;
		int share   = (int) (fFrm / m_pShape->GetWholeFrm());
		fFrm       -= ((float) share * m_pShape->GetWholeFrm());
		m_pShape->SetCurrFrm(fFrm);
		//TRACE("Frm: {:3.2f} life: {:3.2f}", fFrm, m_fCurrLife);

		m_pShape->m_mtxParent.Identity();
		Rotate();
		Scaling();
		Move();
		m_pShape->Tick();

		m_vCurrPos    = m_pShape->CenterPos();
		m_vEmitterDir = m_vCurrPos - m_vPrevShapePos;
		m_vEmitterDir.Normalize();
		m_vPrevShapePos = m_vCurrPos;
	}
	else
	{
		// tick emitter...
		m_vCurrVelocity += m_vAcceleration * CN3Base::s_fSecPerFrm;
		m_vCurrPos      += m_vCurrVelocity * CN3Base::s_fSecPerFrm;
		if (m_vCurrVelocity.Magnitude() != 0.0f)
		{
			m_vEmitterDir = m_vCurrVelocity;
			m_vEmitterDir.Normalize();
		}
		else
			m_vEmitterDir.Set(0, 0, 1);
	}

	// make particles...
	if (m_CurrCreateDelay >= m_fCreateDelay && m_dwState == FX_PART_STATE_LIVE) //파티클 생성...
	{
		m_CurrCreateDelay = 0.0f;
		CreateParticles();
	}

	std::list<CN3FXParticle*>::iterator it;
	it = m_pVBList_Alive.begin();
	while (it != m_pVBList_Alive.end())
	{
		CN3FXParticle* pParticle = (*it);
		if (pParticle && pParticle->Tick() == false)
		{
			it = DestroyParticle(it);
		}
		else
			it++;
	}
	return true;
}

//
//
//
void CN3FXPartParticles::Rotate()
{
	m_pShape->m_mtxParent.Rotation(m_vRotVelocity * m_fCurrLife);
}

//
//
//
void CN3FXPartParticles::Move()
{
	m_vCurrVelocity += m_vAcceleration * CN3Base::s_fSecPerFrm;
	m_vShapePos     += m_vCurrVelocity * CN3Base::s_fSecPerFrm;
	m_pShape->m_mtxParent.PosSet(m_vShapePos);
}

//
//
//
void CN3FXPartParticles::Scaling()
{
	if (m_pRefBundle == nullptr)
		return;

	__Vector3 vScale;
	vScale.Set(1.0f, 1.0f, 1.0f);

	if (m_pRefBundle->m_bDependScale)
		vScale *= m_pRefBundle->m_fTargetScale;

	if (vScale.x < 0)
		vScale.x = 0;
	if (vScale.y < 0)
		vScale.y = 0;
	if (vScale.z < 0)
		vScale.z = 0;

	__Matrix44 mtx;
	mtx.Scale(vScale);
	m_pShape->m_mtxParent *= mtx;
}

//
//	render...
//	일단은 파티클 하나씩 그리고....
//	나중에는 같은 텍스쳐 쓰는 것들끼리 묶어서 그리자...
//
void CN3FXPartParticles::Render()
{
	if (m_pVBList_Alive.size() == 0)
		return;

	if (m_bAlpha) // Alpha 사용
	{
		std::list<CN3FXParticle*>::iterator it;
		it = m_pVBList_Alive.begin();
		for (; it != m_pVBList_Alive.end(); it++)
		{
			CN3FXParticle* pParticle = (*it);
			if (pParticle->m_iTexIdx >= m_iNumTex)
				continue;

			__AlphaPrimitive* pAP = s_AlphaMgr.Add();
			if (pAP != nullptr)
			{
				pAP->bUseVB          = FALSE;
				pAP->dwBlendDest     = m_dwDestBlend;
				pAP->dwBlendSrc      = m_dwSrcBlend;
				pAP->dwFVF           = FVF_XYZCOLORT1;
				pAP->dwPrimitiveSize = sizeof(__VertexXyzColorT1);
				pAP->fCameraDistance = CameraDist(
					pParticle->m_pVB[0], pParticle->m_pVB[1], pParticle->m_pVB[2]);

				if (m_ppRefTex[pParticle->m_iTexIdx])
					pAP->lpTex = m_ppRefTex[pParticle->m_iTexIdx]->Get();
				else
					pAP->lpTex = nullptr;

				pAP->MtxWorld        = __Matrix44::GetIdentity();
				pAP->nRenderFlags    = m_dwRenderFlag; // | RF_UV_CLAMP;
				pAP->ePrimitiveType  = D3DPT_TRIANGLEFAN;
				pAP->nPrimitiveCount = 2;
				pAP->nVertexCount    = NUM_VERTEX_PARTICLE;
				pAP->pVertices       = pParticle->m_pVB;
				pAP->pwIndices       = nullptr;
			}
		}

		return; // 렌더링 안하지롱.
	}

	CN3Base::s_lpD3DDev->SetFVF(FVF_XYZCOLORT1);

	//s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, m_bAlpha );
	//s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND, m_dwSrcBlend );
	//s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND, m_dwDestBlend );

	//uint32_t dwTAddrU, dwTAddrV;
	//s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ADDRESSU, &dwTAddrU );
	//s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ADDRESSV, &dwTAddrV );

	//s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
	//s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	DWORD dwCullMode = 0, dwZWriteEnable = 0, dwZBufferEnable = 0, dwLight = 0, dwAlpha = 0;
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWriteEnable);
	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZBufferEnable);
	s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCullMode);
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);

	if (m_bAlpha != dwAlpha)
	{
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, m_bAlpha);
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, m_dwSrcBlend);
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, m_dwDestBlend);
	}
	if (m_dwZEnable != dwZBufferEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, m_dwZEnable);
	if (m_dwZWrite != dwZWriteEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, m_dwZWrite);
	if (m_dwLight != dwLight)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, m_dwLight);
	if (m_dwDoubleSide != dwCullMode)
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, m_dwDoubleSide);

	std::list<CN3FXParticle*>::iterator it;
	it = m_pVBList_Alive.begin();
	for (; it != m_pVBList_Alive.end(); it++)
	{
		CN3FXParticle* pParticle = (*it);
		if (pParticle->m_iTexIdx < m_iNumTex)
			CN3Base::s_lpD3DDev->SetTexture(0, m_ppRefTex[pParticle->m_iTexIdx]->Get());
		else
			continue;

		//for(int i=0;i<6;i++) m_pIB[i] = pParticle->m_iID + m_wUnitIB[i];

		CN3Base::s_lpD3DDev->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN, 2, pParticle->m_pVB, sizeof(__VertexXyzColorT1));
		//hr = CN3Base::s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, m_pIB, D3DFMT_INDEX16, m_pVB, sizeof(__VertexXyzColorT1));
	}

	if (m_bAlpha != dwAlpha)
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	if (m_dwZEnable != dwZBufferEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZBufferEnable);
	if (m_dwZWrite != dwZWriteEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable);
	if (m_dwLight != dwLight)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
	if (m_dwDoubleSide != dwCullMode)
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCullMode);

	//CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSU, dwTAddrU );
	//CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSV, dwTAddrV );
}

float CN3FXPartParticles::CameraDist(
	const __VertexXyzColorT1& v1, const __VertexXyzColorT1& v2, const __VertexXyzColorT1& v3) const
{
	__Vector3 vA = v1 - v3;
	__Vector3 vB = v2 - v3;
	__Vector3 vN;
	vN.Cross(vA, vB);
	vN.Normalize();

	float D = -(vN.x * v1.x + vN.y * v1.y + vN.z * v1.z);

	return (
		vN.x * s_CameraData.vEye.x + vN.y * s_CameraData.vEye.y + vN.z * s_CameraData.vEye.z + D);
}

//
//
//
void CN3FXPartParticles::InitVB()
{
	for (auto itr = m_pVBList_Alive.begin(); itr != m_pVBList_Alive.end(); ++itr)
		delete *itr;
	m_pVBList_Alive.clear();

	for (auto itr = m_pVBList_Dead.begin(); itr != m_pVBList_Dead.end(); ++itr)
		delete *itr;
	m_pVBList_Dead.clear();

	for (int i = 0; i < m_iNumParticle; i++)
	{
		CN3FXParticle* pParticle = new CN3FXParticle;
		pParticle->m_iID         = i * NUM_VERTEX_PARTICLE;
		m_pVBList_Dead.push_back(pParticle);
	}

	if (m_pVB)
		delete[] m_pVB;
	m_pVB = new __VertexXyzColorT1[m_iNumParticle * NUM_VERTEX_PARTICLE];

	//if(m_pIB) delete[] m_pIB;
	//m_pIB = new uint16_t[m_iNumParticle*6];
}

//
//
//
std::list<CN3FXParticle*>::iterator CN3FXPartParticles::DestroyParticle(
	std::list<CN3FXParticle*>::iterator it)
{
	CN3FXParticle* pParticle = (*it);

	it                       = m_pVBList_Alive.erase(it);
	m_pVBList_Dead.push_back(pParticle);

	pParticle->m_dwColor   = 0xffffffff;
	pParticle->m_fCurrLife = 0.0f;
	pParticle->m_fDropVel  = 0.0f;
	pParticle->m_fDropY    = 0.0f;
	pParticle->m_fLife     = m_pair_fParticleLife.first
						 + ((m_pair_fParticleLife.second - m_pair_fParticleLife.first)
							 * (float) (rand() % 100) / 100.0f);
	pParticle->m_fSize = m_pair_fParticleSize.first
						 + ((m_pair_fParticleSize.second - m_pair_fParticleSize.first)
							 * (float) (rand() % 100) / 100.0f);
	pParticle->m_iNumTex = m_iNumTex;
	pParticle->m_iTexIdx = 0;
	pParticle->m_vLcPos.Set(0.0f, 0.0f, 0.0f);
	return it;
}

//
//
//
void CN3FXPartParticles::CreateParticles_Spread()
{
	if (m_pRefBundle == nullptr)
		return;

	std::list<CN3FXParticle*>::iterator it;

	for (int i = 0; i < m_iNumCreate; i++)
	{
		it = m_pVBList_Dead.begin();
		if (it == m_pVBList_Dead.end())
			break;

		CN3FXParticle* pParticle = (*it);

		__Vector3 vDir;
		__Matrix44 RotMtx;

		float fUnitAngleXZ = 0.0f;
		if (m_uEmitCon.fEmitAngle != 0)
			fUnitAngleXZ = (float) (rand() % (int) m_uEmitCon.fEmitAngle)
						   - (m_uEmitCon.fEmitAngle / 2.0f);

		float fUnitAxisZ = (float) (rand() % 360);

		vDir.Set(sin(DegreesToRadians(fUnitAngleXZ)), 0, cos(DegreesToRadians(fUnitAngleXZ)));

		RotMtx.RotationZ(fUnitAxisZ);
		vDir *= RotMtx;
		vDir.Normalize();

		__Vector3 vDirPart, vDirEmit;
		__Vector3 v(0.0f, 0.0f, 1.0f);
		vDirPart = m_vEmitterDir;
		vDirEmit = m_vPtEmitDir;

		__Quaternion Qt;

		//bundle의 방향 적용..
		if (RotateQuaternion(v, m_pRefBundle->m_vDir, &Qt))
		{
			RotMtx    = Qt;
			//vDir *= RotMtx;
			vDirPart *= RotMtx;
			//vDirEmit *=  RotMtx;
		}
		else if (Qt.w != 1.0f)
		{
			//vDir *= -1.0f;
			vDirPart *= -1.0f;
			//vDirEmit *= -1.0f;
		}

		//part(emiiter)의 방향 적용
		//if(m_vEmitterDir.Magnitude()!=0)
		if (vDirPart.Magnitude() != 0)
		{
			//if(RotateQuaternion(v, m_vEmitterDir, &Qt))
			if (RotateQuaternion(v, vDirPart, &Qt))
			{
				RotMtx    = Qt;
				//vDir *= RotMtx;
				//vDirPart *= RotMtx;
				vDirEmit *= RotMtx;
			}
			else if (Qt.w != 1.0f)
			{
				//vDir *= -1.0f;
				//vDirPart *= -1.0f;
				vDirEmit *= -1.0f;
			}
		}

		//뿌려지는 방향 적용..
		//if(RotateQuaternion(v, m_vPtEmitDir, &Qt))
		if (RotateQuaternion(v, vDirEmit, &Qt))
		{
			RotMtx  = Qt;
			vDir   *= RotMtx;
			//vDirPart *= RotMtx;
			//vDirEmit *=  RotMtx;
		}
		else if (Qt.w != 1.0f)
		{
			vDir *= -1.0f;
			//vDirPart *= -1.0f;
			//vDirEmit *= -1.0f;
		}

		pParticle->m_vAxis     = vDirEmit;
		pParticle->m_vVelocity = vDir * m_fPtVelocity;

		if (m_pRefBundle->m_bDependScale)
		{
			pParticle->m_fSize     *= m_pRefBundle->m_fTargetScale;

			//pParticle->m_vVelocity.x *= m_pRefBundle->m_vTargetScale.x;
			//pParticle->m_vVelocity.y *= m_pRefBundle->m_vTargetScale.y;
			//pParticle->m_vVelocity.z *= m_pRefBundle->m_vTargetScale.z;
			pParticle->m_vVelocity *= m_pRefBundle->m_fTargetScale;

			__Vector3 MaxCreate, MinCreate;
			if (m_bAnimKey && m_pShape)
			{
				__Vector3 Scale = m_pShape->Scale();
				MaxCreate.x     = m_MaxCreateRange.x * Scale.x;
				MaxCreate.y     = m_MaxCreateRange.y * Scale.y;
				MaxCreate.z     = m_MaxCreateRange.z * Scale.z;

				MinCreate.x     = m_MinCreateRange.x * Scale.x;
				MinCreate.y     = m_MinCreateRange.y * Scale.y;
				MinCreate.z     = m_MinCreateRange.z * Scale.z;
			}
			else
			{
				//MaxCreate.x = m_MaxCreateRange.x * m_pRefBundle->m_vTargetScale.x;
				//MaxCreate.y = m_MaxCreateRange.y * m_pRefBundle->m_vTargetScale.y;
				//MaxCreate.z = m_MaxCreateRange.z * m_pRefBundle->m_vTargetScale.z;
				MaxCreate = m_MaxCreateRange * m_pRefBundle->m_fTargetScale;

				//MinCreate.x = m_MinCreateRange.x * m_pRefBundle->m_vTargetScale.x;
				//MinCreate.y = m_MinCreateRange.y * m_pRefBundle->m_vTargetScale.y;
				//MinCreate.z = m_MinCreateRange.z * m_pRefBundle->m_vTargetScale.z;
				MinCreate = m_MinCreateRange * m_pRefBundle->m_fTargetScale;
			}

			__Vector3 vCreatePos;
			vCreatePos.Set(
				MinCreate.x + ((MaxCreate.x - MinCreate.x) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.y + ((MaxCreate.y - MinCreate.y) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.z + ((MaxCreate.z - MinCreate.z) * ((float) (rand() % 100) / 100.0f)));

			if (RotateQuaternion(__Vector3(0.0f, 0.0f, 1.0f), pParticle->m_vAxis, &Qt))
			{
				RotMtx      = Qt;
				vCreatePos *= RotMtx;
			}
			else if (Qt.w != 1.0f)
			{
				vCreatePos *= -1.0f;
			}

			pParticle->m_vCreatePoint = m_pRefBundle->m_vPos + m_vCurrPos;
			pParticle->m_vLcPos       = vCreatePos;
		}
		else
		{
			__Vector3 MaxCreate, MinCreate;
			MaxCreate = m_MaxCreateRange;
			MinCreate = m_MinCreateRange;
			if (m_bAnimKey && m_pShape)
			{
				__Vector3 Scale  = m_pShape->Scale();
				MaxCreate.x     *= Scale.x;
				MaxCreate.y     *= Scale.y;
				MaxCreate.z     *= Scale.z;

				MinCreate.x     *= Scale.x;
				MinCreate.y     *= Scale.y;
				MinCreate.z     *= Scale.z;
			}

			__Vector3 vCreatePos;
			vCreatePos.Set(
				MinCreate.x + ((MaxCreate.x - MinCreate.x) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.y + ((MaxCreate.y - MinCreate.y) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.z + ((MaxCreate.z - MinCreate.z) * ((float) (rand() % 100) / 100.0f)));

			if (RotateQuaternion(__Vector3(0.0f, 0.0f, 1.0f), pParticle->m_vAxis, &Qt))
			{
				RotMtx      = Qt;
				vCreatePos *= RotMtx;
			}
			else if (Qt.w != 1.0f)
			{
				vCreatePos *= -1.0f;
			}

			pParticle->m_vCreatePoint = m_pRefBundle->m_vPos + m_vCurrPos;
			pParticle->m_vLcPos       = vCreatePos;
		}

		m_pVBList_Alive.push_back((*it));
		m_pVBList_Dead.erase(it);
	}
}

//
//
//
void CN3FXPartParticles::CreateParticles_Gather()
{
	if (m_pRefBundle == nullptr)
		return;

	std::list<CN3FXParticle*>::iterator it;

	for (int i = 0; i < m_iNumCreate; i++)
	{
		it = m_pVBList_Dead.begin();
		if (it == m_pVBList_Dead.end())
			break;

		CN3FXParticle* pParticle = (*it);

		__Vector3 vDir;
		__Matrix44 RotMtx;

		vDir.Set(m_uEmitCon.vGatherPoint.x, m_uEmitCon.vGatherPoint.y, m_uEmitCon.vGatherPoint.z);
		//	vDir.Normalize();

		__Vector3 vDirPart, vDirEmit;
		__Vector3 v(0.0f, 0.0f, 1.0f);
		vDirPart = m_vEmitterDir;
		vDirEmit = m_vPtEmitDir;

		__Quaternion Qt;

		//bundle의 방향 적용..
		if (RotateQuaternion(v, m_pRefBundle->m_vDir, &Qt))
		{
			RotMtx    = Qt;
			//vDir *= RotMtx;
			vDirPart *= RotMtx;
			//vDirEmit *=  RotMtx;
		}
		else if (Qt.w != 1.0f)
		{
			//vDir *= -1.0f;
			vDirPart *= -1.0f;
			//vDirEmit *= -1.0f;
		}

		//part(emiiter)의 방향 적용
		if (vDirPart.Magnitude() != 0)
		{
			if (RotateQuaternion(v, vDirPart, &Qt))
			{
				RotMtx    = Qt;
				//vDir *= RotMtx;
				//vDirPart *= RotMtx;
				vDirEmit *= RotMtx;
			}
			else if (Qt.w != 1.0f)
			{
				//vDir *= -1.0f;
				//vDirPart *= -1.0f;
				vDirEmit *= -1.0f;
			}
		}
		//뿌려지는 방향 적용..
		//if(RotateQuaternion(v, m_vPtEmitDir, &Qt))
		if (RotateQuaternion(v, vDirEmit, &Qt))
		{
			RotMtx  = Qt;
			vDir   *= RotMtx;
			//vDirPart *= RotMtx;
			//vDirEmit *=  RotMtx;
		}
		else if (Qt.w != 1.0f)
		{
			vDir *= -1.0f;
			//vDirPart *= -1.0f;
			//vDirEmit *= -1.0f;
		}

		pParticle->m_vAxis = vDirEmit;

		if (m_pRefBundle->m_bDependScale)
		{
			//pParticle->m_fSize *= m_pRefBundle->m_vTargetScale.x;
			pParticle->m_fSize     *= m_pRefBundle->m_fTargetScale;

			//pParticle->m_vVelocity.x *= m_pRefBundle->m_vTargetScale.x;
			//pParticle->m_vVelocity.y *= m_pRefBundle->m_vTargetScale.y;
			//pParticle->m_vVelocity.z *= m_pRefBundle->m_vTargetScale.z;
			pParticle->m_vVelocity *= m_pRefBundle->m_fTargetScale;

			__Vector3 MaxCreate, MinCreate;
			if (m_bAnimKey && m_pShape)
			{
				__Vector3 Scale = m_pShape->Scale();
				MaxCreate.x     = m_MaxCreateRange.x * Scale.x;
				MaxCreate.y     = m_MaxCreateRange.y * Scale.y;
				MaxCreate.z     = m_MaxCreateRange.z * Scale.z;

				MinCreate.x     = m_MinCreateRange.x * Scale.x;
				MinCreate.y     = m_MinCreateRange.y * Scale.y;
				MinCreate.z     = m_MinCreateRange.z * Scale.z;
			}
			else
			{
				//MaxCreate.x = m_MaxCreateRange.x * m_pRefBundle->m_vTargetScale.x;
				//MaxCreate.y = m_MaxCreateRange.y * m_pRefBundle->m_vTargetScale.y;
				//MaxCreate.z = m_MaxCreateRange.z * m_pRefBundle->m_vTargetScale.z;
				MaxCreate = m_MaxCreateRange * m_pRefBundle->m_fTargetScale;

				//MinCreate.x = m_MinCreateRange.x * m_pRefBundle->m_vTargetScale.x;
				//MinCreate.y = m_MinCreateRange.y * m_pRefBundle->m_vTargetScale.y;
				//MinCreate.z = m_MinCreateRange.z * m_pRefBundle->m_vTargetScale.z;
				MinCreate = m_MinCreateRange * m_pRefBundle->m_fTargetScale;
			}

			__Vector3 vCreatePos;
			vCreatePos.Set(
				MinCreate.x + ((MaxCreate.x - MinCreate.x) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.y + ((MaxCreate.y - MinCreate.y) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.z + ((MaxCreate.z - MinCreate.z) * ((float) (rand() % 100) / 100.0f)));

			if (RotateQuaternion(__Vector3(0.0f, 0.0f, 1.0f), pParticle->m_vAxis, &Qt))
			{
				RotMtx      = Qt;
				vCreatePos *= RotMtx;
			}
			else if (Qt.w != 1.0f)
			{
				vCreatePos *= -1.0f;
			}

			pParticle->m_vCreatePoint  = m_pRefBundle->m_vPos + m_vCurrPos;
			pParticle->m_vLcPos        = vCreatePos;
			vDir                      -= vCreatePos;
			vDir.Normalize();
			pParticle->m_vVelocity = vDir * m_fPtVelocity;
		}
		else
		{
			__Vector3 MaxCreate, MinCreate;
			MaxCreate = m_MaxCreateRange;
			MinCreate = m_MinCreateRange;
			if (m_bAnimKey && m_pShape)
			{
				__Vector3 Scale  = m_pShape->Scale();
				MaxCreate.x     *= Scale.x;
				MaxCreate.y     *= Scale.y;
				MaxCreate.z     *= Scale.z;

				MinCreate.x     *= Scale.x;
				MinCreate.y     *= Scale.y;
				MinCreate.z     *= Scale.z;
			}

			__Vector3 vCreatePos;
			vCreatePos.Set(
				MinCreate.x + ((MaxCreate.x - MinCreate.x) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.y + ((MaxCreate.y - MinCreate.y) * ((float) (rand() % 100) / 100.0f)),
				MinCreate.z + ((MaxCreate.z - MinCreate.z) * ((float) (rand() % 100) / 100.0f)));

			if (RotateQuaternion(__Vector3(0.0f, 0.0f, 1.0f), pParticle->m_vAxis, &Qt))
			{
				RotMtx      = Qt;
				vCreatePos *= RotMtx;
			}
			else if (Qt.w != 1.0f)
			{
				vCreatePos *= -1.0f;
			}

			pParticle->m_vCreatePoint  = m_pRefBundle->m_vPos + m_vCurrPos;
			pParticle->m_vLcPos        = vCreatePos;
			vDir                      -= vCreatePos;
			vDir.Normalize();
			pParticle->m_vVelocity = vDir * m_fPtVelocity;
		}

		m_pVBList_Alive.push_back((*it));
		m_pVBList_Dead.erase(it);
	}
}

//
//
//
void CN3FXPartParticles::CreateParticles()
{
	int iNumLiveParticle = static_cast<int>(m_pVBList_Alive.size());
	if (iNumLiveParticle > m_iNumLodParticle)
		return;

	if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
		CreateParticles_Spread();
	else if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_GATHER)
		CreateParticles_Gather();
}

//
//
//
bool CN3FXPartParticles::IsDead()
{
	if (m_pVBList_Alive.size() == 0)
		return true;

	return false;
}

void CN3FXPartParticles::Duplicate(CN3FXPartParticles* pSrc)
{
	if (!pSrc)
		return;

	CN3FXPartBase::Duplicate(pSrc);

	m_iNumParticle = pSrc->m_iNumParticle;
	if (m_iNumParticle > 0)
		InitVB();

	m_pair_fParticleSize.first  = pSrc->m_pair_fParticleSize.first;
	m_pair_fParticleSize.second = pSrc->m_pair_fParticleSize.second;

	m_pair_fParticleLife.first  = pSrc->m_pair_fParticleLife.first;
	m_pair_fParticleLife.second = pSrc->m_pair_fParticleLife.second;

	m_MinCreateRange            = pSrc->m_MinCreateRange;
	m_MaxCreateRange            = pSrc->m_MaxCreateRange;

	m_fCreateDelay              = pSrc->m_fCreateDelay;
	m_iNumCreate                = pSrc->m_iNumCreate;

	m_dwEmitType                = pSrc->m_dwEmitType;
	if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
	{
		m_uEmitCon.fEmitAngle = pSrc->m_uEmitCon.fEmitAngle;
	}
	else if (m_dwEmitType == FX_PART_PARTICLE_EMIT_TYPE_GATHER)
	{
		m_uEmitCon.vGatherPoint = pSrc->m_uEmitCon.vGatherPoint;
	}

	m_vPtEmitDir     = pSrc->m_vPtEmitDir;
	m_fPtVelocity    = pSrc->m_fPtVelocity;
	m_fPtAccel       = pSrc->m_fPtAccel;
	m_fPtRotVelocity = pSrc->m_fPtRotVelocity;
	m_fPtGravity     = pSrc->m_fPtGravity;

	m_bChangeColor   = pSrc->m_bChangeColor;
	if (m_bChangeColor)
	{
		memcpy((char*) &(m_dwChangeColor[0]), (char*) pSrc->m_dwChangeColor,
			sizeof(uint32_t) * NUM_KEY_COLOR);
	}

	m_bAnimKey = pSrc->m_bAnimKey;
	if (m_bAnimKey)
	{
		m_fMeshFPS = pSrc->m_fMeshFPS;

		if (m_pShape)
		{
			delete m_pShape;
			m_pShape = nullptr;
		}

		m_pShape    = new CN3FXShape;

		m_pRefShape = s_MngFXShape.Get(pSrc->m_pRefShape->FileName());
		m_pShape->Duplicate(m_pRefShape);
	}

	m_fTexRotateVelocity = pSrc->m_fTexRotateVelocity;
	m_fScaleVelX         = pSrc->m_fScaleVelX;
	m_fScaleVelY         = pSrc->m_fScaleVelY;

	Init();

	return;
}

//
//
//
bool CN3FXPartParticles::GetColor(int key, uint32_t& color) const
{
	if (key < 0 || key >= NUM_KEY_COLOR)
		return false;

	color = m_dwChangeColor[key];
	return true;
}

//
// 두 방향 벡터가 있을때 하나의 방향벡터에서 다른 하나의 방향벡터로 회전하는 mtx구하기..
//
bool CN3FXPartParticles::RotateQuaternion(
	const __Vector3& vSrcDir_, const __Vector3& vDestDir_, __Quaternion* pQt) const
{
	__Vector3 vSrcDir  = vSrcDir_;
	__Vector3 vDestDir = vDestDir_;

	vSrcDir.Normalize();
	vDestDir.Normalize();

	__Vector3 vDirAxis;
	vDirAxis.Cross(vSrcDir, vDestDir);

	float fDirAng = acos(vSrcDir.Dot(vDestDir));
	pQt->RotationAxis(vDirAxis, fDirAng);

	if (vDirAxis.x == 0.0f && vDirAxis.y == 0.0f && vDirAxis.z == 0.0f)
		return false;

	return true;
}
