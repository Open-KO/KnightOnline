// UIPointInitDlg.h: interface for the UIPointInitDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPOINTINITDLG_H__D784EB22_FE0A_4A62_83FF_4664854DE2EC__INCLUDED_)
#define AFX_UIPOINTINITDLG_H__D784EB22_FE0A_4A62_83FF_4664854DE2EC__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

//////////////////////////////////////////////////////////////////////

class CUIPointInitDlg : public CN3UIBase
{
	CN3UIButton* m_pBtn_Ok;
	CN3UIButton* m_pBtn_Cancel;

	CN3UIString* m_pText_NeedGold;
	bool m_bAllpoint;

public:
	CUIPointInitDlg();
	~CUIPointInitDlg() override;

	void InitDlg(bool bAllpoint, int iGold);
	bool OnKeyPress(int iKey) override;
	void Release() override;

	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Close();

	void PushOkButton();
};

#endif // !defined(AFX_UIPOINTINITDLG_H__D784EB22_FE0A_4A62_83FF_4664854DE2EC__INCLUDED_)
