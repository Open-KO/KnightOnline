////////////////////////////////////////////////////////////
//
//	JpegFile - A C++ class to allow reading and writing of
//	RGB and Grayscale JPEG images. (actually, it reads all forms
//	that the JPEG lib will decode into RGB or grayscale) and
//	writes only RGB and Grayscale.
//
//	It is based on a Win32 compilation of the IJG V.6a code.
//
//	This will only work on 32-bit Windows systems. I have only
//	tried this with Win 95, VC++ 4.1.
//
//	This class Copyright 1997, Chris Losinger
//	This is free to use and modify provided my name is included.
//
//	Comments:
//	Thanks to Robert Johnson for discovering a DWORD-alignment bug
//	Thanks to Lee Bode for catching a bug in CMfcappView::OnFileGetdimensionsjpg()
//
////////////////////////////////////////////////////////////
//
//	General Usage:
//
//	#include this file.
//	link with jpeglib2.lib
//
//	All functions here are static. There is no need to have a JpegFile object.
//	There is actually nothing in a JpegFile object anyway.
//
//	So, you can do this :
//
//		BOOL ok = JpegFile::vertFlipBuf(buf, widthbytes, height);
//
//	instead of this :
//
//		JpegFile jpgOb;
//		BOOL ok = jpgOb.vertFlipBuf(buf, widthbytes, height);
//
/////
//
//	Linking usage :
//	It is sometimes necessary to set /NODEFAULTLIB:LIBC (or LIBCD) to use this
//	class.
//
/////
//
//	Error reporting:
//	The class generates message boxes in response to JPEG errors.
//
//	The JpegFile.cpp fn my_error_exit defines the behavior for
//	fatal errors : show msg box, return to caller.
//
//	Warnings are handled by jpeglib.lib - which	generates msg boxes too.
//
////////////////////////////////////////////////////////////////

//
//	for DWORD aligning a buffer
//

#ifndef _INCLUDE_JPEGFILE
#define _INCLUDE_JPEGFILE

#include <cstdio>
#include <string>

#include <Windows.h>

enum e_JpegFileError : uint8_t
{
	ERR_MIN     = 0,       // All error #s >= this value
	ERR_NOT_DIB = 0,       // Tried to load a file, NOT a DIB!
	ERR_MEMORY,            // Not enough memory!
	ERR_READ,              // Error reading file!
	ERR_LOCK,              // Error on a GlobalLock()!
	ERR_OPEN,              // Error opening a file!
	ERR_CREATEPAL,         // Error creating palette.
	ERR_GETDC,             // Couldn't get a DC.
	ERR_CREATEDDB,         // Error create a DDB.
	ERR_STRETCHBLT,        // StretchBlt() returned failure.
	ERR_STRETCHDIBITS,     // StretchDIBits() returned failure.
	ERR_SETDIBITSTODEVICE, // SetDIBitsToDevice() failed.
	ERR_STARTDOC,          // Error calling StartDoc().
	ERR_NOGDIMODULE,       // Couldn't find GDI module in memory.
	ERR_SETABORTPROC,      // Error calling SetAbortProc().
	ERR_STARTPAGE,         // Error calling StartPage().
	ERR_NEWFRAME,          // Error calling NEWFRAME escape.
	ERR_ENDPAGE,           // Error calling EndPage().
	ERR_ENDDOC,            // Error calling EndDoc().
	ERR_SETDIBITS,         // Error calling SetDIBits().
	ERR_FILENOTFOUND,      // Error opening file in GetDib()
	ERR_INVALIDHANDLE,     // Invalid Handle
	ERR_DIBFUNCTION,       // Error on call to DIB function
	ERR_MAX                // All error #s < this value
};

class CJpegFile
{
public:
	////////////////////////////////////////////////////////////////
	// read a JPEG file to an RGB buffer - 3 bytes per pixel
	// returns a ptr to a buffer .
	// caller is responsible for cleanup!!!
	// BYTE *buf = JpegFile::JpegFileToRGB(....);
	// delete [] buf;

	BYTE* JpegFileToRGB(const std::string& fileName, // path to image
		UINT* width,                                 // image width in pixels
		UINT* height);                               // image height

	////////////////////////////////////////////////////////////////
	//	utility functions
	//	to do things like DWORD-align, flip, convert to grayscale, etc.
	//

	////////////////////////////////////////////////////////////////
	// allocates a DWORD-aligned buffer, copies data buffer
	// caller is responsible for delete []'ing the buffer

	BYTE* MakeDwordAlignedBuf(BYTE* dataBuf, // input buf
		UINT widthPix,                       // input pixels
		UINT height,                         // lines
		UINT* uiOutWidthBytes);              // new width bytes

	////////////////////////////////////////////////////////////////
	// vertically flip a buffer - for BMPs
	// in-place

	// note, this routine works on a buffer of width widthBytes: not a
	// buffer of widthPixels.
	BOOL VertFlipBuf(BYTE* inbuf, // input buf
		UINT widthBytes,          // input width bytes
		UINT height);             // height

	// NOTE :
	// the following routines do their magic on buffers with a whole number
	// of pixels per data row! these are assumed to be non DWORD-aligned buffers.

	////////////////////////////////////////////////////////////////
	// swap Red and Blue bytes
	// in-place

	BOOL BGRFromRGB(BYTE* buf, // input buf
		UINT widthPix,         // width in pixels
		UINT height);          // lines

	int PalEntriesOnDevice(HDC hDC);
	WORD DIBNumColors(LPSTR lpDIB);
	WORD PaletteSize(LPSTR lpDIB);
	HANDLE BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
	HBITMAP CopyScreenToBitmap(LPRECT lpRect);
	HANDLE CopyScreenToDIB(LPRECT lpRect);
	HANDLE AllocRoomForDIB(BITMAPINFOHEADER bi, HBITMAP hBitmap);
	HPALETTE GetSystemPalette(void);
	BOOL JpegFromDib(HANDLE hDib,         // Handle to DIB
		int nQuality,                     // JPEG quality (0-100)
		const std::string& csJpeg,        // Pathname to jpeg file
		const char** pcsMsg);             // Error msg to return
	virtual BOOL EncryptJPEG(HANDLE hDib, // Handle to DIB
		int nQuality,                     // JPEG quality (0-100)
		const std::string& csJpeg,        // Pathname to jpeg file
		const char** pcsMsg);             // Error msg to return

	BOOL SaveFromDecryptToJpeg(const std::string& csKsc, const std::string& csJpeg);

	virtual BOOL DecryptJPEG(const std::string& csJpeg);
	virtual BOOL LoadJpegFile(const std::string& /*csJpeg*/)
	{
		return TRUE;
	}

	virtual void DrawImage(HDC hDC)
	{
	}

	static WORD m_r;
	static WORD m_c1;
	static WORD m_c2;
	static BYTE Encrypt(BYTE plain)
	{
		BYTE cipher = 0;
		cipher      = (plain ^ (m_r >> 8));
		m_r         = (cipher + m_r) * m_c1 + m_c2;
		return cipher;
	}

	static BYTE Decrypt(BYTE cipher)
	{
		BYTE plain = 0;
		plain      = (cipher ^ (m_r >> 8));
		m_r        = (cipher + m_r) * m_c1 + m_c2;
		return plain;
	}

	////////////////////////////////////////////////////////////////
	// these do nothing
	CJpegFile();          // creates an empty object
	virtual ~CJpegFile(); // destroys nothing
};

#endif
