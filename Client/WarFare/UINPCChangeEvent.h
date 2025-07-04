﻿// UINPCChangeEvent.h: interface for the CUINPCChangeEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UINPCCHANGEEVENT_H__01943C6E_D7DD_49B1_BBAF_63DE3B65E586__INCLUDED_)
#define AFX_UINPCCHANGEEVENT_H__01943C6E_D7DD_49B1_BBAF_63DE3B65E586__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIPointInitDlg.h"

#include <N3Base/N3UIBase.h>

//////////////////////////////////////////////////////////////////////

class CUINPCChangeEvent : public CN3UIBase
{
	CN3UIButton*		m_pBtn_Change;
	CN3UIButton*		m_pBtn_Repoint0;
	CN3UIButton*		m_pBtn_Repoint1;
	CN3UIButton*		m_pBtn_Close;

	CUIPointInitDlg*	m_pDlg;

	bool					 m_bSendedAllPoint;

public:
	bool OnKeyPress(int iKey);
	void SetVisible(bool bVisible);
	void Release();

	CUINPCChangeEvent();
	virtual ~CUINPCChangeEvent();

	bool	Load(HANDLE hFile);
	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	void	Open();
	void	Close();

	void	ClassChange();
	void	PointChangePriceQuery(bool bAllPoint);
	void	ReceivePriceFromServer(int iGold);
};

#endif // !defined(AFX_UINPCCHANGEEVENT_H__01943C6E_D7DD_49B1_BBAF_63DE3B65E586__INCLUDED_)
