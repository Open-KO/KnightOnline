// GameEng.cpp: implementation of the CGameEng class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameEng.h"
#include "GameBase.h"
#include "N3WorldManager.h"

#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>

constexpr float LIGHTNING_DURATION = 2.0f;

CGameEng::CGameEng()
{
	m_pActiveCam = nullptr;

	// 프로그램이 실행된 경로..
	char szBuf[_MAX_PATH];
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	::GetModuleFileName(nullptr, szBuf, _MAX_PATH);
	_splitpath(szBuf, szDrive, szDir, nullptr, nullptr);
	_makepath(szBuf, szDrive, szDir, nullptr, nullptr);

	///////////////////////////////////////////////////////////////
	// 기본 카메라 세팅..
	CN3Camera* pCamera   = new CN3Camera();
	pCamera->m_bFogUse   = TRUE;
	pCamera->m_Data.fFOV = DegreesToRadians(70);
	pCamera->m_Data.fNP  = 0.7f;
	pCamera->m_Data.fFP  = 512.0f;
	this->CameraAdd(pCamera);
	this->SetActiveCamera(pCamera);

	m_vEyeToReach           = pCamera->EyePos();
	m_vAtToReach            = pCamera->AtPos();

	//	m_eViewPoint = VP_BACKWARD; // 기본은 1.5인칭(플레이어 뒤에서 바라본 모습)으로 시작.
	m_eViewPoint            = VP_THIRD_PERSON; // 기본은 3인칭으로 시작.
	m_fRotPitchFirstPerson  = 0.0f;            // 일인칭 시점일때 카메라 피치 각도.
	m_fRotPitchBackward     = 0.0f;            // 삼인칭 시점일때 카메라 피치 각도.
	m_fRotPitchFoward       = 0.0f;            // 앞에서 본 시점일때  카메라 피치 각도.
	m_fRotPitchThirdFirson  = DegreesToRadians(25.0f);
	m_fOffsetVPGod          = 7.0f;            // 위에서 본시점일때 카메라 거리..
	m_fRotYawVPGod          = 0.0f;            // 위에서 바라본 시점일 경우에 카메라 회전각도..
	m_fZoomBackwardOrFoward = 1.0f;            // 1.5인칭, 앞에서 본 시점일때 카메라 Zoom 1.0f 가 기준이다.
	// 기본 카메라 세팅..
	///////////////////////////////////////////////////////////////

	m_fLightningTimeRemain  = 0.0f; // 번개 칠때 타이머..

	///////////////////////////////////////////////////////////////
	// 기본 라이트 세팅
	/*
	D3DCOLORVALUE crLgt;

	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.8f;
	CN3Light* pLightGlobal = new CN3Light(); // 전체를 비출 라이트..
	pLightGlobal->m_Data.InitDirection(0, __Vector3(0,-1,0), crLgt);
	this->LightAdd(pLightGlobal);

	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.5f;
	CN3Light* pLightGlobal2 = new CN3Light(); // 반대 편에서 전체를 비출 라이트..
	pLightGlobal2->m_Data.InitDirection(1, __Vector3(0,1,0), crLgt);
	this->LightAdd(pLightGlobal2);

	crLgt.a = 0.0f, crLgt.r = crLgt.g = crLgt.b = 0.3f;
	CN3Light* pLight = new CN3Light(); // 카메라와 붙어 다닌다...
	pLight->m_Data.InitPoint(2, __Vector3(0,0,0), crLgt, 32.0f);
	this->LightAdd(pLight);
	*/
	// 기본 라이트 세팅
	///////////////////////////////////////////////////////////////
	m_pRefLightSun          = nullptr;
	m_pRefLightSupport      = nullptr;
	m_pRefLightCam          = nullptr;

	m_fFPDeltaCur           = 1.0f; // 현재
	m_fFPDeltaToReach       = 1.0f; // 이값을 목표로 해서 변한다.

									//	m_fLightDeltaCur = 1.0f;
	//	m_fLightDeltaToReach = 1.0f; // 이값을 목표로 해서 변한다.
}

CGameEng::~CGameEng()
{
	for (CN3Camera* pCamera : m_Cameras)
		delete pCamera;
	m_Cameras.clear();

	/*	it_Light itLgt = m_Lights.begin();
	iSize = m_Lights.size();
	for(i = 0; i < iSize; i++, itLgt++)
	{
		delete *itLgt; 
	}
	m_Lights.clear();

	itLgt = m_LightsBackup.begin();
	iSize = m_LightsBackup.size();
	for(i = 0; i < iSize; i++, itLgt++)
	{
		delete *itLgt; 
	}
	m_LightsBackup.clear();
*/
}

/*
void CGameEng::RestoreLighting()
{
	m_Lights.clear();	
	it_Light itLgt = m_LightsBackup.begin();
	while( itLgt != m_LightsBackup.end() )
	{
		CN3Light* pLgt = *itLgt++;
		m_Lights.push_back(pLgt);
	}
	m_LightsBackup.clear();
}

void CGameEng::BackupReleaseLighting()
{
	m_LightsBackup.clear();
	it_Light itLgt = m_Lights.begin();
	while( itLgt != m_Lights.end() )
	{
		CN3Light* pLgt = *itLgt++;
		m_LightsBackup.push_back(pLgt);
	}
	m_Lights.clear();
}
*/

void CGameEng::SetDefaultLight(CN3Light* pSun, CN3Light* pSupport, CN3Light* pCam)
{
	m_pRefLightSun     = pSun;
	m_pRefLightSupport = pSupport;
	m_pRefLightCam     = pCam;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

void CGameEng::Tick(const D3DCOLOR* crDiffuses, // Diffuse 라이트 색깔.. 3 개 쓴다.
	const D3DCOLOR* crAmbients,                 // Ambient 라이트 색깔.. 3 개 쓴다.
	const D3DCOLOR crFog,                       // 안개 색깔..
	const __Vector3& vPosPlayer,                // 플레이어 위치
	const __Quaternion& qtPlayer,               // 회전 쿼터니언
	float fHeightPlayer,                        // 키를 인수로 넣으면 카메라와 라이트 처리..
	float fSunRadianZ)                          // 해의 Z 각도..
{
	if (nullptr == m_pActiveCam)
		return;

	float fRadius      = fHeightPlayer * 2.0f;
	float fYaw         = 0;
	__Quaternion qtRot = qtPlayer;
	__Vector3 vAxis(0, 1, 0);
	qtRot.AxisAngle(vAxis, fYaw);

	if (vAxis.y < 0) // 회전축이 음수이면.
	{
		vAxis.y *= -1.0f;
		fYaw    *= -1.0f;
	}

	switch (m_eViewPoint)
	{
		case VP_BACKWARD:
		{
			qtRot.RotationYawPitchRoll(fYaw, m_fRotPitchBackward, 0);
			__Matrix44 mtxRot = qtRot;

			m_vEyeToReach.Set(0, 0, -(fRadius / s_CameraData.fFOV) * m_fZoomBackwardOrFoward);
			m_vAtToReach    = vPosPlayer;
			m_vAtToReach.y += fHeightPlayer * 0.8f;
			m_vEyeToReach   = m_vAtToReach + (m_vEyeToReach * mtxRot);
		}
		break;
		case VP_FIRST_PERSON:
		{
			qtRot.RotationYawPitchRoll(fYaw, m_fRotPitchFirstPerson, 0);
			__Matrix44 mtxRot  = qtRot;

			m_vEyeToReach      = vPosPlayer;
			m_vEyeToReach.y   += fHeightPlayer - 0.1f;
			m_vAtToReach.Set(0, 0, 1);
			m_vAtToReach = m_vEyeToReach + (m_vAtToReach * mtxRot);
		}
		break;
		case VP_FOWARD:
		{
			qtRot.RotationYawPitchRoll(fYaw, -m_fRotPitchFoward, 0);
			__Matrix44 mtxRot = qtRot;

			m_vEyeToReach.Set(0, 0, fRadius * m_fZoomBackwardOrFoward);
			m_vAtToReach    = vPosPlayer;
			m_vAtToReach.y += fHeightPlayer * 0.8f;
			m_vEyeToReach   = m_vAtToReach + (m_vEyeToReach * mtxRot);
		}
		break;
		case VP_THIRD_PERSON:
		{
			qtRot.RotationYawPitchRoll(m_fRotYawVPGod, m_fRotPitchThirdFirson, 0);
			__Matrix44 mtxRot  = qtRot;

			m_vAtToReach       = vPosPlayer;
			m_vAtToReach.y    += fHeightPlayer * 0.8f;
			m_vEyeToReach.Set(0, 0, -m_fOffsetVPGod);
			m_vEyeToReach = m_vAtToReach + (m_vEyeToReach * mtxRot);
		}
		break;

		default:
			break;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// 카메라 충돌 체크...
	if (VP_FIRST_PERSON == m_eViewPoint)                                       // 일인칭때는 충돌체크 안한다.
	{
		m_pActiveCam->LookAt(m_vEyeToReach, m_vAtToReach, __Vector3(0, 1, 0)); // 처다본다..
	}
	else
	{
		__Vector3 vEyeResult = m_vEyeToReach;
		float fNP            = m_pActiveCam->m_Data.fNP;
		CGameBase::ACT_WORLD->CheckCollisionCameraWithTerrain(vEyeResult, m_vAtToReach, fNP); // 지형과 충돌체크
		CGameBase::ACT_WORLD->CheckCollisionCameraWithShape(vEyeResult, m_vAtToReach, fNP);   // 오브젝트와 충돌체크..
		m_pActiveCam->LookAt(vEyeResult, m_vAtToReach, __Vector3(0, 1, 0));                   // 처다본다..
	}
	// 카메라 충돌 체크...
	////////////////////////////////////////////////////////////////////////////////////

	// 파 플레인 값을 조정..
	// ApplyCameraAndLight 에서 실제로 안개등의 값을 조절한다.
	if (m_fFPDeltaCur != m_fFPDeltaToReach)
	{
		float fFPChange  = (m_fFPDeltaToReach - m_fFPDeltaCur) * s_fSecPerFrm / 5.0f; // 5초동안 변하게 한다.
		m_fFPDeltaCur   += fFPChange;

		if (fFPChange < 0 && m_fFPDeltaCur < m_fFPDeltaToReach)
			m_fFPDeltaCur = m_fFPDeltaToReach;
		if (fFPChange > 0 && m_fFPDeltaCur > m_fFPDeltaToReach)
			m_fFPDeltaCur = m_fFPDeltaToReach;
	}

	m_pActiveCam->m_Data.fFP = s_Options.iViewDist * m_fFPDeltaCur;

	m_pActiveCam->m_FogColor = crFog; // 안개색을 맞춘다..
	m_pActiveCam->Tick();             // 적용및 사면체등등의 값들을 계산..

	__Matrix44 mtxRotSun;
	mtxRotSun.RotationZ(fSunRadianZ); // 해의 각도에 맞춘다..

	/*
	it_Light itLgt = m_Lights.begin();
	int iSize = m_Lights.size();
	for(int i = 0; i < iSize; i++, itLgt++)
	{
		CN3Light* pLight = *itLgt;
		__ASSERT(pLight, "Light pointer is NULL!!!");
		
		if(0 == pLight->m_Data.nNumber) // 기본 디렉셔널 라이트
		{
			// View Matrix 각도와 방향을 맞춘다..
//			__Vector3 vDir(0.0f,-1.5f,1.0f);
//			vDir.Normalize();
//			__Matrix44 mtxVI = s_CameraData.mtxViewInverse;
//			mtxVI.PosSet(0,0,0);
//			pLight->m_Data.Direction = vDir * mtxVI;

			// 해와 방향을 맞춘다..
			__Matrix44 mtxRot; mtxRot.RotationZ(fSunRadianZ);
			__Vector3 vDir(-1,0,1);
			vDir *= mtxRot;
			vDir.Normalize();
			pLight->m_Data.Direction = vDir;
			
			// 라이트 컬러 적용..
			pLight->m_Data.Diffuse = ::_D3DCOLOR_To_D3DCOLORVALUE(crDiffuses[0]);
			pLight->m_Data.Ambient = ::_D3DCOLOR_To_D3DCOLORVALUE(crAmbients[0]);
		}
		else if(1 == pLight->m_Data.nNumber)
		{
			__Vector3 vDir(2,-3, 2); // 위에서 아래로 ...
			vDir.Normalize();
			pLight->m_Data.Direction = vDir;
			
			// 라이트 컬러 적용..
			pLight->m_Data.Diffuse = ::_D3DCOLOR_To_D3DCOLORVALUE(crDiffuses[1]);
			pLight->m_Data.Ambient = ::_D3DCOLOR_To_D3DCOLORVALUE(crAmbients[1]);
		}
		else if(2 == pLight->m_Data.nNumber)
		{
			__Vector3 vPos = s_CameraData.vEye;
			vPos.y += 16.0f;
			pLight->PosSet(vPos); // 카메라 위에 가게 한다..
			
			// 라이트 컬러 적용..
			pLight->m_Data.Diffuse = ::_D3DCOLOR_To_D3DCOLORVALUE(crDiffuses[2]);
			pLight->m_Data.Ambient = ::_D3DCOLOR_To_D3DCOLORVALUE(crAmbients[2]);
		}


		// 번개 처리..
		if(m_fLightningTimeRemain > 0)
		{
			float fLightningDelta = 0;

			if(m_fLightningTimeRemain > LIGHTNING_DURATION * 0.8f)
				fLightningDelta = (m_fLightningTimeRemain - LIGHTNING_DURATION * 0.8f) / (LIGHTNING_DURATION * 0.2f);
			else
				fLightningDelta = m_fLightningTimeRemain / (LIGHTNING_DURATION * 0.8f);

			pLight->m_Data.Diffuse.r += (1.0f - pLight->m_Data.Diffuse.r) * fLightningDelta * 0.4f;
			pLight->m_Data.Diffuse.g += (1.0f - pLight->m_Data.Diffuse.g) * fLightningDelta * 0.5f;
			pLight->m_Data.Diffuse.b += (1.0f - pLight->m_Data.Diffuse.b) * fLightningDelta;

			m_fLightningTimeRemain -= CN3Base::s_fSecPerFrm;
			if(m_fLightningTimeRemain < 0) m_fLightningTimeRemain = 0;
		}

		pLight->Tick();
	}
	*/
	if (m_pRefLightSun)
	{
		// 해와 방향을 맞춘다..
		__Matrix44 mtxRot;
		mtxRot.RotationZ(fSunRadianZ);
		__Vector3 vDir(-1, 0, 1);
		vDir *= mtxRot;
		vDir.Normalize();
		m_pRefLightSun->m_Data.Direction = vDir;

		// 라이트 컬러 적용..
		m_pRefLightSun->m_Data.Diffuse   = ::_D3DCOLOR_To_D3DCOLORVALUE(crDiffuses[0]);
		m_pRefLightSun->m_Data.Ambient   = ::_D3DCOLOR_To_D3DCOLORVALUE(crAmbients[0]);

		// 번개 처리..
		if (m_fLightningTimeRemain > 0)
		{
			float fLightningDelta = 0;

			if (m_fLightningTimeRemain > LIGHTNING_DURATION * 0.8f)
				fLightningDelta = (m_fLightningTimeRemain - LIGHTNING_DURATION * 0.8f) / (LIGHTNING_DURATION * 0.2f);
			else
				fLightningDelta = m_fLightningTimeRemain / (LIGHTNING_DURATION * 0.8f);

			m_pRefLightSun->m_Data.Diffuse.r += (1.0f - m_pRefLightSun->m_Data.Diffuse.r) * fLightningDelta * 0.4f;
			m_pRefLightSun->m_Data.Diffuse.g += (1.0f - m_pRefLightSun->m_Data.Diffuse.g) * fLightningDelta * 0.5f;
			m_pRefLightSun->m_Data.Diffuse.b += (1.0f - m_pRefLightSun->m_Data.Diffuse.b) * fLightningDelta;

			m_fLightningTimeRemain           -= CN3Base::s_fSecPerFrm;
			if (m_fLightningTimeRemain < 0)
				m_fLightningTimeRemain = 0;
		}
	}
	if (m_pRefLightSupport)
	{
		__Vector3 vDir(2, -3, 2); // 위에서 아래로 ...
		vDir.Normalize();
		m_pRefLightSupport->m_Data.Direction = vDir;

		// 라이트 컬러 적용..
		m_pRefLightSupport->m_Data.Diffuse   = ::_D3DCOLOR_To_D3DCOLORVALUE(crDiffuses[1]);
		m_pRefLightSupport->m_Data.Ambient   = ::_D3DCOLOR_To_D3DCOLORVALUE(crAmbients[1]);

		// 번개 처리..
		if (m_fLightningTimeRemain > 0)
		{
			float fLightningDelta = 0;

			if (m_fLightningTimeRemain > LIGHTNING_DURATION * 0.8f)
				fLightningDelta = (m_fLightningTimeRemain - LIGHTNING_DURATION * 0.8f) / (LIGHTNING_DURATION * 0.2f);
			else
				fLightningDelta = m_fLightningTimeRemain / (LIGHTNING_DURATION * 0.8f);

			m_pRefLightSupport->m_Data.Diffuse.r += (1.0f - m_pRefLightSupport->m_Data.Diffuse.r) * fLightningDelta * 0.4f;
			m_pRefLightSupport->m_Data.Diffuse.g += (1.0f - m_pRefLightSupport->m_Data.Diffuse.g) * fLightningDelta * 0.5f;
			m_pRefLightSupport->m_Data.Diffuse.b += (1.0f - m_pRefLightSupport->m_Data.Diffuse.b) * fLightningDelta;

			m_fLightningTimeRemain               -= CN3Base::s_fSecPerFrm;
			if (m_fLightningTimeRemain < 0)
				m_fLightningTimeRemain = 0;
		}
	}
	if (m_pRefLightCam)
	{
		__Vector3 vPos  = s_CameraData.vEye;
		vPos.y         += 16.0f;
		m_pRefLightCam->PosSet(vPos); // 카메라 위에 가게 한다..

		// 라이트 컬러 적용..
		m_pRefLightCam->m_Data.Diffuse = ::_D3DCOLOR_To_D3DCOLORVALUE(crDiffuses[2]);
		m_pRefLightCam->m_Data.Ambient = ::_D3DCOLOR_To_D3DCOLORVALUE(crAmbients[2]);

		// 번개 처리..
		if (m_fLightningTimeRemain > 0)
		{
			float fLightningDelta = 0;

			if (m_fLightningTimeRemain > LIGHTNING_DURATION * 0.8f)
				fLightningDelta = (m_fLightningTimeRemain - LIGHTNING_DURATION * 0.8f) / (LIGHTNING_DURATION * 0.2f);
			else
				fLightningDelta = m_fLightningTimeRemain / (LIGHTNING_DURATION * 0.8f);

			m_pRefLightCam->m_Data.Diffuse.r += (1.0f - m_pRefLightCam->m_Data.Diffuse.r) * fLightningDelta * 0.4f;
			m_pRefLightCam->m_Data.Diffuse.g += (1.0f - m_pRefLightCam->m_Data.Diffuse.g) * fLightningDelta * 0.5f;
			m_pRefLightCam->m_Data.Diffuse.b += (1.0f - m_pRefLightCam->m_Data.Diffuse.b) * fLightningDelta;

			m_fLightningTimeRemain           -= CN3Base::s_fSecPerFrm;
			if (m_fLightningTimeRemain < 0)
				m_fLightningTimeRemain = 0;
		}
	}
}

void CGameEng::ApplyCameraAndLight()
{
	if (m_pActiveCam)
	{
		m_pActiveCam->Apply(); // 위의 거 대신에 하나만 쓴다...
	}
	/*
	for(it_Light it = m_Lights.begin(), itEnd = m_Lights.end(); it != itEnd; it++)
	{
		CN3Light* pLight = *it;
		__ASSERT(pLight, "Light pointer is NULL!!!");
		
		pLight->Apply();
	}
*/
}

void CGameEng::ViewPointChange(e_ViewPoint eVP)
{
	if (eVP < VP_BACKWARD || eVP > VP_THIRD_PERSON)
	{
		if (m_eViewPoint < VP_BACKWARD)
			m_eViewPoint = VP_BACKWARD;
		if (m_eViewPoint >= VP_THIRD_PERSON)
			m_eViewPoint = VP_THIRD_PERSON;

		int iVP = (int) (m_eViewPoint) + 1;
		if (iVP > (VP_THIRD_PERSON))
			iVP = VP_BACKWARD;
		m_eViewPoint = (e_ViewPoint) iVP; // 시점 증가..
	}
	else
	{
		m_eViewPoint = eVP; // 시점 증가..
	}
}

void CGameEng::CameraPitchAdd(float fRotXPerSec)
{
	if (nullptr == m_pActiveCam)
		return;

	constexpr float PitchMax = DegreesToRadians(70.0f);
	constexpr float PitchMin = -DegreesToRadians(50.0f);

	if (VP_BACKWARD == m_eViewPoint)
	{
		m_fRotPitchBackward += fRotXPerSec * s_fSecPerFrm;
		if (m_fRotPitchBackward > PitchMax)
			m_fRotPitchBackward = PitchMax; // 아래 보는값
		else if (m_fRotPitchBackward < PitchMin)
			m_fRotPitchBackward = PitchMin; // 위보는 값..
	}
	else if (VP_FIRST_PERSON == m_eViewPoint)
	{
		m_fRotPitchFirstPerson += fRotXPerSec * s_fSecPerFrm;
		if (m_fRotPitchFirstPerson > PitchMax)
			m_fRotPitchFirstPerson = PitchMax; // 아래 보는값
		else if (m_fRotPitchFirstPerson < PitchMin)
			m_fRotPitchFirstPerson = PitchMin; // 위보는 값..
	}
	else if (VP_FOWARD == m_eViewPoint)
	{
		m_fRotPitchFoward += fRotXPerSec * s_fSecPerFrm;
		if (m_fRotPitchFoward > PitchMax)
			m_fRotPitchFoward = PitchMax;                    // 아래 보는값
		else if (m_fRotPitchFoward < PitchMin)
			m_fRotPitchFoward = PitchMin;                    // 위보는 값..
	}
	else if (VP_THIRD_PERSON == m_eViewPoint)                // 위에서 보는 건 거리만 늘였다 줄였다 한다..
	{
		constexpr float PitchMin  = DegreesToRadians(10.0f); // 젤 밑에값 한계

		m_fRotPitchThirdFirson   += fRotXPerSec * s_fSecPerFrm;
		if (m_fRotPitchThirdFirson > PitchMax)
			m_fRotPitchThirdFirson = PitchMax; // 아래 보는값
		else if (m_fRotPitchThirdFirson < PitchMin)
			m_fRotPitchThirdFirson = PitchMin; // 위보는 값..
	}
}

void CGameEng::CameraYawAdd(float fRotYPerSec)
{
	if (VP_THIRD_PERSON == m_eViewPoint)              // 위에서 보는 건 ..
	{
		m_fRotYawVPGod += fRotYPerSec * s_fSecPerFrm; // 위에서 바라본 시점일 경우에 카메라 회전각도..
		return;                                       // 돌아간다!
	}
}

void CGameEng::CameraZoom(float fDelta)
{
	if (nullptr == m_pActiveCam)
		return;
	if (VP_BACKWARD == m_eViewPoint || VP_FOWARD == m_eViewPoint)
	{
		m_fZoomBackwardOrFoward -= fDelta * s_fSecPerFrm; // 1.5인칭, 앞에서 본 시점일때 카메라 Zoom 1.0f 가 기준이다.
		if (m_fZoomBackwardOrFoward < 0.4f)
			m_fZoomBackwardOrFoward = 0.4f;               // 아래 보는값
		else if (m_fZoomBackwardOrFoward >= 1.6f)
			m_fZoomBackwardOrFoward = 1.6f;
	}
	else if (VP_THIRD_PERSON == m_eViewPoint) // 위에서 보는 건 거리만 늘였다 줄였다 한다..
	{
		// NOTE (zoom way back): search for "zoom way back" to get here

		m_fOffsetVPGod -= 4.0f * fDelta * s_fSecPerFrm;
		if (m_fOffsetVPGod < 2.0f)
			m_fOffsetVPGod = 2.0f;  // 너무작아지면 멈추고..
		else if (m_fOffsetVPGod > 10.0f)
			m_fOffsetVPGod = 10.0f; // 너무 커져도 멈추고..
		return;                     // 돌아간다!
	}
}

void CGameEng::FarPlaneDeltaSet(float fFPDelta, bool bUpdateImmediately)
{
	if (fFPDelta < 0.25f)
		fFPDelta = 0.25f;
	if (fFPDelta > 1.0f)
		fFPDelta = 1.0f;

	m_fFPDeltaToReach = fFPDelta;
	if (bUpdateImmediately)
		m_fFPDeltaCur = fFPDelta;
}

//void CGameEng::LightDeltaSet(float fLightDelta, bool bUpdateImmediately)
//{
//	if(fLightDelta < 0.25f) fLightDelta = 0.25f;
//	if(fLightDelta > 1.0f) fLightDelta = 1.0f;
//
//	m_fLightDeltaToReach = fLightDelta;
//	if(bUpdateImmediately) m_fLightDeltaCur = fLightDelta;
//}

void CGameEng::Lightning()
{
	m_fLightningTimeRemain = LIGHTNING_DURATION; // 번개 칠때 타이머..
}
