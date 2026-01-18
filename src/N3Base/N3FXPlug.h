// N3FXPlug.h: interface for the CN3FXPlug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3FXPLUG_H__32183758_2BF2_456F_B6AA_CBE9C248CDDE__INCLUDED_)
#define AFX_N3FXPLUG_H__32183758_2BF2_456F_B6AA_CBE9C248CDDE__INCLUDED_

#pragma once

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
	~CN3FXPlugPart() override;

	// Attributes
public:
protected:
	class CN3FXBundle* m_pFXB;
	int m_nRefIndex;        // referance index (캐릭터 : joint index)

	__Vector3 m_vOffsetPos; // Joint와 떨어진 정도
	__Vector3 m_vOffsetDir; // Joint와 떨어진 방향

							// Operations
public:
	void Tick(const __Matrix44& mtxParent);
	void Tick(const class CN3Chr* pChr);
	void Render();
	void Release() override;
	bool Load(File& file) override;

	const CN3FXBundle* GetFXB() const
	{
		return m_pFXB;
	}
	void SetFXB(const std::string& strFN);
	int GetRefIndex() const
	{
		return m_nRefIndex;
	}
	void SetRefIdx(int nRefIndex)
	{
		m_nRefIndex = nRefIndex;
	}
	void StopFXB(bool bImmediately);
	void TriggerFXB();
#ifdef _N3TOOL
	bool Save(File& file) override;
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
	~CN3FXPlug() override;

	// Attributes
public:
protected:
	std::vector<class CN3FXPlugPart*> m_FXPParts;

	// Operations
public:
	void Tick(const CN3Chr* pChr);
	void Render();
	void Release() override;
	bool Load(File& file) override;

	void StopAll(bool bImmediately = false); // FX Stop
	void TriggerAll();                       // FX 시작

#ifdef _N3TOOL
	bool Save(File& file) override;
	void RemoveFXPParts_HaveNoBundle(); // 번들 없는 Part들 제거하기

	CN3FXPlugPart* FXPPartAdd();
	void FXPPartDelete(int nIndex);
#endif
protected:
};

#endif // !defined(AFX_N3FXPLUG_H__32183758_2BF2_456F_B6AA_CBE9C248CDDE__INCLUDED_)
