// UICmdList.h: interface for the CUICmdList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICmdList_H)
#define AFX_UICmdList_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

class CUICmdEdit;
class CUICmdList : public CN3UIBase
{
protected:
	enum e_CmdListSelection : uint8_t
	{
		CMD_LIST_SEL_CATEGORY = 0,	// Category list
		CMD_LIST_SEL_COMMAND		// Command list
	};

	enum e_CmdListCategory
	{
		CMD_LIST_CAT_PRIVATE,	
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

	CN3UIButton*		m_pBtn_cancel;
	CN3UIList*			m_pList_CmdCat;
	CN3UIList*			m_pList_Cmds;

	bool				m_bOpenningNow;		// 열리고 있다..
	bool				m_bClosingNow;		// 닫히고 있다..
	float				m_fMoveDelta;		// 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..
	int					m_iSelectedCategory;
	e_CmdListSelection	m_eSelectedList;

	std::map<uint16_t, std::string> m_mapCmds;

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
	bool ExecuteCommand(int iCmdIndex);
	void Tick() override;
	void Render() override;
	void RenderSelectionBorder(CN3UIList* pListToRender);
};

#endif // !defined(AFX_UICmdList)



