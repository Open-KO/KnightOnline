﻿#pragma once

#include "N3UIBase.h"

class CUIExitMenu : public CN3UIBase
{
public:
	CUIExitMenu();
	~CUIExitMenu() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void ReturnToCharacterSelection();
	bool Load(HANDLE hFile) override;

protected:
	CN3UIButton* m_pBtn_Chr;
	CN3UIButton* m_pBtn_Option;
	CN3UIButton* m_pBtn_Exit;
	CN3UIButton* m_pBtn_Cancel;
};
