// UILevelGuide.h: interface for the CUILevelGuide class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UILevelGuide_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)
#define AFX_UILevelGuide_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

class CUILevelGuide : public CN3UIBase
{
protected:
	static constexpr int SEARCH_RANGE			= 5;		// ex: searches 5 levels up and down
	static constexpr int MAX_LIST_SIZE			= 20;		// max list size
	static constexpr int VISIBLE_ENTRY_COUNT	= 3;
	static constexpr int MAX_PAGE_COUNT
		= (MAX_LIST_SIZE / VISIBLE_ENTRY_COUNT) + ((MAX_LIST_SIZE % VISIBLE_ENTRY_COUNT) != 0 ? 1 : 0);

	CN3UIEdit*		m_pEdit_Level;
	CN3UIScrollBar*	m_pScroll_Guide[VISIBLE_ENTRY_COUNT];
	CN3UIString*	m_pText_Guide[VISIBLE_ENTRY_COUNT];
	CN3UIString*	m_pText_Title[VISIBLE_ENTRY_COUNT];
	CN3UIString*	m_pText_Page;
	CN3UIButton*	m_pBtn_Check;
	CN3UIString*	m_pText_Level;
	CN3UIButton*	m_pBtn_Up;
	CN3UIButton*	m_pBtn_Down;
	CN3UIButton*	m_pBtn_Cancel;

	int				m_iSearchLevel;
	int				m_iCurrentPage;
	std::string		m_saQuestTitle[MAX_LIST_SIZE];
	std::string		m_saQuestText[MAX_LIST_SIZE];

public:
	CUILevelGuide();
	~CUILevelGuide() override;
	bool Load(HANDLE hFile) override;
	void Release() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;
	void LoadContent();

protected:
	void SetTopLine(CN3UIScrollBar* pScroll, CN3UIString* pTextGuide);
};

#endif // !defined(AFX_UIStateBar_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)
