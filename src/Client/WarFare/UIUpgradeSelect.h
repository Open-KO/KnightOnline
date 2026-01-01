#pragma once

#include <N3BASE/N3UIBase.h>

class CUIUpgradeSelect : public CN3UIBase
{
public:
	inline void SetNpcID(int iNpcID) {
		m_iNpcID = iNpcID;
	}

	CUIUpgradeSelect();
	~CUIUpgradeSelect() override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void SetVisible(bool bVisible) override;

protected:
	CN3UIButton*	m_pBtn_Upgrade_1;
	CN3UIButton*	m_pBtn_Upgrade_2;
	CN3UIButton*	m_pBtn_Close;
	int				m_iNpcID;
};
