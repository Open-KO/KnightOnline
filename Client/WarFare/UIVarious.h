// UIVarious.h: interface for the CUIState class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIVarious_H__094F9A3E_1620_4E07_8726_82881B21B48C__INCLUDED_)
#define AFX_UIVarious_H__094F9A3E_1620_4E07_8726_82881B21B48C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameDef.h"

#include <N3Base/N3UIBase.h>

#include <string>
#include <list>

class CUIState : public CN3UIBase  
{
public:
	CN3UIString*	m_pText_ID;
	CN3UIString*	m_pText_Level;
	CN3UIString*	m_pText_RealmPoint;

	CN3UIString*	m_pText_Class;
	CN3UIString*	m_pText_Race;
	CN3UIString*	m_pText_Nation;

	CN3UIString*	m_pText_HP;
	CN3UIString*	m_pText_MP;
	CN3UIString*	m_pText_Exp;
	CN3UIString*	m_pText_AP;			// 공격력
	CN3UIString*	m_pText_GP;			// 방어력
	CN3UIString*	m_pText_Weight;

	CN3UIString*	m_pText_BonusPoint;

	CN3UIButton*	m_pBtn_Strength;
	CN3UIButton*	m_pBtn_Stamina;
	CN3UIButton*	m_pBtn_Dexterity;
	CN3UIButton*	m_pBtn_MagicAttak;
	CN3UIButton*	m_pBtn_Intelligence;

	CN3UIString*	m_pText_Strength;
	CN3UIString*	m_pText_Stamina;
	CN3UIString*	m_pText_Dexterity;
	CN3UIString*	m_pText_MagicAttak;
	CN3UIString*	m_pText_Intelligence;

	CN3UIString*	m_pText_RegistFire;
	CN3UIString*	m_pText_RegistMagic;
	CN3UIString*	m_pText_RegistIce;
	CN3UIString*	m_pText_RegistCurse;
	CN3UIString*	m_pText_RegistLight;
	CN3UIString*	m_pText_RegistPoison;

	CN3UIBase*		m_pImg_MAP;
	CN3UIBase*		m_pImg_Sta;
	CN3UIBase*		m_pImg_Int;
	CN3UIBase*		m_pImg_Dex;
	CN3UIBase*		m_pImg_Str;

public:
	void	UpdateBonusPointAndButtons(int iBonusPointRemain); // 보너스 포인트 적용이 가능한가??

	void	UpdateID(const std::string& szID);
	void	UpdateLevel(int iVal);
	void	UpdateRealmPoint(int iLoyalty, int iLoyaltyMonthly); // Edited by @Demircivi while integrating monthly np system. // 국가 기여도는 10을 나누어서 표시

	void	UpdateHP(int iVal, int iValMax);
	void	UpdateMSP(int iVal, int iValMax);
	void	UpdateExp(int64_t iVal, int64_t iValMax);
	void	UpdateAttackPoint(int iVal, int iDelta);
	void	UpdateGuardPoint(int iVal, int iDelta);
	void	UpdateWeight(int iVal, int iValMax);
	void	UpdateStrength(int iVal, int iDelta);
	void	UpdateStamina(int iVal, int iDelta);
	void	UpdateDexterity(int iVal, int iDelta);
	void	UpdateIntelligence(int iVal, int iDelta);
	void	UpdateMagicAttak(int iVal, int iDelta);

	void	UpdateRegistFire(int iVal, int iDelta);
	void	UpdateRegistCold(int iVal, int iDelta);
	void	UpdateRegistLight(int iVal, int iDelta);
	void	UpdateRegistMagic(int iVal, int iDelta);
	void	UpdateRegistCurse(int iVal, int iDelta);
	void	UpdateRegistPoison(int iVal, int iDelta);

	static std::string FormatWithDelta(int iVal, int iDelta);

	void			MsgSendAblityPointChange(uint8_t byType, int16_t siValueDelta); // 능력치 변화 패킷으로 보내기..

	virtual bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	virtual bool	Load(HANDLE hFile);

	virtual void	Release();
	CUIState();
	virtual ~CUIState();
};

struct __KnightsMemberInfo // 기사 단원 정보..
{
	std::string		szName;
	e_KnightsDuty	eDuty;	// 기사단 직위.
	int				iLevel; // 플레이어 레벨..
	e_Class			eClass; // 직업.
	int				iConnected;
};

typedef std::list<__KnightsMemberInfo>::iterator it_KMI;
#define MAX_CLAN_GRADE	5

class CUIKnights : public CN3UIBase  
{
protected:
	int								m_iPageCur;
	std::list<__KnightsMemberInfo>	m_MemberList;

	CN3UIString*	m_pText_Name;
	CN3UIString*	m_pText_Duty;
	CN3UIString*	m_pText_Page;
	CN3UIString*	m_pText_MemberCount;

	CN3UIImage*		m_pImage_Grade[MAX_CLAN_GRADE];

	CN3UIList*		m_pList_CharGrades;
	CN3UIList*		m_pList_CharIDs;
	CN3UIList*		m_pList_CharLevels;
	CN3UIList*		m_pList_CharJobs;

	CN3UIButton*	m_pBtn_Admit;
	CN3UIButton*	m_pBtn_Appoint;
	CN3UIButton*	m_pBtn_Remove;
	CN3UIButton*	m_pBtn_Refresh;

public:	
	float			m_fTimeLimit_Refresh;
	float			m_fTimeLimit_Appoint;
	float			m_fTimeLimit_Remove;
	float			m_fTimeLimit_Admit;
	
public:
	void	Clear();
	void	UpdateKnightsName(const std::string& szName);
	void	UpdateKnightsDuty(e_KnightsDuty eDuty);
	void	UpdateKnightsGrade(int iVal);
	void	UpdateKnightsRank(int iVal);
	void	UpdateExceptList();

	void	MemberListUpdate();
	void	MemberListSort();

	void	RefreshList(); 

	void	MsgSend_MemberInfoAll();

	void	SetVisible(bool bVisible);		
	virtual bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	virtual bool Load(HANDLE hFile);

	void ClearLists();

	void UpdatePageNumber(int nNewPageNr = -1);
	void UpdateMemberCount(int nMemberCount);

	void PreviousPageButtonHandler();
	void NextPageButtonHandler();
	void RefreshButtonHandler(bool blBypassTime = false);
	void WhisperButtonHandler();
	void AdmitButtonHandler();
	void RemoveButtonHandler();
	void AppointButtonHandler();

	virtual void Release();
	CUIKnights();
	virtual ~CUIKnights();

	//쓰지 않음....
	bool	NeedMemberListRequest() { if(m_MemberList.empty()) return true; else return false; }
	
	void	ChangeUIByDuty(e_KnightsDuty eDuty);
	//void	VisibleAppointButtons(bool bVisible); // 기사단장 전용 임명 Interface
	
	//void	MsgSend_MemberInfoOnline(int iPage);
	bool	MsgRecv_MemberInfo(Packet& pkt);
	
	//void	MsgSend_MemberJoinAdmit();
	//void	MsgSend_MemberJoinReject();
	//void	MsgSend_MemberPunish();
	//void	MsgSend_MemberRemove();
	//void	MsgSend_DutyAppoint(e_KnightsDuty eDuty);
};

struct __FriendsInfo // 기사 단원 정보..
{
	std::string	szName;
	int			iID; // ID
	bool		bOnLine; // 접속했나?
	bool		bIsParty; // 파티 플레이중인가?

	void Init()
	{
		szName = "";
		iID = -1;
		bOnLine = false;
		bIsParty = false;
	}

	bool operator () (const __FriendsInfo& x, const __FriendsInfo& y) const
	{
		return (x.szName >= y.szName);
	}

	__FriendsInfo()
	{
		Init();
	}
};

typedef std::map<std::string, __FriendsInfo>::iterator it_FI;
typedef std::map<std::string, __FriendsInfo>::value_type val_FI;

class CUIFriends : public CN3UIBase  
{
protected:
	int m_iPageCur;
	std::map<std::string, __FriendsInfo> m_MapFriends;

	CN3UIList* m_pList_Friends;
	CN3UIString* m_pText_Page;

	CN3UIButton* m_pBtn_NextPage;
	CN3UIButton* m_pBtn_PrevPage;
	
	CN3UIButton* m_pBtn_Refresh;
	CN3UIButton* m_pBtn_Party;
	CN3UIButton* m_pBtn_Whisper;
	CN3UIButton* m_pBtn_Add;
	CN3UIButton* m_pBtn_Delete;

public:
	void		SaveListToTextFile(const std::string& szID);  // 문자열이 있으면 추가하고.. 없으면 몽땅 저장..
	void		MsgSend_MemberInfo(bool bDisableInterval); // 현재 페이지 정보 요청
	void		MsgSend_MemberInfo(const std::string& szID); // 이넘만 요청..
	void		MsgRecv_MemberInfo(Packet& pkt);
	void		UpdateList();
	bool		MemberDelete(const std::string& szID);
	bool		MemberAdd(const std::string& szID, int iID, bool bOnLine, bool bIsParty);

	virtual bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	virtual bool Load(HANDLE hFile);
	
	CUIFriends();
	virtual ~CUIFriends();

};



class CUIQuest : public CN3UIBase
{
protected:


public:
	virtual bool Load(HANDLE hFile);

	CUIQuest();
	virtual ~CUIQuest();
};



class CUIVarious : public CN3UIBase // 다용도 UI
{
public:
	CUIState*		m_pPageState;
	CUIKnights*		m_pPageKnights;
	CUIQuest*		m_pPageQuest;
	CUIFriends*		m_pPageFriends;
	
protected:
	CN3UIButton*	m_pBtn_Knights;
	CN3UIButton*	m_pBtn_State;
	CN3UIButton*	m_pBtn_Quest;
	CN3UIButton*	m_pBtn_Friends;
	CN3UIButton*	m_pBtn_Close;

	bool		m_bOpenningNow; // 열리고 있다..
	bool		m_bClosingNow;	// 닫히고 있다..
	float		m_fMoveDelta; // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..

public:
	void		SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false);
	void		SetVisible(bool bVisible);
	bool		OnKeyPress(int iKey);
	void		UpdatePageButtons(CN3UIButton* pButtonToActive);
	void		UpdateAllStates(const struct __InfoPlayerBase* pInfoBase, const struct __InfoPlayerMySelf* pInfoExt);
	void		UpdateKnightsInfo(); // 기사단 관련 정보 업데이트
	void		Open();
	void		Close();
	
	virtual void	Tick();
	virtual	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	virtual bool	Load(HANDLE hFile);

	virtual void Release();
	CUIVarious();
	virtual ~CUIVarious();
};

#endif // !defined(AFX_UIVarious_H__094F9A3E_1620_4E07_8726_82881B21B48C__INCLUDED_)
