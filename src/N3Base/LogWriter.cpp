// LogWriter.cpp: implementation of the CLogWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3Base.h"
#include "LogWriter.h"

#include <FileIO/FileWriter.h>

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

	FileWriter file;
	if (!file.OpenExisting(s_szFileName))
	{
		if (!file.Create(s_szFileName))
			return;
	}

	auto fileSize = file.Size();

	// 파일 사이즈가 너무 크면 지운다..
	if (fileSize > 256'000)
	{
		file.Close();

		std::error_code ec;
		std::filesystem::remove(s_szFileName, ec);

		if (!file.Create(s_szFileName))
			return;
	}

	file.Seek(0, SEEK_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	std::string buff;
	SYSTEMTIME time;
	GetLocalTime(&time);

	buff = "---------------------------------------------------------------------------\r\n";
	file.Write(buff.data(), buff.length());

	buff = fmt::format("// Begin writing log... [{:02}/{:02} {:02}:{:02}]\r\n", time.wMonth,
		time.wDay, time.wHour, time.wMinute);
	file.Write(buff.data(), buff.length());
}

void CLogWriter::Close()
{
	FileWriter file;
	if (!file.OpenExisting(s_szFileName))
	{
		if (!file.Create(s_szFileName))
			return;
	}

	file.Seek(0, SEEK_END); // 추가 하기 위해서 파일의 끝으로 옮기고..

	std::string buff;
	SYSTEMTIME time;
	GetLocalTime(&time);

	buff = fmt::format("// End writing log... [{:02}/{:02} {:02}:{:02}]\r\n", time.wMonth,
		time.wDay, time.wHour, time.wMinute);
	file.Write(buff.data(), buff.length());

	buff = "---------------------------------------------------------------------------\r\n";
	file.Write(buff.data(), buff.length());
}

void CLogWriter::Write(const std::string_view message)
{
	if (s_szFileName.empty() || message.empty())
		return;

	FileWriter file;
	if (!file.OpenExisting(s_szFileName))
	{
		if (!file.Create(s_szFileName))
			return;
	}

	SYSTEMTIME time;
	GetLocalTime(&time);

	std::string outputMessage = fmt::format(
		"    [{:02}:{:02}:{:02}] {}\r\n", time.wHour, time.wMinute, time.wSecond, message);

	file.Seek(0, SEEK_END); // 추가 하기 위해서 파일의 끝으로 옮기고..
	file.Write(outputMessage.data(), outputMessage.length());
}
