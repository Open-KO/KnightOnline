// UILevelGuide.h: interface for the CUILevelGuide class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UILevelGuide_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)
#define AFX_UILevelGuide_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUILevelGuide : public CN3UIBase
{
protected:
	static constexpr int MAX_SEARCH_LEVEL_RANGE = 5;
	static constexpr int MAX_QUESTS_PER_PAGE    = 3;

	CN3UIEdit* m_pEdit_Level;
	CN3UIScrollBar* m_pScroll_Guide[MAX_QUESTS_PER_PAGE];
	CN3UIString* m_pText_Guide[MAX_QUESTS_PER_PAGE];
	CN3UIString* m_pText_Title[MAX_QUESTS_PER_PAGE];
	CN3UIString* m_pText_Page;
	CN3UIButton* m_pBtn_Check;
	CN3UIString* m_pText_Level;
	CN3UIButton* m_pBtn_Up;
	CN3UIButton* m_pBtn_Down;
	CN3UIButton* m_pBtn_Cancel;

	int m_iSearchLevel;
	int m_iPageNo;

public:
	CUILevelGuide();
	~CUILevelGuide() override;
	bool Load(File& file) override;
	void Release() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;
	void SearchQuests();
	void SetPageNo(int iPageNo);

protected:
	void SetTopLine(CN3UIScrollBar* pScroll, CN3UIString* pTextGuide);
};

#endif // !defined(AFX_UIStateBar_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)
