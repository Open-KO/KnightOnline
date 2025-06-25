// DBAgent.cpp: implementation of the CDBAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Aujard.h"
#include "DBAgent.h"
#include "AujardDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern CRITICAL_SECTION g_LogFileWrite;

CDBAgent::CDBAgent()
{
}

CDBAgent::~CDBAgent()
{
}

BOOL CDBAgent::DatabaseInit()
{
	//	Main DB Connecting..
	/////////////////////////////////////////////////////////////////////////////////////
	m_pMain = (CAujardDlg*) AfxGetApp()->GetMainWnd();

	CString strConnect;
	strConnect.Format(_T("ODBC;DSN=%s;UID=%s;PWD=%s"), m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);

	m_GameDB.SetLoginTimeout(10);
	if (!m_GameDB.Open(nullptr, FALSE, FALSE, strConnect))
	{
		AfxMessageBox(_T("GameDB SQL Connection Fail..."));
		return FALSE;
	}

	strConnect.Format(_T("ODBC;DSN=%s;UID=%s;PWD=%s"), m_pMain->m_strAccountDSN, m_pMain->m_strAccountUID, m_pMain->m_strAccountPWD);

	m_AccountDB.SetLoginTimeout(10);

	if (!m_AccountDB.Open(nullptr, FALSE, FALSE, strConnect))
	{
		AfxMessageBox(_T("AccountDB SQL Connection Fail..."));
		return FALSE;
	}

	m_AccountDB1.SetLoginTimeout(10);

	if (!m_AccountDB1.Open(nullptr, FALSE, FALSE, strConnect))
	{
		AfxMessageBox(_T("AccountDB1 SQL Connection Fail..."));
		return FALSE;
	}

	return TRUE;
}

void CDBAgent::ReConnectODBC(CDatabase* m_db, const TCHAR* strdb, const TCHAR* strname, const TCHAR* strpwd)
{
	char strlog[256] = {};
	CTime t = CTime::GetCurrentTime();
	sprintf(strlog, "Try ReConnectODBC... \r\n");
	m_pMain->WriteLogFile(strlog);
	//m_pMain->m_LogFile.Write(strlog, strlen(strlog));

	// DATABASE 연결...
	CString strConnect;
	strConnect.Format(_T("DSN=%s;UID=%s;PWD=%s"), strdb, strname, strpwd);
	int iCount = 0;

	DBProcessNumber(1);

	do
	{
		iCount++;
		if (iCount >= 4)
			break;

		m_db->SetLoginTimeout(10);

		try
		{
			m_db->OpenEx((LPCTSTR) strConnect, CDatabase::noOdbcDialog);
		}
		catch (CDBException* e)
		{
			e->Delete();
		}

	}
	while (!m_db->IsOpen());
}

void CDBAgent::MUserInit(int uid)
{
	_USER_DATA* pUser = m_UserDataArray[uid];
	if (pUser == nullptr)
		return;

	memset(pUser, 0, sizeof(_USER_DATA));
	pUser->m_bAuthority = AUTHORITY_USER;
}

BOOL CDBAgent::LoadUserData(const char* accountid, const char* userid, int uid)
{
	SQLHSTMT		hstmt;
	SQLRETURN		retcode;
	BOOL			retval;
	_USER_DATA*		pUser = nullptr;
	TCHAR			szSQL[1024] = {};

	//wsprintf(szSQL, TEXT("{? = call LOAD_USER_DATA ('%hs')}"), userid);
	wsprintf(szSQL, TEXT("{call LOAD_USER_DATA ('%hs', '%hs', ?)}"), accountid, userid);

	SQLCHAR Nation, Race, HairColor, Rank, Title, Level;
	SQLINTEGER Exp, Loyalty, Gold, PX, PZ, PY, dwTime, MannerPoint, LoyaltyMonthly;
	SQLCHAR Face, City, Fame, Authority, Points;
	SQLSMALLINT Hp, Mp, Sp, sRet, Class, Bind, Knights, QuestCount;
	SQLCHAR Str, Sta, Dex, Intel, Cha, Zone;
	char strSkill[10] = {},
		strItem[400] = {},
		strSerial[400] = {},
		strQuest[400] = {};

	SQLINTEGER Indexind = SQL_NTS;

	hstmt = nullptr;

	DBProcessNumber(2);

	char logstr[256] = {};
	sprintf(logstr, "LoadUserData : name=%s\r\n", userid);
	//m_pMain->m_LogFile.Write(logstr, strlen(logstr));

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode != SQL_SUCCESS)
	{
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData Fail 000 : name=%s\r\n", userid);
	//	m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}

	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &sRet, 0, &Indexind);
	if (retcode != SQL_SUCCESS)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData Fail : name=%s, retcode=%d\r\n", userid, retcode);
	//	m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}

	retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
	if (retcode == SQL_SUCCESS)
	{ //|| retcode == SQL_SUCCESS_WITH_INFO){
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLGetData(hstmt, 1, SQL_C_TINYINT, &Nation, 0, &Indexind);
			SQLGetData(hstmt, 2, SQL_C_TINYINT, &Race, 0, &Indexind);
			SQLGetData(hstmt, 3, SQL_C_SSHORT, &Class, 0, &Indexind);
			SQLGetData(hstmt, 4, SQL_C_TINYINT, &HairColor, 0, &Indexind);
			SQLGetData(hstmt, 5, SQL_C_TINYINT, &Rank, 0, &Indexind);
			SQLGetData(hstmt, 6, SQL_C_TINYINT, &Title, 0, &Indexind);
			SQLGetData(hstmt, 7, SQL_C_TINYINT, &Level, 0, &Indexind);
			SQLGetData(hstmt, 8, SQL_C_LONG, &Exp, 0, &Indexind);
			SQLGetData(hstmt, 9, SQL_C_LONG, &Loyalty, 0, &Indexind);
			SQLGetData(hstmt, 10, SQL_C_TINYINT, &Face, 0, &Indexind);
			SQLGetData(hstmt, 11, SQL_C_TINYINT, &City, 0, &Indexind);
			SQLGetData(hstmt, 12, SQL_C_SSHORT, &Knights, 0, &Indexind);
			SQLGetData(hstmt, 13, SQL_C_TINYINT, &Fame, 0, &Indexind);
			SQLGetData(hstmt, 14, SQL_C_SSHORT, &Hp, 0, &Indexind);
			SQLGetData(hstmt, 15, SQL_C_SSHORT, &Mp, 0, &Indexind);
			SQLGetData(hstmt, 16, SQL_C_SSHORT, &Sp, 0, &Indexind);
			SQLGetData(hstmt, 17, SQL_C_TINYINT, &Str, 0, &Indexind);
			SQLGetData(hstmt, 18, SQL_C_TINYINT, &Sta, 0, &Indexind);
			SQLGetData(hstmt, 19, SQL_C_TINYINT, &Dex, 0, &Indexind);
			SQLGetData(hstmt, 20, SQL_C_TINYINT, &Intel, 0, &Indexind);
			SQLGetData(hstmt, 21, SQL_C_TINYINT, &Cha, 0, &Indexind);
			SQLGetData(hstmt, 22, SQL_C_TINYINT, &Authority, 0, &Indexind);
			SQLGetData(hstmt, 23, SQL_C_TINYINT, &Points, 0, &Indexind);
			SQLGetData(hstmt, 24, SQL_C_LONG, &Gold, 0, &Indexind);
			SQLGetData(hstmt, 25, SQL_C_TINYINT, &Zone, 0, &Indexind);
			SQLGetData(hstmt, 26, SQL_C_SSHORT, &Bind, 0, &Indexind);
			SQLGetData(hstmt, 27, SQL_C_LONG, &PX, 0, &Indexind);
			SQLGetData(hstmt, 28, SQL_C_LONG, &PZ, 0, &Indexind);
			SQLGetData(hstmt, 29, SQL_C_LONG, &PY, 0, &Indexind);
			SQLGetData(hstmt, 30, SQL_C_LONG, &dwTime, 0, &Indexind);
			SQLGetData(hstmt, 31, SQL_C_CHAR, strSkill, 10, &Indexind);
			SQLGetData(hstmt, 32, SQL_C_CHAR, strItem, 400, &Indexind);
			SQLGetData(hstmt, 33, SQL_C_CHAR, strSerial, 400, &Indexind);
			SQLGetData(hstmt, 34, SQL_C_SSHORT, &QuestCount, 0, &Indexind);
			SQLGetData(hstmt, 35, SQL_C_CHAR, strQuest, 400, &Indexind);
			SQLGetData(hstmt, 36, SQL_C_LONG, &MannerPoint, 0, &Indexind);
			SQLGetData(hstmt, 37, SQL_C_LONG, &LoyaltyMonthly, 0, &Indexind);
			retval = TRUE;
		}
		else
		{
			memset(logstr, 0, sizeof(logstr));
			sprintf(logstr, "LoadUserData Fail 222 : name=%s, retcode=%d\r\n", userid, retcode);
	//		m_pMain->m_LogFile.Write(logstr, strlen(logstr));
			retval = FALSE;
		}
	}
	else
	{
		if (DisplayErrorMsg(hstmt) == -1)
		{
			char logstr[256] = {};
			sprintf(logstr, "[Error-DB Fail] LoadUserData : name=%s\r\n", userid);
	//		m_pMain->m_LogFile.Write(logstr, strlen(logstr));

			m_GameDB.Close();

			if (!m_GameDB.IsOpen())
			{
				ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
				return FALSE;
			}
		}
		retval = FALSE;

		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData Fail 333 : name=%s, retcode=%d\r\n", userid, retcode);
	//	m_pMain->m_LogFile.Write(logstr, strlen(logstr));
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	// 엠겜 유저가 아니다.
/*	if(sRet == 0)	{
		memset( logstr, 0x00, 256);
		sprintf( logstr, "LoadUserData Fail : name=%s, sRet= %d, retval=%d, nation=%d \r\n", userid, sRet, retval, Nation );
		m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}	*/

	if (retval == 0)
	{
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData Fail : name=%s, retval= %d \r\n", userid, retval);
	//	m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}

	pUser = (_USER_DATA*) m_UserDataArray[uid];
	if (pUser == nullptr)
	{
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData point is Fail : name=%s, uid= %d \r\n", userid, uid);
	//	m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}
	
	if (strlen(pUser->m_id) != 0)
	{
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData id length is null Fail : name=%s,  \r\n", userid);
	//	m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}

	if (dwTime != 0)
	{
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "[LoadUserData dwTime Error : name=%s, dwTime=%d\r\n", userid, dwTime);
		// m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		TRACE(logstr);
	}

	// 아직 종료되지 않은 유저...
	if (pUser->m_bLogout)
	{
		memset(logstr, 0, sizeof(logstr));
		sprintf(logstr, "LoadUserData logout Fail : name=%s, logout= %d \r\n", userid, pUser->m_bLogout);
		// m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		return FALSE;
	}

	memset(logstr, 0, sizeof(logstr));
	sprintf(logstr, "LoadUserData Success : name=%s\r\n", userid);
	//m_pMain->m_LogFile.Write(logstr, strlen(logstr));

	strcpy(pUser->m_id, userid);

	pUser->m_bZone = Zone;
	pUser->m_curx = (float) (PX / 100);
	pUser->m_curz = (float) (PZ / 100);
	pUser->m_cury = (float) (PY / 100);

	pUser->m_bNation = Nation;
	pUser->m_bRace = Race;
	pUser->m_sClass = Class;
	pUser->m_bHairColor = HairColor;
	pUser->m_bRank = Rank;
	pUser->m_bTitle = Title;
	pUser->m_bLevel = Level;
	pUser->m_iExp = Exp;
	pUser->m_iLoyalty = Loyalty;
	pUser->m_bFace = Face;
	pUser->m_bCity = City;
	pUser->m_bKnights = Knights;
	// 작업 : clan정보를 받아와야 한다
	//pUser->m_sClan = clan;
	pUser->m_bFame = Fame;
	pUser->m_sHp = Hp;
	pUser->m_sMp = Mp;
	pUser->m_sSp = Sp;
	pUser->m_bStr = Str;
	pUser->m_bSta = Sta;
	pUser->m_bDex = Dex;
	pUser->m_bIntel = Intel;
	pUser->m_bCha = Cha;
	pUser->m_bAuthority = Authority;
	pUser->m_bPoints = Points;
	pUser->m_iGold = Gold;
	pUser->m_sBind = Bind;
	pUser->m_dwTime = dwTime + 1;

	pUser->m_iMannerPoint = MannerPoint;
	pUser->m_iLoyaltyMonthly = LoyaltyMonthly;

	CTime t = CTime::GetCurrentTime();
	memset(logstr, 0, sizeof(logstr));
	sprintf(logstr, "[LoadUserData %d-%d-%d]: name=%s, nation=%d, zone=%d, level=%d, exp=%d, money=%d\r\n", t.GetHour(), t.GetMinute(), t.GetSecond(), userid, Nation, Zone, Level, Exp, Gold);
	//m_pMain->m_LogFile.Write(logstr, strlen(logstr));

	int index = 0, serial_index = 0;
	for (int i = 0; i < 9; i++)
		pUser->m_bstrSkill[i] = GetByte(strSkill, index);

	index = 0;
	DWORD itemid = 0;
	short count = 0, duration = 0;
	int64_t serial = 0;
	_ITEM_TABLE* pTable = nullptr;

	// 착용갯수 + 소유갯수(14+28=42)
	for (int i = 0; i < HAVE_MAX + SLOT_MAX; i++)
	{
		itemid = GetDWORD(strItem, index);
		duration = GetShort(strItem, index);
		count = GetShort(strItem, index);

		serial = GetInt64(strSerial, serial_index);		// item serial number

		pTable = m_pMain->m_ItemtableArray.GetData(itemid);

		if (pTable != nullptr)
		{
			pUser->m_sItemArray[i].nNum = itemid;
			pUser->m_sItemArray[i].sDuration = duration;
			pUser->m_sItemArray[i].nSerialNum = serial;
			pUser->m_sItemArray[i].byFlag = 0;
			pUser->m_sItemArray[i].sTimeRemaining = 0;

			if (count > ITEMCOUNT_MAX)
			{
				pUser->m_sItemArray[i].sCount = ITEMCOUNT_MAX;
			}
			else if (pTable->m_bCountable && count <= 0)
			{
				pUser->m_sItemArray[i].nNum = 0;
				pUser->m_sItemArray[i].sDuration = 0;
				pUser->m_sItemArray[i].sCount = 0;
				pUser->m_sItemArray[i].nSerialNum = 0;
			}
			else
			{
				if (count <= 0)
					pUser->m_sItemArray[i].sCount = 1;

				pUser->m_sItemArray[i].sCount = count;
			}

			TRACE(_T("%hs : %d slot (%d : %I64d)\n"), pUser->m_id, i, pUser->m_sItemArray[i].nNum, pUser->m_sItemArray[i].nSerialNum);
			sprintf(logstr, "%s : %d slot (%d : %I64d)\n", pUser->m_id, i, pUser->m_sItemArray[i].nNum, pUser->m_sItemArray[i].nSerialNum);
			//m_pMain->WriteLogFile( logstr );
			//m_pMain->m_LogFile.Write(logstr, strlen(logstr));
		}
		else
		{
			pUser->m_sItemArray[i].nNum = 0;
			pUser->m_sItemArray[i].sDuration = 0;
			pUser->m_sItemArray[i].sCount = 0;
			pUser->m_sItemArray[i].nSerialNum = 0;
			pUser->m_sItemArray[i].byFlag = 0;
			pUser->m_sItemArray[i].sTimeRemaining = 0;

			if (itemid > 0)
			{
				sprintf(logstr, "Item Drop : name=%s, itemid=%d\r\n", userid, itemid);
				m_pMain->WriteLogFile(logstr);
				//m_pMain->m_LogFile.Write(logstr, strlen(logstr));
			}

			continue;
		}
	}

	short sQuestTotal = 0;
	index = 0;
	for (int i = 0; i < MAX_QUEST; i++)
	{
		_USER_QUEST& quest = pUser->m_quests[i];
		quest.sQuestID = GetShort(strQuest, index);
		quest.byQuestState = GetByte(strQuest, index);

		if (quest.sQuestID > 100
			|| quest.byQuestState > 3)
		{
			memset(&quest, 0, sizeof(_USER_QUEST));
			continue;
		}

		if (quest.sQuestID > 0)
			++sQuestTotal;
	}

	if (QuestCount != sQuestTotal)
		pUser->m_sQuestCount = sQuestTotal;

	if (pUser->m_bLevel == 1
		&& pUser->m_iExp == 0
		&& pUser->m_iGold == 0)
	{
		int empty_slot = 0;
		for (int j = SLOT_MAX; j < HAVE_MAX + SLOT_MAX; j++)
		{
			if (pUser->m_sItemArray[j].nNum == 0)
			{
				empty_slot = j;
				break;
			}
		}

		if (empty_slot == HAVE_MAX + SLOT_MAX)
			return retval;

		switch (pUser->m_sClass)
		{
			case 101:
				pUser->m_sItemArray[empty_slot].nNum = 120010000;
				pUser->m_sItemArray[empty_slot].sDuration = 5000;
				break;

			case 102:
				pUser->m_sItemArray[empty_slot].nNum = 110010000;
				pUser->m_sItemArray[empty_slot].sDuration = 4000;
				break;

			case 103:
				pUser->m_sItemArray[empty_slot].nNum = 180010000;
				pUser->m_sItemArray[empty_slot].sDuration = 5000;
				break;

			case 104:
				pUser->m_sItemArray[empty_slot].nNum = 190010000;
				pUser->m_sItemArray[empty_slot].sDuration = 10000;
				break;

			case 201:
				pUser->m_sItemArray[empty_slot].nNum = 120050000;
				pUser->m_sItemArray[empty_slot].sDuration = 5000;
				break;

			case 202:
				pUser->m_sItemArray[empty_slot].nNum = 110050000;
				pUser->m_sItemArray[empty_slot].sDuration = 4000;
				break;

			case 203:
				pUser->m_sItemArray[empty_slot].nNum = 180050000;
				pUser->m_sItemArray[empty_slot].sDuration = 5000;
				break;

			case 204:
				pUser->m_sItemArray[empty_slot].nNum = 190050000;
				pUser->m_sItemArray[empty_slot].sDuration = 10000;
				break;
		}

		pUser->m_sItemArray[empty_slot].sCount = 1;
		pUser->m_sItemArray[empty_slot].nSerialNum = 0;
	}

	return retval;
}

int CDBAgent::UpdateUser(const char* userid, int uid, int type)
{
	SQLHSTMT		hstmt;
	SQLRETURN		retcode;
	TCHAR			szSQL[4096] = {};
	SDWORD			sStrItem, sStrSkill, sStrSerial, sStrQuest;

	_USER_DATA*		pUser = nullptr;

	DBProcessNumber(3);

	pUser = (_USER_DATA*) m_UserDataArray[uid];
	if (pUser == nullptr)
		return -1;

	if (_strnicmp(pUser->m_id, userid, MAX_ID_SIZE) != 0)
		return -1;

	if (type == UPDATE_PACKET_SAVE)
		pUser->m_dwTime++;
	else if (type == UPDATE_LOGOUT
		|| type == UPDATE_ALL_SAVE)
		pUser->m_dwTime = 0;

	char strSkill[10] = {},
		strItem[400] = {},
		strSerial[400] = {},
		strQuest[400] = {};
	short sQuestTotal = 0;
	sStrSkill = sizeof(strSkill);
	sStrItem = sizeof(strItem);
	sStrSerial = sizeof(strSerial);
	sStrQuest = sizeof(strQuest);

	int index = 0, serial_index = 0;
	for (int i = 0; i < 9; i++)
		SetByte(strSkill, pUser->m_bstrSkill[i], index);

	index = 0;
	for (int i = 0; i < MAX_QUEST; i++)
	{
		_USER_QUEST& quest = pUser->m_quests[i];

		if (quest.sQuestID > 100
			|| quest.byQuestState > 3)
		{
			memset(&quest, 0, sizeof(_USER_QUEST));
		}
		else
		{
			if (quest.sQuestID > 0)
				++sQuestTotal;
		}

		SetShort(strQuest, quest.sQuestID, index);
		SetByte(strQuest, quest.byQuestState, index);
	}

	if (sQuestTotal != pUser->m_sQuestCount)
		pUser->m_sQuestCount = sQuestTotal;

	index = 0;

	// 착용갯수 + 소유갯수(14+28=42)
	for (int i = 0; i < HAVE_MAX + SLOT_MAX; i++)
	{
		if (pUser->m_sItemArray[i].nNum > 0)
		{
			if (m_pMain->m_ItemtableArray.GetData(pUser->m_sItemArray[i].nNum) == nullptr)
				TRACE(_T("Item Drop Saved(%d) : %d (%hs)\n"), i, pUser->m_sItemArray[i].nNum, pUser->m_id);
		}

		SetDWORD(strItem, pUser->m_sItemArray[i].nNum, index);
		SetShort(strItem, pUser->m_sItemArray[i].sDuration, index);
		SetShort(strItem, pUser->m_sItemArray[i].sCount, index);

		SetInt64(strSerial, pUser->m_sItemArray[i].nSerialNum, serial_index);
	}

	// 작업 : clan정보도 업데이트
	wsprintf(
		szSQL,
		TEXT("{call UPDATE_USER_DATA ( \'%hs\', %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,?,?,?,?,%d,%d)}"),
		pUser->m_id,
		pUser->m_bNation,
		pUser->m_bRace,
		pUser->m_sClass,
		pUser->m_bHairColor,
		pUser->m_bRank,
		pUser->m_bTitle,
		pUser->m_bLevel,
		pUser->m_iExp,
		pUser->m_iLoyalty,
		pUser->m_bFace,
		pUser->m_bCity,
		pUser->m_bKnights,
		pUser->m_bFame,
		pUser->m_sHp,
		pUser->m_sMp,
		pUser->m_sSp,
		pUser->m_bStr,
		pUser->m_bSta,
		pUser->m_bDex,
		pUser->m_bIntel,
		pUser->m_bCha,
		pUser->m_bAuthority,
		pUser->m_bPoints,
		pUser->m_iGold,
		pUser->m_bZone,
		pUser->m_sBind,
		static_cast<int>(pUser->m_curx * 100),
		static_cast<int>(pUser->m_curz * 100),
		static_cast<int>(pUser->m_cury * 100),
		pUser->m_dwTime,
		sQuestTotal,
		// @strSkill
		// @strItem
		// @strSerial
		// @strQuest
		pUser->m_iMannerPoint,
		pUser->m_iLoyaltyMonthly);

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(strSkill), 0, strSkill, 0, &sStrSkill);
		retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(strItem), 0, strItem, 0, &sStrItem);
		retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(strSerial), 0, strSerial, 0, &sStrSerial);
		retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(strQuest), 0, strQuest, 0, &sStrQuest);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return 0;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

				TCHAR logstr[1024] = {};
				wsprintf(logstr, _T("[Error-DB Fail] %s, Skill[%hs] Item[%hs] \r\n"), szSQL, strSkill, strItem);
				LogFileWrite(logstr);
				return 0;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return 1;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return 0;
}

int CDBAgent::AccountLogInReq(char* id, char* pw)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call ACCOUNT_LOGIN( \'%hs\', \'%hs\', ?)}"), id, pw);

	DBProcessNumber(4);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &sParmRet, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_SUCCESS
				|| retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				if (sParmRet == 0)
					return -1;
				else
					return sParmRet - 1;	// sParmRet == Nation + 1....
			}
			else
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return FALSE;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return FALSE;
			}
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return FALSE;
}

BOOL CDBAgent::NationSelect(char* id, int nation)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call NATION_SELECT ( ?, \'%hs\', %d)}"), id, nation);

	DBProcessNumber(5);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &sParmRet, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return FALSE;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return FALSE;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

			if (sParmRet < 0)
				return FALSE;
			else
				return TRUE;
		}
	}

	return FALSE;
}

int CDBAgent::CreateNewChar(char* accountid, int index, char* charid, int race, int Class, int hair, int face, int str, int sta, int dex, int intel, int cha)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call CREATE_NEW_CHAR ( ?, \'%hs\', %d, \'%hs\', %d,%d,%d,%d,%d,%d,%d,%d,%d)}"), accountid, index, charid, race, Class, hair, face, str, sta, dex, intel, cha);

	DBProcessNumber(6);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &sParmRet, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return -1;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return sParmRet;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return sParmRet;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return -1;
}

BOOL CDBAgent::DeleteChar(int index, char* id, char* charid, char* socno)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{ call DELETE_CHAR ( \'%hs\', %d, \'%hs\', \'%hs\', ? )}"), id, index, charid, socno);

	DBProcessNumber(7);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &sParmRet, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();

					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return FALSE;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return FALSE;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return sParmRet;
		}
	}

	return FALSE;
}

BOOL CDBAgent::LoadCharInfo(char* id, char* buff, int& buff_index)
{
	SQLHSTMT		hstmt;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	BOOL			retval;
	CString			userid;

	userid = id;
	userid.TrimRight();
	strcpy(id, CT2A(userid));

	wsprintf(szSQL, TEXT("{call LOAD_CHAR_INFO ('%hs', ?)}"), id);

	DBProcessNumber(8);

	SQLCHAR Race = 0, HairColor = 0, Level = 0, Face = 0, Zone = 0;
	SQLSMALLINT sRet, Class = 0;
	char strItem[400] = {};

	SQLINTEGER Indexind = SQL_NTS;

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode != SQL_SUCCESS)
		return FALSE;

	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &sRet, 0, &Indexind);
	if (retcode != SQL_SUCCESS)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return FALSE;
	}

	retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
	if (retcode == SQL_SUCCESS
		|| retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLGetData(hstmt, 1, SQL_C_TINYINT, &Race, 0, &Indexind);
			SQLGetData(hstmt, 2, SQL_C_SSHORT, &Class, 0, &Indexind);
			SQLGetData(hstmt, 3, SQL_C_TINYINT, &HairColor, 0, &Indexind);
			SQLGetData(hstmt, 4, SQL_C_TINYINT, &Level, 0, &Indexind);
			SQLGetData(hstmt, 5, SQL_C_TINYINT, &Face, 0, &Indexind);
			SQLGetData(hstmt, 6, SQL_C_TINYINT, &Zone, 0, &Indexind);
			SQLGetData(hstmt, 7, SQL_C_CHAR, strItem, 400, &Indexind);
			retval = TRUE;
		}
		else
		{
			retval = FALSE;
		}
	}
	else
	{
		if (DisplayErrorMsg(hstmt) == -1)
		{
			m_GameDB.Close();

			if (!m_GameDB.IsOpen())
			{
				ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
				return FALSE;
			}
		}

		retval = FALSE;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	SetShort(buff, strlen(id), buff_index);
	SetString(buff, (char*) id, strlen(id), buff_index);
	SetByte(buff, Race, buff_index);
	SetShort(buff, Class, buff_index);
	SetByte(buff, Level, buff_index);
	SetByte(buff, Face, buff_index);
	SetByte(buff, HairColor, buff_index);
	SetByte(buff, Zone, buff_index);

	int tempid = 0, count = 0, index = 0, duration = 0;
	for (int i = 0; i < SLOT_MAX; i++)
	{
		tempid = GetDWORD(strItem, index);
		duration = GetShort(strItem, index);
		count = GetShort(strItem, index);

		if (i == HEAD
			|| i == BREAST
			|| i == SHOULDER
			|| i == LEG
			|| i == GLOVE
			|| i == FOOT
			|| i == LEFTHAND
			|| i == RIGHTHAND)
		{
			SetDWORD(buff, tempid, buff_index);
			SetShort(buff, duration, buff_index);
		}
	}

	return retval;
}

BOOL CDBAgent::GetAllCharID(const char* id, char* char1, char* char2, char* char3, char* char4, char* char5)
{
	SQLHSTMT		hstmt;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	BOOL			retval;
	_USER_DATA*		pUser = nullptr;
	CString			Item;

	wsprintf(szSQL, TEXT("{? = call LOAD_ACCOUNT_CHARID ('%hs')}"), id);

	DBProcessNumber(9);

	SQLSMALLINT sRet;
	char charid1[MAX_ID_SIZE + 1] = {},
		charid2[MAX_ID_SIZE + 1] = {},
		charid3[MAX_ID_SIZE + 1] = {},
		charid4[MAX_ID_SIZE + 1] = {},
		charid5[MAX_ID_SIZE + 1] = {};

	SQLINTEGER Indexind = SQL_NTS;

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode != SQL_SUCCESS)
		return FALSE;

	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &sRet, 0, &Indexind);
	if (retcode != SQL_SUCCESS)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return FALSE;
	}

	retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
	if (retcode == SQL_SUCCESS
		|| retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLGetData(hstmt, 1, SQL_C_CHAR, charid1, MAX_ID_SIZE + 1, &Indexind);
			SQLGetData(hstmt, 2, SQL_C_CHAR, charid2, MAX_ID_SIZE + 1, &Indexind);
			SQLGetData(hstmt, 3, SQL_C_CHAR, charid3, MAX_ID_SIZE + 1, &Indexind);
			SQLGetData(hstmt, 4, SQL_C_CHAR, charid4, MAX_ID_SIZE + 1, &Indexind);
			SQLGetData(hstmt, 5, SQL_C_CHAR, charid5, MAX_ID_SIZE + 1, &Indexind);
			retval = TRUE;
		}
		else
		{
			retval = FALSE;
		}
	}
	else
	{
		if (DisplayErrorMsg(hstmt) == -1)
		{
			m_GameDB.Close();
			if (!m_GameDB.IsOpen())
			{
				ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
				return FALSE;
			}
		}

		retval = FALSE;
	}

	if (sRet == 0)
	{
		retval = FALSE;
	}
	else
	{
		strcpy(char1, charid1);
		strcpy(char2, charid2);
		strcpy(char3, charid3);
		strcpy(char4, charid4);
		strcpy(char5, charid5);
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return retval;
}

int CDBAgent::CreateKnights(int knightsindex, int nation, char* name, char* chief, int iFlag)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet = 0, sKnightIndex = 0;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call CREATE_KNIGHTS ( ?, %d, %d, %d, \'%hs\', \'%hs\' )}"), knightsindex, nation, iFlag, name, chief);
	//wsprintf( szSQL, TEXT( "{call CREATE_KNIGHTS ( ?, ?, %d, %d, \'%hs\', \'%hs\' )}" ), nation, iFlag, name, chief );

	DBProcessNumber(10);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &sParmRet, 0, &cbParmRet);
		//retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0,0, &sKnightIndex,0, &cbParmRet );
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return FALSE;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return sParmRet;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return sParmRet;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return -1;
}

int CDBAgent::UpdateKnights(int type, char* userid, int knightsindex, int domination)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call UPDATE_KNIGHTS ( ?, %d, \'%hs\', %d, %d)}"), (BYTE) type, userid, knightsindex, (BYTE) domination);
	//wsprintf( szSQL, TEXT( "{call UPDATE_KNIGHTS2 ( ?, %d, \'%hs\', %d, %d)}" ), type, userid, knightsindex, domination );

	DBProcessNumber(11);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &sParmRet, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return FALSE;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return sParmRet;
			}

			TRACE(_T("DB - UpdateKnights - command=%d, name=%hs, index=%d, result=%d \n"), type, userid, knightsindex, domination);
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return sParmRet;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return -1;
}

int CDBAgent::DeleteKnights(int knightsindex)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call DELETE_KNIGHTS ( ?, %d )}"), knightsindex);
	//wsprintf( szSQL, TEXT( "{call DELETE_KNIGHTS2 ( ?, %d )}" ), knightsindex );

	DBProcessNumber(12);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &sParmRet, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return -1;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return sParmRet;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return sParmRet;
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return -1;
}

int CDBAgent::LoadKnightsAllMembers(int knightsindex, int start, char* temp_buff, int& buff_index)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL			bData = TRUE;
	int				count = 0, temp_index = 0, sid = 0;
	CString			tempid;
	TCHAR			szSQL[1024] = {};

	SQLCHAR userid[MAX_ID_SIZE + 1] = {};
	char szNewID[MAX_ID_SIZE + 1] = {};
	SQLCHAR Fame, Level;
	SQLSMALLINT	Class;
	SQLINTEGER Indexind = SQL_NTS;
	_USER_DATA* pUser = nullptr;

	//wsprintf( szSQL, TEXT( "SELECT strUserId, Fame, [Level], Class FROM USERDATA WHERE Knights = %d" ), knightsindex );
	wsprintf(szSQL, TEXT("{call LOAD_KNIGHTS_MEMBERS ( %d )}"), knightsindex);

	DBProcessNumber(13);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			while (bData)
			{
				retcode = SQLFetch(hstmt);
				if (retcode == SQL_SUCCESS
					|| retcode == SQL_SUCCESS_WITH_INFO)
				{
					SQLGetData(hstmt, 1, SQL_C_CHAR, userid, MAX_ID_SIZE + 1, &Indexind);
					SQLGetData(hstmt, 2, SQL_C_TINYINT, &Fame, 0, &Indexind);
					SQLGetData(hstmt, 3, SQL_C_TINYINT, &Level, 0, &Indexind);
					SQLGetData(hstmt, 4, SQL_C_SSHORT, &Class, 0, &Indexind);

				//	if( count < start ) {
				//		count++;
				//		continue;
				//	}

					tempid = userid;
					tempid.TrimRight();
					
					strcpy_s(szNewID, CT2A(tempid));

					SetShort(temp_buff, (short) strlen(szNewID), temp_index);
					SetString(temp_buff, szNewID, (int) strlen(szNewID), temp_index);
					SetByte(temp_buff, Fame, temp_index);
					SetByte(temp_buff, Level, temp_index);
					SetShort(temp_buff, Class, temp_index);

					sid = -1;
					//(_USER_DATA*)m_UserDataArray[uid];
					pUser = m_pMain->GetUserPtr((const char*) (LPCTSTR) tempid, sid);
					//pUser = (_USER_DATA*)m_UserDataArray[sid];
					if (pUser != nullptr)
						SetByte(temp_buff, 1, temp_index);
					else
						SetByte(temp_buff, 0, temp_index);

					count++;
					//if( count >= start + 10 )
					//	break;

					bData = TRUE;
				}
				else
				{
					bData = FALSE;
				}
			}
		}
		else
		{
			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_GameDB.Close();
				if (!m_GameDB.IsOpen())
				{
					ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
					return 0;
				}
			}
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	buff_index = temp_index;
	return (count - start);
}

BOOL CDBAgent::UpdateConCurrentUserCount(int serverno, int zoneno, int t_count)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};

	switch (zoneno)
	{
		case 1:
			wsprintf(szSQL, TEXT("UPDATE CONCURRENT SET zone1_count = %d WHERE serverid = %d"), t_count, serverno);
			break;

		case 2:
			wsprintf(szSQL, TEXT("UPDATE CONCURRENT SET zone2_count = %d WHERE serverid = %d"), t_count, serverno);
			break;

		case 3:
			wsprintf(szSQL, TEXT("UPDATE CONCURRENT SET zone3_count = %d WHERE serverid = %d"), t_count, serverno);
			break;
	}

	DBProcessNumber(14);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_AccountDB1.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_ERROR)
		{
			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_AccountDB1.Close();
				if (!m_AccountDB1.IsOpen())
				{
					ReConnectODBC(&m_AccountDB1, m_pMain->m_strAccountDSN, m_pMain->m_strAccountUID, m_pMain->m_strAccountPWD);
					return FALSE;
				}
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return FALSE;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return TRUE;
}

BOOL CDBAgent::LoadWarehouseData(const char* accountid, int uid)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL retval;
	TCHAR			szSQL[1024] = {};

	_USER_DATA* pUser = nullptr;
	_ITEM_TABLE* pTable = nullptr;
	SQLINTEGER	Money = 0, dwTime = 0;
	char strItem[1600] = {}, strSerial[1600] = {};
	SQLINTEGER Indexind = SQL_NTS;

	wsprintf(szSQL, TEXT("SELECT nMoney, dwTime, WarehouseData, strSerial FROM WAREHOUSE WHERE strAccountID = \'%hs\'"), accountid);

	DBProcessNumber(15);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode != SQL_SUCCESS)
		return FALSE;

	retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
	if (retcode == SQL_SUCCESS
		|| retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLGetData(hstmt, 1, SQL_C_LONG, &Money, 0, &Indexind);
			SQLGetData(hstmt, 2, SQL_C_LONG, &dwTime, 0, &Indexind);
			SQLGetData(hstmt, 3, SQL_C_CHAR, strItem, 1600, &Indexind);
			SQLGetData(hstmt, 4, SQL_C_CHAR, strSerial, 1600, &Indexind);
			retval = TRUE;
		}
		else
			retval = FALSE;
	}
	else
	{
		if (DisplayErrorMsg(hstmt) == -1)
		{
			m_GameDB.Close();
			if (!m_GameDB.IsOpen())
			{
				ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
				return FALSE;
			}
		}
		retval = FALSE;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	if (!retval)
		return FALSE;

	pUser = (_USER_DATA*) m_UserDataArray[uid];
	if (pUser == nullptr)
		return FALSE;

	if (strlen(pUser->m_id) == 0)
		return FALSE;

	pUser->m_iBank = Money;

	int index = 0, serial_index = 0;
	DWORD itemid = 0;
	short count = 0, duration = 0;
	int64_t serial = 0;
	for (int i = 0; i < WAREHOUSE_MAX; i++)
	{
		itemid = GetDWORD(strItem, index);
		duration = GetShort(strItem, index);
		count = GetShort(strItem, index);

		serial = GetInt64(strSerial, serial_index);

		pTable = m_pMain->m_ItemtableArray.GetData(itemid);
		if (pTable != nullptr)
		{
			pUser->m_sWarehouseArray[i].nNum = itemid;
			pUser->m_sWarehouseArray[i].sDuration = duration;

			if (count > ITEMCOUNT_MAX)
				pUser->m_sWarehouseArray[i].sCount = ITEMCOUNT_MAX;
			else if (count <= 0)
				count = 1;

			pUser->m_sWarehouseArray[i].sCount = count;
			pUser->m_sWarehouseArray[i].nSerialNum = serial;
			TRACE(_T("%hs : %d ware slot (%d : %I64d)\n"), pUser->m_id, i, pUser->m_sWarehouseArray[i].nNum, pUser->m_sWarehouseArray[i].nSerialNum);
		}
		else
		{
			pUser->m_sWarehouseArray[i].nNum = 0;
			pUser->m_sWarehouseArray[i].sDuration = 0;
			pUser->m_sWarehouseArray[i].sCount = 0;

			if (itemid > 0)
			{
				char logstr[256] = {};
				sprintf(logstr, "Warehouse Item Drop(%d) : %d (%s)\r\n", i, itemid, accountid);
				//m_pMain->WriteLogFile( logstr );
				//m_pMain->m_LogFile.Write(logstr, strlen(logstr));
			}
		}
	}

	return retval;
}

int CDBAgent::UpdateWarehouseData(const char* accountid, int uid, int type)
{
	SQLHSTMT		hstmt;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SDWORD			sStrItem, sStrSerial;

	_USER_DATA* pUser = nullptr;

	pUser = (_USER_DATA*) m_UserDataArray[uid];
	if (pUser == nullptr)
		return -1;

	if (strlen(accountid) == 0)
		return -1;

	if (_strnicmp(pUser->m_Accountid, accountid, MAX_ID_SIZE) != 0)
		return -1;

	if (type == UPDATE_LOGOUT
		|| type == UPDATE_ALL_SAVE)
		pUser->m_dwTime = 0;

	char strItem[1600] = {},
		strSerial[1600] = {};
	sStrItem = sizeof(strItem);
	sStrSerial = sizeof(strSerial);

	int index = 0, serial_index = 0;
	for (int i = 0; i < WAREHOUSE_MAX; i++)
	{
		SetDWORD(strItem, pUser->m_sWarehouseArray[i].nNum, index);
		SetShort(strItem, pUser->m_sWarehouseArray[i].sDuration, index);
		SetShort(strItem, pUser->m_sWarehouseArray[i].sCount, index);

		SetInt64(strSerial, pUser->m_sWarehouseArray[i].nSerialNum, serial_index);
	}

	wsprintf(szSQL, TEXT("{call UPDATE_WAREHOUSE ( \'%hs\', %d,%d,?,?)}"), accountid, pUser->m_iBank, pUser->m_dwTime);

	DBProcessNumber(16);

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(strItem), 0, strItem, 0, &sStrItem);
		retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(strSerial), 0, strSerial, 0, &sStrSerial);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_ERROR)
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_GameDB.Close();
					if (!m_GameDB.IsOpen())
					{
						ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
						return 0;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

				TCHAR logstr[2048] = {};
				wsprintf(logstr, _T("%s, Item[%hs] \r\n"), szSQL, strItem);
				LogFileWrite(logstr);
				return FALSE;
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return 1;
		}
		else
		{
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		}
	}

	return 0;
}

BOOL CDBAgent::LoadKnightsInfo(int index, char* buff, int& buff_index)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL			bData = TRUE, retval = FALSE;
	CString			tempid;
	TCHAR			szSQL[1024] = {};

	SQLCHAR IDName[MAX_ID_SIZE + 1] = {}, Nation;
	char szKnightsName[MAX_ID_SIZE + 1] = {};
	SQLSMALLINT	IDNum, Members;
	SQLINTEGER Indexind = SQL_NTS, nPoints = 0;

	int len = 0;

	wsprintf(szSQL, TEXT("SELECT IDNum, Nation, IDName, Members, Points FROM KNIGHTS WHERE IDNum=%d"), index);

	DBProcessNumber(17);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS
				|| retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLGetData(hstmt, 1, SQL_C_SSHORT, &IDNum, 0, &Indexind);
				SQLGetData(hstmt, 2, SQL_C_TINYINT, &Nation, 0, &Indexind);
				SQLGetData(hstmt, 3, SQL_C_CHAR, IDName, MAX_ID_SIZE + 1, &Indexind);
				SQLGetData(hstmt, 4, SQL_C_SSHORT, &Members, 0, &Indexind);
				SQLGetData(hstmt, 5, SQL_C_LONG, &nPoints, 0, &Indexind);

				tempid = IDName;
				tempid.TrimRight();

				strcpy_s(szKnightsName, CT2A(tempid));

				SetShort(buff, IDNum, buff_index);
				SetByte(buff, Nation, buff_index);
				SetShort(buff, (short) strlen(szKnightsName), buff_index);
				SetString(buff, szKnightsName, (int) strlen(szKnightsName), buff_index);
				SetShort(buff, Members, buff_index);
				SetDWORD(buff, nPoints, buff_index);

				retval = TRUE;
			}

		}
		else
		{
			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_GameDB.Close();
				if (!m_GameDB.IsOpen())
				{
					ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
					return FALSE;
				}
			}
			retval = FALSE;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
	else
	{
		return FALSE;
	}

	return retval;
}

BOOL CDBAgent::SetLogInInfo(const char* accountid, const char* charid, const char* serverip, int serverno, const char* clientip, BYTE bInit)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLINTEGER		cbParmRet = SQL_NTS;
	BOOL			bSuccess = TRUE;

	if (bInit == 0x01)
	{
		wsprintf(szSQL, TEXT("INSERT INTO CURRENTUSER (strAccountID, strCharID, nServerNo, strServerIP, strClientIP) VALUES (\'%hs\',\'%hs\',%d,\'%hs\',\'%hs\')"), accountid, charid, serverno, serverip, clientip);
	}
	else if (bInit == 0x02)
	{
		wsprintf(szSQL, TEXT("UPDATE CURRENTUSER SET nServerNo=%d, strServerIP=\'%hs\' WHERE strAccountID = \'%hs\'"), serverno, serverip, accountid);
	}
	else
	{
		return FALSE;
	}

	DBProcessNumber(18);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_AccountDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_ERROR)
		{
			bSuccess = FALSE;

			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_AccountDB.Close();
				if (!m_AccountDB.IsOpen())
				{
					ReConnectODBC(&m_AccountDB, m_pMain->m_strAccountDSN, m_pMain->m_strAccountUID, m_pMain->m_strAccountPWD);
					return FALSE;
				}
			}
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return bSuccess;
}

int CDBAgent::AccountLogout(const char* accountid, int iLogoutCode)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};
	SQLSMALLINT		sParmRet = 0;
	SQLINTEGER		iUnk = 0;
	SQLINTEGER		cbParmRet = SQL_NTS;

	wsprintf(szSQL, TEXT("{call ACCOUNT_LOGOUT( \'%hs\', %d, ?, ?)}"), accountid, iLogoutCode);

	DBProcessNumber(19);

	CTime t = CTime::GetCurrentTime();
	char strlog[256] = {};
	sprintf(strlog, "[AccountLogout] acname=%s \r\n", accountid);
	m_pMain->WriteLogFile(strlog);
	//m_pMain->m_LogFile.Write(strlog, strlen(strlog));
	TRACE(strlog);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_AccountDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &sParmRet, 0, &cbParmRet);
		retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &iUnk, 0, &cbParmRet);
		if (retcode == SQL_SUCCESS)
		{
			retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
			if (retcode == SQL_SUCCESS
				|| retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				if (sParmRet == 0)
					return -1;
				else
					return sParmRet; // 1 == success
			}
			else
			{
				if (DisplayErrorMsg(hstmt) == -1)
				{
					m_AccountDB.Close();
					if (!m_AccountDB.IsOpen())
					{
						ReConnectODBC(&m_AccountDB, m_pMain->m_strAccountDSN, m_pMain->m_strAccountUID, m_pMain->m_strAccountPWD);
						return FALSE;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return -1;
			}
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return -1;
}

BOOL CDBAgent::CheckUserData(const char* accountid, const char* charid, int type, int nTimeNumber, int comparedata)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL			bData = TRUE, retval = FALSE;
	TCHAR			szSQL[1024] = {};

	SQLINTEGER Indexind = SQL_NTS, dwTime = 0, iData = 0;

	if (type == 1)
		wsprintf(szSQL, TEXT("SELECT dwTime, nMoney FROM WAREHOUSE WHERE strAccountID=\'%hs\'"), accountid);
	else
		wsprintf(szSQL, TEXT("SELECT dwTime, [Exp] FROM USERDATA WHERE strUserID=\'%hs\'"), charid);

	DBProcessNumber(20);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS
				|| retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLGetData(hstmt, 1, SQL_C_LONG, &dwTime, 0, &Indexind);
				SQLGetData(hstmt, 2, SQL_C_LONG, &iData, 0, &Indexind);	// type:1 -> Bank Money type:2 -> Exp

				// check userdata have saved
				if (nTimeNumber != dwTime
					|| comparedata != iData)
					retval = FALSE;
				else
					retval = TRUE;
			}

		}
		else
		{
			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_GameDB.Close();
				if (!m_GameDB.IsOpen())
				{
					ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
					return FALSE;
				}
			}
			retval = FALSE;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
	else
	{
		return FALSE;
	}

	return retval;
}

void CDBAgent::LoadKnightsAllList(int nation)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL			bData = TRUE;
	int				count = 0;
	CString			tempid;
	TCHAR			szSQL[1024] = {};

	char send_buff[512] = {},
		temp_buff[512] = {};
	int send_index = 0, temp_index = 0;

	SQLCHAR bRanking = 0;
	SQLSMALLINT	shKnights = 0;
	SQLINTEGER Indexind = SQL_NTS, nPoints = 0;

	if (nation == 3)	// battle zone
	{
		wsprintf(szSQL, TEXT("SELECT IDNum, Points, Ranking FROM KNIGHTS WHERE Points <> 0 ORDER BY Points DESC"), nation);
	}
	else
	{
		wsprintf(szSQL, TEXT("SELECT IDNum, Points, Ranking FROM KNIGHTS WHERE Nation=%d AND Points <> 0 ORDER BY Points DESC"), nation);
	}

	DBProcessNumber(21);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_SUCCESS
			|| retcode == SQL_SUCCESS_WITH_INFO)
		{
			while (bData)
			{
				retcode = SQLFetch(hstmt);
				if (retcode == SQL_SUCCESS
					|| retcode == SQL_SUCCESS_WITH_INFO)
				{
					SQLGetData(hstmt, 1, SQL_C_SSHORT, &shKnights, 0, &Indexind);
					SQLGetData(hstmt, 2, SQL_C_LONG, &nPoints, 0, &Indexind);
					SQLGetData(hstmt, 3, SQL_C_TINYINT, &bRanking, 0, &Indexind);

					SetShort(temp_buff, shKnights, temp_index);
					SetDWORD(temp_buff, nPoints, temp_index);
					SetByte(temp_buff, bRanking, temp_index);

					count++;

					// 40개 단위로 보낸다
					if (count >= 40)
					{
						SetByte(send_buff, KNIGHTS_ALLLIST_REQ, send_index);
						SetShort(send_buff, -1, send_index);
						SetByte(send_buff, count, send_index);
						SetString(send_buff, temp_buff, temp_index, send_index);

						do
						{
							if (m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index) == 1)
								break;

							count++;
						}
						while (count < 50);

						if (count >= 50)
							m_pMain->m_OutputList.AddString(_T("LoadKnightsAllList Packet Drop!!!"));

						memset(send_buff, 0, sizeof(send_buff));
						memset(temp_buff, 0, sizeof(temp_buff));
						send_index = temp_index = 0;
						count = 0;
					}

					bData = TRUE;
				}
				else
				{
					bData = FALSE;
				}
			}
		}
		else
		{
			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_GameDB.Close();
				if (!m_GameDB.IsOpen())
				{
					ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
					return;
				}
			}
		}

		// 40개를 보내지 못한 경우
		if (count < 40)
		{
			SetByte(send_buff, KNIGHTS_ALLLIST_REQ, send_index);
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, count, send_index);
			SetString(send_buff, temp_buff, temp_index, send_index);

			do
			{
				if (m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index) == 1)
					break;

				count++;
			}
			while (count < 50);

			if (count >= 50)
				m_pMain->m_OutputList.AddString(_T("LoadKnightsAllList Packet Drop!!!"));
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
}

void CDBAgent::DBProcessNumber(int number)
{
	CString strDBNum;
	strDBNum.Format(_T(" %4d "), number);

	m_pMain->GetDlgItem(IDC_DB_PROCESS)->SetWindowText(strDBNum);
	m_pMain->GetDlgItem(IDC_DB_PROCESS)->UpdateWindow();
}

BOOL CDBAgent::UpdateBattleEvent(const char* charid, int nation)
{
	SQLHSTMT		hstmt;
	SQLRETURN		retcode;
	TCHAR			szSQL[1024] = {};

	DBProcessNumber(22);

	wsprintf(szSQL, TEXT("UPDATE BATTLE SET byNation=%d, strUserName=\'%hs\' WHERE sIndex=%d"), nation, charid, 1);

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_GameDB.m_hdbc, &hstmt);
	if (retcode == SQL_SUCCESS)
	{
		retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
		if (retcode == SQL_ERROR)
		{
			if (DisplayErrorMsg(hstmt) == -1)
			{
				m_GameDB.Close();
				if (!m_GameDB.IsOpen())
				{
					ReConnectODBC(&m_GameDB, m_pMain->m_strGameDSN, m_pMain->m_strGameUID, m_pMain->m_strGamePWD);
					return 0;
				}
			}

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return FALSE;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return TRUE;
}

BOOL CDBAgent::CheckCouponEvent(const char* accountid)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL			bData = TRUE, retval = FALSE;
	TCHAR			szSQL[1024] = {};
	SQLINTEGER		Indexind = SQL_NTS;
	SQLSMALLINT		sRet = 0;

	wsprintf(szSQL, TEXT("{call CHECK_COUPON_EVENT (\'%hs\', ?)}"), accountid);

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_AccountDB.m_hdbc, &hstmt);
	if (retcode != SQL_SUCCESS)
		return FALSE;

	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &sRet, 0, &Indexind);
	if (retcode != SQL_SUCCESS)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return FALSE;
	}

	retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
	if (retcode == SQL_SUCCESS
		|| retcode == SQL_SUCCESS_WITH_INFO)
	{
		if (sRet == 0)
			retval = TRUE;
		else
			retval = FALSE;
	}
	else
	{
		if (DisplayErrorMsg(hstmt) == -1)
		{
			m_AccountDB.Close();
			if (!m_AccountDB.IsOpen())
			{
				ReConnectODBC(&m_AccountDB, m_pMain->m_strAccountDSN, m_pMain->m_strAccountUID, m_pMain->m_strAccountPWD);
				return FALSE;
			}
		}

		retval = FALSE;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	return retval;
}

BOOL CDBAgent::UpdateCouponEvent(const char* accountid, char* charid, char* cpid, int itemid, int count)
{
	SQLHSTMT		hstmt = nullptr;
	SQLRETURN		retcode;
	BOOL			bData = TRUE, retval = FALSE;
	TCHAR			szSQL[1024] = {};
	SQLINTEGER		Indexind = SQL_NTS;
	SQLSMALLINT		sRet = 0;

	wsprintf(szSQL, TEXT("{call UPDATE_COUPON_EVENT (\'%hs\', \'%hs\', \'%hs\', %d, %d)}"), accountid, charid, cpid, itemid, count);

	hstmt = nullptr;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_AccountDB.m_hdbc, &hstmt);
	if (retcode != SQL_SUCCESS)
		return FALSE;

	retcode = SQLExecDirect(hstmt, (SQLTCHAR*) szSQL, SQL_NTS);
	if (retcode == SQL_SUCCESS
		|| retcode == SQL_SUCCESS_WITH_INFO)
	{
		retval = TRUE;
		//if( sRet == 1 )	retval = TRUE;
		//else retval = FALSE;
	}
	else
	{
		if (DisplayErrorMsg(hstmt) == -1)
		{
			m_AccountDB.Close();
			if (!m_AccountDB.IsOpen())
			{
				ReConnectODBC(&m_AccountDB, m_pMain->m_strAccountDSN, m_pMain->m_strAccountUID, m_pMain->m_strAccountPWD);
				return FALSE;
			}
		}

		retval = FALSE;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return retval;
}
