#ifndef SERVER_EBENEZER_MAGICPROCESS_H
#define SERVER_EBENEZER_MAGICPROCESS_H

#pragma once

#include "GameDefine.h"

#define NONE_R				0	
#define	FIRE_R				1
#define	COLD_R				2
#define LIGHTNING_R			3
#define MAGIC_R				4
#define DISEASE_R			5
#define POISON_R			6
#define LIGHT_R				7
#define DARKNESS_R			8

class EbenezerApp;
class CUser;
class CMagicProcess
{
public:
	int16_t GetWeatherDamage(int16_t damage, int16_t attribute);
	void SendType4BuffRemove(int tid, uint8_t buff);
	void Type3Cancel(int magicid, int tid);
	void Type4Cancel(int magicid, int tid);
	bool UserRegionCheck(int sid, int tid, int magicid, int radius, int16_t mousex = 0, int16_t mousez = 0) const;
	int16_t GetMagicDamage(int sid, int tid, int total_hit, int attribute);
	void ExecuteType10(int magicid);
	void ExecuteType9(int magicid);
	void ExecuteType8(int magicid, int sid, int tid, int data1, int data2, int data3);
	void ExecuteType7(int magicid);
	void ExecuteType6(int magicid);
	void ExecuteType5(int magicid, int sid, int tid, int data1, int data2, int data3);
	void ExecuteType4(int magicid, int sid, int tid, int data1, int data2, int data3);
	void ExecuteType3(int magicid, int sid, int tid, int data1, int data2, int data3);
	uint8_t ExecuteType2(int magicid, int sid, int tid, int data1, int data2, int data3);
	uint8_t ExecuteType1(int magicid, int sid, int tid, int data1, int data2, int data3);

	model::Magic* IsAvailable(int magicid, int tid, int sid, uint8_t type, int data1, int data2, int data3);
	void MagicPacket(char* pBuf);

	CMagicProcess();
	virtual ~CMagicProcess();

	EbenezerApp*	m_pMain;
	CUser*			m_pSrcUser;
	uint8_t			m_bMagicState;
};

#endif // SERVER_EBENEZER_MAGICPROCESS_H
