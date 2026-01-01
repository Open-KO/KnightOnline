// PosDummy.h: interface for the CPosDummy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POSDUMMY_H__61F97647_F058_413E_A1C2_1F153B020C9D__INCLUDED_)
#define AFX_POSDUMMY_H__61F97647_F058_413E_A1C2_1F153B020C9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TransDummy.h"

class CPosDummy : public CTransDummy  
{
public:
	CPosDummy();
	virtual ~CPosDummy();

public:
protected:

public:
	virtual void SetVertex(__Vector3* pVertex);
	virtual void SetSelObj(CN3Transform* pObj);
	virtual BOOL MouseMsgFilter(LPMSG pMsg);
protected:

};

#endif // !defined(AFX_POSDUMMY_H__61F97647_F058_413E_A1C2_1F153B020C9D__INCLUDED_)
