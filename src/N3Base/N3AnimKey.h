// AniKeyPos.h: interface for the CAniKeyPos class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3AnimKey_h__INCLUDED_)
#define AFX_N3AnimKey_h__INCLUDED_

#pragma once

#include "N3Base.h"

// NOLINTNEXTLINE(performance-enum-size): used by the file format, must be this size
enum ANIMATION_KEY_TYPE : uint32_t
{
	KEY_VECTOR3    = 0,
	KEY_QUATERNION = 1,
	KEY_UNKNOWN    = 0xFFFFFFFF
};

class CN3AnimKey : CN3Base
{
	friend class CN3Joint;

protected:
	ANIMATION_KEY_TYPE m_eType; // Key Type - Position Rotation Scale
	int m_nCount;               // 키 카운트
	float m_fSamplingRate;      // Sampling Rate - 표준은 30 Frame Per Sec 이다..
	void* m_pDatas;

public:
	// 키 형태, 벡터워 쿼터니언 형태가 있다..
	ANIMATION_KEY_TYPE Type() const
	{
		return m_eType;
	}

	void Add(CN3AnimKey& AKSrc, int nIndexS, int nIndexE);

	bool DataGet(float fFrm, __Vector3& v) const
	{
		if (KEY_VECTOR3 != m_eType)
			return false;

		if (m_nCount <= 0)
			return false;

		float fD   = 30.0f / m_fSamplingRate;

		int nIndex = (int) (fFrm * (m_fSamplingRate / 30.0f));
		if (nIndex < 0 || nIndex > m_nCount)
			return false;

		float fDelta = 0;
		if (nIndex == m_nCount)
		{
			nIndex = m_nCount - 1;
			fDelta = 0;
		}
		else
		{
			fDelta = (fFrm - nIndex * fD) / fD;
		}

		if (0.0f != fDelta)
		{
			v = (((__Vector3*) m_pDatas)[nIndex] * (1.0f - fDelta))
				+ (((__Vector3*) m_pDatas)[nIndex + 1] * fDelta);
		}
		else
			v = ((__Vector3*) m_pDatas)[nIndex];
		return true;
	}

	bool DataGet(float fFrm, __Quaternion& q) const
	{
		if (KEY_QUATERNION != m_eType)
			return false;

		if (m_nCount <= 0)
			return false;

		float fD   = 30.0f / m_fSamplingRate;

		int nIndex = (int) (fFrm * (m_fSamplingRate / 30.0f));
		if (nIndex < 0 || nIndex > m_nCount)
			return false;

		float fDelta = 0;
		if (nIndex == m_nCount)
		{
			nIndex = m_nCount - 1;
			fDelta = 0;
		}
		else
		{
			fDelta = (fFrm - nIndex * fD) / fD;
		}

		if (0.0f != fDelta)
			q.Slerp(
				((__Quaternion*) m_pDatas)[nIndex], ((__Quaternion*) m_pDatas)[nIndex + 1], fDelta);
		else
			q = ((__Quaternion*) m_pDatas)[nIndex];
		return true;
	}

	bool Load(File& file);

#ifdef _N3TOOL
	void* DataGet(int index)
	{
		if (index < 0 || index >= m_nCount)
			return nullptr;
		else if (KEY_VECTOR3 == m_eType)
			return &(((__Vector3*) m_pDatas)[index]);
		else if (KEY_QUATERNION == m_eType)
			return &(((__Quaternion*) m_pDatas)[index]);
		else
			return nullptr;
	}
	bool Save(File& file);
#endif // end of

	int Count() const
	{
		return m_nCount;
	}

	float SamplingRate() const
	{
		return m_fSamplingRate;
	}

	void Release() override;
	void Alloc(int nCount, float fSamplingRate = 30.0f, ANIMATION_KEY_TYPE eType = KEY_VECTOR3);

	void Duplicate(CN3AnimKey* pSrc);

	void* GetDatas()
	{
		return m_pDatas;
	}

	void MultiplyDataVector(const __Vector3& vM);

	CN3AnimKey();
	~CN3AnimKey() override;
};

#endif // !defined(AFX_N3AnimKey_h__INCLUDED_)
