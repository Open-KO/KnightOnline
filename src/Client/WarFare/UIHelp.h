// UIHelp.h: interface for the CUIHelp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIHELP_H__EFE9F4A4_295F_4A67_B97A_1DF248F1101A__INCLUDED_)
#define AFX_UIHELP_H__EFE9F4A4_295F_4A67_B97A_1DF248F1101A__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUIHelp : public CN3UIBase
{
public:
	static constexpr int MAX_HELP_PAGE = 3;

	CN3UIBase* m_pPages[MAX_HELP_PAGE];

	CN3UIButton* m_pBtn_Prev;
	CN3UIButton* m_pBtn_Next;
	CN3UIButton* m_pBtn_Close;

public:
	CUIHelp();
	~CUIHelp() override;

	void SetVisible(bool bVisible) override;
	bool OnKeyPress(int iKey) override;
	void Release() override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
};

#endif // !defined(AFX_UIHELP_H__EFE9F4A4_295F_4A67_B97A_1DF248F1101A__INCLUDED_)
