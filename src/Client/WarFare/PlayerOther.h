// PlayerOther.h: interface for the CPlayerOther class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PlayerOther_H__06F94EA0_0640_4532_B8CC_7FB9A99291AB__INCLUDED_)
#define AFX_PlayerOther_H__06F94EA0_0640_4532_B8CC_7FB9A99291AB__INCLUDED_

#pragma once

#include "GameBase.h"
#include "PlayerNPC.h"

class CPlayerOther : public CPlayerNPC
{
	friend class CPlayerOtherMgr;

public:
	__InfoPlayerOther m_InfoExt; // 캐릭터 정보 확장..
	bool m_bSit;

public:
	void InitFace() override;
	void InitHair() override;
	void KnightsInfoSet(int iID, const std::string& szName, int iGrade, int iRank) override;
	void SetSoundAndInitFont(uint32_t dwFontFlag = 0U) override;

	bool Init(enum e_Race eRace, int iFace, int iHair, uint32_t* pdwItemIDs, int* piItenDurabilities);
	void Tick() override;

	CPlayerOther();
	~CPlayerOther() override;
};

#endif // !defined(AFX_PlayerOther_H__06F94EA0_0640_4532_B8CC_7FB9A99291AB__INCLUDED_)
