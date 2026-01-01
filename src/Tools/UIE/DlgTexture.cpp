// DlgTexture.cpp : implementation file
//

#include "stdafx.h"
#include "UIE.h"
#include "DlgTexture.h"
#include "MainFrm.h"

#include <N3Base/N3UIDef.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTexture dialog


CDlgTexture::CDlgTexture(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgTexture::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTexture)
	m_RadioEditMode = 0;
	//}}AFX_DATA_INIT
	m_pTexViewer = new CTexViewer;
	m_iImageTypeCount = 0;
	ZeroMemory(m_szImageTypeNames, sizeof(char)*MAX_IMAGETYPE*_MAX_PATH);
	m_hAccelTable = nullptr;
}

CDlgTexture::~CDlgTexture()
{
	if (m_pTexViewer) {delete m_pTexViewer; m_pTexViewer = nullptr;}
}

void CDlgTexture::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTexture)
	DDX_Control(pDX, IDC_COMBO_IMAGETYPE, m_ImageType);
	DDX_Radio(pDX, IDC_RADIO_SELECT, m_RadioEditMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTexture, CDialog)
	//{{AFX_MSG_MAP(CDlgTexture)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO_SELECT, OnRadioSelect)
	ON_BN_CLICKED(IDC_RADIO_ZOOM, OnRadioZoom)
	ON_BN_CLICKED(IDC_RADIO_HAND, OnRadioHand)
	ON_BN_DOUBLECLICKED(IDC_RADIO_ZOOM, OnDoubleclickedRadioZoom)
	ON_CBN_SELCHANGE(IDC_COMBO_IMAGETYPE, OnSelchangeComboImagetype)
	ON_BN_CLICKED(IDC_BUTTON_AUTOSELECT_H, OnButtonAutoselectH)
	ON_BN_CLICKED(IDC_BUTTON_AUTOSELECT_V, OnButtonAutoselectV)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UM_UPDATE_INFO, OnUpdateInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTexture message handlers

int CDlgTexture::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ASSERT(m_pTexViewer);
	if (FALSE == m_pTexViewer->Create(nullptr, "TexViewer", WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1000)) return -1;
	
	return 0;
}

BOOL CDlgTexture::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_iImageTypeCount>0)
	{
		m_ImageType.ShowWindow(SW_SHOW);
		int i;
		for (i=0; i<m_iImageTypeCount; ++i)	m_ImageType.AddString(m_szImageTypeNames[i]);
		m_ImageType.SetCurSel(0);
		m_pTexViewer->SetImageTypeIndex(0);
	}
	else m_ImageType.ShowWindow(SW_HIDE);
	Resize();

	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCEL_DLGTEXTURE));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgTexture::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	Resize();	
}

void CDlgTexture::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (m_pTexViewer) m_pTexViewer->DestroyWindow();
	
}

void CDlgTexture::OnSelchangeComboImagetype() 
{
	ASSERT(m_pTexViewer && m_ImageType.GetCurSel() >=0);
	m_pTexViewer->SetImageTypeIndex(m_ImageType.GetCurSel());
	
}

void CDlgTexture::OnOK() 
{
	ASSERT(m_pTexViewer);
	if (m_iImageTypeCount>0)
	{	// 모두 영역 선택이 되었나 체크
		int i;
		for (i=0; i<m_iImageTypeCount; ++i)
		{
			if (-1 == m_pTexViewer->GetImageRect(i).left)
			{
				CString str, strLBText;
				m_ImageType.GetLBText(i, strLBText);
				str.Format("%s의 영역이 선택되지 않았습니다.", strLBText);
				MessageBox(str);
				return;
			}
		}
		CDialog::OnOK();
	}
	else
	{
		if (-1 != m_pTexViewer->GetSelectedRect().left)	CDialog::OnOK();
		else MessageBox("영역이 선택 되지 않았습니다.");
	}
}

void CDlgTexture::SetTexture(LPCTSTR pszFileName)
{
	if (m_pTexViewer) m_pTexViewer->SetTexture(pszFileName);
}

void CDlgTexture::OnRadioSelect() 
{
	if (nullptr == m_pTexViewer) return;
	CTexViewer::eEDITMODE eEditMode = m_pTexViewer->SetEditMode(CTexViewer::EDITMODE_SELECT);
	if (CTexViewer::EDITMODE_SELECT != eEditMode)
	{	// 모드 바꾸기 실패
		UpdateData(TRUE);
		m_RadioEditMode = eEditMode;
		UpdateData(FALSE);
	}
}

void CDlgTexture::OnRadioZoom() 
{
	if (nullptr == m_pTexViewer) return;
	CTexViewer::eEDITMODE eEditMode = m_pTexViewer->SetEditMode(CTexViewer::EDITMODE_ZOOM);
	if (CTexViewer::EDITMODE_ZOOM != eEditMode)
	{	// 모드 바꾸기 실패
		UpdateData(TRUE);
		m_RadioEditMode = eEditMode;
		UpdateData(FALSE);
	}
}

void CDlgTexture::OnDoubleclickedRadioZoom() 
{
	if (nullptr == m_pTexViewer) return;
	if (m_pTexViewer) m_pTexViewer->Zoom(1.0f);
}

void CDlgTexture::OnRadioHand() 
{
	if (nullptr == m_pTexViewer) return;
	CTexViewer::eEDITMODE eEditMode = m_pTexViewer->SetEditMode(CTexViewer::EDITMODE_HAND);
	if (CTexViewer::EDITMODE_HAND != eEditMode)
	{	// 모드 바꾸기 실패
		UpdateData(TRUE);
		m_RadioEditMode = eEditMode;
		UpdateData(FALSE);
	}
}

LRESULT	CDlgTexture::OnUpdateInfo(WPARAM wParam, LPARAM lParam)
{
	if (nullptr == m_pTexViewer) return 0;
	CWnd* pStatic = GetDlgItem(IDC_STATIC_INFO);
	if (nullptr == pStatic) return 0;

	CPoint	ptImage = CPoint((short)HIWORD(lParam), (short)LOWORD(lParam) );
	CRect	rcSelected = m_pTexViewer->GetSelectedRect();
	CRect	rcTmp = rcSelected;
	if (rcTmp.left > rcTmp.right) {rcSelected.left = rcTmp.right; rcSelected.right = rcTmp.left;}
	if (rcTmp.top > rcTmp.bottom) {rcSelected.top = rcTmp.bottom; rcSelected.bottom = rcTmp.top;}
	CString strPos, strRect, strAll;
	if (wParam) strPos.Format("X : %d\nY : %d\n", ptImage.x, ptImage.y);
	else strPos.Format("X :\nY :\n");

	if (-1 == rcSelected.left) strRect.Format("Left :\nTop :\nRight :\nBottom :\nWidth :\nHeight :\n");
	else strRect.Format("Left : %d\nTop : %d\nRight : %d\nBottom : %d\nWidth : %d\nHeight : %d\n",
		rcSelected.left,rcSelected.top,rcSelected.right,rcSelected.bottom,rcSelected.Width(),rcSelected.Height());
	strAll = strPos + strRect;
	
	pStatic->SetWindowText(strAll);
	return 0;
}

// control들 다시 배치
void CDlgTexture::Resize()
{
	CWnd* pOKBtn = GetDlgItem(IDOK);
	CWnd* pCancelBtn = GetDlgItem(IDCANCEL);
	if (pOKBtn && pCancelBtn)
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		const int iOffset = 3;
		CRect rc, rcClient;
		GetClientRect(&rcClient);
		CWnd* pWnd = nullptr;
		pWnd = GetDlgItem(IDC_STATIC_INFO);
		pWnd->GetWindowRect(&rc);

		// texture window 배치
		int iTexViewerWidth = rcClient.Width() - rc.Width() - iOffset;
		int iTexViewerHeight = rcClient.Height();
		if (iTexViewerWidth < 0)
			iTexViewerWidth = 0;
		else if (iTexViewerWidth > static_cast<int>(pFrm->m_Eng.s_DevParam.BackBufferWidth))
			iTexViewerWidth = static_cast<int>(pFrm->m_Eng.s_DevParam.BackBufferWidth);

		if (iTexViewerHeight < 0)
			iTexViewerHeight = 0;
		else if (iTexViewerHeight > static_cast<int>(pFrm->m_Eng.s_DevParam.BackBufferHeight))
			iTexViewerHeight = static_cast<int>(pFrm->m_Eng.s_DevParam.BackBufferHeight);

		m_pTexViewer->MoveWindow(0, 0, iTexViewerWidth, iTexViewerHeight);

		// 버튼들 배치
		pWnd = GetDlgItem(IDC_STATIC_INFO);
		pWnd->GetWindowRect(&rc);
		CPoint ptCtrl(rcClient.Width()-rc.Width(), 0);
		pWnd->MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();

		pWnd = GetDlgItem(IDC_RADIO_SELECT);
		pWnd->GetWindowRect(&rc);
		pWnd->MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();
		pWnd = GetDlgItem(IDC_RADIO_ZOOM);
		pWnd->GetWindowRect(&rc);
		pWnd->MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();
		pWnd = GetDlgItem(IDC_RADIO_HAND);
		pWnd->GetWindowRect(&rc);
		pWnd->MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();
		
		if (m_iImageTypeCount>0)
		{
			pWnd = GetDlgItem(IDC_BUTTON_AUTOSELECT_H);
			pWnd->ShowWindow(SW_SHOW);
			pWnd->GetWindowRect(&rc);
			pWnd->MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();
			pWnd = GetDlgItem(IDC_BUTTON_AUTOSELECT_V);
			pWnd->ShowWindow(SW_SHOW);
			pWnd->GetWindowRect(&rc);
			pWnd->MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();

			m_ImageType.GetWindowRect(&rc);
			m_ImageType.MoveWindow(ptCtrl.x, ptCtrl.y, rc.Width(), rc.Height());	 ptCtrl.y += rc.Height();
		}

		ptCtrl.y = rcClient.bottom;
		pOKBtn->GetWindowRect(&rc);
		pOKBtn->MoveWindow(ptCtrl.x, ptCtrl.y-rc.Height(), rc.Width(), rc.Height());	 ptCtrl.y -= rc.Height();
		pCancelBtn->GetWindowRect(&rc);
		pCancelBtn->MoveWindow(ptCtrl.x, ptCtrl.y-rc.Height(), rc.Width(), rc.Height());	 ptCtrl.y -= rc.Height();
	}
}

BOOL CDlgTexture::GetSelectedUVRect(struct __FLOAT_RECT* pFRect) const
{
	if (nullptr == m_pTexViewer) return FALSE;
	return m_pTexViewer->GetSelectedUVRect(pFRect);
}

void CDlgTexture::SetSelectedUVRect(const __FLOAT_RECT* pFRect)	// 현재 선택된 UV좌표 넣기
{
	if (nullptr == m_pTexViewer) return;
	m_pTexViewer->SetSelectedUVRect(pFRect);
}

CRect CDlgTexture::GetSelectedRect() const
{
	if (nullptr == m_pTexViewer) return CRect(-1,-1,-1,-1);
	return m_pTexViewer->GetSelectedRect();
}

void CDlgTexture::SetImageTypes(int iCount, char** pszNames)
{
	ASSERT(0 == m_iImageTypeCount && iCount<=MAX_IMAGETYPE && m_pTexViewer);
	if (0 > iCount || nullptr == pszNames) return;
	m_pTexViewer->SetImageTypeCount(iCount);
	m_iImageTypeCount = iCount;

	int i;
	for (i=0; i<iCount; ++i) lstrcpy(m_szImageTypeNames[i], pszNames[i]);
}

CRect CDlgTexture::GetImageRect(int iIndex, __FLOAT_RECT* pUVRect)
{
	ASSERT(m_pTexViewer);
	CRect rc = m_pTexViewer->GetImageRect(iIndex);
	CSize TexSize = m_pTexViewer->GetTexSize();
	if (pUVRect)
	{
		ZeroMemory(pUVRect, sizeof(__FLOAT_RECT));
		if (TexSize.cx > 1 && TexSize.cy > 1)
		{
			pUVRect->left = (float)rc.left/(TexSize.cx);
			pUVRect->right = (float)rc.right/(TexSize.cx);
			pUVRect->top = (float)rc.top/(TexSize.cy);
			pUVRect->bottom = (float)rc.bottom/(TexSize.cy);
		}
	}
	return rc;
	
}

void CDlgTexture::OnButtonAutoselectH() 
{
	ASSERT(m_pTexViewer);
	CString strErrMsg;
	if (FALSE == m_pTexViewer->AutoMultiRectSelect(TRUE, strErrMsg))
	{
		MessageBox(strErrMsg);
	}	
}

void CDlgTexture::OnButtonAutoselectV() 
{
	ASSERT(m_pTexViewer);
	CString strErrMsg;
	if (FALSE == m_pTexViewer->AutoMultiRectSelect(FALSE, strErrMsg))
	{
		MessageBox(strErrMsg);
	}	
}

BOOL CDlgTexture::PreTranslateMessage(MSG* pMsg) 
{
	if (m_hAccelTable) TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}
