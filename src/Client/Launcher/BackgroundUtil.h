#ifndef _BACKGROUNDUTIL_H_
#define _BACKGROUNDUTIL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CBackgroundUtil  
{
public:
	BOOL SetBitmap(UINT uResourceID = 0);
	CBackgroundUtil();
	virtual ~CBackgroundUtil();

	int BMPWidth() { return m_nBmpWidth; }
	int BMPHeight() { return m_nBmpHeight; }

protected:
	BOOL TileBitmap(CDC* pDC, CRect rc);
	BOOL GetBitmapAndPalette(UINT nIDResource, CBitmap& bitmap, CPalette& pal);
	// For background bitmap
	CBitmap m_BmpPattern;
	CPalette m_BmpPalette;
	int m_nBmpWidth;
	int m_nBmpHeight;
};

#endif
