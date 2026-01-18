// UIClassChange.h: interface for the CUIClassChange class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICLASSCHANGE_H__74E2A304_165D_44C9_88C5_00746B408CCF__INCLUDED_)
#define AFX_UICLASSCHANGE_H__74E2A304_165D_44C9_88C5_00746B408CCF__INCLUDED_

#pragma once

#include "GameDef.h"
#include <N3Base/N3UIBase.h>

class CUIClassChange : public CN3UIBase
{
	CN3UIButton* m_pBtn_Ok;
	CN3UIButton* m_pBtn_Cancel;
	CN3UIButton* m_pBtn_Class;

	CN3UIString* m_pText_Warning;
	CN3UIString* m_pText_Info;
	CN3UIString* m_pText_Title;
	CN3UIString* m_pText_Message;

	e_Class m_eClass;

public:
	CUIClassChange();
	~CUIClassChange() override;

	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void Open(int iCode);
	void Close();

	void RestorePrevClass();
	void ChangeToWarningState();
	void ChangeToNormalState();
};

#endif // !defined(AFX_UICLASSCHANGE_H__74E2A304_165D_44C9_88C5_00746B408CCF__INCLUDED_)
