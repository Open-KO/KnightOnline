#include "StdAfx.h"
#include "UITargetBar.h"
#include "GameBase.h"
#include "text_resources.h"

#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>

CUITargetBar::CUITargetBar()
{
	m_pProgress_HP         = nullptr;
	m_pProgress_HP_slow    = nullptr;
	m_pProgress_HP_drop    = nullptr;
	m_pProgress_HP_lasting = nullptr;

	m_pStringID            = nullptr;
	m_fTimeSendPacketLast  = 0;
}

CUITargetBar::~CUITargetBar()
{
}

void CUITargetBar::Release()
{
	CN3UIBase::Release();

	m_pProgress_HP         = nullptr;
	m_pProgress_HP_slow    = nullptr;
	m_pProgress_HP_drop    = nullptr;
	m_pProgress_HP_lasting = nullptr;
	m_pStringID            = nullptr;
	m_fTimeSendPacketLast  = 0;
}

void CUITargetBar::UpdateHP(int iHP, int iHPMax, bool bUpdateImmediately)
{
	__ASSERT(iHPMax > 0, "Invalid Max HP");
	if (iHP < 0 || iHPMax <= 0)
		return;
	if (nullptr == m_pProgress_HP)
		return;

	int iPercentage = iHP * 100 / iHPMax;

	if (bUpdateImmediately)
		m_pProgress_HP->SetCurValue(iPercentage);
	else
		m_pProgress_HP->SetCurValue(iPercentage, 0.5f, 50.0f); // 1초뒤에 초당 50 의 속도로 변하게 한다.
	return;
}

BOOL CUITargetBar::SetIDString(const std::string& szID, D3DCOLOR crID)
{
	m_pStringID->SetString(szID);
	m_pStringID->SetColor(crID);
	return TRUE;
}

bool CUITargetBar::Load(File& file)
{
	if (CN3UIBase::Load(file) == false)
		return false;
	CN3UIString* amountStr = new CN3UIString();
	amountStr->Init(this);

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP, GetChildByID<CN3UIProgress>("pro_target"));
	N3_VERIFY_UI_COMPONENT(m_pStringID, GetChildByID<CN3UIString>("text_target"));

	if (m_pProgress_HP != nullptr)
		m_pProgress_HP->SetRange(0, 100);

	// 폰트를 바꾼다.
	if (m_pStringID != nullptr)
	{
		std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);

		uint32_t dwH         = m_pStringID->GetFontHeight();
		m_pStringID->SetFont(szFontID, dwH, FALSE, FALSE);
	}

	// NOTE: new target health bars depending on poison or curse
	N3_VERIFY_UI_COMPONENT(m_pProgress_HP_slow, GetChildByID<CN3UIProgress>("Progress_HP_slow"));
	if (m_pProgress_HP_slow != nullptr)
	{
		m_pProgress_HP_slow->SetRange(0, 100);
		m_pProgress_HP_slow->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP_drop, GetChildByID<CN3UIProgress>("Progress_HP_drop"));
	if (m_pProgress_HP_drop != nullptr)
	{
		m_pProgress_HP_drop->SetRange(0, 100);
		m_pProgress_HP_drop->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP_lasting, GetChildByID<CN3UIProgress>("Progress_HP_lasting"));
	if (m_pProgress_HP_lasting != nullptr)
	{
		m_pProgress_HP_lasting->SetRange(0, 100);
		m_pProgress_HP_lasting->SetVisible(false);
	}

	return true;
}
