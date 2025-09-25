// EbenezerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EbenezerDlg.h"
#include "OperationMessage.h"
#include "User.h"
#include "db_resources.h"

#include <shared/crc32.h>
#include <shared/lzf.h>
#include <shared/packets.h>
#include <shared/StringUtils.h>

#include <db-library/ConnectionManager.h>

constexpr int GAME_TIME       	= 100;
constexpr int SEND_TIME			= 200;
constexpr int PACKET_CHECK		= 300;
constexpr int ALIVE_TIME		= 400;
constexpr int MARKET_BBS_TIME	= 1000;

constexpr int NUM_FLAG_VICTORY    = 4;
constexpr int AWARD_GOLD          = 5000;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NOTE: Explicitly handled under DEBUG_NEW override
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/RecordSetLoader_Vector.h>

import EbenezerBinder;

using namespace db;

CRITICAL_SECTION g_serial_critical;
CRITICAL_SECTION g_region_critical;

CEbenezerDlg* CEbenezerDlg::s_pInstance = nullptr;
CIOCPort CEbenezerDlg::m_Iocport;

uint16_t g_increase_serial = 1;
bool g_serverdown_flag = false;

DWORD WINAPI ReadQueueThread(LPVOID lp)
{
	CEbenezerDlg* pMain = (CEbenezerDlg*) lp;
	int recvlen = 0, index = 0, uid = -1, send_index = 0, buff_length = 0;
	uint8_t command, result;
	char pBuf[1024] = {}, send_buff[1024] = {};
	CUser* pUser = nullptr;
	int currenttime = 0;

	while (true)
	{
		if (pMain->m_LoggerRecvQueue.GetFrontMode() == R)
			continue;

		index = 0;
		recvlen = pMain->m_LoggerRecvQueue.GetData(pBuf);

		if (recvlen > MAX_PKTSIZE
			|| recvlen == 0)
		{
			Sleep(1);
			continue;
		}

		command = GetByte(pBuf, index);
		uid = GetShort(pBuf, index);

		if (command == (KNIGHTS_ALLLIST_REQ + 0x10)
			&& uid == -1)
		{
			pMain->m_KnightsManager.RecvKnightsAllList(pBuf + index);
			continue;
		}

		if (uid < 0
			|| uid >= MAX_USER)
			goto loop_pass;

		pUser = (CUser*) pMain->m_Iocport.m_SockArray[uid];
		if (pUser == nullptr)
			goto loop_pass;

		switch (command)
		{
			case WIZ_LOGIN:
				result = GetByte(pBuf, index);
				if (result == 0xFF)
					memset(pUser->m_strAccountID, 0, sizeof(pUser->m_strAccountID));
				SetByte(send_buff, WIZ_LOGIN, send_index);
				SetByte(send_buff, result, send_index);					// 성공시 국가 정보
				pUser->Send(send_buff, send_index);
				break;

			case WIZ_SEL_NATION:
				SetByte(send_buff, WIZ_SEL_NATION, send_index);
				SetByte(send_buff, GetByte(pBuf, index), send_index);	// 국가 정보
				pUser->Send(send_buff, send_index);
				break;

			case WIZ_NEW_CHAR:
				result = GetByte(pBuf, index);
				SetByte(send_buff, WIZ_NEW_CHAR, send_index);
				SetByte(send_buff, result, send_index);					// 성공시 국가 정보
				pUser->Send(send_buff, send_index);
				break;

			case WIZ_DEL_CHAR:
				pUser->RecvDeleteChar(pBuf + index);
			/*	result = GetByte( pBuf, index );
				SetByte( send_buff, WIZ_DEL_CHAR, send_index );
				SetByte( send_buff, result, send_index );					// 성공시 국가 정보
				SetByte( send_buff, GetByte( pBuf, index ), send_index );
				pUser->Send( send_buff, send_index );	*/
				break;

			case WIZ_SEL_CHAR:
				pUser->SelectCharacter(pBuf + index);
				break;

			case WIZ_ALLCHAR_INFO_REQ:
				buff_length = GetShort(pBuf, index);
				if (buff_length > recvlen)
					break;

				SetByte(send_buff, WIZ_ALLCHAR_INFO_REQ, send_index);
				SetString(send_buff, pBuf + index, buff_length, send_index);
				pUser->Send(send_buff, send_index);
				break;

			case WIZ_LOGOUT:
				if (pUser != nullptr
					&& strlen(pUser->m_pUserData->m_id) != 0)
				{
					spdlog::debug("EbenezerDlg::ReadQueueThread: WIZ_LOGOUT [charId={}]",
						pUser->m_pUserData->m_id);
					pUser->Close();
				}
				break;

			case KNIGHTS_CREATE + 0x10:
			case KNIGHTS_JOIN + 0x10:
			case KNIGHTS_WITHDRAW + 0x10:
			case KNIGHTS_REMOVE + 0x10:
			case KNIGHTS_ADMIT + 0x10:
			case KNIGHTS_REJECT + 0x10:
			case KNIGHTS_CHIEF + 0x10:
			case KNIGHTS_VICECHIEF + 0x10:
			case KNIGHTS_OFFICER + 0x10:
			case KNIGHTS_PUNISH + 0x10:
			case KNIGHTS_DESTROY + 0x10:
			case KNIGHTS_MEMBER_REQ + 0x10:
			case KNIGHTS_STASH + 0x10:
			case KNIGHTS_LIST_REQ + 0x10:
			case KNIGHTS_ALLLIST_REQ + 0x10:
				pMain->m_KnightsManager.ReceiveKnightsProcess(pUser, pBuf + index, command);
				break;

			case WIZ_LOGIN_INFO:
				result = GetByte(pBuf, index);
				if (result == 0x00)
					pUser->Close();
				break;

			case DB_COUPON_EVENT:
				if (pUser != nullptr)
					pUser->CouponEvent(pBuf + index);
				break;
		}

	loop_pass:
		recvlen = 0;
		memset(pBuf, 0, sizeof(pBuf));
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEbenezerDlg dialog

CEbenezerDlg::CEbenezerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CEbenezerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEbenezerDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bMMFCreate = false;
	m_hReadQueueThread = nullptr;

	m_nYear = 0;
	m_nMonth = 0;
	m_nDate = 0;
	m_nHour = 0;
	m_nMin = 0;
	m_nWeather = 0;
	m_nAmount = 0;
	m_sPartyIndex = 0;

	m_nCastleCapture = 0;

	m_bKarusFlag = 0;
	m_bElmoradFlag = 0;

	m_byKarusOpenFlag = m_byElmoradOpenFlag = 0;
	m_byBanishFlag = 0;
	m_sBanishDelay = 0;

	m_sKarusDead = 0;
	m_sElmoradDead = 0;

	for (int i = 0; i < INVASION_MONUMENT_COUNT; i++)
	{
		_karusInvasionMonumentLastCapturedNation[i] = 0;
		_elmoradInvasionMonumentLastCapturedNation[i] = 0;
	}

	m_bVictory = 0;
	m_byOldVictory = 0;
	m_byBattleSave = 0;
	m_sKarusCount = 0;
	m_sElmoradCount = 0;

	m_nBattleZoneOpenWeek = m_nBattleZoneOpenHourStart = m_nBattleZoneOpenHourEnd = 0;

	m_byBattleOpen = NO_BATTLE;
	m_byOldBattleOpen = NO_BATTLE;
	m_bFirstServerFlag = false;
	m_bPointCheckFlag = false;

	m_nServerIndex = 0;
	m_nServerNo = 0;
	m_nServerGroupNo = 0;
	m_nServerGroup = 0;
	m_iPacketCount = 0;
	m_iSendPacketCount = 0;
	m_iRecvPacketCount = 0;
	m_sDiscount = 0;

	m_pUdpSocket = nullptr;

	for (int h = 0; h < MAX_BBS_POST; h++)
	{
		m_sBuyID[h] = -1;
		memset(m_strBuyTitle[h], 0, sizeof(m_strBuyTitle[h]));
		memset(m_strBuyMessage[h], 0, sizeof(m_strBuyMessage[h]));
		m_iBuyPrice[h] = 0;
		m_fBuyStartTime[h] = 0.0f;

		m_sSellID[h] = -1;
		memset(m_strSellTitle[h], 0, sizeof(m_strSellTitle[h]));
		memset(m_strSellMessage[h], 0, sizeof(m_strSellMessage[h]));
		m_iSellPrice[h] = 0;
		m_fSellStartTime[h] = 0.0f;
	}

	memset(m_ppNotice, 0, sizeof(m_ppNotice));
	memset(m_AIServerIP, 0, sizeof(m_AIServerIP));

	m_bPermanentChatMode = false;			// 비러머글 남는 공지 --;
	m_bPermanentChatFlag = false;
	memset(m_strPermanentChat, 0, sizeof(m_strPermanentChat));

	memset(m_strKarusCaptain, 0, sizeof(m_strKarusCaptain));
	memset(m_strElmoradCaptain, 0, sizeof(m_strElmoradCaptain));

	m_bySanta = 0;		// 갓댐 산타!!! >.<

	_beefRoastVictoryType = BEEF_ROAST_VICTORY_PENDING_RESTART_AFTER_VICTORY;

	_monsterChallengeActiveType = 0;
	_monsterChallengeState = 0;
	_monsterChallengePlayerCount = 0;

	ConnectionManager::Create();
}

CEbenezerDlg::~CEbenezerDlg()
{
	ConnectionManager::Destroy();
}

void CEbenezerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEbenezerDlg)
	DDX_Control(pDX, IDC_GONGJI_EDIT, m_AnnounceEdit);
	DDX_Control(pDX, IDC_LIST1, _outputList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEbenezerDlg, CDialog)
	//{{AFX_MSG_MAP(CEbenezerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEbenezerDlg message handlers

BOOL CEbenezerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	s_pInstance = this;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	srand(static_cast<uint32_t>(time(nullptr)));

	// Compress Init
	memset(m_CompBuf, 0, sizeof(m_CompBuf));	// 압축할 데이터를 모으는 버퍼
	m_iCompIndex = 0;							// 압축할 데이터의 길이
	m_CompCount = 0;							// 압축할 데이터의 개수

	m_sZoneCount = 0;
	m_sSocketCount = 0;
	m_sErrorSocketCount = 0;
	m_KnightsManager.m_pMain = this;
	// sungyong 2002.05.23
	m_sSendSocket = 0;
	m_bFirstServerFlag = false;
	m_bServerCheckFlag = false;
	m_sReSocketCount = 0;
	m_fReConnectStart = 0.0f;
	// sungyong~ 2002.05.23
	
	InitializeCriticalSection(&g_serial_critical);
	InitializeCriticalSection(&g_region_critical);

	LoadConfig();

	m_Iocport.Init(MAX_USER, CLIENT_SOCKSIZE, 4);

	for (int i = 0; i < MAX_USER; i++)
		m_Iocport.m_SockArrayInActive[i] = new CUser();

	_ZONE_SERVERINFO* pInfo = m_ServerArray.GetData(m_nServerNo);
	if (pInfo == nullptr)
	{
		AfxMessageBox(_T("No Listen Port!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!m_Iocport.Listen(pInfo->sPort))
	{
		AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	AddOutputMessage(fmt::format("Listening on 0.0.0.0:{} - not accepting user connections yet",
		pInfo->sPort));

	if (!InitializeMMF())
	{
		AfxMessageBox(_T("Main Shared Memory Initialize Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!m_LoggerSendQueue.InitializeMMF(MAX_PKTSIZE, MAX_COUNT, _T(SMQ_LOGGERSEND)))
	{
		AfxMessageBox(_T("SMQ Send Shared Memory Initialize Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!m_LoggerRecvQueue.InitializeMMF(MAX_PKTSIZE, MAX_COUNT, _T(SMQ_LOGGERRECV)))
	{
		AfxMessageBox(_T("SMQ Recv Shared Memory Initialize Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!m_ItemLoggerSendQ.InitializeMMF(MAX_PKTSIZE, MAX_COUNT, _T(SMQ_ITEMLOGGER)))
	{
		AfxMessageBox(_T("SMQ ItemLog Shared Memory Initialize Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading ITEM table");
	if (!LoadItemTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache ITEM table, closing");
		AfxMessageBox(_T("ItemTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC table");
	if (!LoadMagicTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC table, closing");
		AfxMessageBox(_T("MagicTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE1 table");
	if (!LoadMagicType1())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE1 table, closing");
		AfxMessageBox(_T("MagicType1 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE2 table");
	if (!LoadMagicType2())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE2 table, closing");
		AfxMessageBox(_T("MagicType2 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE3 table");
	if (!LoadMagicType3())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE3 table, closing");
		AfxMessageBox(_T("MagicType3 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE4 table");
	if (!LoadMagicType4())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE4 table, closing");
		AfxMessageBox(_T("MagicType4 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE5 table");
	if (!LoadMagicType5())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE5 table, closing");
		AfxMessageBox(_T("MagicType5 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE7 table");
	if (!LoadMagicType7())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE7 table, closing");
		AfxMessageBox(_T("MagicType7 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading MAGIC_TYPE8 table");
	if (!LoadMagicType8())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache MAGIC_TYPE8 table, closing");
		AfxMessageBox(_T("MagicType8 Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading COEFFICIENT table");
	if (!LoadCoefficientTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache COEFFICIENT table, closing");
		AfxMessageBox(_T("COEFFICIENT Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading LEVEL_UP table");
	if (!LoadLevelUpTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache LEVEL_UP table, closing");
		AfxMessageBox(_T("LevelUpTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading KNIGHTS table");
	if (!LoadAllKnights())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache KNIGHTS table, closing");
		AfxMessageBox(_T("KnightsData Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading KNIGHTS_USER table");
	if (!LoadAllKnightsUserData())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache KNIGHTS_USER table, closing");
		AfxMessageBox(_T("LoadAllKnightsUserData Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading KNIGHTS_SIEGE_WARFARE table");
	if (!LoadKnightsSiegeWarfareTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache KNIGHTS_SIEGE_WARFARE table, closing");
		AfxMessageBox(_T("LoadKnightsSiegeWarfareTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading HOME table");
	if (!LoadHomeTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache HOME table, closing");
		AfxMessageBox(_T("LoadHomeTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading START_POSITION table");
	if (!LoadStartPositionTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache START_POSITION table, closing");
		AfxMessageBox(_T("LoadStartPositionTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading BATTLE table");
	if (!LoadBattleTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache BATTLE table, closing");
		AfxMessageBox(_T("LoadBattleTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading SERVER_RESOURCE table");
	if (!LoadServerResourceTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache SERVER_RESOURCE, closing");
		AfxMessageBox(_T("LoadServerResourceTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading EVENT_TRIGGER table");
	if (!LoadEventTriggerTable())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to cache EVENT_TRIGGER, closing");
		AfxMessageBox(_T("LoadEventTriggerTable Load Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	spdlog::info("EbenezerDlg::OnInitDialog: loading maps");
	if (!MapFileLoad())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to load maps, closing");
		AfxPostQuitMessage(0);
	}

	LoadNoticeData();

	DWORD id;
	m_hReadQueueThread = CreateThread(nullptr, 0, ReadQueueThread, this, 0, &id);

	m_pUdpSocket = new CUdpSocket(this);
	if (!m_pUdpSocket->CreateSocket())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to create UDP socket");
		AfxMessageBox(_T("Udp Socket Create Fail"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!AIServerConnect())
	{
		spdlog::error("EbenezerDlg::OnInitDialog: failed to connect to the AIServer");
#ifndef _DEBUG
		AfxPostQuitMessage(0);
#endif
	}

#ifdef _DEBUG
	// meant to be called when AI finishes connecting, to allow users to connect.
	// we allow user connections early in debug builds so we do not need to wait
	// for a full server load to log in
	UserAcceptThread();
#endif

	CTime cur = CTime::GetCurrentTime();
	std::wstring starttime = std::format(L"Ebenezer started: {:02}/{:02} {:02}:{:02}",
		cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute());
	AddOutputMessage(starttime);
	spdlog::info("EbenezerDlg::OnInitDialog: successfully initialized");

	return TRUE;
}

void CEbenezerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEbenezerDlg::OnPaint()
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
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEbenezerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CEbenezerDlg::DestroyWindow()
{
	KillTimer(GAME_TIME);
	KillTimer(SEND_TIME);
	KillTimer(ALIVE_TIME);
	KillTimer(MARKET_BBS_TIME);
	KillTimer(PACKET_CHECK);

	if (m_hReadQueueThread != nullptr)
		TerminateThread(m_hReadQueueThread, 0);

	if (m_bMMFCreate)
	{
		UnmapViewOfFile(m_lpMMFile);
		CloseHandle(m_hMMFile);
	}

	if (!m_ItemTableMap.IsEmpty())
		m_ItemTableMap.DeleteAllData();

	if (!m_MagicTableMap.IsEmpty())
		m_MagicTableMap.DeleteAllData();

	if (!m_MagicType1TableMap.IsEmpty())
		m_MagicType1TableMap.DeleteAllData();

	if (!m_MagicType2TableMap.IsEmpty())
		m_MagicType2TableMap.DeleteAllData();

	if (!m_MagicType3TableMap.IsEmpty())
		m_MagicType3TableMap.DeleteAllData();

	if (!m_MagicType4TableMap.IsEmpty())
		m_MagicType4TableMap.DeleteAllData();

	if (!m_MagicType5TableMap.IsEmpty())
		m_MagicType5TableMap.DeleteAllData();

	if (!m_MagicType7TableMap.IsEmpty())
		m_MagicType7TableMap.DeleteAllData();

	if (!m_MagicType8TableMap.IsEmpty())
		m_MagicType8TableMap.DeleteAllData();

	if (!m_NpcMap.IsEmpty())
		m_NpcMap.DeleteAllData();

	if (!m_AISocketMap.IsEmpty())
		m_AISocketMap.DeleteAllData();

	if (!m_PartyMap.IsEmpty())
		m_PartyMap.DeleteAllData();

	if (!m_CoefficientTableMap.IsEmpty())
		m_CoefficientTableMap.DeleteAllData();

	if (!m_KnightsMap.IsEmpty())
		m_KnightsMap.DeleteAllData();

	if (!m_ServerArray.IsEmpty())
		m_ServerArray.DeleteAllData();

	if (!m_ServerGroupArray.IsEmpty())
		m_ServerGroupArray.DeleteAllData();

	if (!m_HomeTableMap.IsEmpty())
		m_HomeTableMap.DeleteAllData();

	if (!m_StartPositionTableMap.IsEmpty())
		m_StartPositionTableMap.DeleteAllData();

	for (C3DMap* pMap : m_ZoneArray)
		delete pMap;
	m_ZoneArray.clear();

	for (model::LevelUp* pLevelUp : m_LevelUpTableArray)
		delete pLevelUp;
	m_LevelUpTableArray.clear();

	if (!m_EventMap.IsEmpty())
		m_EventMap.DeleteAllData();

	delete m_pUdpSocket;
	m_pUdpSocket = nullptr;

	s_pInstance = nullptr;

	DeleteCriticalSection(&g_serial_critical);
	DeleteCriticalSection(&g_region_critical);

	return CDialog::DestroyWindow();
}

void CEbenezerDlg::UserAcceptThread()
{
	// User Socket Accept
	ResumeThread(m_Iocport.m_hAcceptThread);
	AddOutputMessage(_T("Accepting user connections"));
}

CUser* CEbenezerDlg::GetUserPtr(const char* userid, NameType type)
{
	// Account id check....
	if (type == NameType::Account)
	{
		for (int i = 0; i < MAX_USER; i++)
		{
			CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
			if (pUser != nullptr
				&& _strnicmp(pUser->m_strAccountID, userid, MAX_ID_SIZE) == 0)
				return pUser;
		}
	}
	// character id check...
	else // if (type == NameType::Character)
	{
		for (int i = 0; i < MAX_USER; i++)
		{
			CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
			if (pUser != nullptr
				&& _strnicmp(pUser->m_pUserData->m_id, userid, MAX_ID_SIZE) == 0)
				return pUser;
		}
	}

	return nullptr;
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CEbenezerDlg::AddOutputMessage(const std::string& msg)
{
	std::wstring wMsg = LocalToWide(msg);
	AddOutputMessage(wMsg);
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CEbenezerDlg::AddOutputMessage(const std::wstring& msg)
{
	_outputList.AddString(msg.data());
	
	// Set the focus to the last item and ensure it is visible
	int lastIndex = _outputList.GetCount()-1;
	_outputList.SetTopIndex(lastIndex);
}

void CEbenezerDlg::OnTimer(UINT nIDEvent)
{
	// sungyong 2002.05.23
	int retval = 0;

	switch (nIDEvent)
	{
		case GAME_TIME:
			UpdateGameTime();

			// AIServer Socket Alive Check Routine
			if (m_bFirstServerFlag)
			{
				int count = 0;
				for (int i = 0; i < MAX_AI_SOCKET; i++)
				{
					CAISocket* pAISock = m_AISocketMap.GetData(i);
					if (pAISock != nullptr
						&& pAISock->GetState() == STATE_DISCONNECTED)
						AISocketConnect(i, true);
					else if (pAISock == nullptr)
						AISocketConnect(i, true);
					else
						count++;
				}

				if (count <= 0)
					DeleteAllNpcList();
			}
			// sungyong~ 2002.05.23
			break;

		case SEND_TIME:
			m_Iocport.m_PostOverlapped.Offset = OVL_SEND;
			retval = PostQueuedCompletionStatus(m_Iocport.m_hSendIOCPort, 0, 0, &m_Iocport.m_PostOverlapped);		
			if (!retval)
			{
				int errValue = GetLastError();
				spdlog::error("EbenezerDlg::OnTimer: PostQueuedCompletionStatus error code {}", errValue);
			}
			break;

		case ALIVE_TIME:
			CheckAliveUser();
			break;

		case MARKET_BBS_TIME:
			MarketBBSTimeCheck();
			break;

		case PACKET_CHECK:
			WritePacketLog();
			break;
	}

	CDialog::OnTimer(nIDEvent);
}

// sungyong 2002.05.22
bool CEbenezerDlg::AIServerConnect()
{
	std::string errorReason;
	for (int i = 0; i < MAX_AI_SOCKET; i++)
	{
		if (!AISocketConnect(i, false, &errorReason))
		{
			std::wstring msg = Utf8ToWide(errorReason);
			AfxMessageBox(msg.c_str());
			return false;
		}
	}

	return true;
}

bool CEbenezerDlg::AISocketConnect(int zone, bool flag, std::string* errorReason_ /*= nullptr*/)
{
	CAISocket* pAISock = nullptr;
	int send_index = 0;
	char pBuf[128] = {};
	std::string errorReason;

	//if( m_nServerNo == 3 ) return false;

	pAISock = m_AISocketMap.GetData(zone);
	if (pAISock != nullptr)
	{
		if (pAISock->GetState() != STATE_DISCONNECTED)
			return true;

		m_AISocketMap.DeleteData(zone);
	}

	pAISock = new CAISocket(zone);

	if (!pAISock->Create())
	{
		delete pAISock;

		errorReason = fmt::format("Failed to create new AI socket for zone {}", zone);
		spdlog::error("EbenezerDlg::AISocketConnect: {}", errorReason);

		if (errorReason_ != nullptr)
			*errorReason_ = errorReason;

		return false;
	}

	int port = GetAIServerPort();
	if (port < 0)
	{
		delete pAISock;

		errorReason = fmt::format("Invalid port, unsupported m_nServerNo {} (zone {})", m_nServerNo, zone);
		spdlog::error("EbenezerDlg::AISocketConnect: {}", errorReason);

		if (errorReason_ != nullptr)
			*errorReason_ = errorReason;

		return false;
	}

	if (!pAISock->Connect(&m_Iocport, m_AIServerIP, port))
	{
		delete pAISock;

		errorReason = fmt::format("Failed to connect to AI server (zone {}) ({}:{})",
			zone, m_AIServerIP, port);
		spdlog::error("EbenezerDlg::AISocketConnect: {}", errorReason);

		if (errorReason_ != nullptr)
			*errorReason_ = errorReason;

		return false;
	}

	SetByte(pBuf, AI_SERVER_CONNECT, send_index);
	SetByte(pBuf, zone, send_index);

	// 재접속
	if (flag)
		SetByte(pBuf, 1, send_index);
	// 처음 접속..
	else
		SetByte(pBuf, 0, send_index);

	pAISock->Send(pBuf, send_index);

	// 해야할일 :이 부분 처리.....
	//SendAllUserInfo();
	//m_sSocketCount = zone;
	m_AISocketMap.PutData(zone, pAISock);

	spdlog::debug("EbenezerDlg::AISocketConnect: connected to zone {}", zone);
	return true;
}
// ~sungyong 2002.05.22

int CEbenezerDlg::GetAIServerPort() const
{
	switch (m_nServerNo)
	{
		case KARUS:
			return AI_KARUS_SOCKET_PORT;

		case ELMORAD:
			return AI_ELMO_SOCKET_PORT;

		case BATTLE:
			return AI_BATTLE_SOCKET_PORT;

		default:
			return -1;
	}
}

void CEbenezerDlg::Send_All(char* pBuf, int len, CUser* pExceptUser, int nation)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser == pExceptUser)
			continue;

		if (pUser->GetState() == STATE_GAMESTART)
		{
			if (nation == 0)
				pUser->Send(pBuf, len);
			else if (nation == pUser->m_pUserData->m_bNation)
				pUser->Send(pBuf, len);
		}
	}
}

void CEbenezerDlg::Send_Region(char* pBuf, int len, int zone, int x, int z, CUser* pExceptUser, bool bDirect)
{
	C3DMap* pMap = GetMapByID(zone);
	if (pMap == nullptr)
		return;

	Send_UnitRegion(pMap, pBuf, len, x, z, pExceptUser, bDirect);
	Send_UnitRegion(pMap, pBuf, len, x - 1, z - 1, pExceptUser, bDirect);	// NW
	Send_UnitRegion(pMap, pBuf, len, x, z - 1, pExceptUser, bDirect);		// N
	Send_UnitRegion(pMap, pBuf, len, x + 1, z - 1, pExceptUser, bDirect);	// NE
	Send_UnitRegion(pMap, pBuf, len, x - 1, z, pExceptUser, bDirect);		// W
	Send_UnitRegion(pMap, pBuf, len, x + 1, z, pExceptUser, bDirect);		// E
	Send_UnitRegion(pMap, pBuf, len, x - 1, z + 1, pExceptUser, bDirect);	// SW
	Send_UnitRegion(pMap, pBuf, len, x, z + 1, pExceptUser, bDirect);		// S
	Send_UnitRegion(pMap, pBuf, len, x + 1, z + 1, pExceptUser, bDirect);	// SE
}

void CEbenezerDlg::Send_UnitRegion(C3DMap* pMap, char* pBuf, int len, int x, int z, CUser* pExceptUser, bool bDirect)
{
	if (pMap == nullptr)
		return;

	if (x < 0
		|| z < 0
		|| x > pMap->GetXRegionMax()
		|| z > pMap->GetZRegionMax())
		return;

	EnterCriticalSection(&g_region_critical);

	for (const auto& [_, pUid] : pMap->m_ppRegion[x][z].m_RegionUserArray)
	{
		int uid = *pUid;
		if (uid < 0)
			continue;

		CUser* pUser = (CUser*) m_Iocport.m_SockArray[uid];
		if (pUser == pExceptUser)
			continue;

		if (pUser != nullptr
			&& (pUser->GetState() == STATE_GAMESTART))
		{
			if (bDirect)
				pUser->Send(pBuf, len);
			else
				pUser->RegionPacketAdd(pBuf, len);
		}
	}

	LeaveCriticalSection(&g_region_critical);
}

void CEbenezerDlg::Send_NearRegion(char* pBuf, int len, int zone, int region_x, int region_z, float curx, float curz, CUser* pExceptUser)
{
	C3DMap* pMap = GetMapByID(zone);
	if (pMap == nullptr)
		return;

	int left_border = region_x * VIEW_DISTANCE, top_border = region_z * VIEW_DISTANCE;

	Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z, curx, curz, pExceptUser);

	// RIGHT
	if (((curx - left_border) > (VIEW_DISTANCE / 2.0f)))
	{
		// BOTTOM
		if (((curz - top_border) > (VIEW_DISTANCE / 2.0f)))
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x + 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z + 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x + 1, region_z + 1, curx, curz, pExceptUser);
		}
		// TOP
		else
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x + 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z - 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x + 1, region_z - 1, curx, curz, pExceptUser);
		}
	}
	// LEFT
	else
	{
		// BOTTOM
		if (((curz - top_border) > (VIEW_DISTANCE / 2.0f)))
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x - 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z + 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x - 1, region_z + 1, curx, curz, pExceptUser);
		}
		// TOP
		else
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x - 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z - 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x - 1, region_z - 1, curx, curz, pExceptUser);
		}
	}
}

void CEbenezerDlg::Send_FilterUnitRegion(C3DMap* pMap, char* pBuf, int len, int x, int z, float ref_x, float ref_z, CUser* pExceptUser)
{
	if (pMap == nullptr)
		return;

	if (x < 0
		|| z < 0
		|| x > pMap->GetXRegionMax()
		|| z > pMap->GetZRegionMax())
		return;

	EnterCriticalSection(&g_region_critical);

	auto Iter1 = pMap->m_ppRegion[x][z].m_RegionUserArray.begin();
	auto Iter2 = pMap->m_ppRegion[x][z].m_RegionUserArray.end();

	for (const auto& [_, pUid] : pMap->m_ppRegion[x][z].m_RegionUserArray)
	{
		int uid = *pUid;
		if (uid < 0)
			continue;

		CUser* pUser = (CUser*) m_Iocport.m_SockArray[uid];
		if (pUser == pExceptUser)
			continue;

		if (pUser != nullptr
			&& pUser->GetState() == STATE_GAMESTART)
		{
			double fDist = sqrt(
				pow((pUser->m_pUserData->m_curx - ref_x), 2)
				+ pow((pUser->m_pUserData->m_curz - ref_z), 2));
			if (fDist < 32)
				pUser->RegionPacketAdd(pBuf, len);
		}
	}

	LeaveCriticalSection(&g_region_critical);
}

void CEbenezerDlg::Send_PartyMember(int party, char* pBuf, int len)
{
	if (party < 0)
		return;

	_PARTY_GROUP* pParty = m_PartyMap.GetData(party);
	if (pParty == nullptr)
		return;

	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] == -1
			|| pParty->uid[i] >= MAX_USER)
			continue;

		CUser* pUser = (CUser*) m_Iocport.m_SockArray[pParty->uid[i]];
		if (pUser != nullptr)
			pUser->Send(pBuf, len);
	}
}

void CEbenezerDlg::Send_KnightsMember(int index, char* pBuf, int len, int zone)
{
	if (index <= 0)
		return;

	CKnights* pKnights = m_KnightsMap.GetData(index);
	if (pKnights == nullptr)
		return;

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bKnights != index)
			continue;

		if (zone != 100
			&& pUser->m_pUserData->m_bZone != zone)
			continue;

		pUser->Send(pBuf, len);
	}
}

// sungyong 2002.05.22
void CEbenezerDlg::Send_AIServer(int zone, char* pBuf, int len)
{
	for (int i = 0; i < MAX_AI_SOCKET; i++)
	{
		CAISocket* pSocket = m_AISocketMap.GetData(i);
		if (pSocket == nullptr)
		{
			m_sSendSocket++;
			if (m_sSendSocket >= MAX_AI_SOCKET)
				m_sSendSocket = 0;

			continue;
		}

		if (i == m_sSendSocket)
		{
			int send_size = pSocket->Send(pBuf, len);
			int old_send_socket = m_sSendSocket;
			m_sSendSocket++;
			if (m_sSendSocket >= MAX_AI_SOCKET)
				m_sSendSocket = 0;
			if (send_size == 0)
				continue;

			//TRACE(_T(" <--- Send_AIServer : length = %d, socket = %d \n"), send_size, old_send_socket);
			return;
		}
	}
}
// ~sungyong 2002.05.22

bool CEbenezerDlg::InitializeMMF()
{
	bool bCreate = true;

	uint32_t filesize = MAX_USER * ALLOCATED_USER_DATA_BLOCK;
	m_hMMFile = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, filesize, _T("KNIGHT_DB"));

	if (m_hMMFile != nullptr
		&& GetLastError() == ERROR_ALREADY_EXISTS)
	{
		m_hMMFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, _T("KNIGHT_DB"));
		if (m_hMMFile == nullptr)
		{
			m_hMMFile = INVALID_HANDLE_VALUE;
			return false;
		}

		bCreate = false;
	}

	AddOutputMessage(_T("Shared memory created successfully"));

	m_lpMMFile = (char*) MapViewOfFile(m_hMMFile, FILE_MAP_WRITE, 0, 0, 0);
	if (m_lpMMFile == nullptr)
		return false;

	memset(m_lpMMFile, 0, filesize);

	m_bMMFCreate = bCreate;

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArrayInActive[i];
		if (pUser != nullptr)
			pUser->m_pUserData = (_USER_DATA*) (m_lpMMFile + i * ALLOCATED_USER_DATA_BLOCK);
	}

	return true;
}

bool CEbenezerDlg::MapFileLoad()
{
	using ModelType = model::ZoneInfo;

	bool loaded = false;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([&](db::ModelRecordSet<ModelType>& recordset)
	{
		CString szFullPath, errormsg;

		m_ZoneArray.reserve(10);

		// Build the base MAP directory
		std::filesystem::path mapDir(GetProgPath().GetString());
		mapDir /= MAP_DIR;

		// Resolve it to strip the relative references to be nice.
		if (std::filesystem::exists(mapDir))
			mapDir = std::filesystem::canonical(mapDir);

		do
		{
			ModelType row = {};
			recordset.get_ref(row);

			std::filesystem::path mapPath
				= mapDir / row.Name;

			szFullPath.Format(_T("%ls"), mapPath.c_str());

			CFile file;
			if (!file.Open(szFullPath, CFile::modeRead))
			{
				errormsg.Format(_T("File Open Fail - %s\n"), szFullPath);
				AfxMessageBox(errormsg);
				return;
			}

			C3DMap* pMap = new C3DMap();

			pMap->m_nServerNo = row.ServerId;
			pMap->m_nZoneNumber = row.ZoneId;
			pMap->m_fInitX = (float) (row.InitX / 100.0);
			pMap->m_fInitZ = (float) (row.InitZ / 100.0);
			pMap->m_fInitY = (float) (row.InitY / 100.0);
			pMap->m_bType = row.Type;

			if (!pMap->LoadMap(file.m_hFile))
			{
				errormsg.Format(_T("Map Load Fail - %s\n"), szFullPath);
				AfxMessageBox(errormsg);
				delete pMap;
				return;
			}

			file.Close();

			m_ZoneArray.push_back(pMap);

			// 스트립트를 읽어 들인다.
			EVENT* pEvent = new EVENT;
			if (!pEvent->LoadEvent(row.ZoneId))
			{
				delete pEvent;
				continue;
			}

			if (!m_EventMap.PutData(pEvent->m_Zone, pEvent))
				delete pEvent;
		}
		while (recordset.next());

		loaded = true;
	});

	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return loaded;
}

void CEbenezerDlg::ReportTableLoadError(const recordset_loader::Error& err, const char* source)
{
	std::string error = fmt::format("EbenezerDlg::ReportTableLoadError: {} failed: {}",
		source, err.Message);
	std::wstring werror = LocalToWide(error);
	AfxMessageBox(werror.c_str());
	spdlog::error(error);
}

bool CEbenezerDlg::LoadItemTable()
{
	recordset_loader::STLMap loader(m_ItemTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicTable()
{
	recordset_loader::STLMap loader(m_MagicTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType1()
{
	recordset_loader::STLMap loader(m_MagicType1TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType2()
{
	recordset_loader::STLMap loader(m_MagicType2TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType3()
{
	recordset_loader::STLMap loader(m_MagicType3TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType4()
{
	recordset_loader::STLMap loader(m_MagicType4TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType5()
{
	recordset_loader::STLMap loader(m_MagicType5TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType7()
{
	recordset_loader::STLMap loader(m_MagicType7TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadMagicType8()
{
	recordset_loader::STLMap loader(m_MagicType8TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadCoefficientTable()
{
	recordset_loader::STLMap loader(m_CoefficientTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadLevelUpTable()
{
	recordset_loader::Vector<model::LevelUp> loader(m_LevelUpTableArray);
	if (!loader.Load_ForbidEmpty(true))
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

void CEbenezerDlg::LoadConfig()
{
	int year = 0, month = 0, date = 0, hour = 0, serverCount = 0, sgroup_count = 0;
	std::string key;

	CString exePath(GetProgPath());
	std::string exePathUtf8(CT2A(exePath, CP_UTF8));

	std::filesystem::path iniPath(exePath.GetString());
	iniPath /= L"gameserver.ini";

	m_Ini.Load(iniPath);

	_logger.Setup(m_Ini, exePathUtf8);
	
	m_nYear = m_Ini.GetInt("TIMER", "YEAR", 1);
	m_nMonth = m_Ini.GetInt("TIMER", "MONTH", 1);
	m_nDate = m_Ini.GetInt("TIMER", "DATE", 1);
	m_nHour = m_Ini.GetInt("TIMER", "HOUR", 1);
	m_nWeather = m_Ini.GetInt("TIMER", "WEATHER", 1);

//	m_nBattleZoneOpenWeek  = m_Ini.GetInt("BATTLE", "WEEK", 3);
	m_nBattleZoneOpenWeek = m_Ini.GetInt("BATTLE", "WEEK", 5);
	m_nBattleZoneOpenHourStart = m_Ini.GetInt("BATTLE", "START_TIME", 20);
	m_nBattleZoneOpenHourEnd = m_Ini.GetInt("BATTLE", "END_TIME", 0);

	std::string datasourceName = m_Ini.GetString("ODBC", "GAME_DSN", "KN_online");
	std::string datasourceUser = m_Ini.GetString("ODBC", "GAME_UID", "knight");
	std::string datasourcePass = m_Ini.GetString("ODBC", "GAME_PWD", "knight");

	ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	m_Ini.GetString("AI_SERVER", "IP", "127.0.0.1", m_AIServerIP, _countof(m_AIServerIP));

	// NOTE: officially this is required to be explicitly set, so it defaults to 0 and fails.
	m_nServerIndex = m_Ini.GetInt("SG_INFO", "SERVER_INDEX", 1);

	m_nCastleCapture = m_Ini.GetInt("CASTLE", "NATION", 1);
	m_nServerNo = m_Ini.GetInt("ZONE_INFO", "MY_INFO", 1);
	m_nServerGroup = m_Ini.GetInt("ZONE_INFO", "SERVER_NUM", 0);
	serverCount = m_Ini.GetInt("ZONE_INFO", "SERVER_COUNT", 1);
	if (serverCount < 1)
	{
		AfxMessageBox(_T("SERVER_COUNT must be greater than 0"));
		spdlog::error("EbenezerDlg::LoadConfig: invalid SERVER_COUNT={}", serverCount);
		return;
	}

	for (int i = 0; i < serverCount; i++)
	{
		_ZONE_SERVERINFO* pInfo = new _ZONE_SERVERINFO;

		key = fmt::format("SERVER_{:02}", i);
		pInfo->sServerNo = m_Ini.GetInt("ZONE_INFO", key, 1);

		key = fmt::format("SERVER_IP_{:02}", i);
		m_Ini.GetString("ZONE_INFO", key, "127.0.0.1", pInfo->strServerIP, _countof(pInfo->strServerIP));

		pInfo->sPort = _LISTEN_PORT + pInfo->sServerNo;

		m_ServerArray.PutData(pInfo->sServerNo, pInfo);
	}

	if (m_nServerGroup != 0)
	{
		m_nServerGroupNo = m_Ini.GetInt("SG_INFO", "GMY_INFO", 1);
		sgroup_count = m_Ini.GetInt("SG_INFO", "GSERVER_COUNT", 1);
		if (sgroup_count < 1)
		{
			AfxMessageBox(_T("Server group count error!!"));
			return;
		}

		for (int i = 0; i < sgroup_count; i++)
		{
			_ZONE_SERVERINFO* pInfo = new _ZONE_SERVERINFO;

			key = fmt::format("GSERVER_{:02}", i);
			pInfo->sServerNo = m_Ini.GetInt("SG_INFO", key, 1);

			key = fmt::format("GSERVER_IP_{:02}", i);
			m_Ini.GetString("SG_INFO", key, "127.0.0.1", pInfo->strServerIP, _countof(pInfo->strServerIP));

			pInfo->sPort = _LISTEN_PORT + pInfo->sServerNo;

			m_ServerGroupArray.PutData(pInfo->sServerNo, pInfo);
		}
	}

	// Trigger a save to flush defaults to file.
	m_Ini.Save();

	SetTimer(GAME_TIME, 6000, nullptr);
	SetTimer(SEND_TIME, 200, nullptr);
	SetTimer(ALIVE_TIME, 34000, nullptr);
	SetTimer(MARKET_BBS_TIME, 300000, nullptr);
	SetTimer(PACKET_CHECK, 360000, nullptr);
}

void EbenezerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool,
	const std::string& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::EbenezerEvent, ini::EVENT_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::EbenezerRegion, ini::REGION_LOG_FILE);
}

void CEbenezerDlg::UpdateGameTime()
{
	CUser* pUser = nullptr;
	bool bKnights = false;

	m_nMin++;

	BattleZoneOpenTimer();	// Check if it's time for the BattleZone to open or end.

	if (m_nMin == 60)
	{
		m_nHour++;
		m_nMin = 0;

		UpdateWeather();
		SetGameTime();

		//  갓댐 산타!! >.<
		if (m_bySanta != 0)
			FlySanta();
		//
	}

	if (m_nHour == 24)
	{
		m_nDate++;
		m_nHour = 0;
		bKnights = true;
	}

	if (m_nDate == 31)
	{
		m_nMonth++;
		m_nDate = 1;
	}

	if (m_nMonth == 13)
	{
		m_nYear++;
		m_nMonth = 1;
	}

	// ai status check packet...
	m_sErrorSocketCount++;

	int send_index = 0;
	char pSendBuf[256] = {};
	//SetByte(pSendBuf, AG_CHECK_ALIVE_REQ, send_index);
	//Send_AIServer(1000, pSendBuf, send_index);

	// 시간과 날씨 정보를 보낸다..
	memset(pSendBuf, 0, sizeof(pSendBuf));
	send_index = 0;
	SetByte(pSendBuf, AG_TIME_WEATHER, send_index);
	SetShort(pSendBuf, m_nYear, send_index);				// time info
	SetShort(pSendBuf, m_nMonth, send_index);
	SetShort(pSendBuf, m_nDate, send_index);
	SetShort(pSendBuf, m_nHour, send_index);
	SetShort(pSendBuf, m_nMin, send_index);
	SetByte(pSendBuf, (uint8_t) m_nWeather, send_index);		// weather info
	SetShort(pSendBuf, m_nAmount, send_index);
	Send_AIServer(1000, pSendBuf, send_index);

	if (bKnights)
	{
		memset(pSendBuf, 0, sizeof(pSendBuf));
		send_index = 0;
		SetByte(pSendBuf, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(pSendBuf, KNIGHTS_ALLLIST_REQ + 0x10, send_index);
		SetByte(pSendBuf, m_nServerNo, send_index);
		m_LoggerSendQueue.PutData(pSendBuf, send_index);
	}
}

void CEbenezerDlg::UpdateWeather()
{
	int weather = 0, result = 0, send_index = 0;
	char send_buff[256] = {};

	result = myrand(0, 100);

//	if (result < 5)
	if (result < 2)
		weather = WEATHER_SNOW;
//	else if (result < 15)
	else if (result < 7)
		weather = WEATHER_RAIN;
	else
		weather = WEATHER_FINE;

	m_nAmount = myrand(0, 100);

	// WEATHER_FINE 일때 m_nAmount 는 안개 정도 표시
	if (weather == WEATHER_FINE)
	{
		if (m_nAmount > 70)
			m_nAmount = m_nAmount / 2;
		else
			m_nAmount = 0;
	}

	m_nWeather = weather;

	SetByte(send_buff, WIZ_WEATHER, send_index);
	SetByte(send_buff, (uint8_t) m_nWeather, send_index);
	SetShort(send_buff, m_nAmount, send_index);
	Send_All(send_buff, send_index);
}

void CEbenezerDlg::SetGameTime()
{
	m_Ini.SetInt("TIMER", "YEAR", m_nYear);
	m_Ini.SetInt("TIMER", "MONTH", m_nMonth);
	m_Ini.SetInt("TIMER", "DATE", m_nDate);
	m_Ini.SetInt("TIMER", "HOUR", m_nHour);
	m_Ini.SetInt("TIMER", "WEATHER", m_nWeather);
	m_Ini.Save();
}

void CEbenezerDlg::UserInOutForMe(CUser* pSendUser)
{
	int send_index = 0, buff_index = 0, i = 0, j = 0, t_count = 0, prevIndex = 0;
	C3DMap* pMap = nullptr;
	int region_x = -1, region_z = -1, user_count = 0, uid = -1;
	char buff[16384] = {}, send_buff[49152] = {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	send_index = 3;		// packet command 와 user_count 를 나중에 셋팅한다...
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ;			// CENTER
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH WEST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ - 1;		// NORTH
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH EAST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ;		// WEST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ;		// EAST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;
	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH WEST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;
	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ + 1;		// SOUTH
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);
	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH EAST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex = buff_index + send_index;
	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerDlg::UserInOutForMe: buffer overflow [prevIndex={}, line={}]", prevIndex, __LINE__);
		return;
	}

	SetString(send_buff, buff, buff_index, send_index);

	int temp_index = 0;
	SetByte(send_buff, WIZ_REQ_USERIN, temp_index);
	SetShort(send_buff, t_count, temp_index);

	pSendUser->SendCompressingPacket(send_buff, send_index);
}

void CEbenezerDlg::RegionUserInOutForMe(CUser* pSendUser)
{
	int send_index = 0, buff_index = 0, i = 0, j = 0, t_count = 0;
	C3DMap* pMap = nullptr;
	int region_x = -1, region_z = -1, userCount = 0, uid_sendindex = 0;
	char uid_buff[2048] = {}, send_buff[16384] = {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	uid_sendindex = 3;	// packet command 와 user_count 는 나중에 셋팅한다...

	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ;			// CENTER
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH WEST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ - 1;		// NORTH
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH EAST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ;		// WEST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ;		// EAST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH WEST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ + 1;		// SOUTH
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH EAST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(send_buff, uid_buff, buff_index, uid_sendindex);

	int temp_index = 0;
	SetByte(send_buff, WIZ_REGIONCHANGE, temp_index);
	SetShort(send_buff, userCount, temp_index);

	pSendUser->Send(send_buff, uid_sendindex);

	if (userCount > 500)
		spdlog::debug("EbenezerDlg::RegionUserInOutForMe: userCount={}", userCount);
}

int CEbenezerDlg::GetRegionUserIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count)
{
	if (pMap == nullptr)
		return 0;

	if (region_x < 0
		|| region_z < 0
		|| region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	EnterCriticalSection(&g_region_critical);

	for (const auto& [_, pUid] : pMap->m_ppRegion[region_x][region_z].m_RegionUserArray)
	{
		int uid = *pUid;
		if (uid < 0)
			continue;

		CUser* pUser = (CUser*) m_Iocport.m_SockArray[uid];
		if (pUser == nullptr)
			continue;

		if (pUser->m_RegionX != region_x
			|| pUser->m_RegionZ != region_z)
			continue;

		if (pUser->GetState() != STATE_GAMESTART)
			continue;

		SetShort(buff, pUser->GetSocketID(), buff_index);
		pUser->GetUserInfo(buff, buff_index);

		++t_count;
	}

	LeaveCriticalSection(&g_region_critical);

	return buff_index;
}

int CEbenezerDlg::GetRegionUserList(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count)
{
	if (pMap == nullptr)
		return 0;

	if (region_x < 0
		|| region_z < 0
		|| region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	EnterCriticalSection(&g_region_critical);

	for (const auto& [_, pUid] : pMap->m_ppRegion[region_x][region_z].m_RegionUserArray)
	{
		int uid = *pUid;
		if (uid < 0)
			continue;

		CUser* pUser = (CUser*) m_Iocport.m_SockArray[uid];
		if (pUser != nullptr
			&& pUser->GetState() == STATE_GAMESTART)
		{
			SetShort(buff, pUser->GetSocketID(), buff_index);
			t_count++;
		}
	}

	LeaveCriticalSection(&g_region_critical);

	return buff_index;
}

void CEbenezerDlg::NpcInOutForMe(CUser* pSendUser)
{
	int send_index = 0, buff_index = 0, t_count = 0;
	C3DMap* pMap = nullptr;
	int region_x = -1, region_z = -1;
	char buff[8192] = {}, send_buff[32768] = {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	send_index = 3;		// packet command 와 user_count 를 나중에 셋팅한다...
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ;			// CENTER
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH WEST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ - 1;		// NORTH
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH EAST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ;		// WEST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ;		// EAST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH WEST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ + 1;		// SOUTH
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	memset(buff, 0, sizeof(buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH EAST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(send_buff, buff, buff_index, send_index);

	int temp_index = 0;
	SetByte(send_buff, WIZ_REQ_NPCIN, temp_index);
	SetShort(send_buff, t_count, temp_index);

	pSendUser->SendCompressingPacket(send_buff, send_index);
}

int CEbenezerDlg::GetRegionNpcIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count)
{
	// 포인터 참조하면 안됨
	if (!m_bPointCheckFlag)
		return 0;

	if (pMap == nullptr)
		return 0;

	if (region_x < 0
		|| region_z < 0
		|| region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	EnterCriticalSection(&g_region_critical);

	//string.Format("---- GetRegionNpcIn , x=%d, z=%d ----\r\n", region_x, region_z);
	//EnterCriticalSection( &g_LogFile_critical );
	//m_RegionLogFile.Write( string, string.GetLength() );
	//LeaveCriticalSection( &g_LogFile_critical );

	for (const auto& [_, pNid] : pMap->m_ppRegion[region_x][region_z].m_RegionNpcArray)
	{
		int nid = *pNid;
		if (nid < 0)
			continue;

		CNpc* pNpc = m_NpcMap.GetData(nid);
		if (pNpc == nullptr)
			continue;

		if (pNpc->m_sRegion_X != region_x
			|| pNpc->m_sRegion_Z != region_z)
			continue;

		SetShort(buff, pNpc->m_sNid, buff_index);
		pNpc->GetNpcInfo(buff, buff_index);

		t_count++;

		//string.Format("nid=%d, name=%s, count=%d \r\n", pNpc->m_sNid, pNpc->m_strName, t_count);
		//EnterCriticalSection( &g_LogFile_critical );
		//m_RegionLogFile.Write( string, string.GetLength() );
		//LeaveCriticalSection( &g_LogFile_critical );

		//if( pNpc->m_sCurZone > 100 ) 
		//	TRACE(_T("GetRegionNpcIn rx=%d, rz=%d, nid=%d, name=%hs, count=%d\n"), region_x, region_z, pNpc->m_sNid, pNpc->m_strName, t_count);
	}

	LeaveCriticalSection(&g_region_critical);

	return buff_index;
}

void CEbenezerDlg::RegionNpcInfoForMe(CUser* pSendUser, int nType)
{
	int send_index = 0, buff_index = 0, i = 0, j = 0, t_count = 0;
	C3DMap* pMap = nullptr;
	int region_x = -1, region_z = -1, npcCount = 0, nid_sendindex = 0;
	char nid_buff[1024] = {}, send_buff[8192] = {};
	CString string;

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	nid_sendindex = 3;	// packet command 와 user_count 는 나중에 셋팅한다...

	// test
	if (nType == 1)
	{
		spdlog::get(logger::EbenezerRegion)->info("RegionNpcInfoForMe start: charId={} x={} z={}",
			pSendUser->m_pUserData->m_id, pSendUser->m_RegionX, pSendUser->m_RegionZ);
	}

	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ;			// CENTER
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH WEST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ - 1;		// NORTH
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ - 1;	// NORTH EAST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ;		// WEST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ;		// EAST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX - 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH WEST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX;	region_z = pSendUser->m_RegionZ + 1;		// SOUTH
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x = pSendUser->m_RegionX + 1;	region_z = pSendUser->m_RegionZ + 1;	// SOUTH EAST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(send_buff, nid_buff, buff_index, nid_sendindex);

	int temp_index = 0;

	// test 
	if (nType == 1)
	{
		SetByte(send_buff, WIZ_TEST_PACKET, temp_index);
		spdlog::get(logger::EbenezerRegion)->info("RegionNpcInfoForMe end: charId={} x={} z={} count={}",
			pSendUser->m_pUserData->m_id, pSendUser->m_RegionX, pSendUser->m_RegionZ, npcCount);
	}
	else
	{
		SetByte(send_buff, WIZ_NPC_REGION, temp_index);
	}

	SetShort(send_buff, npcCount, temp_index);

	pSendUser->Send(send_buff, nid_sendindex);

	if (npcCount > 500)
		spdlog::debug("EbenezerDlg::RegionNpcInfoForMe: npcCount={}", npcCount);
}

int CEbenezerDlg::GetRegionNpcList(C3DMap* pMap, int region_x, int region_z, char* nid_buff, int& t_count, int nType)
{
	// 포인터 참조하면 안됨
	if (!m_bPointCheckFlag)
		return 0;

	if (pMap == nullptr)
		return 0;

	if (region_x < 0
		|| region_z < 0
		|| region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	EnterCriticalSection(&g_region_critical);

	if (nType == 1)
	{
		spdlog::get(logger::EbenezerRegion)->info("GetRegionNpcList: x={} z={}",
			region_x, region_z);
	}

	for (const auto& [_, pNid] : pMap->m_ppRegion[region_x][region_z].m_RegionNpcArray)
	{
		int npcId = *pNid;
		if (npcId < 0)
			continue;

		CNpc* pNpc = m_NpcMap.GetData(npcId);
		//if( pNpc && (pNpc->m_NpcState == NPC_LIVE ) ) {  // 수정할 것,,
		if (pNpc != nullptr)
		{
			SetShort(nid_buff, pNpc->m_sNid, buff_index);
			t_count++;
			if (nType == 1)
			{
				spdlog::get(logger::EbenezerRegion)->info("GetRegionNpcList: serial={}",
					pNpc->m_sNid);
			}
		}
		else if (nType == 1)
		{
			spdlog::get(logger::EbenezerRegion)->error("GetRegionNpcList: not found: npcId={}",
				npcId);
		}
	}

	LeaveCriticalSection(&g_region_critical);

	return buff_index;
}

int CEbenezerDlg::GetZoneIndex(int zonenumber)
{
	for (size_t i = 0; i < m_ZoneArray.size(); i++)
	{
		C3DMap* pMap = m_ZoneArray[i];
		if (pMap != nullptr
			&& zonenumber == pMap->m_nZoneNumber)
			return static_cast<int>(i);
	}

	return -1;
}

BOOL CEbenezerDlg::PreTranslateMessage(MSG* pMsg)
{
	char buff[1024] = {};
	char chatstr[256] = {}, killstr[256] = {};
	int chatlen = 0, buffindex = 0;

	std::string buff2;
//
	bool permanent_off = false;
//
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			::GetWindowTextA(m_AnnounceEdit.GetSafeHwnd(), chatstr, 256);

			UpdateData(TRUE);
			chatlen = strlen(chatstr);
			if (chatlen == 0)
				return TRUE;

			m_AnnounceEdit.SetWindowText(_T(""));
			UpdateData(FALSE);

			OperationMessage opMessage(this, nullptr);
			if (opMessage.Process(chatstr))
				return TRUE;

			// 비러머글 남는 공지 --;
			if (_strnicmp("/permanent", chatstr, 10) == 0)
			{
				m_bPermanentChatMode = true;
				m_bPermanentChatFlag = true;
				return TRUE;
			}

			if (_strnicmp("/offpermanent", chatstr, 13) == 0)
			{
				m_bPermanentChatMode = false;
				m_bPermanentChatFlag = false;
				permanent_off = true;
//				return TRUE;	//이것은 고의적으로 TRUE를 뺐었음
			}
//

			std::string finalstr;

			// 비러머글 남는 공지		
			if (m_bPermanentChatFlag)
				finalstr = fmt::format("- {} -", chatstr);
			else
				finalstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);

			//
			SetByte(buff, WIZ_CHAT, buffindex);
			// SetByte( buff, PUBLIC_CHAT, buffindex );

			// 비러머글 남는 공지
			if (permanent_off)
			{
				SetByte(buff, END_PERMANENT_CHAT, buffindex);
			}
			else if (!m_bPermanentChatFlag)
			{
				SetByte(buff, PUBLIC_CHAT, buffindex);
			}
			else
			{
				SetByte(buff, PERMANENT_CHAT, buffindex);
				strcpy(m_strPermanentChat, finalstr.c_str());
				m_bPermanentChatFlag = false;
			}
//
			SetByte(buff, 0x01, buffindex);		// nation
			SetShort(buff, -1, buffindex);		// sid
			SetByte(buff, 0, buffindex);		// sender name length
			SetString2(buff, finalstr, buffindex);
			Send_All(buff, buffindex);

			buffindex = 0;
			memset(buff, 0x00, 1024);
			SetByte(buff, STS_CHAT, buffindex);
			SetString2(buff, finalstr, buffindex);

			for (const auto& [_, pInfo] : m_ServerArray)
			{
				if (pInfo != nullptr
					&& pInfo->sServerNo != m_nServerNo)
					m_pUdpSocket->SendUDPPacket(pInfo->strServerIP, buff, buffindex);
			}

			return TRUE;
		}

		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	if (pMsg->wParam == VK_F8)
		SyncTest(1);
	else if (pMsg->wParam == VK_F9)
		SyncTest(2);

	return CDialog::PreTranslateMessage(pMsg);
}

bool CEbenezerDlg::LoadNoticeData()
{
	CString ProgPath = GetProgPath();
	CString NoticePath = ProgPath + "Notice.txt";
	CString buff;
	CStdioFile txt_file;
	int count = 0;

	if (!txt_file.Open(NoticePath, CFile::modeRead))
	{
#if !defined(_DEBUG)
		AfxMessageBox(_T("cannot open Notice.txt!!"));
#endif
		spdlog::warn("EbenezerDlg::LoadNoticeData: failed to open Notice.txt");
		return false;
	}

	while (txt_file.ReadString(buff))
	{
		if (count > 19)
		{
			AfxMessageBox(_T("Notice Count Overflow!!"));
			spdlog::error("EbenezerDlg::LoadNoticeData: notice count overflow [count={}]", count);
		}

		strcpy(m_ppNotice[count], CT2A(buff));
		count++;
	}

	txt_file.Close();

	return true;
}

void CEbenezerDlg::SyncTest(int nType)
{
	char strPath[100] = {};
	if (nType == 1)
		strcpy(strPath, "c:\\userlist.txt");
	else if (nType == 2)
		strcpy(strPath, "c:\\npclist.txt");

	FILE* stream = fopen(strPath, "w");
	if (stream == nullptr)
		return;

	int len = 0;
	char pBuf[256] = {};

	SetByte(pBuf, AG_CHECK_ALIVE_REQ, len);

	for (C3DMap* pMap : m_ZoneArray)
	{
		if (pMap == nullptr)
			continue;

		CAISocket* pSocket = m_AISocketMap.GetData(pMap->m_nZoneNumber);
		if (pSocket == nullptr)
			continue;

		int size = pSocket->Send(pBuf, len);
		fprintf(stream, "size=%d, zone=%d, number=%d\n", size, pSocket->m_iZoneNum, pMap->m_nZoneNumber);

		//return;
	}

	fprintf(stream, "*****   Region List  *****\n");

	for (C3DMap* pMap : m_ZoneArray)
	{
		//if (k != 2) continue;		// 201 존만 체크..
		if (pMap == nullptr)
			continue;

		for (int i = 0; i < pMap->GetXRegionMax(); i++)
		{
			for (int j = 0; j < pMap->GetZRegionMax(); j++)
			{
				EnterCriticalSection(&g_region_critical);
				int total_user = pMap->m_ppRegion[i][j].m_RegionUserArray.GetSize();
				int total_mon = pMap->m_ppRegion[i][j].m_RegionNpcArray.GetSize();
				LeaveCriticalSection(&g_region_critical);

				if (total_user > 0
					|| total_mon > 0)
				{
					fprintf(stream, "rx=%d, rz=%d, user=%d, monster=%d\n", i, j, total_user, total_mon);
					SyncRegionTest(pMap, i, j, stream, nType);
				}
			}
		}
	}

	fclose(stream);
}

void CEbenezerDlg::SyncRegionTest(C3DMap* pMap, int rx, int rz, FILE* pfile, int nType)
{
	fprintf(pfile, "ZONE=%d, [%d,%d] : ", pMap->m_nZoneNumber, rx, rz);

	std::map<int, int*>::iterator		Iter1;
	std::map<int, int*>::iterator		Iter2;

	EnterCriticalSection(&g_region_critical);

	if (nType == 2)
	{
		Iter1 = pMap->m_ppRegion[rx][rz].m_RegionNpcArray.begin();
		Iter2 = pMap->m_ppRegion[rx][rz].m_RegionNpcArray.end();
	}
	else if (nType == 1)
	{
		Iter1 = pMap->m_ppRegion[rx][rz].m_RegionUserArray.begin();
		Iter2 = pMap->m_ppRegion[rx][rz].m_RegionUserArray.end();
	}

	for (; Iter1 != Iter2; Iter1++)
	{
		int nid = *Iter1->second;
		if (nType == 1)
		{
			CUser* pUser = (CUser*) m_Iocport.m_SockArray[nid];
			if (pUser == nullptr)
			{
				spdlog::error("EbenezerDlg::SyncRegionTest: userId={} not found", nid);
				fprintf(pfile, "%d(fail)	", nid);
				continue;
			}

			fprintf(pfile, "%d(%d,%d)	", nid, (int) pUser->m_pUserData->m_curx, (int) pUser->m_pUserData->m_curz);
		}
		else if (nType == 2)
		{
			CNpc* pNpc = m_NpcMap.GetData(nid);
			if (pNpc == nullptr)
			{
				spdlog::error("EbenezerDlg::SyncRegionTest: npcId={} not found", nid);
				fprintf(pfile, "%d(fail)	", nid);
				continue;
			}

			fprintf(pfile, "%d(%d,%d)	", nid, (int) pNpc->m_fCurX, (int) pNpc->m_fCurZ);
		}
	}

	fprintf(pfile, "\n");

	LeaveCriticalSection(&g_region_critical);
}

void CEbenezerDlg::SendAllUserInfo()
{
	int send_index = 0;
	char send_buff[2048] = {};

	SetByte(send_buff, AG_SERVER_INFO, send_index);
	SetByte(send_buff, SERVER_INFO_START, send_index);
	Send_AIServer(1000, send_buff, send_index);

	int count = 0;
	send_index = 2;
	memset(send_buff, 0, sizeof(send_buff));
	int send_count = 0;
	int send_tot = 0;
	int tot = 20;

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser != nullptr)
		{
			pUser->SendUserInfo(send_buff, send_index);
			count++;
			if (count == tot)
			{
				SetByte(send_buff, AG_USER_INFO_ALL, send_count);
				SetByte(send_buff, (uint8_t) count, send_count);
				m_CompCount++;
				memset(m_CompBuf, 0, sizeof(m_CompBuf));
				memcpy(m_CompBuf, send_buff, send_index);
				m_iCompIndex = send_index;
				SendCompressedData();
				send_index = 2;
				send_count = 0;
				count = 0;
				send_tot++;
				//TRACE(_T("AllNpcInfo - send_count=%d, count=%d\n"), send_tot, count);
				memset(send_buff, 0, sizeof(send_buff));
				//Sleep(320);
			}
		}
	}

	if (count != 0
		&& count < (tot - 1))
	{
		send_count = 0;
		SetByte(send_buff, AG_USER_INFO_ALL, send_count);
		SetByte(send_buff, (uint8_t) count, send_count);
		Send_AIServer(1000, send_buff, send_index);
		send_tot++;
		//TRACE(_T("AllNpcInfo - send_count=%d, count=%d\n"), send_tot, count);
		//Sleep(1);
	}

	// 파티에 대한 정보도 보내도록 한다....
	EnterCriticalSection(&g_region_critical);

	for (int i = 0; i < m_PartyMap.GetSize(); i++)
	{
		_PARTY_GROUP* pParty = m_PartyMap.GetData(i);
		if (pParty == nullptr)
			continue;

		send_index = 0;
		ZeroMemory(send_buff, sizeof(send_buff));
		SetByte(send_buff, AG_PARTY_INFO_ALL, send_index);
		SetShort(send_buff, i, send_index);					// 파티 번호
		//if( i == pParty->wIndex )
		for (int j = 0; j < 8; j++)
		{
			SetShort(send_buff, pParty->uid[j], send_index);				// 유저 번호
			//SetShort(send_buff, pParty->sHp[j], send_index );				// HP
			//SetByte(send_buff, pParty->bLevel[j], send_index );				// Level
			//SetShort(send_buff, pParty->sClass[j], send_index );			// Class
		}

		Send_AIServer(1000, send_buff, send_index);
	}

	LeaveCriticalSection(&g_region_critical);

	send_index = 0;
	ZeroMemory(send_buff, sizeof(send_buff));
	SetByte(send_buff, AG_SERVER_INFO, send_index);
	SetByte(send_buff, SERVER_INFO_END, send_index);
	Send_AIServer(1000, send_buff, send_index);

	spdlog::trace("EbenezerDlg::SendAllUserInfo: completed");
}

void CEbenezerDlg::SendCompressedData()
{
	if (m_CompCount <= 0
		|| m_iCompIndex <= 0)
	{
		m_CompCount = 0;
		m_iCompIndex = 0;
		return;
	}

	int send_index = 0;
	char send_buff[32000] = {};
	uint8_t comp_buff[32000] = {};
	unsigned int comp_data_len = 0;
	uint32_t crc_value = 0;

	comp_data_len = lzf_compress(m_CompBuf, m_iCompIndex, comp_buff, sizeof(comp_buff));

	_ASSERT(comp_data_len != 0 && comp_data_len <= sizeof(comp_buff));

	if (comp_data_len == 0
		|| comp_data_len > sizeof(comp_buff))
	{
		spdlog::error("EbenezerDlg::SendCompressedData: Failed to compress AI packet");
		return;
	}

	crc_value = crc32(reinterpret_cast<uint8_t*>(m_CompBuf), m_iCompIndex);

	SetByte(send_buff, AG_COMPRESSED_DATA, send_index);
	SetShort(send_buff, (int16_t) comp_data_len, send_index);
	SetShort(send_buff, (int16_t) m_iCompIndex, send_index);
	SetDWORD(send_buff, crc_value, send_index);
	SetShort(send_buff, (int16_t) m_CompCount, send_index);
	SetString(send_buff, reinterpret_cast<const char*>(comp_buff), comp_data_len, send_index);

	Send_AIServer(1000, send_buff, send_index);

	m_CompCount = 0;
	m_iCompIndex = 0;
}

// sungyong 2002. 05. 23
void CEbenezerDlg::DeleteAllNpcList(int flag)
{
	if (!m_bServerCheckFlag)
		return;

	if (m_bPointCheckFlag)
	{
		m_bPointCheckFlag = false;
		spdlog::error("EbenezerDlg::DeleteAllNpcList: pointCheckFlag set");
		return;
	}
	
	spdlog::debug("EbenezerDlg::DeleteAllNpcList: start");

	CUser* pUser = nullptr;

	// region Npc Array Delete
	for (C3DMap* pMap : m_ZoneArray)
	{
		if (pMap == nullptr)
			continue;

		for (int i = 0; i < pMap->GetXRegionMax(); i++)
		{
			for (int j = 0; j < pMap->GetZRegionMax(); j++)
			{
				if (!pMap->m_ppRegion[i][j].m_RegionNpcArray.IsEmpty())
					pMap->m_ppRegion[i][j].m_RegionNpcArray.DeleteAllData();
			}
		}
	}

	// Npc Array Delete
	if (!m_NpcMap.IsEmpty())
		m_NpcMap.DeleteAllData();

	m_bServerCheckFlag = false;

	AddOutputMessage(_T("DeleteAllNpcList complete"));
	spdlog::debug("EbenezerDlg::DeleteAllNpcList: end");
}
// ~sungyong 2002. 05. 23

void CEbenezerDlg::KillUser(const char* strbuff)
{
	if (strlen(strbuff) <= 0
		|| strlen(strbuff) > MAX_ID_SIZE)
		return;

	CUser* pUser = GetUserPtr(strbuff, NameType::Character);
	if (pUser != nullptr)
		pUser->Close();
}

CNpc* CEbenezerDlg::GetNpcPtr(int sid, int cur_zone)
{
	// 포인터 참조하면 안됨
	if (!m_bPointCheckFlag)
		return nullptr;

	for (const auto& [_, pNpc] : m_NpcMap)
	{
		if (pNpc == nullptr)
			continue;

		if (pNpc->m_sCurZone != cur_zone)
			continue;

		if (pNpc->m_sPid == sid)
			return pNpc;
	}

	return nullptr;
}

CKnights* CEbenezerDlg::GetKnightsPtr(int16_t clanId)
{
	if (clanId <= 0)
		return nullptr;

	return m_KnightsMap.GetData(clanId);
}

void CEbenezerDlg::WithdrawUserOut()
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser != nullptr
			&& pUser->m_pUserData->m_bZone == pUser->m_pUserData->m_bNation)
		{
			C3DMap* pMap = GetMapByID(pUser->m_pUserData->m_bNation);
			if (pMap != nullptr)
				pUser->ZoneChange(pMap->m_nZoneNumber, pMap->m_fInitX, pMap->m_fInitZ);
		}
	}
}

void CEbenezerDlg::AliveUserCheck()
{
	float currenttime = TimeGet();

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() != STATE_GAMESTART)
			continue;

/*
		if ((currenttime - pUser->m_fHPLastTime) > 300)
			pUser->Close();
*/
		for (int k = 0; k < MAX_TYPE3_REPEAT; k++)
		{
			if ((currenttime - pUser->m_fHPLastTime[k]) > 300)
			{
				pUser->Close();
				break;
			}
		}
	}
}

/////// BATTLEZONE RELATED by Yookozuna 2002.6.18 /////////////////
void CEbenezerDlg::BattleZoneOpenTimer()
{
	CTime cur = CTime::GetCurrentTime();

	// sungyong modify
	int nWeek = cur.GetDayOfWeek();
	int nTime = cur.GetHour();
	char send_buff[128] = {};
	int send_index = 0, loser_nation = 0, snow_battle = 0;
	CUser* pKarusUser = nullptr;
	CUser* pElmoUser = nullptr;

/*	if( m_byBattleOpen == NO_BATTLE )	{	// When Battlezone is closed, open it!
		if( nWeek == m_nBattleZoneOpenWeek && nTime == m_nBattleZoneOpenHourStart )	{	// 수요일, 20시에 전쟁존 open
			TRACE(_T("전쟁 자동 시작 - week=%d, time=%d\n"), nWeek, nTime);
			BattleZoneOpen(BATTLEZONE_OPEN);
//			KickOutZoneUsers(ZONE_FRONTIER);	// Kick out users in frontier zone.
		}
	}
	else {	  // When Battlezone is open, close it!
		if( nWeek == (m_nBattleZoneOpenWeek+1) && nTime == m_nBattleZoneOpenHourEnd )	{	// 목요일, 0시에 전쟁존 close
			TRACE(_T("전쟁 자동 종료 - week=%d, time=%d\n"), nWeek, nTime);
			m_byBanishFlag = 1;
		}
	}	*/

	if (m_byBattleOpen == NATION_BATTLE)
		BattleZoneCurrentUsers();

	if (m_byBanishFlag == 1)
	{
		if (m_sBanishDelay == 0)
		{
			m_byBattleOpen = NO_BATTLE;
			m_byKarusOpenFlag = 0;		// 카루스 땅으로 넘어갈 수 없도록
			m_byElmoradOpenFlag = 0;	// 엘모 땅으로 넘어갈 수 없도록

			memset(m_strKarusCaptain, 0, sizeof(m_strKarusCaptain));
			memset(m_strElmoradCaptain, 0, sizeof(m_strElmoradCaptain));

			// original: 전쟁 종료 0단계
			spdlog::debug("EbenezerDlg::BattleZoneOpenTimer: war ended, stage 0");

			if (m_nServerNo == KARUS)
			{
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
				SetByte(send_buff, UDP_BATTLE_EVENT_PACKET, send_index);
				SetByte(send_buff, BATTLE_EVENT_KILL_USER, send_index);
				SetByte(send_buff, 1, send_index);						// karus의 정보 전송
				SetShort(send_buff, m_sKarusDead, send_index);
				SetShort(send_buff, m_sElmoradDead, send_index);
				Send_UDP_All(send_buff, send_index);
			}
		}

		m_sBanishDelay++;

		if (m_sBanishDelay == 3)
		{
			// 눈싸움 전쟁
			if (m_byOldBattleOpen == SNOW_BATTLE)
			{
				if (m_sKarusDead > m_sElmoradDead)
				{
					m_bVictory = ELMORAD;
					loser_nation = KARUS;
				}
				else if (m_sKarusDead < m_sElmoradDead)
				{
					m_bVictory = KARUS;
					loser_nation = ELMORAD;
				}
				else if (m_sKarusDead == m_sElmoradDead)
				{
					m_bVictory = 0;
				}
			}

			if (m_bVictory == 0)
			{
				BattleZoneOpen(BATTLEZONE_CLOSE);
			}
			else if (m_bVictory)
			{
				if (m_bVictory == KARUS)
					loser_nation = ELMORAD;
				else if (m_bVictory == ELMORAD)
					loser_nation = KARUS;

				Announcement(DECLARE_WINNER, m_bVictory);
				Announcement(DECLARE_LOSER, loser_nation);
			}
			spdlog::debug("EbenezerDlg::BattleZoneOpenTimer: War ended, stage 1: m_bVictory={}",
				m_bVictory);
		}
		else if (m_sBanishDelay == 8)
		{
			Announcement(DECLARE_BAN);
		}
		else if (m_sBanishDelay == 10)
		{
			spdlog::debug("EbenezerDlg::BattleZoneOpenTimer: War ended, stage 2: All users return to their own nation");
			BanishLosers();
		}
		else if (m_sBanishDelay == 20)
		{
			// original: 전쟁 종료 3단계 - 초기화 해주세여
			spdlog::debug("EbenezerDlg::BattleZoneOpenTimer: War ended, stage 3: resetting battlezone");
			SetByte(send_buff, AG_BATTLE_EVENT, send_index);
			SetByte(send_buff, BATTLE_EVENT_OPEN, send_index);
			SetByte(send_buff, BATTLEZONE_CLOSE, send_index);
			Send_AIServer(1000, send_buff, send_index);
			ResetBattleZone();
		}
	}

	// ~
}

void CEbenezerDlg::BattleZoneOpen(int nType)
{
	int send_index = 0;
	char send_buff[1024] = {};

	// Open battlezone.
	if (nType == BATTLEZONE_OPEN)
	{
		m_byBattleOpen = NATION_BATTLE;
		m_byOldBattleOpen = NATION_BATTLE;
	}
	// Open snow battlezone.
	else if (nType == SNOW_BATTLEZONE_OPEN)
	{
		m_byBattleOpen = SNOW_BATTLE;
		m_byOldBattleOpen = SNOW_BATTLE;
	}
	// battle close
	else if (nType == BATTLEZONE_CLOSE)
	{
		m_byBattleOpen = NO_BATTLE;
		Announcement(BATTLEZONE_CLOSE);
	}
	else
	{
		return;
	}

	Announcement(nType);	// Send an announcement out that the battlezone is open/closed.
//
	KickOutZoneUsers(ZONE_FRONTIER);
//
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, AG_BATTLE_EVENT, send_index);		// Send packet to AI server.
	SetByte(send_buff, BATTLE_EVENT_OPEN, send_index);
	SetByte(send_buff, nType, send_index);
	Send_AIServer(1000, send_buff, send_index);
}

void CEbenezerDlg::BattleZoneVictoryCheck()
{
	// WINNER DECLARATION PROCEDURE !!!
	if (m_bKarusFlag >= NUM_FLAG_VICTORY)
		m_bVictory = KARUS;
	else if (m_bElmoradFlag >= NUM_FLAG_VICTORY)
		m_bVictory = ELMORAD;
	else
		return;

	Announcement(DECLARE_WINNER);

	// GOLD DISTRIBUTION PROCEDURE FOR WINNERS !!!
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pTUser = (CUser*) m_Iocport.m_SockArray[i];     // Get target info.
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bNation == m_bVictory
			// Zone Check!
			&& pTUser->m_pUserData->m_bZone == pTUser->m_pUserData->m_bNation)
			pTUser->m_pUserData->m_iGold += AWARD_GOLD;	// Target is in the area.
	}
}

void CEbenezerDlg::BanishLosers()
{
	// EVACUATION PROCEDURE FOR LOSERS !!!		
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pTUser = (CUser*) m_Iocport.m_SockArray[i];     // Get target info.
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bFame == COMMAND_CAPTAIN)
		{
			pTUser->m_pUserData->m_bFame = CHIEF;

			char send_buff[256] = {};
			int send_index = 0;
			SetByte(send_buff, WIZ_AUTHORITY_CHANGE, send_index);
			SetByte(send_buff, COMMAND_AUTHORITY, send_index);
			SetShort(send_buff, pTUser->GetSocketID(), send_index);
			SetByte(send_buff, pTUser->m_pUserData->m_bFame, send_index);
			pTUser->Send(send_buff, send_index);
		}

		if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation)
			pTUser->KickOutZoneUser(true);
	}
}

void CEbenezerDlg::ResetBattleZone()
{
	m_bVictory = 0;
	m_byBanishFlag = 0;
	m_sBanishDelay = 0;
	m_bKarusFlag = 0;
	m_bElmoradFlag = 0;
	m_byKarusOpenFlag = m_byElmoradOpenFlag = 0;
	m_byBattleOpen = NO_BATTLE;
	m_byOldBattleOpen = NO_BATTLE;
	m_sKarusDead = m_sElmoradDead = 0;
	m_byBattleSave = 0;
	m_sKarusCount = 0;
	m_sElmoradCount = 0;
	// REMEMBER TO MAKE ALL FLAGS AND LEVERS NEUTRAL AGAIN!!!!!!!!!!
}

void CEbenezerDlg::Announcement(uint8_t type, int nation, int chat_type)
{
	int send_index = 0;
	char send_buff[1024] = {};

	std::string chatstr;

	switch (type)
	{
		case BATTLEZONE_OPEN:
			chatstr = fmt::format_db_resource(IDP_BATTLEZONE_OPEN);
			break;

		case SNOW_BATTLEZONE_OPEN:
			chatstr = fmt::format_db_resource(IDP_BATTLEZONE_OPEN);
			break;

		case DECLARE_WINNER:
			if (m_bVictory == KARUS)
				chatstr = fmt::format_db_resource(IDP_KARUS_VICTORY, m_sElmoradDead, m_sKarusDead);
			else if (m_bVictory == ELMORAD)
				chatstr = fmt::format_db_resource(IDP_ELMORAD_VICTORY, m_sKarusDead, m_sElmoradDead);
			else
				return;
			break;

		case DECLARE_LOSER:
			if (m_bVictory == KARUS)
				chatstr = fmt::format_db_resource(IDS_ELMORAD_LOSER, m_sKarusDead, m_sElmoradDead);
			else if (m_bVictory == ELMORAD)
				chatstr = fmt::format_db_resource(IDS_KARUS_LOSER, m_sElmoradDead, m_sKarusDead);
			else
				return;
			break;

		case DECLARE_BAN:
			chatstr = fmt::format_db_resource(IDS_BANISH_USER);
			break;

		case BATTLEZONE_CLOSE:
			chatstr = fmt::format_db_resource(IDS_BATTLE_CLOSE);
			break;

		case KARUS_CAPTAIN_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_KARUS_CAPTAIN, m_strKarusCaptain);
			break;

		case ELMORAD_CAPTAIN_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_ELMO_CAPTAIN, m_strElmoradCaptain);
			break;

		case KARUS_CAPTAIN_DEPRIVE_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_KARUS_CAPTAIN_DEPRIVE, m_strKarusCaptain);
			break;

		case ELMORAD_CAPTAIN_DEPRIVE_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_ELMO_CAPTAIN_DEPRIVE, m_strElmoradCaptain);
			break;
	}

	chatstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, chat_type, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, chatstr, send_index);

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() == STATE_GAMESTART)
		{
			if (nation == 0)
				pUser->Send(send_buff, send_index);
			else if (nation == pUser->m_pUserData->m_bNation)
				pUser->Send(send_buff, send_index);
		}
	}
}

bool CEbenezerDlg::LoadStartPositionTable()
{
	recordset_loader::STLMap loader(m_StartPositionTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadServerResourceTable()
{
	ServerResourceTableMap tableMap;

	recordset_loader::STLMap loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	// NOTE: Not a name, but still falls under the same umbrella - this won't be an issue with DBs not padding these.
#if defined(DB_COMPAT_PADDED_NAMES)
	for (auto& [_, serverResource] : tableMap)
		rtrim(serverResource->Resource);
#endif

	m_ServerResourceTableMap.Swap(tableMap);
	return true;
}

bool CEbenezerDlg::LoadHomeTable()
{
	recordset_loader::STLMap loader(m_HomeTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadAllKnights()
{
	using ModelType = model::Knights;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([this](db::ModelRecordSet<ModelType>& recordset)
	{
		do
		{
			ModelType row = {};
			recordset.get_ref(row);

			CKnights* pKnights = new CKnights();
			pKnights->InitializeValue();

			pKnights->m_sIndex = row.ID;
			pKnights->m_byFlag = row.Flag;
			pKnights->m_byNation = row.Nation;

#if defined(DB_COMPAT_PADDED_NAMES)
			rtrim(row.Name);
#endif
			strcpy(pKnights->m_strName, row.Name.c_str());

#if defined(DB_COMPAT_PADDED_NAMES)
			rtrim(row.Chief);
#endif
			strcpy(pKnights->m_strChief, row.Chief.c_str());

			if (row.ViceChief1.has_value())
			{
#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(*row.ViceChief1);
#endif
				strcpy(pKnights->m_strViceChief_1, row.ViceChief1->c_str());
			}

			if (row.ViceChief2.has_value())
			{
#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(*row.ViceChief2);
#endif
				strcpy(pKnights->m_strViceChief_2, row.ViceChief2->c_str());
			}

			if (row.ViceChief3.has_value())
			{
#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(*row.ViceChief3);
#endif
				strcpy(pKnights->m_strViceChief_3, row.ViceChief3->c_str());
			}

			pKnights->m_sMembers = row.Members;
			pKnights->m_nMoney = row.Gold;
			pKnights->m_sAllianceKnights = row.AllianceKnights;
			pKnights->m_sMarkVersion = row.MarkVersion;
			pKnights->m_sCape = row.Cape;
			pKnights->m_sDomination = row.Domination;
			pKnights->m_nPoints = row.Points;
			pKnights->m_byGrade = GetKnightsGrade(row.Points);
			pKnights->m_byRanking = row.Ranking;

			for (int i = 0; i < MAX_CLAN; i++)
			{
				pKnights->m_arKnightsUser[i].byUsed = 0;
				strcpy(pKnights->m_arKnightsUser[i].strUserName, "");
			}

			if (!m_KnightsMap.PutData(pKnights->m_sIndex, pKnights))
			{
				spdlog::error("EbenezerDlg::LoadAllKnights: failed to put into KnightsMap [knightsId={}]",
					pKnights->m_sIndex);
				delete pKnights;
			}
		}
		while (recordset.next());
	});

	if (!loader.Load_AllowEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadAllKnightsUserData()
{
	using ModelType = model::KnightsUser;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([this](db::ModelRecordSet<ModelType>& recordset)
	{
		do
		{
			ModelType row = {};
			recordset.get_ref(row);

#if defined(DB_COMPAT_PADDED_NAMES)
			rtrim(row.UserId);
#endif
			m_KnightsManager.AddKnightsUser(row.KnightsId, row.UserId.c_str());
		}
		while (recordset.next());
	});

	if (!loader.Load_AllowEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

bool CEbenezerDlg::LoadKnightsSiegeWarfareTable()
{
	using ModelType = model::KnightsSiegeWarfare;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([this](db::ModelRecordSet<ModelType>& recordset)
	{
		do
		{
			ModelType row = {};
			recordset.get_ref(row);
			m_KnightsSiegeWar._castleIndex		= row.CastleIndex;
			m_KnightsSiegeWar._masterKnights	= row.MasterKnights;
		}
		while (recordset.next());
	});

	if (!loader.Load_AllowEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

int CEbenezerDlg::GetKnightsAllMembers(int knightsindex, char* temp_buff, int& buff_index, int type)
{
	if (knightsindex <= 0)
		return 0;

	int count = 0;
	if (type == 0)
	{
		for (int i = 0; i < MAX_USER; i++)
		{
			CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
			if (pUser == nullptr)
				continue;

			// 같은 소속의 클랜..
			if (pUser->m_pUserData->m_bKnights == knightsindex)
			{
				SetShort(temp_buff, strlen(pUser->m_pUserData->m_id), buff_index);
				SetString(temp_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), buff_index);
				SetByte(temp_buff, pUser->m_pUserData->m_bFame, buff_index);
				SetByte(temp_buff, pUser->m_pUserData->m_bLevel, buff_index);
				SetShort(temp_buff, pUser->m_pUserData->m_sClass, buff_index);
				SetByte(temp_buff, 1, buff_index);
				count++;
			}
		}
	}
	else if (type == 1)
	{
		CKnights* pKnights = m_KnightsMap.GetData(knightsindex);
		if (pKnights == nullptr)
			return 0;

		for (int i = 0; i < MAX_CLAN; i++)
		{
			if (pKnights->m_arKnightsUser[i].byUsed == 1)
			{
				// 접속중인 회원
				CUser* pUser = GetUserPtr(pKnights->m_arKnightsUser[i].strUserName, NameType::Character);
				if (pUser != nullptr)
				{
					if (pUser->m_pUserData->m_bKnights == knightsindex)
					{
						SetShort(temp_buff, strlen(pUser->m_pUserData->m_id), buff_index);
						SetString(temp_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), buff_index);
						SetByte(temp_buff, pUser->m_pUserData->m_bFame, buff_index);
						SetByte(temp_buff, pUser->m_pUserData->m_bLevel, buff_index);
						SetShort(temp_buff, pUser->m_pUserData->m_sClass, buff_index);
						SetByte(temp_buff, 1, buff_index);
						count++;
					}
					// 다른존에서 탈퇴나 추방된 유저이므로 메모리에서 삭제
					else
					{
						m_KnightsManager.RemoveKnightsUser(knightsindex, pUser->m_pUserData->m_id);
					}
				}
				// 비접속중인 회원
				else
				{
					SetShort(temp_buff, strlen(pKnights->m_arKnightsUser[i].strUserName), buff_index);
					SetString(temp_buff, pKnights->m_arKnightsUser[i].strUserName, strlen(pKnights->m_arKnightsUser[i].strUserName), buff_index);
					SetByte(temp_buff, 0, buff_index);
					SetByte(temp_buff, 0, buff_index);
					SetShort(temp_buff, 0, buff_index);
					SetByte(temp_buff, 0, buff_index);
					count++;
				}

			}
		}
	}

	return count;
}

void CEbenezerDlg::MarketBBSTimeCheck()
{
	int send_index = 0;
	char send_buff[256] = {};
	float currenttime = 0.0f;

	currenttime = TimeGet();

	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_sBuyID[i] != -1)
		{
			CUser* pUser = (CUser*) m_Iocport.m_SockArray[m_sBuyID[i]];
			if (pUser == nullptr)
			{
				MarketBBSBuyDelete(i);
				continue;
			}

			if (m_fBuyStartTime[i] + BBS_CHECK_TIME < currenttime)
			{
//				if (pUser->m_pUserData->m_iGold >= BUY_POST_PRICE) {
//					pUser->m_pUserData->m_iGold -= BUY_POST_PRICE ;
//					m_fBuyStartTime[i] = TimeGet();

//					memset(send_buff, 0, sizeof(send_buff));
//					send_index = 0;	
//					SetByte( send_buff, WIZ_GOLD_CHANGE, send_index );	// Now the target
//					SetByte( send_buff, 0x02, send_index );
//					SetDWORD( send_buff, BUY_POST_PRICE, send_index );
//					SetDWORD( send_buff, pUser->m_pUserData->m_iGold, send_index );
//					pUser->Send( send_buff, send_index );	
//				}
//				else {
				MarketBBSBuyDelete(i);
//				}
			}
		}

		// SELL!!!
		if (m_sSellID[i] != -1)
		{
			CUser* pUser = (CUser*) m_Iocport.m_SockArray[m_sSellID[i]];
			if (pUser == nullptr)
			{
				MarketBBSSellDelete(i);
				continue;
			}

			if (m_fSellStartTime[i] + BBS_CHECK_TIME < currenttime)
			{
//				if (pUser->m_pUserData->m_iGold >= SELL_POST_PRICE) {
//					pUser->m_pUserData->m_iGold -= SELL_POST_PRICE ;
//					m_fSellStartTime[i] = TimeGet();

//					memset(send_buff, 0, sizeof(send_buff));
//					send_index = 0;
//					SetByte( send_buff, WIZ_GOLD_CHANGE, send_index );	// Now the target
//					SetByte( send_buff, 0x02, send_index );
//					SetDWORD( send_buff, SELL_POST_PRICE, send_index );
//					SetDWORD( send_buff, pUser->m_pUserData->m_iGold, send_index );
//					pUser->Send( send_buff, send_index );	
//				}
//				else {
				MarketBBSSellDelete(i);
//				}
			}
		}
	}
}

void CEbenezerDlg::MarketBBSBuyDelete(int16_t index)
{
	m_sBuyID[index] = -1;
	memset(m_strBuyTitle[index], 0, sizeof(m_strBuyTitle[index]));
	memset(m_strBuyMessage[index], 0, sizeof(m_strBuyMessage[index]));
	m_iBuyPrice[index] = 0;
	m_fBuyStartTime[index] = 0.0f;
}

void CEbenezerDlg::MarketBBSSellDelete(int16_t index)
{
	m_sSellID[index] = -1;
	memset(m_strSellTitle[index], 0, sizeof(m_strSellTitle[index]));
	memset(m_strSellMessage[index], 0, sizeof(m_strSellMessage[index]));
	m_iSellPrice[index] = 0;
	m_fSellStartTime[index] = 0.0f;
}

void CEbenezerDlg::WritePacketLog()
{
	spdlog::info("EbenezerDlg::WritePacketLog: send={} realsend={} recv={}",
		m_iPacketCount, m_iSendPacketCount, m_iRecvPacketCount);
}

int CEbenezerDlg::GetKnightsGrade(int nPoints)
{
	// 클랜등급 = 클랜원 국가 기여도의 총합 / 24
	int nClanPoints = nPoints / 24;

	int nGrade = 5;
	if (nClanPoints >= 0
		&& nClanPoints < 2000)
		nGrade = 5;
	else if (nClanPoints >= 2000
		&& nClanPoints < 5000)
		nGrade = 4;
	else if (nClanPoints >= 5000
		&& nClanPoints < 10000)
		nGrade = 3;
	else if (nClanPoints >= 10000
		&& nClanPoints < 20000)
		nGrade = 2;
	else if (nClanPoints >= 20000)
		nGrade = 1;

	return nGrade;
}

void CEbenezerDlg::CheckAliveUser()
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() == STATE_GAMESTART)
		{
			if (pUser->m_sAliveCount > 3)
			{
				pUser->Close();
				spdlog::debug("EbenezerDlg::CheckAliveUser: User Alive Close [userId={} charId={}]",
					pUser->GetSocketID(), pUser->m_pUserData->m_id);
			}

			pUser->m_sAliveCount++;
		}
	}
}

void CEbenezerDlg::KickOutAllUsers()
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		pUser->Close();
		Sleep(1000);
	}
}

int64_t CEbenezerDlg::GenerateItemSerial()
{
	MYINT64 serial;
	MYSHORT	increase;
	serial.i = 0;
	increase.w = 0;

	CTime t = CTime::GetCurrentTime();

	EnterCriticalSection(&g_serial_critical);

	increase.w = g_increase_serial++;

	serial.b[7] = (uint8_t) m_nServerNo;
	serial.b[6] = (uint8_t) (t.GetYear() % 100);
	serial.b[5] = (uint8_t) t.GetMonth();
	serial.b[4] = (uint8_t) t.GetDay();
	serial.b[3] = (uint8_t) t.GetHour();
	serial.b[2] = (uint8_t) t.GetMinute();
	serial.b[1] = increase.b[1];
	serial.b[0] = increase.b[0];

	LeaveCriticalSection(&g_serial_critical);

//	TRACE(_T("Generate Item Serial : %I64d\n"), serial.i);
	return serial.i;
}

void CEbenezerDlg::KickOutZoneUsers(int16_t zone)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pTUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pTUser == nullptr)
			continue;

		// Only kick out users in requested zone.
		if (pTUser->m_pUserData->m_bZone == zone)
		{
			C3DMap* pMap = GetMapByID(pTUser->m_pUserData->m_bNation);
			if (pMap != nullptr)
				pTUser->ZoneChange(pMap->m_nZoneNumber, pMap->m_fInitX, pMap->m_fInitZ);	// Move user to native zone.
		}
	}
}

void CEbenezerDlg::Send_UDP_All(char* pBuf, int len, int group_type)
{
	std::map<int, _ZONE_SERVERINFO*>::iterator Iter1;
	std::map<int, _ZONE_SERVERINFO*>::iterator Iter2;

	int server_number = 0;

	if (group_type == 0)
	{
		Iter1 = m_ServerArray.begin();
		Iter2 = m_ServerArray.end();
		server_number = m_nServerNo;
	}
	else
	{
		Iter1 = m_ServerGroupArray.begin();
		Iter2 = m_ServerGroupArray.end();
		server_number = m_nServerGroupNo;
	}

	for (; Iter1 != Iter2; Iter1++)
	{
		_ZONE_SERVERINFO* pInfo = (*Iter1).second;
		if (pInfo != nullptr
			&& pInfo->sServerNo != server_number)
			m_pUdpSocket->SendUDPPacket(pInfo->strServerIP, pBuf, len);
	}
}

bool CEbenezerDlg::LoadBattleTable()
{
	using ModelType = model::Battle;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([this](db::ModelRecordSet<ModelType>& recordset)
	{
		do
		{
			ModelType row = {};
			recordset.get_ref(row);

			m_byOldVictory = row.Nation;
		}
		while (recordset.next());
	});

	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	return true;
}

void CEbenezerDlg::Send_CommandChat(char* pBuf, int len, int nation, CUser* pExceptUser)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		// if (pUser == pExceptUser)
		//	continue;

		if (pUser->GetState() == STATE_GAMESTART
			&& pUser->m_pUserData->m_bNation == nation)
			pUser->Send(pBuf, len);
	}
}

bool CEbenezerDlg::LoadKnightsRankTable()
{
	using ModelType = model::KnightsRating;

	std::string strKarusCaptain[5], strElmoCaptain[5];

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([&](db::ModelRecordSet<ModelType>& recordset)
	{
		char send_buff[1024] = {};
		int nKarusRank = 0, nElmoRank = 0, nFindKarus = 0, nFindElmo = 0, send_index = 0;

		do
		{
			ModelType row = {};
			recordset.get_ref(row);
		
			CKnights* pKnights = m_KnightsMap.GetData(row.Index);
			if (pKnights == nullptr)
				continue;

#if defined(DB_COMPAT_PADDED_NAMES)
			rtrim(row.Name);
#endif

			if (pKnights->m_byNation == KARUS)
			{
				//if (nKarusRank == 5 || nFindKarus == 1)
				if (nKarusRank == 5)
					continue;			// 5위까지 클랜장이 없으면 대장은 없음			

				//nKarusRank++;

				CUser* pUser = GetUserPtr(pKnights->m_strChief, NameType::Character);
				if (pUser == nullptr)
					continue;

				if (pUser->m_pUserData->m_bZone != ZONE_BATTLE)
					continue;

				if (pUser->m_pUserData->m_bKnights == row.Index)
				{
					pUser->m_pUserData->m_bFame = COMMAND_CAPTAIN;
					strKarusCaptain[nKarusRank] = fmt::format("[{}][{}]", row.Name, pUser->m_pUserData->m_id);
					nKarusRank++;

					nFindKarus = 1;
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, WIZ_AUTHORITY_CHANGE, send_index);
					SetByte(send_buff, COMMAND_AUTHORITY, send_index);
					SetShort(send_buff, pUser->GetSocketID(), send_index);
					SetByte(send_buff, pUser->m_pUserData->m_bFame, send_index);
					//pUser->Send( send_buff, send_index );
					Send_Region(send_buff, send_index, pUser->m_pUserData->m_bZone, pUser->m_RegionX, pUser->m_RegionZ);

					//strcpy( m_strKarusCaptain, pUser->m_pUserData->m_id );
					//Announcement( KARUS_CAPTAIN_NOTIFY, KARUS );
					//TRACE(_T("Karus Captain - %hs, rank=%d, index=%d\n"), pUser->m_pUserData->m_id, row.Rank, row.Index);
				}
			}
			else if (pKnights->m_byNation == ELMORAD)
			{
				//if (nElmoRank == 5 || nFindElmo == 1)
				if (nElmoRank == 5)
					continue;			// 5위까지 클랜장이 없으면 대장은 없음			

				//nElmoRank++;

				CUser* pUser = GetUserPtr(pKnights->m_strChief, NameType::Character);
				if (pUser == nullptr)
					continue;

				if (pUser->m_pUserData->m_bZone != ZONE_BATTLE)
					continue;

				if (pUser->m_pUserData->m_bKnights == row.Index)
				{
					pUser->m_pUserData->m_bFame = COMMAND_CAPTAIN;
					strElmoCaptain[nElmoRank] = fmt::format("[{}][{}]", row.Name, pUser->m_pUserData->m_id);
					nFindElmo = 1;
					nElmoRank++;

					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, WIZ_AUTHORITY_CHANGE, send_index);
					SetByte(send_buff, COMMAND_AUTHORITY, send_index);
					SetShort(send_buff, pUser->GetSocketID(), send_index);
					SetByte(send_buff, pUser->m_pUserData->m_bFame, send_index);
					//pUser->Send( send_buff, send_index );
					Send_Region(send_buff, send_index, pUser->m_pUserData->m_bZone, pUser->m_RegionX, pUser->m_RegionZ);

					//strcpy( m_strElmoradCaptain, pUser->m_pUserData->m_id );
					//Announcement( ELMORAD_CAPTAIN_NOTIFY, ELMORAD );
					//TRACE(_T("Elmo Captain - %hs, rank=%d, index=%d\n"), pUser->m_pUserData->m_id, row.Rank, row.Index);
				}
			}
		}
		while (recordset.next());
	});

	if (!loader.Load_AllowEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	std::string strKarusCaptainName = fmt::format_db_resource(IDS_KARUS_CAPTAIN,
		strKarusCaptain[0], strKarusCaptain[1], strKarusCaptain[2], strKarusCaptain[3], strKarusCaptain[4]);

	std::string strElmoCaptainName = fmt::format_db_resource(IDS_ELMO_CAPTAIN,
		strElmoCaptain[0], strElmoCaptain[1], strElmoCaptain[2], strElmoCaptain[3], strElmoCaptain[4]);

	spdlog::trace("EbenezerDlg::LoadKnightsRankTable: success");

	char send_buff[1024] = {},
		temp_buff[1024] = {};
	int send_index = 0, temp_index = 0;

	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, WAR_SYSTEM_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, strKarusCaptainName, send_index);

	SetByte(temp_buff, WIZ_CHAT, temp_index);
	SetByte(temp_buff, WAR_SYSTEM_CHAT, temp_index);
	SetByte(temp_buff, 1, temp_index);
	SetShort(temp_buff, -1, temp_index);
	SetByte(temp_buff, 0, send_index);			// sender name length
	SetString2(temp_buff, strElmoCaptainName, temp_index);

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() == STATE_GAMESTART)
		{
			if (pUser->m_pUserData->m_bNation == KARUS)
				pUser->Send(send_buff, send_index);
			else if (pUser->m_pUserData->m_bNation == ELMORAD)
				pUser->Send(temp_buff, temp_index);
		}
	}

	return true;
}

void CEbenezerDlg::BattleZoneCurrentUsers()
{
	C3DMap* pMap = GetMapByID(ZONE_BATTLE);
	if (pMap == nullptr)
		return;

	// 현재의 서버가 배틀존 서버가 아니라면 리턴
	if (m_nServerNo != pMap->m_nServerNo)
		return;

	char send_buff[128] = {};
	int nKarusMan = 0, nElmoradMan = 0, send_index = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bZone == ZONE_BATTLE)
		{
			if (pUser->m_pUserData->m_bNation == KARUS)
				nKarusMan++;
			else if (pUser->m_pUserData->m_bNation == ELMORAD)
				nElmoradMan++;
		}
	}

	m_sKarusCount = nKarusMan;
	m_sElmoradCount = nElmoradMan;

	//TRACE(_T("---> BattleZoneCurrentUsers - karus=%d, elmorad=%d\n"), m_sKarusCount, m_sElmoradCount);

	SetByte(send_buff, UDP_BATTLEZONE_CURRENT_USERS, send_index);
	SetShort(send_buff, m_sKarusCount, send_index);
	SetShort(send_buff, m_sElmoradCount, send_index);
	Send_UDP_All(send_buff, send_index);
}

void CEbenezerDlg::FlySanta()
{
	int send_index = 0;
	char send_buff[128] = {};

	SetByte(send_buff, WIZ_SANTA, send_index);
	SetByte(send_buff, m_bySanta, send_index);
	Send_All(send_buff, send_index);
}

C3DMap* CEbenezerDlg::GetMapByIndex(int iZoneIndex) const
{
	if (iZoneIndex < 0
		|| iZoneIndex >= static_cast<int>(m_ZoneArray.size()))
		return nullptr;

	return m_ZoneArray[iZoneIndex];
}

C3DMap* CEbenezerDlg::GetMapByID(int iZoneID) const
{
	for (C3DMap* pMap : m_ZoneArray)
	{
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == iZoneID)
			return pMap;
	}

	return nullptr;
}

bool CEbenezerDlg::LoadEventTriggerTable()
{
	using ModelType = model::EventTrigger;

	EventTriggerMap localMap;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([&](db::ModelRecordSet<ModelType>& recordset)
	{
		do
		{
			ModelType row = {};
			recordset.get_ref(row);

			uint32_t key = GetEventTriggerKey(row.NpcType, row.NpcId);

			bool inserted = localMap.insert(std::make_pair(key, row.TriggerNumber)).second;
			if (!inserted)
			{
				spdlog::error("EbenezerDlg::LoadEventTriggerTable: failed to insert into EventTriggerMap [NpcType={} NpcId={}]",
					row.NpcType, row.NpcId);
			}
		}
		while (recordset.next());
	});

	if (!loader.Load_AllowEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	m_EventTriggerMap.swap(localMap);
	return true;
}

uint32_t CEbenezerDlg::GetEventTriggerKey(uint8_t byNpcType, uint16_t sTrapNumber) const
{
	// This is just a more efficient way of packing both components into a single key
	// instead of using and comparing std::pair<> in our map.
	return (static_cast<uint32_t>(byNpcType) << 16) | sTrapNumber;
}

int32_t CEbenezerDlg::GetEventTrigger(uint8_t byNpcType, uint16_t sTrapNumber) const
{
	uint32_t key = GetEventTriggerKey(byNpcType, sTrapNumber);

	auto itr = m_EventTriggerMap.find(key);
	if (itr == m_EventTriggerMap.end())
		return -1;

	return itr->second;
}
