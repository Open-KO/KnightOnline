#ifndef SERVER_AISERVER_MAGICPROCESS_H
#define SERVER_AISERVER_MAGICPROCESS_H

#pragma once

#include "Extern.h"

namespace AIServer
{

class AIServerApp;
class CUser;
class CNpc;
class CMagicProcess
{
public:
	AIServerApp* m_pMain;
	CUser* m_pSrcUser;

public:
	CMagicProcess();
	virtual ~CMagicProcess();

	int16_t GetWeatherDamage(int16_t damage, int16_t attribute);
	void ExecuteType10(int magicid);
	void ExecuteType9(int magicid);
	void ExecuteType8(int magicid);
	void ExecuteType7(int magicid, int tid, int data1, int data2, int data3, int moral);
	void ExecuteType6(int magicid);
	void ExecuteType5(int magicid);
	void ExecuteType4(int magicid, int sid, int tid, int data1, int data2, int data3, int moral);
	void ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral,
		int dexpoint, int righthand_damage);
	uint8_t ExecuteType2(int magicid, int tid, int data1, int data2, int data3);
	uint8_t ExecuteType1(int magicid, int tid, int data1, int data2, int data3,
		uint8_t sequence); // sequence => type1 or type2
	void SendMagicAttackResult(uint8_t opcode, int magicId, int sourceId, int targetId,
		int data1 = 0, int data2 = 0, int data3 = 0, int data4 = 0, int data5 = 0, int data6 = 0,
		int data7 = 0);
	int16_t GetMagicDamage(
		int tid, int total_hit, int attribute, int dexpoint, int righthand_damage);
	int16_t AreaAttack(int magictype, int magicid, int moral, int data1, int data2, int data3,
		int dexpoint, int righthand_damage);
	void AreaAttackDamage(int magictype, int rx, int rz, int magicid, int moral, int data1,
		int data2, int data3, int dexpoint, int righthand_damage);

	model::Magic* IsAvailable(int magicid, int tid, uint8_t type);
	void MagicPacket(char* pBuf);
};

} // namespace AIServer

#endif // SERVER_AISERVER_MAGICPROCESS_H
