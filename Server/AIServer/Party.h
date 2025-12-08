#ifndef SERVER_AISERVER_PARTY_H
#define SERVER_AISERVER_PARTY_H

#pragma once

class AIServerApp;
class CParty
{
public:
	AIServerApp* m_pMain;

public:
	CParty();
	virtual ~CParty();

	void PartyDelete(char* pBuf);
	void PartyRemove(char* pBuf);
	void PartyInsert(char* pBuf);
	void PartyCreate(char* pBuf);
	void PartyProcess(char* pBuf);
};

#endif // SERVER_AISERVER_PARTY_H
