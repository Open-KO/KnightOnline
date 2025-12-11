// N3FXGroup.h: interface for the CN3FXGroup class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXGROUP_H__
#define __N3FXGROUP_H__

#include "N3BaseFileAccess.h"
#include "N3FXDef.h"

#include <list>

class CN3FXGroup : public CN3BaseFileAccess  
{
public:
	int			m_iVersion;
	std::list<__FXBInfo*> FXBList;
	
public:
	int	GetCount() const
	{
		return static_cast<int>(FXBList.size());
	}

	__FXBInfo* GetFXBInfo(int idx);

	virtual bool	Load(HANDLE hFile); // 핸들에서 읽어오기..
	bool	Save(HANDLE hFile);	// 저장하기..
	
	CN3FXGroup();	
	virtual ~CN3FXGroup();

#ifdef _N3TOOL	//툴에서만 쓰는 함수들...
public:
	bool	DecodeScriptFile(const char* lpPathName);
#endif // end of _N3TOOL
};

#endif // #ifndef __N3FXGROUP_H__
