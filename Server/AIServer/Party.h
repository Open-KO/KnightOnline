// Party.h: interface for the CParty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTY_H__4F806B4E_F764_4983_9A47_46254233A7BA__INCLUDED_)
#define AFX_PARTY_H__4F806B4E_F764_4983_9A47_46254233A7BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "PartyUser.h"
//#include "STLMap.h"

//typedef CSTLMap <CPartyUser>			PartyUserArray;
class CServerDlg;

class CParty
{
public:
	//int		m_iSid;				// Party Number
	//short	m_sCurUser;			// 파티 인원 수 
	//short	m_sCurLevel;		// 파티원의 총 레벨 합
	CServerDlg* m_pMain;
	//PartyUserArray	m_arPartyUser;

public:
	CParty();
	virtual ~CParty();

	void Initialize();

	void PartyDelete(char* pBuf);
	void PartyRemove(char* pBuf);
	void PartyInsert(char* pBuf);
	void PartyCreate(char* pBuf);
	void PartyProcess(char* pBuf);

};

#endif // !defined(AFX_PARTY_H__4F806B4E_F764_4983_9A47_46254233A7BA__INCLUDED_)
