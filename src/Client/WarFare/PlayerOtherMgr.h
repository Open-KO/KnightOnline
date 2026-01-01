// PlayerOtherMgr.h: interface for the CPlayerOtherMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PlayerOtherMgr_H__B32C59B8_6C08_494E_B9DE_338B3CD026C5__INCLUDED_)
#define AFX_PlayerOtherMgr_H__B32C59B8_6C08_494E_B9DE_338B3CD026C5__INCLUDED_

#include <map>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PlayerOther.h"
#include "GameBase.h"
#include "GameProcedure.h"

//typedef std::list<CPlayerOther*>::iterator it_UPC;
//typedef std::list<CPlayerNPC*>::iterator it_NPC;
//typedef std::list<CPlayerBase*>::iterator it_BPC;

typedef std::map<int, CPlayerOther*>::iterator it_UPC;
typedef std::map<int, CPlayerNPC*>::iterator it_NPC;
typedef std::map<int, CPlayerBase*>::iterator it_BPC;

typedef std::map<int, CPlayerOther*>::value_type val_UPC;
typedef std::map<int, CPlayerNPC*>::value_type val_NPC;
typedef std::map<int, CPlayerBase*>::value_type val_BPC;

class CPlayerOtherMgr : public CGameBase
{
public:
//	std::list<CPlayerNPC*>		m_NPCs;		// NPC
//	std::list<CPlayerOther*>	m_UPCs;		// User Player Character
//	std::list<CPlayerNPC*>		m_Corpses;	// 죽은놈.. 죽는 에니메이션 및 시간이 지나면 없어지게 한다..
	std::map<int, CPlayerNPC*>		m_NPCs;		// NPC
	std::map<int, CPlayerOther*>	m_UPCs;		// User Player Character
	std::map<int, CPlayerNPC*>		m_Corpses;	// 죽은놈.. 죽는 에니메이션 및 시간이 지나면 없어지게 한다..
	int								m_iChrCountToRender; // 렌더링되는 캐릭 카운트

public:
	bool				IsValidCharacter(CPlayerBase* pCharacter);
	void				Tick(const __Vector3& vPosPlayer);
	void				Render(float fSunAngle);

	void				NPCAdd(CPlayerNPC* pNPC);
	bool				NPCDelete(int iID);								// 고유 ID 와 일치하는 NPC를 리스트에서 제거.. 및 리소스 해제 
	void				UPCAdd(CPlayerOther* pPlayer);
	bool				UPCDelete(int iID);								// 고유 ID 와 일치하는 NPC를 리스트에서 제거.. 및 리소스 해제 

//	CPlayerOther*		UPCGetByName(const char* szID);					// User Player Character 와 NPC 를 조사해서 포인터를 가져온다.
	CPlayerOther*		UPCGetByID(int iID, bool bFromAliveOnly);		// User Player Character 와 NPC 를 조사해서 포인터를 가져온다.
//	CPlayerNPC*			NPCGetByName(const char* szID);					// User Player Character 와 NPC 를 조사해서 포인터를 가져온다.
	CPlayerNPC*			NPCGetByID(int iID, bool bFromAliveOnly);		// User Player Character 와 NPC 를 조사해서 포인터를 가져온다.
	CPlayerNPC*			NPCGetByPos(const __Vector3& vPos);
	CPlayerNPC*			CharacterGetByID(int iID, bool bFromAliveOnly); // User, NPC 안 가리고 가져온다..
	CPlayerNPC*			CharacterGetByNearestEnemy(const __Vector3& vPosPlayer); // 가장 가까운 적 가져오기..
	CPlayerNPC*			CharacterGetByNearestNPC(const __Vector3& vPosPlayer);
	bool				CharacterDelete(int iID); // User, NPC 안 가리고 지운다..

	CPlayerBase*		CorpseGetByID(int iID);							// 시체들에서 Player Character 와 NPC 를 조사해서 포인터를 가져온다.
	void				CorpseRemove(CPlayerNPC* pCorpse, bool bRemoveImmediately = false);
	void				CorpseAdd(CPlayerNPC* pNPC);
	void				CorpseAdd(int iID);
	CPlayerNPC*			CorpseGetNearstNPC(bool bMustHaveItem, e_Nation eNation, const __Vector3& vPosPlayer); // 가장 가까운 적 시체 가져오기..
	void				MoveToCorpsesForcely(CPlayerNPC* pNPC, bool bErase);			// 아이디가 겹치거나 하면 강제로 시체를 만든다..


	//.. Picking된 PlayerOther 계산..
	CPlayerNPC*			Pick(int ixScreen, int iyScreen, int& iIDResult, __Vector3* pvPick = nullptr);
	CPlayerNPC*			PickNPC(int ixScreen, int iyScreen, int& iIDResult, __Vector3* pvPick = nullptr);
	CPlayerOther*		PickUPC(int ixScreen, int iyScreen, int& iIDResult, __Vector3* pvPick = nullptr);
	CPlayerNPC*			PickPrecisely(int ixScreen, int iyScreen, int& iIDResult, __Vector3* pvPick = nullptr);
	CPlayerNPC*			PickNPCPrecisely(int ixScreen, int iyScreen, int& iIDResult, __Vector3* pvPick = nullptr);
	CPlayerOther*		PickUPCPrecisely(int ixScreen, int iyScreen, int& iIDResult, __Vector3* pvPick = nullptr);
	CPlayerNPC*			PickAllPrecisely(int ixScreen, int iyScreen, int &iIDResult, __Vector3* pvPick);
	CPlayerNPC*			PickCorpse(int ixScreen, int iyScreen, int& iIDResult); // 시체중 클릭..
	static int			SortByCameraDistance(const void* pArg1, const void* pArg2);

	void				ReleaseUPCs();
	void				ReleaseNPCs();
	void				ReleaseCorpses();
	void				Release();

	CPlayerOtherMgr();
	virtual ~CPlayerOtherMgr();
};

inline CPlayerOther* CPlayerOtherMgr::UPCGetByID(int iID, bool bFromAliveOnly)
{
	it_UPC it = m_UPCs.find(iID);
	if(it != m_UPCs.end())
	{
		CPlayerOther* pUPC = it->second;
		if(bFromAliveOnly)
		{
			if(PSA_DEATH != pUPC->m_eState) return pUPC;
			else return nullptr;
		}
		else return pUPC;
	}
	else return nullptr;
}

inline CPlayerNPC* CPlayerOtherMgr::NPCGetByID(int iID, bool bFromAliveOnly)
{
	it_NPC it = m_NPCs.find(iID);
	if(it != m_NPCs.end())
	{
		CPlayerNPC* pNPC = it->second;
		if(bFromAliveOnly)
		{
			if(PSA_DEATH != pNPC->m_eState) return pNPC;
			else return nullptr;
		}
		else return pNPC;
	}
	else return nullptr;
}

inline CPlayerBase* CPlayerOtherMgr::CorpseGetByID(int iID)
{
	it_NPC it = m_Corpses.find(iID);
	if(it != m_Corpses.end()) return it->second;
	else return nullptr;
}

inline CPlayerNPC* CPlayerOtherMgr::CharacterGetByID(int iID, bool bFromAliveOnly)
{
	if(iID < 0) return 0;

	CPlayerNPC* pBPC = this->NPCGetByID(iID, bFromAliveOnly);		// 먼저  NPC 중에서 찾아보고..
	if(nullptr == pBPC) pBPC = this->UPCGetByID(iID, bFromAliveOnly);			// 없음 User 들도 찾아본다..

	return pBPC;
}

inline void CPlayerOtherMgr::UPCAdd(CPlayerOther* pUPC)
{
	it_UPC it = m_UPCs.find(pUPC->IDNumber());
	if(it == m_UPCs.end()) // 중복된게 없으면..
	{
		m_UPCs.insert(val_UPC(pUPC->IDNumber(), pUPC));
	}
	else // 중복되었으면..
	{
		delete it->second;
		it->second = pUPC;
	}
}

inline bool CPlayerOtherMgr::UPCDelete(int iID)
{
	it_UPC it = m_UPCs.find(iID);
	if(it == m_UPCs.end()) return false;

	delete it->second;
	m_UPCs.erase(it);
	return true;
}

inline void CPlayerOtherMgr::NPCAdd(CPlayerNPC* pNPC)
{
	it_NPC it = m_NPCs.find(pNPC->IDNumber());
	if(it == m_NPCs.end()) // 중복된게 없으면..
	{
		m_NPCs.insert(val_NPC(pNPC->IDNumber(), pNPC));
	}
	else // 중복되었으면..
	{
		delete it->second;
		it->second = pNPC;
	}
}

inline bool CPlayerOtherMgr::NPCDelete(int iID)
{
	it_NPC it = m_NPCs.find(iID);
	if(it == m_NPCs.end()) return false;

	delete it->second;
	m_NPCs.erase(it);
	return true;
}

#endif // !defined(AFX_PlayerOtherMgr_H__B32C59B8_6C08_494E_B9DE_338B3CD026C5__INCLUDED_)
