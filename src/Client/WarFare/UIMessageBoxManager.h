// UIMessageBoxManager.h: interface for the CUIMessageBoxManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMESSAGEBOXMANAGER_H__34454EC5_5C0E_42EE_8321_6C0ECC0EACD0__INCLUDED_)
#define AFX_UIMESSAGEBOXMANAGER_H__34454EC5_5C0E_42EE_8321_6C0ECC0EACD0__INCLUDED_

#pragma once

#include <map>

#include "UIMessageBox.h"
#include "GameBase.h"

typedef std::map<std::string, CUIMessageBox*>::iterator it_UBM;
typedef std::map<std::string, CUIMessageBox*>::value_type val_UBM;
typedef std::map<std::string, CUIMessageBox*>::reverse_iterator rit_UBM;

class CUIMessageBoxManager : public CGameBase
{
public:
	std::map<std::string, CUIMessageBox*> m_UBMs;
	CUIMessageBox* m_pMsgBoxLatestRef;

public:
	CUIMessageBox* ReFocusMsgBox();

	CUIMessageBox* GetFocusMsgBox()
	{
		return m_pMsgBoxLatestRef;
	}

	void Release() override;
	uint32_t MouseProcAndTick(uint32_t& dwFlags, const POINT& ptCur, const POINT& ptOld);
	void MessageBoxCloseAll();
	void Render();
	int GetCount() const;

	void MessageBoxClose(const std::string& szMsg);
	std::string MessageBoxPost(const std::string& szMsg, const std::string& szTitle, int iStyle, e_Behavior eBehavior = BEHAVIOR_NOTHING);

	CUIMessageBoxManager();
	~CUIMessageBoxManager() override;
};

#endif // !defined(AFX_UIMESSAGEBOXMANAGER_H__34454EC5_5C0E_42EE_8321_6C0ECC0EACD0__INCLUDED_)
