// ScaleDummy.h: interface for the CScaleDummy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCALEDUMMY_H__53B4998F_13C1_456E_97D3_820A76BFC855__INCLUDED_)
#define AFX_SCALEDUMMY_H__53B4998F_13C1_456E_97D3_820A76BFC855__INCLUDED_

#pragma once

#include "TransDummy.h"

class CScaleDummy : public CTransDummy
{
public:
	CScaleDummy();
	virtual ~CScaleDummy();

public:
protected:
public:
	virtual BOOL MouseMsgFilter(LPMSG pMsg);

protected:
};

#endif // !defined(AFX_SCALEDUMMY_H__53B4998F_13C1_456E_97D3_820A76BFC855__INCLUDED_)
