// N3Light.cpp: implementation of the CN3Light class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3Light.h"

CN3Light::CN3Light()
{
	m_dwType |= OBJ_LIGHT;
	m_Data    = {};
}

CN3Light::~CN3Light()
{
}

void CN3Light::Release()
{
	m_Data = {};
	CN3Transform::Release();
}

bool CN3Light::Load(File& file)
{
	CN3Transform::Load(file);

	file.Read(&m_Data, sizeof(m_Data)); // 라이트 세팅.

	__ASSERT(m_Data.nNumber >= 0 && m_Data.nNumber < 8,
		"Light Loading Warning - Light 번호가 범위를 벗어났습니다.");

	return true;
}

#ifdef _N3TOOL
bool CN3Light::Save(File& file)
{
	CN3Transform::Save(file);

	file.Write(&m_Data, sizeof(m_Data)); // 라이트 세팅.

	return true;
}
#endif // end of _N3TOOL

void CN3Light::Tick(float fFrm)
{
	CN3Transform::Tick(fFrm);

	m_Data.Position = m_vPos;
}

void CN3Light::Apply()
{
	__ASSERT(m_Data.nNumber >= 0 && m_Data.nNumber < 8, "Invalid Light Number");
	s_lpD3DDev->LightEnable(m_Data.nNumber, m_Data.bOn);
	if (m_Data.bOn)
	{
		if (m_Data.Type == D3DLIGHT_POINT || m_Data.Type == D3DLIGHT_DIRECTIONAL
			|| m_Data.Type == D3DLIGHT_SPOT)
			s_lpD3DDev->SetLight(m_Data.nNumber, m_Data.toD3D());
	}
}
