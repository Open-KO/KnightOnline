// UITradeBBSEditDlg.h: interface for the CUITradeBBSEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEBBSEDITDLG_H__3AEA6C66_D30B_421A_BA47_DD907AD094F3__INCLUDED_)
#define AFX_UITRADEBBSEDITDLG_H__3AEA6C66_D30B_421A_BA47_DD907AD094F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

class CUITradeBBSEditDlg : public CN3UIBase
{
protected:
	class CN3UIEdit*		m_pEditTitle;
	class CN3UIEdit*		m_pEditPrice;
	class CN3UIEdit*		m_pEditExplanation;
	class CN3UIButton*		m_pBtn_Ok;
	class CN3UIButton*		m_pBtn_Cancel;

public:
	bool OnKeyPress(int iChar);
	void SetVisible(bool bVisible);
	void ShowWindow(int iID = -1, CN3UIBase* pParent = NULL);
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	std::string GetTradeTitle();
	std::string GetTradeExplanation();
	int			GetPrice();

	CUITradeBBSEditDlg();
	virtual ~CUITradeBBSEditDlg();

};

#endif // !defined(AFX_UITRADEBBSEDITDLG_H__3AEA6C66_D30B_421A_BA47_DD907AD094F3__INCLUDED_)
