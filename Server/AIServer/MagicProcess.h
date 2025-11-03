// MagicProcess.h: interface for the CMagicProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAGICPROCESS_H__9FE02686_F482_4C68_849A_130DE441D38D__INCLUDED_)
#define AFX_MAGICPROCESS_H__9FE02686_F482_4C68_849A_130DE441D38D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Extern.h"

class AiServerInstance;
class CUser;
class CNpc;

class CMagicProcess
{
public:
	AiServerInstance*	m_pMain;
	CUser*				m_pSrcUser;

	uint8_t				m_bMagicState;
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
	void ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral, int dexpoint, int righthand_damage);
	uint8_t ExecuteType2(int magicid, int tid, int data1, int data2, int data3);
	uint8_t ExecuteType1(int magicid, int tid, int data1, int data2, int data3, uint8_t sequence);	// sequence => type1 or type2
	int16_t GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint, int righthand_damage);
	int16_t AreaAttack(int magictype, int magicid, int moral, int data1, int data2, int data3, int dexpoint, int righthand_damage);
	void  AreaAttackDamage(int magictype, int rx, int rz, int magicid, int moral, int data1, int data2, int data3, int dexpoint, int righthand_damage);

	model::Magic* IsAvailable(int magicid, int tid, uint8_t type);
	void MagicPacket(char* pBuf);

};

#endif // !defined(AFX_MAGICPROCESS_H__9FE02686_F482_4C68_849A_130DE441D38D__INCLUDED_)
