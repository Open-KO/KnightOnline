// MagicProcess.h: interface for the CMagicProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAGICPROCESS_H__C39F1966_3F41_47A9_B26A_77F311683A05__INCLUDED_)
#define AFX_MAGICPROCESS_H__C39F1966_3F41_47A9_B26A_77F311683A05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

class CEbenezerDlg;
class CUser;

class CMagicProcess
{
public:
	short GetWeatherDamage(short damage, short attribute);
	void SendType4BuffRemove(short tid, BYTE buff);
	void Type3Cancel(int magicid, short tid);
	void Type4Cancel(int magicid, short  tid);
	bool UserRegionCheck(int sid, int tid, int magicid, int radius, short mousex = 0, short mousez = 0) const;
	short GetMagicDamage(int sid, int tid, int total_hit, int attribute);
	void ExecuteType10(int magicid);
	void ExecuteType9(int magicid);
	void ExecuteType8(int magicid, int sid, int tid, int data1, int data2, int data3);
	void ExecuteType7(int magicid);
	void ExecuteType6(int magicid);
	void ExecuteType5(int magicid, int sid, int tid, int data1, int data2, int data3);
	void ExecuteType4(int magicid, int sid, int tid, int data1, int data2, int data3);
	void ExecuteType3(int magicid, int sid, int tid, int data1, int data2, int data3);
	BYTE ExecuteType2(int magicid, int sid, int tid, int data1, int data2, int data3);
	BYTE ExecuteType1(int magicid, int sid, int tid, int data1, int data2, int data3);

	model::Magic* IsAvailable(int magicid, int tid, int sid, BYTE type, int data1, int data2, int data3);
	void MagicPacket(char* pBuf, int len);

	CMagicProcess();
	virtual ~CMagicProcess();

	CEbenezerDlg*	m_pMain;
	CUser*			m_pSrcUser;

	BYTE	m_bMagicState;
};

#endif // !defined(AFX_MAGICPROCESS_H__C39F1966_3F41_47A9_B26A_77F311683A05__INCLUDED_)
