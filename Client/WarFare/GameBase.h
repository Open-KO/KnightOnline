#pragma once

#include <N3Base/N3Base.h>
#include <N3Base/N3TableBase.h>

#include "GameDef.h"

class CGameBase : public CN3Base
{
#define ACT_WORLD s_pWorldMgr->GetActiveWorld()

public:
	static CN3TableBase<__TABLE_TEXTS>				s_pTbl_Texts;
	static CN3TableBase<__TABLE_ZONE>				s_pTbl_Zones;			// Zone 정보에 관한 테이블..
	static CN3TableBase<__TABLE_UI_RESRC>			s_pTbl_UI;				// UI FileName Table
	static CN3TableBase<__TABLE_ITEM_BASIC>			s_pTbl_Items_Basic;		// 각 유저의(내 자신과 주위 다른 사람) 클레스별 장착 아이템 리소스 테이블
	static CN3TableBase<__TABLE_ITEM_EXT>			s_pTbl_Items_Exts[MAX_ITEM_EXTENSION];		// 각 유저의(내 자신과 주위 다른 사람) 클레스별 장착 아이템 리소스 테이블
	static CN3TableBase<__TABLE_PLAYER_LOOKS>		s_pTbl_UPC_Looks;		// 각 유저의(내 자신과 주위 다른 사람) 클레스별 기본 스킨
	static CN3TableBase<__TABLE_PLAYER_LOOKS>		s_pTbl_NPC_Looks;		// NPC, Mob 기본 모습 - 6개의 캐릭터 파트(), 2개의 플러그
	static CN3TableBase<__TABLE_UPC_SKILL>			s_pTbl_Skill;			// Skill 정보에 관한 Table
	static CN3TableBase<__TABLE_EXCHANGE_QUEST>		s_pTbl_Exchange_Quest;	// 교환 퀘스트에 관한 테이블..
	static CN3TableBase<__TABLE_FX>					s_pTbl_FXSource;		// FX소스 정보 테이블..
	static CN3TableBase<__TABLE_QUEST_MENU>			s_pTbl_QuestMenu;		// 퀘스트 선택 메뉴
	static CN3TableBase<__TABLE_QUEST_TALK>			s_pTbl_QuestTalk;		// 퀘스트 지문

	static class CN3WorldManager*		s_pWorldMgr;								// 월드 매니져 클래스..
	static class CPlayerOtherMgr*		s_pOPMgr;									// Other Player Manager - 다른 유저 관리 클래스..
	static class CPlayerMySelf*			s_pPlayer;			// 유저 클래스..
	
protected:
	static void StaticMemberInit(); // 테이블 및 기본 지형, 오브젝트, 캐리터등 초기화..
	static void StaticMemberRelease(); // 테이블 및 기본 지형, 오브젝트, 캐리터등 초기화..

public:
	static bool GetText(uint32_t nResourceID, std::string* szText);
	static bool GetTextF(uint32_t nResourceID, std::string* szText, ...);
	static bool	GetTextByAttrib(e_ItemAttrib eAttrib, std::string& szAttrib);
	static bool GetTextByClass(e_Class eClass, std::string& szText);
	static bool GetTextByItemClass(e_ItemClass eItemClass, std::string& szText);
	static bool GetTextByKnightsDuty(e_KnightsDuty eDuty, std::string& szText);
	static bool GetTextByNation(e_Nation eNation, std::string& szText);
	static bool GetTextByRace(e_Race eRace, std::string& szText);
	static D3DCOLOR				GetIDColorByLevelDifference(int iLevelDiff); // 레벨 차이에 따른 ID 색 돌려준다.
	static e_Class_Represent	GetRepresentClass(e_Class eClass); // 세부직업을 넣어주면 대표되는 직업을 돌려준다.
	static e_ItemType MakeResrcFileNameForUPC(	__TABLE_ITEM_BASIC* pItem,		// 아이템 데이터...
												std::string* szResrcFN,			// Resource FileName
												std::string* szIconFN,			// Icon FileName
												e_PartPosition& ePartPosition,	// Part 일경우 Index
												e_PlugPosition& ePlugPosition,  // Plug 일경우 Index
												e_Race eRace = RACE_UNKNOWN);

	class CPlayerBase*	CharacterGetByID(int iID, bool bFromAlive);
	bool				IsValidCharacter(CPlayerBase* pCharacter);
	static std::string FormatNumber(int iNumber);

	CGameBase();
	virtual ~CGameBase();
};
