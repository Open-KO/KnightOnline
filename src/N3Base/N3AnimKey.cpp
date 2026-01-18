// N3AnimKey.cpp: implementation of the CN3AnimKey class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3AnimKey.h"

CN3AnimKey::CN3AnimKey()
{
	m_eType         = KEY_VECTOR3;

	m_nCount        = 0; // 키 카운트
	m_fSamplingRate = 30.0;
	m_pDatas        = nullptr;
}

CN3AnimKey::~CN3AnimKey()
{
	// NOLINTNEXTLINE(clang-diagnostic-delete-incomplete)
	delete[] m_pDatas;
	m_pDatas = nullptr;
}

void CN3AnimKey::Release()
{
	m_nCount        = 0; // 키 카운트
	m_fSamplingRate = 30.0;

	// NOLINTNEXTLINE(clang-diagnostic-delete-incomplete)
	delete[] m_pDatas;
	m_pDatas = nullptr;

	CN3Base::Release();
}

void CN3AnimKey::Alloc(int nCount, float fSamplingRate, ANIMATION_KEY_TYPE eType)
{
	if (nCount <= 0)
		return;

	if (KEY_VECTOR3 != eType && KEY_QUATERNION != eType)
		return;

	if (fSamplingRate <= 0.0f)
	{
		__ASSERT(0, "Animation Sampling Rate 는 반드시 0 보다 커야 합니다.");
		return;
	}

	if (m_pDatas != nullptr)
	{
		// NOLINTNEXTLINE(clang-diagnostic-delete-incomplete)
		delete[] m_pDatas;
		m_pDatas = nullptr;
		m_nCount = 0;
	}

	m_eType         = eType;
	m_nCount        = nCount;
	m_fSamplingRate = fSamplingRate;

	if (KEY_VECTOR3 == m_eType)
	{
		m_pDatas = new __Vector3[nCount + 1]; // 한개 더 여유있게 할당.
		memset(m_pDatas, 0, sizeof(__Vector3) * (nCount + 1));
	}
	else if (KEY_QUATERNION == m_eType)
	{
		m_pDatas = new __Quaternion[nCount + 1]; // 한개 더 여유있게 할당.
		memset(m_pDatas, 0, sizeof(__Quaternion) * (nCount + 1));
	}
}

bool CN3AnimKey::Load(File& file)
{
	if (m_nCount > 0)
		Release();

	file.Read(&m_nCount, 4); // 키가 몇개 있는지

	// 키값을 파일에서 읽기..
	if (m_nCount > 0)
	{
		file.Read(&m_eType, 4);         // Key Type
		file.Read(&m_fSamplingRate, 4); // Sampling Rate

		Alloc(m_nCount, m_fSamplingRate, m_eType);

		if (KEY_VECTOR3 == m_eType)
		{
			file.Read(m_pDatas, sizeof(__Vector3) * m_nCount);
			__Vector3* pKeys = (__Vector3*) m_pDatas; // 끝에 하나더 복사해준다.
			pKeys[m_nCount]  = pKeys[m_nCount - 1];
		}
		else if (KEY_QUATERNION == m_eType)
		{
			file.Read(m_pDatas, sizeof(__Quaternion) * m_nCount);
			__Quaternion* pKeys = (__Quaternion*) m_pDatas; // 끝에 하나더 복사해준다.
			pKeys[m_nCount]     = pKeys[m_nCount - 1];
		}
	}

	return true;
}

#ifdef _N3TOOL
bool CN3AnimKey::Save(File& file)
{
	file.Write(&m_nCount, 4); // 키가 몇개 있는지

	if (m_nCount > 0)
	{
		file.Write(&m_eType, 4);         // Key Type
		file.Write(&m_fSamplingRate, 4); // Sampling Rate

		if (KEY_VECTOR3 == m_eType)
			file.Write(m_pDatas, sizeof(__Vector3) * m_nCount);
		else if (KEY_QUATERNION == m_eType)
			file.Write(m_pDatas, sizeof(__Quaternion) * m_nCount);
	}

	return true;
}
#endif // end of #ifdef _N3TOOL

/*
void CN3AnimKey::Translate(int nIndexStart, int nIndexEnd, float fDelta, int nKFlags)
{
	if(nIndexStart < 0 || nIndexStart >= m_nCount) return;
	if(nIndexEnd < 0 || nIndexEnd >= m_nCount) return;

	for(int i = nIndexStart; i <= nIndexEnd; i++)
	{
		if(nKFlags & 1) m_pDatas[i].x += fDelta;
		if(nKFlags & 2) m_pDatas[i].y += fDelta;
		if(nKFlags & 4) m_pDatas[i].z += fDelta;
	}
}
*/
#ifdef _N3TOOL
void CN3AnimKey::Add(CN3AnimKey& AKSrc, int nIndexS, int nIndexE)
{
	if (AKSrc.Count() <= 0)
		return;
	if (nIndexE < nIndexS)
		return;
	if (this->Type() != AKSrc.Type())
		return;

	int nAddCount  = nIndexE - nIndexS + 1; // 추가할 갯수

	int nPrevCount = m_nCount;
	void* pVBackup = nullptr;
	if (nPrevCount > 0) // backup
	{
		if (KEY_VECTOR3 == m_eType)
		{
			pVBackup = new __Vector3[nPrevCount];
			memcpy(pVBackup, m_pDatas, sizeof(__Vector3) * nPrevCount);
		}
		else if (KEY_QUATERNION == m_eType)
		{
			pVBackup = new __Quaternion[nPrevCount];
			memcpy(pVBackup, m_pDatas, sizeof(__Quaternion) * nPrevCount);
		}
	}

	this->Alloc(nPrevCount + nAddCount, AKSrc.SamplingRate(), AKSrc.Type()); // 새로 할당.
	if (nPrevCount > 0)
	{
		if (KEY_VECTOR3 == m_eType)
		{
			memcpy(m_pDatas, pVBackup, sizeof(__Vector3) * nPrevCount); // 백업받은걸 restore
		}
		else if (KEY_QUATERNION == m_eType)
		{
			memcpy(m_pDatas, pVBackup, sizeof(__Quaternion) * nPrevCount); // 백업받은걸 restore
		}
	}

	if (KEY_VECTOR3 == m_eType)
	{
		for (int i = 0; i < nAddCount; i++) // 추가.
		{
			__Vector3* pvTmp = (__Vector3*) AKSrc.DataGet(nIndexS + i);
			if (pvTmp)
				((__Vector3*) m_pDatas)[nPrevCount + i] = *pvTmp;
			else
				((__Vector3*) m_pDatas)[nPrevCount + i].Zero();
		}
	}
	else if (KEY_QUATERNION == m_eType)
	{
		for (int i = 0; i < nAddCount; i++) // 추가.
		{
			__Quaternion* pvTmp = (__Quaternion*) AKSrc.DataGet(nIndexS + i);
			if (pvTmp)
				((__Quaternion*) m_pDatas)[nPrevCount + i] = *pvTmp;
			else
			{
				((__Quaternion*) m_pDatas)[nPrevCount + i]
					.x                   = ((__Quaternion*) m_pDatas)[nPrevCount + i]
							 .y          = ((__Quaternion*) m_pDatas)[nPrevCount + i]
									  .z = ((__Quaternion*) m_pDatas)[nPrevCount + i].w = 0;
			}
		}
	}

	delete[] pVBackup;
	pVBackup = nullptr;
}
#endif // end of _N3TOOL

void CN3AnimKey::Duplicate(CN3AnimKey* pSrc)
{
	if (m_nCount > 0)
		this->Release();

	m_nCount = pSrc->Count();

	// 키값을 파일에서 읽기..
	if (m_nCount > 0)
	{
		m_eType         = pSrc->Type();
		m_fSamplingRate = pSrc->SamplingRate();

		this->Alloc(m_nCount, m_fSamplingRate, m_eType);

		if (KEY_VECTOR3 == m_eType)
		{
			char* pData = (char*) pSrc->GetDatas();
			memcpy(m_pDatas, pData, sizeof(__Vector3) * m_nCount);
		}
		else if (KEY_QUATERNION == m_eType)
		{
			char* pData = (char*) pSrc->GetDatas();
			memcpy(m_pDatas, pData, sizeof(__Quaternion) * m_nCount);
		}
	}
}

void CN3AnimKey::MultiplyDataVector(const __Vector3& vM)
{
	if (KEY_VECTOR3 != m_eType)
		return;

	for (int i = 0; i < m_nCount; i++)
	{
		__Vector3 vT                = ((__Vector3*) m_pDatas)[i];
		vT                         *= vM;
		((__Vector3*) m_pDatas)[i]  = vT;
	}
}
