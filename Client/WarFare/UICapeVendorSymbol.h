// UICapeVendorSymbol.h: interface for the CUICapeVendorSymbol and CUICapeVendorSymbolPreview class.
//
//////////////////////////////////////////////////////////////////////

#ifndef UICAPEVENDORSYMBOL_H_INCLUDED
#define UICAPEVENDORSYMBOL_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>
#include <N3Base/N3UIButton.h>


class CUICapeVendorSymbol : public CN3UIBase
{
protected:

public:
	CUICapeVendorSymbol();
	~CUICapeVendorSymbol() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;

};

class CUICapeVendorPreview : public CN3UIBase
{
protected:

public:
	CUICapeVendorPreview();
	~CUICapeVendorPreview() override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;

};

#endif // UICAPEVENDORSYMBOL_H_INCLUDED
