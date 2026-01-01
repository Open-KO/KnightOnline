// DTex.cpp: implementation of the CDTex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "LyTerrainDef.h"
#include <N3Base/N3Texture.h>

#include "DTex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDTex::CDTex()
{
	m_pTex = nullptr;
	m_ID = -1;
	
	ZeroMemory(m_Attr, NUM_DTEXTILE*NUM_DTEXTILE*sizeof(DTEXATTR));
}

CDTex::~CDTex()
{
	delete m_pTex;
	m_pTex = nullptr;
}


//
//	Release..
//
void CDTex::Release()
{
	if(m_pTex)
	{
		m_pTex->Release();
		delete m_pTex;
		m_pTex = nullptr;
	}
}


//
//	Init..
//
void CDTex::Init()
{
	Release();

	if(!m_pTex) m_pTex = new CN3Texture;
	m_ID = -1;
	
	int x,z;
	for(x=0;x<NUM_DTEXTILE;x++)
	{
		for(z=0;z<NUM_DTEXTILE;z++)
		{
			m_Attr[x][z].Group = 0;
			m_Attr[x][z].Attr = x;
		}
	}
}


//
//	SetAttr...
//	타일하나에 속성 집어 넣기..
//
void CDTex::SetAttr(int x, int y, DTEXATTR attr)
{
	m_Attr[x][y] = attr;
}
