// MacProgressCtrl.cpp : implementation file
//
//	CMacProgressCtrl class, version 1.0
//
//	Copyright (c) 1999 Paul M. Meidinger (pmmeidinger@yahoo.com)
//
// Feel free to modifiy and/or distribute this file, but
// do not remove this header.
//
// I would appreciate a notification of any bugs discovered or
// improvements that could be made.
//
// This file is provided "as is" with no expressed or implied warranty.
//
//	History:
//		PMM	12/21/1999		Initial implementation.

#include "stdafx.h"
#include "MacProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDT_INDETERMINATE 100
#define IND_BAND_WIDTH    20

// Funtion prototypes.
COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal);
COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal);

//-------------------------------------------------------------------
//
COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal)
//
// Return Value:	None.
//
// Parameters	:	crColor - References a COLORREF structure.
//						byReduceVal - The amount to reduce the RGB values by.
//
// Remarks		:	Lightens a color by increasing the RGB values by the given number.
//
{
	BYTE byRed   = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue  = GetBValue(crColor);

	if ((byRed + byIncreaseVal) <= 255)
		byRed = BYTE(byRed + byIncreaseVal);
	if ((byGreen + byIncreaseVal) <= 255)
		byGreen = BYTE(byGreen + byIncreaseVal);
	if ((byBlue + byIncreaseVal) <= 255)
		byBlue = BYTE(byBlue + byIncreaseVal);

	return RGB(byRed, byGreen, byBlue);
} // LightenColorref

//-------------------------------------------------------------------
//
COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal)
//
// Return Value:	None.
//
// Parameters	:	crColor - References a COLORREF structure.
//						byReduceVal - The amount to reduce the RGB values by.
//
// Remarks		:	Darkens a color by reducing the RGB values by the given number.
//
{
	BYTE byRed   = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue  = GetBValue(crColor);

	if (byRed >= byReduceVal)
		byRed = BYTE(byRed - byReduceVal);
	if (byGreen >= byReduceVal)
		byGreen = BYTE(byGreen - byReduceVal);
	if (byBlue >= byReduceVal)
		byBlue = BYTE(byBlue - byReduceVal);

	return RGB(byRed, byGreen, byBlue);
} // DarkenColorref

/////////////////////////////////////////////////////////////////////////////
// CMacProgressCtrl

//-------------------------------------------------------------------
//
CMacProgressCtrl::CMacProgressCtrl()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Standard constructor.
//
{
	m_bIndeterminate = FALSE;
	m_nIndOffset     = 0;
	m_crColor        = ::GetSysColor(COLOR_HIGHLIGHT);
	GetColors();
	CreatePens();
} // CMacProgressCtrl

//-------------------------------------------------------------------
//
CMacProgressCtrl::~CMacProgressCtrl()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	None.
//
{
	DeletePens();
} // ~CMacProgressCtrl

BEGIN_MESSAGE_MAP(CMacProgressCtrl, CProgressCtrl)
//{{AFX_MSG_MAP(CMacProgressCtrl)
ON_WM_PAINT()
ON_WM_TIMER()
ON_WM_ERASEBKGND()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacProgressCtrl message handlers

//-------------------------------------------------------------------
//
void CMacProgressCtrl::OnPaint()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	The framework calls this member function when Windows
//						or an application makes a request to repaint a portion
//						of an application뭩 window.
//
{
	CPaintDC dcPaint(this); // device context for painting
	CRect rect, rectClient;
	GetClientRect(rectClient);
	rect           = rectClient;
	BOOL bVertical = GetStyle() & PBS_VERTICAL;

	// Create a memory DC for drawing.
	CDC dc;
	dc.CreateCompatibleDC(&dcPaint);
	int nSavedDC = dc.SaveDC();
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dcPaint, rect.Width(), rect.Height());
	CBitmap* pOldBmp = dc.SelectObject(&bmp);

	CBrush br1(m_crColorLightest);
	CBrush br2(::GetSysColor(COLOR_3DFACE));
	dc.FillRect(rect, &br2);

	int nLower, nUpper;
	GetRange(nLower, nUpper);

	// Determine the size of the bar and draw it.
	if (bVertical)
	{
		if (!m_bIndeterminate)
			rect.top = rect.bottom
					   - int(((float) rect.Height() * float(GetPos() - nLower))
							 / float(nUpper - nLower));
		dc.FillRect(rect, &br1);
		DrawVerticalBar(&dc, rect);
	}
	else
	{
		if (!m_bIndeterminate)
			rect.right = int(
				((float) rect.Width() * float(GetPos() - nLower)) / float(nUpper - nLower));
		dc.FillRect(rect, &br1);
		DrawHorizontalBar(&dc, rect);
	}

	dcPaint.BitBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), &dc,
		rectClient.left, rectClient.top, SRCCOPY);

	dc.SelectObject(pOldBmp);
	dc.RestoreDC(nSavedDC);
	dc.DeleteDC();
} // OnPaint

//-------------------------------------------------------------------
//
void CMacProgressCtrl::DrawHorizontalBar(CDC* pDC, const CRect rect)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies the device context object.
//						rect - Specifies the rectangle of the progess bar.
//
// Remarks		:	Draws a horizontal progress bar.
//
{
	if (!rect.Width())
		return;

	int nLeft     = rect.left;
	int nTop      = rect.top;
	int nBottom   = rect.bottom;

	// Assume we're not drawing the indeterminate state.
	CPen* pOldPen = pDC->SelectObject(&m_penColorLight);

	if (m_bIndeterminate)
	{
		pOldPen       = pDC->SelectObject(&m_penColor);
		int nNumBands = (rect.Width() / IND_BAND_WIDTH) + 2;
		int nHeight   = rect.Height() + 1;

		int nAdjust   = nLeft - IND_BAND_WIDTH + m_nIndOffset;
		int nXpos     = 0;
		int nYpos1    = nTop + 1;
		int nYpos2    = nBottom - 2;

		for (int i = 0; i < nNumBands; i++)
		{
			nXpos = nAdjust + (i * IND_BAND_WIDTH);

			pDC->SelectObject(&m_penColorDarker);
			pDC->MoveTo(nXpos + 1, nTop);
			pDC->LineTo(nXpos + nHeight, nBottom);

			pDC->SelectObject(&m_penColorDark);
			pDC->MoveTo(nXpos + 2, nTop);
			pDC->LineTo(nXpos + nHeight + 1, nBottom);
			pDC->MoveTo(nXpos + 10, nTop);
			pDC->LineTo(nXpos + nHeight + 9, nBottom);

			pDC->SelectObject(&m_penColor);
			pDC->MoveTo(nXpos + 3, nTop);
			pDC->LineTo(nXpos + nHeight + 2, nBottom);
			pDC->MoveTo(nXpos + 9, nTop);
			pDC->LineTo(nXpos + nHeight + 8, nBottom);

			pDC->SelectObject(&m_penColorLight);
			pDC->MoveTo(nXpos + 4, nTop);
			pDC->LineTo(nXpos + nHeight + 3, nBottom);
			pDC->MoveTo(nXpos + 8, nTop);
			pDC->LineTo(nXpos + nHeight + 7, nBottom);

			pDC->SelectObject(&m_penColorLighter);
			pDC->MoveTo(nXpos + 5, nTop);
			pDC->LineTo(nXpos + nHeight + 4, nBottom);
			pDC->MoveTo(nXpos + 7, nTop);
			pDC->LineTo(nXpos + nHeight + 6, nBottom);
		} // for the number of bands
	} // if indeterminate
	else
	{
		int nRight = rect.right;

		pDC->MoveTo(nLeft + 2, nBottom - 4);
		pDC->LineTo(nRight - 2, nBottom - 4);
		pDC->MoveTo(nLeft + 2, nTop + 2);
		pDC->LineTo(nRight - 2, nTop + 2);
		pDC->SetPixel(nLeft + 1, nBottom - 3, m_crColorLight);
		pDC->SetPixel(nLeft + 1, nTop + 1, m_crColorLight);

		pDC->SelectObject(&m_penColorLighter);
		pDC->MoveTo(nLeft + 2, nBottom - 5);
		pDC->LineTo(nRight - 3, nBottom - 5);
		pDC->LineTo(nRight - 3, nTop + 3);
		pDC->LineTo(nLeft + 1, nTop + 3);
		pDC->SetPixel(nLeft + 1, nBottom - 4, m_crColorLighter);
		pDC->SetPixel(nLeft + 1, nTop + 2, m_crColorLighter);

		pDC->SelectObject(&m_penColor);
		pDC->MoveTo(nLeft, nBottom - 1);
		pDC->LineTo(nLeft, nTop);
		pDC->LineTo(nLeft + 2, nTop);
		pDC->SetPixel(nLeft + 1, nBottom - 2, m_crColor);
		pDC->MoveTo(nLeft + 2, nBottom - 3);
		pDC->LineTo(nRight - 2, nBottom - 3);
		pDC->MoveTo(nLeft + 2, nTop + 1);
		pDC->LineTo(nRight - 1, nTop + 1);

		pDC->SelectObject(&m_penColorDark);
		pDC->MoveTo(nLeft + 2, nBottom - 2);
		pDC->LineTo(nRight - 2, nBottom - 2);
		pDC->LineTo(nRight - 2, nTop + 1);
		pDC->MoveTo(nLeft + 2, nTop);
		pDC->LineTo(nRight, nTop);
		pDC->SetPixel(nLeft + 1, nBottom - 1, m_crColorDark);

		pDC->SelectObject(&m_penColorDarker);
		pDC->MoveTo(nLeft + 2, nBottom - 1);
		pDC->LineTo(nRight - 1, nBottom - 1);
		pDC->LineTo(nRight - 1, nTop);

		pDC->SelectObject(&m_penShadow);
		pDC->MoveTo(nRight, nTop);
		pDC->LineTo(nRight, nBottom);

		pDC->SelectObject(&m_penLiteShadow);
		pDC->MoveTo(nRight + 1, nTop);
		pDC->LineTo(nRight + 1, nBottom);
	} // if not indeterminate

	pDC->SelectObject(pOldPen);
} // DrawHorizontalBar

//-------------------------------------------------------------------
//
void CMacProgressCtrl::DrawVerticalBar(CDC* pDC, const CRect rect)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies the device context object.
//						rect - Specifies the rectangle of the progess bar.
//
// Remarks		:	Draws a vertical progress bar.
//
{
	int nHeight = rect.Height();
	if (!nHeight)
		return;

	int nLeft     = rect.left;
	int nTop      = rect.top;
	int nRight    = rect.right;
	int nBottom   = rect.bottom;

	CPen* pOldPen = pDC->SelectObject(&m_penColor);

	if (m_bIndeterminate)
	{
		int nNumBands = (nHeight / IND_BAND_WIDTH) + 2;
		int nHeight   = rect.Width() + 1;

		int nAdjust   = nBottom - m_nIndOffset;
		int nXpos1    = nLeft;
		int nXpos2    = nRight + 1;
		int nYpos     = nTop + 1;

		for (int i = 0; i < nNumBands; i++)
		{
			nYpos = nAdjust - (i * IND_BAND_WIDTH);

			pDC->SelectObject(&m_penColorDarker);
			pDC->MoveTo(nXpos1, nYpos);
			pDC->LineTo(nXpos2, nYpos + nHeight);

			pDC->SelectObject(&m_penColorDark);
			pDC->MoveTo(nXpos1, nYpos + 1);
			pDC->LineTo(nXpos2, nYpos + nHeight + 1);
			pDC->MoveTo(nXpos1, nYpos + 9);
			pDC->LineTo(nXpos2, nYpos + nHeight + 9);

			pDC->SelectObject(&m_penColor);
			pDC->MoveTo(nXpos1, nYpos + 2);
			pDC->LineTo(nXpos2, nYpos + nHeight + 2);
			pDC->MoveTo(nXpos1, nYpos + 8);
			pDC->LineTo(nXpos2, nYpos + nHeight + 8);

			pDC->SelectObject(&m_penColorLight);
			pDC->MoveTo(nXpos1, nYpos + 3);
			pDC->LineTo(nXpos2, nYpos + nHeight + 3);
			pDC->MoveTo(nXpos1, nYpos + 7);
			pDC->LineTo(nXpos2, nYpos + nHeight + 7);

			pDC->SelectObject(&m_penColorLighter);
			pDC->MoveTo(nXpos1, nYpos + 4);
			pDC->LineTo(nXpos2, nYpos + nHeight + 4);
			pDC->MoveTo(nXpos1, nYpos + 6);
			pDC->LineTo(nXpos2, nYpos + nHeight + 6);
		} // for the number of bands
	} // if indeterminate
	else
	{
		if (nHeight > 3)
		{
			pDC->MoveTo(nLeft, nTop + 1);
			pDC->LineTo(nLeft, nTop);
			pDC->LineTo(nRight, nTop);
			pDC->MoveTo(nLeft + 1, nBottom - 2);
			pDC->LineTo(nLeft + 1, nTop + 1);
			pDC->MoveTo(nRight - 3, nBottom - 3);
			pDC->LineTo(nRight - 3, nTop + 1);
			pDC->SetPixel(nRight - 2, nTop + 1, m_crColor);

			pDC->SelectObject(&m_penColorLight);
			pDC->MoveTo(nLeft + 2, nBottom - 3);
			pDC->LineTo(nLeft + 2, nTop + 1);
			pDC->MoveTo(nRight - 4, nBottom - 3);
			pDC->LineTo(nRight - 4, nTop + 1);
			pDC->SetPixel(nLeft + 1, nTop + 1, m_crColorLight);
			pDC->SetPixel(nRight - 3, nTop + 1, m_crColorLight);

			pDC->SelectObject(&m_penColorLighter);
			pDC->MoveTo(nLeft + 3, nBottom - 3);
			pDC->LineTo(nLeft + 3, nTop + 1);
			pDC->MoveTo(nRight - 5, nBottom - 3);
			pDC->LineTo(nRight - 5, nTop + 1);
			pDC->SetPixel(nLeft + 2, nTop + 1, m_crColorLighter);
			pDC->SetPixel(nRight - 4, nTop + 1, m_crColorLighter);

			pDC->SelectObject(&m_penColorDark);
			pDC->MoveTo(nLeft, nBottom - 1);
			pDC->LineTo(nLeft, nTop + 1);
			pDC->MoveTo(nLeft + 2, nBottom - 2);
			pDC->LineTo(nRight - 2, nBottom - 2);
			pDC->LineTo(nRight - 2, nTop + 1);
			pDC->SetPixel(nRight - 1, nTop + 1, m_crColorDark);

			pDC->SelectObject(&m_penColorDarker);
			pDC->MoveTo(nLeft + 1, nBottom - 1);
			pDC->LineTo(nRight - 1, nBottom - 1);
			pDC->LineTo(nRight - 1, nTop + 1);
		}
		else
		{
			CBrush br(m_crColor);
			CBrush* pOldBrush = pDC->SelectObject(&br);
			pDC->SelectObject(&m_penColorDark);
			pDC->Rectangle(rect);
			pDC->SelectObject(pOldBrush);
		}
	} // if not indeterminate

	pDC->SelectObject(pOldPen);
} // DrawVerticalBar

//-------------------------------------------------------------------
//
BOOL CMacProgressCtrl::OnEraseBkgnd(CDC* pDC)
//
// Return Value:	Nonzero if it erases the background; otherwise 0.
//
// Parameters	:	pDC - Specifies the device-context object.
//
// Remarks		:	The framework calls this member function when the
//						CWnd object background needs erasing (for example,
//						when resized). It is called to prepare an invalidated
//						region for painting.
//
{
	return TRUE;
} // OnEraseBkgnd

//-------------------------------------------------------------------
//
void CMacProgressCtrl::GetColors()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Calculates the lighter and darker colors, as well as
//						the shadow colors.
//
{
	m_crColorLight           = LightenColor(m_crColor, 51);
	m_crColorLighter         = LightenColor(m_crColorLight, 51);
	m_crColorLightest        = LightenColor(m_crColorLighter, 51);
	m_crColorDark            = DarkenColor(m_crColor, 51);
	m_crColorDarker          = DarkenColor(m_crColorDark, 51);
	m_crDkShadow             = ::GetSysColor(COLOR_3DDKSHADOW);
	m_crLiteShadow           = ::GetSysColor(COLOR_3DSHADOW);

	// Get a color halfway between COLOR_3DDKSHADOW and COLOR_3DSHADOW
	BYTE byRed3DDkShadow     = GetRValue(m_crDkShadow);
	BYTE byRed3DLiteShadow   = GetRValue(m_crLiteShadow);
	BYTE byGreen3DDkShadow   = GetGValue(m_crDkShadow);
	BYTE byGreen3DLiteShadow = GetGValue(m_crLiteShadow);
	BYTE byBlue3DDkShadow    = GetBValue(m_crDkShadow);
	BYTE byBlue3DLiteShadow  = GetBValue(m_crLiteShadow);

	m_crShadow               = RGB(byRed3DLiteShadow + ((byRed3DDkShadow - byRed3DLiteShadow) >> 1),
					  byGreen3DLiteShadow + ((byGreen3DDkShadow - byGreen3DLiteShadow) >> 1),
					  byBlue3DLiteShadow + ((byBlue3DDkShadow - byBlue3DLiteShadow) >> 1));
} // GetColors

//-------------------------------------------------------------------
//
void CMacProgressCtrl::SetColor(COLORREF crColor)
//
// Return Value:	None.
//
// Parameters	:	crColor - New color.
//
// Remarks		:	Sets the progress	bar control's color. The lighter
//						darker colors are recalculated, and the pens recreated.
//
{
	m_crColor = crColor;
	GetColors();
	CreatePens();
	RedrawWindow();
} // SetColor

//-------------------------------------------------------------------
//
COLORREF CMacProgressCtrl::GetColor()
//
// Return Value:	The current color.
//
// Parameters	:	None.
//
// Remarks		:	Returns the progress bar control's current color.
//
{
	return m_crColor;
} // GetColor

//-------------------------------------------------------------------
//
void CMacProgressCtrl::CreatePens()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Deletes the pen objects, if necessary, and creates them.
//
{
	DeletePens();

	m_penColorLight.CreatePen(PS_SOLID, 1, m_crColorLight);
	m_penColorLighter.CreatePen(PS_SOLID, 1, m_crColorLighter);
	m_penColor.CreatePen(PS_SOLID, 1, m_crColor);
	m_penColorDark.CreatePen(PS_SOLID, 1, m_crColorDark);
	m_penColorDarker.CreatePen(PS_SOLID, 1, m_crColorDarker);
	m_penDkShadow.CreatePen(PS_SOLID, 1, m_crDkShadow);
	m_penShadow.CreatePen(PS_SOLID, 1, m_crShadow);
	m_penLiteShadow.CreatePen(PS_SOLID, 1, m_crLiteShadow);
} // CreatePens

//-------------------------------------------------------------------
//
void CMacProgressCtrl::DeletePens()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Deletes the pen objects.
//
{
	if (m_penColorLight.m_hObject)
		m_penColorLight.DeleteObject();
	if (m_penColorLighter.m_hObject)
		m_penColorLighter.DeleteObject();
	if (m_penColor.m_hObject)
		m_penColor.DeleteObject();
	if (m_penColorDark.m_hObject)
		m_penColorDark.DeleteObject();
	if (m_penColorDarker.m_hObject)
		m_penColorDarker.DeleteObject();
	if (m_penDkShadow.m_hObject)
		m_penDkShadow.DeleteObject();
	if (m_penShadow.m_hObject)
		m_penShadow.DeleteObject();
	if (m_penLiteShadow.m_hObject)
		m_penLiteShadow.DeleteObject();
} // DeletePens

//-------------------------------------------------------------------
//
void CMacProgressCtrl::SetIndeterminate(BOOL bIndeterminate)
//
// Return Value:	None.
//
// Parameters	:	bIndeterminate - Specifies the indeterminate state.
//
// Remarks		:	Sets the indeterminate flag.
//
{
	m_bIndeterminate = bIndeterminate;

	if (m_bIndeterminate)
	{
		CRect rect;
		GetClientRect(rect);
		m_nIndOffset = 0;

		RedrawWindow();
		SetTimer(IDT_INDETERMINATE, 25, nullptr);
	}
	else
	{
		KillTimer(IDT_INDETERMINATE);
		RedrawWindow();
	}
} // SetIndeterminate

//-------------------------------------------------------------------
//
BOOL CMacProgressCtrl::GetIndeterminate()
//
// Return Value:	m_bIndeterminate.
//
// Parameters	:	None.
//
// Remarks		:	Returns m_bIndeterminate.
//
{
	return m_bIndeterminate;
} // GetIndeterminate

//-------------------------------------------------------------------
//
void CMacProgressCtrl::OnTimer(UINT_PTR nIDEvent)
//
// Return Value:	None.
//
// Parameters	:	nIDEvent - Specifies the identifier of the timer.
//
// Remarks		:	The framework calls this member function after each
//						interval specified in the SetTimer member function used
//						to install a timer.
//
{
	// Increment the indeterminate bar offset and redraw the window.
	if (nIDEvent == IDT_INDETERMINATE)
	{
		KillTimer(nIDEvent);

		if (++m_nIndOffset > IND_BAND_WIDTH - 1)
			m_nIndOffset = 0;
		RedrawWindow();

		SetTimer(IDT_INDETERMINATE, 25, nullptr);
	}
} // OnTimer
