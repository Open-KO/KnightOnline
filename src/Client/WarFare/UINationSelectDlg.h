// UINationSelectDlg.h: interface for the CUINationSelectDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UINATIONSELECTDLG_H__AC80EE20_C90E_4B08_807A_5431F8F23BF7__INCLUDED_)
#define AFX_UINATIONSELECTDLG_H__AC80EE20_C90E_4B08_807A_5431F8F23BF7__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUINationSelectDlg : public CN3UIBase
{
public:
	CN3UIBase* m_pBtnKarus;
	CN3UIBase* m_pBtnElmorad;
	CN3UIBase* m_pBtnBack;
	class CGameProcNationSelect* m_pProcNationSelectRef;

public:
	void Release() override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	CUINationSelectDlg();
	~CUINationSelectDlg() override;
};

#endif // !defined(AFX_UINATIONSELECTDLG_H__AC80EE20_C90E_4B08_807A_5431F8F23BF7__INCLUDED_)
