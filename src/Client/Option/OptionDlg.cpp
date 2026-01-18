// OptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Option.h"
#include "OptionDlg.h"
#include <algorithm>
#include <set>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum
	{
		IDD = IDD_ABOUTBOX
	};
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
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
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionDlg dialog

COptionDlg::COptionDlg(CWnd* pParent /*=nullptr*/) : CDialog(COptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon  = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Option = {}; // 옵션 초기화
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionDlg)
	DDX_Control(pDX, IDC_SLD_EFFECT_COUNT, m_SldEffectCount);
	DDX_Control(pDX, IDC_SLD_SOUND_DISTANCE, m_SldEffectSoundDist);
	DDX_Control(pDX, IDC_CB_COLORDEPTH, m_CB_ColorDepth);
	DDX_Control(pDX, IDC_CB_RESOLUTION, m_CB_Resolution);
	DDX_Control(pDX, IDC_SLD_VIEWING_DISTANCE, m_SldViewDist);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
//{{AFX_MSG_MAP(COptionDlg)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_B_APPLY_AND_EXECUTE, OnBApplyAndExecute)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_B_VERSION, OnBVersion)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

struct Resolution
{
	uint32_t Width;
	uint32_t Height;
};

static std::vector<Resolution> s_supportedResolutions;

static Resolution DefaultResolutions[] = {
	{ 1024, 768 }, { 1152, 864 }, { 1280, 768 },  //
	{ 1280, 800 }, { 1280, 960 }, { 1280, 1024 }, //
	{ 1360, 768 }, { 1366, 768 }, { 1600, 1200 }  //
};

// The game supports at minimum, a resolution of 1024x768.
// The UIs will not fit on anything smaller than this.
constexpr Resolution MIN_RESOLUTION = { 1024, 768 };

/*
 * LoadSupportedResolutions will load a vector of valid resolutions for the primary monitor.
 * If no resolutions are detectable for the primary monitor, falls back to the default, hardcoded list.
 */
void COptionDlg::LoadSupportedResolutions()
{
	// We point to the device name instead of copying it / using it directly
	// that way if we don't find a primary monitor, we pass nullptr as the first param into EnumDisplaySettings.
	TCHAR* primaryDeviceName = nullptr;
	int deviceNumber         = 0;

	// Get the primary monitor
	DISPLAY_DEVICE device {};
	device.cb = sizeof(DISPLAY_DEVICE);

	while (EnumDisplayDevices(nullptr, deviceNumber, &device, EDD_GET_DEVICE_INTERFACE_NAME))
	{
		if (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
		{
			// Primary monitor found; get a pointer to the name and break out of the loop
			primaryDeviceName = &device.DeviceName[0];
			break;
		}

		deviceNumber++;
	}

	// Order such that higher resolutions appear at the top of the drop down list.
	// With modern monitors, this list can get pretty long.
	// If we're going to send a user scrolling for a resolution, it should be one that most
	// users aren't looking for.
	auto cmp = [](const Resolution& a, const Resolution& b) -> bool
	{ return a.Width > b.Width || (a.Width == b.Width && a.Height > b.Height); };

	// The same resolution can be listed many times on a monitor depending on available refresh rates.
	// We should limit it to unique resolutions only.
	std::set<Resolution, decltype(cmp)> loadedResolutions(cmp);

	// Discover resolution settings
	DEVMODE devmode {};
	devmode.dmSize = sizeof(DEVMODE);
	for (int iModeNum = 0; EnumDisplaySettings(primaryDeviceName, iModeNum, &devmode); iModeNum++)
	{
		// Only support 32-bit resolutions.
		// Officially the game supports 16-bit, but we only care about the resolutions themselves here.
		// We also just don't really bother to go out of our way to support that anymore anyway.
		if (devmode.dmBitsPerPel != 32)
			continue;

		Resolution resolution = { devmode.dmPelsWidth, devmode.dmPelsHeight };

		// Filter out resolutions with dimensions smaller than our minimum (1024x768)
		if (resolution.Width < MIN_RESOLUTION.Width || resolution.Height < MIN_RESOLUTION.Height)
			continue;

		auto itr = loadedResolutions.insert(resolution);
		if (!itr.second)
			continue;

		s_supportedResolutions.push_back(std::move(resolution));
	}

	// We failed to dynamically pull available resolutions, fall back to the hardcoded list
	if (s_supportedResolutions.empty())
	{
		s_supportedResolutions.insert(s_supportedResolutions.begin(),
			std::begin(DefaultResolutions), std::end(DefaultResolutions));
	}

	// Sort the vector such that higher resolutions appear at the top of the drop down list.
	std::sort(s_supportedResolutions.begin(), s_supportedResolutions.end(), cmp);
}

/////////////////////////////////////////////////////////////////////////////
// COptionDlg message handlers

BOOL COptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		CString strAboutMenu;
		(void) strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);  // Set big icon
	SetIcon(m_hIcon, FALSE); // Set small icon

	// 각종 컨트롤 초기화..
	m_SldEffectCount.SetRange(1000, 2000);
	m_SldViewDist.SetRange(256, 512);
	m_SldEffectSoundDist.SetRange(20, 48);

	LoadSupportedResolutions();

	int iAdd = 0;

	CString szResolution;
	for (const auto& resolution : s_supportedResolutions)
	{
		szResolution.Format(_T("%u X %u"), resolution.Width, resolution.Height);
		iAdd = m_CB_Resolution.AddString(szResolution);

		m_CB_Resolution.SetItemData(iAdd, MAKELPARAM(resolution.Height, resolution.Width));
	}

	iAdd = m_CB_ColorDepth.AddString(_T("16 Bit"));
	m_CB_ColorDepth.SetItemData(iAdd, 16);

	iAdd = m_CB_ColorDepth.AddString(_T("32 Bit"));
	m_CB_ColorDepth.SetItemData(iAdd, 32);

	TCHAR szBuff[_MAX_PATH] {};
	GetCurrentDirectory(_countof(szBuff), szBuff);

	m_szInstalledPath       = szBuff;

	// Version 표시
	CString szServerIniPath = m_szInstalledPath + _T("\\Server.Ini");
	DWORD dwVersion         = GetPrivateProfileInt(_T("Version"), _T("Files"), 0, szServerIniPath);
	SetDlgItemInt(IDC_E_VERSION, dwVersion);

	// 세팅을 읽어온다..
	SettingLoad(m_szInstalledPath + _T("\\Option.ini"));
	SettingUpdate();

	return TRUE; // return TRUE  unless you set the focus to a control
}

void COptionDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void COptionDlg::OnPaint()
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
HCURSOR COptionDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void COptionDlg::OnOK()
{
	SettingSave(m_szInstalledPath + _T("\\Option.ini"));

	CDialog::OnOK();
}

void COptionDlg::OnBApplyAndExecute()
{
	CString szExeFN  = m_szInstalledPath + _T("\\"); // 실행 파일 이름 만들고..
	szExeFN         += _T("Launcher.exe");

	ShellExecute(
		nullptr, _T("open"), szExeFN, _T(""), m_szInstalledPath, SW_SHOWNORMAL); // 게임 실행..

	OnOK();
}

void COptionDlg::SettingSave(CString szIniFile)
{
	if (szIniFile.GetLength() <= 0)
		return;

	CString szBuff;

	if (IsDlgButtonChecked(IDC_R_TEX_CHR_HIGH))
		m_Option.iTexLOD_Chr = 0;
	else if (IsDlgButtonChecked(IDC_R_TEX_CHR_LOW))
		m_Option.iTexLOD_Chr = 1;
	else
		m_Option.iTexLOD_Chr = 1;

	if (IsDlgButtonChecked(IDC_R_TEX_SHAPE_HIGH))
		m_Option.iTexLOD_Shape = 0;
	else if (IsDlgButtonChecked(IDC_R_TEX_SHAPE_LOW))
		m_Option.iTexLOD_Shape = 1;
	else
		m_Option.iTexLOD_Shape = 0;

	if (IsDlgButtonChecked(IDC_R_TEX_TERRAIN_HIGH))
		m_Option.iTexLOD_Terrain = 0;
	else if (IsDlgButtonChecked(IDC_R_TEX_TERRAIN_LOW))
		m_Option.iTexLOD_Terrain = 1;
	else
		m_Option.iTexLOD_Terrain = 1;

	if (IsDlgButtonChecked(IDC_C_SHADOW))
		m_Option.iUseShadow = 1;
	else
		m_Option.iUseShadow = 0;

	if (IsDlgButtonChecked(IDC_C_WINDOW_MODE))
		m_Option.bWindowMode = true;
	else
		m_Option.bWindowMode = false;

	if (IsDlgButtonChecked(IDC_C_SHOW_WEAPON_EFFECT))
		m_Option.bEffectVisible = true;
	else
		m_Option.bEffectVisible = false;

	int iSel             = m_CB_Resolution.GetCurSel();

	m_Option.iViewWidth  = 1024;
	m_Option.iViewHeight = 768;

	if (iSel >= 0 && iSel < (int) s_supportedResolutions.size())
	{
		const auto& resolution = s_supportedResolutions[iSel];
		m_Option.iViewWidth    = resolution.Width;
		m_Option.iViewHeight   = resolution.Height;
	}

	iSel = m_CB_ColorDepth.GetCurSel();
	if (CB_ERR != iSel)
	{
		m_Option.iViewColorDepth = static_cast<int>(m_CB_ColorDepth.GetItemData(iSel));
		if (m_Option.iViewColorDepth != 16 && m_Option.iViewColorDepth != 32)
			m_Option.iViewColorDepth = 16;
	}
	else
	{
		m_Option.iViewColorDepth = 16;
	}

	m_Option.iEffectCount = m_SldEffectCount.GetPos();
	if (m_Option.iEffectCount < 1000)
		m_Option.iEffectCount = 1000;
	else if (m_Option.iEffectCount > 2000)
		m_Option.iEffectCount = 2000;

	m_Option.iViewDist = m_SldViewDist.GetPos();
	if (m_Option.iViewDist < 256)
		m_Option.iViewDist = 256;
	else if (m_Option.iViewDist > 512)
		m_Option.iViewDist = 512;

	m_Option.iEffectSndDist = m_SldEffectSoundDist.GetPos();
	if (m_Option.iEffectSndDist < 20)
		m_Option.iEffectSndDist = 20;
	else if (m_Option.iEffectSndDist > 48)
		m_Option.iEffectSndDist = 48;

	szBuff.Format(_T("%d"), m_Option.iTexLOD_Chr);
	WritePrivateProfileString(_T("Texture"), _T("LOD_Chr"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iTexLOD_Shape);
	WritePrivateProfileString(_T("Texture"), _T("LOD_Shape"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iTexLOD_Terrain);
	WritePrivateProfileString(_T("Texture"), _T("LOD_Terrain"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iUseShadow);
	WritePrivateProfileString(_T("Shadow"), _T("Use"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iViewWidth);
	WritePrivateProfileString(_T("ViewPort"), _T("Width"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iViewHeight);
	WritePrivateProfileString(_T("ViewPort"), _T("Height"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iViewColorDepth);
	WritePrivateProfileString(_T("ViewPort"), _T("ColorDepth"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iViewDist);
	WritePrivateProfileString(_T("ViewPort"), _T("Distance"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iEffectSndDist);
	WritePrivateProfileString(_T("Sound"), _T("Distance"), szBuff, szIniFile);
	szBuff.Format(_T("%d"), m_Option.iEffectCount);
	WritePrivateProfileString(_T("Effect"), _T("Count"), szBuff, szIniFile);

	m_Option.bSoundBgm = (IsDlgButtonChecked(IDC_C_SOUND_BGM)) ? true : false;
	m_Option.bSoundBgm ? szBuff = "1" : szBuff = "0";
	WritePrivateProfileString(_T("Sound"), _T("Bgm"), szBuff, szIniFile);

	m_Option.bSoundEffect = (IsDlgButtonChecked(IDC_C_SOUND_EFFECT)) ? true : false;
	m_Option.bSoundEffect ? szBuff = "1" : szBuff = "0";
	WritePrivateProfileString(_T("Sound"), _T("Effect"), szBuff, szIniFile);

	m_Option.bSndDuplicated = (IsDlgButtonChecked(IDC_C_SOUND_DUPLICATE)) ? true : false;
	m_Option.bSndDuplicated ? szBuff = "1" : szBuff = "0";
	WritePrivateProfileString(_T("Sound"), _T("Duplicate"), szBuff, szIniFile);

	m_Option.bWindowCursor = (IsDlgButtonChecked(IDC_C_CURSOR_WINDOW)) ? true : false;
	m_Option.bWindowCursor ? szBuff = "1" : szBuff = "0";
	WritePrivateProfileString(_T("Cursor"), _T("WindowCursor"), szBuff, szIniFile);

	m_Option.bWindowMode = (IsDlgButtonChecked(IDC_C_WINDOW_MODE)) ? true : false;
	m_Option.bWindowMode ? szBuff = "1" : szBuff = "0";
	WritePrivateProfileString(_T("Screen"), _T("WindowMode"), szBuff, szIniFile);

	m_Option.bEffectVisible = (IsDlgButtonChecked(IDC_C_SHOW_WEAPON_EFFECT)) ? true : false;
	m_Option.bEffectVisible ? szBuff = "1" : szBuff = "0";
	WritePrivateProfileString(_T("WeaponEffect"), _T("EffectVisible"), szBuff, szIniFile);
}

void COptionDlg::SettingLoad(CString szIniFile)
{
	if (szIniFile.GetLength() <= 0)
		return;

	m_Option.iTexLOD_Chr     = GetPrivateProfileInt(_T("Texture"), _T("LOD_Chr"), 0, szIniFile);
	m_Option.iTexLOD_Shape   = GetPrivateProfileInt(_T("Texture"), _T("LOD_Shape"), 0, szIniFile);
	m_Option.iTexLOD_Terrain = GetPrivateProfileInt(_T("Texture"), _T("LOD_Terrain"), 0, szIniFile);
	m_Option.iUseShadow      = GetPrivateProfileInt(_T("Shadow"), _T("Use"), 1, szIniFile);
	m_Option.iViewWidth      = GetPrivateProfileInt(_T("ViewPort"), _T("Width"), 1024, szIniFile);
	m_Option.iViewHeight     = GetPrivateProfileInt(_T("ViewPort"), _T("Height"), 768, szIniFile);
	m_Option.iViewColorDepth = GetPrivateProfileInt(
		_T("ViewPort"), _T("ColorDepth"), 16, szIniFile);
	m_Option.iViewDist      = GetPrivateProfileInt(_T("ViewPort"), _T("Distance"), 512, szIniFile);
	m_Option.iEffectSndDist = GetPrivateProfileInt(_T("Sound"), _T("Distance"), 48, szIniFile);
	m_Option.iEffectCount   = GetPrivateProfileInt(_T("Effect"), _T("Count"), 2000, szIniFile);

	m_Option.bSoundBgm      = GetPrivateProfileInt(_T("Sound"), _T("Bgm"), 1, szIniFile) != 0;
	m_Option.bSoundEffect   = GetPrivateProfileInt(_T("Sound"), _T("Effect"), 1, szIniFile) != 0;
	m_Option.bSndDuplicated = GetPrivateProfileInt(_T("Sound"), _T("Duplicate"), 0, szIniFile) != 0;
	m_Option.bWindowCursor  = GetPrivateProfileInt(_T("Cursor"), _T("WindowCursor"), 1, szIniFile)
							 != 0;
	m_Option.bWindowMode = GetPrivateProfileInt(_T("Screen"), _T("WindowMode"), 0, szIniFile) != 0;
	m_Option.bEffectVisible = GetPrivateProfileInt(
								  _T("WeaponEffect"), _T("EffectVisible"), 1, szIniFile)
							  != 0;
}

void COptionDlg::SettingUpdate()
{
	if (m_Option.iTexLOD_Chr != 0)
		CheckRadioButton(IDC_R_TEX_CHR_HIGH, IDC_R_TEX_CHR_LOW, IDC_R_TEX_CHR_LOW);
	else
		CheckRadioButton(IDC_R_TEX_CHR_HIGH, IDC_R_TEX_CHR_LOW, IDC_R_TEX_CHR_HIGH);

	if (m_Option.iTexLOD_Shape != 0)
		CheckRadioButton(IDC_R_TEX_SHAPE_HIGH, IDC_R_TEX_SHAPE_LOW, IDC_R_TEX_SHAPE_LOW);
	else
		CheckRadioButton(IDC_R_TEX_SHAPE_HIGH, IDC_R_TEX_SHAPE_LOW, IDC_R_TEX_SHAPE_HIGH);

	if (m_Option.iTexLOD_Terrain != 0)
		CheckRadioButton(IDC_R_TEX_TERRAIN_HIGH, IDC_R_TEX_TERRAIN_LOW, IDC_R_TEX_TERRAIN_LOW);
	else
		CheckRadioButton(IDC_R_TEX_TERRAIN_HIGH, IDC_R_TEX_TERRAIN_LOW, IDC_R_TEX_TERRAIN_HIGH);

	CheckDlgButton(IDC_C_SHADOW, m_Option.iUseShadow);

	int iSel = 0;
	for (int i = 0; i < (int) s_supportedResolutions.size(); i++)
	{
		const auto& resolution = s_supportedResolutions[i];
		if (m_Option.iViewWidth == resolution.Width && m_Option.iViewHeight == resolution.Height)
		{
			iSel = i;
			break;
		}
	}

	m_CB_Resolution.SetCurSel(iSel);

	if (16 == m_Option.iViewColorDepth)
		iSel = 0;
	else if (32 == m_Option.iViewColorDepth)
		iSel = 1;
	m_CB_ColorDepth.SetCurSel(iSel);

	m_SldEffectCount.SetPos(m_Option.iEffectCount);
	m_SldViewDist.SetPos(m_Option.iViewDist);
	m_SldEffectSoundDist.SetPos(m_Option.iEffectSndDist);

	CheckDlgButton(IDC_C_SOUND_BGM, m_Option.bSoundBgm);
	CheckDlgButton(IDC_C_SOUND_EFFECT, m_Option.bSoundEffect);
	CheckDlgButton(IDC_C_SOUND_DUPLICATE, m_Option.bSndDuplicated);
	CheckDlgButton(IDC_C_CURSOR_WINDOW, m_Option.bWindowCursor);
	CheckDlgButton(IDC_C_WINDOW_MODE, m_Option.bWindowMode);
	CheckDlgButton(IDC_C_SHOW_WEAPON_EFFECT, m_Option.bEffectVisible);
}

void COptionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if ((void*) pScrollBar == (void*) (&m_SldEffectCount))
		m_Option.iEffectCount = m_SldEffectCount.GetPos();
	else if ((void*) pScrollBar == (void*) (&m_SldViewDist))
		m_Option.iViewDist = m_SldViewDist.GetPos();
	else if ((void*) pScrollBar == (void*) (&m_SldEffectSoundDist))
		m_Option.iEffectSndDist = m_SldEffectSoundDist.GetPos();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void COptionDlg::OnBVersion()
{
	CString szMsg;
	(void) szMsg.LoadString(IDS_CONFIRM_WRITE_REGISRY);

	// 한번 물어본다..
	if (IDNO == MessageBox(szMsg, _T(""), MB_YESNO))
		return;

	DWORD dwVersion = GetDlgItemInt(IDC_E_VERSION);

	CString szVersion, szServerIniPath;
	szVersion.Format(_T("%d"), dwVersion);
	szServerIniPath = m_szInstalledPath + _T("\\Server.ini");
	WritePrivateProfileString(_T("Version"), _T("Files"), szVersion, szServerIniPath);
}
