// N3BaseFileAccess.h: interface for the CN3BaseFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3BASEFILEACCESS_H__C99953BD_12BE_4B37_823F_4F4B2379FF74__INCLUDED_)
#define AFX_N3BASEFILEACCESS_H__C99953BD_12BE_4B37_823F_4F4B2379FF74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3Base.h"
#include <string>

#include <FileIO/File.h>

//-----------------------------------------------------------------------------
#define N3FORMAT_VER_UNKN 0x00000000
#define N3FORMAT_VER_1068 0x00000001 // (1<<0)
#define N3FORMAT_VER_1264 0x00000002 // (1<<1)
#define N3FORMAT_VER_1298 0x00000004 // (1<<2)
#define N3FORMAT_VER_2062 0x00000008 // (1<<3)
#define N3FORMAT_VER_CURR 0x40000000 // NOTE: not even going to attempting this right now
#define N3FORMAT_VER_HERO 0x80000000 // NOTE: Hero Online formatting

static constexpr int N3FORMAT_VER_DEFAULT = N3FORMAT_VER_1264;//N3FORMAT_VER_1068;

//-----------------------------------------------------------------------------
class CN3BaseFileAccess : public CN3Base
{
protected:
	std::string		m_szFileName; // Base Path 를 제외한 로컬 경로 + 파일 이름

public:
	uint32_t m_iFileFormatVersion;
	int m_iLOD; // 로딩할때 쓸 LOD

public:
	const std::string& FileName() const { return m_szFileName; } // Full Path
	void FileNameSet(const std::string& szFileName);

	bool LoadFromFile(); // 파일에서 읽어오기.
	virtual bool LoadFromFile(const std::string& szFileName, uint32_t iVer = N3FORMAT_VER_DEFAULT); // 파일에서 읽어오기.
	virtual bool Load(File& file); // 핸들에서 읽어오기..

	virtual bool SaveToFile(); // 현재 파일 이름대로 저장.
	virtual bool SaveToFile(const std::string& szFileName); // 새이름으로 저장.
	virtual bool Save(File& file); // 핸들을 통해 저장..

public:
	void Release();

	CN3BaseFileAccess();
	virtual ~CN3BaseFileAccess();
};

#endif // !defined(AFX_N3BASEFILEACCESS_H__C99953BD_12BE_4B37_823F_4F4B2379FF74__INCLUDED_)
