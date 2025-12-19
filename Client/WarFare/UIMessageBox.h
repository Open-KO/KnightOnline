// UIMessageBox.h: interface for the CUIMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMESSAGEBOX_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_)
#define AFX_UIMESSAGEBOX_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>
#include "GameDef.h"

#define MB_CANCEL                 0x00000006L

class CUIMessageBox : public CN3UIBase  
{
protected:
	CN3UIButton*	m_pBtn_OK;
	CN3UIButton*	m_pBtn_Yes;
	CN3UIButton*	m_pBtn_No;
	CN3UIButton*	m_pBtn_Cancel;
	CN3UIString*	m_pText_Message;
	CN3UIString*	m_pText_Title;
	CN3UIEdit*		m_pEdit_Common;

public:
	e_Behavior	m_eBehavior;	// OK 버튼을 누르면 할짓...
	int			m_iStyle;		// 메시지 박스 스타일

public:
	bool OnKeyPress(int iKey) override;
	void SetVisible(bool bVisible) override;
	void SetBoxStyle(int iStyle);
	void SetVisibleEditControl(bool bVisible); // Edit Control Visible
	void SetTitle(const std::string& szTitle);
	void SetText(const std::string& szMsg);
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	
	CUIMessageBox();
	~CUIMessageBox() override;
};

#endif // !defined(AFX_UIMESSAGEBOX_H__943941D4_06D0_40A0_BEF2_DA3A27406EDC__INCLUDED_)
