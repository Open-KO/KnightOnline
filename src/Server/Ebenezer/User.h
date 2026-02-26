#ifndef SERVER_EBENEZER_USER_H
#define SERVER_EBENEZER_USER_H

#pragma once

#include "Define.h"
#include "GameDefine.h"
#include "MagicProcess.h"
#include "Npc.h"
#include "EVENT.h"
#include "EVENT_DATA.h"
#include "LOGIC_ELSE.h"
#include "EXEC.h"

#include <shared/JvCryption.h>
#include <shared-server/TcpServerSocket.h>

#include <list>
#include <span>

namespace Ebenezer
{

using ItemList                         = std::list<_EXCHANGE_ITEM*>;
using UserEventList                    = std::list<int>; // 이밴트를 위하여 ^^;

inline constexpr int BANISH_DELAY_TIME = 30;

class EbenezerApp;
class CUser : public TcpServerSocket
{
public:
	_USER_DATA* m_pUserData              = nullptr;

	// Login -> Select Char 까지 한시적으로만 쓰는변수. 이외에는 _USER_DATA 안에있는 변수를 쓴다...agent 와의 데이터 동기화를 위해...
	char m_strAccountID[MAX_ID_SIZE + 1] = {};

	int16_t m_RegionX                    = 0;    // 현재 영역 X 좌표
	int16_t m_RegionZ                    = 0;    // 현재 영역 Z 좌표

	int m_iMaxExp                        = 0;    // 다음 레벨이 되기 위해 필요한 Exp량
	int m_iMaxWeight                     = 0;    // 들 수 있는 최대 무게
	uint8_t m_sSpeed                     = 0;    // 스피드

	int16_t m_sBodyAc                    = 0;    // 맨몸 방어력

	int16_t m_sTotalHit                  = 0;    // 총 타격공격력
	int16_t m_sTotalAc                   = 0;    // 총 방어력
	float m_fTotalHitRate                = 0.0f; // 총 공격성공 민첩성
	float m_fTotalEvasionRate            = 0.0f; // 총 방어 민첩성

	int16_t m_sItemMaxHp                 = 0;    // 아이템 총 최대 HP Bonus
	int16_t m_sItemMaxMp                 = 0;    // 아이템 총 최대 MP Bonus
	int m_iItemWeight                    = 0;    // 아이템 총무게
	int16_t m_sItemHit                   = 0;    // 아이템 총타격치
	int16_t m_sItemAc                    = 0;    // 아이템 총방어력
	int16_t m_sItemStr                   = 0;    // 아이템 총힘 보너스
	int16_t m_sItemSta                   = 0;    // 아이템 총체력 보너스
	int16_t m_sItemDex                   = 0;    // 아이템 총민첩성 보너스
	int16_t m_sItemIntel                 = 0;    // 아이템 총지능 보너스
	int16_t m_sItemCham                  = 0;    // 아이템 총매력보너스
	int16_t m_sItemHitrate               = 0;    // 아이템 총타격율
	int16_t m_sItemEvasionrate           = 0;    // 아이템 총회피율

	uint8_t m_bFireR                     = 0;    // 불 마법 저항력
	uint8_t m_bColdR                     = 0;    // 얼음 마법 저항력
	uint8_t m_bLightningR                = 0;    // 전기 마법 저항력
	uint8_t m_bMagicR                    = 0;    // 기타 마법 저항력
	uint8_t m_bDiseaseR                  = 0;    // 저주 마법 저항력
	uint8_t m_bPoisonR                   = 0;    // 독 마법 저항력

	uint8_t m_bMagicTypeLeftHand         = 0;    // The type of magic item in user's left hand
	uint8_t m_bMagicTypeRightHand        = 0;    // The type of magic item in user's right hand
	int16_t m_sMagicAmountLeftHand       = 0;    // The amount of magic item in user's left hand
	int16_t m_sMagicAmountRightHand      = 0;    // The amount of magic item in user's left hand

	int16_t m_sDaggerR                   = 0;    // Resistance to Dagger
	int16_t m_sSwordR                    = 0;    // Resistance to Sword
	int16_t m_sAxeR                      = 0;    // Resistance to Axe
	int16_t m_sMaceR                     = 0;    // Resistance to Mace
	int16_t m_sSpearR                    = 0;    // Resistance to Spear
	int16_t m_sBowR                      = 0;    // Resistance to Bow

	int16_t m_iMaxHp                     = 0;
	int16_t m_iMaxMp                     = 0;

	int16_t m_iZoneIndex                 = -1;

	float m_fWill_x                      = 0.0f;
	float m_fWill_z                      = 0.0f;
	float m_fWill_y                      = 0.0f;

	uint8_t m_bResHpType                 = 0;  // HP 회복타입
	uint8_t m_bWarp                      = 0;  // 존이동중...
	uint8_t m_bNeedParty                 = 0;  // 파티....구해요

	int16_t m_sPartyIndex                = -1;
	int16_t m_sExchangeUser              = -1; // 교환중인 유저
	uint8_t m_bExchangeOK                = 0;

	ItemList m_ExchangeItemList;
	_ITEM_DATA m_MirrorItem[HAVE_MAX]       = {}; // 교환시 백업 아이템 리스트를 쓴다.

	int16_t m_sPrivateChatUser              = -1;

	double m_fHPLastTimeNormal              = false; // For Automatic HP recovery.
	int16_t m_bHPAmountNormal               = 0;
	uint8_t m_bHPDurationNormal             = 0;
	uint8_t m_bHPIntervalNormal             = 0;

	// For Automatic HP recovery and Type 3 durational HP recovery.
	double m_fHPLastTime[MAX_TYPE3_REPEAT]  = {};
	double m_fHPStartTime[MAX_TYPE3_REPEAT] = {};
	int16_t m_bHPAmount[MAX_TYPE3_REPEAT]   = {};
	uint8_t m_bHPDuration[MAX_TYPE3_REPEAT] = {};
	uint8_t m_bHPInterval[MAX_TYPE3_REPEAT] = {};
	int16_t m_sSourceID[MAX_TYPE3_REPEAT]   = {};
	bool m_bType3Flag                       = false;

	double m_fAreaLastTime                  = 0.0; // For Area Damage spells Type 3.
	double m_fAreaStartTime                 = 0.0;
	uint8_t m_bAreaInterval                 = 0;
	int m_iAreaMagicID                      = 0;

	uint8_t m_bAttackSpeedAmount            = 0; // For Character stats in Type 4 Durational Spells.
	uint8_t m_bSpeedAmount                  = 0;
	int16_t m_sACAmount                     = 0;
	uint8_t m_bAttackAmount                 = 0;
	int16_t m_sMaxHPAmount                  = 0;
	uint8_t m_bHitRateAmount                = 0;
	int16_t m_sAvoidRateAmount              = 0;
	int16_t m_sStrAmount                    = 0;
	int16_t m_sStaAmount                    = 0;
	int16_t m_sDexAmount                    = 0;
	int16_t m_sIntelAmount                  = 0;
	int16_t m_sChaAmount                    = 0;
	uint8_t m_bFireRAmount                  = 0;
	uint8_t m_bColdRAmount                  = 0;
	uint8_t m_bLightningRAmount             = 0;
	uint8_t m_bMagicRAmount                 = 0;
	uint8_t m_bDiseaseRAmount               = 0;
	uint8_t m_bPoisonRAmount                = 0;

	int16_t m_sDuration1                    = 0;
	int16_t m_sDuration2                    = 0;
	int16_t m_sDuration3                    = 0;
	int16_t m_sDuration4                    = 0;
	int16_t m_sDuration5                    = 0;
	int16_t m_sDuration6                    = 0;
	int16_t m_sDuration7                    = 0;
	int16_t m_sDuration8                    = 0;
	int16_t m_sDuration9                    = 0;
	double m_fStartTime1                    = 0.0;
	double m_fStartTime2                    = 0.0;
	double m_fStartTime3                    = 0.0;
	double m_fStartTime4                    = 0.0;
	double m_fStartTime5                    = 0.0;
	double m_fStartTime6                    = 0.0;
	double m_fStartTime7                    = 0.0;
	double m_fStartTime8                    = 0.0;
	double m_fStartTime9                    = 0.0;

	uint8_t m_bType4Buff[MAX_TYPE4_BUFF]    = {};
	bool m_bType4Flag                       = false;

	EbenezerApp* m_pMain                    = nullptr;
	CMagicProcess m_MagicProcess            = {};

	double m_fSpeedHackClientTime           = 0.0;
	double m_fSpeedHackServerTime           = 0.0;
	uint8_t m_bSpeedHackCheck               = 0;

	int16_t m_sFriendUser                   = -1;  // who are you trying to make friends with?

	double m_fBlinkStartTime                = 0.0; // When did you start to blink?

	int16_t m_sAliveCount                   = 0;

	uint8_t m_bAbnormalType    = ABNORMAL_NORMAL; // Is the player normal, a giant, or a dwarf?

	int16_t m_sWhoKilledMe     = -1;              // Who killed me???
	int m_iLostExp             = 0;               // Experience point that was lost when you died.

	double m_fLastTrapAreaTime = 0.0;             // The last moment you were in the trap area.

	bool m_bZoneChangeFlag     = false;           // 성용씨 미워!!

	uint8_t m_bRegeneType      = 0;     // Did you die and go home or did you type '/town'?

	double m_fLastRegeneTime   = 0.0;   // The last moment you got resurrected.

	bool m_bZoneChangeSameZone = false; // Did the server change when you warped?

	// 이밴트용 관련.... 정애씨 이거 보면 코카스 쏠께요 ^^;
	// 실행중인 선택 메세지박스 이벤트
	int m_iSelMsgEvent[MAX_MESSAGE_EVENT] = {};
	UserEventList m_arUserEvent;                   // 실행한 이벤트 리스트
	int16_t m_sEventNid                      = -1; // 마지막으로 선택한 이벤트 NPC 번호

	char m_strCouponId[MAX_COUPON_ID_LENGTH] = {};
	int m_iEditBoxEvent                      = -1;

	// 이미 실행된 이밴트 리스트들 :)
	int16_t m_sEvent[MAX_CURRENT_EVENT]      = {};

	bool m_bIsPartyLeader                    = false;
	uint8_t m_byInvisibilityState            = 0;
	int16_t m_sDirection                     = 0;
	bool m_bIsChicken                        = false;
	uint8_t m_byKnightsRank                  = 0;
	uint8_t m_byPersonalRank                 = 0;

	// Selected exchange slot for last exchange (1~5).
	// Applicable only to exchange type 101.
	uint8_t m_byLastExchangeNum              = 0;

	// Game socket specific:
	_REGION_BUFFER* _regionBuffer            = nullptr;

	CJvCryption _jvCryption                  = {};
	bool _jvCryptionEnabled                  = false;

	uint32_t _sendValue                      = 0;
	uint32_t _recvValue                      = 0;

	int16_t m_sEventDiceRoll                 = 0;

	CUser(test_tag);
	CUser(TcpServerSocketManager* socketManager);
	~CUser() override;

	std::string_view GetImplName() const override;
	void Initialize() override;
	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;
	void SendCompressingPacket(const char* pData, int len);
	void RegionPacketAdd(char* pBuf, int len);
	int RegionPacketClear(char* GetBuf);
	void CloseProcess() override;
	void Parsing(int len, char* pData) override;

	float GetDistanceSquared2D(float targetX, float targetZ) const;
	float GetDistance2D(float targetX, float targetZ) const;
	bool CheckMiddleStatueCapture() const;
	void SetZoneAbilityChange(int zone);
	int16_t GetMaxWeightForClient() const;
	int16_t GetCurrentWeightForClient() const;
	void RecvZoneChange(char* pBuf);
	void GameStart(char* pBuf);
	void GetUserInfo(char* buff, int& buff_index);
	void RecvDeleteChar(const char* pBuf);
	bool ExistComEvent(int eventid) const;
	void SaveComEvent(int eventid);
	bool CheckItemCount(int itemid, int16_t min, int16_t max) const;
	bool CheckClanGrade(int min, int max) const;
	bool CheckKnight() const;
	void CouponEvent(const char* pBuf);
	void LogCoupon(int itemid, int count);
	void RecvEditBox(char* pBuf);
	void ResetEditBox();
	void ItemUpgradeProcess(char* pBuf);
	void ItemUpgrade(char* pBuf);
	void ItemUpgradeAccesories(char* pBuf);
	void SendItemUpgradeFailed(e_ItemUpgradeResult resultCode);
	bool MatchingItemUpgrade(uint8_t inventoryPosition, int itemRequested, int itemExpected) const;
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
	void SendSay(int16_t eventIdUp, int16_t eventIdOk, int16_t message1, int16_t message2 = -1,
		int16_t message3 = -1, int16_t message4 = -1, int16_t message5 = -1, int16_t message6 = -1,
		int16_t message7 = -1, int16_t message8 = -1);
	bool CheckClass(int16_t class1, int16_t class2 = -1, int16_t class3 = -1, int16_t class4 = -1,
		int16_t class5 = -1, int16_t class6 = -1) const;
	bool CheckPromotionEligible();
	void RecvSelectMsg(char* pBuf);
	void ResetSelectMsg();
	bool GiveItem(int itemid, int16_t count);

	/// \brief Attempts to count number of itemId from the user
	/// \return true when all items were successfully taken, false otherwise
	bool RobItem(int itemId, int16_t count);

	/// \brief Attempts to remove all the items from the user in the input array
	/// \return true when all items were successfully taken, false otherwise
	bool CheckAndRobItems(std::span<const ItemPair> items, int gold = 0);

	/// \brief Checks to see if a user has count number of itemIds
	/// \return true when the user has count number of itemIds, false otherwise
	bool CheckExistItem(int itemId, int16_t count) const;

	/// \brief Checks to see if a user has up to 5 sets of items
	/// \return true when the user has all the items, false otherwise
	bool CheckExistItemAnd(int id1, int16_t count1, int id2, int16_t count2, int id3 = -1,
		int16_t count3 = -1, int id4 = -1, int16_t count4 = -1, int id5 = -1,
		int16_t count5 = -1) const;

	/// \brief Checks to see if a user has all the items in the input array
	/// \return true when the user has all the items, false otherwise
	bool CheckExistItemAnd(std::span<const ItemPair> items) const;

	bool CheckWeight(int itemid, int16_t count) const;
	bool CheckSkillPoint(uint8_t skillnum, uint8_t min, uint8_t max) const;
	bool CheckSkillTotal(uint8_t min, uint8_t max) const;
	bool CheckStatTotal(uint8_t min, uint8_t max) const;
	bool CheckExistEvent(e_QuestId questId, e_QuestState questState) const;
	bool GoldLose(int amount);
	void GoldGain(int amount);
	void SendItemWeight();
	void ItemLogToAgent(const char* srcid, const char* tarid, int type, int64_t serial, int itemid,
		int count, int durability);
	void TestPacket();
	bool RunEvent(const EVENT_DATA* pEventData);
	bool CheckEventLogic(const EVENT_DATA* pEventData);
	void ClientEvent(char* pBuf);
	void KickOut(char* pBuf);
	void SetLogInInfoToDB(uint8_t bInit);
	void BlinkTimeCheck(double currentTime);
	void MarketBBSSellPostFilter();
	void MarketBBSBuyPostFilter();
	void MarketBBSMessage(char* pBuf);
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
	bool WarpListObjectEvent(int16_t objectIndex, int16_t npcId);
	bool FlagObjectEvent(int16_t objectIndex, int16_t npcId);
	bool GateLeverObjectEvent(int16_t objectIndex, int16_t npcId);
	bool GateObjectEvent(int16_t objectIndex, int16_t npcId);
	bool BindObjectEvent(int16_t objectIndex, int16_t npcId);
	void SendItemUpgradeRequest(int16_t npcId);
	void InitType3();
	bool GetWarpList(int warp_group);
	void ServerChangeOk(char* pBuf);
	void ZoneConCurrentUsers(char* pBuf);
	void SelectWarpList(char* pBuf);
	void GoldChange(int tid, int gold);

	/// \brief Attempts to perform a skill point reset
	/// \param isFree set to true to bypass cost calculation/charge
	/// \originalName AllSkillPointChange
	void SkillPointResetRequest(bool isFree = false);

	/// \brief Sends an error response for a failed skill point reset
	void SendResetSkillError(e_ClassChangeResult errorCode, int cost);

	/// \brief Attempts to perform a stat point reset
	/// \param isFree set to true to bypass cost calculation/charge
	/// \originalName AllPointChange
	void StatPointResetRequest(bool isFree = false);

	/// \brief Sends an error response for a failed stat point reset
	void SendResetStatError(e_ClassChangeResult errorCode, int cost);

	/// \brief Sends novice promotion eligibility status
	/// \originalName ClassChangeReq
	void NovicePromotionStatusRequest();

	/// \brief Sends a class change status req back to the client in response
	/// to non-free Skill/Stat reset requests
	void ClassChangeRespecReq();

	void FriendReport(char* pBuf);
	std::shared_ptr<CUser> GetItemRoutingUser(int itemid, int16_t itemcount);
	bool GetStartPosition(int16_t* x, int16_t* z, int zoneId) const;
	void Home();
	void ReportBug(char* pBuf);
	int GetEmptySlot(int itemid, int bCountable) const;
	int GetNumberOfEmptySlots() const;
	void InitType4();
	void WarehouseProcess(char* pBuf);
	int16_t GetACDamage(int damage, int tid);
	int16_t GetMagicDamage(int damage, int tid);
	void Type3AreaDuration(double currentTime);
	void ServerStatusCheck();
	void SpeedHackTime(char* pBuf);
	void OperatorCommand(char* pBuf);
	void ItemRemove(char* pBuf);
	void SendAllKnightsID();
	uint8_t ItemCountChange(int itemid, int type, int amount);
	void Type4Duration(double currentTime);
	void ItemRepair(char* pBuf);
	int ExchangeDone();
	void HPTimeChange(double currentTime);
	void HPTimeChangeType3(double currentTime);
	void ItemDurationChange(int slot, int maxvalue, int curvalue, int amount);
	void ItemWoreOut(int type, int damage);
	void Dead();
	void LoyaltyDivide(int tid);
	void UserDataSaveToAgent();
	void CountConcurrentUser();
	void SendUserInfo(char* temp_send, int& index);
	void ChatTargetSelect(char* pBuf);
	bool ItemEquipAvailable(const model::Item* pTable) const;
	void ClassChange(char* pBuf);

	/// \brief Issues the level 60 promotion quest
	void GivePromotionQuest();

	/// \brief Validates that the newClassId is valid promotion path from the current classId
	/// \return true when the user's newClassId is a valid promotion path, false otherwise
	bool ValidatePromotion(e_Class newClassId) const;

	/// \brief Updates the user's class and broadcasts any required messages
	void HandlePromotion(e_Class newClassId);

	void MSpChange(int amount);
	void UpdateGameWeather(char* pBuf, uint8_t type);
	void ObjectEvent(char* pBuf);
	void SendObjectEventFailed(uint8_t objectType, uint8_t errorCode = 0);
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

	/// \brief Processes any national point change associated with killing a target
	void LoyaltyChange(int targetId);

	/// \brief Changes national loyalty by a set amount.
	void ChangeLoyalty(int loyaltyChange, bool isExcludeMonthly);

	/// \brief Changes manner loyalty by a set amount.
	void ChangeMannerPoint(int loyaltyChange);

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
	void Warp(float x, float z);
	void RequestNpcIn(char* pBuf);
	void SetUserAbility();
	void LevelChange(int16_t level, bool bLevelUp = true);
	void HpChange(int amount, int type = 0, bool attack = false);
	int16_t GetDamage(int tid, int magicid);
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
	void SelectCharacter(const char* pBuf);
	void Send2AI_UserUpdateInfo();
	void Attack(char* pBuf);
	void UserInOut(uint8_t Type);
	void MoveProcess(char* pBuf);
	void Rotate(char* pBuf);
	void LoginProcess(char* pBuf);

	/// \brief Attempts to perform a character's first job change
	void PromoteUserNovice();

	/// \brief Attempts to perform a character's second job change
	void PromoteUser();

	/// \brief Updates an event/quest status
	/// \returns true if quest event was successfully saved, false otherwise
	bool SaveEvent(e_QuestId questId, e_QuestState questState);
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_USER_H
