// BitMapFile.cpp: implementation of the CBitMapFile class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "BitMapFile.h"

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>

CBitMapFile::CBitMapFile()
{
	m_pPixels = nullptr;
	Release();
}

CBitMapFile::~CBitMapFile()
{
	Release();
}

void CBitMapFile::Release()
{
	memset(&m_bmfHeader, 0, sizeof(m_bmfHeader));
	memset(&m_bmInfoHeader, 0, sizeof(m_bmInfoHeader));
	::GlobalFree(m_pPixels); // 실제 픽셀 데이터
	m_pPixels = nullptr;
}

bool CBitMapFile::Load(File& file)
{
	Release(); // 일단 다 해제하고..

	// 파일 헤더 읽기
	file.Read(&m_bmfHeader, sizeof(m_bmfHeader));

	// bmp 파일임을 나타내는 "BM"마커 확인
	if (m_bmfHeader.bfType != 0x4D42)
	{
		MessageBoxW(::GetActiveWindow(), L"원본 파일이 bitmap파일이 아닙니다.", L"error", MB_OK);
		return false;
	}

	// BITMAPINFOHEADER 얻기
	file.Read(&m_bmInfoHeader, sizeof(m_bmInfoHeader));

	// 픽셀당 비트 수 확인
	uint16_t wBitCount = m_bmInfoHeader.biBitCount;
	if (24 != wBitCount || m_bmInfoHeader.biWidth <= 0
		|| m_bmInfoHeader.biHeight <= 0) // 24비트 bmp가 아니면 return해 버린다.
	{
		MessageBoxW(::GetActiveWindow(),
			L"원본 bitmap이 너비, 높이에 이상이 있거나 24bit파일이 아닙니다.", L"error", MB_OK);
		return false;
	}

	// 실제 이미지의 메모리상에 잡힌 가로 길이 (24bit)
	int iRealWidth = ((int) ((m_bmInfoHeader.biWidth * 3 + 3) / 4)) * 4;

	// 새로 만들 이미지 메모리 할당
	int iDIBSize   = iRealWidth * m_bmInfoHeader.biHeight;

	m_pPixels      = ::GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, iDIBSize);
	if (m_pPixels == nullptr)
	{
		MessageBoxW(::GetActiveWindow(), L"메모리를 할당하지 못했습니다.", L"error", MB_OK);
		return false;
	}

	uint8_t* pixelBuffer = reinterpret_cast<uint8_t*>(m_pPixels);

	// 픽셀을 읽는다..
	for (int y = m_bmInfoHeader.biHeight - 1; y >= 0; y--) // 비트맵은 위아래가 거꾸로 있다..
		file.Read(&pixelBuffer[y * iRealWidth], iRealWidth);

	return true;
}

void* CBitMapFile::Pixels(int x, int y)
{
	if (24 != m_bmInfoHeader.biBitCount)
		return nullptr;

	int nPitch           = Pitch();
	int nPixelSize       = 3;

	uint8_t* pixelBuffer = reinterpret_cast<uint8_t*>(m_pPixels);
	return &pixelBuffer[y * nPitch + x * nPixelSize];
}

bool CBitMapFile::Save(File& file)
{
	// 파일 헤더 쓰기
	file.Write(&m_bmfHeader, sizeof(m_bmfHeader));

	// BITMAPINFOHEADER 쓰기
	file.Write(&m_bmInfoHeader, sizeof(m_bmInfoHeader));

	// 실제 이미지의 메모리상에 잡힌 가로 길이 (24bit)
	int iRealWidth             = this->Pitch();

	const uint8_t* pixelBuffer = reinterpret_cast<uint8_t*>(m_pPixels);

	// 픽셀을 저장한다...
	for (int y = m_bmInfoHeader.biHeight - 1; y >= 0; y--) // 비트맵은 위아래가 거꾸로 있다..
		file.Write(&pixelBuffer[y * iRealWidth], iRealWidth);

	return true;
}

bool CBitMapFile::SaveRectToFile(const std::string& szFN, RECT rc)
{
	if (szFN.empty())
		return false;

	if (rc.right <= rc.left)
		return false;

	if (rc.bottom <= rc.top)
		return false;

	if (rc.left < 0)
		rc.left = 0;

	if (rc.top < 0)
		rc.top = 0;

	if (rc.right > m_bmInfoHeader.biWidth)
		rc.right = m_bmInfoHeader.biWidth;

	if (rc.bottom > m_bmInfoHeader.biHeight)
		rc.bottom = m_bmInfoHeader.biHeight;

	int nWidth  = rc.right - rc.left;
	int nHeight = rc.bottom - rc.top;

	if (nWidth <= 0 || nHeight <= 0)
	{
		MessageBoxW(::GetActiveWindow(), L"가로 세로가 0이하인 bitmap으로 저장할수 없습니다.",
			L"error", MB_OK);
		return false;
	}

	FileWriter file;

	// 쓰기 모드로 파일 열기
	if (!file.Create(szFN))
	{
		MessageBoxW(::GetActiveWindow(), L"원본 bitmap을 열 수 없습니다.", L"error", MB_OK);
		return false;
	}

	// 실제 이미지의 메모리상에 잡힌 가로 길이 (24bit)
	int iRealWidthDest                = ((int) ((nWidth * 3 + 3) / 4)) * 4;
	int iDestDIBSize                  = sizeof(BITMAPINFOHEADER) + iRealWidthDest * nHeight;

	// 새로 만들 이미지 file header 정보 채우기
	BITMAPFILEHEADER bmfHeaderDest    = m_bmfHeader;
	bmfHeaderDest.bfType              = 0x4D42; // "BM"
	bmfHeaderDest.bfSize              = sizeof(bmfHeaderDest) + iDestDIBSize;
	bmfHeaderDest.bfOffBits           = sizeof(bmfHeaderDest) + sizeof(BITMAPINFOHEADER);

	// 새로 만들 이미지 bitmap info header 정보 채우기
	BITMAPINFOHEADER bmInfoHeaderDest = m_bmInfoHeader;
	bmInfoHeaderDest.biSize           = sizeof(bmInfoHeaderDest);
	bmInfoHeaderDest.biWidth          = nWidth;
	bmInfoHeaderDest.biHeight         = nHeight;
	bmInfoHeaderDest.biPlanes         = 1;
	bmInfoHeaderDest.biSizeImage      = iRealWidthDest * nHeight;

	// 파일 헤더 쓰기
	file.Write(&bmfHeaderDest, sizeof(bmfHeaderDest));

	// BITMAPINFOHEADER 쓰기
	file.Write(&bmInfoHeaderDest, sizeof(bmInfoHeaderDest));

	// 픽셀을 저장한다...
	int iRealWidth = ((int) ((m_bmInfoHeader.biWidth * 3 + 3) / 4)) * 4;
	for (int y = rc.bottom - 1; y >= rc.top; y--)
	{
		void* pPixelDest = ((uint8_t*) m_pPixels) + iRealWidth * y + (rc.left * 3);
		file.Write(pPixelDest, iRealWidthDest); // 라인 쓰기..
	}

	return true;
}

bool CBitMapFile::LoadFromFile(const char* pszFN)
{
	if (pszFN == nullptr || lstrlen(pszFN) <= 0)
		return false;

	FileReader file;
	if (!file.OpenExisting(pszFN))
		return false;

	return Load(file);
}

bool CBitMapFile::SaveToFile(const char* pszFN)
{
	if (pszFN == nullptr || lstrlen(pszFN) <= 0)
		return false;

	FileWriter file;
	if (!file.Create(pszFN))
		return false;

	return Save(file);
}

bool CBitMapFile::Create(int nWidth, int nHeight, int nBPP)
{
	if (nWidth <= 0 || nHeight <= 0)
		return false;
	this->Release(); // 일단 다 해제하고..

	if (24 != nBPP)
		return FALSE;

	int iRealWidth = ((nWidth * 3 + 3) / 4) * 4; // 실제 이미지의 메모리상에 잡힌 가로 길이 (24bit)
	int iDIBSize   = iRealWidth * nHeight;       // 새로 만들 이미지 메모리 할당

	m_pPixels      = ::GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, iDIBSize);
	if (m_pPixels == nullptr)
	{
		MessageBoxW(::GetActiveWindow(), L"메모리를 할당하지 못했습니다.", L"error", MB_OK);
		return false;
	}

	memset(m_pPixels, 0, iDIBSize);

	// 새로 만들 이미지 file header 정보 채우기
	m_bmfHeader.bfType         = 0x4D42; // "BM"
	m_bmfHeader.bfSize         = sizeof(m_bmfHeader) + iDIBSize;
	m_bmfHeader.bfOffBits      = sizeof(m_bmfHeader) + sizeof(BITMAPINFOHEADER);

	// 새로 만들 이미지 bitmap info header 정보 채우기
	m_bmInfoHeader.biSize      = sizeof(m_bmInfoHeader);
	m_bmInfoHeader.biWidth     = nWidth;
	m_bmInfoHeader.biHeight    = nHeight;
	m_bmInfoHeader.biBitCount  = nBPP;
	m_bmInfoHeader.biPlanes    = 1;
	m_bmInfoHeader.biSizeImage = iRealWidth * nHeight;

	return true;
}
