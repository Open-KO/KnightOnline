// N3BaseFileAccess.cpp: implementation of the CN3BaseFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3BaseFileAccess.h"

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>

CN3BaseFileAccess::CN3BaseFileAccess()
{
	m_iFileFormatVersion  = N3FORMAT_VER_DEFAULT;

	m_dwType             |= OBJ_BASE_FILEACCESS;
	m_iLOD                = 0; // 로딩할때 쓸 LOD
}

CN3BaseFileAccess::~CN3BaseFileAccess()
{
}

void CN3BaseFileAccess::Release()
{
	m_szFileName.clear();
	m_iLOD = 0; // 로딩할때 쓸 LOD
	CN3Base::Release();
}

void CN3BaseFileAccess::FileNameSet(const std::string& szFileName)
{
	std::string szTmpFN = szFileName;

	if (!szTmpFN.empty())
		CharLower(&szTmpFN[0]);          // 모두 소문자로 만든다..

	size_t pos = szTmpFN.find(s_szPath); // 문자열에 Base Path 와 일치하는 이름이 있는지 본다.
	if (pos != std::string::npos)
		m_szFileName = szTmpFN.substr(s_szPath.size()); // 경로가 일치하면.. 긴경로는 짤라준다..
	else
		m_szFileName = szTmpFN;
}

bool CN3BaseFileAccess::LoadSupportedVersions(File& file)
{
	return Load(file);
}

bool CN3BaseFileAccess::Load(File& file)
{
	constexpr int MAX_SUPPORTED_NAME_LENGTH = 256;

	int nL                                  = 0;
	if (!file.Read(&nL, 4))
		throw std::runtime_error("CN3BaseFileAccess: failed to load length, EOF");

	if (nL < 0 || nL > MAX_SUPPORTED_NAME_LENGTH)
		throw std::runtime_error("CN3BaseFileAccess: invalid length");

	if (nL == 0)
		return true;

	size_t bytesRead = 0;
	m_szName.assign(nL, '\0');
	file.Read(&m_szName[0], nL, &bytesRead);

	return static_cast<int>(bytesRead) == nL;
}

bool CN3BaseFileAccess::LoadFromFile()
{
	if (m_szFileName.empty())
	{
#ifdef _N3GAME
		CLogWriter::Write("Can't open file (read)");
#endif
		return false;
	}

	std::string szFullPath;

	// 문자열에 ':', '\\', '//' 이 들어 있으면 전체 경로이다..
	if (m_szFileName.find(':') != std::string::npos
		|| m_szFileName.find("\\\\") != std::string::npos
		|| m_szFileName.find("//") != std::string::npos)
	{
		szFullPath = m_szFileName;
	}
	else
	{
		if (!s_szPath.empty())
			szFullPath = s_szPath;

		szFullPath += m_szFileName;
	}

	FileReader file;
	if (!file.OpenExisting(szFullPath))
	{
		std::string szErr = szFullPath + " - Can't open file (read)";
#ifdef _N3GAME
		CLogWriter::Write(szErr);
#endif
#ifdef _N3TOOL
		MessageBox(s_hWndBase, szErr.c_str(), "File Handle error", MB_OK);
#endif
		return false;
	}

	try
	{
		if (LoadSupportedVersions(file))
			return true;
	}
	catch (const std::exception& ex)
	{
		std::string szErr = szFullPath + " - Failed to read file (" + std::string(ex.what()) + ")";
#ifdef _N3GAME
		CLogWriter::Write(szErr);
#endif
#ifdef _N3TOOL
		MessageBox(s_hWndBase, szErr.c_str(), "Failed to read file", MB_OK);
#endif
	}

	// File failed to load. To avoid it being in an unexpected inconsistent state,
	// we should release it.
	Release();
	return false;
}

bool CN3BaseFileAccess::LoadFromFile(const std::string& szFileName)
{
	FileNameSet(szFileName);
	return LoadFromFile();
}

bool CN3BaseFileAccess::SaveToFile()
{
	if (m_szFileName.empty())
	{
		std::string szErr = m_szName + " Can't open file (write) - NULL String";
		MessageBox(s_hWndBase, szErr.c_str(), "File Open Error", MB_OK);
		return false;
	}

	std::string szFullPath;

	// 문자열에 ':', '\\', '//' 이 들어 있으면 전체 경로이다..
	if (m_szFileName.find(':') != std::string::npos
		|| m_szFileName.find("\\\\") != std::string::npos
		|| m_szFileName.find("//") != std::string::npos)
	{
		szFullPath = m_szFileName;
	}
	else
	{
		if (!s_szPath.empty())
			szFullPath = s_szPath;

		szFullPath += m_szFileName;
	}

	FileWriter file;
	if (!file.Create(szFullPath))
	{
		std::string szErr = szFullPath + " - Can't open file(write)";
		MessageBox(s_hWndBase, szErr.c_str(), "File Handle error", MB_OK);
		return false;
	}

	Save(file);
	return true;
}

bool CN3BaseFileAccess::SaveToFile(const std::string& szFileName)
{
	FileNameSet(szFileName);
	return SaveToFile();
}

bool CN3BaseFileAccess::Save(File& file)
{
	int nL = static_cast<int>(m_szName.size());
	file.Write(&nL, 4);
	if (nL > 0)
		file.Write(m_szName.c_str(), nL);

	return true;
}
