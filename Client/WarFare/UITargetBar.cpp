#include "StdAfx.h"
#include "UITargetBar.h"
#include "GameBase.h"
#include "text_resources.h"

#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>

CUITargetBar::CUITargetBar()
{
	m_pProgressHP = NULL;
	m_pStringID = NULL;
	m_fTimeSendPacketLast = 0;
}

CUITargetBar::~CUITargetBar()
{
}

void CUITargetBar::Release()
{
	CN3UIBase::Release();

	m_pProgressHP = NULL;
	m_pStringID = NULL;
	m_fTimeSendPacketLast = 0;
}

void CUITargetBar::UpdateHP(int iHP, int iHPMax, bool bUpdateImmediately)
{
	__ASSERT(iHPMax > 0, "Invalid Max HP");
	if(iHP < 0 || iHPMax <= 0) return;
	if(NULL == m_pProgressHP) return;

	int iPercentage = iHP * 100 / iHPMax;

	if(bUpdateImmediately) m_pProgressHP->SetCurValue(iPercentage);
	else m_pProgressHP->SetCurValue(iPercentage, 0.5f, 50.0f);				// 1초뒤에 초당 50 의 속도로 변하게 한다.
	return;
}

BOOL CUITargetBar::SetIDString(const std::string& szID, D3DCOLOR crID)
{
	m_pStringID->SetString(szID);
	m_pStringID->SetColor(crID);
	return TRUE;
}

bool CUITargetBar::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;
	CN3UIString* amountStr = new CN3UIString();
	amountStr->Init(this);

    m_pProgressHP = (CN3UIProgress*)GetChildByID("pro_target");	__ASSERT(m_pProgressHP, "NULL UI Component!!");
	m_pStringID = (CN3UIString*)GetChildByID("text_target");	__ASSERT(m_pStringID, "NULL UI Component!!");

	if(m_pProgressHP) m_pProgressHP->SetRange(0, 100);
	if(m_pStringID) // 폰트를 바꾼다.
	{
		std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);
		
		uint32_t dwH = m_pStringID->GetFontHeight();
		m_pStringID->SetFont(szFontID, dwH, FALSE, FALSE);
	}

	// NOTE: new target health bars depending on poison or curse
	CN3UIProgress* m_pProgressHP_posion = (CN3UIProgress*)GetChildByID("Progress_HP_slow");
	__ASSERT(m_pProgressHP_posion, "NULL UI Component!!");
	if (m_pProgressHP_posion) {
		m_pProgressHP_posion->SetRange(0, 100);
		m_pProgressHP_posion->SetVisible(false);
	}

	CN3UIProgress* m_pProgressHP_curse = (CN3UIProgress*)GetChildByID("Progress_HP_drop");
	__ASSERT(m_pProgressHP_curse, "NULL UI Component!!");
	if (m_pProgressHP_curse) {
		m_pProgressHP_curse->SetRange(0, 100);
		m_pProgressHP_curse->SetVisible(false);
	}

	CN3UIProgress* m_pProgressHP_lasting = (CN3UIProgress*)GetChildByID("Progress_HP_lasting");
	__ASSERT(m_pProgressHP_lasting, "NULL UI Component!!");
	if (m_pProgressHP_lasting) {
		m_pProgressHP_lasting->SetRange(0, 100);
		m_pProgressHP_lasting->SetVisible(false);
	}

	return true;
}
