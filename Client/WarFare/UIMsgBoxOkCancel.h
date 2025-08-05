// UIMsgBoxOkCancel.h: interface for the CUIMsgBoxOkCancel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMSGBOXOKCANCEL_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_)
#define AFX_UIMSGBOXOKCANCEL_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3UIWndBase.h"

class CUIMsgBoxOkCancel : public CN3UIBase {
	e_UIWND				m_eCallerWnd;
	e_UIWND_DISTRICT	m_eCallerWndDistrict;
	bool m_bLocked;
public:
	CN3UIButton* m_pBtn_OK;
	CN3UIButton* m_pBtn_Cancel;
	CN3UIString* m_pText_Msg;
	
public:
	CUIMsgBoxOkCancel();
	virtual ~CUIMsgBoxOkCancel() override;
	bool Load(HANDLE hFile) override;
	void Release() override;
	void SetVisible(bool bVisible) override;
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void Close();
	void Open(e_UIWND eUW, e_UIWND_DISTRICT eUD);
	void SetText(const std::string& szMsg);
	bool IsLocked() const { return m_bLocked; }
	e_UIWND	GetCallerWnd() const { return m_eCallerWnd; }
	e_UIWND_DISTRICT GetCallerWndDistrict() const { return m_eCallerWndDistrict; }
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
};

#endif // !defined(AFX_UIMSGBOXOKCANCEL_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_)
