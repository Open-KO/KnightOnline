// UICapeVendorShop.h: interface for the CUICapeVendorShop class.
//
//////////////////////////////////////////////////////////////////////

#ifndef UICAPEVENDORSHOP_H_INCLUDED
#define UICAPEVENDORSHOP_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>
#include <N3Base/N3UIButton.h>


class Packet;
class CUICapeVendorShop : public CN3UIBase
{
protected:
	CN3UIButton* m_pBtnLeft;
	CN3UIButton* m_pBtnRight;
	CN3UIButton* m_pBtnColorUp;
	CN3UIButton* m_pBtnColorDown;
	CN3UIButton* m_pBtnPatternUp;
	CN3UIButton* m_pBtnPatternDown;
	CN3UIButton* m_pBtnCancel;
	CN3UIButton* m_pBtnPurchase;

public:
	CUICapeVendorShop();
	~CUICapeVendorShop() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;

};

#endif // UICAPEVENDORSHOP_H_INCLUDED
