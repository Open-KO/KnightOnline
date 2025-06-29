// UITradeList.h: interface for the CUITradeList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADELIST_H__B4807FE6_499A_47DE_BEA0_642E52708F0D__INCLUDED_)
#define AFX_UITRADELIST_H__B4807FE6_499A_47DE_BEA0_642E52708F0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

//////////////////////////////////////////////////////////////////////

class CUITradeList : public CN3UIBase
{
	CN3UIString*		m_pStr_List[9];
	std::string			m_sStr[40];
	CN3UIScrollBar*		m_pScrollbar;	// scrollbar 참조포인터(실제 m_Child로 관리)

public:
	CUITradeList();
	virtual ~CUITradeList();

	void Release();

	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	void Open(int iIDTarget = -1);
	void Close();

	void SetTopLine(int iTopLine);		// 맨 윗줄을 지정해준다.
};

#endif // !defined(AFX_UITRADELIST_H__B4807FE6_499A_47DE_BEA0_642E52708F0D__INCLUDED_)
