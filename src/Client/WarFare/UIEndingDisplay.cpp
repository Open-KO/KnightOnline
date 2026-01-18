// UIEndingDisplay.cpp: implementation of the CUIEndingDisplay class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIEndingDisplay.h"
#include "UIManager.h"
#include "GameEng.h"

CUIEndingDisplay::CUIEndingDisplay()
{
}

CUIEndingDisplay::~CUIEndingDisplay()
{
	CUIEndingDisplay::Release();
}

void CUIEndingDisplay::Release()
{
	CN3UIBase::Release();
}

void CUIEndingDisplay::Render()
{
	// 화면 가운데로 맞추기..
	RECT rc = this->GetRegion();
	int iX  = (s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	int iY  = (s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	this->SetPos(iX, iY);

	D3DCOLOR crEnv = 0x00000000;
	CGameEng::Clear(crEnv);             // 배경은 검은색
	CGameEng::s_lpD3DDev->BeginScene(); // 씬 렌더 ㅅ작...

	CUIManager::RenderStateSet();
	CN3UIBase::Render();
	CUIManager::RenderStateRestore();

	CGameEng::s_lpD3DDev->EndScene(); // 씬 렌더 시작...
	CGameEng::Present(CN3Base::s_hWndBase);
}
