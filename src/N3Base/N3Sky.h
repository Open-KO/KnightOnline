// N3Sky.h: interface for the CN3Sky class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3SKY_H__0966C25E_2E50_48A2_9DF4_E4CD279E73CE__INCLUDED_)
#define AFX_N3SKY_H__0966C25E_2E50_48A2_9DF4_E4CD279E73CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3Base.h"
#include "N3ColorChange.h"

//class CN3Texture;
class CN3Sky : public CN3Base  
{
	friend class CN3SkyMng;
public:
	CN3Sky();
	virtual ~CN3Sky();

// Attributes
public:
protected:
	CN3ColorChange		m_SkyColor;
	CN3ColorChange		m_FogColor;

	__VertexXyzColor	m_vFronts[4];
	__VertexXyzColor	m_Bottom[4];

// Operations
public:
	void				Init();
	void				Render();
	void				Release() override;
	virtual				void Tick();
protected:
};

#endif // !defined(AFX_N3SKY_H__0966C25E_2E50_48A2_9DF4_E4CD279E73CE__INCLUDED_)
