// UITargetBar.h: interface for the UITargetBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITARGETBAR_H__E572A55F_D4DE_471D_A0E8_CB587043DD5B__INCLUDED_)
#define AFX_UITARGETBAR_H__E572A55F_D4DE_471D_A0E8_CB587043DD5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

class CUITargetBar : public CN3UIBase
{
public:
	class CN3UIProgress*	m_pProgressHP;
	class CN3UIString *		m_pStringID;
	float m_fTimeSendPacketLast; // 이걸 기준으로 1초에 한번씩 타겟의 정보 요청..

public:
	void	UpdateHP(int iHP, int iMaxHP, bool bUpdateImmediately);
	BOOL	SetIDString(const std::string& szID, D3DCOLOR crID);
	
	bool	Load(HANDLE hFile);
	void	Release();
	CUITargetBar();
	virtual ~CUITargetBar();
};

#endif // !defined(AFX_UITARGETBAR_H__E572A55F_D4DE_471D_A0E8_CB587043DD5B__INCLUDED_)
