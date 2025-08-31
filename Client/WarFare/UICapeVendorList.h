// UICapeVendorList.h: interface for the CUICapeVendorList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef UICAPEVENDORLIST_H_INCLUDED
#define UICAPEVENDORLIST_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>
#include <N3Base/N3UIButton.h>

class CUICapeVendorList : public CN3UIBase
{
protected:

public:
	CUICapeVendorList();
	~CUICapeVendorList() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;
};

#endif // UICAPEVENDORLIST_H_INCLUDED
