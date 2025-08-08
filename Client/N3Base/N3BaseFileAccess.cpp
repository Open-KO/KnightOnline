// N3BaseFileAccess.cpp: implementation of the CN3BaseFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3BaseFileAccess.h"
#include <vector>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CN3BaseFileAccess::CN3BaseFileAccess()
{
	m_iFileFormatVersion = N3FORMAT_VER_UNKN;

	m_dwType |= OBJ_BASE_FILEACCESS;
	m_szFileName = "";
	m_iLOD = 0; // 로딩할때 쓸 LOD
}

CN3BaseFileAccess::~CN3BaseFileAccess()
{
}

void CN3BaseFileAccess::Release()
{
	m_szFileName = "";
	m_iLOD = 0; // 로딩할때 쓸 LOD
	CN3Base::Release();
}

void CN3BaseFileAccess::FileNameSet(const std::string& szFileName)
{
	std::string szTmpFN = szFileName;

	if(!szTmpFN.empty()) CharLower(&(szTmpFN[0])); // 모두 소문자로 만든다..
 	int iPos = szTmpFN.find(s_szPath); // 문자열에 Base Path 와 일치하는 이름이 있는지 본다.
	if(iPos >= 0)
		m_szFileName = szTmpFN.substr(s_szPath.size()); // 경로가 일치하면.. 긴경로는 짤라준다..
	else 
		m_szFileName = szTmpFN;
}

bool CN3BaseFileAccess::Load(HANDLE hFile)
{
	if(m_iFileFormatVersion == N3FORMAT_VER_UNKN) {
		// NOTE: unknow version format
		printf("Unknown version type\n");
	}

	m_szName = "";

	DWORD dwRWC = 0;
	int nL = 0;
	ReadFile(hFile, &nL, 4, &dwRWC, NULL);
	if(nL > 0) 
	{
		std::vector<char> buffer(nL+1, NULL);
		ReadFile(hFile, &buffer[0], nL, &dwRWC, NULL);
		m_szName = &buffer[0];
	}

	return true;
}

bool CN3BaseFileAccess::LoadFromFile()
{
	if(m_szFileName.size() <= 0)
	{
#ifdef _N3GAME
		CLogWriter::Write("Can't open file (read)");
#endif
		return false;
	}

	std::string szFullPath;
	if(-1 != m_szFileName.find(':') || -1 != m_szFileName.find("\\\\") || -1 != m_szFileName.find("//")) // 문자열에 ':', '\\', '//' 이 들어 있으면 전체 경로이다..
	{
		szFullPath = m_szFileName;
	}
	else
	{
		if(NULL != s_szPath.size() > 0) szFullPath = s_szPath;
		szFullPath += m_szFileName;
	}

	DWORD dwRWC = 0;
	HANDLE hFile = ::CreateFile(szFullPath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(INVALID_HANDLE_VALUE == hFile)
	{
		std::string szErr = szFullPath + " - Can't open file (read)";
#ifdef _N3TOOL
		MessageBox(s_hWndBase, szErr.c_str(), "File Handle error", MB_OK);
#endif
#ifdef _N3GAME 
		CLogWriter::Write(szErr);
#endif
		return false;
	}

	bool bSuccess =	this->Load(hFile);

	CloseHandle(hFile);

	return bSuccess;
}

bool CN3BaseFileAccess::LoadFromFile(const std::string& szFileName, uint32_t iVer)
{
	m_iFileFormatVersion = iVer;
	this->FileNameSet(szFileName);
	return this->LoadFromFile();
}

bool CN3BaseFileAccess::SaveToFile()
{
	if(m_szFileName.size() <= 0)
	{
		std::string szErr = m_szName + " Can't open file (write) - NULL String";
		MessageBox(s_hWndBase, szErr.c_str(), "File Open Error", MB_OK);
		return false;
	}

	std::string szFullPath;
	if(-1 != m_szFileName.find(':') || -1 != m_szFileName.find("\\\\") || -1 != m_szFileName.find("//")) // 문자열에 ':', '\\', '//' 이 들어 있으면 전체 경로이다..
	{
		szFullPath = m_szFileName;
	}
	else
	{
		if(s_szPath.size() > 0) szFullPath = s_szPath;
		szFullPath += m_szFileName;
	}

	DWORD dwRWC = 0;
	HANDLE hFile = ::CreateFile(szFullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		std::string szErr = szFullPath + " - Can't open file(write)";
		MessageBox(s_hWndBase, szErr.c_str(), "File Handle error", MB_OK);
		return false;
	}

	this->Save(hFile);

	CloseHandle(hFile);
	return true;
}

bool CN3BaseFileAccess::SaveToFile(const std::string& szFileName)
{
	this->FileNameSet(szFileName);
	return this->SaveToFile();
}

bool CN3BaseFileAccess::Save(HANDLE hFile)
{
	DWORD dwRWC = 0;

	int nL = m_szName.size();
	WriteFile(hFile, &nL, 4, &dwRWC, NULL);
	if(nL > 0) WriteFile(hFile, m_szName.c_str(), nL, &dwRWC, NULL);

	return true;
}
