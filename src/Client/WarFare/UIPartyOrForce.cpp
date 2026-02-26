// UIPartyOrForce.cpp: implementation of the CUIPartyOrForce class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIPartyOrForce.h"
#include "UIManager.h"
#include "GameProcMain.h"
#include "PlayerOtherMgr.h"

#include <N3Base/N3UIArea.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIStatic.h>

CUIPartyOrForce::CUIPartyOrForce()
{
	for (int i = 0; i < MAX_PARTY_SIZE; i++)
	{
		m_pProgress_HPs[i]       = nullptr; // 부대원갯수 만큼... HP Gauge
		m_pProgress_HPReduce[i]  = nullptr; // 부대원갯수 만큼... HP Reduce
		m_pProgress_HPSlow[i]    = nullptr; // HP Slow
		m_pProgress_HPLasting[i] = nullptr; // HP Lasting
		m_pProgress_MP[i]        = nullptr; // MP Bar
		m_pStatic_IDs[i]         = nullptr; // Names for each party member
		m_pAreas[i]              = nullptr;
	}

	m_iIndexSelected = -1; // 현재 선택된 멤버인덱스..
}

CUIPartyOrForce::~CUIPartyOrForce()
{
}

void CUIPartyOrForce::Release()
{
	CN3UIBase::Release();

	m_Members.clear();
	m_iIndexSelected = -1; // 현재 선택된 멤버인덱스..

	for (int i = 0; i < MAX_PARTY_SIZE; i++)
	{
		m_pProgress_HPs[i]       = nullptr; // 부대원갯수 만큼... HP Gauge
		m_pProgress_HPReduce[i]  = nullptr; // 부대원갯수 만큼... HP Reduce
		m_pProgress_HPSlow[i]    = nullptr; // HP Slow
		m_pProgress_HPLasting[i] = nullptr; // HP Lasting
		m_pProgress_MP[i]        = nullptr; // MP Bar
		m_pStatic_IDs[i]         = nullptr; // Names for each party member
		m_pAreas[i]              = nullptr;
	}
}

bool CUIPartyOrForce::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	std::string szID;
	for (int i = 0; i < MAX_PARTY_SIZE; i++) // 빈곳을 찾자..
	{
		szID = fmt::format("progress_hp_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pProgress_HPs[i], GetChildByID<CN3UIProgress>(szID));
		if (m_pProgress_HPs[i] != nullptr)
		{
			m_pProgress_HPs[i]->SetVisible(false);
			m_pProgress_HPs[i]->SetRange(0, 100);
		}

		szID = fmt::format("progress_hp_{}_slow", i);
		N3_VERIFY_UI_COMPONENT(m_pProgress_HPSlow[i], GetChildByID<CN3UIProgress>(szID));
		if (m_pProgress_HPSlow[i] != nullptr)
		{
			m_pProgress_HPSlow[i]->SetVisible(false);
			m_pProgress_HPSlow[i]->SetRange(0, 100);
		}

		szID = fmt::format("progress_hp_{}_drop", i);
		N3_VERIFY_UI_COMPONENT(m_pProgress_HPReduce[i], GetChildByID<CN3UIProgress>(szID));
		if (m_pProgress_HPReduce[i] != nullptr)
		{
			m_pProgress_HPReduce[i]->SetVisible(false);
			m_pProgress_HPReduce[i]->SetRange(0, 100);
		}

		szID = fmt::format("progress_hp_{}_lasting", i);
		N3_VERIFY_UI_COMPONENT(m_pProgress_HPLasting[i], GetChildByID<CN3UIProgress>(szID));
		if (m_pProgress_HPLasting[i] != nullptr)
		{
			m_pProgress_HPLasting[i]->SetVisible(false);
			m_pProgress_HPLasting[i]->SetRange(0, 100);
		}

		szID = fmt::format("progress_mp_{}_curse", i);
		N3_VERIFY_UI_COMPONENT(m_pProgress_MP[i], GetChildByID<CN3UIProgress>(szID));
		if (m_pProgress_MP[i] != nullptr)
		{
			m_pProgress_MP[i]->SetVisible(false);
			m_pProgress_MP[i]->SetRange(0, 100);
		}

		szID = fmt::format("static_name_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pStatic_IDs[i], GetChildByID<CN3UIStatic>(szID));
		if (m_pStatic_IDs[i] != nullptr)
			m_pStatic_IDs[i]->SetVisible(false);

		szID = fmt::format("Area_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pAreas[i], GetChildByID<CN3UIArea>(szID));
	}

	MemberInfoReInit();

	return true;
}

bool CUIPartyOrForce::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		__InfoPartyOrForce* pIP = nullptr;
		auto it = m_Members.begin(), itEnd = m_Members.end();
		for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
		{
			//			if(m_pStatic_IDs[i] && pSender == m_pStatic_IDs[i])
			if (pSender == m_pAreas[i])
			{
				pIP              = &(*it);
				m_iIndexSelected = i; // 현재 선택된 멤버인덱스..
				break;
			}
		}

		if (pIP)
			CGameProcedure::s_pProcMain->TargetSelect(pIP->iID, true);
		else
			CGameProcedure::s_pProcMain->TargetSelect(-1, false);
	}

	return true;
}

void CUIPartyOrForce::Render()
{
	if (false == m_bVisible)
		return;

	CN3UIBase::Render();

	if (m_iIndexSelected < 0 || m_iIndexSelected >= static_cast<int>(m_Members.size()) || m_iIndexSelected >= MAX_PARTY_SIZE)
		return;

	if (nullptr == m_pStatic_IDs[m_iIndexSelected] || nullptr == m_pProgress_HPs[m_iIndexSelected])
		return;

	RECT rc1    = m_pStatic_IDs[m_iIndexSelected]->GetRegion();
	rc1.left   -= 2;
	rc1.top    -= 2;
	rc1.right  += 2;
	rc1.bottom += 2;

	RECT rc2    = m_pProgress_HPs[m_iIndexSelected]->GetRegion();
	rc2.left   -= 2;
	rc2.top    -= 2;
	rc2.right  += 2;
	rc2.bottom += 2;

	RECT rc;
	rc.left   = (rc1.left < rc2.left) ? rc1.left : rc2.left;
	rc.top    = (rc1.top < rc2.top) ? rc1.top : rc2.top;
	rc.right  = (rc1.right > rc2.right) ? rc1.right : rc2.right;
	rc.bottom = (rc1.bottom > rc2.bottom) ? rc1.bottom : rc2.bottom;

	CN3Base::RenderLines(rc, 0xff00ff00); // 선택 표시..
}

bool CUIPartyOrForce::TargetByIndex(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_Members.size()))
		return false;

	auto it = m_Members.begin();
	std::advance(it, iIndex);

	__InfoPartyOrForce* pIP = &(*it);
	m_iIndexSelected        = iIndex; // 현재 선택된 멤버인덱스..

	if (pIP)
		CGameProcedure::s_pProcMain->TargetSelect(pIP->iID, true);

	return true;
}

const __InfoPartyOrForce* CUIPartyOrForce::MemberInfoGetByID(int iID, int& iIndexResult)
{
	if (m_Members.empty())
		return nullptr;

	auto it = m_Members.begin(), itEnd = m_Members.end();
	iIndexResult = 0;
	for (; it != itEnd; it++, iIndexResult++)
	{
		if (iID == it->iID)
		{
			return &(*it);
		}
	}

	iIndexResult = -1;
	return nullptr;
}

const __InfoPartyOrForce* CUIPartyOrForce::MemberInfoGetByIndex(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_Members.size()))
		return nullptr;

	auto it = m_Members.begin();
	std::advance(it, iIndex);

	return &(*it);
}

CPlayerOther* CUIPartyOrForce::MemberGetByNearst(const __Vector3& vPosPlayer)
{
	if (m_Members.empty())
		return nullptr;

	float fDistMin = FLT_MAX, fDistTmp = 0;
	CPlayerOther* pTarget = nullptr;

	auto it = m_Members.begin(), itEnd = m_Members.end();
	for (; it != itEnd; it++)
	{
		CPlayerOther* pUPC = CGameBase::s_pOPMgr->UPCGetByID(it->iID, false);
		if (nullptr == pUPC)
			continue;

		fDistTmp = pUPC->Distance(vPosPlayer);
		if (fDistTmp < fDistMin)
		{
			pTarget  = pUPC;
			fDistMin = fDistTmp;
		}
	}

	return pTarget;
}

const __InfoPartyOrForce* CUIPartyOrForce::MemberAdd(
	int iID, const std::string& szID, int iLevel, e_Class eClass, int iHP, int iHPMax, int iMP, int iMPMax)
{
	__InfoPartyOrForce InfoTmp;
	InfoTmp.iID    = iID;
	InfoTmp.szID   = szID;
	InfoTmp.iLevel = iLevel;
	InfoTmp.iHP    = iHP;
	InfoTmp.iHPMax = iHPMax;
	InfoTmp.iMP    = iMP;
	InfoTmp.iMPMax = iMPMax;
	InfoTmp.eClass = eClass;

	m_Members.push_back(std::move(InfoTmp));

	auto it = m_Members.end();
	it--;

	MemberInfoReInit();

	return &(*it);
}

bool CUIPartyOrForce::MemberRemove(int iID)
{
	if (m_Members.empty())
		return false;

	auto it = m_Members.begin(), itEnd = m_Members.end();
	for (; it != itEnd; it++)
	{
		if (iID == it->iID)
		{
			m_Members.erase(it);
			this->MemberInfoReInit();
			return true;
		}
	}

	return false;
}

void CUIPartyOrForce::MemberDestroy()
{
	m_Members.clear();

	// 빈곳을 찾자..
	for (int i = 0; i < MAX_PARTY_SIZE; i++)
	{
		if (m_pProgress_HPs[i] != nullptr)
			m_pProgress_HPs[i]->SetVisible(false);

		if (m_pProgress_HPReduce[i] != nullptr)
			m_pProgress_HPReduce[i]->SetVisible(false);

		if (m_pProgress_MP[i] != nullptr)
			m_pProgress_MP[i]->SetVisible(false);

		if (m_pProgress_HPSlow[i] != nullptr)
			m_pProgress_HPSlow[i]->SetVisible(false);

		if (m_pProgress_HPLasting[i] != nullptr)
			m_pProgress_HPLasting[i]->SetVisible(false);

		if (m_pStatic_IDs[i] != nullptr)
			m_pStatic_IDs[i]->SetVisible(false);
	}

	MemberInfoReInit();
}

void CUIPartyOrForce::MemberInfoReInit() // 파티원 구성이 변경될때.. 순서 및 각종 정보 업데이트..
{
	auto it = m_Members.begin(), itEnd = m_Members.end();
	for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
	{
		__InfoPartyOrForce* pIP = &(*it); // 디버깅 하기 쉬우라고 이렇게 했다..
		if (pIP->iHPMax <= 0)
		{
			__ASSERT(0, "Invalid Party memeber HP");
			continue;
		}

		const int iHPPercent = pIP->iHP * 100 / pIP->iHPMax;

		if (m_pProgress_HPs[i] != nullptr)
			m_pProgress_HPs[i]->SetCurValue(iHPPercent);

		if (m_pProgress_HPReduce[i] != nullptr)
			m_pProgress_HPReduce[i]->SetCurValue(iHPPercent);

		if (m_pProgress_HPSlow[i] != nullptr)
			m_pProgress_HPSlow[i]->SetCurValue(iHPPercent);

		if (m_pProgress_HPLasting[i] != nullptr)
			m_pProgress_HPLasting[i]->SetCurValue(iHPPercent);

		if (m_pProgress_MP[i] != nullptr)
			m_pProgress_MP[i]->SetCurValue(pIP->iMP * 100 / pIP->iMPMax);

		if (m_pStatic_IDs[i] != nullptr)
		{
			m_pStatic_IDs[i]->SetString(pIP->szID);
			m_pStatic_IDs[i]->SetVisible(true);
		}
	}

	if (m_Members.empty())
		SetVisible(false); // 멤버가 없으면 숨긴다.
	else
		SetVisible(true);  // 멤버가 있으면 보인다.
}

const __InfoPartyOrForce* CUIPartyOrForce::MemberInfoGetSelected()
{
	if (m_iIndexSelected < 0 || m_iIndexSelected >= static_cast<int>(m_Members.size()))
		return nullptr;

	auto it = m_Members.begin();
	std::advance(it, m_iIndexSelected);

	return &(*it);
}

void CUIPartyOrForce::MemberHPChange(int iID, int iHP, int iHPMax, int iMP, int iMPMax)
{
	auto it = m_Members.begin(), itEnd = m_Members.end();
	for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
	{
		__InfoPartyOrForce* pIP = &(*it); // 디버깅 하기 쉬우라고 이렇게 했다..
		if (pIP->iID == iID)
		{
			pIP->iHP             = iHP;
			pIP->iHPMax          = iHPMax;
			pIP->iMP             = iMP;
			pIP->iMPMax          = iMPMax;

			const int iHPPercent = pIP->iHP * 100 / pIP->iHPMax;

			if (m_pProgress_HPs[i] != nullptr)
				m_pProgress_HPs[i]->SetCurValue(iHPPercent, 0.7f, 50.0f);

			if (m_pProgress_HPReduce[i] != nullptr)
				m_pProgress_HPReduce[i]->SetCurValue(iHPPercent, 0.7f, 50.0f);

			if (m_pProgress_HPSlow[i] != nullptr)
				m_pProgress_HPSlow[i]->SetCurValue(iHPPercent, 0.7f, 50.0f);

			if (m_pProgress_HPLasting[i] != nullptr)
				m_pProgress_HPLasting[i]->SetCurValue(iHPPercent, 0.7f, 50.0f);

			if (m_pProgress_MP[i] != nullptr)
				m_pProgress_MP[i]->SetCurValue(pIP->iMP * 100 / pIP->iMPMax, 0.7f, 50.0f);
			break;
		}
	}
}

void CUIPartyOrForce::MemberStatusChange(int iID, e_PartyStatus ePS, bool bSuffer)
{
	auto it = m_Members.begin(), itEnd = m_Members.end();
	__InfoPartyOrForce* pIP = nullptr;
	for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
	{
		pIP = &(*it); // 디버깅 하기 쉬우라고 이렇게 했다..
		if (pIP->iID == iID)
		{
			if (PARTY_STATUS_DOWN_HP == ePS)
				pIP->bSufferDown_HP = bSuffer;
			else if (PARTY_STATUS_DOWN_ETC == ePS)
				pIP->bSufferDown_Etc = bSuffer;
			break;
		}
	}
}

void CUIPartyOrForce::MemberLevelChange(int iID, int iLevel)
{
	auto it = m_Members.begin(), itEnd = m_Members.end();
	__InfoPartyOrForce* pIP = nullptr;
	for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
	{
		pIP = &(*it); // 디버깅 하기 쉬우라고 이렇게 했다..
		if (pIP->iID == iID)
		{
			pIP->iLevel = iLevel;
			break;
		}
	}
}

void CUIPartyOrForce::MemberClassChange(int iID, e_Class eClass)
{
	auto it = m_Members.begin(), itEnd = m_Members.end();
	__InfoPartyOrForce* pIP = nullptr;
	for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
	{
		pIP = &(*it); // 디버깅 하기 쉬우라고 이렇게 했다..
		if (pIP->iID == iID)
		{
			pIP->eClass = eClass;
			break;
		}
	}
}

void CUIPartyOrForce::Tick()
{
	CN3UIBase::Tick();

	bool bBlink      = false;
	uint64_t dwTime  = GetTickCount64();

	dwTime           = dwTime / 1000;
	dwTime          %= 2;

	if (dwTime == 1)
		bBlink = true;

	auto it = m_Members.begin(), itEnd = m_Members.end();
	for (int i = 0; it != itEnd && i < MAX_PARTY_SIZE; it++, i++)
	{
		__InfoPartyOrForce* pIP = &(*it); // 디버깅 하기 쉬우라고 이렇게 했다..
		if (m_pProgress_HPs[i] != nullptr)
		{
			if (pIP->bSufferDown_HP || pIP->bSufferDown_Etc)
				m_pProgress_HPs[i]->SetVisible(false);
			else
				m_pProgress_HPs[i]->SetVisible(true);
		}

		if (pIP->bSufferDown_HP && pIP->bSufferDown_Etc)
		{
			if (bBlink)
			{
				if (m_pProgress_HPReduce[i] != nullptr)
					m_pProgress_HPReduce[i]->SetVisible(true);

				if (m_pProgress_MP[i] != nullptr)
					m_pProgress_MP[i]->SetVisible(false);
			}
			else
			{
				if (m_pProgress_HPReduce[i] != nullptr)
					m_pProgress_HPReduce[i]->SetVisible(false);

				if (m_pProgress_MP[i] != nullptr)
					m_pProgress_MP[i]->SetVisible(true);
			}
		}
		else
		{
			if (m_pProgress_HPReduce[i] != nullptr)
			{
				if (pIP->bSufferDown_HP)
					m_pProgress_HPReduce[i]->SetVisible(true);
				else
					m_pProgress_HPReduce[i]->SetVisible(false);
			}

			if (m_pProgress_MP[i] != nullptr)
			{
				if (pIP->bSufferDown_Etc)
					m_pProgress_MP[i]->SetVisible(true);
				else
					m_pProgress_MP[i]->SetVisible(false);
			}
		}

		if (m_pProgress_MP[i] != nullptr)
			m_pProgress_MP[i]->SetVisible(true);
	}
}

bool CUIPartyOrForce::OnKeyPress(int iKey)
{
	// hotkey가 포커스 잡혀있을때는 다른 ui를 닫을수 없으므로 DIK_ESCAPE가 들어오면 포커스를 다시잡고
	if (iKey == DIK_ESCAPE)
	{
		// 열려있는 다른 유아이를 닫아준다.
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
		CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
		if (pFocus != nullptr && pFocus != this)
			pFocus->OnKeyPress(iKey);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
