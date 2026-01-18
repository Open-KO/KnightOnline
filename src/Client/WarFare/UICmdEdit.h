// UICmdEdit.h: interface for the UICmdEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__UICMDEDIT_H__)
#define __UICMDEDIT_H__

#pragma once

#include <N3Base/N3UIBase.h>

//////////////////////////////////////////////////////////////////////

class CUICmdEdit : public CN3UIBase
{
public:
	CN3UIString* m_pText_Title;
	CN3UIButton* m_pBtn_Ok;
	CN3UIButton* m_pBtn_Cancel;
	CN3UIEdit* m_pEdit_Box;
	std::string m_szArg1;

public:
	void SetVisible(bool bVisible) override;
	void Open(const std::string& msg);

	void Release() override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	CUICmdEdit();
	~CUICmdEdit() override;
};

#endif //#if !defined(__UICMDEDIT_H__)
