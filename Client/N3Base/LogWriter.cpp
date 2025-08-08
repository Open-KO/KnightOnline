// LogWriter.cpp: implementation of the CLogWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include <stdio.h>
#include "N3Base.h"
#include "LogWriter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
std::string CLogWriter::s_szFileName;

CLogWriter::CLogWriter()
{
}

CLogWriter::~CLogWriter()
{
}

void CLogWriter::Open(const std::string& szFN)
{
	if (szFN.empty())
		return;

	s_szFileName = szFN;

	HANDLE hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
	}

	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
	if (dwSizeLow > 256000)  // 파일 사이즈가 너무 크면 지운다..
	{
		CloseHandle(hFile);
		::DeleteFile(s_szFileName.c_str());
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
	}

	::SetFilePointer(hFile, 0, nullptr, FILE_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	std::string buff;
	SYSTEMTIME time;
	GetLocalTime(&time);
	DWORD dwRWC = 0;

	buff = "---------------------------------------------------------------------------\r\n";
	WriteFile(hFile, buff.data(), static_cast<DWORD>(buff.length()), &dwRWC, nullptr);

	buff = fmt::format("// Begin writing log... [{:02}/{:02} {:02}:{:02}]\r\n",
		time.wMonth, time.wDay, time.wHour, time.wMinute);
	WriteFile(hFile, buff.data(), static_cast<DWORD>(buff.length()), &dwRWC, nullptr);

	CloseHandle(hFile);
}

void CLogWriter::Close()
{
	HANDLE hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
	}

	::SetFilePointer(hFile, 0, nullptr, FILE_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	std::string buff;
	SYSTEMTIME time;
	GetLocalTime(&time);
	DWORD dwRWC = 0;

	buff = fmt::format("// End writing log... [{:02}/{:02} {:02}:{:02}]\r\n",
		time.wMonth, time.wDay, time.wHour, time.wMinute);
	WriteFile(hFile, buff.data(), static_cast<DWORD>(buff.length()), &dwRWC, nullptr);

	buff = "---------------------------------------------------------------------------\r\n";
	WriteFile(hFile, buff.data(), static_cast<DWORD>(buff.length()), &dwRWC, nullptr);

	CloseHandle(hFile);
}

void CLogWriter::Write(const std::string_view message)
{
	if (s_szFileName.empty()
		|| message.empty())
		return;

	HANDLE hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
	}

	SYSTEMTIME time;
	GetLocalTime(&time);

	std::string outputMessage = fmt::format("    [{:02}:{:02}:{:02}] {}\r\n",
		time.wHour, time.wMinute, time.wSecond, message);

	::SetFilePointer(hFile, 0, nullptr, FILE_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	DWORD dwRWC = 0;
	WriteFile(hFile, outputMessage.data(), static_cast<DWORD>(outputMessage.length()), &dwRWC, nullptr);
	CloseHandle(hFile);
}
