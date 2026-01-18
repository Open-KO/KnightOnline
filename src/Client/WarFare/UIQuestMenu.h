// UIQuestMenu.h: interface for the CUIQuestMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIQUESTMENU_H__B74550FB_798B_4DB8_91DD_EE5994976EDE__INCLUDED_)
#define AFX_UIQUESTMENU_H__B74550FB_798B_4DB8_91DD_EE5994976EDE__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class Packet;
class CUIQuestMenu : public CN3UIBase
{
protected:
	static constexpr int MAX_STRING_MENU = 10;

	CN3UIString* m_pTextTitle;
	CN3UIString* m_pTextSample;
	CN3UIString* m_pTextMenu[MAX_STRING_MENU];

	int m_iMenuCnt;

	CN3UIImage* m_pTextMenuImg[MAX_STRING_MENU];
	CN3UIImage* m_pTextMenuImgBk[MAX_STRING_MENU];
	CN3UIButton* m_pTextMenuBtn[MAX_STRING_MENU];

	CN3UIButton* m_pBtnClose;
	CN3UIString* m_pStrNpcName;
	CN3UIScrollBar* m_pScrollBar;
	CN3UIButton* m_pBtnMenu;
	CN3UIImage* m_pImageBtn;
	CN3UIImage* m_pImageBottom;
	CN3UIImage* m_pImageMenu;

public:
	CUIQuestMenu();
	~CUIQuestMenu() override;
	bool Load(File& file) override;
	void SetVisible(bool bVisible) override;
	bool OnKeyPress(int iKey) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void MsgSend_SelectMenu(uint8_t index);
	void InitBase();
	void Open(Packet& pkt);

protected:
	void UpdateTextForScroll();
};

#endif // !defined(AFX_UIQUESTMENU_H__B74550FB_798B_4DB8_91DD_EE5994976EDE__INCLUDED_)
