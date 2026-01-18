// UINPCChangeEvent.h: interface for the CUINPCChangeEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UINPCCHANGEEVENT_H__01943C6E_D7DD_49B1_BBAF_63DE3B65E586__INCLUDED_)
#define AFX_UINPCCHANGEEVENT_H__01943C6E_D7DD_49B1_BBAF_63DE3B65E586__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUIPointInitDlg;
class CUINPCChangeEvent : public CN3UIBase
{
	CN3UIButton* m_pBtn_Repoint0;
	CN3UIButton* m_pBtn_Repoint1;
	CN3UIButton* m_pBtn_Close;

	CUIPointInitDlg* m_pDlg;

	bool m_bSendedAllPoint;

public:
	bool OnKeyPress(int iKey) override;
	void SetVisible(bool bVisible) override;
	void Release() override;

	CUINPCChangeEvent();
	~CUINPCChangeEvent() override;

	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void Open();
	void Close();

	void PointChangePriceQuery(bool bAllPoint);
	void ReceivePriceFromServer(int iGold);
};

#endif // !defined(AFX_UINPCCHANGEEVENT_H__01943C6E_D7DD_49B1_BBAF_63DE3B65E586__INCLUDED_)
