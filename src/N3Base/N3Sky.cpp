// N3Sky.cpp: implementation of the CN3Sky class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3Sky.h"
#include "N3PMesh.h"
#include "N3PMeshInstance.h"
#include "N3Texture.h"

CN3Sky::CN3Sky()
{
	m_SkyColor.ChangeColor(0xFF5284DE); // Day color
	m_FogColor.ChangeColor(0xFFB5C6DE); // Day color
}

CN3Sky::~CN3Sky()
{
}

void CN3Sky::Release()
{
	CN3Base::Release();
	m_SkyColor.ChangeColor(0xFF5284DE); // Day color
	m_FogColor.ChangeColor(0xFFB5C6DE); // Day color
										//m_SkyColor.ChangeColor(0xFF081021);	// Night color
										//m_FogColor.ChangeColor(0xFF102942);	// Night color
}

void CN3Sky::Tick()
{
	m_SkyColor.Tick();
	m_FogColor.Tick();

	D3DCOLOR FogColor = m_FogColor.GetCurColor();
	for (int i = 0; i < 4; ++i)
	{
		m_vFronts[i].color = (m_vFronts[i].color & 0xff000000) | (FogColor & 0x00ffffff);
		m_Bottom[i].color  = FogColor;
	}
}

void CN3Sky::Render()
{
	// Set up a rotation matrix to orient the billboard towards the camera.
	__Matrix44 matWorld;
	__Vector3 vDir = s_CameraData.vEye - s_CameraData.vAt; // Camera direction
	if (0.0f == vDir.x)
		matWorld.Identity();
	else if (vDir.x > 0.0f)
		matWorld.RotationY(-atanf(vDir.z / vDir.x) - (__PI * 0.5f));
	else
		matWorld.RotationY(-atanf(vDir.z / vDir.x) + (__PI * 0.5f));
	s_lpD3DDev->SetTransform(D3DTS_WORLD, matWorld.toD3D());

	s_lpD3DDev->SetTexture(
		0, nullptr); // Do not set a texture as we want to create an illusion of distance fog.
	s_lpD3DDev->SetFVF(
		FVF_XYZCOLOR); // D3DFVF_XYZ | D3DFVF_DIFFUSE - Spreads the texture around the x, y, z vertices.

	// Draws the front and bottom billboard.
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_Bottom, sizeof(m_Bottom[0]));
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_vFronts, sizeof(m_vFronts[0]));
}

void CN3Sky::Init()
{
	Release();

	const float fWidth = 3.5f; // Width of billboard.
	const float fTopY  = 0.5f; // Height of the front billboard above the reddish part of the sky.
	const float fBottomY =
		0.1f; // Height of the bottom front billboard below the reddish part of the glow - the bottom is pure fog.
	const float fDistance     = 1.5f;  // Distance/Length of the front billboard.
	const float fBottomOffset = -5.0f; // Height of the bottom billboard.
	const D3DCOLOR color      = m_FogColor.GetCurColor();

	// Vertices of the front billboard.
	m_vFronts[0].Set(fWidth, fTopY, fDistance, 0x00ffffff & color);
	m_vFronts[1].Set(fWidth, fBottomY, fDistance, color);
	m_vFronts[2].Set(-fWidth, fBottomY, fDistance, color);
	m_vFronts[3].Set(-fWidth, fTopY, fDistance, 0x00ffffff & color);

	// Vertices of the bottom billboard.
	m_Bottom[0].Set(fWidth, fBottomY, fDistance, color);
	m_Bottom[1].Set(fWidth, fBottomOffset, fDistance, color);
	m_Bottom[2].Set(-fWidth, fBottomOffset, fDistance, color);
	m_Bottom[3].Set(-fWidth, fBottomY, fDistance, color);
}
