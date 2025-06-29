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
	CN3UIEdit* m_pEditLevel;
	CN3UIScrollBar* m_pScrollGuide2;
	CN3UIScrollBar* m_pScrollGuide1;
	CN3UIScrollBar* m_pScrollGuide0;
	CN3UIString* m_pTextPage;
	//CN3UIString* m_pTextInfo; //static, no id
	CN3UIButton* m_pButtonCheck;
	CN3UIString* m_pTextGuide2;
	CN3UIString* m_pTextGuide1;
	CN3UIString* m_pTextGuide0;
	CN3UIString* m_pTextTitle2;
	CN3UIString* m_pTextTitle1;
	CN3UIString* m_pTextTitle0;
	CN3UIString* m_pTextLevel;
	CN3UIButton* m_pButtonUp;
	CN3UIButton* m_pButtonDown;
	CN3UIButton* m_pButtonCancel;

	static constexpr int SEARCH_RANGE = 5; //ex: searches 5 levels up and down
	static constexpr int CONTENT_COUNT = 20; //number of contents, equal to m_saQuestTitle,m_saQuestText size
	int m_iSearchLevel;
	int m_iCurrentPage;
	std::string m_saQuestTitle[CONTENT_COUNT];
	std::string m_saQuestText[CONTENT_COUNT];
	void SetTopLine(CN3UIScrollBar* pScroll, CN3UIString* pTextGuide);
public:
	bool Load(HANDLE hFile) override;
	void Release() override;
	CUILevelGuide();
	~CUILevelGuide() override;
	void SetVisible(bool bVisible);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	bool OnKeyPress(int iKey);
	void LoadContent();

};

#endif // !defined(AFX_UIStateBar_H__C1BBB503_F9E5_43BB_93CB_C542AC016F85__INCLUDED_)
