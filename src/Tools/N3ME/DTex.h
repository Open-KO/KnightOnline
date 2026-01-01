// DTex.h: interface for the CDTexAttr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DTEXATTR_H__B351DEE0_1602_478C_B5EE_0AC30727C7E8__INCLUDED_)
#define AFX_DTEXATTR_H__B351DEE0_1602_478C_B5EE_0AC30727C7E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3Texture;
#include "LyTerrainDef.h"

class CDTex
{
private:
		
public:
	DTEXATTR		m_Attr[NUM_DTEXTILE][NUM_DTEXTILE];
	CN3Texture*		m_pTex;
	int				m_ID;
	
public:
	void Init();
	void Release();
	void SetAttr(int x, int y, DTEXATTR attr);

	CDTex();
	virtual ~CDTex();
};

#endif // !defined(AFX_DTEXATTR_H__B351DEE0_1602_478C_B5EE_0AC30727C7E8__INCLUDED_)
