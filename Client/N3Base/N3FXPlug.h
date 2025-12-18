// N3FXPlug.h: interface for the CN3FXPlug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3FXPLUG_H__32183758_2BF2_456F_B6AA_CBE9C248CDDE__INCLUDED_)
#define AFX_N3FXPLUG_H__32183758_2BF2_456F_B6AA_CBE9C248CDDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////////////
// CN3FXPlugPart
#include "N3BaseFileAccess.h"

#include <vector>

class CN3FXPlugPart : public CN3BaseFileAccess  
{
#ifdef _N3TOOL
	friend class CFormViewProperty;
	friend class CN3CEView;
#endif

public:
	CN3FXPlugPart();
	virtual ~CN3FXPlugPart();

// Attributes
public:
protected:
	class CN3FXBundle*	m_pFXB;
	int					m_nRefIndex;	// referance index (캐릭터 : joint index)

	__Vector3			m_vOffsetPos;	// Joint와 떨어진 정도
	__Vector3			m_vOffsetDir;	// Joint와 떨어진 방향

// Operations
public:
	void				Tick(const __Matrix44& mtxParent);
	void				Tick(const class CN3Chr* pChr);
	void				Render();
	virtual void		Release();
	virtual bool		Load(File& file);

	const CN3FXBundle*	GetFXB() const {return m_pFXB;}
	void				SetFXB(const std::string& strFN);
	int					GetRefIndex() const {return m_nRefIndex;}
	void				SetRefIdx(int nRefIndex) {m_nRefIndex = nRefIndex;}
	void				StopFXB(bool bImmediately);
	void				TriggerFXB();
#ifdef _N3TOOL
	virtual bool		Save(HANDLE hFile);
#endif
protected:
};

////////////////////////////////////////////////////////////////////////////////////
// CN3FXPlug
class CN3FXPlug : public CN3BaseFileAccess  
{
#ifdef _N3TOOL
	friend class CFormViewProperty;
#endif

public:
	CN3FXPlug();
	virtual ~CN3FXPlug();

// Attributes
public:
protected:
	std::vector<class CN3FXPlugPart*> m_FXPParts;

// Operations
public:
	void			Tick(const CN3Chr* pChr);
	void			Render();
	virtual void	Release();
	virtual bool	Load(File& file);

	void			StopAll(bool bImmediately = false);	// FX Stop
	void			TriggerAll();						// FX 시작

#ifdef _N3TOOL
	virtual bool	Save(HANDLE hFile);
	void			RemoveFXPParts_HaveNoBundle();		// 번들 없는 Part들 제거하기

	CN3FXPlugPart*	FXPPartAdd();
	void			FXPPartDelete(int nIndex);
#endif
protected:
};

#endif // !defined(AFX_N3FXPLUG_H__32183758_2BF2_456F_B6AA_CBE9C248CDDE__INCLUDED_)
