// N3FXPlug.cpp: implementation of the CN3FXPlug class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXPlug.h"
#include "N3FXBundle.h"
#include "N3Chr.h"

CN3FXPlugPart::CN3FXPlugPart() : m_vOffsetPos(0, 0, 0), m_vOffsetDir(0, 0, 1)
{
	m_dwType    |= OBJ_FX_PLUG_PART;
	m_pFXB       = nullptr;
	m_nRefIndex  = -1;
}

CN3FXPlugPart::~CN3FXPlugPart()
{
	delete m_pFXB;
}

void CN3FXPlugPart::Release()
{
	CN3BaseFileAccess::Release();

	delete m_pFXB;
	m_pFXB      = nullptr;
	m_nRefIndex = -1;
	m_vOffsetPos.Set(0, 0, 0);
	m_vOffsetDir.Set(0, 0, 1);
}

bool CN3FXPlugPart::Load(File& file)
{
	if (!CN3BaseFileAccess::Load(file))
		return false;

	__ASSERT(nullptr == m_pFXB, "must null");

	int nStrLen = 0;
	file.Read(&nStrLen, sizeof(nStrLen));
	if (nStrLen > 0)
	{
		char szFN[_MAX_PATH];
		file.Read(szFN, nStrLen);
		szFN[nStrLen] = '\0';

		m_pFXB        = new CN3FXBundle();
		if (!m_pFXB->LoadFromFile(szFN))
		{
			delete m_pFXB;
			m_pFXB = nullptr;
		}
		else
		{
			m_pFXB->Init();
			m_pFXB->Trigger();
		}
	}

	file.Read(&m_nRefIndex, sizeof(m_nRefIndex));
	file.Read(&m_vOffsetPos, sizeof(m_vOffsetPos));
	file.Read(&m_vOffsetDir, sizeof(m_vOffsetDir));

	return true;
}

#ifdef _N3TOOL
bool CN3FXPlugPart::Save(File& file)
{
	if (!CN3BaseFileAccess::Save(file))
		return false;

	__ASSERT(m_pFXB, "no FXB");

	int nStrLen = static_cast<int>(m_pFXB->FileName().size());
	file.Write(&nStrLen, sizeof(nStrLen));
	file.Write(m_pFXB->FileName().c_str(), nStrLen);
	file.Write(&m_nRefIndex, sizeof(m_nRefIndex));
	file.Write(&m_vOffsetPos, sizeof(m_vOffsetPos));
	file.Write(&m_vOffsetDir, sizeof(m_vOffsetDir));

	return true;
}
#endif

void CN3FXPlugPart::Tick(const __Matrix44& mtxParent)
{
	if (m_pFXB)
	{
		// 위치
		m_pFXB->m_vPos = m_vOffsetPos * mtxParent;

		// 회전
		static __Matrix44 mtxRot;
		mtxRot = mtxParent;
		mtxRot.PosSet(0, 0, 0);
		m_pFXB->m_vDir = m_vOffsetDir * mtxRot;

		m_pFXB->Tick();
	}
}

void CN3FXPlugPart::Tick(const CN3Chr* pChr)
{
	__ASSERT(pChr, "no chr");
	if (m_pFXB)
	{
		// 위치
		const __Matrix44* pMtxJoint = pChr->MatrixGet(m_nRefIndex);
		if (nullptr == pMtxJoint)
			return;

		static __Matrix44 mtx;
		mtx             = *(pMtxJoint);
		mtx            *= pChr->m_Matrix;
		m_pFXB->m_vPos  = m_vOffsetPos * mtx;

		// 회전
		mtx.PosSet(0, 0, 0);
		m_pFXB->m_vDir = m_vOffsetDir * mtx;

		m_pFXB->Tick();
	}
}

void CN3FXPlugPart::Render()
{
	if (m_pFXB)
		m_pFXB->Render();
}

void CN3FXPlugPart::SetFXB(const std::string& strFN)
{
	if (nullptr == m_pFXB)
		m_pFXB = new CN3FXBundle();
	else
		m_pFXB->Release();
	m_pFXB->LoadFromFile(strFN);

	m_vOffsetPos = m_pFXB->m_vPos; //일단 FXB에 설정되어 있는 vPos와 vDir값을 가져와서 적용.
	m_vOffsetDir = m_pFXB->m_vDir;

	m_pFXB->Init();                // FX 나오게 하기
	m_pFXB->Trigger();
}

void CN3FXPlugPart::StopFXB(bool bImmediately)
{
	if (m_pFXB)
		m_pFXB->Stop(bImmediately);
}

void CN3FXPlugPart::TriggerFXB()
{
	if (m_pFXB)
		m_pFXB->Trigger();
}

////////////////////////////////////////////////////////////////////////////////////
// CN3FXPlug
CN3FXPlug::CN3FXPlug()
{
	m_dwType |= OBJ_FX_PLUG;
}

CN3FXPlug::~CN3FXPlug()
{
	for (CN3FXPlugPart* pPart : m_FXPParts)
		delete pPart;
	m_FXPParts.clear();
}

void CN3FXPlug::Release()
{
	CN3BaseFileAccess::Release();

	for (CN3FXPlugPart* pPart : m_FXPParts)
		delete pPart;
	m_FXPParts.clear();
}

bool CN3FXPlug::Load(File& file)
{
	if (!CN3BaseFileAccess::Load(file))
		return false;

	__ASSERT(0 == m_FXPParts.size(), "must 0");

	int nCount = 0;
	file.Read(&nCount, sizeof(nCount)); // Part의 갯수

	if (nCount > 0)
		m_FXPParts.assign(nCount, nullptr);

	for (int i = 0; i < nCount; ++i)
	{
		m_FXPParts[i] = new CN3FXPlugPart();
		m_FXPParts[i]->Load(file);
	}
	return true;
}

void CN3FXPlug::Tick(const CN3Chr* pChr)
{
	if (pChr == nullptr)
		return;

	for (CN3FXPlugPart* pPart : m_FXPParts)
		pPart->Tick(pChr);
}

void CN3FXPlug::Render()
{
	for (CN3FXPlugPart* pPart : m_FXPParts)
		pPart->Render();
}

void CN3FXPlug::StopAll(bool bImmediately)
{
	for (CN3FXPlugPart* pPart : m_FXPParts)
		pPart->StopFXB(bImmediately);
}

void CN3FXPlug::TriggerAll()
{
	for (CN3FXPlugPart* pPart : m_FXPParts)
		pPart->TriggerFXB();
}

#ifdef _N3TOOL
bool CN3FXPlug::Save(File& file)
{
	if (!CN3BaseFileAccess::Save(file))
		return false;

	RemoveFXPParts_HaveNoBundle();       // 번들 없는 파트들 지우기

	int nCount = static_cast<int>(m_FXPParts.size());
	file.Write(&nCount, sizeof(nCount)); // Part의 갯수

	for (CN3FXPlugPart* pPart : m_FXPParts)
		pPart->Save(file);

	return true;
}

void CN3FXPlug::RemoveFXPParts_HaveNoBundle() // 번들 없는 Part들 제거하기
{
	for (auto itr = m_FXPParts.begin(); itr != m_FXPParts.end();)
	{
		CN3FXPlugPart* pPart = *itr;
		if (pPart == nullptr)
		{
			itr = m_FXPParts.erase(itr);
		}
		else if (pPart->GetFXB() == nullptr)
		{
			delete pPart; // FXB가 없으면 이 파트는 지운다.
			itr = m_FXPParts.erase(itr);
		}
		else
		{
			++itr;
		}
	}
}

CN3FXPlugPart* CN3FXPlug::FXPPartAdd()
{
	CN3FXPlugPart* pPart = new CN3FXPlugPart();
	m_FXPParts.push_back(pPart);
	return pPart;
}

void CN3FXPlug::FXPPartDelete(int nIndex)
{
	if (nIndex < 0 || nIndex >= static_cast<int>(m_FXPParts.size()))
		return;

	std::vector<CN3FXPlugPart*>::iterator itor = m_FXPParts.begin();
	for (int i = 0; i < nIndex; ++i)
		++itor;
	delete (*itor);
	m_FXPParts.erase(itor);
}

#endif

