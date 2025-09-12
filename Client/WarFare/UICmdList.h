// UICmdList.h: interface for the CUICmdList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICmdList_H)
#define AFX_UICmdList_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

#include <unordered_map>

enum e_ChatCmd
{
	// Private
	CMD_WHISPER, CMD_TOWN, CMD_EXIT, CMD_GREETING, CMD_GREETING2, CMD_GREETING3,
	CMD_PROVOKE, CMD_PROVOKE2, CMD_PROVOKE3, CMD_GAME_SAVE, CMD_RECOMMEND, CMD_INDIVIDUAL_BATTLE,
#if 0 // unused officially -- they exist, but aren't loaded (& they're Korean in our data)
	CMDSIT_STAND, CMD_WALK_RUN, CMD_LOCATION,
#endif
	
	// Trade
	CMD_TRADE, CMD_FORBIDTRADE, CMD_PERMITTRADE, CMD_MERCHANT,

	// Party
	CMD_PARTY, CMD_LEAVEPARTY, CMD_RECRUITPARTY, CMD_FORBIDPARTY, CMD_PERMITPARTY,

	// Clan
	CMD_JOINCLAN, CMD_WITHDRAWCLAN, CMD_FIRECLAN, CMD_COMMAND, CMD_CLAN_WAR,
	CMD_SURRENDER, CMD_APPOINTVICECHIEF, CMD_CLAN_CHAT, CMD_CLAN_BATTLE,

	// Knights
	CMD_CONFEDERACY, CMD_BAN_KNIGHTS, CMD_QUIT_KNIGHTS, CMD_BASE, CMD_DECLARATION,

	// GM
	CMD_VISIBLE, CMD_INVISIBLE, CMD_CLEAN, CMD_RAINING, CMD_SNOWING, CMD_TIME, CMD_CU_COUNT,
	CMD_NOTICE, CMD_ARREST, CMD_FORBIDCONNECT, CMD_FORBIDCHAT, CMD_PERMITCHAT, CMD_NOTICEALL,
	CMD_CUTOFF, CMD_VIEW, CMD_UNVIEW, CMD_FORBIDUSER, CMD_SUMMONUSER,
	CMD_ATTACKDISABLE, CMD_ATTACKENABLE, CMD_PLC,

	// Guardian Monster
	CMD_HIDE, CMD_GUARD, CMD_DEFEND, CMD_LOOK_OUT, CMD_STRATEGIC_FORMATION, CMD_REST, CMD_DESTROY,

	// King
	CMD_ROYALORDER, CMD_PRIZE, CMD_EXPERIENCEPOINT, CMD_DROPRATE, CMD_RAIN, CMD_SNOW, CMD_CLEAR, CMD_REWARD,

	CMD_COUNT,
	CMD_UNKNOWN = 0xffffffff
};

class CUICmdEdit;
class CUICmdList : public CN3UIBase
{
protected:
	enum e_CmdListSelection : uint8_t
	{
		CMD_LIST_SEL_CATEGORY = 0,	// Category list
		CMD_LIST_SEL_COMMAND		// Command list
	};

	enum e_CmdListCategory : uint8_t
	{
		CMD_LIST_CAT_PRIVATE = 0,
		CMD_LIST_CAT_TRADE,
		CMD_LIST_CAT_PARTY,
		CMD_LIST_CAT_CLAN,
		CMD_LIST_CAT_KNIGHTS,
		CMD_LIST_CAT_GUARDIAN,
		CMD_LIST_CAT_KING,
		CMD_LIST_CAT_GM,
		CMD_LIST_CAT_COUNT
	};

	CUICmdEdit*			m_pUICmdEdit;

	CN3UIButton*		m_pBtn_Cancel;
	CN3UIList*			m_pList_CmdCat;
	CN3UIList*			m_pList_Cmds;

	bool				m_bOpenningNow;		// 열리고 있다..
	bool				m_bClosingNow;		// 닫히고 있다..
	float				m_fMoveDelta;		// 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..
	int					m_iSelectedCategory;
	e_CmdListSelection	m_eSelectedList;

	struct CommandInfo
	{
		uint32_t			ResourceID;
		e_ChatCmd			Command;
	};

	std::multimap<e_CmdListCategory, CommandInfo>		m_categoryToCommandInfoMap;

public:
	CUICmdList();
	~CUICmdList() override;
	bool Load(HANDLE hFile) override;
	void Release() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override; // 메시지를 받는다.. 보낸놈, msg
	bool OnKeyPress(int iKey) override;
	void Open();
	void Close();
	bool CreateCategoryList();
	bool UpdateCommandList(int iCatIndex);
	void AppendToCommandMap(e_CmdListCategory eCategory, e_ChatCmd eBaseCmd, int iFirstResourceID, int iLastResourceID);
	bool ExecuteCommand(int iCmdIndex);
	void Tick() override;
	void Render() override;
	void RenderSelectionBorder(CN3UIList* pListToRender);
	bool OnMouseWheelEvent(short delta) override;
};

#endif // !defined(AFX_UICmdList)



