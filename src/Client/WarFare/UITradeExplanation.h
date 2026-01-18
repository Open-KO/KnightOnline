// UITradeExplanation.h: interface for the CUITradeExplanation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEEXPLANATION_H__DFFA77BD_3013_4389_84CB_EB1DFAD7F3FA__INCLUDED_)
#define AFX_UITRADEEXPLANATION_H__DFFA77BD_3013_4389_84CB_EB1DFAD7F3FA__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUITradeExplanation : public CN3UIBase
{
protected:
	class CN3UIButton* m_pBtn_PageUp;
	class CN3UIButton* m_pBtn_PageDown;
	class CN3UIButton* m_pBtn_Close;
	class CN3UIString* m_pText_Explanation;

	std::string m_szExplanation;
	int m_iCurSel;

public:
	bool OnKeyPress(int iKey) override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void SetExplanation(int iCurSel, const std::string& szExplanation);

	CUITradeExplanation();
	~CUITradeExplanation() override;
};

#endif // !defined(AFX_UITRADEEXPLANATION_H__DFFA77BD_3013_4389_84CB_EB1DFAD7F3FA__INCLUDED_)
