#ifndef __N3ENG_H_
#define __N3ENG_H_

#pragma once

#include "N3Base.h"
#include "N3BaseFileAccess.h"

//-----------------------------------------------------------------------------
/*!
This class is used for initializing and configuring DirectX
*/
class CN3Eng : public CN3Base
{
public:
	typedef struct __D3DDEV_INFO
	{
		char szDeviceName[128];
		char szDeviceDesc[128];
		int nAdapter;
		int nDevice;

		D3DDEVTYPE DevType;
		int nModeCount;
		D3DDISPLAYMODE* pModes;
	} __D3DDevInfo;

protected:
	int m_nModeActive;
	int m_nAdapterCount;
	__D3DDEV_INFO m_DeviceInfo;

public:
	LPDIRECT3D9 m_lpD3D;

public:
	void Release() override;
	void SetViewPort(RECT& pRC);
	static void SetDefaultEnvironment();
	void LookAt(const __Vector3& vEye, const __Vector3& vAt, const __Vector3& vUp);
	bool Reset(bool bWindowed, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP);
	void SetProjection(float fNear, float fFar, float fLens, float fAspect);

	static void ClearAuto(RECT* pRC = nullptr);
	static void ClearZBuffer(const RECT* pRC = nullptr);
	static void Clear(D3DCOLOR crFill, RECT* pRC = nullptr);
	static void Present(HWND hWnd, RECT* pRC = nullptr);
	static void WaitForDeviceRestoration();

	bool Init(BOOL bWindowed, HWND hWnd, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP,
		BOOL bUseHW);

	BOOL FindDepthStencilFormat(UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat,
		D3DFORMAT* pDepthStencilFormat);

public:
	CN3Eng();
	~CN3Eng() override;
};

#endif
