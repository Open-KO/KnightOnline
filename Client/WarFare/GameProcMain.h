#pragma once

#include "GameProcedure.h"
#include <set>

typedef std::set<int>::iterator it_ID;
typedef std::pair<it_ID, bool> pair_ID;

class CGameProcMain : public CGameProcedure  
{
	friend class CGameProcedure;
protected:
	std::set<int> m_SetNPCID;
	std::set<int> m_SetUPCID;

public:
#ifdef _N3_64GRID_
	CServerMesh		*m_pSMesh;									// 서버에게 필요한 메쉬 클래스..
#endif
	class CUIInventory*			m_pUIInventory;			// 인벤토리
	class CUIVarious*			m_pUIVar;				// 캐릭터 상태창, 기사단 관리 등이 페이지로 들어간 다용도 UI
	class CUIChat*				m_pUIChatDlg;			// 채팅 입출력 대화상자..
	class CUIMessageWnd*		m_pUIMsgDlg;			// 게임 메시지 출력 상자.
	
	// Folded UI
	class CUIChat2*			m_pUIChatDlg2;
	class CUIMessageWnd2*	m_pUIMsgDlg2;

	class CUIStateBar*			m_pUIStateBarAndMiniMap;	// mp,hp,exp, minimap....
	class CUICmd*				m_pUICmd;				// 왼쪽 하단의 명령버튼 창..
	class CUITargetBar*			m_pUITargetBar;			// 타겟 상태창..
	class CUICmdList*			m_pUICmdList;	    
	class CUICmdEdit*			m_pUICmdEdit;
	class CUITransactionDlg*	m_pUITransactionDlg;
	class CUIDroppedItemDlg*	m_pUIDroppedItemDlg;
	class CSubProcPerTrade*		m_pSubProcPerTrade;
	class CUIHelp*				m_pUIHelp;
	class CUIPartyOrForce*		m_pUIPartyOrForce;
	class CUISkillTreeDlg*		m_pUISkillTreeDlg;
	class CUIHotKeyDlg*			m_pUIHotKeyDlg;
	class CUINotice*			m_pUINotice;
	class CUIClassChange*		m_pUIClassChange;
	class CUINPCEvent*			m_pUINpcEvent;
	class CUIRepairTooltipDlg*	m_pUIRepairTooltip;
	class CUIWareHouseDlg*		m_pUIWareHouseDlg;
	class CUIInn*				m_pUIInn;
	class CUICreateClanName*	m_pUICreateClanName;
	class CUIExitMenu*			m_pUIExitMenu;


	class CUIKnightsOperation*	m_pUIKnightsOp;					// 기사단 리스트 보기, 가입, 등...
	class CUIPartyBBS*			m_pUIPartyBBS;					// 파티 지원 게시판.
	class CUITradeSellBBS*		m_pUITradeBBS;					// 상거래 게시판
	class CUIQuestMenu*			m_pUIQuestMenu;					// Quest Menu
	class CUIQuestTalk*			m_pUIQuestTalk;					// Quest Talk
	class CUIDead*				m_pUIDead;						// Dead UI
	class CUITradeBBSSelector*	m_pUITradeBBSSelector;			// 상거래 게시판 종류 선택
	class CUITradeBBSEditDlg*	m_pUITradeBBSEdit;				// 상거래 게시물 설명

	class CUIUpgradeSelect*		m_pUIUpgradeSelect;
	class CUILevelGuide*		m_pUILevelGuide;

	class CN3Shape*				m_pTargetSymbol;				// 플레이어가 타겟으로 잡은 캐릭터의 위치위에 그리면 된다..

	class CN3SndObjStream*		m_pSnd_Town, *m_pSnd_Battle;	//마을음악, 전투음악 포인터..
	class CMagicSkillMng*		m_pMagicSkillMng;

	class CUINPCChangeEvent*	m_pUINpcChange;
	class CUIWarp*				m_pUIWarp;
	class CWarMessage*			m_pWarMessage;					// 전쟁관련 메시지
	class CLightMgr*			m_pLightMgr;

	   
	//..
	BOOL		m_bLoadComplete;							// 로딩이 완료되었나??
	
	float		m_fMsgSendTimeMove;							// 최근 이동 메시지 보낸 시간..
	float		m_fMsgSendTimeRot;							// 최근 회전 메시지 보낸 시간..
	float		m_fPlayerYawSended; // 최근에 메시지를 보낸 시점의 플레이어 y 축 회전값.
	float		m_fRequestGameSave;	// 최근에 게임 데이터 저장을 요청한 시간
	__Vector3	m_vPlayerPosSended; // 최근에 메시지를 보낸 시점의 플레이어 위치.

	__Vector3	m_vMouseLBClickedPos;
	__Vector3	m_vMouseSkillPos;

	float		m_fExitTimer;
	float		m_fLBClickTime;

	int			m_iJoinReqClan;
	int			m_iJoinReqClanRequierID;

	int			KM_COUNT;

	BOOL		m_bIsExitCanceled;
	e_ExitType	m_eExitType;
	int			m_iExitTimeRemaining; // seconds remaining until exit

	float		m_fMBRotateTime;

	std::string	m_szWarpDestination;

protected:
	virtual bool ProcessPacket(Packet& pkt);

	bool	MsgRecv_CharacterSelect(Packet& pkt); // virtual
	int		MsgRecv_VersionCheck(Packet& pkt); // virtual

	bool	MsgRecv_MyInfo_All(Packet& pkt);
	void	MsgRecv_MyInfo_HP(Packet& pkt);
	void	MsgRecv_MyInfo_MSP(Packet& pkt);
	void	MsgRecv_MyInfo_EXP(Packet& pkt);
	bool	MsgRecv_MyInfo_LevelChange(Packet& pkt);
	void	MsgRecv_MyInfo_RealmPoint(Packet& pkt);
	void	MsgRecv_MyInfo_PointChange(Packet& pkt);

	bool	MsgRecv_Chat(Packet& pkt);
	bool	MsgRecv_UserMove(Packet& pkt);
	bool	MsgRecv_Rotation(Packet& pkt);
//	bool	MsgRecv_Dead(Packet& pkt);
	bool	MsgRecv_Regen(Packet& pkt);
	void	MsgRecv_Corpse(Packet& pkt);
	bool	MsgRecv_Time(Packet& pkt);
	bool	MsgRecv_Weather(Packet& pkt);
	
	bool	MsgRecv_UserInAndRequest(Packet& pkt);		// 주위 영역의 모든 User 아이디를 카운트만큼 받는다... 글구.. 업데이트가 필요한 것만 서버에게 요청..
	bool	MsgRecv_UserInRequested(Packet& pkt);					// 서버에게 요청한 유저의 아이디들을 받아서 User 처리..
	bool	MsgRecv_UserInOut(Packet& pkt);
	bool	MsgRecv_UserIn(Packet& pkt, bool bWithFX=false);
	bool	MsgRecv_UserOut(Packet& pkt);
	void	MsgRecv_UserState(Packet& pkt);

	bool	MsgRecv_NPCInAndRequest(Packet& pkt);		// 주위 영역의 모든 NPC 아이디를 카운트만큼 받는다... 글구.. 업데이트가 필요한 것만 서버에게 요청..
	bool	MsgRecv_NPCInRequested(Packet& pkt);				// 서버에게 요청한 NPC 아이디들을 받아서 User 처리..
	bool	MsgRecv_NPCInOut(Packet& pkt);
	bool	MsgRecv_NPCIn(Packet& pkt);
	bool	MsgRecv_NPCOut(Packet& pkt);
	bool	MsgRecv_NPCMove(Packet& pkt);

	bool	MsgRecv_Attack(Packet& pkt);
	bool	MsgRecv_Dead(Packet& pkt);

	bool	MsgRecv_ItemMove(Packet& pkt);					// Item Move에 대한 응답..
	bool	MsgRecv_ItemBundleDrop(Packet& pkt);			// Item 이 필드에 나타나는데에 대한 응답
	bool	MsgRecv_ItemBundleOpen(Packet& pkt);			// 아이템 상자를 열거나 시체를 뒤진다..
	bool	MsgRecv_ItemTradeStart(Packet& pkt);			// 아이템 상거래..
	bool	MsgRecv_ItemTradeResult(Packet& pkt);			// 아이템 상거래 결과..
	bool	MsgRecv_ItemDroppedGetResult(Packet& pkt);	// 땅에 떨어진 아이템 먹기 결과..
	bool	MsgRecv_ItemWeightChange(Packet& pkt);		// 아이템 무게 변화..

	bool	MsgRecv_UserLookChange(Packet& pkt);			// 겉모습이 바뀐다.. 아이템 착용등

	void	MsgRecv_TargetHP(Packet& pkt);

	void	MsgRecv_ZoneChange(Packet& pkt);			// 걍 단순한 존 체인지.

	void	MsgRecv_Notice(Packet& pkt);
	void	MsgRecv_PartyOrForce(Packet& pkt);
	void	MsgRecv_PerTrade(Packet& pkt);

	void	MsgRecv_SkillChange(Packet& pkt);			// 스킬 변화..
	void	MsgRecv_MagicProcess(Packet& pkt);			// 스킬 변화..
	void	MsgRecv_ClassChange(Packet& pkt);			// 직업 변화..
	void	MsgRecv_ClassPromotion(Packet& pkt);

	void	MsgRecv_ObjectEvent(Packet& pkt);			// 오브젝트 이벤트 신청에 대한 응답

	void	MsgRecv_DurabilityChange(Packet& pkt);		// 내구력 변경..
	void	MsgRecv_NpcEvent(Packet& pkt);				// Npc Event(Exchange, Repair both).. 

	void	MsgRecv_Knights(Packet& pkt);				// 기사단 조작 관련 패킷..
	void	MsgRecv_KnightsListBasic(Packet& pkt);		// 기사단 기본 정보들..
	
	void	MsgRecv_CompressedPacket(Packet& pkt);		// 압축된 패킷이다... 압축 풀고 루프를 돌면서 한번더 파싱해야 한다!!!
	void	MsgRecv_ContinousPacket(Packet& pkt);		// 붙어서 오는 패킷이다.. 루프를 돌면서 한번더 파싱해야 한다!!!

	void	MsgRecv_ItemRepair(Packet& pkt);			// Item Repair Result.. 
	void	MsgRecv_ItemCountChange(Packet& pkt);		// Item Count Change..
	void	MsgRecv_ItemDestroy(Packet& pkt);			// Item Count Change..

	void	MsgRecv_WareHouse(Packet& pkt);			// 보관함 관련 패킷..
	void	MsgRecv_WareHouseOpen(Packet& pkt);		// 보관함 오픈..

	void	MsgRecv_NpcChangeOpen(Packet& pkt);		// Class Change와 초기화..
	void	MsgRecv_AllPointInit(Packet& pkt);			// All Point 초기화..
	void	MsgRecv_SkillPointInit(Packet& pkt);		// Skill Point 초기화..
	void	MsgRecv_PointChangePriceQueryRequest(Packet& pkt);		// 가격에 대한 응답 패킷..

	void	MsgRecv_NoahChange(Packet& pkt);			// 노아 변경..
	void	MsgRecv_WarpList(Packet& pkt);				// 워프 리스트..
	void	MsgRecv_WarpList_Error(Packet& pkt);		// 워프 리스트..

	//	void	MsgRecv_ServerCheckAndRequestConcurrentUserCount(Packet& pkt);			// 서버 IP 와 포트를 받아 동접자를 체크해 본다..
//	void	MsgRecv_ConcurrentUserCountAndSendServerCheck(Packet& pkt);
	
	//knights...
	void	MsgRecv_Knights_Create(Packet& pkt);
	void	MsgRecv_Knights_Withdraw(Packet& pkt);
	void	MsgRecv_Knights_Join(Packet& pkt);
	void	MsgRecv_Knights_Leave(Packet& pkt);
	void	MsgRecv_Knights_AppointViceChief(Packet& pkt);
	void	MsgRecv_Knights_MemberInfoAll(Packet& pkt);
	void	MsgRecv_Knights_GradeChangeAll(Packet& pkt);
	void	MsgRecv_Knights_Duty_Change(Packet& pkt);
	void	MsgRecv_Knigts_Join_Req(Packet& pkt);
	void	MsgRecv_ItemUpgrade(Packet& pkt);

	void	MsgRecv_ZoneAbility(Packet& pkt);

public:
	void	ProcessUIKeyInput(bool bEnable = true);
	bool	OnMouseMove(POINT ptCur, POINT ptPrev);
	bool	OnMouseLbtnDown(POINT ptCur, POINT ptPrev);
	bool	OnMouseLBtnPressd(POINT ptCur, POINT ptPrev);
	bool	OnMouseLBtnPress(POINT ptCur, POINT ptPrev);
	bool	OnMouseLDBtnPress(POINT ptCur, POINT ptPrev);
	
	/// \brief attempts to start the auto-attack process
	/// \returns true if auto-attack process started, false otherwise
	bool	TryStartAttack();
	bool	OnMouseRbtnDown(POINT ptCur, POINT ptPrev);
	bool	OnMouseRBtnPressd(POINT ptCur, POINT ptPrev);
	bool	OnMouseRBtnPress(POINT ptCur, POINT ptPrev);
	bool	OnMouseRDBtnPress(POINT ptCur, POINT ptPrev);
	
	void	ControlViewVDegree(int16_t sValue);
	void	PlayBGM_Town();
	void	PlayBGM_Battle();

	void	DoCommercialTransaction(int iTradeID);								// 상거래..

	const __InfoPartyOrForce*	PartyOrForceConditionGet(bool& bIAmLeader, bool& bIAmMember, int& iMemberIndex, class CPlayerBase*& pTarget);
	void						TargetSelect(int iID, bool bMustAlive);
	void						TargetSelect(class CPlayerNPC* pTarget);
	
	void	CommandToggleUIChat();
	void	CommandToggleUIMsgWnd();

	bool	CommandToggleUIInventory();
	bool	CommandToggleUIState();
	bool	CommandToggleAttackContinous();
	bool	CommandToggleMoveContinous();
	bool	CommandToggleWalkRun();
	bool	CommandToggleUISkillTree();
	bool	CommandToggleUIMiniMap();
	bool	CommandToggleCmdList();
	bool	CommandToggleLevelGuide();
	bool	OpenCmdEdit(std::string msg);

	void	CommandMove(e_MoveDirection eMD, bool bStartOrEnd); // 움직이는 방향(전후진, 멈춤), 움직이기 시작하는가?
	void	CommandEnableAttackContinous(bool bEnable, CPlayerBase* pTarget);

	/// \brief contains the logic that should be executed whenever starting to auto-attack
	void	StartAutoAttack(CPlayerBase* target);
	/// \brief contains the logic that should be executed whenever auto-attacking is stopped
	void	StopAutoAttack(CPlayerBase* target = nullptr);
	
	void	CommandCameraChange(); // 카메라 시점 바꾸기..
	void	CommandSitDown(bool bLimitInterval, bool bSitDown, bool bImmediately = false);

	void	CommandTargetSelect_NearestEnemy(); // 가장 가까운 적 타겟 잡기..
	void	CommandTargetSelect_NearestOurForce(); // 가장 가까운 파티 타겟잡기..
	void	CommandTargetSelect_NearestNPC(); // target nearest NPC

	void	CloseUIs(); // 각종 상거래, 워프등등... UI 닫기..

	void	MsgOutput(const std::string& szMsg, D3DCOLOR crMsg);

	void	InitZone(int iZone, const __Vector3& vPosPlayer);
	void	InitUI();
	void	InitPlayerPosition(const __Vector3& vPos); // 플레이어 위치 초기화.. 일으켜 세우고, 기본동작을 취하게 한다.

	void	MsgSend_Continous();												// 특정 조건(?)하에서 서버에게 정기적으로 메시지를 보냄..
	void	MsgSend_Attack(int iTargetID, float fInterval, float fDistance);	// 공격 패킷 날리기 - 테이블의 공격 주기를 같이 줘서 해킹을 막는다.
	void	MsgSend_Move(bool bMove, bool bContinous);							// 서버에게 움직임 패킷을 날린다.. // 움직이는가 ? 주기적으로 움직이는 건가?
	void	MsgSend_Rotation();													// 서버에게 회전 패킷을 날린다..
	void	MsgSend_Chat(enum e_ChatMode eMode, const std::string& szChat);		// 서버에게 채팅 메시지를 날린다..
	void	MsgSend_ChatSelectTarget(const std::string& szTargetID); // 일대일 채팅 상대 정하기.
	void	MsgSend_Regen();
	bool	MsgSend_RequestItemBundleOpen(CPlayerNPC* pCorpse); // 아이템 상자를 열거나 시체를 뒤진다..
	void	MsgSend_RequestTargetHP(int16_t siIDTarget, uint8_t byUpdateImmediately); // 0x00 - 점차 늘어나게끔.. 0x01 - 즉시 업데이트..
	void	MsgSend_GameStart();
	bool	MsgSend_NPCEvent(int16_t siIDTarget);
	void	MsgSend_NPCInRequest(int iID); // NPC 정보가 없을 경우 요청한다..
	void	MsgSend_UserInRequest(int iID); // User 정보가 없을 경우 요청한다..
	void	MsgSend_Warp(); // 워프?? - 존체인지가 될수도 있다..
	void	MsgSend_StateChange(enum e_SubPacket_State eSP, int iState);
	void	MsgSend_PerTradeReq(int iDestID, bool bNear = true);
	void	MsgSend_SpeedCheck(bool bInit = false);

	void	MsgSend_PartyOrForcePermit(int iPartyOrForce, bool bYesNo); // iPartyOrForce 1 : Party, 2:Force
	void	MsgSend_PartyOrForceLeave(int iPartyOrForce); // iPartyOrForce 1 : Party, 2:Force
	bool	MsgSend_PartyOrForceCreate(int iPartyOrForce, const std::string& szID); // iPartyOrForce 1 : Party, 2:Force

	void	MsgSend_ObjectEvent(int iEventID, int iNPCID);		// 오브젝트에 설정되어 있는 이벤트 요청..
	void	MsgSend_Weather(int iWeather, int iPercent);
	void	MsgSend_Time(int iHour, int iMin);
	void	MsgSend_Administrator(enum e_SubPacket_Administrator eSP, const std::string& szID);
	
	void	MsgSend_KnightsJoin(int iTargetID);
	void	MsgSend_KnightsLeave(std::string& szName);
	void	MsgSend_KnightsWithdraw();
	void	MsgSend_KnightsAppointViceChief(std::string& szName);
	void	MsgSend_KnightsJoinReq(bool bJoin);	
	void	MsgSend_PerTradeBBSReq(std::string szName, int iDestID);
	void	MsgSend_CharacterSelect(); // virtual


	void	ProcessPlayerInclination();				// 경사 처리..(가만히 있어도 경사가 급하면 미끄러짐..).
	void	ProcessLocalInput(uint32_t dwMouseFlags);	// 키보드 눌린것을 처리한다..

	void	RequestExit();
	void	ParseChattingCommand(const std::string& szCmd);
	

	void	UpdateUI_PartyOrForceButtons(); // 커맨드 줄에 있는 파티 버튼을 상황에 따라 업데이트 해준다.
	void	UpdateUI_MiniMap();
	void	UpdateUI_TargetBar();
	void	UpdateBGM();
	void	UpdateCameraAndLight();

	void	RenderTarget();
	
	void	Init();									// UI 와 UI 리소스등을 읽는다.
	void	Release();								// Release..	
	void	ReleaseUIs();
	void	ReleaseSound();
	
	void	Tick();								// 잡다한 계산..
	void	Render();							// 렌더링..

	CGameProcMain();									// 생성자.
	virtual ~CGameProcMain();							// 소멸자.
};
