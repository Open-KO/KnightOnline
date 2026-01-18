#ifndef CLIENT_WARFARE_UIEXITMENU_H
#define CLIENT_WARFARE_UIEXITMENU_H

#pragma once

#include <N3Base/N3UIBase.h>

class CUIExitMenu : public CN3UIBase
{
public:
	CUIExitMenu();
	~CUIExitMenu() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void ReturnToCharacterSelection();
	bool Load(File& file) override;

protected:
	CN3UIButton* m_pBtn_Chr;
	CN3UIButton* m_pBtn_Option;
	CN3UIButton* m_pBtn_Exit;
	CN3UIButton* m_pBtn_Cancel;
};

#endif // CLIENT_WARFARE_UIEXITMENU_H
