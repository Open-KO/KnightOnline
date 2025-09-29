// User.h: interface for the CUser class.
// 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USER_H__5FEC1968_ED75_4AAF_A4DB_CB48F6940B2E__INCLUDED_)
#define AFX_USER_H__5FEC1968_ED75_4AAF_A4DB_CB48F6940B2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOCPSocket2.h"
#include "define.h"
#include "GameDefine.h"
#include "MagicProcess.h"
#include "Npc.h"
#include "EVENT.h"
#include "EVENT_DATA.h"
#include "LOGIC_ELSE.h"
#include "EXEC.h"     

#include <list>
typedef	 std::list<_EXCHANGE_ITEM*>		ItemList;
typedef  std::list<int>				UserEventList;	// 이밴트를 위하여 ^^;

#define BANISH_DELAY_TIME    30

class CEbenezerDlg;
class CUser : public CIOCPSocket2
{
public:
	_USER_DATA* m_pUserData;

	char	m_strAccountID[MAX_ID_SIZE + 1];	// Login -> Select Char 까지 한시적으로만 쓰는변수. 이외에는 _USER_DATA 안에있는 변수를 쓴다...agent 와의 데이터 동기화를 위해...

	int16_t	m_RegionX;						// 현재 영역 X 좌표
	int16_t	m_RegionZ;						// 현재 영역 Z 좌표

	int		m_iMaxExp;						// 다음 레벨이 되기 위해 필요한 Exp량
	int		m_iMaxWeight;					// 들 수 있는 최대 무게
	uint8_t	m_sSpeed;						// 스피드

	int16_t	m_sBodyAc;						// 맨몸 방어력

	int16_t	m_sTotalHit;					// 총 타격공격력	
	int16_t	m_sTotalAc;						// 총 방어력
	float	m_fTotalHitRate;				// 총 공격성공 민첩성
	float	m_fTotalEvasionRate;			// 총 방어 민첩성

	int16_t   m_sItemMaxHp;                   // 아이템 총 최대 HP Bonus
	int16_t   m_sItemMaxMp;                   // 아이템 총 최대 MP Bonus
	int		m_iItemWeight;					// 아이템 총무게
	int16_t	m_sItemHit;						// 아이템 총타격치
	int16_t	m_sItemAc;						// 아이템 총방어력
	int16_t	m_sItemStr;						// 아이템 총힘 보너스
	int16_t	m_sItemSta;						// 아이템 총체력 보너스
	int16_t	m_sItemDex;						// 아이템 총민첩성 보너스
	int16_t	m_sItemIntel;					// 아이템 총지능 보너스
	int16_t	m_sItemCham;					// 아이템 총매력보너스
	int16_t	m_sItemHitrate;					// 아이템 총타격율
	int16_t	m_sItemEvasionrate;				// 아이템 총회피율

	uint8_t	m_bFireR;						// 불 마법 저항력
	uint8_t	m_bColdR;						// 얼음 마법 저항력
	uint8_t	m_bLightningR;					// 전기 마법 저항력
	uint8_t	m_bMagicR;						// 기타 마법 저항력
	uint8_t	m_bDiseaseR;					// 저주 마법 저항력
	uint8_t	m_bPoisonR;						// 독 마법 저항력

	uint8_t	m_bMagicTypeLeftHand;			// The type of magic item in user's left hand  
	uint8_t	m_bMagicTypeRightHand;			// The type of magic item in user's right hand
	int16_t	m_sMagicAmountLeftHand;         // The amount of magic item in user's left hand
	int16_t	m_sMagicAmountRightHand;        // The amount of magic item in user's left hand

	int16_t	m_sDaggerR;						// Resistance to Dagger
	int16_t	m_sSwordR;						// Resistance to Sword
	int16_t	m_sAxeR;						// Resistance to Axe
	int16_t	m_sMaceR;						// Resistance to Mace
	int16_t	m_sSpearR;						// Resistance to Spear
	int16_t	m_sBowR;						// Resistance to Bow		

	int16_t	m_iMaxHp;
	int16_t	m_iMaxMp;

	int16_t	m_iZoneIndex;

	float	m_fWill_x;
	float	m_fWill_z;
	float	m_fWill_y;

	uint8_t	m_bResHpType;					// HP 회복타입
	uint8_t	m_bWarp;						// 존이동중...
	uint8_t	m_bNeedParty;					// 파티....구해요

	int16_t	m_sPartyIndex;
	int16_t	m_sExchangeUser;				// 교환중인 유저
	uint8_t	m_bExchangeOK;

	ItemList	m_ExchangeItemList;
	_ITEM_DATA	m_MirrorItem[HAVE_MAX];			// 교환시 백업 아이템 리스트를 쓴다.

	int16_t	m_sPrivateChatUser;

	float	m_fHPLastTimeNormal;					// For Automatic HP recovery. 
	float	m_fHPStartTimeNormal;
	int16_t	m_bHPAmountNormal;
	uint8_t	m_bHPDurationNormal;
	uint8_t	m_bHPIntervalNormal;

	float	m_fHPLastTime[MAX_TYPE3_REPEAT];		// For Automatic HP recovery and Type 3 durational HP recovery.
	float	m_fHPStartTime[MAX_TYPE3_REPEAT];
	int16_t	m_bHPAmount[MAX_TYPE3_REPEAT];
	uint8_t	m_bHPDuration[MAX_TYPE3_REPEAT];
	uint8_t	m_bHPInterval[MAX_TYPE3_REPEAT];
	int16_t	m_sSourceID[MAX_TYPE3_REPEAT];
	bool	m_bType3Flag;

	float	m_fAreaLastTime;			// For Area Damage spells Type 3.
	float	m_fAreaStartTime;
	uint8_t	m_bAreaInterval;
	int		m_iAreaMagicID;

	uint8_t	m_bAttackSpeedAmount;		// For Character stats in Type 4 Durational Spells.
	uint8_t	m_bSpeedAmount;
	int16_t	m_sACAmount;
	uint8_t	m_bAttackAmount;
	int16_t	m_sMaxHPAmount;
	uint8_t	m_bHitRateAmount;
	int16_t	m_sAvoidRateAmount;
	int16_t	m_sStrAmount;
	int16_t	m_sStaAmount;
	int16_t	m_sDexAmount;
	int16_t	m_sIntelAmount;
	int16_t	m_sChaAmount;
	uint8_t	m_bFireRAmount;
	uint8_t	m_bColdRAmount;
	uint8_t	m_bLightningRAmount;
	uint8_t	m_bMagicRAmount;
	uint8_t	m_bDiseaseRAmount;
	uint8_t	m_bPoisonRAmount;

	int16_t	m_sDuration1;  float   m_fStartTime1;
	int16_t	m_sDuration2;  float   m_fStartTime2;
	int16_t	m_sDuration3;  float   m_fStartTime3;
	int16_t	m_sDuration4;  float   m_fStartTime4;
	int16_t	m_sDuration5;  float   m_fStartTime5;
	int16_t	m_sDuration6;  float   m_fStartTime6;
	int16_t	m_sDuration7;  float   m_fStartTime7;
	int16_t	m_sDuration8;  float   m_fStartTime8;
	int16_t	m_sDuration9;  float   m_fStartTime9;

	uint8_t	m_bType4Buff[MAX_TYPE4_BUFF];
	bool	m_bType4Flag;

	CEbenezerDlg* m_pMain;
	CMagicProcess m_MagicProcess;

	float	m_fSpeedHackClientTime, m_fSpeedHackServerTime;
	uint8_t	m_bSpeedHackCheck;

	int16_t	m_sFriendUser;				// who are you trying to make friends with?

	float	m_fBlinkStartTime;			// When did you start to blink?

	int16_t	m_sAliveCount;

	uint8_t	m_bAbnormalType;			// Is the player normal,a giant, or a dwarf?

	int16_t	m_sWhoKilledMe;				// Who killed me???
	int		m_iLostExp;					// Experience point that was lost when you died.

	float	m_fLastTrapAreaTime;		// The last moment you were in the trap area.

	bool	m_bZoneChangeFlag;			// 성용씨 미워!!

	uint8_t	m_bRegeneType;				// Did you die and go home or did you type '/town'?

	float	m_fLastRegeneTime;			// The last moment you got resurrected.

	bool	m_bZoneChangeSameZone;		// Did the server change when you warped?

	// 이밴트용 관련.... 정애씨 이거 보면 코카스 쏠께요 ^^;
//	int					m_iSelMsgEvent[5];	// 실행중인 선택 메세지박스 이벤트
	int					m_iSelMsgEvent[MAX_MESSAGE_EVENT];	// 실행중인 선택 메세지박스 이벤트
	int16_t				m_sEventNid;		// 마지막으로 선택한 이벤트 NPC 번호
	UserEventList		m_arUserEvent;		// 실행한 이벤트 리스트

	char				m_strCouponId[MAX_COUPON_ID_LENGTH];		// What was the number of the coupon?
	int					m_iEditBoxEvent;

	int16_t				m_sEvent[MAX_CURRENT_EVENT];				// 이미 실행된 이밴트 리스트들 :)

	bool				m_bIsPartyLeader;
	uint8_t				m_byInvisibilityState;
	int16_t				m_sDirection;
	bool				m_bIsChicken;
	uint8_t				m_byKnightsRank;
	uint8_t				m_byPersonalRank;

	// Selected exchange slot for last exchange (1~5).
	// Applicable only to exchange type 101.
	uint8_t				m_byLastExchangeNum;

public:
	bool CheckMiddleStatueCapture() const;
	void SetZoneAbilityChange(int zone);
	int16_t GetMaxWeightForClient() const;
	int16_t GetCurrentWeightForClient() const;
	void RecvZoneChange(char* pBuf);
	void GameStart(char* pBuf);
	void GetUserInfo(char* buff, int& buff_index);
	void RecvDeleteChar(char* pBuf);
	bool ExistComEvent(int eventid) const;
	void SaveComEvent(int eventid);
	bool CheckItemCount(int itemid, int16_t min, int16_t max) const;
	bool CheckClanGrade(int min, int max) const;
	bool CheckKnight() const;
	void CouponEvent(char* pBuf);
	void LogCoupon(int itemid, int count);
	void RecvEditBox(char* pBuf);
	bool CheckCouponUsed() const;
	bool CheckRandom(int16_t percent) const;
	void OpenEditBox(int message, int event);
	bool CheckEditBox() const;
	void NativeZoneReturn();
	void EventMoneyItemGet(int itemid, int count);
	void KickOutZoneUser(bool home = false);
	void TrapProcess();
	bool JobGroupCheck(int16_t jobgroupid) const;
	void SelectMsg(const EXEC* pExec);
	void SendNpcSay(const EXEC* pExec);
	void SendSay(int16_t eventIdUp, int16_t eventIdOk, int16_t message1, int16_t message2 = -1, int16_t message3 = -1, int16_t message4 = -1, int16_t message5 = -1, int16_t message6 = -1, int16_t message7 = -1, int16_t message8 = -1);
	bool CheckClass(int16_t class1, int16_t class2 = -1, int16_t class3 = -1, int16_t class4 = -1, int16_t class5 = -1, int16_t class6 = -1) const;
	bool CheckPromotionEligible();
	void RecvSelectMsg(char* pBuf);
	bool GiveItem(int itemid, int16_t count);
	bool RobItem(int itemid, int16_t count);
	bool CheckExistItem(int itemid, int16_t count) const;
	bool CheckWeight(int itemid, int16_t count) const;
	bool CheckSkillPoint(uint8_t skillnum, uint8_t min, uint8_t max) const;
	bool CheckExistEvent(int16_t questId, uint8_t questState) const;
	bool GoldLose(int gold);
	void GoldGain(int gold);
	void SendItemWeight();
	void ItemLogToAgent(const char* srcid, const char* tarid, int type, int64_t serial, int itemid, int count, int durability);
	void TestPacket(char* pBuf);
	bool RunEvent(const EVENT_DATA* pEventData);
	bool RunNpcEvent(CNpc* pNpc, const EXEC* pExec);
	bool CheckEventLogic(const EVENT_DATA* pEventData);
	void ClientEvent(char* pBuf);
	void KickOut(char* pBuf);
	void SetLogInInfoToDB(uint8_t bInit);
	void BlinkTimeCheck(float currenttime);
	void MarketBBSSellPostFilter();
	void MarketBBSBuyPostFilter();
	void MarketBBSMessage(char* pBuf);
	void MarketBBSSellDelete(int16_t index);
	void MarketBBSBuyDelete(int16_t index);
	void MarketBBSUserDelete();
	void MarketBBSTimeCheck();
	void MarketBBSRemotePurchase(char* pBuf);
	void MarketBBSReport(char* pBuf, uint8_t type);
	void MarketBBSDelete(char* pBuf);
	void MarketBBSRegister(char* pBuf);
	void MarketBBS(char* pBuf);
	void PartyBBSNeeded(char* pBuf, uint8_t type);
	void PartyBBSDelete(char* pBuf);
	void PartyBBSRegister(char* pBuf);
	void PartyBBS(char* pBuf);
	void Corpse();
	void FriendAccept(char* pBuf);
	void FriendRequest(char* pBuf);
	void Friend(char* pBuf);
	bool WarpListObjectEvent(int16_t objectindex, int16_t nid);
	bool FlagObjectEvent(int16_t objectindex, int16_t nid);
	bool GateLeverObjectEvent(int16_t objectindex, int16_t nid);
	bool GateObjectEvent(int16_t objectindex, int16_t nid);
	bool BindObjectEvent(int16_t objectindex, int16_t nid);
	void InitType3();
	bool GetWarpList(int warp_group);
	void ServerChangeOk(char* pBuf);
	void ZoneConCurrentUsers(char* pBuf);
	void SelectWarpList(char* pBuf);
	void GoldChange(int16_t tid, int gold);
	void AllSkillPointChange();
	void AllPointChange();
	void ClassChangeReq();
	void FriendReport(char* pBuf);
	CUser* GetItemRoutingUser(int itemid, int16_t itemcount);
	bool GetStartPosition(int16_t* x, int16_t* z);
	void Home();
	void ReportBug(char* pBuf);
	int GetEmptySlot(int itemid, int bCountable) const;
	int GetNumberOfEmptySlots() const;
	void InitType4();
	void WarehouseProcess(char* pBuf);
	int16_t GetACDamage(int damage, int16_t tid);
	int16_t GetMagicDamage(int damage, int16_t tid);
	void Type3AreaDuration(float currenttime);
	void ServerStatusCheck();
	void SpeedHackTime(char* pBuf);
	void OperatorCommand(char* pBuf);
	void ItemRemove(char* pBuf);
	void SendAllKnightsID();
	uint8_t ItemCountChange(int itemid, int type, int amount);
	void Type4Duration(float currenttime);
	void ItemRepair(char* pBuf);
	int ExchangeDone();
	void HPTimeChange(float currenttime);
	void HPTimeChangeType3(float currenttime);
	void ItemDurationChange(int slot, int maxvalue, int curvalue, int amount);
	void ItemWoreOut(int type, int damage);
	void Dead();
	void LoyaltyDivide(int16_t tid);
	void UserDataSaveToAgent();
	void CountConcurrentUser();
	void SendUserInfo(char* temp_send, int& index);
	void ChatTargetSelect(char* pBuf);
	bool ItemEquipAvailable(const model::Item* pTable) const;
	void ClassChange(char* pBuf);
	void MSpChange(int amount);
	void UpdateGameWeather(char* pBuf, uint8_t type);
	void ObjectEvent(char* pBuf);
	void SkillPointChange(char* pBuf);
	bool ExecuteExchange();
	void InitExchange(bool bStart);
	void ExchangeCancel();
	void ExchangeDecide();
	void ExchangeAdd(char* pBuf);
	void ExchangeAgree(char* pBuf);
	void ExchangeReq(char* pBuf);
	void ExchangeProcess(char* pBuf);
	void PartyDelete();
	void PartyRemove(int memberid);
	void PartyInsert();
	void PartyCancel();
	void PartyRequest(int memberid, bool bCreate);
	void PartyProcess(char* pBuf);
	void SendNotice();
	void UserLookChange(int pos, int itemid, int durability);
	void SpeedHackUser();
	void VersionCheck();
	void LoyaltyChange(int16_t tid);
	void StateChange(char* pBuf);
	void PointChange(char* pBuf);
	void ZoneChange(int zone, float x, float z);
	void ItemGet(char* pBuf);
	static bool IsValidName(const char* name);
	void AllCharInfoToAgent();
	void SelNationToAgent(char* pBuf);
	void DelCharToAgent(char* pBuf);
	void NewCharToAgent(char* pBuf);
	void BundleOpenReq(char* pBuf);
	void SendTargetHP(uint8_t echo, int tid, int damage = 0);
	void ItemTrade(char* pBuf);
	void NpcEvent(char* pBuf);
	bool IsValidSlotPos(model::Item* pTable, int destpos) const;
	void ItemMove(char* pBuf);
	void Warp(char* pBuf);
	void RequestNpcIn(char* pBuf);
	void SetUserAbility();
	void LevelChange(int16_t level, bool bLevelUp = true);
	void HpChange(int amount, int type = 0, bool attack = false);
	int16_t GetDamage(int16_t tid, int magicid);
	void SetSlotItemValue();
	uint8_t GetHitRate(float rate);
	void RequestUserIn(char* pBuf);
	void InsertRegion(int del_x, int del_z);
	void RemoveRegion(int del_x, int del_z);
	void RegisterRegion();
	void SetDetailData();
	void SendTimeStatus();
	void Regene(char* pBuf, int magicid = 0);
	void SetMaxMp();
	void SetMaxHp(int iFlag = 0); // 0:default, 1:hp를 maxhp만큼 채워주기
	void ExpChange(int iExp);
	void Chat(char* pBuf);
	void LogOut();
	void SelCharToAgent(char* pBuf);
	void SendMyInfo(int type);
	void SelectCharacter(char* pBuf);
	void Send2AI_UserUpdateInfo();
	void Attack(char* pBuf);
	void UserInOut(uint8_t Type);
	void Initialize();
	void MoveProcess(char* pBuf);
	void Rotate(char* pBuf);
	void LoginProcess(char* pBuf);
	void Parsing(int len, char* pData);
	void CloseProcess();
	CUser();
	virtual ~CUser();
};

#endif // !defined(AFX_USER_H__5FEC1968_ED75_4AAF_A4DB_CB48F6940B2E__INCLUDED_)
