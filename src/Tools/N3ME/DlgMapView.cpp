// DlgMapView.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgMapView.h"
#include "MainFrm.h"
#include "MapMng.h"
#include <N3Base/N3Texture.h>
#include <N3Base/N3EngTool.h>
#include <N3Base/N3Scene.h>
#include "QTNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMapView dialog

CDlgMapView::CDlgMapView(CWnd* pParent /*=nullptr*/) : CDialog(CDlgMapView::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMapView)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgMapView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMapView)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgMapView, CDialog)
//{{AFX_MSG_MAP(CDlgMapView)
ON_WM_PAINT()
ON_BN_CLICKED(IDC_MAP_UPDATA, OnMapUpdata)
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEWHEEL()
ON_BN_CLICKED(IDC_MAP_VIEW_CANCEL, OnMapViewCancel)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMapView message handlers

BOOL CDlgMapView::OnInitDialog()
{
	CDialog::OnInitDialog();

	lbuttondown = FALSE;
	SetRect(&DragRect, 0, 0, 0, 0);

	hbrush          = (HBRUSH) GetStockObject(NULL_BRUSH);
	PenRed          = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	PenBlue         = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));

	Brush_Size      = 128;
	MousePoint.y    = 0;
	MousePoint.x    = 0;
	Select_Map_Edit = FALSE;

	// TODO: Add extra initialization here
	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMapView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_pFrame->GetMapMng()->GetTerrain() != nullptr)
	{
		CWnd* pWnd = GetDlgItem(IDC_MAP_VIEW);
		CRect rc;
		pWnd->GetClientRect(&rc);

		HDC dc;
		dc = ::GetDC(pWnd->m_hWnd);

		SetStretchBltMode(dc, STRETCH_HALFTONE);
		StretchDIBits(dc, 0, 0, rc.right, rc.bottom, 0, 0, Map_View_Size, Map_View_Size,
			(void*) BMF.m_pPixels, (BITMAPINFO*) BMF.GetBitmapInfoHeader(), DIB_RGB_COLORS,
			SRCCOPY);

		OldBrush = (HBRUSH) SelectObject(dc, (HBRUSH) hbrush);
		oldpen   = (HPEN) SelectObject(dc, (HPEN) PenBlue);

		Rectangle(
			dc, MousePoint.x, MousePoint.y, MousePoint.x + Brush_Size, MousePoint.y + Brush_Size);
		/*
		oldpen	=(HPEN)SelectObject(dc, (HPEN)PenRed);

		for( int i = 0; i < Map_View_Size/2 ; i+= Brush_Size)
		{
			MoveToEx(dc,i,Map_View_Size/2,nullptr);
			LineTo(dc,i,0);
		}

		for( i = 0; i < Map_View_Size/2 ; i+= Brush_Size)
		{
			MoveToEx(dc,Map_View_Size/2,i,nullptr);
			LineTo(dc,0,i);
		}
*/
	}
}

void CDlgMapView::OnMapUpdata()
{
	// TODO: Add your control notification handler code here
	if (m_pFrame->GetMapMng()->GetTerrain() != nullptr)
	{
		CWnd* pWnd = GetDlgItem(IDC_MAP_VIEW);
		ColorMapTemp();
		OnPaint();
	}
}

void CDlgMapView::UpData()
{
	OnMapUpdata();
};

void CDlgMapView::ColorMapTemp(void)
{
	if (m_pFrame->GetMapMng()->GetTerrain()->m_iNumColorMap <= 0)
		return;

	int m_iNumColorMap     = m_pFrame->GetMapMng()->GetTerrain()->m_iNumColorMap;
	int m_iColorMapTexSize = m_pFrame->GetMapMng()->GetTerrain()->m_iColorMapTexSize;

	if (false
		== BMF.Create(m_iNumColorMap * m_iColorMapTexSize, m_iNumColorMap * m_iColorMapTexSize))
		return;

	Map_View_Size = m_iNumColorMap * m_iColorMapTexSize;

	int x = 0, z = 0, x2 = 0, z2 = 0;
	for (z = 0; z < m_iNumColorMap; z++)
	{
		for (x = 0; x < m_iNumColorMap; x++)
		{
			if (m_pFrame->GetMapMng()->GetTerrain()->m_pColorTexture[x][z].Get() == nullptr
				|| m_pFrame->GetMapMng()->GetTerrain()->m_pColorTexture[x][z].PixelFormat()
					   != D3DFMT_X8R8G8B8
				|| m_pFrame->GetMapMng()->GetTerrain()->m_pColorTexture[x][z].Width()
					   != m_iColorMapTexSize
				|| m_pFrame->GetMapMng()->GetTerrain()->m_pColorTexture[x][z].Height()
					   != m_iColorMapTexSize)
			{
				::MessageBox(::GetActiveWindow(), "Colormap's format is invalid",
					"Can't export colormap", MB_OK);
			}

			char* pPixelDest = (char*) (BMF.Pixels(
				x * m_iColorMapTexSize, (m_iNumColorMap - z - 1) * m_iColorMapTexSize));
			int nPitchDest   = BMF.Pitch();
			D3DLOCKED_RECT lr;
			m_pFrame->GetMapMng()->GetTerrain()->m_pColorTexture[x][z].Get()->LockRect(
				0, &lr, nullptr, 0);
			char* pPixelSrc = (char*) (lr.pBits);

			for (z2 = 0; z2 < m_iColorMapTexSize; z2++)
			{
				char* pDestTmp = pPixelDest + z2 * nPitchDest;
				char* pSrcTmp  = pPixelSrc + z2 * lr.Pitch;
				for (x2 = 0; x2 < m_iColorMapTexSize; x2++)
				{
					*(pDestTmp + 0)  = *(pSrcTmp + 0); // Red
					*(pDestTmp + 1)  = *(pSrcTmp + 1); // Green
					*(pDestTmp + 2)  = *(pSrcTmp + 2); // Blue

					pDestTmp        += 3;
					pSrcTmp         += 4;
				}
			}

			m_pFrame->GetMapMng()->GetTerrain()->m_pColorTexture[x][z].Get()->UnlockRect(0);
		}
	}
}

void CDlgMapView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CDialog::OnMouseMove(nFlags, point);
	if (lbuttondown == TRUE)
	{
		MousePoint.x = point.x - Brush_Size / 2;
		MousePoint.y = point.y - Brush_Size / 2;

		if (MousePoint.x < 0)
			MousePoint.x = 0;
		if (MousePoint.y < 0)
			MousePoint.y = 0;

		if (MousePoint.x + Brush_Size > 512)
			MousePoint.x = 512 - Brush_Size;

		if (MousePoint.y + Brush_Size > 512)
			MousePoint.y = 512 - Brush_Size;

		OnPaint();
	}
}

void CDlgMapView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	MousePoint.x = point.x - Brush_Size / 2;
	MousePoint.y = point.y - Brush_Size / 2;

	if (MousePoint.x < 0)
		MousePoint.x = 0;
	if (MousePoint.y < 0)
		MousePoint.y = 0;

	if (MousePoint.x + Brush_Size > 512)
		MousePoint.x = 512 - Brush_Size;

	if (MousePoint.y + Brush_Size > 512)
		MousePoint.y = 512 - Brush_Size;

	lbuttondown = TRUE;

	OnPaint();

	CDialog::OnLButtonDown(nFlags, point);
}

void CDlgMapView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (lbuttondown == TRUE)
	{
		lbuttondown = FALSE;
		SetMapView();
	}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CDlgMapView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	if (zDelta < 0)
	{
		if (Brush_Size < 255)
		{
			Brush_Size += 1;
			if (MousePoint.x < 0)
				MousePoint.x = 0;
			if (MousePoint.y < 0)
				MousePoint.y = 0;

			if (MousePoint.x + Brush_Size > 512)
				MousePoint.x = 512 - Brush_Size;

			if (MousePoint.y + Brush_Size > 512)
				MousePoint.y = 512 - Brush_Size;
			OnPaint();
		}
	}
	else
	{
		if (Brush_Size > 64)
		{
			Brush_Size -= 1;
			if (MousePoint.x < 0)
				MousePoint.x = 0;
			if (MousePoint.y < 0)
				MousePoint.y = 0;

			if (MousePoint.x + Brush_Size > 512)
				MousePoint.x = 512 - Brush_Size;

			if (MousePoint.y + Brush_Size > 512)
				MousePoint.y = 512 - Brush_Size;
			OnPaint();
		}
	}

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CDlgMapView::OnMapViewCancel()
{
	// TODO: Add your control notification handler code here
	ShowWindow(FALSE);
	Select_Map_Edit = FALSE;
}

void CDlgMapView::SetMapView(void)
{
	SetRect(&DragRect, MousePoint.x, MousePoint.y, MousePoint.x + Brush_Size,
		MousePoint.y + Brush_Size);

	int temp           = (512 - Brush_Size) - DragRect.top;
	DragRect.top       = temp;
	DragRect.bottom    = DragRect.top + Brush_Size;

	CN3Camera* pCamera = m_pFrame->GetMapMng()->GetSceneOutput()->CameraGetActive();
	if (m_pFrame->GetMapMng()->GetTerrain() == nullptr || pCamera == nullptr)
		return;

	__Vector3 vPos, vAt, vUp;
	__Vector3 vDir = pCamera->Dir();
	int half_x     = (DragRect.right - DragRect.left) / 2;
	int half_z     = (DragRect.bottom - DragRect.top) / 2;
	float xx       = (DragRect.left + half_x) * Map_View_Size / 512;
	float zz       = (512 - (DragRect.bottom - half_z) - 1) * Map_View_Size / 512;

	vAt.Set(xx + 30, 20, zz + 30);
	vPos.Set(xx + 30 - (vDir.x * 50), 25, zz + 30 - (vDir.z * 50));
	vUp.Set(0.0f, 1.0f, 0.0f);
	pCamera->LookAt(vPos, vAt, vUp);

	pCamera->Apply();

	m_pFrame->GetMapMng()->Invalidate();
};

void CDlgMapView::Tick(void)
{
	if (m_pFrame->GetMapMng()->GetTerrain() != nullptr)
	{
	}
};

void CDlgMapView::Render(void)
{
	float xx          = DragRect.left * Map_View_Size / 512;
	float zz          = (512 - DragRect.bottom - 1) * Map_View_Size / 512;

	int HeightMapSize = (Brush_Size * 2 / TERRAIN_CELL_SIZE) + 1;

	int sx            = xx / TERRAIN_CELL_SIZE;
	int sz            = zz / TERRAIN_CELL_SIZE;

	int ex            = sx + HeightMapSize;
	int ez            = sz + HeightMapSize;

	RECT RenderRect;
	SetRect(&RenderRect, sx, sz, ex, ez);
	POINT point;
	for (CQTNode* pQTNode : m_pFrame->GetMapMng()->GetTerrain()->m_RenderNodes)
	{
		point.x = pQTNode->GetCenterX();
		point.y = pQTNode->GetCenterZ();

		if (PtInRect(&RenderRect, point))
			pQTNode->Render();
	}
}
