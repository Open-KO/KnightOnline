// UICmdList.h: interface for the CUICmdList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICmdList_H)
#define AFX_UICurtail_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Base.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIList.h>
#include <N3Base/N3UIEdit.h>

#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3Texture.h>
#include <N3Base/N3UITooltip.h>
//#include "N3UIDBCLButton.h"

class CUICmdList : public CN3UIBase
{
protected:

	class CUICmdEdit*	m_pUICmdEdit;

	CN3UIButton*	m_pBtn_cancel;
	CN3UIList*		m_pList_CmdCat;
	CN3UIList*		m_pList_Cmds;

	bool		m_bOpenningNow; // It's opening...
	bool		m_bClosingNow;	// It's closing...
	float		m_fMoveDelta; // To make it open and close smoothly, floating-point numbers are used for calculating the current position.
	int			m_iSelectedCategory;
	int			m_iSelectedTab; //0 => upper(categories), 1 => bottom(commands)
	bool		m_bIsKing;

	enum iCmd
	{
		CMD_LIST_PRIVATE,	
		CMD_LIST_TRADE,	
		CMD_LIST_PARTY,
		CMD_LIST_CLAN,
		CMD_LIST_KNIGHTS,
		CMD_LIST_GUARDIAN,
		CMD_LIST_KING,
		CMD_LIST_GM
	};

	std::map<uint16_t, std::string> m_mapCmds;
	// Attributes
public:
	CUICmdList();
	~CUICmdList() override;
	bool Load(HANDLE hFile) override;
	void Release() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override; // Receive message... sender, msg
	bool OnKeyPress(int iKey) override;
	void SetKing(bool bState){ m_bIsKing = bState; };

	void Open();
	//void OpenEdit();
	void Close();
	bool CreateCategoryList();
	bool UpdateCommandList(uint8_t cmd);
	void UpdateBorders();
	bool ExecuteCommand(uint8_t cmdSel);

	

	void Tick() override;
	void Render() override; // Minimap rendering...
	

	
	
	

};

#endif // !defined(AFX_UICmdList)



