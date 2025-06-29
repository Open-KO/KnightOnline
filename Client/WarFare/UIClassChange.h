// UIClassChange.h: interface for the CUIClassChange class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICLASSCHANGE_H__74E2A304_165D_44C9_88C5_00746B408CCF__INCLUDED_)
#define AFX_UICLASSCHANGE_H__74E2A304_165D_44C9_88C5_00746B408CCF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameDef.h"

#include <N3Base/N3UIBase.h>

enum e_ClassChangeState {	UISTATE_NORMAL = 1, UISTATE_WARNING	};

//////////////////////////////////////////////////////////////////////

class CUIClassChange : public CN3UIBase  
{
	CN3UIButton*		m_pBtn_Ok;
	CN3UIButton*		m_pBtn_Cancel;
	CN3UIButton*		m_pBtn_Class0;
	CN3UIButton*		m_pBtn_Class1;
	
	CN3UIString*		m_pText_Warning;
	CN3UIString*		m_pText_Info0;
	CN3UIString*		m_pText_Info1;
	CN3UIString*		m_pText_Title;
	CN3UIString*		m_pText_Message;

	e_Class				m_eClass;
	e_ClassChangeState	m_eClassChangeState;

public:
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false);
	void Release();

	CUIClassChange();
	virtual ~CUIClassChange();

	bool	Load(HANDLE hFile);
	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	void	Open(int iCode);
	void	Close();

	void	RestorePrevClass();
	void	ChangeToWarningState();
	void	ChangeToNormalState();
};

#endif // !defined(AFX_UICLASSCHANGE_H__74E2A304_165D_44C9_88C5_00746B408CCF__INCLUDED_)
