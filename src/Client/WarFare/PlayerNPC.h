// PlayerNPC.h: interface for the CPlayerNPC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERNPC_H__8A79604A_EFA6_408E_A0EB_4B7BF4FDDAB4__INCLUDED_)
#define AFX_PLAYERNPC_H__8A79604A_EFA6_408E_A0EB_4B7BF4FDDAB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PlayerBase.h"
#include "GameBase.h"

//////////////////////////
// Other Player..
#include "PlayerBase.h"

class CPlayerNPC : public CPlayerBase
{
	friend class CPlayerOtherMgr;
public:
	void	MoveTo(float fPosX, float fPosY, float fPosZ, float fMoveSpeed, int iMoveMode); // 이 위치로 이동..
	void	Tick() override;
	void	SetSoundAndInitFont(uint32_t dwFontFlag = 0UL) override;

	CPlayerNPC();
	virtual ~CPlayerNPC();
};

#endif // !defined(AFX_PLAYERNPC_H__8A79604A_EFA6_408E_A0EB_4B7BF4FDDAB4__INCLUDED_)
