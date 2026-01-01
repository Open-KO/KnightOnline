// MacSliderCtrl.cpp : implementation file
//
//	CMacSliderCtrl class, version 1.0
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
#include "MacSliderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Funtion prototypes.
COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal);
COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal);

//-------------------------------------------------------------------
//
COLORREF CMacSliderCtrl::LightenColor(const COLORREF crColor, BYTE byIncreaseVal)
//
// Return Value:	None.
//
// Parameters	:	crColor - References a COLORREF structure.
//						byReduceVal - The amount to reduce the RGB values by.
//
// Remarks		:	Lightens a color by increasing the RGB values by the given number.
//
{
	BYTE byRed = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue = GetBValue(crColor);

	if ((byRed + byIncreaseVal) <= 255)
		byRed = BYTE(byRed + byIncreaseVal);
	if ((byGreen + byIncreaseVal)	<= 255)
		byGreen = BYTE(byGreen + byIncreaseVal);
	if ((byBlue + byIncreaseVal) <= 255)
		byBlue = BYTE(byBlue + byIncreaseVal);

	return RGB(byRed, byGreen, byBlue);
}	// LightenColorref

//-------------------------------------------------------------------
//
COLORREF CMacSliderCtrl::DarkenColor(const COLORREF crColor, BYTE byReduceVal)
//
// Return Value:	None.
//
// Parameters	:	crColor - References a COLORREF structure.
//						byReduceVal - The amount to reduce the RGB values by.
//
// Remarks		:	Darkens a color by reducing the RGB values by the given number.
//
{
	BYTE byRed = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue = GetBValue(crColor);

	if (byRed >= byReduceVal)
		byRed = BYTE(byRed - byReduceVal);
	if (byGreen >= byReduceVal)
		byGreen = BYTE(byGreen - byReduceVal);
	if (byBlue >= byReduceVal)
		byBlue = BYTE(byBlue - byReduceVal);

	return RGB(byRed, byGreen, byBlue);
}	// DarkenColorref

/////////////////////////////////////////////////////////////////////////////
// CMacSliderCtrl

//-------------------------------------------------------------------
//
CMacSliderCtrl::CMacSliderCtrl()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Standard constructor.
//
{
	m_crThumb = ::GetSysColor(COLOR_3DFACE);
	m_crChannel = ::GetSysColor(COLOR_WINDOW);
	m_crSelection = ::GetSysColor(COLOR_HIGHLIGHT);
	GetColors();
	CreatePens();
}	// CMacSliderCtrl

//-------------------------------------------------------------------
//
CMacSliderCtrl::~CMacSliderCtrl()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Destructor.
//
{
	DeletePens();
}	// ~CMacSliderCtrl


BEGIN_MESSAGE_MAP(CMacSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CMacSliderCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacSliderCtrl message handlers

//-------------------------------------------------------------------
//
void CMacSliderCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Return Value:	None.
//
// Parameters	:	pNMHDR - Used to get a pointer to a NM_CUSTOMDRAW structure.
//						pResult - Will receive a value which depends on the current
//							drawing stage.
//
// Remarks		:	Sent by the slider control to notify the parent window 
//						about drawing operations. This notification is sent in 
//						the form of a WM_NOTIFY message.
//
{
	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;

	// CDDS_PREPAINT is at the beginning of the paint cycle. You 
	// implement custom draw by returning the proper value. In this 
	// case, we're requesting item-specific notifications.
	if (lpcd->dwDrawStage == CDDS_PREPAINT)
	{
		// Request prepaint notifications for each item.
		*pResult = CDRF_NOTIFYITEMDRAW;         
		return;
	}

	
	
	
	
	
	

	
	
	
	// Because we returned CDRF_NOTIFYITEMDRAW in response to
	// CDDS_PREPAINT, CDDS_ITEMPREPAINT is sent when the control is
	// about to paint an item.
	if (lpcd->dwDrawStage == CDDS_ITEMPREPAINT)
	{	
		CDC *pDC = CDC::FromHandle(lpcd->hdc);
		CRect rect(lpcd->rc);
		int nSavedDC = pDC->SaveDC();

		if (lpcd->dwItemSpec == TBCD_TICS)
		{
			*pResult = CDRF_DODEFAULT;
			return;
		}	// if drawing tics
		else if (lpcd->dwItemSpec ==  TBCD_THUMB)
		{
			// If the slider has been clicked on (selected) darken
			// the thumb. Save the thumb color, in case it is changed.
			COLORREF crSavedThumb = m_crThumb;

			if (lpcd->uItemState & CDIS_SELECTED)
			{
				m_crThumb = DarkenColor(m_crThumb, 51);
				GetColors();
				CreatePens();
			}

			if (rect.Height() > rect.Width())
				DrawHorizontalThumb(pDC, rect);
			else
				DrawVerticalThumb(pDC, rect);

			// Restore the thumb color.
			if (lpcd->uItemState & CDIS_SELECTED)
			{
				m_crThumb = crSavedThumb;
				GetColors();
				CreatePens();
			}
		}	// if drawing thumb
		else if (lpcd->dwItemSpec == TBCD_CHANNEL)
		{
			// If the slider is disabled, change the thumb
			// to the current face color.
			COLORREF crSavedChannel = m_crChannel;
			if (!IsWindowEnabled())
			{
				m_crChannel = ::GetSysColor(COLOR_3DFACE);
			}

			CRect rcThumb;
			GetThumbRect(rcThumb);
			DrawChannel(pDC, rect, rcThumb.Height() > rcThumb.Width());

			m_crChannel = crSavedChannel;





			CRect rcTmp;
			this->CalcRectToFill(rect, rcTmp, m_AnimData.fFrmPlugTraceStart, m_AnimData.fFrmPlugTraceEnd);	pDC->FillSolidRect(&rcTmp, 0x00ff0000);

			this->CalcRectToFill(rect, rcTmp, m_AnimData.fFrmSound0);			pDC->FillSolidRect(&rcTmp, 0x0000ff00);
			this->CalcRectToFill(rect, rcTmp, m_AnimData.fFrmSound1);			pDC->FillSolidRect(&rcTmp, 0x0000ff00);

			this->CalcRectToFill(rect, rcTmp, m_AnimData.fFrmStrike0); 			pDC->FillSolidRect(&rcTmp, 0x000000ff);
			this->CalcRectToFill(rect, rcTmp, m_AnimData.fFrmStrike1); 			pDC->FillSolidRect(&rcTmp, 0x000000ff);


		}	// if drawing channel

		pDC->RestoreDC(nSavedDC);
	
		*pResult = CDRF_SKIPDEFAULT;
		return;
	}

	*pResult = 0;
}	// OnCustomDraw

//-------------------------------------------------------------------
//
void CMacSliderCtrl::DrawHorizontalThumb(CDC *pDC, const CRect &rect)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies a device context.
//						rect - The rectangle for the thumb.
//
// Remarks		:	Draws a thumb for a horizontal slider control.
//
{
	CRect rc(rect);
	int nWidth = rect.Width();
	int nMid = nWidth >> 1;

	CPen penFrame(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWFRAME));
	CPen *pOldPen = pDC->SelectObject(&penFrame);	
	CBrush br(m_crThumb);
	CBrush *pOldBrush = pDC->SelectObject(&br);

	DWORD dwStyle = GetStyle();

	// No points on thumb.
	if (dwStyle & TBS_BOTH)
	{
		pDC->RoundRect(rc, CPoint(2, 2));
		rc.DeflateRect(1, 1, 1, 1);
		pDC->Draw3dRect(rc, m_crThumbLight, m_crThumbDark);

		pDC->SetPixel(rc.left, rc.top, m_crThumbLighter);
		pDC->SetPixel(rc.right - 1, rc.top, m_crThumb);
		pDC->SetPixel(rc.right - 1, rc.bottom - 1, m_crThumbDarker);
		pDC->SetPixel(rc.left, rc.bottom - 1, m_crThumb);
	}
	// Point on top.
	else if (dwStyle & TBS_TOP)
	{
		pDC->MoveTo(rc.left, rc.top + nMid - 1);
		pDC->LineTo(rc.left, rc.bottom - 2);
		pDC->LineTo(rc.left + 1, rc.bottom - 1);
		pDC->LineTo(rc.right - 2, rc.bottom - 1);
		pDC->LineTo(rc.right - 1, rc.bottom - 2);
		pDC->LineTo(rc.right - 1, rc.top + nMid - 1);
		pDC->LineTo(rc.right - nMid, rc.top);
		pDC->LineTo(rc.left + nMid - 1, rc.top);
		pDC->LineTo(rc.left, rc.top + nMid - 1);

		pDC->FloodFill(rc.left + nMid, rc.top + nMid, ::GetSysColor(COLOR_WINDOWFRAME));

		if (nWidth > 5)
		{
			pDC->SelectObject(&m_penThumbLight);
			pDC->MoveTo(rc.left + 1, rc.top + nMid);
			pDC->LineTo(rc.left + 1, rc.bottom - 2);
			pDC->MoveTo(rc.right - nMid, rc.top + 1);
			pDC->LineTo(rc.right - nMid - 2, rc.top + 1);

			pDC->SelectObject(&m_penThumbLighter);
			pDC->MoveTo(rc.left + 1, rc.top + nMid - 1);
			pDC->LineTo(rc.left + nMid, rc.top);


			pDC->SelectObject(&m_penThumbDark);
			pDC->MoveTo(rc.left + 2, rc.bottom - 2);
			pDC->LineTo(rc.right - 2, rc.bottom - 2);
			pDC->LineTo(rc.right - 2, rc.bottom - 2);
			pDC->LineTo(rc.right - 2, rc.top + nMid - 1);
			pDC->LineTo(rc.right - nMid, rc.top + 1);
		}

		rc.DeflateRect(1, 2, 1, 0);
	}	// if point at top of thumb
	// Point is on bottom.
	else
	{
		pDC->MoveTo(rc.left, rc.top + 1);
		pDC->LineTo(rc.left, rc.bottom - nMid);
		pDC->LineTo(rc.left + nMid - 1, rc.bottom - 1);
		pDC->LineTo(rc.right - nMid, rc.bottom - 1);
		pDC->LineTo(rc.right - 1, rc.bottom - nMid);
		pDC->LineTo(rc.right - 1, rc.top + 1);
		pDC->LineTo(rc.right - 2, rc.top);
		pDC->LineTo(rc.left, rc.top);

		pDC->FloodFill(rc.left + nMid, rc.top + nMid, ::GetSysColor(COLOR_WINDOWFRAME));

		if (nWidth > 5)
		{
			pDC->SetPixel(rc.left + 1, rc.top + 1, m_crThumbLighter);

			pDC->SelectObject(&m_penThumbLight);
			pDC->MoveTo(rc.left + 1, rc.top + 2);
			pDC->LineTo(rc.left + 1, rc.bottom - nMid + 1);
			pDC->MoveTo(rc.left + 2, rc.top + 1);
			pDC->LineTo(rc.right - 2, rc.top + 1);

			pDC->SelectObject(&m_penThumbDark);
			pDC->MoveTo(rc.left + 2, rc.bottom - nMid + 1);
			pDC->LineTo(rc.left + nMid - 1, rc.bottom - 2);
			pDC->LineTo(rc.right - nMid, rc.bottom - 2);
			pDC->MoveTo(rc.right - 2, rc.bottom - nMid - 1);
			pDC->LineTo(rc.right - 2, rc.top + 1);

			pDC->SelectObject(&m_penThumbDarker);
			pDC->MoveTo(rc.right - nMid, rc.bottom - 2);
			pDC->LineTo(rc.right - 1, rc.bottom - nMid - 1);
		}

		rc.DeflateRect(1, 0, 1, 2);
	}	// if point at bottom of thumb

	// Only draw the gripper if the slider is enabled.
	if (IsWindowEnabled())
	{
		if (nWidth > 5)
		{
			pDC->SelectObject(&m_penThumbLight);
			pDC->MoveTo(rc.left + 1, rc.top + 5);
			pDC->LineTo(rc.left + 1, rc.bottom - 5);
			pDC->MoveTo(rc.left + 3, rc.top + 5);
			pDC->LineTo(rc.left + 3, rc.bottom - 5);
			if (nWidth > 9)
			{
				pDC->MoveTo(rc.left + 5, rc.top + 5);
				pDC->LineTo(rc.left + 5, rc.bottom - 5);
				pDC->MoveTo(rc.left + 7, rc.top + 5);
				pDC->LineTo(rc.left + 7, rc.bottom - 5);
			}
		}

		if (nWidth > 5)
		{
			pDC->SelectObject(&m_penThumbDarker);
			pDC->MoveTo(rc.left + 2, rc.top + 5);
			pDC->LineTo(rc.left + 2, rc.bottom - 4);
			pDC->MoveTo(rc.left + 4, rc.top + 5);
			pDC->LineTo(rc.left + 4, rc.bottom - 4);
			if (nWidth > 9)
			{
				pDC->MoveTo(rc.left + 6, rc.top + 5);
				pDC->LineTo(rc.left + 6, rc.bottom - 4);
			}
		}

		if (nWidth > 5)
		{
			pDC->SetPixel(rc.left + 1, rc.top + 4, m_crThumbLighter);
			pDC->SetPixel(rc.left + 3, rc.top + 4, m_crThumbLighter);
			if (nWidth > 9)
			{
				pDC->SetPixel(rc.left + 5, rc.top + 4, m_crThumbLighter);
				pDC->SetPixel(rc.left + 7, rc.top + 4, m_crThumbLighter);
			}
		}
	}	// if (IsWindowEnabled())

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}	// DrawHorizontalThumb

//-------------------------------------------------------------------
//
void CMacSliderCtrl::DrawVerticalThumb(CDC *pDC, const CRect &rect)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies a device context.
//						rect - The rectangle for the thumb.
//
// Remarks		:	Draws a thumb for a vertical slider control.
//
{
	CRect rc(rect);
	int nHeight = rect.Height();
	int nMid = nHeight >> 1;

	CPen penFrame(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWFRAME));
	CPen *pOldPen = pDC->SelectObject(&penFrame);	
	CBrush br(m_crThumb);
	CBrush *pOldBrush = pDC->SelectObject(&br);

	DWORD dwStyle = GetStyle();

	// No points on the thumb.
	if (dwStyle & TBS_BOTH)
	{
		pDC->RoundRect(rc, CPoint(2, 2));
		rc.DeflateRect(1, 1, 1, 1);
		pDC->Draw3dRect(rc, m_crThumbLight, m_crThumbDark);

		pDC->SetPixel(rc.left, rc.top, m_crThumbLighter);
		pDC->SetPixel(rc.right - 1, rc.top, m_crThumb);
		pDC->SetPixel(rc.right - 1, rc.bottom - 1, m_crThumbDarker);
		pDC->SetPixel(rc.left, rc.bottom - 1, m_crThumb);
	}
	// Point is on left side.
	else if (dwStyle & TBS_TOP)
	{
		pDC->MoveTo(rc.left + nMid - 1, rc.top);
		pDC->LineTo(rc.left, rc.top + nMid - 1);
		pDC->LineTo(rc.left, rc.bottom - nMid);
		pDC->LineTo(rc.left + nMid - 1, rc.bottom - 1);
		pDC->LineTo(rc.right - 2, rc.bottom - 1);
		pDC->LineTo(rc.right - 1, rc.bottom - 2);
		pDC->LineTo(rc.right - 1, rc.top + 1);
		pDC->LineTo(rc.right - 2, rc.top);
		pDC->LineTo(rc.left + nMid - 2, rc.top);

		pDC->FloodFill(rc.left + nMid, rc.top + nMid, ::GetSysColor(COLOR_WINDOWFRAME));

		pDC->SelectObject(&m_penThumbLighter);
		pDC->MoveTo(rc.left + 1, rc.top + nMid - 1);
		pDC->LineTo(rc.left + nMid, rc.top);

		pDC->SelectObject(&m_penThumbLight);
		pDC->MoveTo(rc.left + 1, rc.top + nMid + 1);
		pDC->LineTo(rc.left + 1, rc.top + nMid - 1);
		pDC->MoveTo(rc.left + nMid, rc.top + 1);
		pDC->LineTo(rc.right - 2, rc.top + 1);

		pDC->SelectObject(&m_penThumbDark);
		pDC->MoveTo(rc.left + 2, rc.bottom - nMid + 1);
		pDC->LineTo(rc.left + nMid - 1, rc.bottom - 2);
		pDC->LineTo(rc.right - 2, rc.bottom - 2);
		pDC->LineTo(rc.right - 2, rc.top + 1);

		rc.DeflateRect(2, 1, 0, 0);
	}	// if point on left side
	// Point is on right side.
	else
	{
		pDC->MoveTo(rc.left, rc.top + 1);
		pDC->LineTo(rc.left, rc.bottom - 2);
		pDC->LineTo(rc.left + 1, rc.bottom - 1);
		pDC->LineTo(rc.right - nMid, rc.bottom - 1);
		pDC->LineTo(rc.right - 1, rc.bottom - nMid);
		pDC->LineTo(rc.right - 1, rc.top + nMid - 1);
		pDC->LineTo(rc.right - nMid, rc.top);
		pDC->LineTo(rc.left, rc.top);

		pDC->FloodFill(rc.left + nMid, rc.top + nMid, ::GetSysColor(COLOR_WINDOWFRAME));

		pDC->SelectObject(&m_penThumbLight);
		pDC->MoveTo(rc.left + 1, rc.bottom - 2);
		pDC->LineTo(rc.left + 1, rc.top + 1);
		pDC->LineTo(rc.right - nMid + 1, rc.top + 1);

		pDC->SetPixel(rc.left + 1, rc.top + 1, m_crThumbLighter);

		pDC->SelectObject(&m_penThumbDark);
		pDC->MoveTo(rc.left + 2, rc.bottom - 2);
		pDC->LineTo(rc.right - nMid, rc.bottom - 2);
		pDC->MoveTo(rc.right - 2, rc.top + nMid);
		pDC->LineTo(rc.right - 2, rc.top + nMid - 1);
		pDC->LineTo(rc.right - nMid, rc.top + 1);

		pDC->SelectObject(&m_penThumbDarker);
		pDC->MoveTo(rc.right - nMid, rc.bottom - 2);
		pDC->LineTo(rc.right - 1, rc.bottom - nMid - 1);

		rc.DeflateRect(0, 1, 1, 0);
	}	// if point is on right side

	// Only draw the gripper if the slider is enabled.
	if (IsWindowEnabled())
	{
		if (nHeight > 5)
		{
			pDC->SelectObject(&m_penThumbLight);
			pDC->MoveTo(rc.left + 5, rc.top + 1);
			pDC->LineTo(rc.right - 5, rc.top + 1);
			pDC->MoveTo(rc.left + 5, rc.top + 3);
			pDC->LineTo(rc.right - 5, rc.top + 3);
			if (nHeight > 9)
			{
				pDC->MoveTo(rc.left + 5, rc.top + 5);
				pDC->LineTo(rc.right - 5, rc.top + 5);
				pDC->MoveTo(rc.left + 5, rc.top + 7);
				pDC->LineTo(rc.right - 5, rc.top + 7);
			}
		}

		if (nHeight > 5)
		{
			pDC->SelectObject(&m_penThumbDarker);
			pDC->MoveTo(rc.left + 5, rc.top + 2);
			pDC->LineTo(rc.right - 4, rc.top + 2);
			pDC->MoveTo(rc.left + 5, rc.top + 4);
			pDC->LineTo(rc.right - 4, rc.top + 4);
			if (nHeight > 9)
			{
				pDC->MoveTo(rc.left + 5, rc.top + 6);
				pDC->LineTo(rc.right - 4, rc.top + 6);
			}
		}

		if (nHeight > 5)
		{
			pDC->SetPixel(rc.left + 4, rc.top + 1, m_crThumbLighter);
			pDC->SetPixel(rc.left + 4, rc.top + 3, m_crThumbLighter);
			if (nHeight > 9)
			{
				pDC->SetPixel(rc.left + 4, rc.top + 5, m_crThumbLighter);
				pDC->SetPixel(rc.left + 4, rc.top + 7, m_crThumbLighter);
			}
		}
	}	// if (IsWindowEnabled())

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}	// DrawVerticalThumb

//-------------------------------------------------------------------
//
void CMacSliderCtrl::DrawChannel(CDC *pDC, const CRect &rect, BOOL bHorz)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies a device context.
//						rect - The rectangle for the channel.
//						bHorz - TRUE if the slider is horizontal; FALSE otherwise.
//
// Remarks		:	Draws the slider's channel and selection, if any.
//
{
	CRect rc(rect);

	// If the slider has a selection, shrink the rect a little.
	int nRangeMin = GetRangeMin();
	int nRangeMax = GetRangeMax();
	if (nRangeMin || nRangeMax)
		rc.InflateRect(0, -1, 0, -1);

	// Another minor adjustment for the rect.
	if (bHorz)
		rc.InflateRect(-1, 0, -1, 1);
	else
		rc.InflateRect(0, -1, 0, -1);

	CPen *pOldPen = pDC->SelectObject(&m_penDkShadow);
	CBrush br(m_crChannel);
	CBrush *pOldBrush = pDC->SelectObject(&br);

	// If the height or width is > 3, draw the inner round rect.
	if ((bHorz && rc.Height() > 3) || (!bHorz && rc.Width() > 3))
		pDC->RoundRect(rc, CPoint(2, 2));

	rc.InflateRect(1, 1, 1, 0);

	// Give it a sunken appearance.
	pDC->SelectObject(&m_penShadow);
	pDC->MoveTo(rc.left, rc.bottom - 2);
	pDC->LineTo(rc.left, rc.top + 2);
	pDC->LineTo(rc.left + 2, rc.top);
	pDC->LineTo(rc.right - 2, rc.top);

	pDC->SelectObject(&m_penHilight);
	pDC->MoveTo(rc.left + 1, rc.bottom - 1);
	pDC->LineTo(rc.left + 2, rc.bottom);
	pDC->LineTo(rc.right - 2, rc.bottom);
	pDC->LineTo(rc.right - 2, rc.bottom - 1);
	pDC->LineTo(rc.right - 1, rc.bottom - 1);
	pDC->LineTo(rc.right - 1, rc.top + 2);
	pDC->LineTo(rc.right - 3, rc.top);

	// Draw the selection, if there is one.
	int nSelMin, nSelMax;
	GetSelection(nSelMin, nSelMax);
	if (nSelMin < nSelMax)
	{
		CRect rcSel(rect);
		int nAdjust = (bHorz ? rect.Width() : rect.Height());
		int nStart = int(((float)nAdjust * float(nSelMin - nRangeMin)) / float(nRangeMax - nRangeMin));
		int nEnd = int(((float)nAdjust * float(nSelMax - nRangeMin)) / float(nRangeMax - nRangeMin));

		if (bHorz)
		{
			rcSel.left += nStart;
			rcSel.right = rcSel.left + (nEnd - nStart);
		}
		else
		{
			rcSel.top += nStart;
			rcSel.bottom = rcSel.top + (nEnd - nStart);
		}

		if (bHorz)
			rcSel.DeflateRect(2, 2, 2, 1);
		else
			rcSel.DeflateRect(1, 2, 1, 2);

		CBrush br2(m_crSelection);
		pDC->FillRect(rcSel, &br2);
	}

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}	// DrawChannel

//-------------------------------------------------------------------
//
void CMacSliderCtrl::GetColors()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Calculates the lighter and darker colors, as well as 
//						the shadow colors.
//
{
	m_crThumbLight = LightenColor(m_crThumb, 51);
	m_crThumbLighter = LightenColor(m_crThumbLight, 51);
	m_crThumbDark = DarkenColor(m_crThumb, 51);
	m_crThumbDarker = DarkenColor(m_crThumbDark, 51);

	m_crHilight = ::GetSysColor(COLOR_3DHILIGHT);

	// Get a color halfway between COLOR_3DSHADOW AND COLOR_3DFACE,
	// and COLOR_3DDKSHADOW and COLOR_3DSHADOW.
	COLORREF crFace = ::GetSysColor(COLOR_3DFACE);
	COLORREF crShadow = ::GetSysColor(COLOR_3DSHADOW);
	COLORREF crDkShadow = ::GetSysColor(COLOR_3DDKSHADOW);
	BYTE byRedFace = GetRValue(crFace);
	BYTE byRedShadow = GetRValue(crShadow);
	BYTE byRedDkShadow = GetRValue(crDkShadow);
	BYTE byGreenFace = GetGValue(crFace);
	BYTE byGreenShadow = GetGValue(crShadow);
	BYTE byGreenDkShadow = GetGValue(crDkShadow);
	BYTE byBlueFace = GetBValue(crFace);
	BYTE byBlueShadow = GetBValue(crShadow);
	BYTE byBlueDkShadow= GetBValue(crDkShadow);

	m_crShadow = RGB(byRedFace + ((byRedShadow - byRedFace) >> 1),
							byGreenFace + ((byGreenShadow - byGreenFace) >> 1),
							byBlueFace + ((byBlueShadow - byBlueFace) >> 1));

	m_crDkShadow = RGB(byRedShadow + ((byRedDkShadow - byRedShadow) >> 1),
							byGreenShadow + ((byGreenDkShadow - byGreenShadow) >> 1),
							byBlueShadow + ((byBlueDkShadow - byBlueShadow) >> 1));
}	// GetColors

//-------------------------------------------------------------------
//
void CMacSliderCtrl::CreatePens()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Deletes the pen objects, if necessary, and creates them.
//
{
	DeletePens();

	m_penThumbLight.CreatePen(PS_SOLID, 1, m_crThumbLight);
	m_penThumbLighter.CreatePen(PS_SOLID, 1, m_crThumbLighter);
	m_penThumb.CreatePen(PS_SOLID, 1, m_crThumb);
	m_penThumbDark.CreatePen(PS_SOLID, 1, m_crThumbDark);
	m_penThumbDarker.CreatePen(PS_SOLID, 1, m_crThumbDarker);
	m_penDkShadow.CreatePen(PS_SOLID, 1, m_crDkShadow);
	m_penShadow.CreatePen(PS_SOLID, 1, m_crShadow);
	m_penHilight.CreatePen(PS_SOLID, 1, m_crHilight);
}	// CreatePens

//-------------------------------------------------------------------
//
void CMacSliderCtrl::DeletePens()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Deletes the pen objects.
//
{
	if (m_penThumbLight.m_hObject)
		m_penThumbLight.DeleteObject();
	if (m_penThumbLighter.m_hObject)
		m_penThumbLighter.DeleteObject();
	if (m_penThumb.m_hObject)
		m_penThumb.DeleteObject();
	if (m_penThumbDark.m_hObject)
		m_penThumbDark.DeleteObject();
	if (m_penThumbDarker.m_hObject)
		m_penThumbDarker.DeleteObject();
	if (m_penDkShadow.m_hObject)
		m_penDkShadow.DeleteObject();
	if (m_penShadow.m_hObject)
		m_penShadow.DeleteObject();
	if (m_penHilight.m_hObject)
		m_penHilight.DeleteObject();
}	// DeletePens

//-------------------------------------------------------------------
//
void CMacSliderCtrl::SetChannelColor(COLORREF crColor)
//
// Return Value:	None.
//
// Parameters	:	crColor - The new channel color.
//
// Remarks		:	Sets the channel color.
//
{
	m_crChannel = crColor;
	GetColors();
	CreatePens();
}	// SetChannelColor

//-------------------------------------------------------------------
//
COLORREF CMacSliderCtrl::GetChannelColor()
//
// Return Value:	The current channel color.
//
// Parameters	:	None.
//
// Remarks		:	Returns the current channel color.
//
{
	return m_crChannel;
}	// GetChannelColor

//-------------------------------------------------------------------
//
void CMacSliderCtrl::SetThumbColor(COLORREF crColor)
//
// Return Value:	None.
//
// Parameters	:	crColor - The new thumb color.
//
// Remarks		:	Sets the channel color.
//
{
	m_crThumb = crColor;
	GetColors();
	CreatePens();
	RedrawWindow();
}	// SetThumbColor

//-------------------------------------------------------------------
//
COLORREF CMacSliderCtrl::GetThumbColor()
//
// Return Value:	The current thumb color.
//
// Parameters	:	None.
//
// Remarks		:	Returns the current thumb color.
//
{
	return m_crThumb;
}	// GetThumbColor

//-------------------------------------------------------------------
//
void CMacSliderCtrl::SetSelectionColor(COLORREF crColor)
//
// Return Value:	None.
//
// Parameters	:	crColor - The new selection color.
//
// Remarks		:	Sets the	selection color.
//
{
	m_crSelection = crColor;
	GetColors();
	CreatePens();
	RedrawWindow();
}	// SetSelectionColor

//-------------------------------------------------------------------
//
COLORREF CMacSliderCtrl::GetSelectionColor()
//
// Return Value:	The current selection color.
//
// Parameters	:	None.
//
// Remarks		:	Returns the current selection color.
//
{
	return m_crSelection;
}	// GetSelectionColor







void CMacSliderCtrl::SetAnimationData(const __AnimData& Data)
{
	m_AnimData = Data;
	this->SetRange((int)(Data.fFrmStart / FRM_PRECISION), (int)(Data.fFrmEnd/FRM_PRECISION));
	this->Invalidate();
}

void CMacSliderCtrl::CalcRectToFill(const RECT& rcWhole, RECT& rc, float fValue)
{
	int iW = rcWhole.right - rcWhole.left;
	float fTmp = (fValue - m_AnimData.fFrmStart) / (m_AnimData.fFrmEnd - m_AnimData.fFrmStart);
	rc.left		= rcWhole.left + (int)(iW * fTmp) - 2;
	rc.right	= rcWhole.left + (int)(iW * fTmp) + 2;
	rc.top		= rcWhole.top - 2;
	rc.bottom	= rcWhole.bottom + 2;
}

void CMacSliderCtrl::CalcRectToFill(const RECT& rcWhole, RECT& rc, float fValueStart, float fValueEnd)
{
	int iW = rcWhole.right - rcWhole.left;
	float fTmp0 = (fValueStart - m_AnimData.fFrmStart) / (m_AnimData.fFrmEnd - m_AnimData.fFrmStart);
	float fTmp1 = (fValueEnd - m_AnimData.fFrmStart) / (m_AnimData.fFrmEnd - m_AnimData.fFrmStart);
	rc.left		= rcWhole.left + (int)(iW * fTmp0) - 1;
	rc.right	= rcWhole.left + (int)(iW * fTmp1) + 1;
	rc.top		= rcWhole.top - 1;
	rc.bottom	= rcWhole.bottom + 1;
}

float CMacSliderCtrl::GetCurrentFrame()
{
	float fFrm = CSliderCtrl::GetPos() * FRM_PRECISION;
	return fFrm;
}

void CMacSliderCtrl::SetCurrentFrame(float fFrm)
{
	int iPos = (int)(fFrm / FRM_PRECISION);
	CSliderCtrl::SetPos(iPos);
	this->Invalidate();
}
