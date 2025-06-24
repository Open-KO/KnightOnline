// DBAgent.h: interface for the CDBAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_)
#define AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "define.h"
#include <vector>

typedef std::vector <_USER_DATA*>			UserDataArray;

class CAujardDlg;
class CDBAgent
{
public:
	BOOL UpdateCouponEvent(const char* accountid, char* charid, char* cpid, int itemid, int count);
	BOOL CheckCouponEvent(const char* accountid);
	BOOL UpdateBattleEvent(const char* charid, int nation);
	void DBProcessNumber(int number);
	void LoadKnightsAllList(int nation);
	BOOL CheckUserData(const char* accountid, const char* charid, int type, int nTimeNumber, int comparedata);
	int AccountLogout(const char* accountid);
	BOOL SetLogInInfo(const char* accountid, const char* charid, const char* serverip, int serverno, const char* clientip, BYTE bInit);
	BOOL LoadKnightsInfo(int index, char* buff, int& buff_index);
	int UpdateWarehouseData(const char* accountid, int uid, int type);
	BOOL LoadWarehouseData(const char* accountid, int uid);
	BOOL UpdateConCurrentUserCount(int serverno, int zoneno, int t_count);
	int LoadKnightsAllMembers(int knightsindex, int start, char* temp_buff, int& buff_index);
	int DeleteKnights(int knightsindex);
	int UpdateKnights(int Type, char* userid, int knightsindex, int domination);
	int CreateKnights(int knightsindex, int nation, char* name, char* chief, int iFlag = 1);
	BOOL GetAllCharID(const char* id, char* char1, char* char2, char* char3, char* char4, char* char5);
	BOOL LoadCharInfo(char* id, char* buff, int& buff_index);
	BOOL NationSelect(char* id, int nation);
	BOOL DeleteChar(int index, char* id, char* charid, char* socno);
	int CreateNewChar(char* accountid, int index, char* charid, int race, int Class, int hair, int face, int str, int sta, int dex, int intel, int cha);
	int	 AccountLogInReq(char* id, char* pw);	// return Nation value
	int UpdateUser(const char* userid, int uid, int type);
	BOOL LoadUserData(char* userid, int uid);
	void ReConnectODBC(CDatabase* m_db, char* strdb, char* strname, char* strpwd);
	BOOL DatabaseInit();
	void MUserInit(int uid);
	CDBAgent();
	virtual ~CDBAgent();

	CDatabase	m_GameDB, m_AccountDB, m_GameDB1, m_AccountDB1;

	UserDataArray	m_UserDataArray;

	CAujardDlg* m_pMain;
};

#endif // !defined(AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_)
