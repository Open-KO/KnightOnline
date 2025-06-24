// KnightsManager.h: interface for the CKnightsManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KNIGHTSMANAGER_H__B3BA0329_28DF_4E7F_BC19_101D7A69E896__INCLUDED_)
#define AFX_KNIGHTSMANAGER_H__B3BA0329_28DF_4E7F_BC19_101D7A69E896__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUser;
class CEbenezerDlg;
class CKnightsManager
{
public:
	void RecvKnightsAllList(char* pBuf);
	// knight packet
	void SetKnightsUser(int index, char* UserName);
	BOOL ModifyKnightsUser(int index, char* UserName);
	BOOL RemoveKnightsUser(int index, char* UserName);
	BOOL AddKnightsUser(int index, char* UserName);
	void RecvKnightsList(char* pBuf);
	void RecvDestroyKnights(CUser* pUser, char* pBuf);
	void RecvModifyFame(CUser* pUser, char* pBuf, BYTE command);
	void RecvJoinKnights(CUser* pUser, char* pBuf, BYTE command);
	void RecvCreateKnights(CUser* pUser, char* pBuf);
	void ReceiveKnightsProcess(CUser* pUser, char* pBuf, BYTE command);
	void CurrentKnightsMember(CUser* pUser, char* pBuf);
	void AllKnightsMember(CUser* pUser, char* pBuf);
	void AllKnightsList(CUser* pUser, char* pBuf);
	void ModifyKnightsMember(CUser* pUser, char* pBuf, BYTE command);
	void DestroyKnights(CUser* pUser);
	void WithdrawKnights(CUser* pUser, char* pBuf);
	void JoinKnights(CUser* pUser, char* pBuf);
	void JoinKnightsReq(CUser* pUser, char* pBuf);
	int GetKnightsIndex(int nation);
	BOOL IsAvailableName(const char* strname);
	void CreateKnights(CUser* pUser, char* pBuf);
	void PacketProcess(CUser* pUser, char* pBuf);

	CKnightsManager();
	virtual ~CKnightsManager();

	CEbenezerDlg* m_pMain;
//	CDatabase	m_KnightsDB;
private:

};

#endif // !defined(AFX_KNIGHTSMANAGER_H__B3BA0329_28DF_4E7F_BC19_101D7A69E896__INCLUDED_)
