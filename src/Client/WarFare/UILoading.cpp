// UILoading.cpp: implementation of the UILoading class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UILoading.h"
#include "GameDef.h"
#include "GameEng.h"
#include "GameProcedure.h"
#include "UIManager.h"

#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>

CUILoading::CUILoading()
{
	m_pText_Info        = nullptr;
	m_pText_Version     = nullptr;
	m_pProgress_Loading = nullptr;
}

CUILoading::~CUILoading()
{
	CUILoading::Release();
}

void CUILoading::Release()
{
	CN3UIBase::Release();

	m_pText_Info        = nullptr;
	m_pText_Version     = nullptr;
	m_pProgress_Loading = nullptr;
}

bool CUILoading::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pText_Version, GetChildByID<CN3UIString>("Text_Version"));
	if (m_pText_Version != nullptr)
	{
		std::string version = fmt::format("Ver. {:.3f}", CURRENT_VERSION / 1000.0f);
		m_pText_Version->SetString(version);
	}

	N3_VERIFY_UI_COMPONENT(m_pText_Info, GetChildByID<CN3UIString>("Text_Info"));
	N3_VERIFY_UI_COMPONENT(m_pProgress_Loading, GetChildByID<CN3UIProgress>("Progress_Loading"));

	SetPosCenter();                  // 가운데로 맞추기..
	m_pText_Version->SetPos(10, 10); // Version 은 맨위에 표시..

	if (m_pProgress_Loading != nullptr)
		m_pProgress_Loading->SetRange(0, 100);

	return true;
}

void CUILoading::Render(const std::string& szInfo, int iPercentage)
{
	if (m_pText_Info)
		m_pText_Info->SetString(szInfo);
	if (m_pProgress_Loading)
		m_pProgress_Loading->SetCurValue(iPercentage);

	D3DCOLOR crEnv = 0x00000000;
	CGameProcedure::s_pEng->Clear(crEnv); // 배경은 검은색
	CN3Base::s_lpD3DDev->BeginScene();    // 씬 렌더 ㅅ작...

	CN3UIBase::Tick();
	CUIManager::RenderStateSet();
	CN3UIBase::Render();
	CUIManager::RenderStateRestore();

	CN3Base::s_lpD3DDev->EndScene(); // 씬 렌더 시작...
	CGameProcedure::s_pEng->Present(CN3Base::s_hWndBase);
}
