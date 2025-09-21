// AujardDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Aujard.h"
#include "AujardDlg.h"

#include <process.h>
#include <shared/Ini.h>
#include <shared/StringConversion.h>
#include <db-library/ConnectionManager.h>

#include <spdlog/spdlog.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NOTE: Explicitly handled under DEBUG_NEW override
#include <db-library/RecordSetLoader_STLMap.h>

constexpr int PROCESS_CHECK		= 100;
constexpr int CONCURRENT_CHECK	= 200;
constexpr int SERIAL_TIME		= 300;
constexpr int PACKET_CHECK		= 400;
constexpr int DB_POOL_CHECK		= 500;

import AujardBinder;
import AujardModel;
namespace model = aujard_model;

WORD g_increase_serial = 50001;

CAujardDlg* CAujardDlg::_instance = nullptr;

DWORD WINAPI ReadQueueThread(LPVOID lp)
{
	CAujardDlg* main = (CAujardDlg*) lp;
	int recvLen = 0, index = 0;
	BYTE command;
	char recvBuff[1024] = {};

	while (TRUE)
	{
		if (main->LoggerRecvQueue.GetFrontMode() != R)
		{
			index = 0;
			recvLen = main->LoggerRecvQueue.GetData(recvBuff);
			if (recvLen > MAX_PKTSIZE)
			{
				Sleep(1);
				continue;
			}

			command = GetByte(recvBuff, index);
			switch (command)
			{
				case WIZ_LOGIN:
					main->AccountLogIn(recvBuff + index);
					break;

				case WIZ_NEW_CHAR:
					main->CreateNewChar(recvBuff + index);
					break;

				case WIZ_DEL_CHAR:
					main->DeleteChar(recvBuff + index);
					break;

				case WIZ_SEL_CHAR:
					main->SelectCharacter(recvBuff + index);
					break;

				case WIZ_SEL_NATION:
					main->SelectNation(recvBuff + index);
					break;

				case WIZ_ALLCHAR_INFO_REQ:
					main->AllCharInfoReq(recvBuff + index);
					break;

				case WIZ_LOGOUT:
					main->UserLogOut(recvBuff + index);
					break;

				case WIZ_DATASAVE:
					main->UserDataSave(recvBuff + index);
					break;

				case WIZ_KNIGHTS_PROCESS:
					main->KnightsPacket(recvBuff + index);
					break;

				case WIZ_CLAN_PROCESS:
					main->KnightsPacket(recvBuff + index);
					break;

				case WIZ_LOGIN_INFO:
					main->SetLogInInfo(recvBuff + index);
					break;

				case WIZ_KICKOUT:
					main->UserKickOut(recvBuff + index);
					break;

				case WIZ_BATTLE_EVENT:
					main->BattleEventResult(recvBuff + index);
					break;

				case DB_COUPON_EVENT:
					main->CouponEvent(recvBuff + index);
					break;
			}

			recvLen = 0;
			memset(recvBuff, 0, sizeof(recvBuff));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAujardDlg dialog

CAujardDlg::CAujardDlg(CWnd* parent /*=nullptr*/)
	: CDialog(IDD, parent),
	_logger(logger::Aujard)
{
	//{{AFX_DATA_INIT(CAujardDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	_icon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	_sendPacketCount = 0;
	_packetCount = 0;
	_recvPacketCount = 0;

	db::ConnectionManager::DefaultConnectionTimeout = DB_PROCESS_TIMEOUT;
	db::ConnectionManager::Create();
}

CAujardDlg::~CAujardDlg()
{
	db::ConnectionManager::Destroy();
}

/// \brief performs MFC data exchange
/// \see https://learn.microsoft.com/en-us/cpp/mfc/dialog-data-exchange?view=msvc-170
void CAujardDlg::DoDataExchange(CDataExchange* data)
{
	CDialog::DoDataExchange(data);
	//{{AFX_DATA_MAP(CAujardDlg)
	DDX_Control(data, IDC_OUT_LIST, _outputList);
	DDX_Control(data, IDC_DB_PROCESS, DBProcessNum);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAujardDlg, CDialog)
	//{{AFX_MSG_MAP(CAujardDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAujardDlg message handlers

BOOL CAujardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_instance = this;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(_icon, TRUE);			// Set big icon
	SetIcon(_icon, FALSE);		// Set small icon

	CString exePath(GetProgPath());
	std::string exePathUtf8(CT2A(exePath, CP_UTF8));

	std::filesystem::path iniPath(exePath.GetString());
	iniPath /= L"Aujard.ini";

	CIni ini(iniPath);

	// configure logger
	_logger.Setup(ini, exePathUtf8);

	LoggerRecvQueue.InitailizeMMF(MAX_PKTSIZE, MAX_COUNT, _T(SMQ_LOGGERSEND), FALSE);	// Dispatcher 의 Send Queue
	LoggerSendQueue.InitailizeMMF(MAX_PKTSIZE, MAX_COUNT, _T(SMQ_LOGGERRECV), FALSE);	// Dispatcher 의 Read Queue

	if (!InitSharedMemory())
	{
		AfxMessageBox(_T("Main Shared Memory Initialize Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}
	
	// TODO: This should be Knight_Account
	// TODO: This should only fetch the once.
	// The above won't be necessary after stored procedures are replaced, so it can be replaced then.
	std::string datasourceName, datasourceUser, datasourcePass;

	// TODO: This should be Knight_Account
	datasourceName = ini.GetString(ini::ODBC, ini::ACCOUNT_DSN, "KN_online");
	datasourceUser = ini.GetString(ini::ODBC, ini::ACCOUNT_UID, "knight");
	datasourcePass = ini.GetString(ini::ODBC, ini::ACCOUNT_PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::ACCOUNT,
		datasourceName, datasourceUser, datasourcePass);

	datasourceName = ini.GetString(ini::ODBC, ini::GAME_DSN, "KN_online");
	datasourceUser = ini.GetString(ini::ODBC, ini::GAME_UID, "knight");
	datasourcePass = ini.GetString(ini::ODBC, ini::GAME_PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	_serverId = ini.GetInt(ini::ZONE_INFO, ini::GROUP_INFO, 1);
	_zoneId = ini.GetInt(ini::ZONE_INFO, ini::ZONE_INFO, 1);

	// Trigger a save to flush defaults to file.
	ini.Save();

	if (!_dbAgent.InitDatabase())
	{
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!LoadItemTable())
	{
		AfxMessageBox(_T("Load ItemTable Fail!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	SetTimer(PROCESS_CHECK, 40000, nullptr);
	SetTimer(CONCURRENT_CHECK, 300000, nullptr);
//	SetTimer( SERIAL_TIME, 60000, nullptr );
	SetTimer(PACKET_CHECK, 120000, nullptr);
	SetTimer(DB_POOL_CHECK, 60000, nullptr);

	DWORD id;
	_readQueueThread = ::CreateThread(nullptr, 0, ReadQueueThread, this, 0, &id);

	spdlog::info("AujardDlg::OnInitDialog: initialized");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAujardDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, _icon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

/// \brief The system calls this to obtain the cursor to display while the user drags
///  the minimized window.
HCURSOR CAujardDlg::OnQueryDragIcon()
{
	return (HCURSOR) _icon;
}

/// \brief destroys all resources associated with the dialog window
BOOL CAujardDlg::DestroyWindow()
{
	KillTimer(PROCESS_CHECK);
	KillTimer(CONCURRENT_CHECK);
//	KillTimer(SERIAL_TIME);
	KillTimer(PACKET_CHECK);
	KillTimer(DB_POOL_CHECK);

	if (_readQueueThread != nullptr)
		::TerminateThread(_readQueueThread, 0);

	if (!ItemArray.IsEmpty())
		ItemArray.DeleteAllData();
	
	_instance = nullptr;

	return CDialog::DestroyWindow();
}

/// \brief initializes shared memory with other server applications
BOOL CAujardDlg::InitSharedMemory()
{
	DWORD filesize = MAX_USER * ALLOCATED_USER_DATA_BLOCK;

	_sharedMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, _T("KNIGHT_DB"));
	if (_sharedMemoryHandle == nullptr)
	{
		_sharedMemoryHandle = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	AddOutputMessage(_T("Shared memory loaded successfully"));
	spdlog::info("AujardDlg::InitSharedMemory: shared memory loaded successfully");

	_sharedMemoryFile = (char*) MapViewOfFile(_sharedMemoryHandle, FILE_MAP_WRITE, 0, 0, 0);
	if (_sharedMemoryFile == nullptr)
		return FALSE;

	_dbAgent.UserData.reserve(MAX_USER);

	for (int i = 0; i < MAX_USER; i++)
	{
		_USER_DATA* pUser = (_USER_DATA*) (_sharedMemoryFile + i * ALLOCATED_USER_DATA_BLOCK);
		_dbAgent.UserData.push_back(pUser);
	}

	return TRUE;
}

/// \brief writes a recordset_loader::Error to an error pop-up
void CAujardDlg::ReportTableLoadError(const recordset_loader::Error& err, const char* source)
{
	std::string error = fmt::format("AujardDlg::ReportTableLoadError: {} failed: {}",
		source, err.Message);
	std::wstring werror = LocalToWide(error);
	AfxMessageBox(werror.c_str());
	spdlog::error(error);
}

/// \brief loads information needed from the ITEM table to a cache map
BOOL CAujardDlg::LoadItemTable()
{
	recordset_loader::STLMap loader(ItemArray);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return FALSE;
	}

	return TRUE;
}

/// \brief loads and sends data after a character is selected
void CAujardDlg::SelectCharacter(char* buffer)
{
	int index = 0, userId = -1, sendIndex = 0, idLen1 = 0, idLen2 = 0, tempUserId = -1,
		retryCount = 0, packetIndex = 0, maxRetry = 50;
	BYTE init = 0x01;
	char sendBuff[256] = {},
		accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {};

	_USER_DATA* user = nullptr;

	userId = GetShort(buffer, index);
	idLen1 = GetShort(buffer, index);
	GetString(accountId, buffer, idLen1, index);
	idLen2 = GetShort(buffer, index);
	GetString(charId, buffer, idLen2, index);
	init = GetByte(buffer, index);
	packetIndex = GetDWORD(buffer, index);

	spdlog::debug("AujardDlg::SelectCharacter: acctId={}, charId={}, index={}, pid={}, front={}",
		accountId, charId, packetIndex, _getpid(), LoggerRecvQueue.GetFrontPointer());
	
	_recvPacketCount++;		// packet count

	if (userId < 0
		|| userId >= MAX_USER)
		goto fail_return;

	if (strlen(accountId) == 0)
		goto fail_return;

	if (strlen(charId) == 0)
		goto fail_return;

	if (GetUserPtr(charId, tempUserId) != nullptr)
	{
		SetShort(sendBuff, tempUserId, sendIndex);
		SetShort(sendBuff, idLen1, sendIndex);
		SetString(sendBuff, accountId, idLen1, sendIndex);
		SetShort(sendBuff, idLen2, sendIndex);
		SetString(sendBuff, charId, idLen2, sendIndex);
		UserLogOut(sendBuff);
		return;
	}

	if (!_dbAgent.LoadUserData(accountId, charId, userId))
		goto fail_return;

	if (!_dbAgent.LoadWarehouseData(accountId, userId))
		goto fail_return;

	user = _dbAgent.UserData[userId];
	if (user == nullptr)
		goto fail_return;

	if (strcpy_s(user->m_Accountid, accountId))
	{
		spdlog::error("AujardDlg::SelectCharacter: failed to write accountId(len: {}, val: {}) to user->m_Accountid",
			std::strlen(accountId), accountId);
		// it didn't fail here before and we don't currently return anything upstream
		// if this exposes any problems we'll have to decide how to handle it then
	}

	SetByte(sendBuff, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x01, sendIndex);
	SetByte(sendBuff, init, sendIndex);

	_packetCount++;	

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
		{
			_sendPacketCount++;
			break;
		}

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_SEL_CHAR"));
		spdlog::error("AujardDlg::SelectCharacter: Packet Drop: WIZ_SEL_CHAR");
	}

	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuff, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x00, sendIndex);

	LoggerSendQueue.PutData(sendBuff, sendIndex);
}

/// \brief Handles a WIZ_LOGOUT request when logging out of the game
/// \details Updates USERDATA and WAREHOUSE as part of logging out, then resets the UserData entry for re-use
/// \see WIZ_LOGOUT
void CAujardDlg::UserLogOut(char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0,
		charIdLen = 0, sendIndex = 0, retryCount = 0,
		maxRetry = 50;
	char charId[MAX_ID_SIZE + 1] = {},
		accountId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	if (strlen(charId) == 0)
		return;

	HandleUserLogout(userId, UPDATE_LOGOUT);

	SetByte(sendBuff, WIZ_LOGOUT, sendIndex);
	SetShort(sendBuff, userId, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_LOGOUT"));
		spdlog::error("AujardDlg::UserLogOut: Packet Drop: WIZ_LOGOUT");
	}
}

/// \brief handles user logout functions
/// \param userId user index for UserData
/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE
/// \param forceLogout should be set to true in panic situations
/// \see UserLogOut(), AllSaveRoutine(), HandleUserUpdate()
bool CAujardDlg::HandleUserLogout(int userId, BYTE saveType, bool forceLogout)
{
	bool logoutResult = false;

	// make sure UserData[userId] value is valid
	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr || std::strlen(pUser->m_id) == 0)
	{
		spdlog::error("AujardDlg::HandleUserLogout: Invalid logout: UserData[{}] is not in use", userId);
		return false;
	}

	// only call AccountLogout for non-zone change logouts
	if (pUser->m_bLogout != 2 || forceLogout)
		logoutResult = _dbAgent.AccountLogout(pUser->m_Accountid);
	else
		logoutResult = true;

	// update UserData (USERDATA/WAREHOUSE)
	bool userdataSuccess = HandleUserUpdate(userId, *pUser, saveType);
	
	// Log results
	bool success = userdataSuccess && logoutResult;
	if (!success)
	{
		spdlog::error("AujardDlg::HandleUserLogout: Invalid Logout: {}, {} (UserData: {}, Logout: {})",
			pUser->m_Accountid, pUser->m_id, userdataSuccess, logoutResult);
		return false;
	}

	spdlog::debug("AujardDlg::HandleUserLogout: Logout: {}, {} (UserData: {}, Logout: {})",
	pUser->m_Accountid, pUser->m_id, userdataSuccess, logoutResult);

	// reset the object stored in UserData[userId] before returning
	// this will reset data like accountId/charId, so logging must
	// occur beforehand
	_dbAgent.ResetUserData(userId);
	
	return success;
}

/// \brief handles user update functions and retry logic
/// \param userId user index for UserData
/// \param user reference to user object
/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE, UPDATE_PACKET_SAVE
/// \see UserDataSave(), HandleUserLogout()
bool CAujardDlg::HandleUserUpdate(int userId, const _USER_DATA& user, BYTE saveType)
{
	DWORD sleepTime = 10;
	int updateWarehouseResult = 0, updateUserResult = 0,
		retryCount = 0, maxRetry = 10;

	// attempt updates
	updateUserResult = _dbAgent.UpdateUser(user.m_id, userId, saveType);
	Sleep(sleepTime);
	updateWarehouseResult = _dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);

	// TODO:  Seems like the following two loops could/should just be combined
	// retry handling for update user/warehouse
	for (retryCount = 0; !updateWarehouseResult || !updateUserResult; retryCount++)
	{
		if (retryCount >= maxRetry)
		{
			spdlog::error("AujardDlg::HandleUserUpdate: UserData Save Error: [accountId={} charId={} (W:{},U:{})]",
				user.m_Accountid, user.m_id, updateWarehouseResult, updateUserResult);
			break;
		}
		// only retry the calls that fail - they're both updating using UserData[userId]->dwTime, so they should sync fine
		if (!updateUserResult)
		{
			updateUserResult = _dbAgent.UpdateUser(user.m_id, userId, saveType);
		}
		Sleep(sleepTime);
		if (!updateWarehouseResult)
		{
			updateWarehouseResult = _dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);
		}
	}
	
	// Verify saved data/timestamp
	updateWarehouseResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 1, user.m_dwTime, user.m_iBank);
	updateUserResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 2, user.m_dwTime, user.m_iExp);
	for (retryCount = 0; !updateWarehouseResult || !updateUserResult; retryCount++)
	{
		if (retryCount >= maxRetry)
		{
			if (!updateWarehouseResult)
			{
				spdlog::error("AujardDlg::HandleUserUpdate: Warehouse Save Check Error [accountId={} charId={} (W:{})]",
					user.m_Accountid, user.m_id, updateWarehouseResult);
			}
			if (!updateUserResult)
			{
				spdlog::error("AujardDlg::HandleUserUpdate: UserData Save Check Error: [accountId={} charId={} (U:{})]",
					user.m_Accountid, user.m_id, updateUserResult);
			}
			break;
		}
		if (!updateWarehouseResult)
		{
			_dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);
			updateWarehouseResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 1, user.m_dwTime, user.m_iBank);
		}
		Sleep(sleepTime);
		if (!updateUserResult)
		{
			_dbAgent.UpdateUser(user.m_id, userId, saveType);
			updateUserResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 2, user.m_dwTime, user.m_iExp);
		}
	}

	return static_cast<bool>(updateUserResult) && static_cast<bool>(updateWarehouseResult);
}

/// \brief handles a WIZ_LOGIN request to a selected game server
/// \see WIZ_LOGIN
void CAujardDlg::AccountLogIn(char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0,
		passwordLen = 0, sendIndex = 0, retryCount = 0,
		maxRetry = 50;
	int nation = -1;

	char accountId[MAX_ID_SIZE + 1] = {},
		password[MAX_PW_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	passwordLen = GetShort(buffer, index);
	GetString(password, buffer, passwordLen, index);

	nation = _dbAgent.AccountLogInReq(accountId, password);

	SetByte(sendBuff, WIZ_LOGIN, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, nation, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_LOGIN"));
		spdlog::error("AujardDlg::AccountLogIn: Packet Drop: WIZ_LOGIN");
	}
}

/// \brief handles a WIZ_SEL_NATION request to a selected game server
/// \see WIZ_SEL_NATION
void CAujardDlg::SelectNation(char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, sendIndex = 0,
		retryCount = 0, maxRetry = 50;
	int nation = -1;
	char accountId[MAX_ID_SIZE + 1] = {},
		password[MAX_PW_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	nation = GetByte(buffer, index);

	bool result = _dbAgent.NationSelect(accountId, nation);

	SetByte(sendBuff, WIZ_SEL_NATION, sendIndex);
	SetShort(sendBuff, userId, sendIndex);

	if (result)
		SetByte(sendBuff, nation, sendIndex);
	else
		SetByte(sendBuff, 0x00, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_SEL_NATION"));
		spdlog::error("AujardDlg::SelectNation: Packet Drop: WIZ_SEL_NATION");
	}
}

/// \brief handles a WIZ_NEW_CHAR request
/// \see WIZ_NEW_CHAR
void CAujardDlg::CreateNewChar(char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0, sendIndex = 0,
		result = 0, retryCount = 0, charIndex = 0, race = 0, Class = 0, hair = 0,
		face = 0, str = 0, sta = 0, dex = 0, intel = 0, cha = 0, maxRetry = 50;
	char accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIndex = GetByte(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	race = GetByte(buffer, index);
	Class = GetShort(buffer, index);
	face = GetByte(buffer, index);
	hair = GetByte(buffer, index);
	str = GetByte(buffer, index);
	sta = GetByte(buffer, index);
	dex = GetByte(buffer, index);
	intel = GetByte(buffer, index);
	cha = GetByte(buffer, index);

	result = _dbAgent.CreateNewChar(accountId, charIndex, charId, race, Class, hair, face, str, sta, dex, intel, cha);

	SetByte(sendBuff, WIZ_NEW_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_NEW_CHAR"));
		spdlog::error("AujardDlg::CreateNewChar: Packet Drop: WIZ_NEW_CHAR");
	}
}

/// \brief handles a WIZ_DEL_CHAR request
/// \todo not implemented, always returns an error to the client
/// \see WIZ_DEL_CHAR
void CAujardDlg::DeleteChar(char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0,
		sendIndex = 0, result = 0, retryCount = 0, maxRetry = 50;
	int charIndex = 0, socNoLen = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {},
		socNo[15] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIndex = GetByte(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	socNoLen = GetShort(buffer, index);
	GetString(socNo, buffer, socNoLen, index);

	// Not implemented.  Allow result to default to 0.
	//result = _dbAgent.DeleteChar(charindex, accountid, charid, socno);

	spdlog::trace("AujardDlg::DeleteChar: [charId={}, socNo={}]", charId, socNo);

	SetByte(sendBuff, WIZ_DEL_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	if (result > 0)
		SetByte(sendBuff, charIndex, sendIndex);
	else
		SetByte(sendBuff, 0xFF, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_DEL_CHAR"));
		spdlog::error("AujardDlg::DeleteChar: Packet Drop: WIZ_DEL_CHAR");
	}
}

/// \brief handles a WIZ_ALLCHAR_INFO_REQ request
/// \details Loads all character information and sends it to the client
/// \see WIZ_ALLCHAR_INFO_REQ
void CAujardDlg::AllCharInfoReq(char* buffer)
{
	int index = 0, userId = 0, accountIdLen = 0, sendIndex = 0,
		charBuffIndex = 0, retryCount = 0, maxRetry = 50;
	char accountId[MAX_ID_SIZE + 1] = {},
		sendBuff[1024] = {},
		charBuff[1024] = {},
		charId1[MAX_ID_SIZE + 1] = {},
		charId2[MAX_ID_SIZE + 1] = {},
		charId3[MAX_ID_SIZE + 1] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);

	SetByte(charBuff, 0x01, charBuffIndex);	// result

	_dbAgent.GetAllCharID(accountId, charId1, charId2, charId3);
	_dbAgent.LoadCharInfo(charId1, charBuff, charBuffIndex);
	_dbAgent.LoadCharInfo(charId2, charBuff, charBuffIndex);
	_dbAgent.LoadCharInfo(charId3, charBuff, charBuffIndex);

	SetByte(sendBuff, WIZ_ALLCHAR_INFO_REQ, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetShort(sendBuff, charBuffIndex, sendIndex);
	SetString(sendBuff, charBuff, charBuffIndex, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: WIZ_ALLCHAR_INFO_REQ"));
		spdlog::error("AujardDlg::AllCharInfoReq: Packet Drop: WIZ_ALLCHAR_INFO_REQ");
	}
}

BOOL CAujardDlg::PreTranslateMessage(MSG* msg)
{
	if (msg->message == WM_KEYDOWN)
	{
		if (msg->wParam == VK_RETURN
			|| msg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(msg);
}

/// \brief triggered when the Exit button is clicked. Will ask user to confirm intent to close the program.
void CAujardDlg::OnOK()
{
	if (AfxMessageBox(_T("Do you really want to quit?"), MB_YESNO) == IDYES)
	{
		spdlog::debug("AujardDlg::OnOK: User closing application");
		CDialog::OnOK();
	}
}

void CAujardDlg::OnTimer(UINT EventId)
{
	HANDLE	hProcess = nullptr;

	switch (EventId)
	{
		case PROCESS_CHECK:
			hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_READ, FALSE, LoggerSendQueue.GetProcessId());
			if (hProcess == nullptr)
				AllSaveRoutine();
			break;

		case CONCURRENT_CHECK:
#ifdef __SAMMA
			ConCurrentUserCount();
#endif
			break;

		case SERIAL_TIME:
			g_increase_serial = 50001;
			break;

		case PACKET_CHECK:
			WritePacketLog();
	//		SaveUserData();
			break;

		case DB_POOL_CHECK:
			db::ConnectionManager::ExpireUnusedPoolConnections();
			break;
	}

	CDialog::OnTimer(EventId);
}

/// \brief handling for when OnTimer fails a PROCESS_CHECK with ebenezer
/// \details Logs ebenezer outage, attempts to save all UserData, and resets all UserData[userId] objects
/// \see OnTimer()
void CAujardDlg::AllSaveRoutine()
{
	// TODO:  100ms seems excessive
	DWORD sleepTime = 100;
	bool allUsersSaved = true;
	
	// log the disconnect
	CTime cur = CTime::GetCurrentTime();
	std::wstring msgStr = std::format(L"Ebenezer disconnected: {:04}/{:02}/{:02} {:02}:{:02}",
		cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute());
	AddOutputMessage(msgStr);
	spdlog::error("AujardDlg::AllSaveRoutine: Ebenezer disconnected. Saving users...");
	
	for (int userId = 0; userId < static_cast<int>(_dbAgent.UserData.size()); userId++)
	{
		_USER_DATA* pUser = _dbAgent.UserData[userId];
		if (pUser == nullptr)
		{
			spdlog::debug("AujardDlg::AllSaveRoutine: userId skipped for invalid data: {}", userId);
			continue;
		}

		if (strlen(pUser->m_id) == 0)
			continue;

		if (HandleUserLogout(userId, UPDATE_ALL_SAVE, true))
		{
			spdlog::debug("AujardDlg::AllSaveRoutine: Character saved: {}", pUser->m_id);
		}
		else
		{
			allUsersSaved = false;
			spdlog::error("AujardDlg::AllSaveRoutine: failed to save character: {}", pUser->m_id);
		}
		Sleep(sleepTime);
	}
	if (allUsersSaved)
	{
		msgStr = std::format(L"All UserData saved: {:04}/{:02}/{:02} {:02}:{:02}",
			cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute());
		AddOutputMessage(msgStr);
		spdlog::info("AujardDlg::AllSaveRoutine: Ebenezer disconnect: all users saved successfully");
	}
	else
	{
		msgStr = std::format(L"Not all UserData saved: {:04}/{:02}/{:02} {:02}:{:02}",
			cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute());
		AddOutputMessage(msgStr);
		spdlog::error("AujardDlg::AllSaveRoutine: Ebenezer disconnect: not all users saved");
	}
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CAujardDlg::AddOutputMessage(const std::string& msg)
{
	std::wstring wMsg = LocalToWide(msg);
	AddOutputMessage(wMsg);
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CAujardDlg::AddOutputMessage(const std::wstring& msg)
{
	_outputList.AddString(msg.data());
	
	// Set the focus to the last item and ensure it is visible
	int lastIndex = _outputList.GetCount()-1;
	_outputList.SetTopIndex(lastIndex);
}

/// \brief Called by OnTimer if __SAMMA is defined
void CAujardDlg::ConCurrentUserCount()
{
	int usercount = 0;

	for (int userId = 0; userId < MAX_USER; userId++)
	{
		_USER_DATA* pUser = _dbAgent.UserData[userId];
		if (pUser == nullptr)
			continue;

		if (strlen(pUser->m_id) == 0)
			continue;

		usercount++;
	}

	spdlog::trace("AujardDlg::ConCurrentUserCount: [serverId={} zoneId={} userCount={}]",
		_serverId, _zoneId, usercount);

	_dbAgent.UpdateConCurrentUserCount(_serverId, _zoneId, usercount);
}

/// \brief handles a WIZ_DATASAVE request
/// \see WIZ_DATASAVE
/// \see HandleUserUpdate()
void CAujardDlg::UserDataSave(char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);

	if (userId < 0
		|| userId >= MAX_USER
		|| strlen(accountId) == 0
		|| strlen(charId) == 0)
		return;
	
	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	bool userdataSuccess = HandleUserUpdate(userId, *pUser, UPDATE_PACKET_SAVE);
	if (!userdataSuccess)
	{
		spdlog::error("AujardDlg::UserDataSave: failed for UserData[{}] [accountId={} charId={}]",
			userId, accountId, charId);
	}
}

/// \brief attempts to find a UserData record for charId
/// \param charId
/// \param[out] userId UserData index of the user, if found
/// \return pointer to UserData[userId] object if found, nullptr otherwise
_USER_DATA* CAujardDlg::GetUserPtr(const char* charId, int& userId)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		_USER_DATA* pUser = _dbAgent.UserData[i];
		if (!pUser)
			continue;

		if (_strnicmp(charId, pUser->m_id, MAX_ID_SIZE) == 0)
		{
			userId = i;
			return pUser;
		}
	}

	return nullptr;
}

/// \brief handles WIZ_KNIGHTS_PROCESS and WIZ_CLAN_PROCESS requests
/// \detail calls the appropriate method for the subprocess op-code
/// \see WIZ_KNIGHTS_PROCESS WIZ_CLAN_PROCESS
/// \see "Knights Packet sub define" section in Define.h
void CAujardDlg::KnightsPacket(char* buffer)
{
	int index = 0, nation = 0;
	BYTE command = GetByte(buffer, index);

	switch (command)
	{
		case KNIGHTS_CREATE:
			CreateKnights(buffer + index);
			break;

		case KNIGHTS_JOIN:
			JoinKnights(buffer + index);
			break;

		case KNIGHTS_WITHDRAW:
			WithdrawKnights(buffer + index);
			break;

		case KNIGHTS_REMOVE:
		case KNIGHTS_ADMIT:
		case KNIGHTS_REJECT:
		case KNIGHTS_CHIEF:
		case KNIGHTS_VICECHIEF:
		case KNIGHTS_OFFICER:
		case KNIGHTS_PUNISH:
			ModifyKnightsMember(buffer + index, command);
			break;

		case KNIGHTS_DESTROY:
			DestroyKnights(buffer + index);
			break;

		case KNIGHTS_MEMBER_REQ:
			AllKnightsMember(buffer + index);
			break;

		case KNIGHTS_STASH:
			break;

		case KNIGHTS_LIST_REQ:
			KnightsList(buffer + index);
			break;

		case KNIGHTS_ALLLIST_REQ:
			nation = GetByte(buffer, index);
			_dbAgent.LoadKnightsAllList(nation);
			break;

		default:
			spdlog::error("AujardDlg::KnightsPacket: Invalid WIZ_KNIGHTS_PROCESS command code received: {:X}",
				command);
	}
}

/// \brief attempts to create a knights clan
/// \see KnightsPacket(), KNIGHTS_CREATE
void CAujardDlg::CreateKnights(char* buffer)
{
	int index = 0, sendIndex = 0, nameLen = 0, chiefNameLen = 0, knightsId = 0,
		userId = -1, nation = 0, result = 0, community = 0,
		retryCount = 0, maxRetry = 50;
	char knightsName[MAX_ID_SIZE + 1] = {},
		chiefName[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	community = GetByte(buffer, index);
	knightsId = GetShort(buffer, index);
	nation = GetByte(buffer, index);
	nameLen = GetShort(buffer, index);
	GetString(knightsName, buffer, nameLen, index);
	chiefNameLen = GetShort(buffer, index);
	GetString(chiefName, buffer, chiefNameLen, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	result = _dbAgent.CreateKnights(knightsId, nation, knightsName, chiefName, community);

	spdlog::trace("AujardDlg::CreateKnights: userId={}, knightsId={}, result={}",
		userId, knightsId, result);

	SetByte(sendBuff, KNIGHTS_CREATE, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetByte(sendBuff, community, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);
	SetByte(sendBuff, nation, sendIndex);
	SetShort(sendBuff, nameLen, sendIndex);
	SetString(sendBuff, knightsName, nameLen, sendIndex);
	SetShort(sendBuff, chiefNameLen, sendIndex);
	SetString(sendBuff, chiefName, chiefNameLen, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: KNIGHTS_CREATE"));
		spdlog::error("AujardDlg::CreateKnights: Packet Drop: KNIGHTS_CREATE");
	}
}

/// \brief attempts to add a character to a knights clan
/// \see KnightsPacket(), KNIGHTS_JOIN
void CAujardDlg::JoinKnights(char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		retryCount = 0, maxRetry = 50;
	BYTE result = 0;
	char sendBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	result = _dbAgent.UpdateKnights(KNIGHTS_JOIN, pUser->m_id, knightsId, 0);

	spdlog::trace("AujardDlg::JoinKnights: userId={}, charId={}, knightsId={}, result={}",
		userId, pUser->m_id, knightsId, result);

	SetByte(sendBuff, KNIGHTS_JOIN, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: KNIGHTS_JOIN"));
		spdlog::error("AujardDlg::JoinKnights: Packet Drop: KNIGHTS_JOIN");
	}
}

/// \brief attempt to remove a character from a knights clan
/// \see KnightsPacket(), KNIGHTS_WITHDRAW
void CAujardDlg::WithdrawKnights(char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		retryCount = 0, maxRetry = 50;
	BYTE result = 0;
	char sendBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	result = _dbAgent.UpdateKnights(KNIGHTS_WITHDRAW, pUser->m_id, knightsId, 0);
	spdlog::trace("AujardDlg::WithdrawKnights: userId={}, knightsId={}, result={}",
		userId, knightsId, result);

	SetByte(sendBuff, KNIGHTS_WITHDRAW, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: KNIGHTS_WITHDRAW"));
		spdlog::error("AujardDlg::WithdrawKnights: Packet Drop: KNIGHTS_WITHDRAW");
	}
}

/// \brief attempts to modify a knights character
/// \see KnightsPacket(), KNIGHTS_REMOVE, KNIGHTS_ADMIT, KNIGHTS_REJECT, KNIGHTS_CHIEF,
/// KNIGHTS_VICECHIEF, KNIGHTS_OFFICER, KNIGHTS_PUNISH
void CAujardDlg::ModifyKnightsMember(char* buffer, BYTE command)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1, charIdLen = 0,
		removeFlag = 0, retryCount = 0, maxRetry = 50;
	BYTE result = 0;
	char send_buff[256] = {},
		charId[MAX_ID_SIZE + 1] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	removeFlag = GetByte(buffer, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

/*	if( remove_flag == 0 && command == KNIGHTS_REMOVE )	{		// 없는 유저 추방시에는 디비에서만 처리한다
		result = m_DBAgent.UpdateKnights( command, userid, knightindex, remove_flag );
		TRACE(_T("ModifyKnights - command=%d, nid=%d, index=%d, result=%d \n"), command, uid, knightindex, result);
		return;
	}	*/

	result = _dbAgent.UpdateKnights(command, charId, knightsId, removeFlag);
	spdlog::trace("AujardDlg::ModifyKnights: command={}, userId={}, knightsId={}, result={}",
		command, userId, knightsId, result);

	//SetByte( send_buff, WIZ_KNIGHTS_PROCESS, send_index );
	SetByte(send_buff, command, sendIndex);
	SetShort(send_buff, userId, sendIndex);
	SetByte(send_buff, result, sendIndex);
	SetShort(send_buff, knightsId, sendIndex);
	SetShort(send_buff, charIdLen, sendIndex);
	SetString(send_buff, charId, charIdLen, sendIndex);
	SetByte(send_buff, removeFlag, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(send_buff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		std::string cmdStr;
		switch (command)
		{
		case KNIGHTS_REMOVE:
			cmdStr = "KNIGHTS_REMOVE";
			break;
		case KNIGHTS_ADMIT:
			cmdStr = "KNIGHTS_ADMIT";
			break;
		case KNIGHTS_REJECT:
			cmdStr = "KNIGHTS_REJECT";
			break;
		case KNIGHTS_CHIEF:
			cmdStr = "KNIGHTS_CHIEF";
			break;
		case KNIGHTS_VICECHIEF:
			cmdStr = "KNIGHTS_VICECHIEF";
			break;
		case KNIGHTS_OFFICER:
			cmdStr = "KNIGHTS_OFFICER";
			break;
		case KNIGHTS_PUNISH:
			cmdStr = "KNIGHTS_PUNISH";
			break;
		default:
			cmdStr = "ModifyKnightsMember";
		}
		std::string errMsg = fmt::format("Packet Drop: {}", cmdStr);
		AddOutputMessage(errMsg);
		spdlog::error("AujardDlg::ModifyKnightsMember: {}", errMsg);
	}
}

/// \brief attempts to disband a knights clan
/// \see KnightsPacket(), KNIGHTS_DESTROY
void CAujardDlg::DestroyKnights(char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		retryCount = 0, maxRetry = 50;
	BYTE result = 0;
	char sendBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	if (userId < 0
		|| userId >= MAX_USER)
		return;

	result = _dbAgent.DeleteKnights(knightsId);
	spdlog::trace("AujardDlg::DestroyKnights: userId={}, knightsId={}, result={}",
	userId, knightsId, result);

	SetByte(sendBuff, KNIGHTS_DESTROY, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;
		
		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: KNIGHTS_DESTROY"));
		spdlog::error("AujardDlg::DestroyKnights: Packet Drop: KNIGHTS_DESTROY");
	}
}

/// \brief attempts to return a list of all knights members
/// \see KnightsPacket(), KNIGHTS_MEMBER_REQ
void CAujardDlg::AllKnightsMember(char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		dbIndex = 0, count = 0, retryCount = 0, maxRetry = 50;
	BYTE result = 0;
	char sendBuff[2048] = {},
		dbBuff[2048] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	//int page = GetShort( pBuf, index );

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	//if( page < 0 )  return;

	count = _dbAgent.LoadKnightsAllMembers(knightsId, 0, dbBuff, dbIndex);
	//count = m_DBAgent.LoadKnightsAllMembers( knightindex, page*10, temp_buff, buff_index );

	SetByte(sendBuff, KNIGHTS_MEMBER_REQ, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x00, sendIndex);		// Success
	SetShort(sendBuff, 4 + dbIndex, sendIndex);	// total packet size -> short(*3) + buff_index 
	//SetShort( send_buff, page, send_index );
	SetShort(sendBuff, count, sendIndex);
	SetString(sendBuff, dbBuff, dbIndex, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retryCount++;
	}
	while (retryCount < maxRetry);

	if (retryCount >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: KNIGHTS_MEMBER_REQ"));
		spdlog::error("AujardDlg::AllKnightsMember: Packet Drop: KNIGHTS_MEMBER_REQ");
	}
}

/// \brief attempts to retrieve metadata for a knights clan
/// \see KnightsPacket(), KNIGHTS_LIST_REQ
void CAujardDlg::KnightsList(char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		dbIndex = 0, retry = 0, maxRetry = 50;
	char sendBuff[256] = {},
		dbBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	if (userId < 0
		|| userId >= MAX_USER)
		return;

	_dbAgent.LoadKnightsInfo(knightsId, dbBuff, dbIndex);

	SetByte(sendBuff, KNIGHTS_LIST_REQ, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x00, sendIndex);
	SetString(sendBuff, dbBuff, dbIndex, sendIndex);

	do
	{
		if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
			break;

		retry++;
	}
	while (retry < maxRetry);

	if (retry >= maxRetry)
	{
		AddOutputMessage(_T("Packet Drop: KNIGHTS_LIST_REQ"));
		spdlog::error("AujardDlg::KnightsList: Packet Drop: KNIGHTS_LIST_REQ");
	}
}

/// \brief handles WIZ_LOGIN_INFO requests, updating CURRENTUSER for a user
/// \see WIZ_LOGIN_INFO
void CAujardDlg::SetLogInInfo(char* buffer)
{
	int index = 0, accountIdLen = 0, charIdLen = 0, serverId = 0, serverIpLen = 0,
	clientIpLen = 0, userId = -1, sendIndex = 0, retryCount = 0, maxRetry = 50;
	char accountId[MAX_ID_SIZE + 1] = {},
		serverIp[20] = {},
		clientIp[20] = {},
		charId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	serverIpLen = GetShort(buffer, index);
	GetString(serverIp, buffer, serverIpLen, index);
	serverId = GetShort(buffer, index);
	clientIpLen = GetShort(buffer, index);
	GetString(clientIp, buffer, clientIpLen, index);
	
	// init: 0x01 to insert, 0x02 to update CURRENTUSER
	BYTE init = GetByte(buffer, index);

	if (!_dbAgent.SetLogInInfo(accountId, charId, serverIp, serverId, clientIp, init))
	{
		SetByte(sendBuff, WIZ_LOGIN_INFO, sendIndex);
		SetShort(sendBuff, userId, sendIndex);
		SetByte(sendBuff, 0x00, sendIndex);							// FAIL
		do
		{
			if (LoggerSendQueue.PutData(sendBuff, sendIndex) == 1)
				break;

			retryCount++;
		}
		while (retryCount < maxRetry);

		if (retryCount >= maxRetry)
		{
			AddOutputMessage(_T("Packet Drop: WIZ_LOGIN_INFO"));
			spdlog::error("AujardDlg::SetLoginInfo: Packet Drop: WIZ_LOGIN_INFO");
			spdlog::error("AujardDlg::SetLoginInfo: exceeded max retries [accountId={}, charId={}, init={}]",
				accountId, charId, init);
		}
	}
}

/// \brief handles WIZ_KICKOUT requests
/// \see WIZ_KICKOUT
void CAujardDlg::UserKickOut(char* buffer)
{
	int index = 0, accountIdLen = 0;
	char accountId[MAX_ID_SIZE + 1] = {};

	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);

	_dbAgent.AccountLogout(accountId);
}

/// \brief writes a packet summary line to the log file
void CAujardDlg::WritePacketLog()
{
	spdlog::info("AujardDlg::WritePacketLog: Packet Count: recv={}, send={}, realsend={}",
		_recvPacketCount, _packetCount, _sendPacketCount);
}

/// \brief checks for users who have not saved their data in AUTOSAVE_DELTA milliseconds
/// and performs a UserDataSave() for them.
/// \note this is currently disabled in OnTimer()
/// \see UserDataSave(), OnTimer(), PACKET_CHECK
void CAujardDlg::SaveUserData()
{
	DWORD sleepTime = 100;
	char sendBuff[256] = {};
	int sendIndex = 0;

	for (int userId = 0; userId < MAX_USER; userId++)
	{
		_USER_DATA* user = _dbAgent.UserData[userId];
		// skip user slots that aren't in use
		if (user == nullptr || strlen(user->m_id) == 0)
			continue;

		// GetTickCount(): Retrieves the number of milliseconds that have elapsed since the system was started, up to 49.7 days.
		// (at which point it overflows. neat.)
		if (GetTickCount() - user->m_dwTime > AUTOSAVE_DELTA)
		{
			memset(sendBuff, 0, sizeof(sendBuff));
			sendIndex = 0;
			SetShort(sendBuff, userId, sendIndex);
			SetShort(sendBuff, strlen(user->m_Accountid), sendIndex);
			SetString(sendBuff, user->m_Accountid, strlen(user->m_Accountid), sendIndex);
			SetShort(sendBuff, strlen(user->m_id), sendIndex);
			SetString(sendBuff, user->m_id, strlen(user->m_id), sendIndex);
			UserDataSave(sendBuff);
			Sleep(sleepTime);
		}
	}
}

/// \brief handles WIZ_BATTLE_EVENT requests
/// \details contains which nation won the war and which charId killed the commander
/// \see WIZ_BATTLE_EVENT
void CAujardDlg::BattleEventResult(char* data)
{
	int _type = 0, result = 0, charIdLen = 0, index = 0;
	char charId[MAX_ID_SIZE + 1] = {};

	_type = GetByte(data, index);
	result = GetByte(data, index);
	charIdLen = GetByte(data, index);
	if (charIdLen > 0
		&& charIdLen < MAX_ID_SIZE + 1)
	{
		GetString(charId, data, charIdLen, index);
		spdlog::info("AujardDlg::BattleEventResult : The user who killed the enemy commander is {}, _type={}, nation={}",
			charId, _type, result);
		_dbAgent.UpdateBattleEvent(charId, result);
	}
}

/// \brief handles DB_COUPON_EVENT requests
/// \todo related stored procedures are not implemented
/// \see DB_COUPON_EVENT
void CAujardDlg::CouponEvent(char* data)
{
	int nSid = 0, nEventNum = 0, nLen = 0, nCharLen = 0, nCouponLen = 0, index = 0, nType = 0, nResult = 0, send_index = 0, count = 0;
	int nItemID = 0, nItemCount = 0, nMessageNum = 0;
	char strAccountName[MAX_ID_SIZE + 1] = {},
		strCharName[MAX_ID_SIZE + 1] = {},
		strCouponID[MAX_ID_SIZE + 1] = {},
		send_buff[256] = {};

	nType = GetByte(data, index);
	if (nType == CHECK_COUPON_EVENT)
	{
		nSid = GetShort(data, index);
		nLen = GetShort(data, index);
		GetString(strAccountName, data, nLen, index);
		nEventNum = GetDWORD(data, index);
		// 비러머글 대사문 >.<
		nMessageNum = GetDWORD(data, index);
		// TODO: Not implemented. Allow nResult to default to 0
		// nResult = _dbAgent.CheckCouponEvent(strAccountName);

		SetByte(send_buff, DB_COUPON_EVENT, send_index);
		SetShort(send_buff, nSid, send_index);
		SetByte(send_buff, nResult, send_index);
		SetDWORD(send_buff, nEventNum, send_index);
		// 비러머글 대사문 >.<
		SetDWORD(send_buff, nMessageNum, send_index);
		//

		do
		{
			if (LoggerSendQueue.PutData(send_buff, send_index) == 1)
				break;

			count++;
		}
		while (count < 50);

		if (count >= 50)
		{
			AddOutputMessage(_T("Packet Drop: DB_COUPON_EVENT"));
			spdlog::error("AujardDlg::CouponEvent: Packet Drop: DB_COUPON_EVENT");
		}
	}
	else if (nType == UPDATE_COUPON_EVENT)
	{
		nSid = GetShort(data, index);
		nLen = GetShort(data, index);
		GetString(strAccountName, data, nLen, index);
		nCharLen = GetShort(data, index);
		GetString(strCharName, data, nCharLen, index);
		nCouponLen = GetShort(data, index);
		GetString(strCouponID, data, nCouponLen, index);
		nItemID = GetDWORD(data, index);
		nItemCount = GetDWORD(data, index);

		// TODO: not implemented.  Allow nResult to default to 0
		// nResult = _dbAgent.UpdateCouponEvent(strAccountName, strCharName, strCouponID, nItemID, nItemCount);
	}
}

/// \brief Updates the IDC_DB_PROCESS text with the DB Process Number
/// \note I don't actually see this on the UI, and I'm not sure how practical it is under load.
void CAujardDlg::DBProcessNumber(int number)
{
	CString strDBNum;
	strDBNum.Format(_T(" %4d "), number);

	DBProcessNum.SetWindowText(strDBNum);
	DBProcessNum.UpdateWindow();
}
