// CountableItemEditDlg.h: interface for the CCountableItemEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COUNTABLEITEMEDITDLG_H__55E83755_6471_4A3D_84D7_857B0FF88833__INCLUDED_)
#define AFX_COUNTABLEITEMEDITDLG_H__55E83755_6471_4A3D_84D7_857B0FF88833__INCLUDED_

#pragma once

#include "N3UIWndBase.h"

class CCountableItemEditDlg : public CN3UIBase
{
	bool m_bLocked;
	e_UIWND m_eCallerWnd;
	e_UIWND_DISTRICT m_eCallerWndDistrict;
	bool m_bWareGold;

public:
	CN3UIArea* m_pArea;
	CN3UIImage* m_pImageOfIcon;
	CN3UIEdit* m_pEdit;

	CN3UIButton* m_pBtnOk;
	CN3UIButton* m_pBtnCancel;

public:
	bool OnKeyPress(int iKey) override;
	bool Load(File& file) override;
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void SetVisible(bool bVisible) override;
	int GetQuantity();               // "edit_trade" Edit Control 에서 정수값을 얻오온다..
	void SetQuantity(int iQuantity); // "edit_trade" Edit Control 에서 정수값을 문자열로 세팅한다..

	CCountableItemEditDlg();
	~CCountableItemEditDlg() override;

	void Release() override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	virtual void Open(e_UIWND eUW, e_UIWND_DISTRICT eUD, bool bCountGold, bool bWareGold = false);
	virtual void Close();

	bool IsLocked()
	{
		return m_bLocked;
	}

	e_UIWND GetCallerWnd()
	{
		return m_eCallerWnd;
	}
	e_UIWND_DISTRICT GetCallerWndDistrict()
	{
		return m_eCallerWndDistrict;
	}
};

#endif // !defined(AFX_COUNTABLEITEMEDITDLG_H__55E83755_6471_4A3D_84D7_857B0FF88833__INCLUDED_)
