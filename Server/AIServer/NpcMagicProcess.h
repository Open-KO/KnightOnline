#ifndef SERVER_AISERVER_NPCMAGICPROCESS_H
#define SERVER_AISERVER_NPCMAGICPROCESS_H

#pragma once

#include "Extern.h"

class AIServerApp;
class CNpc;
class CNpcMagicProcess
{
public:
	AIServerApp*	m_pMain;
	CNpc*			m_pSrcNpc;
	uint8_t			m_bMagicState;

public:
	CNpcMagicProcess();
	virtual ~CNpcMagicProcess();

	int16_t GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint);
	void ExecuteType10(int magicid);
	void ExecuteType9(int magicid);
	void ExecuteType8(int magicid, int tid, int sid, int data1, int data2, int data3);
	void ExecuteType7(int magicid);
	void ExecuteType6(int magicid);
	void ExecuteType5(int magicid);
	void ExecuteType4(int magicid, int tid);
	void ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral);
	void ExecuteType2(int magicid, int tid, int data1, int data2, int data3);
	void ExecuteType1(int magicid, int tid, int data1, int data2, int data3);	// sequence => type1 or type2

	model::Magic* IsAvailable(int magicid, int tid, uint8_t type);
	void MagicPacket(char* pBuf, int len);
};

#endif // SERVER_AISERVER_NPCMAGICPROCESS_H
