// UIMsgBoxOkCancel.h: interface for the CUIMsgBoxOkCancel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMSGBOXOKCANCEL_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_)
#define AFX_UIMSGBOXOKCANCEL_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUIMsgBoxOkCancel : public CN3UIBase
{
public:
	static constexpr int CALLBACK_OK     = 1;
	static constexpr int CALLBACK_CANCEL = 2;

	CUIMsgBoxOkCancel();
	~CUIMsgBoxOkCancel() override;
	bool Load(File& file) override;
	void Release() override;
	void SetText(const std::string& szMsg);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;

protected:
	CN3UIButton* m_pBtn_OK;
	CN3UIButton* m_pBtn_Cancel;
	CN3UIString* m_pText_Msg;
};

#endif // !defined(AFX_UIMSGBOXOKCANCEL_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_)
