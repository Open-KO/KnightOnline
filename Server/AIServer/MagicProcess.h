// MagicProcess.h: interface for the CMagicProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAGICPROCESS_H__9FE02686_F482_4C68_849A_130DE441D38D__INCLUDED_)
#define AFX_MAGICPROCESS_H__9FE02686_F482_4C68_849A_130DE441D38D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CServerDlg;
class CUser;
class CNpc;
struct _MAGIC_TABLE;
struct _MAGIC_TYPE4;

class CMagicProcess
{
public:
	CServerDlg*	m_pMain;
	CUser*		m_pSrcUser;

	BYTE		m_bMagicState;
public:
	CMagicProcess();
	virtual ~CMagicProcess();

	short GetWeatherDamage(short damage, short attribute);
	void ExecuteType10(int magicid);
	void ExecuteType9(int magicid);
	void ExecuteType8(int magicid);
	void ExecuteType7(int magicid);
	void ExecuteType6(int magicid);
	void ExecuteType5(int magicid);
	void ExecuteType4(int magicid, int sid, int tid, int data1, int data2, int data3, int moral);
	void ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral, int dexpoint, int righthand_damage);
	BYTE ExecuteType2(int magicid, int tid, int data1, int data2, int data3);
	BYTE ExecuteType1(int magicid, int tid, int data1, int data2, int data3, BYTE sequence);	// sequence => type1 or type2
	short GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint, int righthand_damage);
	short AreaAttack(int magictype, int magicid, int moral, int data1, int data2, int data3, int dexpoint, int righthand_damage);
	void  AreaAttackDamage(int magictype, int rx, int rz, int magicid, int moral, int data1, int data2, int data3, int dexpoint, int righthand_damage);

	_MAGIC_TABLE* IsAvailable(int magicid, int tid, BYTE type);
	void MagicPacket(char* pBuf);

};

#endif // !defined(AFX_MAGICPROCESS_H__9FE02686_F482_4C68_849A_130DE441D38D__INCLUDED_)
