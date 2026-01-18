// GameEng.h: interface for the CGameEng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEENG_H__5422B19E_D43E_4252_B79B_69323824F3F9__INCLUDED_)
#define AFX_GAMEENG_H__5422B19E_D43E_4252_B79B_69323824F3F9__INCLUDED_

#pragma once

#include <N3Base/N3Eng.h>
#include <list>

typedef std::list<class CN3Light*>::iterator it_Light;
typedef std::list<class CN3Camera*>::iterator it_Camera;

// 카메라 시점
enum e_ViewPoint : int8_t
{
	VP_BACKWARD     = 0, // 플레이어 등 뒤에서 바라본 모습 1.5인칭
	VP_FIRST_PERSON = 1, // 일인칭
	VP_FOWARD       = 2, // 플레이어 앞에서 바라봄..
	VP_THIRD_PERSON = 3, // 삼인칭 모드 .. 마우스도 동작함..
	VP_UNKNOWN      = -1 // 개발을 위해 카메라만 빠르게....
};

const float CAMERA_MOVE_SPEED = 5.0f;

class CGameEng : public CN3Eng
{
protected:
	CN3Light* m_pRefLightSun;
	CN3Light* m_pRefLightSupport;
	CN3Light* m_pRefLightCam;
	//std::list<CN3Light*>	m_Lights;
	//std::list<CN3Light*>	m_LightsBackup;
	std::list<CN3Camera*> m_Cameras;
	CN3Camera* m_pActiveCam;

	__Vector3 m_vEyeToReach;
	__Vector3 m_vAtToReach;

	float m_fFPDeltaCur;           // 현재
	float m_fFPDeltaToReach;       // 이값을 목표로 해서 변한다.

	float m_fRotPitchBackward;     // 1.5인칭 시점일때 카메라 피치 각도.
	float m_fRotPitchFirstPerson;  // 일인칭 시점일때 카메라 피치 각도.
	float m_fRotPitchFoward;       // 앞에서 본 시점일때  카메라 피치 각도.
	float m_fRotPitchThirdFirson;  // 삼인칭 시점 카메라 피치 각도

	float m_fOffsetVPGod;          // 위에서 바라본 시점일 경우에 거리..
	float m_fRotYawVPGod;          // 위에서 바라본 시점일 경우에 카메라 회전각도..

	float m_fZoomBackwardOrFoward; // 1.5인칭, 앞에서 본 시점일때 카메라 Zoom 1.0f 가 기준이다.
	float m_fLightningTimeRemain;  // 번개 칠때 타이머..

	e_ViewPoint m_eViewPoint;      // 카메라 시점.

public:
	void SetDefaultLight(CN3Light* pSun, CN3Light* pSupport, CN3Light* pCam);
	void RestoreLighting();
	void BackupReleaseLighting();
	void Lightning(); // 번개 치며 라이트 조절하기.
	float FarPlaneDeltaCur()
	{
		return m_fFPDeltaCur;
	}
	float FarPlaneDeltaToReach()
	{
		return m_fFPDeltaToReach;
	}
	void FarPlaneDeltaSet(float fFPDelta, bool bUpdateImmediately);

	// Light 함수들
	//void			LightAdd(CN3Light* pLight) { m_Lights.push_back(pLight); }
	//int				LightCount() { return m_Lights.size(); }
	//CN3Light*		Light(int index);

	// Camera 함수들
	int CameraCount() const
	{
		return static_cast<int>(m_Cameras.size());
	}

	void CameraSetActiveByIndex(int index)
	{
		CN3Camera* pCam = Camera(index);
		if (pCam != nullptr)
			m_pActiveCam = pCam;
	}

	CN3Camera* CameraGetActive()
	{
		return m_pActiveCam;
	}

	void CameraAdd(CN3Camera* pCamera)
	{
		m_Cameras.push_back(pCamera);
	}

	CN3Camera* Camera(int index);

	// 추가한것..
	void ViewPointChange(e_ViewPoint eVP = VP_UNKNOWN);
	e_ViewPoint ViewPoint()
	{
		return m_eViewPoint;
	} // 시점..
	void CameraPitchAdd(float fRotXPerSec);
	void CameraYawAdd(float fRotYPerSec);
	void CameraZoom(float fDelta);
	float CameraYaw()
	{
		return m_fRotYawVPGod;
	} // 위에서 바라본 시점일 경우에 카메라 회전각도..
	void SetActiveCamera(CN3Camera* pCamera)
	{
		m_pActiveCam = pCamera;
	}
	void Tick(const D3DCOLOR* crDiffuses, // Diffuse 라이트 색깔..
		const D3DCOLOR* crAmbients,       // Ambient 라이트 색깔..
		const D3DCOLOR crFog,             // 안개 색깔..
		const __Vector3& vPosPlayer,      // 플레이어 위치
		const __Quaternion& qtPlayer,     // 회전 쿼터니언
		float fHeightPlayer,              // 키를 인수로 넣으면 카메라와 라이트 처리..
		float fSunRadianZ);               // 해의 Z 각도..
	void ApplyCameraAndLight();           // 카메라와 라이트에 세팅된 값 적용. 라이트는 해의 각도에 따라 다르게 한다.

	CGameEng();
	~CGameEng() override;
};

/*
inline CN3Light* CGameEng::Light(int index)
{
	if(index < 0 || index >= m_Lights.size()) return nullptr;
	it_Light itLgt = m_Lights.begin();
	for(int i = 0; i < index; i++, itLgt++);

	return *itLgt;
}
*/

inline CN3Camera* CGameEng::Camera(int index)
{
	if (index < 0 || index >= static_cast<int>(m_Cameras.size()))
		return nullptr;

	auto it = m_Cameras.begin();
	std::advance(it, index);
	return *it;
}

#endif // !defined(AFX_GAMEENG_H__5422B19E_D43E_4252_B79B_69323824F3F9__INCLUDED_)
