#include "stdafx.h"
#include "BackgroundUtil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBackgroundUtil::CBackgroundUtil()
{
	m_nBmpHeight = 0;
	m_nBmpWidth  = 0;
}

CBackgroundUtil::~CBackgroundUtil()
{
	m_BmpPattern.Detach();
	m_BmpPalette.Detach();
}

BOOL CBackgroundUtil::SetBitmap(UINT uResourceID)
{
	BITMAP bm;
	BOOL bRet;

	// Detach previous resources
	m_BmpPattern.Detach();
	m_BmpPalette.Detach();

	// Default return value
	bRet = TRUE;

	// Load new bitmap
	if (uResourceID != 0)
	{
		bRet = GetBitmapAndPalette(uResourceID, m_BmpPattern, m_BmpPalette);

		// If all ok
		if (bRet == TRUE)
		{
			// Get dimension
			m_BmpPattern.GetBitmap(&bm);
			// Width of the bitmap
			m_nBmpWidth  = bm.bmWidth;
			// Height of the bitmap
			m_nBmpHeight = bm.bmHeight;
		}
	}

	return bRet;

} // End of SetBitmap

BOOL CBackgroundUtil::GetBitmapAndPalette(UINT nIDResource, CBitmap& bitmap, CPalette& pal)
{
	LPCTSTR lpszResourceName = (LPCTSTR) (UINT_PTR) nIDResource;

	HBITMAP hBmp             = (HBITMAP)::LoadImage(
        AfxGetInstanceHandle(), lpszResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBmp == nullptr)
		return FALSE;

	bitmap.Attach(hBmp);

	// Create a logical palette for the bitmap
	DIBSECTION ds;
	BITMAPINFOHEADER& bmInfo = ds.dsBmih;
	bitmap.GetObject(sizeof(ds), &ds);

	int nColors = bmInfo.biClrUsed ? bmInfo.biClrUsed : 1 << bmInfo.biBitCount;

	// Create a halftone palette if colors > 256.
	CClientDC dc(nullptr); // Desktop DC

	if (nColors > 256)
		pal.CreateHalftonePalette(&dc);
	else
	{
		// Create the palette
		RGBQUAD* pRGB = new RGBQUAD[nColors];
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		memDC.SelectObject(&bitmap);
		::GetDIBColorTable(memDC, 0, nColors, pRGB);
		UINT nSize         = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE* pLP    = (LOGPALETTE*) new BYTE[nSize];
		pLP->palVersion    = 0x300;
		pLP->palNumEntries = nColors;
		for (int i = 0; i < nColors; i++)
		{
			pLP->palPalEntry[i].peRed   = pRGB[i].rgbRed;
			pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
			pLP->palPalEntry[i].peBlue  = pRGB[i].rgbBlue;
			pLP->palPalEntry[i].peFlags = 0;
		}
		pal.CreatePalette(pLP);
		delete[] pLP;
		delete[] pRGB;
	}
	return TRUE;
} // End of GetBitmapAndPalette

BOOL CBackgroundUtil::TileBitmap(CDC* pDC, CRect rc)
{
	CDC MemDC;

	int x = 0, y = 0;

	// If there is a bitmap loaded
	if (m_BmpPattern.m_hObject != nullptr)
	{
		MemDC.CreateCompatibleDC(pDC);
		CBitmap* pOldBitmap = MemDC.SelectObject(&m_BmpPattern);

		// Tile the bitmap
		while (y < rc.Height())
		{
			while (x < rc.Width())
			{
				pDC->BitBlt(x, y, m_nBmpWidth, m_nBmpHeight, &MemDC, 0, 0, SRCCOPY);
				x += m_nBmpWidth;
			}
			x  = 0;
			y += m_nBmpHeight;
		}

		MemDC.SelectObject(pOldBitmap);

		return TRUE;
	}

	return FALSE; // Normal behaviour
} // End of TileBitmap
