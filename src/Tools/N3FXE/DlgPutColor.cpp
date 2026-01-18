// DlgPutColor.cpp : implementation file
//

#include "stdafx.h"
#include "n3fxe.h"
#include "DlgPutColor.h"
#include "DlgPercent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPutColor dialog
CDlgPutColor::CDlgPutColor(CWnd* pParent /*=nullptr*/) : CDialog(CDlgPutColor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPutColor)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	for (int i = 0; i < NUM_KEY_COLOR; i++)
	{
		m_Color[i]   = 0x00ffffff;
		m_Opacity[i] = 0xff000000;
	}
}

void CDlgPutColor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPutColor)
	DDX_Control(pDX, IDC_STC_OPACITY, m_stcOpacity);
	DDX_Control(pDX, IDC_STC_COLOR, m_stcColor);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgPutColor, CDialog)
//{{AFX_MSG_MAP(CDlgPutColor)
ON_WM_RBUTTONUP()
ON_WM_LBUTTONUP()
ON_WM_PAINT()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const int PUTCOLOR_INTERVAL = 5;

/////////////////////////////////////////////////////////////////////////////
// CDlgPutColor message handlers

void CDlgPutColor::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd *pColorWnd, *pOpacityWnd, *pMouseWnd;
	pColorWnd   = (CWnd*) &m_stcColor;
	pOpacityWnd = (CWnd*) &m_stcOpacity;
	pMouseWnd   = ChildWindowFromPoint(point);

	CRect rt;
	if (pMouseWnd == pColorWnd)
	{
		CColorDialog ColorDlg;
		if (ColorDlg.DoModal() != IDOK)
			return;

		ClientToScreen(&point);
		pColorWnd->GetWindowRect(&rt);

		int Pos          = (point.x - rt.left) / PUTCOLOR_INTERVAL;
		m_Color[Pos]     = ColorDlg.GetColor();
		m_bColorKey[Pos] = true;

		int idxS         = 0;
		int idxE         = NUM_KEY_COLOR - 1;
		int idx;
		for (idx = Pos - 1; idx >= 0; idx--)
		{
			if (m_bColorKey[idx] == true)
			{
				idxS = idx;
				break;
			}
		}
		GradateColor(idxS, Pos);

		for (idx = Pos + 1; idx < NUM_KEY_COLOR; idx++)
		{
			if (m_bColorKey[idx] == true)
			{
				idxE = idx;
				break;
			}
		}
		GradateColor(Pos, idxE);

		Invalidate();
	}
	if (pMouseWnd == pOpacityWnd)
	{
		CDlgPercent dlg;
		if (dlg.DoModal() != IDOK)
			return;

		ClientToScreen(&point);
		pOpacityWnd->GetWindowRect(&rt);

		int Pos          = (point.x - rt.left) / PUTCOLOR_INTERVAL;
		DWORD Alpha      = dlg.m_iRealValue;
		m_Opacity[Pos]   = (Alpha << 24);
		m_bAlphaKey[Pos] = true;

		int idxS         = 0;
		int idxE         = NUM_KEY_COLOR - 1;
		int idx;
		for (idx = Pos - 1; idx >= 0; idx--)
		{
			if (m_bAlphaKey[idx] == true)
			{
				idxS = idx;
				break;
			}
		}
		GradateAlpha(idxS, Pos);

		for (idx = Pos + 1; idx < NUM_KEY_COLOR; idx++)
		{
			if (m_bAlphaKey[idx] == true)
			{
				idxE = idx;
				break;
			}
		}
		GradateAlpha(Pos, idxE);

		Invalidate();
	}

	CDialog::OnRButtonUp(nFlags, point);
}

void CDlgPutColor::OnRButtonUp(UINT nFlags, CPoint point)
{
	CWnd *pColorWnd, *pOpacityWnd, *pMouseWnd;
	pColorWnd   = GetDlgItem(IDC_STC_COLOR);
	pOpacityWnd = GetDlgItem(IDC_STC_OPACITY);
	pMouseWnd   = ChildWindowFromPoint(point);

	CRect rt;
	if (pMouseWnd == pColorWnd)
	{
		ClientToScreen(&point);
		pColorWnd->GetWindowRect(&rt);

		int Pos = (point.x - rt.left) / PUTCOLOR_INTERVAL;
		if (!m_bColorKey[Pos])
			return;

		int idxS = 0;
		int idxE = NUM_KEY_COLOR - 1;
		int idx;
		for (idx = Pos - 1; idx >= 0; idx--)
		{
			if (m_bColorKey[idx] == true)
			{
				idxS = idx;
				break;
			}
		}
		for (idx = Pos + 1; idx < NUM_KEY_COLOR; idx++)
		{
			if (m_bColorKey[idx] == true)
			{
				idxE = idx;
				break;
			}
		}

		GradateColor(idxS, idxE);

		m_bColorKey[Pos] = false;
		Invalidate();
	}
	if (pMouseWnd == pOpacityWnd)
	{
		ClientToScreen(&point);
		pOpacityWnd->GetWindowRect(&rt);

		int Pos = (point.x - rt.left) / PUTCOLOR_INTERVAL;
		if (!m_bAlphaKey[Pos])
			return;

		int idxS = 0;
		int idxE = NUM_KEY_COLOR - 1;
		int idx;
		for (idx = Pos - 1; idx >= 0; idx--)
		{
			if (m_bAlphaKey[idx] == true)
			{
				idxS = idx;
				break;
			}
		}
		for (idx = Pos + 1; idx < NUM_KEY_COLOR; idx++)
		{
			if (m_bAlphaKey[idx] == true)
			{
				idxE = idx;
				break;
			}
		}

		GradateAlpha(idxS, idxE);

		m_bAlphaKey[Pos] = false;
		Invalidate();
	}

	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CDlgPutColor::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rt;
	m_stcOpacity.GetWindowRect(&rt);
	ScreenToClient(&rt);
	m_stcOpacity.MoveWindow(rt.left, rt.top, PUTCOLOR_INTERVAL * NUM_KEY_COLOR, 20);

	m_stcColor.GetWindowRect(&rt);
	ScreenToClient(&rt);
	m_stcColor.MoveWindow(rt.left, rt.top, PUTCOLOR_INTERVAL * NUM_KEY_COLOR, 20);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPutColor::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.SetBkMode(TRANSPARENT);

	CRect rtColor, rtOp;
	CDC *pDCColor, *pDCOp;

	//
	pDCColor = m_stcColor.GetDC();
	m_stcColor.GetClientRect(&rtColor);

	pDCOp = m_stcOpacity.GetDC();
	m_stcOpacity.GetClientRect(&rtOp);

	CRect MarkRt;
	for (int i = 0; i < NUM_KEY_COLOR; i++)
	{
		rtColor.left  = i * PUTCOLOR_INTERVAL;
		rtColor.right = rtColor.left + PUTCOLOR_INTERVAL + 1;
		pDCColor->FillSolidRect(&rtColor, m_Color[i]);

		if (m_bColorKey[i])
		{
			MarkRt = rtColor;
			m_stcColor.ClientToScreen(&MarkRt);
			this->ScreenToClient(&MarkRt);
			dc.TextOut(MarkRt.left, MarkRt.bottom, "^");
		}

		DWORD Alpha = (m_Opacity[i] >> 24);
		Alpha       = Alpha + (Alpha << 16) + (Alpha << 8);

		rtOp.left   = i * PUTCOLOR_INTERVAL;
		rtOp.right  = rtOp.left + PUTCOLOR_INTERVAL + 1;
		pDCOp->FillSolidRect(&rtOp, Alpha);

		if (m_bAlphaKey[i])
		{
			MarkRt = rtOp;
			m_stcOpacity.ClientToScreen(&MarkRt);
			this->ScreenToClient(&MarkRt);
			dc.TextOut(MarkRt.left, MarkRt.bottom, "^");
		}
	}
	m_stcColor.ReleaseDC(pDCColor);
	m_stcOpacity.ReleaseDC(pDCOp);
}

void CDlgPutColor::GradateColor(int start, int end)
{
	if (start == end)
		return;
	if (start > end)
	{
		int tmp = start;
		start   = end;
		end     = tmp;
	}

	float interval = end - start;
	float unitR, unitG, unitB;
	DWORD startColor, endColor;
	int startR, startG, startB;
	int endR, endG, endB;
	DWORD tmpR, tmpG, tmpB;

	startColor = m_Color[start];
	endColor   = m_Color[end];

	startR     = (int) ((startColor << 24) >> 24);
	startG     = (int) ((startColor << 16) >> 24);
	startB     = (int) ((startColor << 8) >> 24);

	endR       = (int) ((endColor << 24) >> 24);
	endG       = (int) ((endColor << 16) >> 24);
	endB       = (int) ((endColor << 8) >> 24);

	unitR      = (float) (endR - startR) / interval;
	unitG      = (float) (endG - startG) / interval;
	unitB      = (float) (endB - startB) / interval;

	for (int i = start; i < end; i++)
	{
		int idx     = i - start;
		tmpR        = startR + (unitR * (float) idx);
		tmpG        = startG + (unitG * (float) idx);
		tmpB        = startB + (unitB * (float) idx);

		m_Color[i]  = 0x00000000;
		m_Color[i] += tmpR + (tmpG << 8) + (tmpB << 16);
	}
}

void CDlgPutColor::GradateAlpha(int start, int end)
{
	if (start == end)
		return;
	if (start > end)
	{
		int tmp = start;
		start   = end;
		end     = tmp;
	}

	float interval = end - start;
	float unitA;
	DWORD startAlpha, endAlpha;
	int startA;
	int endA;
	DWORD tmpA;

	startAlpha = m_Opacity[start];
	endAlpha   = m_Opacity[end];

	startA     = (int) (startAlpha >> 24);
	endA       = (int) (endAlpha >> 24);

	unitA      = (float) (endA - startA) / interval;

	for (int i = start; i < end; i++)
	{
		int idx      = i - start;
		tmpA         = startA + (unitA * (float) idx);

		m_Opacity[i] = (tmpA << 24);
	}
}
