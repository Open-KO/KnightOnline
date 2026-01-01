// PlayerOther.h: interface for the CPlayerOther class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PlayerOther_H__06F94EA0_0640_4532_B8CC_7FB9A99291AB__INCLUDED_)
#define AFX_PlayerOther_H__06F94EA0_0640_4532_B8CC_7FB9A99291AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameBase.h"
#include "PlayerNPC.h"

class CPlayerOther : public CPlayerNPC
{
	friend class CPlayerOtherMgr;
public:
	__InfoPlayerOther	m_InfoExt;					// 캐릭터 정보 확장..
	bool				m_bSit;

public:
	void	InitFace();
	void	InitHair();
	void	KnightsInfoSet(int iID, const std::string& szName, int iGrade, int iRank);
	void	SetSoundAndInitFont(uint32_t dwFontFlag = 0UL);

	bool	Init(enum e_Race eRace, int iFace, int iHair, uint32_t* pdwItemIDs, int* piItenDurabilities);
	void	Tick();

	CPlayerOther();
	virtual ~CPlayerOther();
};


#endif // !defined(AFX_PlayerOther_H__06F94EA0_0640_4532_B8CC_7FB9A99291AB__INCLUDED_)
