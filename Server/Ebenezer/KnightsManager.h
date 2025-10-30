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
	void RecvKnightsAllList(const char* pBuf);
	// knight packet
	void SetKnightsUser(int knightsId, const char* charId);
	bool ModifyKnightsUser(int knightsId, const char* charId);
	bool RemoveKnightsUser(int knightsId, const char* charId);
	bool AddKnightsUser(int knightsId, const char* charId);
	void RecvKnightsList(const char* pBuf);
	void RecvDestroyKnights(CUser* pUser, const char* pBuf);
	void RecvModifyFame(CUser* pUser, const char* pBuf, uint8_t command);
	void RecvJoinKnights(CUser* pUser, const char* pBuf, uint8_t command);
	void RecvCreateKnights(CUser* pUser, const char* pBuf);
	void ReceiveKnightsProcess(CUser* pUser, const char* pBuf, uint8_t command);
	void CurrentKnightsMember(CUser* pUser, char* pBuf);
	void AllKnightsMember(CUser* pUser, char* pBuf);
	void AllKnightsList(CUser* pUser, char* pBuf);
	void ModifyKnightsMember(CUser* pUser, char* pBuf, uint8_t command);
	void DestroyKnights(CUser* pUser);
	void WithdrawKnights(CUser* pUser, char* pBuf);
	void JoinKnights(CUser* pUser, char* pBuf);
	void JoinKnightsReq(CUser* pUser, char* pBuf);
	int GetKnightsIndex(int nation);
	bool IsAvailableName(const char* strname) const;
	void CreateKnights(CUser* pUser, char* pBuf);
	void PacketProcess(CUser* pUser, char* pBuf);

	CKnightsManager();
	virtual ~CKnightsManager();

	CEbenezerDlg* m_pMain;
//	CDatabase	m_KnightsDB;
private:

};

#endif // !defined(AFX_KNIGHTSMANAGER_H__B3BA0329_28DF_4E7F_BC19_101D7A69E896__INCLUDED_)
