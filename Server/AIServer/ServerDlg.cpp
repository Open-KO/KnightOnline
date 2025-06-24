// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "GameSocket.h"
#include "math.h"
#include "MagicTableSet.h"
#include "MagicType1Set.h"
#include "MagicType2Set.h"
#include "MagicType3Set.h"
#include "MagicType4Set.h"
#include "NpcPosSet.h"
#include "ZoneInfoSet.h"
#include "NpcItemSet.h"
#include "NpcTableSet.h"
#include "MonTableSet.h"
#include "MakeWeaponTableSet.h"
#include "MakeDefensiveTableSet.h"
#include "MakeGradeItemTableSet.h"
#include "MakeLareItemTableSet.h"
#include "Region.h"

#include <shared/globals.h>
#include <shared/Ini.h>

//#include "extern.h"			// 전역 객체

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL g_bNpcExit = FALSE;
ZoneArray g_arZone;

CRITICAL_SECTION g_User_critical;
CRITICAL_SECTION g_region_critical;
CRITICAL_SECTION g_LogFileWrite;

#define CHECK_ALIVE 	100		//  게임서버와 통신이 끊김여부 판단, 타이머 변수
#define REHP_TIME		200
#define MONSTER_SPEED	1500

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

	memset(m_strGameDSN, 0, sizeof(m_strGameDSN));
	memset(m_strGameUID, 0, sizeof(m_strGameUID));
	memset(m_strGamePWD, 0, sizeof(m_strGamePWD));
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerDlg)
	DDX_Control(pDX, IDC_LIST1, m_StatusList);
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
	InitializeCriticalSection(&g_LogFileWrite);
	InitializeCriticalSection(&g_region_critical);

	m_sSocketCount = 0;
	m_sErrorSocketCount = 0;
	m_sMapEventNpc = 0;
	m_sReSocketCount = 0;
	m_fReConnectStart = 0.0f;
	m_bFirstServerFlag = FALSE;
	m_byTestMode = NOW_TEST_MODE;

	// User Point Init
	for (int i = 0; i < MAX_USER; i++)
		m_pUser[i] = nullptr;

	// Server Start
	CString logstr;
	CTime time = CTime::GetCurrentTime();
	logstr.Format(_T("[AI ServerStart - %d-%d-%d, %d:%d]"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute());
	m_StatusList.AddString(logstr);
	logstr.Format(_T("[AI ServerStart - %d-%d-%d, %d:%d]\r\n"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute());
	TRACE(logstr);
	LogFileWrite(logstr);

	//----------------------------------------------------------------------
	//	Logfile initialize
	//----------------------------------------------------------------------
	TCHAR strLogFile[50] = {};
	wsprintf(strLogFile, _T("UserLog-%d-%d-%d.txt"), time.GetYear(), time.GetMonth(), time.GetDay());
	m_UserLogFile.Open(strLogFile, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone);
	m_UserLogFile.SeekToEnd();

	memset(strLogFile, 0, sizeof(strLogFile));
	wsprintf(strLogFile, _T("ItemLog-%d-%d-%d.txt"), time.GetYear(), time.GetMonth(), time.GetDay());
	m_ItemLogFile.Open(strLogFile, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone);
	m_ItemLogFile.SeekToEnd();


	//----------------------------------------------------------------------
	//	DB part initialize
	//----------------------------------------------------------------------
	GetServerInfoIni();

	if (m_byZone == UNIFY_ZONE)
		m_strStatus.Format(_T("UNIFY_ZONE 서버의 현재 상태"));
	else if (m_byZone == KARUS_ZONE)
		m_strStatus.Format(_T("KARUS 서버의 현재 상태"));
	else if (m_byZone == ELMORAD_ZONE)
		m_strStatus.Format(_T("ELMORAD 서버의 현재 상태"));
	else if (m_byZone == BATTLE_ZONE)
		m_strStatus.Format(_T("BATTLE 서버의 현재 상태"));

	//----------------------------------------------------------------------
	//	DB part initialize
	//----------------------------------------------------------------------


	//----------------------------------------------------------------------
	//	Communication Part Initialize ...
	//----------------------------------------------------------------------
	m_Iocport.Init(MAX_SOCKET, 1, 1);

	for (int i = 0; i < MAX_SOCKET; i++)
		m_Iocport.m_SockArrayInActive[i] = new CGameSocket;

	//----------------------------------------------------------------------
	//	Load Magic Table
	//----------------------------------------------------------------------
	if (!GetMagicTableData())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType1Data())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType2Data())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType3Data())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMagicType4Data())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	//----------------------------------------------------------------------
	//	Load NPC Item Table
	//----------------------------------------------------------------------
	if (!GetNpcItemTable())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeWeaponItemTableData())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeDefensiveItemTableData())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeGradeItemTableData())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!GetMakeLareItemTableData())
	{
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
		EndDialog(IDCANCEL);
		return FALSE;
	}

	// NPC 특성치 테이블 Load
	if (!GetNpcTableData())
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	//----------------------------------------------------------------------
	//	Load Zone & Event...
	//----------------------------------------------------------------------
	if (!MapFileLoad())
		AfxPostQuitMessage(0);

	if (!CreateNpcThread())
	{
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
	if (m_byZone == KARUS_ZONE
		|| m_byZone == UNIFY_ZONE)
	{
		if (!m_Iocport.Listen(AI_KARUS_SOCKET_PORT))
			AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"), MB_OK);
	}
	else if (m_byZone == ELMORAD_ZONE)
	{
		if (!m_Iocport.Listen(AI_ELMO_SOCKET_PORT))
			AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"), MB_OK);
	}
	else if (m_byZone == BATTLE_ZONE)
	{
		if (!m_Iocport.Listen(AI_BATTLE_SOCKET_PORT))
			AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"), MB_OK);
	}

	//::ResumeThread( m_Iocport.m_hAcceptThread );
	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
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

//	Magic Table 을 읽는다.
BOOL CServerDlg::GetMagicTableData()
{
	CMagicTableSet MagicTableSet;

	if (!MagicTableSet.Open())
	{
		AfxMessageBox(_T("MagicTable Open Fail!"));
		return FALSE;
	}

	if (MagicTableSet.IsBOF()
		|| MagicTableSet.IsEOF())
	{
		AfxMessageBox(_T("MagicTable Empty!"));
		return FALSE;
	}

	MagicTableSet.MoveFirst();

	while (!MagicTableSet.IsEOF())
	{
		_MAGIC_TABLE* pTableMagic = new _MAGIC_TABLE;

		pTableMagic->iNum = MagicTableSet.m_MagicNum;
		pTableMagic->bMoral = MagicTableSet.m_Moral;
		pTableMagic->bSkillLevel = MagicTableSet.m_SkillLevel;
		pTableMagic->sSkill = MagicTableSet.m_Skill;
		pTableMagic->sMsp = MagicTableSet.m_Msp;
		pTableMagic->sHP = MagicTableSet.m_HP;
		pTableMagic->bItemGroup = MagicTableSet.m_ItemGroup;
		pTableMagic->iUseItem = MagicTableSet.m_UseItem;
		pTableMagic->bCastTime = MagicTableSet.m_CastTime;
		pTableMagic->bReCastTime = MagicTableSet.m_ReCastTime;
		pTableMagic->bSuccessRate = MagicTableSet.m_SuccessRate;
		pTableMagic->bType1 = MagicTableSet.m_Type1;
		pTableMagic->bType2 = MagicTableSet.m_Type2;
		pTableMagic->sRange = MagicTableSet.m_Range;
		pTableMagic->bEtc = MagicTableSet.m_Etc;

		if (!m_MagictableArray.PutData(pTableMagic->iNum, pTableMagic))
		{
			TRACE("MagicTable PutData Fail - %d\n", pTableMagic->iNum);
			delete pTableMagic;
			pTableMagic = nullptr;
		}

		MagicTableSet.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMakeWeaponItemTableData()
{
	CMakeWeaponTableSet	MakeItemTableSet;

	if (!MakeItemTableSet.Open())
	{
		AfxMessageBox(_T("GetMakeWeaponItemTableData Open Fail!"));
		return FALSE;
	}

	if (MakeItemTableSet.IsBOF()
		|| MakeItemTableSet.IsEOF())
	{
		AfxMessageBox(_T("GetMakeWeaponItemTableData Empty!"));
		return FALSE;
	}

	MakeItemTableSet.MoveFirst();

	while (!MakeItemTableSet.IsEOF())
	{
		_MAKE_WEAPON* pTable = new _MAKE_WEAPON;

		pTable->byIndex = MakeItemTableSet.m_byLevel;
		pTable->sClass[0] = MakeItemTableSet.m_sClass_1;
		pTable->sClass[1] = MakeItemTableSet.m_sClass_2;
		pTable->sClass[2] = MakeItemTableSet.m_sClass_3;
		pTable->sClass[3] = MakeItemTableSet.m_sClass_4;
		pTable->sClass[4] = MakeItemTableSet.m_sClass_5;
		pTable->sClass[5] = MakeItemTableSet.m_sClass_6;
		pTable->sClass[6] = MakeItemTableSet.m_sClass_7;
		pTable->sClass[7] = MakeItemTableSet.m_sClass_8;
		pTable->sClass[8] = MakeItemTableSet.m_sClass_9;
		pTable->sClass[9] = MakeItemTableSet.m_sClass_10;
		pTable->sClass[10] = MakeItemTableSet.m_sClass_11;
		pTable->sClass[11] = MakeItemTableSet.m_sClass_12;

		if (!m_MakeWeaponItemArray.PutData(pTable->byIndex, pTable))
		{
			TRACE("GetMakeWeaponItemTableData PutData Fail - %d\n", pTable->byIndex);
			delete pTable;
			pTable = nullptr;
		}

		MakeItemTableSet.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMakeDefensiveItemTableData()
{
	CMakeDefensiveTableSet MakeItemTableSet;

	if (!MakeItemTableSet.Open())
	{
		AfxMessageBox(_T("GetMakeDefensiveItemTableData Open Fail!"));
		return FALSE;
	}

	if (MakeItemTableSet.IsBOF()
		|| MakeItemTableSet.IsEOF())
	{
		AfxMessageBox(_T("GetMakeDefensiveItemTableData Empty!"));
		return FALSE;
	}

	MakeItemTableSet.MoveFirst();

	while (!MakeItemTableSet.IsEOF())
	{
		_MAKE_WEAPON* pTable = new _MAKE_WEAPON;

		pTable->byIndex = MakeItemTableSet.m_byLevel;
		pTable->sClass[0] = MakeItemTableSet.m_sClass_1;
		pTable->sClass[1] = MakeItemTableSet.m_sClass_2;
		pTable->sClass[2] = MakeItemTableSet.m_sClass_3;
		pTable->sClass[3] = MakeItemTableSet.m_sClass_4;
		pTable->sClass[4] = MakeItemTableSet.m_sClass_5;
		pTable->sClass[5] = MakeItemTableSet.m_sClass_6;
		pTable->sClass[6] = MakeItemTableSet.m_sClass_7;

		if (!m_MakeDefensiveItemArray.PutData(pTable->byIndex, pTable))
		{
			TRACE("GetMakeDefensiveItemTableData PutData Fail - %d\n", pTable->byIndex);
			delete pTable;
			pTable = nullptr;
		}

		MakeItemTableSet.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMakeGradeItemTableData()
{
	CMakeGradeItemTableSet MakeItemTableSet;

	if (!MakeItemTableSet.Open())
	{
		AfxMessageBox(_T("MakeGradeItemTable Open Fail!"));
		return FALSE;
	}

	if (MakeItemTableSet.IsBOF()
		|| MakeItemTableSet.IsEOF())
	{
		AfxMessageBox(_T("MakeGradeItemTable Empty!"));
		return FALSE;
	}

	MakeItemTableSet.MoveFirst();

	while (!MakeItemTableSet.IsEOF())
	{
		_MAKE_ITEM_GRADE_CODE* pTable = new _MAKE_ITEM_GRADE_CODE;

		pTable->byItemIndex = MakeItemTableSet.m_byItemIndex;
		pTable->sGrade_1 = MakeItemTableSet.m_byGrade_1;
		pTable->sGrade_2 = MakeItemTableSet.m_byGrade_2;
		pTable->sGrade_3 = MakeItemTableSet.m_byGrade_3;
		pTable->sGrade_4 = MakeItemTableSet.m_byGrade_4;
		pTable->sGrade_5 = MakeItemTableSet.m_byGrade_5;
		pTable->sGrade_6 = MakeItemTableSet.m_byGrade_6;
		pTable->sGrade_7 = MakeItemTableSet.m_byGrade_7;
		pTable->sGrade_8 = MakeItemTableSet.m_byGrade_8;
		pTable->sGrade_9 = MakeItemTableSet.m_byGrade_9;

		if (!m_MakeGradeItemArray.PutData(pTable->byItemIndex, pTable))
		{
			TRACE("MakeGradeItemTable PutData Fail - %d\n", pTable->byItemIndex);
			delete pTable;
			pTable = nullptr;
		}

		MakeItemTableSet.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMakeLareItemTableData()
{
	CMakeLareItemTableSet MakeItemTableSet;

	if (!MakeItemTableSet.Open())
	{
		AfxMessageBox(_T("MakeLareItemTable Open Fail!"));
		return FALSE;
	}

	if (MakeItemTableSet.IsBOF()
		|| MakeItemTableSet.IsEOF())
	{
		AfxMessageBox(_T("MakeLareItemTable Empty!"));
		return FALSE;
	}

	MakeItemTableSet.MoveFirst();

	while (!MakeItemTableSet.IsEOF())
	{
		_MAKE_ITEM_LARE_CODE* pTable = new _MAKE_ITEM_LARE_CODE;

		pTable->byItemLevel = MakeItemTableSet.m_byLevelGrade;
		pTable->sLareItem = MakeItemTableSet.m_sLareItem;
		pTable->sMagicItem = MakeItemTableSet.m_sMagicItem;
		pTable->sGereralItem = MakeItemTableSet.m_sGereralItem;

		if (!m_MakeLareItemArray.PutData(pTable->byItemLevel, pTable))
		{
			TRACE("MakeItemTable PutData Fail - %d\n", pTable->byItemLevel);
			delete pTable;
			pTable = nullptr;
		}

		MakeItemTableSet.MoveNext();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
//	NPC Item Table 을 읽는다.
//
BOOL CServerDlg::GetNpcItemTable()
{
	CNpcItemSet NpcItemSet;
	int nRowCount = 0;
	short i;
	int nItem = 0;

	try
	{
		if (NpcItemSet.IsOpen())
			NpcItemSet.Close();

		NpcItemSet.m_strSort = _T("sIndex");

		if (!NpcItemSet.Open())
		{
			AfxMessageBox(_T("MONSTER ITEM DB Open Fail!"));
			return FALSE;
		}

		if (NpcItemSet.IsBOF())
		{
			AfxMessageBox(_T("MONSTER ITEM DB Empty!"));
			return FALSE;
		}

		while (!NpcItemSet.IsEOF())
		{
			nRowCount++;
			NpcItemSet.MoveNext();
		}

		m_NpcItem.m_nField = NpcItemSet.m_nFields;
		m_NpcItem.m_nRow = nRowCount;

		if (nRowCount == 0)
			return FALSE;

		m_NpcItem.m_ppItem = new int* [m_NpcItem.m_nRow];
		for (i = 0; i < m_NpcItem.m_nRow; i++)
			m_NpcItem.m_ppItem[i] = new int[m_NpcItem.m_nField];

		NpcItemSet.MoveFirst();

		i = 0;
		while (!NpcItemSet.IsEOF())
		{
			m_NpcItem.m_ppItem[i][0] = NpcItemSet.m_sIndex;
			m_NpcItem.m_ppItem[i][1] = NpcItemSet.m_iItem01;
			m_NpcItem.m_ppItem[i][2] = NpcItemSet.m_sPersent01;
			m_NpcItem.m_ppItem[i][3] = NpcItemSet.m_iItem02;
			m_NpcItem.m_ppItem[i][4] = NpcItemSet.m_sPersent02;
			m_NpcItem.m_ppItem[i][5] = NpcItemSet.m_iItem03;
			m_NpcItem.m_ppItem[i][6] = NpcItemSet.m_sPersent03;
			m_NpcItem.m_ppItem[i][7] = NpcItemSet.m_iItem04;
			m_NpcItem.m_ppItem[i][8] = NpcItemSet.m_sPersent04;
			m_NpcItem.m_ppItem[i][9] = NpcItemSet.m_iItem05;
			m_NpcItem.m_ppItem[i][10] = NpcItemSet.m_sPersent05;

			i++;
			NpcItemSet.MoveNext();
		}
	}
	catch (CMemoryException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}
	catch (CDBException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}

	return TRUE;
}

//	Monster Table Data 를 읽는다.
BOOL CServerDlg::GetMonsterTableData()
{
	CMonTableSet NpcTableSet;

	try
	{
		//if(m_arMonTable.GetSize()) return FALSE;

		if (NpcTableSet.IsOpen())
			NpcTableSet.Close();

		if (!NpcTableSet.Open())
		{
			AfxMessageBox(_T("MONSTER DB Open Fail!"));
			return FALSE;
		}

		if (NpcTableSet.IsBOF())
		{
			AfxMessageBox(_T("MONSTER DB Empty!"));
			return FALSE;
		}

		while (!NpcTableSet.IsEOF())
		{
			CNpcTable* Npc = new CNpcTable;
			CString tmpNpcName;
			Npc->Initialize();

			Npc->m_sSid = NpcTableSet.m_sSid;						// MONSTER(NPC) Serial ID

			if (NpcTableSet.m_strName.GetLength() <= MAX_NPC_NAME_SIZE)
				strcpy(Npc->m_strName, CT2A(NpcTableSet.m_strName)); // MONSTER(NPC) Name
			else
				strncpy(Npc->m_strName, CT2A(NpcTableSet.m_strName), MAX_NPC_NAME_SIZE);

			Npc->m_sPid = NpcTableSet.m_sPid;						// MONSTER(NPC) Picture ID
			Npc->m_sSize = NpcTableSet.m_sSize;						// MONSTER(NPC) 캐릭 크기 비율
			Npc->m_iWeapon_1 = NpcTableSet.m_iWeapon1;				// 착용무기
			Npc->m_iWeapon_2 = NpcTableSet.m_iWeapon2;				// 착용무기
			Npc->m_byGroup = NpcTableSet.m_byGroup;					// 소속집단
			Npc->m_byActType = NpcTableSet.m_byActType;				// 행동패턴
			Npc->m_byRank = NpcTableSet.m_byRank;					// 작위
			Npc->m_byTitle = NpcTableSet.m_byTitle;					// 지위
			Npc->m_iSellingGroup = NpcTableSet.m_iSellingGroup;		// item group
			Npc->m_sLevel = NpcTableSet.m_sLevel;					// level
			Npc->m_iExp = NpcTableSet.m_iExp;						// 경험치
			Npc->m_iLoyalty = NpcTableSet.m_iLoyalty;				// loyalty
			Npc->m_iMaxHP = NpcTableSet.m_iHpPoint;					// 최대 HP
			Npc->m_sMaxMP = NpcTableSet.m_sMpPoint;					// 최대 MP
			Npc->m_sAttack = NpcTableSet.m_sAtk;					// 공격값
			Npc->m_sDefense = NpcTableSet.m_sAc;					// 방어값
			Npc->m_sHitRate = NpcTableSet.m_sHitRate;				// 타격성공률
			Npc->m_sEvadeRate = NpcTableSet.m_sEvadeRate;			// 회피성공률
			Npc->m_sDamage = NpcTableSet.m_sDamage;					// 기본 데미지
			Npc->m_sAttackDelay = NpcTableSet.m_sAttackDelay;		// 공격딜레이
			Npc->m_bySpeed_1 = NpcTableSet.m_bySpeed1;				// 이동속도	(걷기)
			Npc->m_bySpeed_2 = NpcTableSet.m_bySpeed2;				// 이동속도	(뛰기)
			Npc->m_sSpeed = MONSTER_SPEED;							// 이동속도	
			Npc->m_sStandTime = NpcTableSet.m_sStandtime;			// 서있는 시간
			Npc->m_iMagic1 = NpcTableSet.m_iMagic1;					// 사용마법 1
			Npc->m_iMagic2 = NpcTableSet.m_iMagic2;					// 사용마법 2
			Npc->m_iMagic3 = NpcTableSet.m_iMagic3;					// 사용마법 3	
			Npc->m_sFireR = NpcTableSet.m_sFireR;					// 화염 저항력
			Npc->m_sColdR = NpcTableSet.m_sColdR;					// 냉기 저항력
			Npc->m_sLightningR = NpcTableSet.m_sLightningR;			// 전기 저항력
			Npc->m_sMagicR = NpcTableSet.m_sMagicR;					// 마법 저항력
			Npc->m_sDiseaseR = NpcTableSet.m_sDiseaseR;				// 저주 저항력
			Npc->m_sPoisonR = NpcTableSet.m_sPoisonR;				// 독 저항력
			Npc->m_sLightR = NpcTableSet.m_sLightR;					// 빛 저항력
			Npc->m_sBulk = NpcTableSet.m_sBulk;
			Npc->m_bySearchRange = NpcTableSet.m_bySearchRange;		// 적 탐지 범위
			Npc->m_byAttackRange = NpcTableSet.m_byAttackRange;		// 사정거리
			Npc->m_byTracingRange = NpcTableSet.m_byTracingRange;	// 추격거리
			//Npc->m_sAI = NpcTableSet.m_sAI;						// 인공지능 인덱스
			Npc->m_tNpcType = NpcTableSet.m_byType;					// NPC Type
																	// 0 : Monster
																	// 1 : Normal NPC

			Npc->m_byFamilyType = NpcTableSet.m_byFamily;			// 몹들사이에서 가족관계를 결정한다.
			//Npc->m_tItemPer;										// 아이템이 떨어질 확률
			//Npc->m_tDnPer;										// 돈이 떨어질확률

			Npc->m_iMoney = NpcTableSet.m_iMoney;					// 떨어지는 돈
			Npc->m_iItem = NpcTableSet.m_sItem;						// 떨어지는 아이템
			Npc->m_byDirectAttack = NpcTableSet.m_byDirectAttack;
			Npc->m_byMagicAttack = NpcTableSet.m_byMagicAttack;

			if (!m_arMonTable.PutData(Npc->m_sSid, Npc))
			{
				TRACE("GetMonsterTableData - PutData Fail - %d\n", Npc->m_sSid);
				delete Npc;
				Npc = nullptr;
			}
			//m_arMonTable.Add(Npc);

			NpcTableSet.MoveNext();
		}

		NpcTableSet.Close();
	}
	catch (CMemoryException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}
	catch (CDBException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}

	return TRUE;
}

//	NPC Table Data 를 읽는다. (경비병 & NPC)
BOOL CServerDlg::GetNpcTableData()
{
	CNpcTableSet NpcTableSet;

	try
	{
		//if(m_arNpcTable.GetSize()) return FALSE;

		if (NpcTableSet.IsOpen())
			NpcTableSet.Close();

		if (!NpcTableSet.Open())
		{
			AfxMessageBox(_T("NPC DB Open Fail!"));
			return FALSE;
		}

		if (NpcTableSet.IsBOF())
		{
			AfxMessageBox(_T("NPC DB Empty!"));
			return FALSE;
		}

		while (!NpcTableSet.IsEOF())
		{
			CNpcTable* Npc = new CNpcTable;
			CString tmpNpcName;
			Npc->Initialize();

			Npc->m_sSid = NpcTableSet.m_sSid;						// MONSTER(NPC) Serial ID

			if (NpcTableSet.m_strName.GetLength() <= MAX_NPC_NAME_SIZE)
				strcpy(Npc->m_strName, CT2A(NpcTableSet.m_strName)); // MONSTER(NPC) Name
			else
				strncpy(Npc->m_strName, CT2A(NpcTableSet.m_strName), MAX_NPC_NAME_SIZE);

			Npc->m_sPid = NpcTableSet.m_sPid;						// MONSTER(NPC) Picture ID
			Npc->m_sSize = NpcTableSet.m_sSize;						// MONSTER(NPC) 캐릭 크기 비율
			Npc->m_iWeapon_1 = NpcTableSet.m_iWeapon1;				// 착용무기
			Npc->m_iWeapon_2 = NpcTableSet.m_iWeapon2;				// 착용무기
			Npc->m_byGroup = NpcTableSet.m_byGroup;					// 소속집단
			Npc->m_byActType = NpcTableSet.m_byActType;				// 행동패턴
			Npc->m_byRank = NpcTableSet.m_byRank;					// 작위
			Npc->m_byTitle = NpcTableSet.m_byTitle;					// 지위
			Npc->m_iSellingGroup = NpcTableSet.m_iSellingGroup;		// item group
			Npc->m_sLevel = NpcTableSet.m_sLevel;					// level
			Npc->m_iExp = NpcTableSet.m_iExp;						// 경험치
			Npc->m_iLoyalty = NpcTableSet.m_iLoyalty;				// loyalty
			Npc->m_iMaxHP = NpcTableSet.m_iHpPoint;					// 최대 HP
			Npc->m_sMaxMP = NpcTableSet.m_sMpPoint;					// 최대 MP
			Npc->m_sAttack = NpcTableSet.m_sAtk;					// 공격값
			Npc->m_sDefense = NpcTableSet.m_sAc;					// 방어값
			Npc->m_sHitRate = NpcTableSet.m_sHitRate;				// 타격성공률
			Npc->m_sEvadeRate = NpcTableSet.m_sEvadeRate;			// 회피성공률
			Npc->m_sDamage = NpcTableSet.m_sDamage;					// 기본 데미지
			Npc->m_sAttackDelay = NpcTableSet.m_sAttackDelay;		// 공격딜레이
			Npc->m_bySpeed_1 = NpcTableSet.m_bySpeed1;				// 이동속도	(걷기)
			Npc->m_bySpeed_2 = NpcTableSet.m_bySpeed2;				// 이동속도	(뛰기)
			Npc->m_sSpeed = MONSTER_SPEED;							// 이동속도	
			Npc->m_sStandTime = NpcTableSet.m_sStandtime;			// 서있는 시간
			Npc->m_iMagic1 = NpcTableSet.m_iMagic1;					// 사용마법 1
			Npc->m_iMagic2 = NpcTableSet.m_iMagic2;					// 사용마법 2
			Npc->m_iMagic3 = NpcTableSet.m_iMagic3;					// 사용마법 3	
			Npc->m_sFireR = NpcTableSet.m_sFireR;					// 화염 저항력
			Npc->m_sColdR = NpcTableSet.m_sColdR;					// 냉기 저항력
			Npc->m_sLightningR = NpcTableSet.m_sLightningR;			// 전기 저항력
			Npc->m_sMagicR = NpcTableSet.m_sMagicR;					// 마법 저항력
			Npc->m_sDiseaseR = NpcTableSet.m_sDiseaseR;				// 저주 저항력
			Npc->m_sPoisonR = NpcTableSet.m_sPoisonR;				// 독 저항력
			Npc->m_sLightR = NpcTableSet.m_sLightR;					// 빛 저항력
			Npc->m_sBulk = NpcTableSet.m_sBulk;
			Npc->m_bySearchRange = NpcTableSet.m_bySearchRange;		// 적 탐지 범위
			Npc->m_byAttackRange = NpcTableSet.m_byAttackRange;		// 사정거리
			Npc->m_byTracingRange = NpcTableSet.m_byTracingRange;	// 추격거리
			//Npc->m_sAI = NpcTableSet.m_sAI;						// 인공지능 인덱스
			Npc->m_tNpcType = NpcTableSet.m_byType;					// NPC Type
																	// 0 : Monster
																	// 1 : Normal NPC

			Npc->m_byFamilyType = NpcTableSet.m_byFamily;			// 몹들사이에서 가족관계를 결정한다.
			//Npc->m_tItemPer;										// 아이템이 떨어질 확률
			//Npc->m_tDnPer;										// 돈이 떨어질확률

			Npc->m_iMoney = NpcTableSet.m_iMoney;					// 떨어지는 돈
			Npc->m_iItem = NpcTableSet.m_sItem;						// 떨어지는 아이템
			Npc->m_byDirectAttack = NpcTableSet.m_byDirectAttack;
			Npc->m_byMagicAttack = NpcTableSet.m_byMagicAttack;

			if (!m_arNpcTable.PutData(Npc->m_sSid, Npc))
			{
				TRACE("GetNpcTableData - PutData Fail - %d\n", Npc->m_sSid);
				delete Npc;
				Npc = nullptr;
			}
			//m_arNpcTable.Add(Npc);

			NpcTableSet.MoveNext();
		}

		NpcTableSet.Close();
	}
	catch (CMemoryException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}
	catch (CDBException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}

	return TRUE;
}

//	Npc Thread 를 만든다.
BOOL CServerDlg::CreateNpcThread()
{
	BOOL	bMoveNext = TRUE;
	int		nSerial = m_sMapEventNpc;
	int		nPathSerial = 1;
	int		nNpcCount = 0;
	int		i = 0, j = 0;
	int		nRandom = 0, nServerNum = 0;
	double  dbSpeed = 0;

	m_TotalNPC = 0;			// DB에 있는 수
	m_CurrentNPC = 0;
	m_CurrentNPCError = 0;

	CNpcTable* pNpcTable = nullptr;
	CRoomEvent* pRoom = nullptr;

	CNpcPosSet NpcPosSet;

	char szPath[500];
	char szX[5];
	char szZ[5];

	float fRandom_X = 0.0f;
	float fRandom_Z = 0.0f;
	BOOL bFindNpcTable = FALSE;

	int nMonsterNumber = 0;

	try
	{
		if (NpcPosSet.IsOpen())
			NpcPosSet.Close();

		if (!NpcPosSet.Open())
		{
			AfxMessageBox(_T("MONSTER_POS DB Open Fail!"));
			return FALSE;
		}

		if (NpcPosSet.IsBOF())
		{
			AfxMessageBox(_T("MONSTER_POS DB Empty!"));
			return FALSE;
		}

		NpcPosSet.MoveFirst();

		while (!NpcPosSet.IsEOF())
		{
			nMonsterNumber = NpcPosSet.m_NumNPC;
			//if( NpcPosSet.m_ZoneID == 101 )	{	// 테스트를 위해서,, 
			//	nMonsterNumber = 1;
				//if(nMonsterNumber > 4)	{
				//	nMonsterNumber = nMonsterNumber / 4;
				//}
			//}	
			nServerNum = 0;
			nServerNum = GetServerNumber(NpcPosSet.m_ZoneID);

			if (m_byZone == nServerNum
				|| m_byZone == UNIFY_ZONE)
			{
				for (j = 0; j < nMonsterNumber; j++)
				{
					CNpc* pNpc = new CNpc;
					pNpc->m_sNid = nSerial++;					// 서버 내에서의 고유 번호
					pNpc->m_sSid = (short) NpcPosSet.m_NpcID;	// MONSTER(NPC) Serial ID

					pNpc->m_byMoveType = NpcPosSet.m_ActType;
					pNpc->m_byInitMoveType = NpcPosSet.m_ActType;
					bFindNpcTable = FALSE;

					if (NpcPosSet.m_ActType >= 0 && NpcPosSet.m_ActType < 100)
					{
						pNpcTable = m_arMonTable.GetData(pNpc->m_sSid);
						if (pNpcTable != nullptr)
							bFindNpcTable = TRUE;
					}
					else if (NpcPosSet.m_ActType >= 100)
					{
						pNpc->m_byMoveType = NpcPosSet.m_ActType - 100;
						//pNpc->m_byInitMoveType = NpcPosSet.m_ActType - 100;

						pNpcTable = m_arNpcTable.GetData(pNpc->m_sSid);
						if (pNpcTable != nullptr)
							bFindNpcTable = TRUE;
					}

					pNpc->m_byBattlePos = 0;

					if (pNpc->m_byMoveType >= 2)
					{
						pNpc->m_byBattlePos = myrand(1, 3);
						pNpc->m_byPathCount = nPathSerial++;
					}

					pNpc->InitPos();

					if (!bFindNpcTable)
					{
						TRACE("#### CreateNpcThread Fail : [nid = %d, sid = %d] #####\n", pNpc->m_sNid, pNpc->m_sSid);
						break;
					}

					if (bMoveNext)
					{
						bMoveNext = FALSE;
						nNpcCount = NpcPosSet.m_NumNPC;
					}

					strcpy(pNpc->m_strName, pNpcTable->m_strName);			// MONSTER(NPC) Name

					pNpc->m_sPid = pNpcTable->m_sPid;						// MONSTER(NPC) Picture ID
					pNpc->m_sSize = pNpcTable->m_sSize;						// 캐릭터의 비율(100 퍼센트 기준)
					pNpc->m_iWeapon_1 = pNpcTable->m_iWeapon_1;				// 착용무기
					pNpc->m_iWeapon_2 = pNpcTable->m_iWeapon_2;				// 착용무기
					pNpc->m_byGroup = pNpcTable->m_byGroup;					// 소속집단
					pNpc->m_byActType = pNpcTable->m_byActType;				// 행동패턴
					pNpc->m_byRank = pNpcTable->m_byRank;					// 작위
					pNpc->m_byTitle = pNpcTable->m_byTitle;					// 지위
					pNpc->m_iSellingGroup = pNpcTable->m_iSellingGroup;
					pNpc->m_sLevel = pNpcTable->m_sLevel;					// level
					pNpc->m_iExp = pNpcTable->m_iExp;						// 경험치
					pNpc->m_iLoyalty = pNpcTable->m_iLoyalty;				// loyalty
					pNpc->m_iHP = pNpcTable->m_iMaxHP;						// 최대 HP
					pNpc->m_iMaxHP = pNpcTable->m_iMaxHP;					// 현재 HP
					pNpc->m_sMP = pNpcTable->m_sMaxMP;						// 최대 MP
					pNpc->m_sMaxMP = pNpcTable->m_sMaxMP;					// 현재 MP
					pNpc->m_sAttack = pNpcTable->m_sAttack;					// 공격값
					pNpc->m_sDefense = pNpcTable->m_sDefense;				// 방어값
					pNpc->m_sHitRate = pNpcTable->m_sHitRate;				// 타격성공률
					pNpc->m_sEvadeRate = pNpcTable->m_sEvadeRate;			// 회피성공률
					pNpc->m_sDamage = pNpcTable->m_sDamage;					// 기본 데미지
					pNpc->m_sAttackDelay = pNpcTable->m_sAttackDelay;		// 공격딜레이
					pNpc->m_sSpeed = pNpcTable->m_sSpeed;					// 이동속도
					dbSpeed = pNpcTable->m_sSpeed;
					pNpc->m_fSpeed_1 = (float) pNpcTable->m_bySpeed_1 * (dbSpeed / 1000);		// 기본 이동 타입
					pNpc->m_fSpeed_2 = (float) pNpcTable->m_bySpeed_2 * (dbSpeed / 1000);		// 뛰는 이동 타입..
					pNpc->m_fOldSpeed_1 = (float) pNpcTable->m_bySpeed_1 * (dbSpeed / 1000);	// 기본 이동 타입
					pNpc->m_fOldSpeed_2 = (float) pNpcTable->m_bySpeed_2 * (dbSpeed / 1000);	// 뛰는 이동 타입..
					pNpc->m_fSecForMetor = 4.0f;							// 초당 갈 수 있는 거리..
					pNpc->m_sStandTime = pNpcTable->m_sStandTime;			// 서있는 시간
					pNpc->m_iMagic1 = pNpcTable->m_iMagic1;					// 사용마법 1
					pNpc->m_iMagic2 = pNpcTable->m_iMagic2;					// 사용마법 2
					pNpc->m_iMagic3 = pNpcTable->m_iMagic3;					// 사용마법 3
					pNpc->m_sFireR = pNpcTable->m_sFireR;					// 화염 저항력
					pNpc->m_sColdR = pNpcTable->m_sColdR;					// 냉기 저항력
					pNpc->m_sLightningR = pNpcTable->m_sLightningR;			// 전기 저항력
					pNpc->m_sMagicR = pNpcTable->m_sMagicR;					// 마법 저항력
					pNpc->m_sDiseaseR = pNpcTable->m_sDiseaseR;				// 저주 저항력
					pNpc->m_sPoisonR = pNpcTable->m_sPoisonR;				// 독 저항력
					pNpc->m_sLightR = pNpcTable->m_sLightR;					// 빛 저항력
					pNpc->m_fBulk = (float) (((double) pNpcTable->m_sBulk / 100) * ((double) pNpcTable->m_sSize / 100));
					pNpc->m_bySearchRange = pNpcTable->m_bySearchRange;		// 적 탐지 범위
					pNpc->m_byAttackRange = pNpcTable->m_byAttackRange;		// 사정거리
					pNpc->m_byTracingRange = pNpcTable->m_byTracingRange;	// 추격거리
					pNpc->m_sAI = pNpcTable->m_sAI;							// 인공지능 인덱스
					pNpc->m_tNpcType = pNpcTable->m_tNpcType;				// NPC Type
					pNpc->m_byFamilyType = pNpcTable->m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
					pNpc->m_iMoney = pNpcTable->m_iMoney;					// 떨어지는 돈
					pNpc->m_iItem = pNpcTable->m_iItem;						// 떨어지는 아이템
					pNpc->m_tNpcLongType = pNpcTable->m_byDirectAttack;
					pNpc->m_byWhatAttackType = pNpcTable->m_byMagicAttack;

					//////// MONSTER POS ////////////////////////////////////////
					pNpc->m_sCurZone = NpcPosSet.m_ZoneID;

					// map에 몬스터의 위치를 랜덤하게 위치시킨다.. (테스트 용 : 수정-DB에서 읽어오는데로 몬 위치 결정)
					nRandom = abs(NpcPosSet.m_LeftX - NpcPosSet.m_RightX);
					if (nRandom <= 1)
					{
						fRandom_X = (float) NpcPosSet.m_LeftX;
					}
					else
					{
						if (NpcPosSet.m_LeftX < NpcPosSet.m_RightX)
							fRandom_X = (float) myrand(NpcPosSet.m_LeftX, NpcPosSet.m_RightX);
						else
							fRandom_X = (float) myrand(NpcPosSet.m_RightX, NpcPosSet.m_LeftX);
					}

					nRandom = abs(NpcPosSet.m_TopZ - NpcPosSet.m_BottomZ);
					if (nRandom <= 1)
					{
						fRandom_Z = (float) NpcPosSet.m_TopZ;
					}
					else
					{
						if (NpcPosSet.m_TopZ < NpcPosSet.m_BottomZ)
							fRandom_Z = (float) myrand(NpcPosSet.m_TopZ, NpcPosSet.m_BottomZ);
						else
							fRandom_Z = (float) myrand(NpcPosSet.m_BottomZ, NpcPosSet.m_TopZ);
					}

					pNpc->m_fCurX = fRandom_X;
					pNpc->m_fCurY = 0;
					pNpc->m_fCurZ = fRandom_Z;

					if (NpcPosSet.m_RegTime < 15)
					{
						TRACE("##### ServerDlg:CreateNpcThread - RegenTime Error :  nid=%d, name=%s, regentime=%d #####\n", pNpc->m_sNid + NPC_BAND, pNpc->m_strName, NpcPosSet.m_RegTime);
						NpcPosSet.m_RegTime = 30;
					}

					pNpc->m_sRegenTime = NpcPosSet.m_RegTime * 1000;	// 초(DB)단위-> 밀리세컨드로

					pNpc->m_sMaxPathCount = NpcPosSet.m_DotCnt;

					if (pNpc->m_byMoveType >= 2
						&& NpcPosSet.m_DotCnt == 0)
					{
						TRACE("##### ServerDlg:CreateNpcThread - Path type Error :  nid=%d, sid=%d, name=%s, acttype=%d, path=%d #####\n", pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName, pNpc->m_byMoveType, pNpc->m_sMaxPathCount);
						return FALSE;
					}

					int index = 0;
					memset(szPath, 0, 500);
					strcpy(szPath, CT2A(NpcPosSet.m_path));

					if (NpcPosSet.m_DotCnt != 0)
					{
						for (int l = 0; l < NpcPosSet.m_DotCnt; l++)
						{
							memset(szX, 0x00, 5);
							memset(szZ, 0x00, 5);
							GetString(szX, szPath, 4, index);
							GetString(szZ, szPath, 4, index);
							pNpc->m_PathList.pPattenPos[l].x = atoi(szX);
							pNpc->m_PathList.pPattenPos[l].z = atoi(szZ);
						//	TRACE(" l=%d, x=%d, z=%d\n", l, pNpc->m_PathList.pPattenPos[l].x, pNpc->m_PathList.pPattenPos[l].z);
						}
					}

					//pNpc->m_byType			= NpcPosSet.m_byType;

					pNpc->m_tItemPer = pNpcTable->m_tItemPer;	// NPC Type
					pNpc->m_tDnPer = pNpcTable->m_tDnPer;	// NPC Type

					pNpc->m_nInitMinX = pNpc->m_nLimitMinX = NpcPosSet.m_LeftX;
					pNpc->m_nInitMinY = pNpc->m_nLimitMinZ = NpcPosSet.m_TopZ;
					pNpc->m_nInitMaxX = pNpc->m_nLimitMaxX = NpcPosSet.m_RightX;
					pNpc->m_nInitMaxY = pNpc->m_nLimitMaxZ = NpcPosSet.m_BottomZ;
					// dungeon work
					pNpc->m_byDungeonFamily = NpcPosSet.m_DungeonFamily;
					pNpc->m_bySpecialType = NpcPosSet.m_SpecialType;
					pNpc->m_byRegenType = NpcPosSet.m_RegenType;
					pNpc->m_byTrapNumber = NpcPosSet.m_TrapNumber;

					if (pNpc->m_byDungeonFamily > 0)
					{
						pNpc->m_nLimitMinX = NpcPosSet.m_LimitMinX;
						pNpc->m_nLimitMinZ = NpcPosSet.m_LimitMinZ;
						pNpc->m_nLimitMaxX = NpcPosSet.m_LimitMaxX;
						pNpc->m_nLimitMaxZ = NpcPosSet.m_LimitMaxZ;
					}

					pNpc->m_ZoneIndex = -1;

					for (i = 0; i < g_arZone.size(); i++)
					{
						if (g_arZone[i]->m_nZoneNumber == pNpc->m_sCurZone)
						{
							pNpc->m_ZoneIndex = i;
							break;
						}
					}

					if (pNpc->m_ZoneIndex == -1)
					{
						AfxMessageBox(_T("Error : CServerDlg,, Invaild zone Index!!"));
						return FALSE;
					}

					//pNpc->Init();
					//m_arNpc.Add(pNpc);
					if (!m_arNpc.PutData(pNpc->m_sNid, pNpc))
					{
						TRACE("Npc PutData Fail - %d\n", pNpc->m_sNid);
						delete pNpc;
						pNpc = nullptr;
					}

					// 
					if (pNpc != nullptr
						&& g_arZone[pNpc->m_ZoneIndex]->m_byRoomEvent > 0
						&& pNpc->m_byDungeonFamily > 0)
					{
						pRoom = nullptr;
						pRoom = g_arZone[pNpc->m_ZoneIndex]->m_arRoomEventArray.GetData(pNpc->m_byDungeonFamily);
						if (pRoom == nullptr)
						{
							TRACE("Error : CServerDlg,, Map Room Npc Fail!! : nid=%d, sid=%d, name=%s, fam=%d, zoneindex=%d\n", pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName, pNpc->m_byDungeonFamily, pNpc->m_ZoneIndex);
							AfxMessageBox(_T("Error : CServerDlg,, Map Room Npc Fail!!"));
							return FALSE;
						}

						int* pInt = nullptr;
						pInt = new int;
						*pInt = pNpc->m_sNid;
						if (!pRoom->m_mapRoomNpcArray.PutData(pNpc->m_sNid, pInt))
						{
							delete pInt;
							TRACE("### Map - Room Array MonsterNid Fail : nid=%d, sid=%d ###\n", pNpc->m_sNid, pNpc->m_sSid);
						}
					}

					m_TotalNPC = nSerial;

					if (--nNpcCount > 0)
						continue;

					bMoveNext = TRUE;
					nNpcCount = 0;
				}
			}

			nPathSerial = 1;
			NpcPosSet.MoveNext();
		}

		NpcPosSet.Close();
	}

	catch (CMemoryException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}
	catch (CDBException* e)
	{
		e->ReportError();
		e->Delete();

		return FALSE;
	}

	int step = 0;
	int nThreadNumber = 0;
	CNpcThread* pNpcThread = nullptr;

	for (i = 0; i < m_arNpc.GetSize(); i++)
	{
		if (step == 0)
			pNpcThread = new CNpcThread;

		pNpcThread->m_pNpc[step] = m_arNpc.GetData(i);
		pNpcThread->m_pNpc[step]->m_sThreadNumber = nThreadNumber;
		pNpcThread->m_pNpc[step]->Init();

		++step;

		if (step == NPC_NUM)
		{
			pNpcThread->m_sThreadNumber = nThreadNumber++;
			pNpcThread->pIOCP = &m_Iocport;
			pNpcThread->m_pThread = AfxBeginThread(NpcThreadProc, &pNpcThread->m_ThreadInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
			m_arNpcThread.push_back(pNpcThread);
			step = 0;
		}
	}
	if (step != 0)
	{
		pNpcThread->m_sThreadNumber = nThreadNumber++;
		pNpcThread->pIOCP = &m_Iocport;
		pNpcThread->m_pThread = AfxBeginThread(NpcThreadProc, &pNpcThread->m_ThreadInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		m_arNpcThread.push_back(pNpcThread);
	}

	// Event Npc Logic
/*
	pNpcThread = nullptr;
	pNpcThread = new CNpcThread;
	pNpcThread->pIOCP = &m_Iocport;
	pNpcThread->m_sThreadNumber = 1000;

	for(i = 0; i < NPC_NUM; i++) // 미리 최대 소환 NPC_NUM마리 메모리 할당
	{
		CNpc*	pNpc = new CNpc;
		pNpc->m_sNid = nSerial++;
		pNpc->m_NpcState = NPC_DEAD;
		pNpc->m_lEventNpc = 0;
		pNpcThread->m_pNpc[i] = pNpc;
		if( !m_arNpc.PutData( pNpc->m_sNid, pNpc) )	{
			TRACE("Npc PutData Fail - %d\n", pNpc->m_sNid);
			delete pNpc;
			pNpc = nullptr;
		}
	}

	pNpcThread->m_pThread = AfxBeginThread(NpcThreadProc, &(pNpcThread->m_ThreadInfo), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_arEventNpcThread.push_back( pNpcThread );
	*/

	m_pZoneEventThread = AfxBeginThread(ZoneEventThreadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

	//TRACE("m_TotalNPC = %d \n", m_TotalNPC);
	CString logstr;
	logstr.Format(_T("[Monster Init - %d]"), m_TotalNPC);
	m_StatusList.AddString(logstr);

	return TRUE;
}

//	NPC Thread 들을 작동시킨다.
void CServerDlg::ResumeAI()
{
	int i, j;

	for (i = 0; i < m_arNpcThread.size(); i++)
	{
		for (j = 0; j < NPC_NUM; j++)
			m_arNpcThread[i]->m_ThreadInfo.pNpc[j] = m_arNpcThread[i]->m_pNpc[j];

		m_arNpcThread[i]->m_ThreadInfo.pIOCP = &m_Iocport;

		::ResumeThread(m_arNpcThread[i]->m_pThread->m_hThread);
	}


	// Event Npc Logic
/*	m_arEventNpcThread[0]->m_ThreadInfo.hWndMsg = this->GetSafeHwnd();
	for(j = 0; j < NPC_NUM; j++)
	{
		m_arEventNpcThread[0]->m_ThreadInfo.pNpc[j] = nullptr;	// 초기 소환 몹이 당연히 없으므로 NULL로 작동을 안시킴
		m_arEventNpcThread[0]->m_ThreadInfo.m_byNpcUsed[j] = 0;
	}
	m_arEventNpcThread[0]->m_ThreadInfo.pIOCP = &m_Iocport;

	::ResumeThread(m_arEventNpcThread[0]->m_pThread->m_hThread);
	*/

	::ResumeThread(m_pZoneEventThread->m_hThread);
}

//	메모리 정리
BOOL CServerDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(CHECK_ALIVE);
	//KillTimer( REHP_TIME );

	g_bNpcExit = TRUE;

	if (m_UserLogFile.m_hFile != CFile::hFileNull)
		m_UserLogFile.Close();

	if (m_ItemLogFile.m_hFile != CFile::hFileNull)
		m_ItemLogFile.Close();

	for (size_t i = 0; i < m_arNpcThread.size(); i++)
		WaitForSingleObject(m_arNpcThread[i]->m_pThread->m_hThread, INFINITE);

	// Event Npc Logic
/*	for(i = 0; i < m_arEventNpcThread.size(); i++)
	{
		WaitForSingleObject(m_arEventNpcThread[i]->m_pThread->m_hThread, INFINITE);
	}	*/

	WaitForSingleObject(m_pZoneEventThread, INFINITE);

	// DB테이블 삭제 부분

	// Map(Zone) Array Delete...
	for (size_t i = 0; i < g_arZone.size(); i++)
		delete g_arZone[i];
	g_arZone.clear();

	// NpcTable Array Delete
	if (!m_arMonTable.IsEmpty())
		m_arMonTable.DeleteAllData();

	// NpcTable Array Delete
	if (!m_arNpcTable.IsEmpty())
		m_arNpcTable.DeleteAllData();

	// NpcThread Array Delete
	for (size_t i = 0; i < m_arNpcThread.size(); i++)
		delete m_arNpcThread[i];
	m_arNpcThread.clear();

	// Event Npc Logic
	// EventNpcThread Array Delete
/*	for(i = 0; i < m_arEventNpcThread.size(); i++)
		delete m_arEventNpcThread[i];
	m_arEventNpcThread.clear();		*/

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

	if (!m_MakeWeaponItemArray.IsEmpty())
		m_MakeWeaponItemArray.DeleteAllData();

	if (!m_MakeDefensiveItemArray.IsEmpty())
		m_MakeDefensiveItemArray.DeleteAllData();

	if (!m_MakeGradeItemArray.IsEmpty())
		m_MakeGradeItemArray.DeleteAllData();

	if (!m_MakeLareItemArray.IsEmpty())
		m_MakeLareItemArray.DeleteAllData();

	// MagicTable Array Delete
	if (!m_MagictableArray.IsEmpty())
		m_MagictableArray.DeleteAllData();

	if (!m_Magictype1Array.IsEmpty())
		m_Magictype1Array.DeleteAllData();

	if (!m_Magictype2Array.IsEmpty())
		m_Magictype2Array.DeleteAllData();

	if (!m_Magictype3Array.IsEmpty())
		m_Magictype3Array.DeleteAllData();

	if (!m_Magictype4Array.IsEmpty())
		m_Magictype4Array.DeleteAllData();

	// Npc Array Delete
	if (!m_arNpc.IsEmpty())
		m_arNpc.DeleteAllData();

	// User Array Delete
	for (int i = 0; i < MAX_USER; i++)
	{
		delete m_pUser[i];
		m_pUser[i] = nullptr;
	}

	// Party Array Delete 
	if (!m_arParty.IsEmpty())
		m_arParty.DeleteAllData();

	while (!m_ZoneNpcList.empty())
		m_ZoneNpcList.pop_front();

	DeleteCriticalSection(&g_User_critical);
	DeleteCriticalSection(&g_LogFileWrite);
	DeleteCriticalSection(&g_region_critical);

	s_pInstance = nullptr;

	return CDialog::DestroyWindow();
}

void CServerDlg::DeleteUserList(int uid)
{
	if (uid < 0
		|| uid >= MAX_USER)
	{
		TRACE("#### ServerDlg:DeleteUserList Uid Fail : uid=%d\n", uid);
		return;
	}

	EnterCriticalSection(&g_User_critical);

	CUser* pUser = nullptr;
	pUser = m_pUser[uid];
	if (!pUser)
	{
		LeaveCriticalSection(&g_User_critical);
		TRACE("#### ServerDlg:DeleteUserList UserPtr Fail : uid=%d\n", uid);
		return;
	}
	if (pUser->m_iUserId == uid)
	{
		TRACE("*** UserLogOut으로 포인터 반환 : uid=%d, %s ***\n", uid, pUser->m_strUserID);
		pUser->m_lUsed = 1;
		delete m_pUser[uid];
		m_pUser[uid] = nullptr;
	}
	else
	{
		TRACE("#### ServerDlg:DeleteUserList Not Uid Fail : uid=%d\n", uid);
	}

	LeaveCriticalSection(&g_User_critical);
}

BOOL CServerDlg::MapFileLoad()
{
	CFile file;
	CString szFullPath, errormsg, sZoneName;
	MAP* pMap = nullptr;
	m_sTotalMap = 0;

	CZoneInfoSet ZoneInfoSet;

	if (!ZoneInfoSet.Open())
	{
		AfxMessageBox(_T("ZoneInfoTable Open Fail!"));
		return FALSE;
	}

	if (ZoneInfoSet.IsBOF()
		|| ZoneInfoSet.IsEOF())
	{
		AfxMessageBox(_T("ZoneInfoTable Empty!"));
		return FALSE;
	}

	// Build the base MAP directory
	std::filesystem::path mapDir(GetProgPath().GetString());
	mapDir /= MAP_DIR;

	// Resolve it to strip the relative references to be nice.
	if (std::filesystem::exists(mapDir))
		mapDir = std::filesystem::canonical(mapDir);

	ZoneInfoSet.MoveFirst();

	while (!ZoneInfoSet.IsEOF())
	{
		sZoneName = ZoneInfoSet.m_strZoneName;

		std::filesystem::path mapPath
			= mapDir / sZoneName.GetString();

		szFullPath.Format(
			_T("%ls"),
			mapPath.c_str());

		if (!file.Open(szFullPath, CFile::modeRead))
		{
			errormsg.Format(_T("파일 Open 실패 - %s\n"), szFullPath);
			AfxMessageBox(errormsg);
			return FALSE;
		}

		pMap = new MAP;
		pMap->m_nServerNo = ZoneInfoSet.m_ServerNo;
		pMap->m_nZoneNumber = ZoneInfoSet.m_ZoneNo;
		strcpy(pMap->m_MapName, CT2A(sZoneName));

		if (!pMap->LoadMap(file.m_hFile))
		{
			errormsg.Format(_T("Map Load 실패 - %s\n"), szFullPath);
			AfxMessageBox(errormsg);
			delete pMap;
			return FALSE;
		}

		// dungeon work
		if (ZoneInfoSet.m_RoomEvent > 0)
		{
			if (!pMap->LoadRoomEvent(ZoneInfoSet.m_RoomEvent))
			{
				errormsg.Format(_T("Map Room Event Load 실패 - %s\n"), szFullPath);
				AfxMessageBox(errormsg);
				delete pMap;
				return FALSE;
			}

			pMap->m_byRoomEvent = 1;
		}

		g_arZone.push_back(pMap);

		ZoneInfoSet.MoveNext();

		file.Close();
		m_sTotalMap++;
	}

	return TRUE;
}

// sungyong 2002.05.23
// game server에 모든 npc정보를 전송..
void CServerDlg::AllNpcInfo()
{
	// server alive check
	CNpc* pNpc = nullptr;
	MAP* pMap = nullptr;
	int nZone = 0, i = 0;
	int size = m_arNpc.GetSize();

	int send_index = 0, zone_index = 0, packet_size = 0;
	int count = 0, send_count = 0, send_tot = 0;
	char send_buff[2048] = {};

	for (i = 0; i < m_sTotalMap; i++)
	{
		pMap = g_arZone[i];
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

		TRACE("****  allNpcInfo start = %d *****\n", nZone);

		for (int i = 0; i < size; i++)
		{
			pNpc = m_arNpc.GetData(i);
			if (pNpc == nullptr)
			{
				TRACE("##### allNpcInfo Fail = %d\n", i);
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
				//TRACE("AllNpcInfo - send_count=%d, count=%d, zone=%d\n", send_tot, count, nZone);
				memset(send_buff, 0, sizeof(send_buff));
				Sleep(50);
			}
		}

		//if( count != 0 )	TRACE("--> AllNpcInfo - send_count=%d, count=%d, zone=%d\n", send_tot, count, nZone);
		if (count != 0
			&& count < NPC_NUM)
		{
			send_count = 0;
			SetByte(send_buff, NPC_INFO_ALL, send_count);
			SetByte(send_buff, (BYTE) count, send_count);
			Send(send_buff, send_index, nZone);
			send_tot++;
			//TRACE("AllNpcInfo - send_count=%d, count=%d, zone=%d\n", send_tot, count, nZone);
			Sleep(50);
		}

		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
		SetByte(send_buff, AG_SERVER_INFO, send_index);
		SetByte(send_buff, SERVER_INFO_END, send_index);
		SetByte(send_buff, nZone, send_index);
		SetShort(send_buff, (short) m_TotalNPC, send_index);
		packet_size = Send(send_buff, send_index, nZone);

		TRACE("****  allNpcInfo end = %d *****\n", nZone);
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
			TRACE("### GetUserPtr :: User Array Overflow [%d] ###\n", nid);

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
				logstr.Format(_T("*** All Socket Closed ***  %d-%d-%d, %d:%d]\r\n"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute());
				//LogFileWrite( logstr );
			}
			count++;
		}
		//TRACE("size = %d, socket_num = %d, i=%d \n", size, pSocket->m_sSocketID, i);
	}

	if (count <= 0)
		DeleteAllUserList(9999);

	RegionCheck();
}

void CServerDlg::DeleteAllUserList(int zone)
{
	if (zone < 0)
		return;

	CString logstr;

	// 모든 소켓이 끊어진 상태...
	if (zone == 9999
		&& m_bFirstServerFlag)
	{
		TRACE("*** DeleteAllUserList - Start *** \n");

		for (MAP* pMap : g_arZone)
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

		int size = g_arZone.size();

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
		if (!m_arParty.IsEmpty())
			m_arParty.DeleteAllData();

		m_bFirstServerFlag = FALSE;
		TRACE("*** DeleteAllUserList - End *** \n");

		logstr.Format(_T("[ DELETE All User List ]"));
		m_StatusList.AddString(logstr);
	}
	else
	{
		if (zone != 9999)
		{
			logstr.Format(_T("[GameServer DisConnect - zone = %d]"), zone);
			m_StatusList.AddString(logstr);
		}
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
		TRACE("#### SendCompressData Fail --> count=%d, index=%d\n", m_CompCount, m_iCompIndex);
		return;
	}

	m_CompMng.FlushAddData();
	m_CompMng.AddData(m_CompBuf, m_iCompIndex);
	m_CompMng.PreCompressWork();
	m_CompMng.Compress();

	int comp_data_len = m_CompMng.GetCompressedDataCount();
	int org_data_len = m_CompMng.GetUnCompressDataLength();
	DWORD crc_value = m_CompMng.GetCrcValue();

	int send_index = 0, packet_size = 0;
	char send_buff[2048] = {};
	SetByte(send_buff, AG_COMPRESSED_DATA, send_index);
	SetShort(send_buff, (short) comp_data_len, send_index);
	SetShort(send_buff, (short) org_data_len, send_index);
	SetDWORD(send_buff, crc_value, send_index);
	SetShort(send_buff, (short) m_CompCount, send_index);

	char* packet = m_CompMng.GetExtractedBufferPtr();
	SetString(send_buff, packet, comp_data_len, send_index);

	if (packet == nullptr)
	{
		m_CompCount = 0;
		m_iCompIndex = 0;
		m_CompMng.FlushAddData();
		TRACE("#### SendCompressData Fail packet==null\n");
		return;
	}

	packet_size = Send(send_buff, send_index, nZone);

	m_CompCount = 0;
	m_iCompIndex = 0;
	m_CompMng.FlushAddData();
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
	FILE* stream = fopen("c:\\aiserver.txt", "w");

	fprintf(stream, "*****   Check ... List  *****\n");

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

		fprintf(stream, " size = %d, socket_num = %d \n", size, pSocket->m_sSocketID);
	}

/*
	int size = m_arNpc.GetSize();
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
		pNpc = m_arNpc.GetData(i);
		if(pNpc == nullptr)
		{
			TRACE("##### allNpcInfo Fail = %d\n", i);
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
		pMap = g_arZone[k];
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

	fclose(stream);
}

CUser* CServerDlg::GetActiveUserPtr(int index)
{
	CUser* pUser = nullptr;

/*	if(index < 0 || index > MAX_USER)	{
		TRACE("### Fail :: User Array Overflow[%d] ###\n", index );
		return nullptr;
	}

	EnterCriticalSection( &g_User_critical );

	if ( m_ppUserActive[index] ) {
		LeaveCriticalSection( &g_User_critical );
		TRACE("### Fail : ActiveUser Array Invalid[%d] ###\n", index );
		return nullptr;
	}
	else {
		pUser = (CUser *)m_ppUserInActive[index];
		if( !pUser ) {
			LeaveCriticalSection( &g_User_critical );
			TRACE("### Fail : InActiveUser Array Invalid[%d] ###\n", index );
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
	for (const auto& [_, pNpc] : m_arNpc)
	{
		if (pNpc != nullptr
			&& strcmp(pNpc->m_strName, pNpcName) == 0)
			return pNpc;
	}

	return nullptr;
}


//	추가할 소환몹의 메모리를 참조하기위해 플래그가 0인 상태것만 넘긴다.
CNpc* CServerDlg::GetEventNpcPtr()
{
	for (auto& [_, pNpc] : m_arNpc)
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

int  CServerDlg::MonsterSummon(const char* pNpcName, int zone, float fx, float fz)
{
	if (zone < 0
		|| zone > (g_arZone.size() + 1))
	{
		TRACE("#### 소환 실패 : %s, zoneindex=%d #####\n", pNpcName, zone);
		return -1;
	}

	CNpc* pNpc = GetNpcPtr(pNpcName);
	if (pNpc == nullptr)
	{
		TRACE("소환할  몬스터의 이름(%s)이 잘못되었습니다.\n", pNpcName);
		return  -1;
	}

	BOOL bFlag = FALSE;
	bFlag = SetSummonNpcData(pNpc, zone, fx, fz);

	return 1;
}

//	소환할 몹의 데이타값을 셋팅한다.
BOOL CServerDlg::SetSummonNpcData(CNpc* pNpc, int zone, float fx, float fz)
{
	int  iCount = 0;
	CNpc* pEventNpc = GetEventNpcPtr();

	if (pEventNpc == nullptr)
	{
		TRACE("소환할수 있는 몹은 최대 20마리입니다.\n");
		return FALSE;
	}

	CString strMsg;

	pEventNpc->m_sSid = pNpc->m_sSid;						// MONSTER(NPC) Serial ID
	pEventNpc->m_byMoveType = 1;
	pEventNpc->m_byInitMoveType = 1;
	pEventNpc->m_byBattlePos = 0;
	strcpy(pEventNpc->m_strName, pNpc->m_strName);			// MONSTER(NPC) Name
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
	pEventNpc->m_sAI = pNpc->m_sAI;							// 인공지능 인덱스
	pEventNpc->m_tNpcType = pNpc->m_tNpcType;				// NPC Type
	pEventNpc->m_byFamilyType = pNpc->m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	pEventNpc->m_iMoney = pNpc->m_iMoney;					// 떨어지는 돈
	pEventNpc->m_iItem = pNpc->m_iItem;						// 떨어지는 아이템
	pEventNpc->m_tNpcLongType = pNpc->m_tNpcLongType;
	pEventNpc->m_byWhatAttackType = pNpc->m_byWhatAttackType;

	//////// MONSTER POS ////////////////////////////////////////
	pEventNpc->m_sCurZone = zone;
	pEventNpc->m_fCurX = fx;
	pEventNpc->m_fCurY = 0;
	pEventNpc->m_fCurZ = fz;
	pEventNpc->m_nInitMinX = pNpc->m_nInitMinX;
	pEventNpc->m_nInitMinY = pNpc->m_nInitMinY;
	pEventNpc->m_nInitMaxX = pNpc->m_nInitMaxX;
	pEventNpc->m_nInitMaxY = pNpc->m_nInitMaxY;
	pEventNpc->m_sRegenTime = pNpc->m_sRegenTime;			// 초(DB)단위-> 밀리세컨드로
	pEventNpc->m_tItemPer = pNpc->m_tItemPer;				// NPC Type
	pEventNpc->m_tDnPer = pNpc->m_tDnPer;					// NPC Type

	pEventNpc->m_ZoneIndex = -1;

	pEventNpc->m_NpcState = NPC_DEAD;						// 상태는 죽은것으로 해야 한다.. 
	pEventNpc->m_bFirstLive = 1;							// 처음 살아난 경우로 해줘야 한다..

	for (int i = 0; i < g_arZone.size(); i++)
	{
		if (g_arZone[i]->m_nZoneNumber == zone)
		{
			pEventNpc->m_ZoneIndex = i;
			break;
		}
	}

	if (pEventNpc->m_ZoneIndex == -1)
	{
		TRACE("Invaild zone Index!!\n");
		return FALSE;
	}

	pEventNpc->Init();

	BOOL bSuccess = FALSE;

	int test = 0;

	for (int i = 0; i < NPC_NUM; i++)
	{
		test = m_arEventNpcThread[0]->m_ThreadInfo.m_byNpcUsed[i];
		TRACE("setsummon == %d, used=%d\n", i, test);
		if (m_arEventNpcThread[0]->m_ThreadInfo.m_byNpcUsed[i] == 0)
		{
			m_arEventNpcThread[0]->m_ThreadInfo.m_byNpcUsed[i] = 1;
			bSuccess = TRUE;
			m_arEventNpcThread[0]->m_ThreadInfo.pNpc[i] = pEventNpc;
			break;
		}
	}

	if (!bSuccess)
	{
		pEventNpc->m_lEventNpc = 0;
		TRACE("### 소환에 실패했습니다. ###\n");
		return FALSE;
	}

	TRACE("*** %d, %s 를 소환하였습니다. state = %d ***\n", pEventNpc->m_sNid + NPC_BAND, pEventNpc->m_strName, pEventNpc->m_NpcState);

	return TRUE;
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

	//TRACE("$$$ random test == 1=%d, 2=%d, 3=%d,, %d,%s $$$\n", count_1, count_2, count_3, __FILE__, __LINE__);

}

BOOL CServerDlg::GetMagicType1Data()
{
	CMagicType1Set MagicType1Set;

	if (!MagicType1Set.Open())
	{
		AfxMessageBox(_T("MagicType1 Open Fail!"));
		return FALSE;
	}

	if (MagicType1Set.IsBOF()
		|| MagicType1Set.IsEOF())
	{
		AfxMessageBox(_T("MagicType1 Empty!"));
		return FALSE;
	}

	MagicType1Set.MoveFirst();

	while (!MagicType1Set.IsEOF())
	{
		_MAGIC_TYPE1* pType1Magic = new _MAGIC_TYPE1;

		pType1Magic->iNum = MagicType1Set.m_iNum;
		pType1Magic->bHitType = MagicType1Set.m_Type;
		pType1Magic->bDelay = MagicType1Set.m_Delay;
		pType1Magic->bComboCount = MagicType1Set.m_ComboCount;
		pType1Magic->bComboType = MagicType1Set.m_ComboType;
		pType1Magic->sComboDamage = MagicType1Set.m_ComboDamage;
		pType1Magic->sHit = MagicType1Set.m_Hit;
		pType1Magic->sHitRate = MagicType1Set.m_HitRate;
		pType1Magic->sRange = MagicType1Set.m_Range;

		if (!m_Magictype1Array.PutData(pType1Magic->iNum, pType1Magic))
		{
			TRACE("MagicType1 PutData Fail - %d\n", pType1Magic->iNum);
			delete pType1Magic;
			pType1Magic = nullptr;
		}

		MagicType1Set.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMagicType2Data()
{
	CMagicType2Set MagicType2Set;

	if (!MagicType2Set.Open())
	{
		AfxMessageBox(_T("MagicType1 Open Fail!"));
		return FALSE;
	}

	if (MagicType2Set.IsBOF()
		|| MagicType2Set.IsEOF())
	{
		AfxMessageBox(_T("MagicType1 Empty!"));
		return FALSE;
	}

	MagicType2Set.MoveFirst();

	while (!MagicType2Set.IsEOF())
	{
		_MAGIC_TYPE2* pType2Magic = new _MAGIC_TYPE2;

		pType2Magic->iNum = MagicType2Set.m_iNum;
		pType2Magic->bHitType = MagicType2Set.m_HitType;
		pType2Magic->sHitRate = MagicType2Set.m_HitRate;
		pType2Magic->sAddDamage = MagicType2Set.m_AddDamage;
		pType2Magic->sAddRange = MagicType2Set.m_AddRange;
		pType2Magic->bNeedArrow = MagicType2Set.m_NeedArrow;

		if (!m_Magictype2Array.PutData(pType2Magic->iNum, pType2Magic))
		{
			TRACE("MagicType2 PutData Fail - %d\n", pType2Magic->iNum);
			delete pType2Magic;
			pType2Magic = nullptr;
		}

		MagicType2Set.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMagicType3Data()
{
	CMagicType3Set MagicType3Set;

	if (!MagicType3Set.Open())
	{
		AfxMessageBox(_T("MagicType3 Open Fail!"));
		return FALSE;
	}

	if (MagicType3Set.IsBOF()
		|| MagicType3Set.IsEOF())
	{
		AfxMessageBox(_T("MagicType3 Empty!"));
		return FALSE;
	}

	MagicType3Set.MoveFirst();

	while (!MagicType3Set.IsEOF())
	{
		_MAGIC_TYPE3* pType3Magic = new _MAGIC_TYPE3;

		pType3Magic->iNum = MagicType3Set.m_iNum;
		pType3Magic->bAttribute = MagicType3Set.m_Attribute;
		pType3Magic->bDirectType = MagicType3Set.m_DirectType;
		//pType3Magic->bDistance = MagicType3Set.m_Distance;
		pType3Magic->bRadius = MagicType3Set.m_Radius;
		pType3Magic->sAngle = MagicType3Set.m_Angle;
		pType3Magic->sDuration = MagicType3Set.m_Duration;
		pType3Magic->sEndDamage = MagicType3Set.m_EndDamage;
		pType3Magic->sFirstDamage = MagicType3Set.m_FirstDamage;
		pType3Magic->sTimeDamage = MagicType3Set.m_TimeDamage;

		if (!m_Magictype3Array.PutData(pType3Magic->iNum, pType3Magic))
		{
			TRACE("MagicType3 PutData Fail - %d\n", pType3Magic->iNum);
			delete pType3Magic;
			pType3Magic = nullptr;
		}

		MagicType3Set.MoveNext();
	}

	return TRUE;
}

BOOL CServerDlg::GetMagicType4Data()
{
	CMagicType4Set MagicType4Set;

	if (!MagicType4Set.Open())
	{
		AfxMessageBox(_T("MagicType4 Open Fail!"));
		return FALSE;
	}

	if (MagicType4Set.IsBOF()
		|| MagicType4Set.IsEOF())
	{
		AfxMessageBox(_T("MagicType4 Empty!"));
		return FALSE;
	}

	MagicType4Set.MoveFirst();

	while (!MagicType4Set.IsEOF())
	{
		_MAGIC_TYPE4* pType4Magic = new _MAGIC_TYPE4;

		pType4Magic->iNum = MagicType4Set.m_iNum;
		pType4Magic->bBuffType = MagicType4Set.m_BuffType;
		pType4Magic->bRadius = MagicType4Set.m_Radius;
		pType4Magic->sDuration = MagicType4Set.m_Duration;
		pType4Magic->bAttackSpeed = MagicType4Set.m_AttackSpeed;
		pType4Magic->bSpeed = MagicType4Set.m_Speed;
		pType4Magic->sAC = MagicType4Set.m_AC;
		pType4Magic->bAttack = MagicType4Set.m_Attack;
		pType4Magic->sMaxHP = MagicType4Set.m_MaxHP;
		pType4Magic->bHitRate = MagicType4Set.m_HitRate;
		pType4Magic->sAvoidRate = MagicType4Set.m_AvoidRate;
		pType4Magic->bStr = MagicType4Set.m_Str;
		pType4Magic->bSta = MagicType4Set.m_Sta;
		pType4Magic->bDex = MagicType4Set.m_Dex;
		pType4Magic->bIntel = MagicType4Set.m_Intel;
		pType4Magic->bCha = MagicType4Set.m_Cha;
		pType4Magic->bFireR = MagicType4Set.m_FireR;
		pType4Magic->bColdR = MagicType4Set.m_ColdR;
		pType4Magic->bLightningR = MagicType4Set.m_LightningR;
		pType4Magic->bMagicR = MagicType4Set.m_MagicR;
		pType4Magic->bDiseaseR = MagicType4Set.m_DiseaseR;
		pType4Magic->bPoisonR = MagicType4Set.m_PoisonR;

		if (!m_Magictype4Array.PutData(pType4Magic->iNum, pType4Magic))
		{
			TRACE("MagicType4 PutData Fail - %d\n", pType4Magic->iNum);
			delete pType4Magic;
			pType4Magic = nullptr;
		}

		MagicType4Set.MoveNext();
	}

	return TRUE;
}

void CServerDlg::RegionCheck()
{
	for (MAP* pMap : g_arZone)
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

BOOL CServerDlg::AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number)
{
	int i = 0, j = 0, objectid = 0;
	CNpcTable* pNpcTable = nullptr;
	BOOL bFindNpcTable = FALSE;
	int offset = 0;
	int nServerNum = 0;
	nServerNum = GetServerNumber(zone_number);
	//if(m_byZone != zone_number)	 return FALSE;
	//if(m_byZone != UNIFY_ZONE)	{
	//	if(m_byZone != nServerNum)	 return FALSE;
	//}

	//if( zone_number > 201 )	return FALSE;	// test
	pNpcTable = m_arNpcTable.GetData(pEvent->sIndex);
	if (pNpcTable == nullptr)
	{
		bFindNpcTable = FALSE;
		TRACE("#### AddObjectEventNpc Fail : [sid = %d], zone=%d #####\n", pEvent->sIndex, zone_number);
		return FALSE;
	}

	bFindNpcTable = TRUE;


	CNpc* pNpc = new CNpc;

	pNpc->m_sNid = m_sMapEventNpc++;				// 서버 내에서의 고유 번호
	pNpc->m_sSid = (short) pEvent->sIndex;			// MONSTER(NPC) Serial ID

	pNpc->m_byMoveType = 100;
	pNpc->m_byInitMoveType = 100;
	bFindNpcTable = FALSE;

	pNpc->m_byMoveType = 0;
	pNpc->m_byInitMoveType = 0;

	pNpc->m_byBattlePos = 0;

	strcpy(pNpc->m_strName, pNpcTable->m_strName);			// MONSTER(NPC) Name
	pNpc->m_sPid = pNpcTable->m_sPid;						// MONSTER(NPC) Picture ID
	pNpc->m_sSize = pNpcTable->m_sSize;						// 캐릭터의 비율(100 퍼센트 기준)
	pNpc->m_iWeapon_1 = pNpcTable->m_iWeapon_1;				// 착용무기
	pNpc->m_iWeapon_2 = pNpcTable->m_iWeapon_2;				// 착용무기
	pNpc->m_byGroup = pNpcTable->m_byGroup;					// 소속집단
	pNpc->m_byActType = pNpcTable->m_byActType;				// 행동패턴
	pNpc->m_byRank = pNpcTable->m_byRank;					// 작위
	pNpc->m_byTitle = pNpcTable->m_byTitle;					// 지위
	pNpc->m_iSellingGroup = pNpcTable->m_iSellingGroup;
	pNpc->m_sLevel = pNpcTable->m_sLevel;					// level
	pNpc->m_iExp = pNpcTable->m_iExp;						// 경험치
	pNpc->m_iLoyalty = pNpcTable->m_iLoyalty;				// loyalty
	pNpc->m_iHP = pNpcTable->m_iMaxHP;						// 최대 HP
	pNpc->m_iMaxHP = pNpcTable->m_iMaxHP;					// 현재 HP
	pNpc->m_sMP = pNpcTable->m_sMaxMP;						// 최대 MP
	pNpc->m_sMaxMP = pNpcTable->m_sMaxMP;					// 현재 MP
	pNpc->m_sAttack = pNpcTable->m_sAttack;					// 공격값
	pNpc->m_sDefense = pNpcTable->m_sDefense;				// 방어값
	pNpc->m_sHitRate = pNpcTable->m_sHitRate;				// 타격성공률
	pNpc->m_sEvadeRate = pNpcTable->m_sEvadeRate;			// 회피성공률
	pNpc->m_sDamage = pNpcTable->m_sDamage;					// 기본 데미지
	pNpc->m_sAttackDelay = pNpcTable->m_sAttackDelay;		// 공격딜레이
	pNpc->m_sSpeed = pNpcTable->m_sSpeed;					// 이동속도
	pNpc->m_fSpeed_1 = (float) pNpcTable->m_bySpeed_1;		// 기본 이동 타입
	pNpc->m_fSpeed_2 = (float) pNpcTable->m_bySpeed_2;		// 뛰는 이동 타입..
	pNpc->m_fOldSpeed_1 = (float) pNpcTable->m_bySpeed_1;	// 기본 이동 타입
	pNpc->m_fOldSpeed_2 = (float) pNpcTable->m_bySpeed_2;	// 뛰는 이동 타입..
	pNpc->m_fSecForMetor = 4.0f;							// 초당 갈 수 있는 거리..
	pNpc->m_sStandTime = pNpcTable->m_sStandTime;			// 서있는 시간
	pNpc->m_iMagic1 = pNpcTable->m_iMagic1;					// 사용마법 1
	pNpc->m_iMagic2 = pNpcTable->m_iMagic2;					// 사용마법 2
	pNpc->m_iMagic3 = pNpcTable->m_iMagic3;					// 사용마법 3
	pNpc->m_sFireR = pNpcTable->m_sFireR;					// 화염 저항력
	pNpc->m_sColdR = pNpcTable->m_sColdR;					// 냉기 저항력
	pNpc->m_sLightningR = pNpcTable->m_sLightningR;			// 전기 저항력
	pNpc->m_sMagicR = pNpcTable->m_sMagicR;					// 마법 저항력
	pNpc->m_sDiseaseR = pNpcTable->m_sDiseaseR;				// 저주 저항력
	pNpc->m_sPoisonR = pNpcTable->m_sPoisonR;				// 독 저항력
	pNpc->m_sLightR = pNpcTable->m_sLightR;					// 빛 저항력
	pNpc->m_fBulk = (float) (((double) pNpcTable->m_sBulk / 100) * ((double) pNpcTable->m_sSize / 100));
	pNpc->m_bySearchRange = pNpcTable->m_bySearchRange;		// 적 탐지 범위
	pNpc->m_byAttackRange = pNpcTable->m_byAttackRange;		// 사정거리
	pNpc->m_byTracingRange = pNpcTable->m_byTracingRange;	// 추격거리
	pNpc->m_sAI = pNpcTable->m_sAI;							// 인공지능 인덱스
	pNpc->m_tNpcType = pNpcTable->m_tNpcType;				// NPC Type
	pNpc->m_byFamilyType = pNpcTable->m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	pNpc->m_iMoney = pNpcTable->m_iMoney;					// 떨어지는 돈
	pNpc->m_iItem = pNpcTable->m_iItem;						// 떨어지는 아이템
	pNpc->m_tNpcLongType = pNpcTable->m_byDirectAttack;
	pNpc->m_byWhatAttackType = pNpcTable->m_byDirectAttack;

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
	pNpc->m_tItemPer = pNpcTable->m_tItemPer;	// NPC Type
	pNpc->m_tDnPer = pNpcTable->m_tDnPer;	// NPC Type

	pNpc->m_ZoneIndex = -1;
	pNpc->m_byObjectType = SPECIAL_OBJECT;
	pNpc->m_bFirstLive = 1;		// 처음 살아난 경우로 해줘야 한다..
	//pNpc->m_ZoneIndex = GetZoneIndex(pNpc->m_sCurZone);
/*
	if(pNpc->m_ZoneIndex == -1)	{
		AfxMessageBox("Invaild zone Index!!");
		return FALSE;
	}	*/

	//pNpc->Init();
	if (!m_arNpc.PutData(pNpc->m_sNid, pNpc))
	{
		TRACE("Npc PutData Fail - %d\n", pNpc->m_sNid);
		delete pNpc;
		pNpc = nullptr;
	}

	m_TotalNPC = m_sMapEventNpc;

	return TRUE;
}

int CServerDlg::GetZoneIndex(int zonenumber)
{
	int t_count = g_arZone.size();
	for (int i = 0; i < t_count; i++)
	{
		if (g_arZone[i]
			&& (zonenumber == g_arZone[i]->m_nZoneNumber))
			return i;
	}

	return -1;
}

int CServerDlg::GetServerNumber(int zonenumber)
{
	int t_count = g_arZone.size();
	for (int i = 0; i < t_count; i++)
	{
		if (g_arZone[i]
			&& (zonenumber == g_arZone[i]->m_nZoneNumber))
			return g_arZone[i]->m_nServerNo;
	}

	return -1;
}

void CServerDlg::ClostSocket(int zonenumber)
{
	CGameSocket* pSocket = nullptr;

	for (int i = 0; i < MAX_SOCKET; i++)
	{
		pSocket = (CGameSocket*) m_Iocport.m_SockArray[i];
		if (pSocket == nullptr)
			continue;

		if (pSocket->m_sSocketID == zonenumber)
		{
			//TRACE("size = %d, socket_num = %d, i=%d \n", size, pSocket->m_sSocketID, i);
			pSocket->CloseProcess();
			m_Iocport.RidIOCPSocket(pSocket->GetSocketID(), pSocket);
		}
	}
}

void CServerDlg::GetServerInfoIni()
{
	std::filesystem::path iniPath(GetProgPath().GetString());
	iniPath /= L"server.ini";

	CIni inifile;
	inifile.Load(iniPath);
	m_byZone = inifile.GetInt(_T("SERVER"), _T("ZONE"), 1);

	inifile.GetString(_T("ODBC"), _T("GAME_DSN"), _T("KN_Online"), m_strGameDSN, sizeof(m_strGameDSN));
	inifile.GetString(_T("ODBC"), _T("GAME_UID"), _T("knight"), m_strGameUID, sizeof(m_strGameUID));
	inifile.GetString(_T("ODBC"), _T("GAME_PWD"), _T("knight"), m_strGamePWD, sizeof(m_strGamePWD));
}

void CServerDlg::SendSystemMsg(char* pMsg, int zone, int type, int who)
{
	int send_index = 0;
	char buff[256] = {};
	short sLength = static_cast<short>(strlen(pMsg));

	SetByte(buff, AG_SYSTEM_MSG, send_index);
	SetByte(buff, type, send_index);				// 채팅형식
	SetShort(buff, who, send_index);				// 누구에게
	SetShort(buff, sLength, send_index);
	SetString(buff, pMsg, sLength, send_index);

	Send(buff, send_index, zone);
}

void CServerDlg::ResetBattleZone()
{
	TRACE("ServerDlg - ResetBattleZone() : start \n");

	for (MAP* pMap : g_arZone)
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

	TRACE("ServerDlg - ResetBattleZone() : end \n");
}

CString CServerDlg::GetGameDBConnectionString()
{
	CString strConnection;
	strConnection.Format(
		_T("ODBC;DSN=%s;UID=%s;PWD=%s"),
		m_strGameDSN,
		m_strGameUID,
		m_strGamePWD);
	return strConnection;
}
