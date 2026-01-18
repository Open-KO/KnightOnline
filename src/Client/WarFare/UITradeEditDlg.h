// UITradeEditDlg.h: interface for the CUITradeEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEEDITDLG_H__347A4D3E_DC71_4F03_8070_946095EB8120__INCLUDED_)
#define AFX_UITRADEEDITDLG_H__347A4D3E_DC71_4F03_8070_946095EB8120__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

//////////////////////////////////////////////////////////////////////

class CSubProcPerTrade;
class CUITradeEditDlg : public CN3UIBase
{
public:
	CSubProcPerTrade* m_pSubProcPerTrade;
	CN3UIArea* m_pArea;
	CN3UIImage* m_pImageOfIcon;

public:
	CUITradeEditDlg();
	~CUITradeEditDlg() override;

	int GetQuantity();
	void SetQuantity(int iQuantity); // "edit_trade" Edit Control 에서 정수값을 문자열로 세팅한다..

	void Release() override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void Open(bool bCountGold);
	void Close();
};

#endif // !defined(AFX_UITRADEEDITDLG_H__347A4D3E_DC71_4F03_8070_946095EB8120__INCLUDED_)
