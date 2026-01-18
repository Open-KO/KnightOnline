////////////////////////////////////////////////////////////
//	JpegFile - A C++ class to allow reading and writing of
//	RGB and Grayscale JPEG images.
//	It is based on the IJG V.6 code.
//
//	This class Copyright 1997, Chris Losinger
//	This is free to use and modify provided my name is
//	included.
//
//	See jpegfile.h for usage.
//
////////////////////////////////////////////////////////////
#include <Windows.h>
#include "JpegFile.h"

#include <cassert>
#include <ctime>
#include <filesystem>

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <csetjmp>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <jpeglib.h>

#ifdef __cplusplus
}
#endif // __cplusplus

constexpr WORD PALVERSION = 0x300;

constexpr auto WIDTHBYTES(auto bits)
{
	return (((bits) + 31) / 32 * 4);
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define IS_WIN30_DIB(lpbi) ((*(LPDWORD) (lpbi)) == sizeof(BITMAPINFOHEADER))
#define DIB_HEADER_MARKER  ((WORD) ('M' << 8) | 'B')
// NOLINTEND(cppcoreguidelines-macro-usage)

// error handler, to avoid those pesky exit(0)'s

struct my_error_mgr
{
	struct jpeg_error_mgr pub; /* "public" fields */

	jmp_buf setjmp_buffer;     /* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

//
//
//

METHODDEF(void) my_error_exit(j_common_ptr cinfo);

//
//	to handle fatal errors.
//	the original JPEG code will just exit(0). can't really
//	do that in Windows....
//

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message)(cinfo, buffer);

	/* Always display the message. */
	//MessageBox(nullptr,buffer,"JPEG Fatal Error",MB_ICONSTOP);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

// store a scanline to our data buffer
void j_putRGBScanline(BYTE* jpegline, int widthPix, BYTE* outBuf, int row);

void j_putGrayScanlineToRGB(BYTE* jpegline, int widthPix, BYTE* outBuf, int row);

static BOOL DibToSamps(HANDLE hDib, int nSampsPerRow, JSAMPARRAY jsmpPixels, const char** pcsMsg);
static RGBQUAD QuadFromWord(WORD b16);

//
//	constructor doesn't do much - there's no real class here...
//

WORD CJpegFile::m_r  = 1124;
WORD CJpegFile::m_c1 = 52845;
WORD CJpegFile::m_c2 = 22719;

CJpegFile::CJpegFile()
{
}

//
//
//

CJpegFile::~CJpegFile()
{
}

//
//	read a JPEG file
//

BYTE* CJpegFile::JpegFileToRGB(const std::string& fileName, UINT* width, UINT* height)

{
	// get our buffer set to hold data
	BYTE* dataBuf = nullptr;

	// basic code from IJG Jpeg Code v6 example.c

	*width        = 0;
	*height       = 0;

	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;
	/* More stuff */
	FILE* infile      = nullptr; /* source file */
	JSAMPARRAY buffer = nullptr; /* Output row buffer */
	int row_stride    = 0;       /* physical row width in output buffer */

	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

#ifdef _MSC_VER
	fopen_s(&infile, fileName.c_str(), "rb");
#else
	infile = fopen(fileName.c_str(), "rb");
#endif

	if (infile == nullptr)
		return nullptr;

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err           = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */

		jpeg_destroy_decompress(&cinfo);

		if (infile != nullptr)
			fclose(infile);

		delete[] dataBuf;

		return nullptr;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/
	(void) jpeg_start_decompress(&cinfo);

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
	const size_t bufferSize = 3 * static_cast<size_t>(cinfo.output_width)
							  * static_cast<size_t>(cinfo.output_height);

	dataBuf = new BYTE[bufferSize];
	if (dataBuf == nullptr)
	{
		//		AfxMessageBox("JpegFile :\nOut of memory",MB_ICONSTOP);

		jpeg_destroy_decompress(&cinfo);

		fclose(infile);

		return nullptr;
	}

	// how big is this thing gonna be?
	*width     = cinfo.output_width;
	*height    = cinfo.output_height;

	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer     = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height)
	{
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */

		// asuumer all 3-components are RGBs
		if (cinfo.out_color_components == 3)
		{
			j_putRGBScanline(buffer[0], *width, dataBuf, cinfo.output_scanline - 1);
		}
		else if (cinfo.out_color_components == 1)
		{
			// assume all single component images are grayscale
			j_putGrayScanlineToRGB(buffer[0], *width, dataBuf, cinfo.output_scanline - 1);
		}
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	return dataBuf;
}

//
//	stash a scanline
//

void j_putRGBScanline(BYTE* jpegline, int widthPix, BYTE* outBuf, int row)
{
	int offset = row * widthPix * 3;
	for (int count = 0; count < widthPix; count++)
	{
		outBuf[offset + count * 3 + 0] = jpegline[count * 3 + 0];
		outBuf[offset + count * 3 + 1] = jpegline[count * 3 + 1];
		outBuf[offset + count * 3 + 2] = jpegline[count * 3 + 2];
	}
}

//
//	stash a gray scanline
//

void j_putGrayScanlineToRGB(BYTE* jpegline, int widthPix, BYTE* outBuf, int row)
{
	int offset = row * widthPix * 3;
	for (int count = 0; count < widthPix; count++)
	{
		// get our grayscale value
		BYTE iGray                     = jpegline[count];

		outBuf[offset + count * 3 + 0] = iGray;
		outBuf[offset + count * 3 + 1] = iGray;
		outBuf[offset + count * 3 + 2] = iGray;
	}
}

//
// copies BYTE buffer into DWORD-aligned BYTE buffer
// return addr of new buffer
//

BYTE* CJpegFile::MakeDwordAlignedBuf(BYTE* dataBuf,
	UINT widthPix,         // pixels!!
	UINT height,
	UINT* uiOutWidthBytes) // bytes!!!
{
	////////////////////////////////////////////////////////////
	// what's going on here? this certainly means trouble
	if (dataBuf == nullptr)
		return nullptr;

	////////////////////////////////////////////////////////////
	// how big is the smallest DWORD-aligned buffer that we can use?
	UINT uiWidthBytes = WIDTHBYTES(widthPix * 24);

	DWORD dwNewsize   = (DWORD) ((DWORD) uiWidthBytes * (DWORD) height);

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
	BYTE* pNew        = new BYTE[dwNewsize];
	if (pNew == nullptr)
		return nullptr;

	////////////////////////////////////////////////////////////
	// copy row-by-row
	UINT uiInWidthBytes = widthPix * 3;
	for (UINT uiCount = 0; uiCount < height; uiCount++)
	{
		BYTE* bpInAdd  = nullptr;
		BYTE* bpOutAdd = nullptr;
		ULONG lInOff = 0, lOutOff = 0;

		lInOff   = uiInWidthBytes * uiCount;
		lOutOff  = uiWidthBytes * uiCount;

		bpInAdd  = dataBuf + lInOff;
		bpOutAdd = pNew + lOutOff;

		memcpy(bpOutAdd, bpInAdd, uiInWidthBytes);
	}

	*uiOutWidthBytes = uiWidthBytes;
	return pNew;
}

//
//	vertically flip a buffer
//	note, this operates on a buffer of widthBytes bytes, not pixels!!!
//

BOOL CJpegFile::VertFlipBuf(BYTE* inbuf, UINT widthBytes, UINT height)
{
	BYTE* tb1    = nullptr;
	BYTE* tb2    = nullptr;
	UINT bufsize = 0;

	if (inbuf == nullptr)
		return FALSE;

	bufsize = widthBytes;

	tb1     = new BYTE[bufsize];
	if (tb1 == nullptr)
		return FALSE;

	tb2 = new BYTE[bufsize];
	if (tb2 == nullptr)
	{
		delete[] tb1;
		return FALSE;
	}

	ULONG off1 = 0, off2 = 0;
	for (UINT row_cnt = 0; row_cnt < (height + 1) / 2; row_cnt++)
	{
		off1 = row_cnt * bufsize;
		off2 = ((height - 1) - row_cnt) * bufsize;

		memcpy(tb1, inbuf + off1, bufsize);
		memcpy(tb2, inbuf + off2, bufsize);
		memcpy(inbuf + off1, tb2, bufsize);
		memcpy(inbuf + off2, tb1, bufsize);
	}

	delete[] tb1;
	delete[] tb2;

	return TRUE;
}

//
//	swap Rs and Bs
//
//	Note! this does its stuff on buffers with a whole number of pixels
//	per data row!!
//

BOOL CJpegFile::BGRFromRGB(BYTE* buf, UINT widthPix, UINT height)
{
	if (buf == nullptr)
		return FALSE;

	for (UINT row = 0; row < height; row++)
	{
		for (UINT col = 0; col < widthPix; col++)
		{
			LPBYTE pRed = nullptr, pBlu = nullptr;
			pRed = &buf[row * widthPix * 3 + col * 3];
			pBlu = &buf[row * widthPix * 3 + col * 3 + 2];

			// swap red and blue
			std::swap(*pRed, *pBlu);
		}
	}
	return TRUE;
}

int CJpegFile::PalEntriesOnDevice(HDC hDC)
{
	int nColors = 0; // number of colors

	/*  Find out the number of palette entries on this
	 *  device.
	 */

	//	nColors = GetDeviceCaps(hDC, SIZEPALETTE);
	nColors     = GetDeviceCaps(hDC, BITSPIXEL);
	if (nColors > 8)
		return 0;

	nColors = 2 << nColors;

	/*  For non-palette devices, we'll use the # of system
	 *  colors for our palette size.
	 */
	if (nColors == 0)
		nColors = GetDeviceCaps(hDC, NUMCOLORS);
	assert(nColors);
	return nColors;
}

HPALETTE CJpegFile::GetSystemPalette()
{
	HPALETTE hPal         = nullptr; // handle to a palette
	HDC hDC               = nullptr; // handle to a DC
	HANDLE hLogPal        = nullptr; // handle to a logical palette
	LPLOGPALETTE lpLogPal = nullptr; // pointer to a logical palette
	int nColors           = 0;       // number of colors

	/* Find out how many palette entries we want. */
	hDC                   = GetDC(nullptr);
	if (hDC == nullptr)
		return nullptr;

	nColors = PalEntriesOnDevice(hDC); // Number of palette entries
	if (nColors == 0)
		return nullptr;

	/* Allocate room for the palette and lock it. */
	hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + nColors * sizeof(PALETTEENTRY));

	/* if we didn't get a logical palette, return nullptr */
	if (hLogPal == nullptr)
		return nullptr;

	/* get a pointer to the logical palette */
	lpLogPal = (LPLOGPALETTE) GlobalLock(hLogPal);
	if (lpLogPal == nullptr)
		return nullptr;

	/* set some important fields */
	lpLogPal->palVersion    = PALVERSION;
	lpLogPal->palNumEntries = nColors;

	/* Copy the current system palette into our logical palette */

	GetSystemPaletteEntries(hDC, 0, nColors, (LPPALETTEENTRY) (lpLogPal->palPalEntry));

	/*  Go ahead and create the palette.  Once it's created,
	 *  we no longer need the LOGPALETTE, so free it.
     */

	hPal = CreatePalette(lpLogPal);

	/* clean up */
	GlobalUnlock(hLogPal);
	GlobalFree(hLogPal);
	ReleaseDC(nullptr, hDC);

	return hPal;
}

WORD CJpegFile::DIBNumColors(LPSTR lpDIB)
{
	WORD wBitCount = 0; // DIB bit count

	/*  If this is a Windows-style DIB, the number of colors in the
	 *  color table can be less than the number of bits per pixel
	 *  allows for (i.e. lpbi->biClrUsed can be set to some value).
	 *  If this is the case, return the appropriate value.
     */

	if (IS_WIN30_DIB(lpDIB))
	{
		DWORD dwClrUsed = ((LPBITMAPINFOHEADER) lpDIB)->biClrUsed;
		if (dwClrUsed)
			return (WORD) dwClrUsed;
	}

	/*  Calculate the number of colors in the color table based on
     *  the number of bits per pixel for the DIB.
     */
	if (IS_WIN30_DIB(lpDIB))
		wBitCount = ((LPBITMAPINFOHEADER) lpDIB)->biBitCount;
	else
		wBitCount = ((LPBITMAPCOREHEADER) lpDIB)->bcBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}

WORD CJpegFile::PaletteSize(LPSTR lpDIB)
{
	/* calculate the size required by the palette */
	if (IS_WIN30_DIB(lpDIB))
		return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
	else
		return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}

HANDLE CJpegFile::BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
	BITMAP bm {};                       // bitmap structure
	BITMAPINFOHEADER bi {};             // bitmap header
	BITMAPINFOHEADER* lpbi = nullptr;   // pointer to BITMAPINFOHEADER
	DWORD dwLen            = 0;         // size of memory block
	HANDLE hDIB = nullptr, h = nullptr; // handle to DIB, temp handle
	HDC hDC     = nullptr;              // handle to DC
	WORD biBits = 0;                    // bits per pixel

	/* check if bitmap handle is valid */
	if (hBitmap == nullptr)
		return nullptr;

	/* fill in BITMAP structure, return nullptr if it didn't work */
	if (!GetObject(hBitmap, sizeof(bm), (LPSTR) &bm))
		return nullptr;

	/* if no palette is specified, use default palette */
	if (hPal == nullptr)
		hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

	/* calculate bits per pixel */
	biBits = bm.bmPlanes * bm.bmBitsPixel;

	/* make sure bits per pixel is valid */
	if (biBits <= 1)
		biBits = 1;
	else if (biBits <= 4)
		biBits = 4;
	else if (biBits <= 8)
		biBits = 8;
	else /* if greater than 8-bit, force to 24-bit */
		biBits = 24;

	/* initialize BITMAPINFOHEADER */
	bi.biSize          = sizeof(BITMAPINFOHEADER);
	bi.biWidth         = bm.bmWidth;
	bi.biHeight        = bm.bmHeight;
	bi.biPlanes        = 1;
	bi.biBitCount      = biBits;
	bi.biCompression   = BI_RGB;
	bi.biSizeImage     = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed       = 0;
	bi.biClrImportant  = 0;

	/* calculate size of memory block required to store BITMAPINFO */
	dwLen              = bi.biSize + PaletteSize((LPSTR) &bi);

	/* get a DC */
	hDC                = GetDC(nullptr);

	/* select and realize our palette */
	hPal               = SelectPalette(hDC, hPal, FALSE);
	RealizePalette(hDC);

	/* alloc memory block to store our bitmap */
	hDIB = GlobalAlloc(GHND, dwLen);

	/* if we couldn't get memory block */
	if (hDIB == nullptr)
	{
		/* clean up and return nullptr */
		SelectPalette(hDC, hPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(nullptr, hDC);
		return nullptr;
	}

	/* lock memory and get pointer to it */
	lpbi = (BITMAPINFOHEADER*) GlobalLock(hDIB);
	if (lpbi == nullptr)
	{
		GlobalFree(hDIB);

		/* clean up and return nullptr */
		SelectPalette(hDC, hPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(nullptr, hDC);
		return nullptr;
	}

	/* use our bitmap info. to fill BITMAPINFOHEADER */
	*lpbi = bi;

	/*  call GetDIBits with a nullptr lpBits param, so it will calculate the
	 *  biSizeImage field for us
	 */
	GetDIBits(hDC, hBitmap, 0, (WORD) bi.biHeight, nullptr, (LPBITMAPINFO) lpbi, DIB_RGB_COLORS);

	/* get the info. returned by GetDIBits and unlock memory block */
	bi = *lpbi;
	GlobalUnlock(hDIB);

	/* if the driver did not fill in the biSizeImage field, make one up */
	if (bi.biSizeImage == 0)
		bi.biSizeImage = WIDTHBYTES((DWORD) bm.bmWidth * biBits) * bm.bmHeight;

	/* realloc the buffer big enough to hold all the bits */
	dwLen = bi.biSize + PaletteSize((LPSTR) &bi) + bi.biSizeImage;

	h     = GlobalReAlloc(hDIB, dwLen, 0);
	if (h == nullptr)
	{
		/* clean up and return nullptr */
		GlobalFree(hDIB);
		hDIB = nullptr;
		SelectPalette(hDC, hPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(nullptr, hDC);
		return nullptr;
	}

	hDIB = h;

	/* lock memory block and get pointer to it */
	lpbi = (BITMAPINFOHEADER*) GlobalLock(hDIB);
	if (lpbi == nullptr)
	{
		/* clean up and return nullptr */
		GlobalFree(hDIB);
		hDIB = nullptr;
		SelectPalette(hDC, hPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(nullptr, hDC);
		return nullptr;
	}

	/*  call GetDIBits with a NON-nullptr lpBits param, and actualy get the
	 *  bits this time
	 */
	if (GetDIBits(hDC, hBitmap, 0, (WORD) bi.biHeight,
			(LPSTR) lpbi + (WORD) lpbi->biSize + PaletteSize((LPSTR) lpbi), (LPBITMAPINFO) lpbi,
			DIB_RGB_COLORS)
		== 0)
	{
		/* clean up and return nullptr */
		GlobalUnlock(hDIB);
		hDIB = nullptr;
		SelectPalette(hDC, hPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(nullptr, hDC);
		return nullptr;
	}
	bi = *lpbi;

	/* clean up */
	GlobalUnlock(hDIB);
	SelectPalette(hDC, hPal, TRUE);
	RealizePalette(hDC);
	ReleaseDC(nullptr, hDC);

	/* return handle to the DIB */
	return hDIB;
}

HBITMAP CJpegFile::CopyScreenToBitmap(LPRECT lpRect)
{
	HDC hScrDC = nullptr, hMemDC = nullptr;          // screen DC and memory DC
	HBITMAP hBitmap = nullptr, hOldBitmap = nullptr; // handles to deice-dependent bitmaps
	int nX = 0, nY = 0, nX2 = 0, nY2 = 0;            // coordinates of rectangle to grab
	int nWidth = 0, nHeight = 0;                     // DIB width and height
	int xScrn = 0, yScrn = 0;                        // screen resolution

	/* check for an empty rectangle */

	if (IsRectEmpty(lpRect))
		return nullptr;

	/*  create a DC for the screen and create
	 *  a memory DC compatible to screen DC
     */
	hScrDC = CreateDC("DISPLAY", nullptr, nullptr, nullptr);
	hMemDC = CreateCompatibleDC(hScrDC);

	/* get points of rectangle to grab */
	nX     = lpRect->left;
	nY     = lpRect->top;
	nX2    = lpRect->right;
	nY2    = lpRect->bottom;

	/* get screen resolution */
	xScrn  = GetDeviceCaps(hScrDC, HORZRES);
	yScrn  = GetDeviceCaps(hScrDC, VERTRES);

	/* make sure bitmap rectangle is visible */
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > xScrn)
		nX2 = xScrn;
	if (nY2 > yScrn)
		nY2 = yScrn;
	nWidth     = nX2 - nX;
	nHeight    = nY2 - nY;

	/* create a bitmap compatible with the screen DC */
	hBitmap    = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

	/* select new bitmap into memory DC */
	hOldBitmap = (HBITMAP) SelectObject(hMemDC, hBitmap);

	/* bitblt screen DC to memory DC */
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);

	/*  select old bitmap back into memory DC and get handle to
	 *  bitmap of the screen
	 */
	hBitmap = (HBITMAP) SelectObject(hMemDC, hOldBitmap);

	/* clean up */
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);

	/* return handle to the bitmap */
	return hBitmap;
}

HANDLE CJpegFile::CopyScreenToDIB(LPRECT lpRect)
{
	HBITMAP hBitmap   = nullptr; // handle to device-dependent bitmap
	HPALETTE hPalette = nullptr; // handle to palette
	HANDLE hDIB       = nullptr; // handle to DIB

	/*  get the device-dependent bitmap in lpRect by calling
	 *  CopyScreenToBitmap and passing it the rectangle to grab
     */

	hBitmap           = CopyScreenToBitmap(lpRect);

	/* check for a valid bitmap handle */
	if (!hBitmap)
		return nullptr;

	/* get the current palette */
	hPalette = GetSystemPalette();

	/* convert the bitmap to a DIB */
	hDIB     = BitmapToDIB(hBitmap, hPalette);

	/* clean up */
	DeleteObject(hPalette);
	DeleteObject(hBitmap);

	/* return handle to the packed-DIB */
	return hDIB;
}

HANDLE CJpegFile::AllocRoomForDIB(BITMAPINFOHEADER bi, HBITMAP hBitmap)
{
	DWORD dwLen             = 0;
	HANDLE hDIB             = nullptr;
	HDC hDC                 = nullptr;
	LPBITMAPINFOHEADER lpbi = nullptr;
	HANDLE hTemp            = nullptr;

	/* Figure out the size needed to hold the BITMAPINFO structure
     * (which includes the BITMAPINFOHEADER and the color table).
     */

	dwLen                   = bi.biSize + PaletteSize((LPSTR) &bi);
	hDIB                    = GlobalAlloc(GHND, dwLen);

	/* Check that DIB handle is valid */
	if (hDIB == nullptr)
		return nullptr;

	/* Set up the BITMAPINFOHEADER in the newly allocated global memory,
	 * then call GetDIBits() with lpBits = nullptr to have it fill in the
	 * biSizeImage field for us.
     */
	lpbi = (LPBITMAPINFOHEADER) GlobalLock(hDIB);
	if (lpbi == nullptr)
	{
		GlobalFree(hDIB);
		return nullptr;
	}

	*lpbi = bi;

	hDC   = GetDC(nullptr);
	GetDIBits(hDC, hBitmap, 0, (WORD) bi.biHeight, nullptr, (LPBITMAPINFO) lpbi, DIB_RGB_COLORS);
	ReleaseDC(nullptr, hDC);

	/* If the driver did not fill in the biSizeImage field,
     * fill it in -- NOTE: this is a bug in the driver!
     */
	if (lpbi->biSizeImage == 0)
		lpbi->biSizeImage = WIDTHBYTES((DWORD) lpbi->biWidth * lpbi->biBitCount) * lpbi->biHeight;

	/* Get the size of the memory block we need */
	dwLen = lpbi->biSize + PaletteSize((LPSTR) &bi) + lpbi->biSizeImage;

	/* Unlock the memory block */
	GlobalUnlock(hDIB);

	/* ReAlloc the buffer big enough to hold all the bits */
	hTemp = GlobalReAlloc(hDIB, dwLen, 0);
	if (hTemp == nullptr)
	{
		GlobalFree(hDIB);
		return nullptr;
	}

	return hTemp;
}

static RGBQUAD QuadFromWord(WORD b16)
{
	BYTE bytVals[] = { 0, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144,
		152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };

	WORD wR        = b16;
	WORD wG        = b16;
	WORD wB        = b16;

	wR <<= 1;
	wR >>= 11;
	wG <<= 6;
	wG >>= 11;
	wB <<= 11;
	wB >>= 11;

	RGBQUAD rgb;

	rgb.rgbReserved = 0;
	rgb.rgbBlue     = bytVals[wB];
	rgb.rgbGreen    = bytVals[wG];
	rgb.rgbRed      = bytVals[wR];

	return rgb;
}

static BOOL DibToSamps(HANDLE hDib, int nSampsPerRow, JSAMPARRAY jsmpPixels, const char** pcsMsg)
{
	// Sanity...
	if (hDib == nullptr || nSampsPerRow <= 0 || pcsMsg == nullptr)
	{
		if (pcsMsg != nullptr)
			*pcsMsg = "Invalid input data";
		return FALSE;
	}

	int r = 0, p = 0, q = 0, b = 0, n = 0, nUnused = 0, nBytesWide = 0, nUsed = 0, nLastBits = 0,
		nLastNibs = 0, nCTEntries = 0, nRow = 0, nByte = 0, nPixel = 0;
	BYTE bytCTEnt              = 0;
	LPBITMAPINFOHEADER pbBmHdr = (LPBITMAPINFOHEADER) GlobalLock(hDib);
	if (pbBmHdr == nullptr)
	{
		if (pcsMsg != nullptr)
			*pcsMsg = "Failed to lock memory";
		return FALSE;
	}

	switch (pbBmHdr->biBitCount)
	{
		case 1:
			nCTEntries = 2; // Monochrome
			break;

		case 4:
			nCTEntries = 16; // 16-color
			break;

		case 8:
			nCTEntries = 256; // 256-color
			break;

		case 16:
		case 24:
		case 32:
			nCTEntries = 0; // No color table needed
			break;

		default:
			if (pcsMsg != nullptr)
				*pcsMsg = "Invalid bitmap bit count";

			GlobalUnlock(hDib);
			return FALSE; //Unsupported format
	}

	//Point to the color table and pixels
	LPRGBQUAD pCTab = reinterpret_cast<RGBQUAD*>(
		reinterpret_cast<char*>(pbBmHdr) + pbBmHdr->biSize);
	LPSTR lpBits = (LPSTR) pbBmHdr + (WORD) pbBmHdr->biSize + (WORD) (nCTEntries * sizeof(RGBQUAD));

	//Different formats for the image bits
	LPBYTE lpPixels = (LPBYTE) lpBits;
	RGBQUAD* pRgbQs = (RGBQUAD*) lpBits;
	WORD* wPixels   = (WORD*) lpBits;

	//Set up the jsamps according to the bitmap's format.
	//Note that rows are processed bottom to top, because
	//that's how bitmaps are created.
	switch (pbBmHdr->biBitCount)
	{
		case 1:
			nUsed      = (pbBmHdr->biWidth + 7) / 8;
			nUnused    = (((nUsed + 3) / 4) * 4) - nUsed;
			nBytesWide = nUsed + nUnused;
			nLastBits  = 8 - ((nUsed * 8) - pbBmHdr->biWidth);

			for (r = 0; r < pbBmHdr->biHeight; r++)
			{
				for (p = 0, q = 0; p < nUsed; p++)
				{
					nRow       = (pbBmHdr->biHeight - r - 1) * nBytesWide;
					nByte      = nRow + p;

					int nBUsed = (p < (nUsed - 1)) ? 8 : nLastBits;
					for (b = 0; b < nBUsed; b++)
					{
						bytCTEnt              = lpPixels[nByte] << b;
						bytCTEnt              = bytCTEnt >> 7;

						jsmpPixels[r][q + 0]  = pCTab[bytCTEnt].rgbRed;
						jsmpPixels[r][q + 1]  = pCTab[bytCTEnt].rgbGreen;
						jsmpPixels[r][q + 2]  = pCTab[bytCTEnt].rgbBlue;

						q                    += 3;
					}
				}
			}
			break;

		case 4:
			nUsed      = (pbBmHdr->biWidth + 1) / 2;
			nUnused    = (((nUsed + 3) / 4) * 4) - nUsed;
			nBytesWide = nUsed + nUnused;
			nLastNibs  = 2 - ((nUsed * 2) - pbBmHdr->biWidth);

			for (r = 0; r < pbBmHdr->biHeight; r++)
			{
				for (p = 0, q = 0; p < nUsed; p++)
				{
					nRow         = (pbBmHdr->biHeight - r - 1) * nBytesWide;
					nByte        = nRow + p;

					int nNibbles = (p < nUsed - 1) ? 2 : nLastNibs;

					for (n = 0; n < nNibbles; n++)
					{
						bytCTEnt  = lpPixels[nByte] << (n * 4);
						// bytCTEnt = bytCTEnt >> (4 - (n * 4));

						int shift = 4 - (n * 4);
						if (shift >= 0)
							bytCTEnt = static_cast<uint8_t>(bytCTEnt >> shift);
						else
							bytCTEnt = static_cast<uint8_t>(bytCTEnt << -shift);

						jsmpPixels[r][q + 0]  = pCTab[bytCTEnt].rgbRed;
						jsmpPixels[r][q + 1]  = pCTab[bytCTEnt].rgbGreen;
						jsmpPixels[r][q + 2]  = pCTab[bytCTEnt].rgbBlue;
						q                    += 3;
					}
				}
			}
			break;

		default:
		case 8: //Each byte is a pointer to a pixel color
			nUnused = (((pbBmHdr->biWidth + 3) / 4) * 4) - pbBmHdr->biWidth;

			for (r = 0; r < pbBmHdr->biHeight; r++)
			{
				for (p = 0, q = 0; p < pbBmHdr->biWidth; p++, q += 3)
				{
					nRow   = (pbBmHdr->biHeight - r - 1) * (pbBmHdr->biWidth + nUnused);
					nPixel = nRow + p;

					jsmpPixels[r][q + 0] = pCTab[lpPixels[nPixel]].rgbRed;
					jsmpPixels[r][q + 1] = pCTab[lpPixels[nPixel]].rgbGreen;
					jsmpPixels[r][q + 2] = pCTab[lpPixels[nPixel]].rgbBlue;
				}
			}
			break;

		case 16: //Hi-color (16 bits per pixel)
			for (r = 0; r < pbBmHdr->biHeight; r++)
			{
				for (p = 0, q = 0; p < pbBmHdr->biWidth; p++, q += 3)
				{
					nRow                 = (pbBmHdr->biHeight - r - 1) * pbBmHdr->biWidth;
					nPixel               = nRow + p;

					RGBQUAD quad         = QuadFromWord(wPixels[nPixel]);

					jsmpPixels[r][q + 0] = quad.rgbRed;
					jsmpPixels[r][q + 1] = quad.rgbGreen;
					jsmpPixels[r][q + 2] = quad.rgbBlue;
				}
			}
			break;

		case 24:
			nBytesWide  = (pbBmHdr->biWidth * 3);
			nUnused     = (((nBytesWide + 3) / 4) * 4) - nBytesWide;
			nBytesWide += nUnused;

			for (r = 0; r < pbBmHdr->biHeight; r++)
			{
				for (p = 0, q = 0; p < (nBytesWide - nUnused); p += 3, q += 3)
				{
					nRow                 = (pbBmHdr->biHeight - r - 1) * nBytesWide;
					nPixel               = nRow + p;

					jsmpPixels[r][q + 0] = lpPixels[nPixel + 2]; //Red
					jsmpPixels[r][q + 1] = lpPixels[nPixel + 1]; //Green
					jsmpPixels[r][q + 2] = lpPixels[nPixel + 0]; //Blue
				}
			}
			break;

		case 32:
			for (r = 0; r < pbBmHdr->biHeight; r++)
			{
				for (p = 0, q = 0; p < pbBmHdr->biWidth; p++, q += 3)
				{
					nRow                 = (pbBmHdr->biHeight - r - 1) * pbBmHdr->biWidth;
					nPixel               = nRow + p;

					jsmpPixels[r][q + 0] = pRgbQs[nPixel].rgbRed;
					jsmpPixels[r][q + 1] = pRgbQs[nPixel].rgbGreen;
					jsmpPixels[r][q + 2] = pRgbQs[nPixel].rgbBlue;
				}
			}
			break;
	} //end switch

	GlobalUnlock(hDib);

	return TRUE;
}

BOOL CJpegFile::JpegFromDib(HANDLE hDib, // Handle to DIB
	int nQuality,                        // JPEG quality (0-100)
	const std::string& csJpeg,           // Pathname to jpeg file
	const char** pcsMsg)                 // Error msg to return
{
	// Basic sanity checks...
	if (nQuality < 0 || nQuality > 100 || hDib == nullptr || pcsMsg == nullptr || csJpeg.empty())
	{
		if (pcsMsg != nullptr)
			*pcsMsg = "Invalid input data";

		return FALSE;
	}

	if (pcsMsg != nullptr)
		*pcsMsg = "";

	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER) GlobalLock(hDib);
	if (lpbi == nullptr)
	{
		if (pcsMsg != nullptr)
			*pcsMsg = "Failed to lock memory";

		return FALSE;
	}

	// Use libjpeg functions to write scanlines to disk in JPEG format
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE* pOutFile       = nullptr;               // Target file
	int nSampsPerRow     = 0;                     // Physical row width in image buffer
	JSAMPARRAY jsmpArray = nullptr;               // Pixel RGB buffer for JPEG file

	cinfo.err            = jpeg_std_error(&jerr); // Use default error handling (ugly!)

	jpeg_create_compress(&cinfo);

#ifdef _MSC_VER
	fopen_s(&pOutFile, csJpeg.c_str(), "wb");
#else
	pOutFile = fopen(csJpeg.c_str(), "wb");
#endif

	if (pOutFile == nullptr)
	{
		if (pcsMsg != nullptr)
			*pcsMsg = "Cannot open file for writing";

		jpeg_destroy_compress(&cinfo);
		GlobalUnlock(hDib);
		return FALSE;
	}

	jpeg_stdio_dest(&cinfo, pOutFile);

	cinfo.image_width      = lpbi->biWidth; //Image width and height, in pixels
	cinfo.image_height     = lpbi->biHeight;
	cinfo.input_components = 3;             //Color components per pixel
	//(RGB_PIXELSIZE - see jmorecfg.h)
	cinfo.in_color_space   = JCS_RGB; //Colorspace of input image

	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo,
		nQuality, //Quality: 0-100 scale
		TRUE);    //Limit to baseline-JPEG values

	jpeg_start_compress(&cinfo, TRUE);

	//JSAMPLEs per row in output buffer
	nSampsPerRow = cinfo.image_width * cinfo.input_components;

	//Allocate array of pixel RGB values
	jsmpArray    = (*cinfo.mem->alloc_sarray)(
        (j_common_ptr) &cinfo, JPOOL_IMAGE, nSampsPerRow, cinfo.image_height);

	GlobalUnlock(hDib);

	BOOL convertedSamples = DibToSamps(hDib, nSampsPerRow, jsmpArray, pcsMsg);
	if (convertedSamples)
	{
		// Write the array of scan lines to the JPEG file
		(void) jpeg_write_scanlines(&cinfo, jsmpArray, cinfo.image_height);
	}

	jpeg_finish_compress(&cinfo); //Always finish

	fclose(pOutFile);

	jpeg_destroy_compress(&cinfo); //Free resources

	return convertedSamples;
}

BOOL CJpegFile::EncryptJPEG(HANDLE hDib, // Handle to DIB
	int nQuality,                        // JPEG quality (0-100)
	const std::string& csJpeg,           // Pathname to jpeg file
	const char** pcsMsg)                 // Error msg to return
{
	if (!JpegFromDib(hDib, nQuality, csJpeg, pcsMsg))
		return FALSE;

	uint8_t* data_byte = nullptr;
	size_t encrypt_len = 0, i = 0;
	FILE* fileHandle = nullptr;

	// JPEG 파일 읽어오기
#ifdef _MSC_VER
	fopen_s(&fileHandle, csJpeg.c_str(), "rb");
#else
	fileHandle = fopen(csJpeg.c_str(), "rb");
#endif
	if (fileHandle == nullptr)
		return FALSE;

	size_t fileSize = 0;
	fseek(fileHandle, 0, SEEK_END);
	fileSize = static_cast<size_t>(ftell(fileHandle));

	if (fileSize == 0)
	{
		fclose(fileHandle);
		return FALSE;
	}

	fseek(fileHandle, 0, SEEK_SET);

	data_byte = new BYTE[fileSize + 8];

	srand((unsigned) time(nullptr));
	for (i = 0; i < 4; i++)
		data_byte[i] = rand() % 0x100;

	data_byte[4] = 'K';
	data_byte[5] = 'S';
	data_byte[6] = 'C';
	data_byte[7] = 1;

	fread(data_byte + 8, fileSize, 1, fileHandle);
	fclose(fileHandle);
	fileHandle  = nullptr;

	m_r         = 1124;

	// JPEG 파일 Encoding
	encrypt_len = fileSize + 8;
	for (i = 0; i < encrypt_len; i++)
		data_byte[i] = Encrypt(data_byte[i]);

	// Encoding 파일 Writing
#ifdef _MSC_VER
	fileHandle = nullptr;
	fopen_s(&fileHandle, csJpeg.c_str(), "wb");
#else
	fileHandle = fopen(csJpeg.c_str(), "wb");
#endif

	if (fileHandle == nullptr)
	{
		delete[] data_byte;
		return FALSE;
	}

	fwrite(data_byte, encrypt_len, 1, fileHandle);
	fclose(fileHandle);

	delete[] data_byte;

	return TRUE;
}

BOOL CJpegFile::DecryptJPEG(const std::string& csJpeg)
{
	char szTempName[MAX_PATH] {};
	std::string szDstpath;
	uint8_t *dst_data = nullptr, *src_data = nullptr;
	size_t jpegLen = 0, kscLen = 0;
	size_t i = 0, j = 0;

	size_t rfv = csJpeg.rfind('\\');
	szDstpath  = csJpeg;

	if (rfv != std::string::npos)
		szDstpath.resize(rfv);

	if (szDstpath.size() == 2)
		szDstpath += '\\'; //_T('\\');

	if (GetTempFileName(szDstpath.c_str(), "ksc", 0, szTempName) == 0)
	{
		//		AfxMessageBox("임시 파일을 생성할 수가 없습니다.", MB_ICONSTOP|MB_OK);
		return FALSE;
	}

	FILE* kscHandle  = nullptr;
	FILE* jpegHandle = nullptr;

#ifdef _MSC_VER
	fopen_s(&kscHandle, csJpeg.c_str(), "rb");
#else
	kscHandle = fopen(csJpeg.c_str(), "rb");
#endif

	if (kscHandle == nullptr)
	{
		//		AfxMessageBox("소스 파일이 존재하지 않습니다. 다른 파일을 선택해주세요.", MB_ICONSTOP|MB_OK);
		return FALSE;
	}

#ifdef _MSC_VER
	fopen_s(&jpegHandle, szTempName, "wb");
#else
	jpegHandle = fopen(szTempName, "wb");
#endif

	if (jpegHandle == nullptr)
	{
		fclose(kscHandle);
		return FALSE;
	}

	fseek(kscHandle, 0, SEEK_END);
	kscLen = static_cast<size_t>(ftell(kscHandle));
	if (kscLen < 8)
	{
		fclose(kscHandle);
		fclose(jpegHandle);
		return FALSE;
	}

	fseek(kscHandle, 0, SEEK_SET);

	jpegLen  = kscLen - 8;

	src_data = new uint8_t[kscLen];
	dst_data = new uint8_t[jpegLen];

	fread(src_data, kscLen, 1, kscHandle);
	fclose(kscHandle);
	kscHandle = nullptr;

	m_r       = 1124;
	for (i = 0; i < 4; i++)
		Decrypt(src_data[i]);

	uint8_t magic[4];
	for (i = 4; i < 8; i++)
		magic[i - 4] = Decrypt(src_data[i]);

	if (magic[0] == 'K' && magic[1] == 'S' && magic[2] == 'C' && magic[3] == 1)
	{
		//버전 1번
	}
	else
	{
		fclose(jpegHandle);

		delete[] dst_data;
		delete[] src_data;
		return FALSE;
	}

	for (j = 0; i < kscLen; i++, j++)
		dst_data[j] = Decrypt(src_data[i]);

	fwrite(dst_data, jpegLen, 1, jpegHandle);
	fclose(jpegHandle);

	delete[] dst_data;
	delete[] src_data;

	LoadJpegFile(szTempName);

	std::error_code ec;
	std::filesystem::remove(szTempName, ec);

	return TRUE;
}

BOOL CJpegFile::SaveFromDecryptToJpeg(const std::string& csKsc, const std::string& csJpeg)
{
	FILE* kscHandle   = nullptr;
	FILE* jpegHandle  = nullptr;
	uint8_t *dst_data = nullptr, *src_data = nullptr;
	size_t i = 0, j = 0;

#ifdef _MSC_VER
	fopen_s(&kscHandle, csKsc.c_str(), "rb");
#else
	kscHandle = fopen(csKsc.c_str(), "rb");
#endif

	if (kscHandle == nullptr)
		return FALSE;

#ifdef _MSC_VER
	fopen_s(&jpegHandle, csJpeg.c_str(), "wb");
#else
	jpegHandle = fopen(csJpeg.c_str(), "wb");
#endif

	if (jpegHandle == nullptr)
	{
		fclose(kscHandle);
		return FALSE;
	}

	size_t kscLen = 0;

	fseek(kscHandle, 0, SEEK_END);
	kscLen = static_cast<size_t>(ftell(kscHandle));
	if (kscLen < 8)
	{
		fclose(jpegHandle);
		fclose(kscHandle);
		return FALSE;
	}

	fseek(kscHandle, 0, SEEK_SET);

	size_t jpegLen = kscLen - 8;

	src_data       = new uint8_t[kscLen];
	dst_data       = new uint8_t[jpegLen];

	fread(src_data, kscLen, 1, kscHandle);
	fclose(kscHandle);
	kscHandle = nullptr;

	m_r       = 1124;
	for (i = 0; i < 8; i++)
		Decrypt(src_data[i]);

	for (j = 0; i < kscLen; i++, j++)
		dst_data[j] = Decrypt(src_data[i]);

	fwrite(dst_data, jpegLen, 1, jpegHandle);
	fclose(jpegHandle);

	delete[] dst_data;
	delete[] src_data;

	return TRUE;
}
