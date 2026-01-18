// N3FXGroup.cpp: implementation of the CN3FXGroup class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXBundle.h"
#include "N3FXGroup.h"

#include <shared/StringUtils.h>

CN3FXGroup::CN3FXGroup()
{
	m_iVersion = 1;

	FXBList.clear();
}

CN3FXGroup::~CN3FXGroup()
{
	std::list<__FXBInfo*>::iterator it, ite;
	ite = FXBList.end();
	it  = FXBList.begin();

	while (it != ite)
	{
		__FXBInfo* pFXB = (*it);
		delete pFXB;
		it++;
	}

	FXBList.clear();
}

///////////////////////////////////////////////////////////////////////

bool CN3FXGroup::Load(File& file)
{
	file.Read(&m_iVersion, sizeof(int));

	int count;
	file.Read(&count, sizeof(int));

	for (int i = 0; i < count; i++)
	{
		__FXBInfo* pFXB = new __FXBInfo;
		file.Read(pFXB, sizeof(__FXBInfo));
		FXBList.push_back(pFXB);
	}
	return true;
}

bool CN3FXGroup::Save(File& file)
{
	file.Write(&m_iVersion, sizeof(int));

	int count = GetCount();
	file.Write(&count, sizeof(int));

	for (__FXBInfo* pFXB : FXBList)
		file.Write(pFXB, sizeof(__FXBInfo));

	return true;
}

//
//	decode script file..
//	스크립트 파일 읽고 해석시킴...
//
#ifdef _N3TOOL
bool CN3FXGroup::DecodeScriptFile(const char* lpPathName)
{
	FILE* stream = fopen(lpPathName, "r");
	if (!stream)
		return false;

	char szGamePathName[_MAX_PATH];
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpPathName, szDrive, szDir, szFName, szExt);
	_makepath(szGamePathName, szDrive, szDir, szFName, "fxg");

	CN3BaseFileAccess::FileNameSet(szGamePathName);

	char szLine[512] = "", szCommand[80] = "", szBuf[4][80] = { "", "", "", "" };
	char* pResult = fgets(szLine, 512, stream);
	sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

	if (lstrcmpi(szCommand, "<n3fxgroup>"))
	{
		fclose(stream);
		return false;
	}

	while (!feof(stream))
	{
		char* pResult = fgets(szLine, 512, stream);
		if (pResult == nullptr)
			continue;

		ZeroMemory(szCommand, 80);
		ZeroMemory(szBuf[0], 80);
		ZeroMemory(szBuf[1], 80);
		ZeroMemory(szBuf[2], 80);
		ZeroMemory(szBuf[3], 80);

		sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

		if (lstrcmpi(szCommand, "<fxb>") == 0)
		{
			__FXBInfo* pFXB = new __FXBInfo;
			strcpy_safe(pFXB->FXBName, szBuf[0]);
			pFXB->joint = atoi(szBuf[1]);
			if (lstrcmpi(szBuf[2], "TRUE") == 0)
				pFXB->IsLooping = TRUE;

			FXBList.push_back(pFXB);
			continue;
		}
	}
	fclose(stream);

	return true;
}
#endif // end of _N3TOOL

__FXBInfo* CN3FXGroup::GetFXBInfo(int idx)
{
	if (idx < 0 || idx >= static_cast<int>(FXBList.size()))
		return nullptr;

	std::list<__FXBInfo*>::iterator it;
	it = FXBList.begin();

	for (int i = 0; i < idx; i++)
		it++;
	return (*it);
}
