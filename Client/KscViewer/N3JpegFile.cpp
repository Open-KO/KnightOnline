// N3JpegFile.cpp: implementation of the CN3JpegFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KscViewer.h"
#include "N3JpegFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3JpegFile::CN3JpegFile()
{
	m_uiWidth	= 0;
	m_uiHeight	= 0;
	m_pImageBuf = NULL;
}

CN3JpegFile::~CN3JpegFile()
{
	if(m_pImageBuf) delete m_pImageBuf; m_pImageBuf = NULL;
}

void CN3JpegFile::Release()
{
	if(m_pImageBuf) delete m_pImageBuf; m_pImageBuf = NULL;

	m_uiWidth	= 0;
	m_uiHeight	= 0;
}

BOOL CN3JpegFile::LoadJpegFile(std::string csJpeg)
{
	// m_buf is the global buffer
	Release();

	// read to buffer tmp
	m_pImageBuf=CJpegFile::JpegFileToRGB(csJpeg.c_str(), &m_uiWidth, &m_uiHeight);

	//////////////////////
	// set up for display

	// do this before DWORD-alignment!!!
	// this works on packed (not DWORD-aligned) buffers
	// swap red and blue for display
	CJpegFile::BGRFromRGB(m_pImageBuf, m_uiWidth, m_uiHeight);

	// vertical flip for display
	CJpegFile::VertFlipBuf(m_pImageBuf, m_uiWidth * 3, m_uiHeight);

	return TRUE;
}

void CN3JpegFile::DrawImage(HDC hDC)
{
	if(hDC == NULL) return;
	if(m_pImageBuf == NULL) return;
	if(m_uiWidth == 0 || m_uiHeight == 0) return;

	// Center It
//		int left = max(clientRect.left, ((clientRect.Width() - (int)m_width) / 2));
//		int top = max(clientRect.top, ((clientRect.Height() - (int)m_height) / 2));
	int left = 0;
	int top = 0;

	// a 24-bit DIB is DWORD-aligned, vertically flipped and 
	// has Red and Blue bytes swapped. we already did the 
	// RGB->BGR and the flip when we read the images, now do
	// the DWORD-align

	UINT uiOutWidthBytes = 0;
	BYTE *tmp;
	// DWORD-align for display
	tmp = CJpegFile::MakeDwordAlignedBuf(m_pImageBuf,
									 m_uiWidth,
									 m_uiHeight,
									 &uiOutWidthBytes);

	// set up a DIB 
	BITMAPINFOHEADER bmiHeader;
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = m_uiWidth;
	bmiHeader.biHeight = m_uiHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 24;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;


	// now blast it to the CDC passed in.
	// lines returns the number of lines actually displayed
	int lines = StretchDIBits(hDC,
								left, top,
								bmiHeader.biWidth,
								bmiHeader.biHeight,
								0,0,
								bmiHeader.biWidth,
								bmiHeader.biHeight,
								tmp,
								(LPBITMAPINFO)&bmiHeader,
								DIB_RGB_COLORS,
								SRCCOPY);

	delete [] tmp;
}
