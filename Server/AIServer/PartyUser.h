// PartyUser.h: interface for the CPartyUser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTYUSER_H__3B9E7175_1970_4C1B_BF20_DB6A728AF1AA__INCLUDED_)
#define AFX_PARTYUSER_H__3B9E7175_1970_4C1B_BF20_DB6A728AF1AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPartyUser
{
public:
	char    m_strUserID[MAX_ID_SIZE + 1];	// 캐릭터의 이름
	int		m_iUserId;					// User의 번호
public:

	CPartyUser();
	virtual ~CPartyUser();

};

#endif // !defined(AFX_PARTYUSER_H__3B9E7175_1970_4C1B_BF20_DB6A728AF1AA__INCLUDED_)
