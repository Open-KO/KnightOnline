// LogWriter.cpp: implementation of the CLogWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include <stdio.h>
#include "N3Base.h"
#include "LogWriter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//HANDLE CLogWriter::s_hFile = NULL;
std::string CLogWriter::s_szFileName = "";

CLogWriter::CLogWriter()
{
}

CLogWriter::~CLogWriter()
{
}

void CLogWriter::Open(const std::string& szFN)
{
//	if(s_hFile || szFN.empty()) return;
	if(szFN.empty()) return;

	s_szFileName = szFN;

	HANDLE hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(INVALID_HANDLE_VALUE == hFile) return;
	}

	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
	if(dwSizeLow > 256000)  // 파일 사이즈가 너무 크면 지운다..
	{
		CloseHandle(hFile);
		::DeleteFile(s_szFileName.c_str());
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(INVALID_HANDLE_VALUE == hFile) return;
	}

	::SetFilePointer(hFile, 0, NULL, FILE_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	char szBuff[1024];
	SYSTEMTIME time;
	GetLocalTime(&time);
	DWORD dwRWC = 0;

	sprintf(szBuff, "---------------------------------------------------------------------------\r\n");
	int iLength = lstrlen(szBuff);
	WriteFile(hFile, szBuff, iLength, &dwRWC, NULL);

	sprintf(szBuff, "// Begin writing log... [%.2d/%.2d %.2d:%.2d]\r\n", time.wMonth, time.wDay, time.wHour, time.wMinute);
	iLength = lstrlen(szBuff);
	WriteFile(hFile, szBuff, iLength, &dwRWC, NULL);

	CloseHandle(hFile);
}

void CLogWriter::Close()
{
	HANDLE hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(INVALID_HANDLE_VALUE == hFile) hFile = NULL;
	}

	if(hFile)
	{
		::SetFilePointer(hFile, 0, NULL, FILE_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

		char szBuff[1024];
		SYSTEMTIME time;
		GetLocalTime(&time);
		DWORD dwRWC = 0;

		sprintf(szBuff, "// End writing log... [%.2d/%.2d %.2d:%.2d]\r\n", time.wMonth, time.wDay, time.wHour, time.wMinute);
		int iLength = lstrlen(szBuff);
		WriteFile(hFile, szBuff, iLength, &dwRWC, NULL);

		sprintf(szBuff, "---------------------------------------------------------------------------\r\n");
		iLength = lstrlen(szBuff);
		WriteFile(hFile, szBuff, iLength, &dwRWC, NULL);

		CloseHandle(hFile);
		hFile = NULL;
	}
}

void CLogWriter::Write(const std::string_view message)
{
	if (s_szFileName.empty()
		|| message.empty())
		return;

	HANDLE hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = CreateFile(s_szFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE)
		return;

	SYSTEMTIME time;
	GetLocalTime(&time);

	std::string outputMessage = fmt::format("    [{:02}:{:02}:{:02}] {}\r\n",
		time.wHour, time.wMinute, time.wSecond, message);

	::SetFilePointer(hFile, 0, nullptr, FILE_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	DWORD dwRWC = 0;
	WriteFile(hFile, outputMessage.data(), static_cast<DWORD>(outputMessage.length()), &dwRWC, nullptr);
	CloseHandle(hFile);
}
