// DTexGroup.h: interface for the CDTexGroup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DTEXGROUP_H__62BBF3A5_4AA0_416F_A7A3_F804F542FE2A__INCLUDED_)
#define AFX_DTEXGROUP_H__62BBF3A5_4AA0_416F_A7A3_F804F542FE2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyTerrainDef.h"
#include <list>

typedef std::list<LPDTEXTILEATTR>::iterator it_DTexTileAttr;

class CDTexGroup  
{
public:
	char		m_Name[40];
	int			m_ID;

	std::list<LPDTEXTILEATTR> m_Attributes[DTEX_MAX];
		
public:
	void Init();
	void Release();
	void DelAttrByDTexID(int DTexIndx);
	void SetAttr(int attr, __DTexTileAttr tile);
	void DelAttr(int attr, __DTexTileAttr tile);
	void ClearDTex();

	CDTexGroup();
	virtual ~CDTexGroup();

};

#endif // !defined(AFX_DTEXGROUP_H__62BBF3A5_4AA0_416F_A7A3_F804F542FE2A__INCLUDED_)
