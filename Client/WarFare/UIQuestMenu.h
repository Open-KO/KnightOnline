// UIQuestMenu.h: interface for the CUIQuestMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIQUESTMENU_H__B74550FB_798B_4DB8_91DD_EE5994976EDE__INCLUDED_)
#define AFX_UIQUESTMENU_H__B74550FB_798B_4DB8_91DD_EE5994976EDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

#define MAX_STRING_MENU	10//5

class Packet;
class CUIQuestMenu   : public CN3UIBase
{
protected:
	class CN3UIString*		m_pTextTitle;
	class CN3UIString*		m_pTextSample;
	class CN3UIString*		m_pTextMenu[MAX_STRING_MENU];

	int m_iMenuCnt;

	// NOTE(srmeier): adding the components for the newer quest menu
	CN3UIImage*  m_pTextMenuImg[MAX_STRING_MENU];
	CN3UIImage*  m_pTextMenuImgBk[MAX_STRING_MENU];
	CN3UIButton* m_pTextMenuBtn[MAX_STRING_MENU];

	CN3UIButton*    m_pBtnClose;
	CN3UIString*    m_pStrNpcName;
	CN3UIScrollBar* m_pScrollBar;
	CN3UIButton*    m_pBtnMenu;
	CN3UIImage*     m_pImageBtn;
	CN3UIImage*     m_pImageBottom;
	CN3UIImage*     m_pImageMenu;

public:
	void SetVisible(bool bVisible);
	bool OnKeyPress(int iKey);
	void	MsgSend_SelectMenu(uint8_t index);
	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void	InitBase();
	bool	Load(HANDLE hFile);
	void	Open(Packet& pkt);

	CUIQuestMenu();
	virtual ~CUIQuestMenu();

};

#endif // !defined(AFX_UIQUESTMENU_H__B74550FB_798B_4DB8_91DD_EE5994976EDE__INCLUDED_)
