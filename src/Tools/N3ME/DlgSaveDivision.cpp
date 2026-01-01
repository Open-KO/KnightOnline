// DlgSaveDivision.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSaveDivision.h"

#include "LyTerrain.h"
#include "MapMng.h"
#include <N3Base/N3Texture.h>
#include <N3Base/N3EngTool.h>
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveDivision dialog


CDlgSaveDivision::CDlgSaveDivision(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgSaveDivision::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSaveDivision)
	m_strTotalSize = _T("");
	//}}AFX_DATA_INIT

	m_pTerrain = nullptr;
	m_pMapMng = nullptr;
	m_pMiniMap = nullptr;

	m_iTotalSize = 0;
	m_iDivisionSize = 0;
	m_ptMousePos.x = m_ptMousePos.y = 0;
	
	m_rtDrawRegion.SetRect(0,0,0,0);
}


void CDlgSaveDivision::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSaveDivision)
	DDX_Control(pDX, IDC_CB_DIVISION, m_cbDivision);
	DDX_Text(pDX, IDC_STC_TOTAL_SIZE, m_strTotalSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSaveDivision, CDialog)
	//{{AFX_MSG_MAP(CDlgSaveDivision)
	ON_CBN_SELCHANGE(IDC_CB_DIVISION, OnSelchangeCbDivision)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveDivision message handlers

BOOL CDlgSaveDivision::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_iTotalSize = (m_pTerrain->m_iHeightMapSize-1) * TERRAIN_CELL_SIZE;
	m_iDivisionSize = m_iTotalSize / 2;

	m_strTotalSize.Format("%dm", m_iTotalSize);

	m_cbDivision.ResetContent();
	int tmpDivisionSize = m_iDivisionSize;
	int i=0;
	while(tmpDivisionSize>TERRAIN_CELL_SIZE)
	{
		CString str;
		str.Format("%dm", tmpDivisionSize);
		m_cbDivision.InsertString(i, (LPCTSTR)str);
		m_cbDivision.SetItemData(i, tmpDivisionSize);

		i++;
		tmpDivisionSize /= 2;
	}
	m_cbDivision.SetCurSel(0);

	CWnd* pView = GetDlgItem(IDC_STATIC_MINIMAP);
	pView->SetWindowPos(nullptr, 0, 0, TEX_VIEW_SIZE, TEX_VIEW_SIZE, SWP_DRAWFRAME|SWP_NOMOVE);

	SetTexView();

	m_rtDrawRegion.SetRect(0, 0, m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize, m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);  

	UpdateData(FALSE);	
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSaveDivision::OnSelchangeCbDivision() 
{
	int idx = m_cbDivision.GetCurSel();
	if (idx < 0)
		return;

	m_iDivisionSize = static_cast<int>(m_cbDivision.GetItemData(idx));
	m_rtDrawRegion.right = m_rtDrawRegion.left + (m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);
	m_rtDrawRegion.bottom = m_rtDrawRegion.top + (m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);

	if (m_rtDrawRegion.left < 0)
	{
		m_rtDrawRegion.left = 0;
		m_rtDrawRegion.right = m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize;
	}

	if (m_rtDrawRegion.right >= TEX_VIEW_SIZE)
	{
		m_rtDrawRegion.right = TEX_VIEW_SIZE - 1;
		m_rtDrawRegion.left = m_rtDrawRegion.right - (m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);
	}

	if (m_rtDrawRegion.top < 0)
	{
		m_rtDrawRegion.top = 0;
		m_rtDrawRegion.bottom = m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize;
	}

	if (m_rtDrawRegion.bottom >= TEX_VIEW_SIZE)
	{
		m_rtDrawRegion.bottom = TEX_VIEW_SIZE - 1;
		m_rtDrawRegion.top = m_rtDrawRegion.bottom - (m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);
	}

	RenderTexnRegion();
}

void CDlgSaveDivision::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDialog::OnMouseMove(nFlags, point);

	if(nFlags & MK_LBUTTON)
	{
		POINT cp = point;
		ClientToScreen(&cp);

		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		CWnd* pView = GetDlgItem(IDC_STATIC_MINIMAP);

		CRect rt;
		pView->GetWindowRect(&rt);
			
		rt.left += m_rtDrawRegion.left;
		rt.top += m_rtDrawRegion.top;
		rt.right = rt.left + m_rtDrawRegion.Width(); 
		rt.bottom = rt.top + m_rtDrawRegion.Height();

		if(rt.PtInRect(cp))
		{
			CPoint PtMove; 
			PtMove.x = cp.x;
			PtMove.y = cp.y;

			m_rtDrawRegion.OffsetRect(PtMove-m_ptMousePos);

			if(m_rtDrawRegion.left<0)
			{
				m_rtDrawRegion.left = 0;
				m_rtDrawRegion.right = m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize;
			}
			if(m_rtDrawRegion.right>=TEX_VIEW_SIZE)
			{
				m_rtDrawRegion.right = TEX_VIEW_SIZE - 1;
				m_rtDrawRegion.left = m_rtDrawRegion.right - (m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);
			}
			if(m_rtDrawRegion.top<0)
			{
				m_rtDrawRegion.top = 0;
				m_rtDrawRegion.bottom = m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize;
				
			}
			if(m_rtDrawRegion.bottom>=TEX_VIEW_SIZE)
			{
				m_rtDrawRegion.bottom = TEX_VIEW_SIZE - 1;
				m_rtDrawRegion.top = m_rtDrawRegion.bottom - (m_iDivisionSize * TEX_VIEW_SIZE / m_iTotalSize);
			}
			
			m_ptMousePos = PtMove;
			
			RenderTexnRegion();
		}
	}
}

void CDlgSaveDivision::OnLButtonDown(UINT nFlags, CPoint point) 
{
	POINT cp = point;
	ClientToScreen(&cp);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CWnd* pView = GetDlgItem(IDC_STATIC_MINIMAP);

	CRect rt;
	pView->GetWindowRect(&rt);

	rt.left += m_rtDrawRegion.left;
	rt.top += m_rtDrawRegion.top;
	rt.right = rt.left + m_rtDrawRegion.Width(); 
	rt.bottom = rt.top + m_rtDrawRegion.Height();

	if(rt.PtInRect(cp))
	{
		m_ptMousePos.x = cp.x;
		m_ptMousePos.y = cp.y;
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CDlgSaveDivision::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RenderTexnRegion();
}

void CDlgSaveDivision::RenderTexnRegion()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CWnd* pView = GetDlgItem(IDC_STATIC_MINIMAP);
	pFrm->m_pEng->RenderTexturePreview(m_pMiniMap, pView->GetSafeHwnd());

	CDC *pDCTex;
	pDCTex = pView->GetDC();

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	CPen* pOldPen = (CPen*)pDCTex->SelectObject(&pen);
	
	pDCTex->MoveTo(m_rtDrawRegion.left, m_rtDrawRegion.top);
	pDCTex->LineTo(m_rtDrawRegion.right, m_rtDrawRegion.top);
	pDCTex->LineTo(m_rtDrawRegion.right, m_rtDrawRegion.bottom);
	pDCTex->LineTo(m_rtDrawRegion.left, m_rtDrawRegion.bottom);
	pDCTex->LineTo(m_rtDrawRegion.left, m_rtDrawRegion.top);

	pDCTex->SelectObject(pOldPen);
	pView->ReleaseDC(pDCTex);
}

void CDlgSaveDivision::SetTexView()
{
	CString strTmpFileName("c:\\MiniMap.bmp");
	m_pTerrain->GenerateMiniMap((LPCTSTR)strTmpFileName, TEX_VIEW_SIZE);

	if(m_pMiniMap) m_pMapMng->m_pMainFrm->m_pEng->s_MngTex.Delete(&m_pMiniMap);

	m_pMiniMap = m_pMapMng->m_pMainFrm->m_pEng->s_MngTex.Get((LPCTSTR)strTmpFileName);
	DeleteFile((LPCTSTR)strTmpFileName);
}

void CDlgSaveDivision::OnOK() 
{
	float x, z;
	x = m_rtDrawRegion.left * m_iTotalSize / TEX_VIEW_SIZE;
	z = (TEX_VIEW_SIZE - m_rtDrawRegion.bottom - 1) * m_iTotalSize / TEX_VIEW_SIZE;
	m_pMapMng->SavePartition(x, z, (float)m_iDivisionSize);
	
	CDialog::OnOK();
}
