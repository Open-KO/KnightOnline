// RotDummy.h: interface for the CRotDummy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROTDUMMY_H__305F5A6A_3477_4523_844F_3AADD7B5CA3D__INCLUDED_)
#define AFX_ROTDUMMY_H__305F5A6A_3477_4523_844F_3AADD7B5CA3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TransDummy.h"

class CRotDummy : public CTransDummy  
{
public:
	CRotDummy();
	virtual ~CRotDummy();

public:
protected:

public:
	void SetSelObj(CN3Transform* pObj);
	virtual BOOL MouseMsgFilter(LPMSG pMsg);
protected:
};

#endif // !defined(AFX_ROTDUMMY_H__305F5A6A_3477_4523_844F_3AADD7B5CA3D__INCLUDED_)
