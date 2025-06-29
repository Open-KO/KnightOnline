// UIHelp.h: interface for the CUIHelp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIHELP_H__EFE9F4A4_295F_4A67_B97A_1DF248F1101A__INCLUDED_)
#define AFX_UIHELP_H__EFE9F4A4_295F_4A67_B97A_1DF248F1101A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

const int MAX_HELP_PAGE = 4;

class CUIHelp : public CN3UIBase  
{
public:
	CN3UIBase* m_pPages[MAX_HELP_PAGE];

	CN3UIBase* m_pBtn_Prev;
	CN3UIBase* m_pBtn_Next;
	CN3UIBase* m_pBtn_Close;

public:
	void SetVisible(bool bVisible);
	bool	OnKeyPress(int iKey);
	void	Release();
	bool	Load(HANDLE hFile);
	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	CUIHelp();
	virtual ~CUIHelp();
};

#endif // !defined(AFX_UIHELP_H__EFE9F4A4_295F_4A67_B97A_1DF248F1101A__INCLUDED_)
