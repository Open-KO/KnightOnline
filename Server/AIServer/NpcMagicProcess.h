// NpcMagicProcess.h: interface for the CNpcMagicProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCMAGICPROCESS_H__7C457180_E271_45E6_BEFC_912BCCAB0604__INCLUDED_)
#define AFX_NPCMAGICPROCESS_H__7C457180_E271_45E6_BEFC_912BCCAB0604__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iocport.h"

class CServerDlg;
class CNpc;
struct _MAGIC_TABLE;
class CNpcMagicProcess
{
public:
	CServerDlg*	m_pMain;
	CNpc*		m_pSrcNpc;

	BYTE		m_bMagicState;

public:
	CNpcMagicProcess();
	virtual ~CNpcMagicProcess();

	short GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint);
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

	_MAGIC_TABLE* IsAvailable(int magicid, int tid, BYTE type);
	void MagicPacket(char* pBuf, int len, CIOCPort* pIOCP);

};

#endif // !defined(AFX_NPCMAGICPROCESS_H__7C457180_E271_45E6_BEFC_912BCCAB0604__INCLUDED_)
