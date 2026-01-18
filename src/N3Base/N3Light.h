// N3Light.h: interface for the CN3Light class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3ILight_h__INCLUDED_)
#define AFX_N3ILight_h__INCLUDED_

#pragma once

#include "N3Transform.h"

class CN3Light : public CN3Transform
{
public:
	struct __Light : public __D3DLight9
	{
	public:
		BOOL bOn;    // 라이트가 켜져 있는지..
		int nNumber; // 0 ~ 8

		void Zero()
		{
			*this = {};
		}

		void InitPoint(int nLgtNumber, const __Vector3& dvPos, const __ColorValue& ltColor,
			float fRange = 10000.0f, float fAttenuation = 0.5f)
		{
			Zero();

			nNumber   = nLgtNumber; // 라이트 번호..
			Type      = D3DLIGHT_POINT;
			Position  = dvPos;
			//Specular =
			Diffuse   = ltColor;
			Ambient.r = ltColor.r * 0.7f;
			Ambient.g = ltColor.g * 0.7f;
			Ambient.b = ltColor.b * 0.7f;

			Falloff   = 1.0f; // 효과가 미미하고 부하기 걸리기 때문에 보통 1.0으로 쓴다.
			if (fRange < 0.0001f)
				fRange = 0.0001f;
			Attenuation0 = 1.0f - fAttenuation;
			Attenuation1 =
				fAttenuation
				/ fRange; // 감쇠 범위계산. 범위의 절반이 정확하게 절반의 감쇠가 되도록 한다..
			Attenuation2 = fAttenuation / (fRange * fRange);
			Range        = fRange * 4.0f;
			bOn          = TRUE;
		}

		void InitDirection(int nLgtNumber, const __Vector3& dvDir, const __ColorValue& ltColor)
		{
			Zero();

			nNumber   = nLgtNumber; // 라이트 번호..
			bOn       = TRUE;

			Type      = D3DLIGHT_DIRECTIONAL;
			Direction = dvDir;

			//Specular =
			Diffuse   = ltColor;
			Ambient.r = ltColor.r * 0.7f;
			Ambient.g = ltColor.g * 0.7f;
			Ambient.b = ltColor.b * 0.7f;
		}

		void InitSpot(int nLgtNumber, const __Vector3& dvPos, const __Vector3& dvDir,
			const __ColorValue& ltColor, float fTheta, float fPhi, float fRange = 10000.0f)
		{
			Zero();

			nNumber   = nLgtNumber; // 라이트 번호..
			Type      = D3DLIGHT_SPOT;
			Position  = dvPos;
			Direction = dvDir;

			Diffuse   = ltColor;
			Ambient.r = ltColor.r * 0.7f;
			Ambient.g = ltColor.g * 0.7f;
			Ambient.b = ltColor.b * 0.7f;

			if (fRange < 0.0001f)
				fRange = 0.0001f;

			Attenuation0 = 1.0f;
			Attenuation1 =
				1.0f
				/ (fRange
					/ 2.0f); // 감쇠 범위계산. 범위의 절반이 정확하게 절반의 감쇠가 되도록 한다..
			Range   = fRange;

			Falloff = 1.0f;  // 효과가 미미하고 부하기 걸리기 때문에 보통 1.0으로 쓴다.
			Theta   = fTheta;
			Phi     = fPhi;
			bOn     = TRUE;
		}
	};

	__Light m_Data;

public:
	void DirSet(const __Vector3& vDir)
	{
		DirSet(vDir.x, vDir.y, vDir.z);
	}

	void DirSet(float fx, float fy, float fz)
	{
		m_Data.Direction = { fx, fy, fz };
	}

	void PosSet(const __Vector3& vPos) override
	{
		PosSet(vPos.x, vPos.y, vPos.z);
	}

	void PosSet(float fx, float fy, float fz) override
	{
		m_Data.Position = m_vPos = { fx, fy, fz };
	}

	void Apply();                                    // 세팅된 라이트값을 실제 D3DDevice 에 적용
	void Tick(float fFrm = FRAME_SELFPLAY) override; // 라이트값만 세팅한다..

	bool Load(File& file) override;
#ifdef _N3TOOL
	bool Save(File& file) override;
#endif // end of _N3TOOL

	void Release() override;
	CN3Light();
	~CN3Light() override;
};

#endif // !defined(AFX_N3ILight_h__INCLUDED_)
