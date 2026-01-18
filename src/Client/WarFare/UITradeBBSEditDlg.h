// UITradeBBSEditDlg.h: interface for the CUITradeBBSEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEBBSEDITDLG_H__3AEA6C66_D30B_421A_BA47_DD907AD094F3__INCLUDED_)
#define AFX_UITRADEBBSEDITDLG_H__3AEA6C66_D30B_421A_BA47_DD907AD094F3__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUITradeBBSEditDlg : public CN3UIBase
{
protected:
	class CN3UIEdit* m_pEditTitle;
	class CN3UIEdit* m_pEditPrice;
	class CN3UIEdit* m_pEditExplanation;
	class CN3UIButton* m_pBtn_Ok;
	class CN3UIButton* m_pBtn_Cancel;

public:
	bool OnKeyPress(int iChar) override;
	void SetVisible(bool bVisible) override;
	void ShowWindow(int iID = -1, CN3UIBase* pParent = nullptr) override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	std::string GetTradeTitle();
	std::string GetTradeExplanation();
	int GetPrice();

	CUITradeBBSEditDlg();
	~CUITradeBBSEditDlg() override;
};

#endif // !defined(AFX_UITRADEBBSEDITDLG_H__3AEA6C66_D30B_421A_BA47_DD907AD094F3__INCLUDED_)
