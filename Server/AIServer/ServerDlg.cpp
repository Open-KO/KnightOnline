// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"

#include "GameSocket.h"
#include "Region.h"

#include <shared/crc32.h>
#include <shared/lzf.h>
#include <shared/globals.h>
#include <shared/Ini.h>
#include <shared/StringConversion.h>
#include <shared/StringUtils.h>

#include <spdlog/spdlog.h>

#include <db-library/ConnectionManager.h>

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NOTE: Explicitly handled under DEBUG_NEW override
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/RecordsetLoader_Vector.h>

bool g_bNpcExit = false;
ZoneArray m_ZoneArray;

CRITICAL_SECTION g_User_critical;
CRITICAL_SECTION g_region_critical;

#define CHECK_ALIVE 	100		//  게임서버와 통신이 끊김여부 판단, 타이머 변수
#define REHP_TIME		200

import AIServerBinder;

using namespace db;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. 3개의 함수 추가
		int GetSpeed(BYTE bySpeed);
		int GetAttackSpeed(BYTE bySpeed);
		int GetCatsSpeed(BYTE bySpeed);
	2. Repent에  맞개 아래의 함수 수정
		CreateNpcThread();
		GetMonsterTableData();
		GetNpcTableData();
		GetNpcItemTable();
*/

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

CServerDlg* CServerDlg::s_pInstance = nullptr;

CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerDlg)
	m_strStatus = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_iYear = 0;
	m_iMonth = 0;
	m_iDate = 0;
	m_iHour = 0;
	m_iMin = 0;
	m_iWeather = 0;
	m_iAmount = 0;
	m_byNight = 1;
	m_byZone = KARUS_ZONE;
	m_byBattleEvent = BATTLEZONE_CLOSE;
	m_sKillKarusNpc = 0;
	m_sKillElmoNpc = 0;
	m_pZoneEventThread = nullptr;
	m_byTestMode = 0;
	//m_ppUserActive = nullptr;
	//m_ppUserInActive = nullptr;

	ConnectionManager::Create();
}

CServerDlg::~CServerDlg()
{
	ConnectionManager::Destroy();
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerDlg)
	DDX_Control(pDX, IDC_LIST1, _outputList);
	DDX_Text(pDX, IDC_STATUS, m_strStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	//{{AFX_MSG_MAP(CServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_GAMESERVER_LOGIN, OnGameServerLogin)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerDlg message handlers

///////////////////////////////////////////////////////////////////////////////
//	각종 초기화
//
BOOL CServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	s_pInstance = this;

	// load config
	GetServerInfoIni();

	// Default Init ...
	DefaultInit();

	// TestCode
	TestCode();

	//----------------------------------------------------------------------
	//	Sets a random number starting point.
	//----------------------------------------------------------------------
	SetTimer(CHECK_ALIVE, 10000, nullptr);

	srand((unsigned int) time(nullptr));
	for (int i = 0; i < 10; i++)
		myrand(1, 10000);	// don't delete

	// Compress Init
	memset(m_CompBuf, 0, sizeof(m_CompBuf));	// 압축할 데이터를 모으는 버퍼
	m_iCompIndex = 0;							// 압축할 데이터의 길이
	m_CompCount = 0;							// 압축할 데이터의 개수

	InitializeCriticalSection(&g_User_critical);
	InitializeCriticalSection(&g_region_critical);

	m_sSocketCount = 0;
	m_sErrorSocketCount = 0;
	m_sMapEventNpc = 0;
	m_sReSocketCount = 0;
	m_fReConnectStart = 0.0f;
	m_bFirstServerFlag = false;
	m_byTestMode = NOW_TEST_MODE;

	// User Point Init
	for (int i = 0; i < MAX_USER; i++)
		m_pUser[i] = nullptr;

	// Server Start messages
	CTime time = CTime::GetCurrentTime();
	std::wstring logstr = std::format(L"[AI ServerStart - {:04}-{:02}-{:02}, {:02}:{:02}]",
		time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute());
	AddOutputMessage(logstr);
	spdlog::info("ServerDlg::OnInitDialog: starting...");

	//----------------------------------------------------------------------
	//	DB part initialize
	//----------------------------------------------------------------------
	if (m_byZone == UNIFY_ZONE)
		m_strStatus.Format(_T("Server Zone: UNIFY"));
	else if (m_byZone == KARUS_ZONE)
		m_strStatus.Format(_T("Server Zone: KARUS"));
	else if (m_byZone == ELMORAD_ZONE)
		m_strStatus.Format(_T("Server Zone: ELMORAD"));
	else if (m_byZone == BATTLE_ZONE)
		m_strStatus.Format(_T("Server Zone: BATTLE"));
	
	//----------------------------------------------------------------------
	//	Communication Part Initialize ...
	//----------------------------------------------------------------------
	spdlog::info("ServerDlg::OnInitDialog: initializing sockets");
	m_Iocport.Init(MAX_SOCKET, 1, 1);

	for (int i = 0; i < MAX_SOCKET; i++)
		m_Iocport.m_SockArrayInActive[i] = new CGameSocket;

	//----------------------------------------------------------------------
	//	Load Magic Table
	//----------------------------------------------------------------------
	if (!GetMagicTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAGIC, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType1Data())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAGIC_TYPE1, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType2Data())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAGIC_TYPE2, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType3Data())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAGIC_TYPE3, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType4Data())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAGIC_TYPE4, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType7Data())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAGIC_TYPE7, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	//----------------------------------------------------------------------
	//	Load NPC Item Table
	//----------------------------------------------------------------------
	if (!GetNpcItemTable())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load K_MONSTER_ITEM, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeWeaponItemTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAKE_WEAPON, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeDefensiveItemTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAKE_DEFENSIVE, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeGradeItemTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAKE_ITEM_GRADECODE, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeRareItemTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load MAKE_ITEM_LARECODE, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	//----------------------------------------------------------------------
	//	Load NPC Chat Table
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//	Load NPC Data & Activate NPC
	//----------------------------------------------------------------------

	// Monster 특성치 테이블 Load
	if (!GetMonsterTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load K_MONSTER, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	// NPC 특성치 테이블 Load
	if (!GetNpcTableData())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load K_NPC, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	//----------------------------------------------------------------------
	//	Load Zone & Event...
	//----------------------------------------------------------------------
	if (!MapFileLoad())
	{
		spdlog::error("ServerDlg::OnInitDialog: failed to load maps, closing server");
		AfxPostQuitMessage(0);
	}

	if (!CreateNpcThread())
	{
		spdlog::error("ServerDlg::OnInitDialog: CreateNpcThread failed, closing server");
		EndDialog(IDCANCEL);
		return FALSE;
	}

	//----------------------------------------------------------------------
	//	Load NPC DN Table
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//	Start NPC THREAD
	//----------------------------------------------------------------------
	ResumeAI();

	//----------------------------------------------------------------------
	//	Start Accepting...
	//----------------------------------------------------------------------
	if (!ListenByZone())
	{
		AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"), MB_OK);
		return FALSE;
	}

	//::ResumeThread( m_Iocport.m_hAcceptThread );
	UpdateData(FALSE);

	spdlog::info("AIServer successfully initialized");
	return TRUE;
}

/// \brief attempts to listen on the port associated with m_byZone
/// \see m_byZone
/// \returns true when successful, otherwise false
bool CServerDlg::ListenByZone()
{
	int port = GetListenPortByZone();
	if (port < 0)
	{
		spdlog::error("ServerDlg::ListenByZone: failed to associate listen port for zone {}", m_byZone);
		return false;
	}

	if (!m_Iocport.Listen(port))
	{
		spdlog::error("ServerDlg::ListenByZone: failed to listen on port {}", port);
		return false;
	}

	AddOutputMessage(fmt::format("Listening on 0.0.0.0:{}", port));
	return true;
}

/// \brief fetches the listen port associated with m_byZone
/// \see m_byZone
/// \returns the associated listen port or -1 if invalid
int CServerDlg::GetListenPortByZone() const
{
	switch (m_byZone)
	{
		case KARUS_ZONE:
		case UNIFY_ZONE:
			return AI_KARUS_SOCKET_PORT;

		case ELMORAD_ZONE:
			return AI_ELMO_SOCKET_PORT;

		case BATTLE_ZONE:
			return AI_BATTLE_SOCKET_PORT;

		default:
			return -1;
	}
}

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerDlg::OnPaint()
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
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CServerDlg::DefaultInit()
{
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
}

void CServerDlg::ReportTableLoadError(const recordset_loader::Error& err, const char* source)
{
	std::string error = fmt::format("ServerDlg::ReportTableLoadError: {} failed: {}",
		source, err.Message);
	std::wstring werror = LocalToWide(error);
	AfxMessageBox(werror.c_str());
	spdlog::error(error);
}

//	Magic Table 을 읽는다.
bool CServerDlg::GetMagicTableData()
{
	recordset_loader::STLMap loader(m_MagicTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}
	
	spdlog::info("ServerDlg::GetMagicTableData: MAGIC loaded");
	return true;
}

bool CServerDlg::GetMakeWeaponItemTableData()
{
	recordset_loader::STLMap loader(m_MakeWeaponTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMakeWeaponItemTableData: MAKE_WEAPON loaded");
	return true;
}

bool CServerDlg::GetMakeDefensiveItemTableData()
{
	recordset_loader::STLMap<MakeWeaponTableMap, model::MakeDefensive> loader(
		m_MakeDefensiveTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMakeDefensiveItemTableData: MAKE_DEFENSIVE loaded");
	return true;
}

bool CServerDlg::GetMakeGradeItemTableData()
{
	recordset_loader::STLMap loader(m_MakeGradeItemArray);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMakeGradeItemTableData: MAKE_ITEM_GRADECODE loaded");
	return true;
}

bool CServerDlg::GetMakeRareItemTableData()
{
	recordset_loader::STLMap loader(m_MakeItemRareCodeTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMakeRareItemTableData: MAKE_ITEM_LARECODE loaded");
	return true;
}

/////////////////////////////////////////////////////////////////////////
//	NPC Item Table 을 읽는다.
//
bool CServerDlg::GetNpcItemTable()
{
	using ModelType = model::MonsterItem;

	std::vector<ModelType*> rows;

	recordset_loader::Vector<ModelType> loader(rows);
	if (!loader.Load_ForbidEmpty(true))
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	m_NpcItem.m_nField = loader.GetColumnCount();
	m_NpcItem.m_nRow = static_cast<int>(rows.size());

	if (rows.empty())
		return false;

	m_NpcItem.m_ppItem = new int* [m_NpcItem.m_nRow];
	for (int i = 0; i < m_NpcItem.m_nRow; i++)
		m_NpcItem.m_ppItem[i] = new int[m_NpcItem.m_nField];

	for (size_t i = 0; i < rows.size(); i++)
	{
		ModelType* row = rows[i];

		m_NpcItem.m_ppItem[i][0] = row->MonsterId;
		m_NpcItem.m_ppItem[i][1] = row->ItemId1;
		m_NpcItem.m_ppItem[i][2] = row->DropChance1;
		m_NpcItem.m_ppItem[i][3] = row->ItemId2;
		m_NpcItem.m_ppItem[i][4] = row->DropChance2;
		m_NpcItem.m_ppItem[i][5] = row->ItemId3;
		m_NpcItem.m_ppItem[i][6] = row->DropChance3;
		m_NpcItem.m_ppItem[i][7] = row->ItemId4;
		m_NpcItem.m_ppItem[i][8] = row->DropChance4;
		m_NpcItem.m_ppItem[i][9] = row->ItemId5;
		m_NpcItem.m_ppItem[i][10] = row->DropChance5;

		delete row;
	}

	rows.clear();

	spdlog::info("ServerDlg::GetNpcItemTable: K_MONSTER_ITEM loaded");
	return true;
}

//	Monster Table Data 를 읽는다.
bool CServerDlg::GetMonsterTableData()
{
	NpcTableMap tableMap;
	recordset_loader::STLMap<
		NpcTableMap,
		model::Monster> loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

#if defined(DB_COMPAT_PADDED_NAMES)
	for (const auto& [_, row] : tableMap)
	{
		if (row->Name.has_value())
			rtrim(*row->Name);
	}
#endif

	m_MonTableMap.Swap(tableMap);

	spdlog::info("ServerDlg::GetMonsterTableData: K_MONSTER loaded");
	return true;
}

//	NPC Table Data 를 읽는다. (경비병 & NPC)
bool CServerDlg::GetNpcTableData()
{
	NpcTableMap tableMap;
	recordset_loader::STLMap loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

#if defined(DB_COMPAT_PADDED_NAMES)
	for (const auto& [_, row] : tableMap)
	{
		if (row->Name.has_value())
			rtrim(*row->Name);
	}
#endif

	m_NpcTableMap.Swap(tableMap);

	spdlog::info("ServerDlg::GetNpcTableData: K_NPC loaded");
	return true;
}

//	Npc Thread 를 만든다.
bool CServerDlg::CreateNpcThread()
{
	m_TotalNPC = 0;			// DB에 있는 수
	m_CurrentNPC = 0;
	m_CurrentNPCError = 0;

	std::vector<model::NpcPos*> rows;
	if (!LoadNpcPosTable(rows))
	{
		spdlog::error("ServerDlg::CreateNpcThread: K_NPCPOS load failed");
		return false;
	}

	for (model::NpcPos* row : rows)
		delete row;
	rows.clear();

	int step = 0;
	int nThreadNumber = 0;
	CNpcThread* pNpcThread = nullptr;

	for (auto& [_, pNpc] : m_NpcMap)
	{
		if (step == 0)
			pNpcThread = new CNpcThread;

		pNpcThread->m_pNpc[step] = pNpc;
		pNpcThread->m_pNpc[step]->m_sThreadNumber = nThreadNumber;
		pNpcThread->m_pNpc[step]->Init();

		++step;

		if (step == NPC_NUM)
		{
			pNpcThread->m_sThreadNumber = nThreadNumber++;
			pNpcThread->pIOCP = &m_Iocport;
			pNpcThread->m_pThread = AfxBeginThread(NpcThreadProc, &pNpcThread->m_ThreadInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
			m_NpcThreadArray.push_back(pNpcThread);
			step = 0;
		}
	}
	if (step != 0)
	{
		pNpcThread->m_sThreadNumber = nThreadNumber++;
		pNpcThread->pIOCP = &m_Iocport;
		pNpcThread->m_pThread = AfxBeginThread(NpcThreadProc, &pNpcThread->m_ThreadInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		m_NpcThreadArray.push_back(pNpcThread);
	}

	// Event Npc Logic
	m_pZoneEventThread = AfxBeginThread(ZoneEventThreadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	
	std::wstring logstr = std::format(L"NPCs initialized: {}",
		m_TotalNPC);
	AddOutputMessage(logstr);

	spdlog::info("ServerDlg::CreateNpcThread: Monsters/NPCs loaded: {}", m_TotalNPC);
	return true;
}

bool CServerDlg::LoadNpcPosTable(std::vector<model::NpcPos*>& rows)
{
	CRoomEvent* pRoom = nullptr;

	recordset_loader::Vector<model::NpcPos> loader(rows);
	if (!loader.Load_ForbidEmpty(true))
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	int nSerial = m_sMapEventNpc;

	for (model::NpcPos* row : rows)
	{
		bool bMoveNext = true;
		int nPathSerial = 1;
		int nNpcCount = 0;

		do
		{
			int nMonsterNumber = row->NumNpc;
			int nServerNum = GetServerNumber(row->ZoneId);

			if (m_byZone == nServerNum
				|| m_byZone == UNIFY_ZONE)
			{
				for (int j = 0; j < nMonsterNumber; j++)
				{
					CNpc* pNpc = new CNpc();
					pNpc->m_sNid = nSerial++;						// 서버 내에서의 고유 번호
					pNpc->m_sSid = (short) row->NpcId;				// MONSTER(NPC) Serial ID

					pNpc->m_byMoveType = row->ActType;
					pNpc->m_byInitMoveType = row->ActType;
					pNpc->m_byDirection = row->Direction;

					model::Npc* pNpcTable = nullptr;

					if (row->ActType >= 0
						&& row->ActType < 100)
					{
						pNpcTable = m_MonTableMap.GetData(pNpc->m_sSid);
					}
					else if (row->ActType >= 100)
					{
						pNpc->m_byMoveType = row->ActType - 100;
						//pNpc->m_byInitMoveType = row->ActType - 100;

						pNpcTable = m_NpcTableMap.GetData(pNpc->m_sSid);
					}

					pNpc->m_byBattlePos = 0;

					if (pNpc->m_byMoveType >= 2)
					{
						pNpc->m_byBattlePos = myrand(1, 3);
						pNpc->m_byPathCount = nPathSerial++;
					}

					pNpc->InitPos();

					if (pNpcTable == nullptr)
					{
						spdlog::error("ServerDlg::LoadNpcPosTable: npc not found [serial={}, npcId={}]",
							pNpc->m_sNid, pNpc->m_sSid);
						break;
					}

					if (bMoveNext)
					{
						bMoveNext = false;
						nNpcCount = row->NumNpc;
					}

					pNpc->Load(pNpcTable, true);

					//////// MONSTER POS ////////////////////////////////////////
					pNpc->m_sCurZone = row->ZoneId;

					float fRandom_X = 0.0f, fRandom_Z = 0.0f;

					// map에 몬스터의 위치를 랜덤하게 위치시킨다.. (테스트 용 : 수정-DB에서 읽어오는데로 몬 위치 결정)
					int nRandom = abs(row->LeftX - row->RightX);
					if (nRandom <= 1)
					{
						fRandom_X = (float) row->LeftX;
					}
					else
					{
						if (row->LeftX < row->RightX)
							fRandom_X = (float) myrand(row->LeftX, row->RightX);
						else
							fRandom_X = (float) myrand(row->RightX, row->LeftX);
					}

					nRandom = abs(row->TopZ - row->BottomZ);
					if (nRandom <= 1)
					{
						fRandom_Z = (float) row->TopZ;
					}
					else
					{
						if (row->TopZ < row->BottomZ)
							fRandom_Z = (float) myrand(row->TopZ, row->BottomZ);
						else
							fRandom_Z = (float) myrand(row->BottomZ, row->TopZ);
					}

					pNpc->m_fCurX = fRandom_X;
					pNpc->m_fCurY = 0;
					pNpc->m_fCurZ = fRandom_Z;

					if (row->RespawnTime < 15)
					{
						spdlog::warn("ServerDlg::LoadNpcPosTable: RegTime below minimum value of 15s [npcId={}, serial={}, npcName={}, RegTime={}]",
							pNpc->m_sSid, pNpc->m_sNid + NPC_BAND, pNpc->m_strName, row->RespawnTime);
						// TODO: Set this to 15 in separate ticket and comment on it deviating from official behavior
						row->RespawnTime = 30;
					}

					pNpc->m_sRegenTime = row->RespawnTime * 1000;	// 초(DB)단위-> 밀리세컨드로

					pNpc->m_sMaxPathCount = row->PathPointCount;

					if (pNpc->m_byMoveType == 2
						|| pNpc->m_byMoveType == 3)
					{
						if (row->PathPointCount == 0
							|| !row->Path.has_value())
						{
							std::string error = fmt::format("ServerDlg::LoadNpcPosTable: NPC expects path to be set [zoneId={} serial={}, npcId={}, npcName={}, moveType={}, pathCount={}]",
								row->ZoneId,
								pNpc->m_sNid + NPC_BAND,
								pNpc->m_sSid,
								pNpc->m_strName,
								pNpc->m_byMoveType,
								pNpc->m_sMaxPathCount);

							spdlog::error(error);
							AfxMessageBox(Utf8ToWide(error).c_str());
							return false;
						}
					}

					int index = 0;

					if (row->PathPointCount != 0
						&& row->Path.has_value())
					{
						// The path is a series of points (x,z), each in the form ("%04d%04d", x, z)
						// As such, we expect there to be at least 8 characters per point.
						constexpr size_t CharactersPerPoint = 8;

						const std::string& path = *row->Path;
						if ((row->PathPointCount * CharactersPerPoint) > path.size())
						{
							std::string error = fmt::format("LoadNpcPosTable: NPC expects a larger path for this PathPointCount [zoneId={} serial={} npcId={} npcName={} moveType={}, pathCount={}]",
								row->ZoneId,
								row->PathPointCount,
								pNpc->m_sNid + NPC_BAND,
								pNpc->m_sSid,
								pNpc->m_strName.c_str(),
								pNpc->m_byMoveType,
								pNpc->m_sMaxPathCount);

							spdlog::error(error);
							AfxMessageBox(Utf8ToWide(error).c_str());
							return false;
						}

						for (int l = 0; l < row->PathPointCount; l++)
						{
							char szX[5] = {}, szZ[5] = {};
							GetString(szX, path.c_str(), 4, index);
							GetString(szZ, path.c_str(), 4, index);
							pNpc->m_PathList.pPattenPos[l].x = atoi(szX);
							pNpc->m_PathList.pPattenPos[l].z = atoi(szZ);
							//	TRACE(_T(" l=%d, x=%d, z=%d\n"), l, pNpc->m_PathList.pPattenPos[l].x, pNpc->m_PathList.pPattenPos[l].z);
						}
					}

					pNpc->m_nInitMinX = pNpc->m_nLimitMinX = row->LeftX;
					pNpc->m_nInitMinY = pNpc->m_nLimitMinZ = row->TopZ;
					pNpc->m_nInitMaxX = pNpc->m_nLimitMaxX = row->RightX;
					pNpc->m_nInitMaxY = pNpc->m_nLimitMaxZ = row->BottomZ;
					// dungeon work
					pNpc->m_byDungeonFamily = row->DungeonFamily;
					pNpc->m_bySpecialType = row->SpecialType;
					pNpc->m_byRegenType = row->RegenType;
					pNpc->m_byTrapNumber = row->TrapNumber;

					if (pNpc->m_byDungeonFamily > 0)
					{
						pNpc->m_nLimitMinX = row->LimitMinX;
						pNpc->m_nLimitMinZ = row->LimitMinZ;
						pNpc->m_nLimitMaxX = row->LimitMaxX;
						pNpc->m_nLimitMaxZ = row->LimitMaxZ;
					}

					pNpc->m_ZoneIndex = -1;

					MAP* pMap = nullptr;
					for (size_t i = 0; i < m_ZoneArray.size(); i++)
					{
						if (m_ZoneArray[i]->m_nZoneNumber == pNpc->m_sCurZone)
						{
							pNpc->m_ZoneIndex = static_cast<short>(i);
							pMap = m_ZoneArray[i];
							break;
						}
					}

					if (pMap == nullptr)
					{
						spdlog::error("ServerDlg::LoadNpcPosTable: NPC invalid zone [npcId:{}, npcZoneId:{}]", pNpc->m_sSid, pNpc->m_sCurZone);
						AfxMessageBox(_T("NPC invalid zone index error (see log)"));
						return false;
					}

					//pNpc->Init();
					//m_NpcMap.Add(pNpc);
					if (!m_NpcMap.PutData(pNpc->m_sNid, pNpc))
					{
						spdlog::error("ServerDlg::LoadNpcPosTable: Npc PutData Fail [serial={}]",
							pNpc->m_sNid);
						delete pNpc;
						pNpc = nullptr;
					}

					if (pNpc != nullptr
						&& pMap->m_byRoomEvent > 0
						&& pNpc->m_byDungeonFamily > 0)
					{
						pRoom = pMap->m_arRoomEventArray.GetData(pNpc->m_byDungeonFamily);
						if (pRoom == nullptr)
						{
							spdlog::error("ServerDlg::LoadNpcPosTable: No RoomEvent for NPC dungeonFamily: serial={}, npcId={}, npcName={}, dungeonFamily={}, zoneId={}",
								pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName, pNpc->m_byDungeonFamily, pNpc->m_ZoneIndex);
							AfxMessageBox(_T("No RoomEvent for NPC dungeonFamily (see log)"));
							return false;
						}

						int* pInt = new int;
						*pInt = pNpc->m_sNid;
						if (!pRoom->m_mapRoomNpcArray.PutData(pNpc->m_sNid, pInt))
						{
							delete pInt;
							spdlog::error("ServerDlg::LoadNpcPosTable: MapRoomNpcArray.PutData failed for NPC: [serial={}, npcId={}]", pNpc->m_sNid, pNpc->m_sSid);
						}
					}

					m_TotalNPC = nSerial;

					if (--nNpcCount > 0)
						continue;

					bMoveNext = true;
					nNpcCount = 0;
				}
			}
		} 
		while (!bMoveNext);
	}

	return true;
}

//	NPC Thread 들을 작동시킨다.
void CServerDlg::ResumeAI()
{
	for (size_t i = 0; i < m_NpcThreadArray.size(); i++)
	{
		for (int j = 0; j < NPC_NUM; j++)
			m_NpcThreadArray[i]->m_ThreadInfo.pNpc[j] = m_NpcThreadArray[i]->m_pNpc[j];

		m_NpcThreadArray[i]->m_ThreadInfo.pIOCP = &m_Iocport;

		ResumeThread(m_NpcThreadArray[i]->m_pThread->m_hThread);
	}


	// Event Npc Logic
/*	m_EventNpcThreadArray[0]->m_ThreadInfo.hWndMsg = this->GetSafeHwnd();
	for(j = 0; j < NPC_NUM; j++)
	{
		m_EventNpcThreadArray[0]->m_ThreadInfo.pNpc[j] = nullptr;	// 초기 소환 몹이 당연히 없으므로 nullptr로 작동을 안시킴
		m_EventNpcThreadArray[0]->m_ThreadInfo.m_byNpcUsed[j] = 0;
	}
	m_EventNpcThreadArray[0]->m_ThreadInfo.pIOCP = &m_Iocport;

	::ResumeThread(m_EventNpcThreadArray[0]->m_pThread->m_hThread);
	*/

	ResumeThread(m_pZoneEventThread->m_hThread);
}

//	메모리 정리
BOOL CServerDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(CHECK_ALIVE);
	//KillTimer( REHP_TIME );

	g_bNpcExit = true;

	for (size_t i = 0; i < m_NpcThreadArray.size(); i++)
		WaitForSingleObject(m_NpcThreadArray[i]->m_pThread->m_hThread, INFINITE);

	// Event Npc Logic
/*	for(i = 0; i < m_EventNpcThreadArray.size(); i++)
	{
		WaitForSingleObject(m_EventNpcThreadArray[i]->m_pThread->m_hThread, INFINITE);
	}	*/

	WaitForSingleObject(m_pZoneEventThread, INFINITE);

	// DB테이블 삭제 부분

	// Map(Zone) Array Delete...
	for (size_t i = 0; i < m_ZoneArray.size(); i++)
		delete m_ZoneArray[i];
	m_ZoneArray.clear();

	// NpcTable Array Delete
	if (!m_MonTableMap.IsEmpty())
		m_MonTableMap.DeleteAllData();

	// NpcTable Array Delete
	if (!m_NpcTableMap.IsEmpty())
		m_NpcTableMap.DeleteAllData();

	// NpcThread Array Delete
	for (size_t i = 0; i < m_NpcThreadArray.size(); i++)
		delete m_NpcThreadArray[i];
	m_NpcThreadArray.clear();

	// Event Npc Logic
	// EventNpcThread Array Delete
/*	for(i = 0; i < m_EventNpcThreadArray.size(); i++)
		delete m_EventNpcThreadArray[i];
	m_EventNpcThreadArray.clear();		*/

	// Item Array Delete
	if (m_NpcItem.m_ppItem)
	{
		for (int i = 0; i < m_NpcItem.m_nRow; i++)
		{
			delete[] m_NpcItem.m_ppItem[i];
			m_NpcItem.m_ppItem[i] = nullptr;
		}
		delete[] m_NpcItem.m_ppItem;
		m_NpcItem.m_ppItem = nullptr;
	}

	if (!m_MakeWeaponTableMap.IsEmpty())
		m_MakeWeaponTableMap.DeleteAllData();

	if (!m_MakeDefensiveTableMap.IsEmpty())
		m_MakeDefensiveTableMap.DeleteAllData();

	if (!m_MakeGradeItemArray.IsEmpty())
		m_MakeGradeItemArray.DeleteAllData();

	if (!m_MakeItemRareCodeTableMap.IsEmpty())
		m_MakeItemRareCodeTableMap.DeleteAllData();

	// MagicTable Array Delete
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

	if (!m_MagicType7TableMap.IsEmpty())
		m_MagicType7TableMap.DeleteAllData();

	// Npc Array Delete
	if (!m_NpcMap.IsEmpty())
		m_NpcMap.DeleteAllData();

	// User Array Delete
	for (int i = 0; i < MAX_USER; i++)
	{
		delete m_pUser[i];
		m_pUser[i] = nullptr;
	}

	// Party Array Delete 
	if (!m_PartyMap.IsEmpty())
		m_PartyMap.DeleteAllData();

	while (!m_ZoneNpcList.empty())
		m_ZoneNpcList.pop_front();

	DeleteCriticalSection(&g_User_critical);
	DeleteCriticalSection(&g_region_critical);

	s_pInstance = nullptr;

	return CDialog::DestroyWindow();
}

void CServerDlg::DeleteUserList(int uid)
{
	if (uid < 0
		|| uid >= MAX_USER)
	{
		spdlog::error("ServerDlg::DeleteUserList: userId invalid: {}", uid);
		return;
	}

	EnterCriticalSection(&g_User_critical);
	CUser* pUser = nullptr;
	pUser = m_pUser[uid];
	if (!pUser)
	{
		LeaveCriticalSection(&g_User_critical);
		spdlog::error("ServerDlg::DeleteUserList: userId not found: {}", uid);
		return;
	}
	if (pUser->m_iUserId == uid)
	{
		spdlog::debug("ServerDlg::DeleteUserList: User Logout: userId={}, charId={}", uid, pUser->m_strUserID);
		pUser->m_lUsed = 1;
		delete m_pUser[uid];
		m_pUser[uid] = nullptr;
	}
	else
	{
		spdlog::warn("ServerDlg::DeleteUserList: userId mismatch : userId={} pUserId={}", uid, pUser->m_iUserId);
	}

	LeaveCriticalSection(&g_User_critical);
}

bool CServerDlg::MapFileLoad()
{
	using ModelType = model::ZoneInfo;

	bool loaded = false;

	m_sTotalMap = 0;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([&](db::ModelRecordSet<ModelType>& recordset)
	{
		CString szFullPath;

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

			szFullPath.Format(
				_T("%ls"),
				mapPath.c_str());

			CFile file;
			if (!file.Open(szFullPath, CFile::modeRead))
			{
				std::wstring werror = std::format(L"ServerDlg::MapFileLoad: Failed to open file: {}",
					mapPath.c_str());
				std::string error = WideToUtf8(werror);
				AfxMessageBox(werror.c_str());
				spdlog::error(error);
				return;
			}

			MAP* pMap = new MAP();
			pMap->m_nServerNo = row.ServerId;
			pMap->m_nZoneNumber = row.ZoneId;

			if (!pMap->LoadMap(file.m_hFile))
			{
				std::wstring werror = std::format(L"ServerDlg::MapFileLoad: Failed to load map file: {}",
					mapPath.c_str());
				std::string error = WideToUtf8(werror);
				AfxMessageBox(werror.c_str());
				spdlog::error(error);
				delete pMap;
				return;
			}

			file.Close();

			// dungeon work
			if (row.RoomEvent > 0)
			{
				if (!pMap->LoadRoomEvent(row.RoomEvent))
				{
					std::wstring werror = std::format(L"ServerDlg::MapFileLoad: LoadRoomEvent failed: {}",
						mapPath.c_str());
					std::string error = WideToUtf8(werror);
					AfxMessageBox(werror.c_str());
					spdlog::error(error);
					delete pMap;
					return;
				}

				pMap->m_byRoomEvent = 1;
			}

			m_ZoneArray.push_back(pMap);
			++m_sTotalMap;
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

// sungyong 2002.05.23
// game server에 모든 npc정보를 전송..
void CServerDlg::AllNpcInfo()
{
	// server alive check
	CNpc* pNpc = nullptr;
	int nZone = 0;
	int size = m_NpcMap.GetSize();

	int send_index = 0, zone_index = 0, packet_size = 0;
	int count = 0, send_count = 0, send_tot = 0;
	char send_buff[2048] = {};

	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap == nullptr)
			continue;

		nZone = pMap->m_nZoneNumber;

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_SERVER_INFO, send_index);
		SetByte(send_buff, SERVER_INFO_START, send_index);
		SetByte(send_buff, nZone, send_index);
		packet_size = Send(send_buff, send_index, nZone);

		zone_index = GetZoneIndex(nZone);
		send_index = 2;
		count = 0;
		send_count = 0;
		m_CompCount = 0;
		m_iCompIndex = 0;
		memset(send_buff, 0, sizeof(send_buff));

		spdlog::debug("ServerDlg::AllNpcInfo: start for zoneIndex={}", nZone);

		for (int i = 0; i < size; i++)
		{
			pNpc = m_NpcMap.GetData(i);
			if (pNpc == nullptr)
			{
				spdlog::warn("ServerDlg::AllNpcInfo: NpcMap[{}] is null", i);
				continue;
			}

			if (pNpc->m_sCurZone != nZone)
				continue;

			pNpc->SendNpcInfoAll(send_buff, send_index, count);
			count++;

			if (count == NPC_NUM)
			{
				SetByte(send_buff, NPC_INFO_ALL, send_count);
				SetByte(send_buff, (BYTE) count, send_count);
				m_CompCount++;
				//::CopyMemory(m_CompBuf+m_iCompIndex, send_buff, send_index);
				memset(m_CompBuf, 0, sizeof(m_CompBuf));
				::CopyMemory(m_CompBuf, send_buff, send_index);
				m_iCompIndex = send_index;
				SendCompressedData(nZone);
				send_index = 2;
				send_count = 0;
				count = 0;
				send_tot++;
				//TRACE(_T("AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
				memset(send_buff, 0, sizeof(send_buff));
				Sleep(50);
			}
		}

		//if( count != 0 )	TRACE(_T("--> AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
		if (count != 0
			&& count < NPC_NUM)
		{
			send_count = 0;
			SetByte(send_buff, NPC_INFO_ALL, send_count);
			SetByte(send_buff, (BYTE) count, send_count);
			Send(send_buff, send_index, nZone);
			send_tot++;
			//TRACE(_T("AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
			Sleep(50);
		}

		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
		SetByte(send_buff, AG_SERVER_INFO, send_index);
		SetByte(send_buff, SERVER_INFO_END, send_index);
		SetByte(send_buff, nZone, send_index);
		SetShort(send_buff, (short) m_TotalNPC, send_index);
		packet_size = Send(send_buff, send_index, nZone);

		spdlog::debug("ServerDlg::AllNpcInfo: end for zoneId={}", nZone);
	}

	Sleep(1000);
}
// ~sungyong 2002.05.23

CUser* CServerDlg::GetUserPtr(int nid)
{
	CUser* pUser = nullptr;

	if (nid < 0
		|| nid >= MAX_USER)
	{
		if (nid != -1)
			spdlog::error("ServerDlg::GetUserPtr: User Array Overflow [{}]", nid);

		return nullptr;
	}

/*	if( !m_ppUserActive[nid] )
		return nullptr;

	if( m_ppUserActive[nid]->m_lUsed == 1 ) return nullptr;	// 포인터 사용을 허락치 않음.. (logout중)

	pUser = (CUser*)m_ppUserActive[nid];
*/
	pUser = m_pUser[nid];
	if (pUser == nullptr)
		return nullptr;

	// 포인터 사용을 허락치 않음.. (logout중)
	if (pUser->m_lUsed == 1)
		return nullptr;

	if (pUser->m_iUserId < 0
		|| pUser->m_iUserId >= MAX_USER)
		return nullptr;

	if (pUser->m_iUserId == nid)
		return pUser;

	return nullptr;
}

void CServerDlg::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
		case CHECK_ALIVE:
			CheckAliveTest();
			break;

		case REHP_TIME:
			//RechargeHp();
			break;
	}

	CDialog::OnTimer(nIDEvent);
}

// sungyong 2002.05.23
void CServerDlg::CheckAliveTest()
{
	int send_index = 0;
	char send_buff[256] = {};
	int iErrorCode = 0;

	SetByte(send_buff, AG_CHECK_ALIVE_REQ, send_index);

	CGameSocket* pSocket = nullptr;
	int size = 0, count = 0;

	CString logstr;
	CTime time = CTime::GetCurrentTime();

	for (int i = 0; i < MAX_SOCKET; i++)
	{
		pSocket = (CGameSocket*) m_Iocport.m_SockArray[i];
		if (pSocket == nullptr)
			continue;

		size = pSocket->Send(send_buff, send_index);
		if (size > 0)
		{
			++m_sErrorSocketCount;
			if (m_sErrorSocketCount == 10)
			{
				spdlog::debug("ServerDlg::CheckAliveTest: all ebenezer sockets are connected");
			}
			count++;
		}
		//TRACE(_T("size = %d, socket_num = %d, i=%d \n"), size, pSocket->m_sSocketID, i);
	}

	if (count <= 0)
		DeleteAllUserList(9999);

	RegionCheck();
}

void CServerDlg::DeleteAllUserList(int zone)
{
	if (zone < 0)
		return;

	// 모든 소켓이 끊어진 상태...
	if (zone == 9999
		&& m_bFirstServerFlag)
	{
		spdlog::debug("ServerDlg::DeleteAllUserList: start");

		for (MAP* pMap : m_ZoneArray)
		{
			if (pMap == nullptr)
				continue;

			for (int i = 0; i < pMap->m_sizeRegion.cx; i++)
			{
				for (int j = 0; j < pMap->m_sizeRegion.cy; j++)
				{
					if (!pMap->m_ppRegion[i][j].m_RegionUserArray.IsEmpty())
						pMap->m_ppRegion[i][j].m_RegionUserArray.DeleteAllData();
				}
			}
		}

		EnterCriticalSection(&g_User_critical);
		// User Array Delete
		for (int i = 0; i < MAX_USER; i++)
		{
			delete m_pUser[i];
			m_pUser[i] = nullptr;
		}
		// 파티 정보 삭제..
		LeaveCriticalSection(&g_User_critical);

		// Party Array Delete 
		if (!m_PartyMap.IsEmpty())
			m_PartyMap.DeleteAllData();

		m_bFirstServerFlag = false;
		spdlog::debug("ServerDlg::DeleteAllUserList: end");

		AddOutputMessage(_T("DeleteAllUserList: done"));
	}
	else if (zone != 9999)
	{
		std::wstring logstr = std::format(L"Ebenezer disconnected from zone={}",
			zone);
		AddOutputMessage(logstr);
		spdlog::info("ServerDlg::DeleteAllUserList: ebenezer zone {} disconnected", zone);
	}
}
// ~sungyong 2002.05.23

void CServerDlg::SendCompressedData(int nZone)
{
	if (m_CompCount <= 0
		|| m_iCompIndex <= 0)
	{
		m_CompCount = 0;
		m_iCompIndex = 0;
		spdlog::error("ServerDlg::SendCompressData: count={}, index={}",
			m_CompCount, m_iCompIndex);
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
		spdlog::error("ServerDlg::SendCompressedData: Failed to compress packet");
		return;
	}

	crc_value = crc32(reinterpret_cast<uint8_t*>(m_CompBuf), m_iCompIndex);

	SetByte(send_buff, AG_COMPRESSED_DATA, send_index);
	SetShort(send_buff, (short) comp_data_len, send_index);
	SetShort(send_buff, (short) m_iCompIndex, send_index);
	SetDWORD(send_buff, crc_value, send_index);
	SetShort(send_buff, (short) m_CompCount, send_index);
	SetString(send_buff, reinterpret_cast<const char*>(comp_buff), comp_data_len, send_index);

	Send(send_buff, send_index, nZone);

	m_CompCount = 0;
	m_iCompIndex = 0;
}

BOOL CServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
			return TRUE;

		if (pMsg->wParam == VK_F9)
			SyncTest();
	}

	return CDialog::PreTranslateMessage(pMsg);
}

// sungyong 2002.05.23
int CServerDlg::Send(char* pData, int length, int nZone)
{
	SEND_DATA* pNewData = nullptr;
	pNewData = new SEND_DATA;
	if (pNewData == nullptr)
		return 0;

	pNewData->sCurZone = nZone;
	pNewData->sLength = length;
	::CopyMemory(pNewData->pBuf, pData, length);

	EnterCriticalSection(&(m_Iocport.m_critSendData));
	m_Iocport.m_SendDataList.push_back(pNewData);
	LeaveCriticalSection(&(m_Iocport.m_critSendData));

	PostQueuedCompletionStatus(m_Iocport.m_hSendIOCP, 0, 0, nullptr);

	return 0;
}
// ~sungyong 2002.05.23

LRESULT CServerDlg::OnGameServerLogin(WPARAM wParam, LPARAM lParam)
{
/*	if( m_bNpcInfoDown ) {
		m_ZoneNpcList.push_back(wParam);
		return;
	}

	AllNpcInfo( wParam );	*/
	return 0;
}

void CServerDlg::GameServerAcceptThread()
{
	::ResumeThread(m_Iocport.m_hAcceptThread);
}

void CServerDlg::SyncTest()
{
	spdlog::info("ServerDlg::SyncTest: begin");

	int send_index = 0;
	char send_buff[256] = {};
	int iErrorCode = 0;

	SetByte(send_buff, AG_CHECK_ALIVE_REQ, send_index);

	CGameSocket* pSocket = nullptr;
	int size = 0;

	for (int i = 0; i < MAX_SOCKET; i++)
	{
		pSocket = (CGameSocket*) m_Iocport.m_SockArray[i];
		if (pSocket == nullptr)
			continue;

		size = pSocket->Send(send_buff, send_index);

		spdlog::info("ServerDlg::SyncTest: size={}, socketId={}", size, pSocket->m_sSocketID);
	}

/*
	int size = m_NpcMap.GetSize();
	CNpc* pNpc = nullptr;
	CUser* pUser = nullptr;
	__Vector3 vUser;
	__Vector3 vNpc;
	__Vector3 vDistance;
	float fDis = 0.0f;
	int count = 0;

	fprintf(stream, "***** NPC List : %d *****\n", size);
	for(int i=0; i<size; i++)
	{
		pNpc = m_NpcMap.GetData(i);
		if(pNpc == nullptr)
		{
			TRACE(_T("##### allNpcInfo Fail = %d\n"), i);
			continue;
		}

		fprintf(stream, "nid=(%d, %s), zone=%d, x=%.2f, z=%.2f, rx=%d, rz=%d\n", pNpc->m_sNid+NPC_BAND, pNpc->m_strName,pNpc->m_sCurZone, pNpc->m_fCurX, pNpc->m_fCurZ, pNpc->m_iRegion_X, pNpc->m_iRegion_Z);

	/*	vNpc.Set(pNpc->m_fCurX, 0, pNpc->m_fCurZ);
		if(pNpc->m_byAttackPos)	{
			//EnterCriticalSection( &g_User_critical );
			pUser = m_arUser.GetData(pNpc->m_Target.id);
			if(pUser == nullptr) {
				fprintf(stream, "## Fail ## nid=(%d, %s), att_pos=%d, x=%.2f, z=%.2f\n", pNpc->m_sNid+NPC_BAND, pNpc->m_strName, pNpc->m_byAttackPos, pNpc->m_fCurX, pNpc->m_fCurZ);
				continue;
			}
			vUser.Set(pNpc->m_Target.x, 0, pNpc->m_Target.z);
			fDis = pNpc->GetDistance(vNpc, vUser);
			//fprintf(stream, "[ target : x=%.2f, z=%.2f ] [ user : x=%.2f, z=%.2f ] \n", pNpc->m_Target.x, pNpc->m_Target.z, pUser->m_curx, pUser->m_curz);
			fprintf(stream, "nid=(%d, %s), att_pos=%d, dis=%.2f, x=%.2f, z=%.2f\n", pNpc->m_sNid+NPC_BAND, pNpc->m_strName, pNpc->m_byAttackPos, fDis, pNpc->m_fCurX, pNpc->m_fCurZ);
			//LeaveCriticalSection( &g_User_critical );
		}	*/
	//}	
/*
	fprintf(stream, "*****   User List  *****\n");

	for(i=0; i<MAX_USER; i++)	{
		//pUser = m_ppUserActive[i];
		pUser = m_pUser[i];
		if(pUser == nullptr)		continue;
		fprintf(stream, "nid=(%d, %s), zone=%d, x=%.2f, z=%.2f, rx=%d, rz=%d\n", pUser->m_iUserId, pUser->m_strUserID, pUser->m_curZone, pUser->m_curx, pUser->m_curz, pUser->m_sRegionX, pUser->m_sRegionZ);
	}

	fprintf(stream, "*****   Region List  *****\n");
	int k=0, total_user = 0, total_mon=0;
	MAP* pMap = nullptr;

	for(k=0; k<m_sTotalMap; k++)	{
		pMap = m_ZoneArray[k];
		if(pMap == nullptr)	continue;
		for( i=0; i<pMap->m_sizeRegion.cx; i++ ) {
			for( int j=0; j<pMap->m_sizeRegion.cy; j++ ) {
				EnterCriticalSection( &g_User_critical );
				total_user = pMap->m_ppRegion[i][j].m_RegionUserArray.GetSize();
				total_mon = pMap->m_ppRegion[i][j].m_RegionNpcArray.GetSize();
				LeaveCriticalSection( &g_User_critical );

				if(total_user > 0 || total_mon > 0)	{
					fprintf(stream, "rx=%d, rz=%d, user=%d, monster=%d\n", i, j, total_user, total_mon);
				}
			}
		}
	}	*/
}

CUser* CServerDlg::GetActiveUserPtr(int index)
{
	CUser* pUser = nullptr;

/*	if(index < 0 || index > MAX_USER)	{
		TRACE(_T("### Fail :: User Array Overflow[%d] ###\n"), index );
		return nullptr;
	}

	EnterCriticalSection( &g_User_critical );

	if ( m_ppUserActive[index] ) {
		LeaveCriticalSection( &g_User_critical );
		TRACE(_T("### Fail : ActiveUser Array Invalid[%d] ###\n"), index );
		return nullptr;
	}
	else {
		pUser = (CUser *)m_ppUserInActive[index];
		if( !pUser ) {
			LeaveCriticalSection( &g_User_critical );
			TRACE(_T("### Fail : InActiveUser Array Invalid[%d] ###\n"), index );
			return nullptr;
		}
	}

	m_ppUserActive[index] = pUser;
	m_ppUserInActive[index] = nullptr;

	LeaveCriticalSection( &g_User_critical );	*/

	return pUser;
}

CNpc* CServerDlg::GetNpcPtr(const char* pNpcName)
{
	for (const auto& [_, pNpc] : m_NpcMap)
	{
		if (pNpc != nullptr
			&& strcmp(pNpc->m_strName.c_str(), pNpcName) == 0)
			return pNpc;
	}

	spdlog::error("ServerDlg::GetNpcPtr: failed to find npc with name: {}", pNpcName);
	return nullptr;
}


//	추가할 소환몹의 메모리를 참조하기위해 플래그가 0인 상태것만 넘긴다.
CNpc* CServerDlg::GetEventNpcPtr()
{
	for (auto& [_, pNpc] : m_NpcMap)
	{
		if (pNpc == nullptr)
			continue;

		if (pNpc->m_lEventNpc != 0)
			continue;

		pNpc->m_lEventNpc = 1;
		return pNpc;
	}

	return nullptr;
}

int CServerDlg::MonsterSummon(const char* pNpcName, int zone_id, float fx, float fz)
{
	MAP* pMap = GetMapByID(zone_id);
	if (pMap == nullptr)
	{
		spdlog::error("ServerDlg::MonsterSummon: No map found for npcName={}, zoneId={}",
			pNpcName, zone_id);
		return -1;
	}

	CNpc* pNpc = GetNpcPtr(pNpcName);
	if (pNpc == nullptr)
	{
		spdlog::error("ServerDlg::MonsterSummon: no NPC found for npcName={}",
			pNpcName);
		return  -1;
	}

	bool bFlag = SetSummonNpcData(pNpc, zone_id, fx, fz);

	return 1;
}

//	소환할 몹의 데이타값을 셋팅한다.
bool CServerDlg::SetSummonNpcData(CNpc* pNpc, int zone_id, float fx, float fz)
{
	int  iCount = 0;
	CNpc* pEventNpc = GetEventNpcPtr();
	if (pEventNpc == nullptr)
	{
		spdlog::error("ServerDlg::SetSummonNpcData: No EventNpc found");
		return false;
	}

	pEventNpc->m_sSid = pNpc->m_sSid;						// MONSTER(NPC) Serial ID
	pEventNpc->m_byMoveType = 1;
	pEventNpc->m_byInitMoveType = 1;
	pEventNpc->m_byBattlePos = 0;
	pEventNpc->m_strName = pNpc->m_strName;					// MONSTER(NPC) Name
	pEventNpc->m_sPid = pNpc->m_sPid;						// MONSTER(NPC) Picture ID
	pEventNpc->m_sSize = pNpc->m_sSize;						// 캐릭터의 비율(100 퍼센트 기준)
	pEventNpc->m_iWeapon_1 = pNpc->m_iWeapon_1;				// 착용무기
	pEventNpc->m_iWeapon_2 = pNpc->m_iWeapon_2;				// 착용무기
	pEventNpc->m_byGroup = pNpc->m_byGroup;					// 소속집단
	pEventNpc->m_byActType = pNpc->m_byActType;				// 행동패턴
	pEventNpc->m_byRank = pNpc->m_byRank;					// 작위
	pEventNpc->m_byTitle = pNpc->m_byTitle;					// 지위
	pEventNpc->m_iSellingGroup = pNpc->m_iSellingGroup;
	pEventNpc->m_sLevel = pNpc->m_sLevel;					// level
	pEventNpc->m_iExp = pNpc->m_iExp;						// 경험치
	pEventNpc->m_iLoyalty = pNpc->m_iLoyalty;				// loyalty
	pEventNpc->m_iHP = pNpc->m_iMaxHP;						// 최대 HP
	pEventNpc->m_iMaxHP = pNpc->m_iMaxHP;					// 현재 HP
	pEventNpc->m_sMP = pNpc->m_sMaxMP;						// 최대 MP
	pEventNpc->m_sMaxMP = pNpc->m_sMaxMP;					// 현재 MP
	pEventNpc->m_sAttack = pNpc->m_sAttack;					// 공격값
	pEventNpc->m_sDefense = pNpc->m_sDefense;				// 방어값
	pEventNpc->m_sHitRate = pNpc->m_sHitRate;				// 타격성공률
	pEventNpc->m_sEvadeRate = pNpc->m_sEvadeRate;			// 회피성공률
	pEventNpc->m_sDamage = pNpc->m_sDamage;					// 기본 데미지
	pEventNpc->m_sAttackDelay = pNpc->m_sAttackDelay;		// 공격딜레이
	pEventNpc->m_sSpeed = pNpc->m_sSpeed;					// 이동속도
	pEventNpc->m_fSpeed_1 = pNpc->m_fSpeed_1;				// 기본 이동 타입
	pEventNpc->m_fSpeed_2 = pNpc->m_fSpeed_2;				// 뛰는 이동 타입..
	pEventNpc->m_fOldSpeed_1 = pNpc->m_fOldSpeed_1;			// 기본 이동 타입
	pEventNpc->m_fOldSpeed_2 = pNpc->m_fOldSpeed_2;			// 뛰는 이동 타입..
	pEventNpc->m_fSecForMetor = 4.0f;						// 초당 갈 수 있는 거리..
	pEventNpc->m_sStandTime = pNpc->m_sStandTime;			// 서있는 시간
	pEventNpc->m_iMagic1 = pNpc->m_iMagic1;					// 사용마법 1
	pEventNpc->m_iMagic2 = pNpc->m_iMagic2;					// 사용마법 2
	pEventNpc->m_iMagic3 = pNpc->m_iMagic3;					// 사용마법 3
	pEventNpc->m_sFireR = pNpc->m_sFireR;					// 화염 저항력
	pEventNpc->m_sColdR = pNpc->m_sColdR;					// 냉기 저항력
	pEventNpc->m_sLightningR = pNpc->m_sLightningR;			// 전기 저항력
	pEventNpc->m_sMagicR = pNpc->m_sMagicR;					// 마법 저항력
	pEventNpc->m_sDiseaseR = pNpc->m_sDiseaseR;				// 저주 저항력
	pEventNpc->m_sPoisonR = pNpc->m_sPoisonR;				// 독 저항력
	pEventNpc->m_sLightR = pNpc->m_sLightR;					// 빛 저항력
	pEventNpc->m_fBulk = pNpc->m_fBulk;
	pEventNpc->m_bySearchRange = pNpc->m_bySearchRange;		// 적 탐지 범위
	pEventNpc->m_byAttackRange = pNpc->m_byAttackRange;		// 사정거리
	pEventNpc->m_byTracingRange = pNpc->m_byTracingRange;	// 추격거리
	pEventNpc->m_tNpcType = pNpc->m_tNpcType;				// NPC Type
	pEventNpc->m_byFamilyType = pNpc->m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	pEventNpc->m_iMoney = pNpc->m_iMoney;					// 떨어지는 돈
	pEventNpc->m_iItem = pNpc->m_iItem;						// 떨어지는 아이템
	pEventNpc->m_tNpcLongType = pNpc->m_tNpcLongType;
	pEventNpc->m_byWhatAttackType = pNpc->m_byWhatAttackType;

	//////// MONSTER POS ////////////////////////////////////////
	pEventNpc->m_sCurZone = static_cast<short>(zone_id);
	pEventNpc->m_fCurX = fx;
	pEventNpc->m_fCurY = 0;
	pEventNpc->m_fCurZ = fz;
	pEventNpc->m_nInitMinX = pNpc->m_nInitMinX;
	pEventNpc->m_nInitMinY = pNpc->m_nInitMinY;
	pEventNpc->m_nInitMaxX = pNpc->m_nInitMaxX;
	pEventNpc->m_nInitMaxY = pNpc->m_nInitMaxY;
	pEventNpc->m_sRegenTime = pNpc->m_sRegenTime;			// 초(DB)단위-> 밀리세컨드로

	pEventNpc->m_ZoneIndex = -1;

	pEventNpc->m_NpcState = NPC_DEAD;						// 상태는 죽은것으로 해야 한다.. 
	pEventNpc->m_bFirstLive = 1;							// 처음 살아난 경우로 해줘야 한다..

	for (size_t i = 0; i < m_ZoneArray.size(); i++)
	{
		if (m_ZoneArray[i]->m_nZoneNumber == zone_id)
		{
			pEventNpc->m_ZoneIndex = static_cast<short>(i);
			break;
		}
	}

	if (pEventNpc->m_ZoneIndex == -1)
	{
		spdlog::error("ServerDlg::SetSummonNpcData: invalid zone index");
		return false;
	}

	pEventNpc->Init();

	bool bSuccess = false;

	int test = 0;

	for (int i = 0; i < NPC_NUM; i++)
	{
		test = m_EventNpcThreadArray[0]->m_ThreadInfo.m_byNpcUsed[i];
		spdlog::debug("ServerDlg::SetSummonNpcData: setsummon == {}, used={}", i, test);
		if (m_EventNpcThreadArray[0]->m_ThreadInfo.m_byNpcUsed[i] == 0)
		{
			m_EventNpcThreadArray[0]->m_ThreadInfo.m_byNpcUsed[i] = 1;
			bSuccess = true;
			m_EventNpcThreadArray[0]->m_ThreadInfo.pNpc[i] = pEventNpc;
			break;
		}
	}

	if (!bSuccess)
	{
		pEventNpc->m_lEventNpc = 0;
		spdlog::error("ServerDlg::SetSummonNpcData: summon failed");
		return false;
	}

	spdlog::debug("ServerDlg::SetSummonNpcData: summoned serial={} npcName={} npcState={}",
		pEventNpc->m_sNid + NPC_BAND, pEventNpc->m_strName, pEventNpc->m_NpcState);

	return true;
}

void CServerDlg::TestCode()
{
	//InitTrigonometricFunction();

	int random = 0, count_1 = 0, count_2 = 0, count_3 = 0;

	// TestCoding
	for (int i = 0; i < 100; i++)
	{
		random = myrand(1, 3);
		if (random == 1)
			count_1++;
		else if (random == 2)
			count_2++;
		else if (random == 3)
			count_3++;
	}

	//TRACE(_T("$$$ random test == 1=%d, 2=%d, 3=%d,, %d,%hs $$$\n"), count_1, count_2, count_3, __FILE__, __LINE__);

}

bool CServerDlg::GetMagicType1Data()
{
	recordset_loader::STLMap loader(m_MagicType1TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMagicType1Data: MAGIC_TYPE1 loaded");
	return true;
}

bool CServerDlg::GetMagicType2Data()
{
	recordset_loader::STLMap loader(m_MagicType2TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMagicType2Data: MAGIC_TYPE2 loaded");
	return true;
}

bool CServerDlg::GetMagicType3Data()
{
	recordset_loader::STLMap loader(m_MagicType3TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMagicType3Data: MAGIC_TYPE3 loaded");
	return true;
}

bool CServerDlg::GetMagicType4Data()
{
	recordset_loader::STLMap loader(m_MagicType4TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMagicType4Data: MAGIC_TYPE4 loaded");
	return true;
}

bool CServerDlg::GetMagicType7Data()
{
	recordset_loader::STLMap loader(m_MagicType7TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		ReportTableLoadError(loader.GetError(), __func__);
		return false;
	}

	spdlog::info("ServerDlg::GetMagicType7Data: MAGIC_TYPE7 loaded");
	return true;
}

void CServerDlg::RegionCheck()
{
	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap == nullptr)
			continue;

		for (int i = 0; i < pMap->m_sizeRegion.cx; i++)
		{
			for (int j = 0; j < pMap->m_sizeRegion.cy; j++)
			{
				EnterCriticalSection(&g_User_critical);
				int total_user = pMap->m_ppRegion[i][j].m_RegionUserArray.GetSize();
				LeaveCriticalSection(&g_User_critical);
				if (total_user > 0)  
					pMap->m_ppRegion[i][j].m_byMoving = 1;
				else
					pMap->m_ppRegion[i][j].m_byMoving = 0;
			}
		}
	}
}

bool CServerDlg::AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number)
{
	int i = 0, j = 0, objectid = 0;
	model::Npc* pNpcTable = nullptr;
	bool bFindNpcTable = false;
	int offset = 0;
	int nServerNum = 0;
	nServerNum = GetServerNumber(zone_number);
	//if(m_byZone != zone_number)	 return false;
	//if(m_byZone != UNIFY_ZONE)	{
	//	if(m_byZone != nServerNum)	 return false;
	//}

	//if( zone_number > 201 )	return false;	// test
	pNpcTable = m_NpcTableMap.GetData(pEvent->sIndex);
	if (pNpcTable == nullptr)
	{
		bFindNpcTable = false;
		spdlog::error("ServerDlg::AddObjectEventNpc error: eventId={} zoneId={}",
			pEvent->sIndex, zone_number);
		return false;
	}

	bFindNpcTable = true;

	CNpc* pNpc = new CNpc();

	pNpc->m_sNid = m_sMapEventNpc++;				// 서버 내에서의 고유 번호
	pNpc->m_sSid = (short) pEvent->sIndex;			// MONSTER(NPC) Serial ID

	pNpc->m_byMoveType = 100;
	pNpc->m_byInitMoveType = 100;
	bFindNpcTable = false;

	pNpc->m_byMoveType = 0;
	pNpc->m_byInitMoveType = 0;

	pNpc->m_byBattlePos = 0;

	pNpc->m_fSecForMetor = 4.0f;					// 초당 갈 수 있는 거리..

	pNpc->Load(pNpcTable, false);

	//////// MONSTER POS ////////////////////////////////////////

	pNpc->m_sCurZone = zone_number;

	pNpc->m_byGateOpen = pEvent->sStatus;
	pNpc->m_fCurX = pEvent->fPosX;
	pNpc->m_fCurY = pEvent->fPosY;
	pNpc->m_fCurZ = pEvent->fPosZ;

	pNpc->m_nInitMinX = pEvent->fPosX - 1;
	pNpc->m_nInitMinY = pEvent->fPosZ - 1;
	pNpc->m_nInitMaxX = pEvent->fPosX + 1;
	pNpc->m_nInitMaxY = pEvent->fPosZ + 1;

	pNpc->m_sRegenTime = 10000 * 1000;	// 초(DB)단위-> 밀리세컨드로
	//pNpc->m_sRegenTime		= 30 * 1000;	// 초(DB)단위-> 밀리세컨드로
	pNpc->m_sMaxPathCount = 0;

	pNpc->m_ZoneIndex = -1;
	pNpc->m_byObjectType = SPECIAL_OBJECT;
	pNpc->m_bFirstLive = 1;		// 처음 살아난 경우로 해줘야 한다..
	//pNpc->m_ZoneIndex = GetZoneIndex(pNpc->m_sCurZone);
/*
	if(pNpc->m_ZoneIndex == -1)	{
		AfxMessageBox("Invaild zone Index!!");
		return false;
	}	*/

	//pNpc->Init();
	if (!m_NpcMap.PutData(pNpc->m_sNid, pNpc))
	{
		spdlog::warn("ServerDlg::AddObjectEventNpc: Npc PutData Fail [serial={}]",
			pNpc->m_sNid);
		delete pNpc;
		pNpc = nullptr;
	}

	m_TotalNPC = m_sMapEventNpc;

	return true;
}

int CServerDlg::GetZoneIndex(int zoneId) const
{
	for (size_t i = 0; i < m_ZoneArray.size(); i++)
	{
		MAP* pMap = m_ZoneArray[i];
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == zoneId)
			return i;
	}

	spdlog::error("ServerDlg::GetZoneIndex: zoneId={} not found", zoneId);
	return -1;
}

int CServerDlg::GetServerNumber(int zoneId) const
{
	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == zoneId)
			return pMap->m_nServerNo;
	}

	spdlog::error("ServerDlg::GetServerNumber: zoneId={} not found", zoneId);
	return -1;
}

void CServerDlg::CloseSocket(int zonenumber)
{
	CGameSocket* pSocket = nullptr;

	for (int i = 0; i < MAX_SOCKET; i++)
	{
		pSocket = (CGameSocket*) m_Iocport.m_SockArray[i];
		if (pSocket == nullptr)
			continue;

		if (pSocket->m_sSocketID == zonenumber)
		{
			//TRACE(_T("size = %d, socket_num = %d, i=%d \n"), size, pSocket->m_sSocketID, i);
			pSocket->CloseProcess();
			m_Iocport.RidIOCPSocket(pSocket->GetSocketID(), pSocket);
		}
	}
}

void CServerDlg::GetServerInfoIni()
{
	CString exePath = GetProgPath();
	std::string exePathUtf8(CT2A(exePath, CP_UTF8));

	std::filesystem::path iniPath(exePath.GetString());
	iniPath /= L"server.ini";
	
	CIni inifile;
	inifile.Load(iniPath);

	// logger setup
	_logger.Setup(inifile, exePathUtf8);
	
	m_byZone = inifile.GetInt(_T("SERVER"), _T("ZONE"), 1);

	std::string datasourceName = inifile.GetString("ODBC", "GAME_DSN", "KN_online");
	std::string datasourceUser = inifile.GetString("ODBC", "GAME_UID", "knight");
	std::string datasourcePass = inifile.GetString("ODBC", "GAME_PWD", "knight");

	ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	// Trigger a save to flush defaults to file.
	inifile.Save();
}

void AIServerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool,
	const std::string& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::AIServerItem, ini::ITEM_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::AIServerUser, ini::USER_LOG_FILE);
}

void CServerDlg::SendSystemMsg(const std::string_view msg, int zone, int type, int who)
{
	int send_index = 0;
	char buff[256] = {};

	SetByte(buff, AG_SYSTEM_MSG, send_index);
	SetByte(buff, type, send_index);				// 채팅형식
	SetShort(buff, who, send_index);				// 누구에게
	SetString2(buff, msg, send_index);

	Send(buff, send_index, zone);
	spdlog::info("ServerDlg::SendSystemMsg: zoneId={} type={} who={} msg={}",
		zone, type, who, msg);
}

void CServerDlg::ResetBattleZone()
{
	spdlog::debug("ServerDlg::ResetBattleZone: start");

	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap== nullptr)
			continue;

		// 현재의 존이 던젼담당하는 존이 아니면 리턴..
		if (pMap->m_byRoomEvent == 0)
			continue;

		// 전체방이 클리어 되었다면
		// if (pMap->IsRoomStatusCheck())
		//	continue;

		pMap->InitializeRoom();
	}

	spdlog::debug("ServerDlg::ResetBattleZone: end");
}

MAP* CServerDlg::GetMapByIndex(int iZoneIndex) const
{
	if (iZoneIndex < 0
		|| iZoneIndex >= static_cast<int>(m_ZoneArray.size()))
	{
		spdlog::error("ServerDlg::GetMapByIndex: zoneIndex={} out of bounds", iZoneIndex);
		return nullptr;
	}

	return m_ZoneArray[iZoneIndex];
}

MAP* CServerDlg::GetMapByID(int iZoneID) const
{
	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == iZoneID)
			return pMap;
	}
	spdlog::error("ServerDlg::GetMapByID: no map found for zoneId={}", iZoneID);
	return nullptr;
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CServerDlg::AddOutputMessage(const std::string& msg)
{
	std::wstring wMsg = LocalToWide(msg);
	AddOutputMessage(wMsg);
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CServerDlg::AddOutputMessage(const std::wstring& msg)
{
	_outputList.AddString(msg.data());
	
	// Set the focus to the last item and ensure it is visible
	int lastIndex = _outputList.GetCount()-1;
	_outputList.SetTopIndex(lastIndex);
}
