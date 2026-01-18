// N3FXPMeshInstance.cpp: implementation of the CN3FXPMeshInstance class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXPMeshInstance.h"

CN3FXPMeshInstance::CN3FXPMeshInstance()
{
	m_pFXPMesh       = nullptr;
	m_pIndices       = nullptr;
	m_pColorVertices = nullptr;
	m_iNumVertices   = 0;
	m_iNumIndices    = 0;
	m_pCollapseUpTo  = nullptr;
}

CN3FXPMeshInstance::CN3FXPMeshInstance(CN3FXPMesh* pN3FXPMesh)
{
	m_pFXPMesh       = nullptr;
	m_pIndices       = nullptr;
	m_pColorVertices = nullptr;
	m_iNumVertices   = 0;
	m_iNumIndices    = 0;
	m_pCollapseUpTo  = nullptr;

	CN3FXPMeshInstance::Create(pN3FXPMesh);
}

CN3FXPMeshInstance::CN3FXPMeshInstance(const std::string& szFN)
{
	m_pFXPMesh       = nullptr;
	m_pIndices       = nullptr;
	m_pColorVertices = nullptr;
	m_iNumVertices   = 0;
	m_iNumIndices    = 0;
	m_pCollapseUpTo  = nullptr;

	this->Create(szFN);
}

CN3FXPMeshInstance::~CN3FXPMeshInstance()
{
	if (m_pColorVertices)
	{
		delete[] m_pColorVertices;
		m_pColorVertices = nullptr;
	}
	if (m_pIndices)
	{
		delete[] m_pIndices;
		m_pIndices = nullptr;
	}

	s_MngFXPMesh.Delete(&m_pFXPMesh); //레퍼런스 카운트를 줄이기 위해
}

void CN3FXPMeshInstance::Release()
{
	delete[] m_pColorVertices;
	m_pColorVertices = nullptr;

	delete[] m_pIndices;
	m_pIndices = nullptr;

	s_MngFXPMesh.Delete(&m_pFXPMesh); //레퍼런스 카운트를 줄이기 위해

	m_pCollapseUpTo = nullptr;
	m_iNumVertices  = 0;
	m_iNumIndices   = 0;
}

bool CN3FXPMeshInstance::Create(CN3FXPMesh* pN3FXPMesh)
{
	if (pN3FXPMesh == nullptr)
	{
		CN3FXPMeshInstance::Release();
		return false;
	}

	m_pFXPMesh = pN3FXPMesh;
	if (m_pFXPMesh == nullptr)
	{
		__ASSERT(m_pFXPMesh, "Failed new CN3FXPMesh");
		return false;
	}

	m_szName            = pN3FXPMesh->m_szName;

	int iMaxNumVertices = m_pFXPMesh->GetMaxNumVertices();
	if (iMaxNumVertices > 0)
	{
		delete[] m_pColorVertices;
		m_pColorVertices = new __VertexXyzColorT1[iMaxNumVertices];
		__ASSERT(m_pColorVertices, "Failed to create Vertex buffer");
		CopyMemory(m_pColorVertices, m_pFXPMesh->m_pColorVertices,
			iMaxNumVertices * sizeof(__VertexXyzColorT1));
	}

	// And setup my instance-specific data.
	// We start with the lowest level of detail.
	int iMaxNumIndices = m_pFXPMesh->GetMaxNumIndices();
	if (iMaxNumIndices > 0)
	{
		delete[] m_pIndices;
		m_pIndices = new uint16_t[m_pFXPMesh->m_iMaxNumIndices];
		__ASSERT(m_pIndices, "Failed to create index buffer");
		CopyMemory(
			m_pIndices, m_pFXPMesh->m_pIndices, m_pFXPMesh->m_iMaxNumIndices * sizeof(uint16_t));
	}

	// lowest level of detail right now.
	m_iNumVertices  = m_pFXPMesh->GetMinNumVertices();
	m_iNumIndices   = m_pFXPMesh->GetMinNumIndices();

	// So far, we're at the last collapse
	m_pCollapseUpTo = m_pFXPMesh->m_pCollapses;

	return true;
}

bool CN3FXPMeshInstance::Create(const std::string& szFN)
{
	if (m_pFXPMesh != nullptr && m_pFXPMesh->FileName() == szFN)
		return true; // 파일 이름이 같으면 새로 만들지 않고 리턴하자

	CN3FXPMeshInstance::Release();

	CN3FXPMesh* pN3FXPMesh = s_MngFXPMesh.Get(szFN);
	return Create(pN3FXPMesh);
}

void CN3FXPMeshInstance::SetLODByNumVertices(int iNumVertices)
{
	if (m_pCollapseUpTo == nullptr)
		return;

	int iDiff = iNumVertices - m_iNumVertices;
	if (iDiff == 0)
		return;

	if (iDiff > 0)
	{
		while (iNumVertices > m_iNumVertices)
		{
			if (m_pCollapseUpTo->NumVerticesToLose + m_iNumVertices > iNumVertices)
				break; // 깜박임 방지 코드..

			if (!SplitOne())
				break;
		}
	}
	else if (iDiff < 0)
	{
		while (iNumVertices < m_iNumVertices)
		{
			if (!CollapseOne())
				break;
		}
	}

	while (m_pCollapseUpTo->bShouldCollapse)
	{
		if (!SplitOne())
			break;
	}
}

void CN3FXPMeshInstance::SetLOD(float value)
{
#define _USE_LODCONTROL_VALUE
#ifdef _USE_LODCONTROL_VALUE
	// value는 distance * FOV이다.
	if (m_pFXPMesh == nullptr)
		return;

	if (m_pFXPMesh->m_iLODCtrlValueCount == 0)
	{ // LODCtrlValue가 없으면 모두 그린다.
		SetLODByNumVertices(0x7fffffff);
		return;
	}

	__ASSERT(m_pFXPMesh->m_pLODCtrlValues, "LOD control value is NULL!");

	CN3PMesh::__LODCtrlValue* pTmpLODCV = m_pFXPMesh->m_pLODCtrlValues
										  + m_pFXPMesh->m_iLODCtrlValueCount - 1;

	if (value < m_pFXPMesh->m_pLODCtrlValues[0].fDist)
	{ // 최소 기준치보다 가까우므로 가장 많은 면으로 그린다.
		SetLODByNumVertices(m_pFXPMesh->m_pLODCtrlValues[0].iNumVertices);
	}
	else if (pTmpLODCV->fDist < value)
	{ // 최대 기준치보다 멀리 있으므로 가장 적은 면으로 그린다.
		SetLODByNumVertices(pTmpLODCV->iNumVertices);
	}
	else
	{ // 중간 값에 맞게 조정된 면 수로 그린다.
		for (int i = 1; i < m_pFXPMesh->m_iLODCtrlValueCount; ++i)
		{
			if (value < m_pFXPMesh->m_pLODCtrlValues[i].fDist)
			{
				CN3PMesh::__LODCtrlValue* pHiValue  = m_pFXPMesh->m_pLODCtrlValues + i;
				CN3PMesh::__LODCtrlValue* pLowValue = pHiValue - 1;
				float fVertices = (pHiValue->iNumVertices - pLowValue->iNumVertices)
								  * (value - pLowValue->fDist)
								  / (pHiValue->fDist - pLowValue->fDist);
				SetLODByNumVertices(pLowValue->iNumVertices + (int) fVertices);
				break;
			}
		}
	}
#else
	// value는 distance * FOV이다.
	if (m_pCollapseUpTo == nullptr || m_pFXPMesh == nullptr)
		return;

	const int iLODCtrlValueCount = 5;
	__PMLODCtrlValue LODCtrlValues[iLODCtrlValueCount];
	__PMLODCtrlValue* pTmpLODCV = &(LODCtrlValues[iLODCtrlValueCount - 1]);

	int iMaxNumVertices         = m_pFXPMesh->GetMaxNumVertices();
	int iMinNumVertices         = m_pFXPMesh->GetMinNumVertices();
	int iDiff                   = iMaxNumVertices - iMinNumVertices;

	float fVolume               = m_pFXPMesh->GetVolume();
	float fD                    = (sqrtf(fVolume) * 3.0f) / (value * 1.0f);
	fD                          = 1.0f;
	//	float fD = fVolume/(value*13.0f) * (400.0f/(float)iMaxNumVertices);
	if (fD > 1.0f)
		SetLODByNumVertices(iMaxNumVertices);
	else if (fD < 0.1f)
		SetLODByNumVertices(iMinNumVertices);
	else
		SetLODByNumVertices(iMinNumVertices + (int) (iDiff * fD));

#endif
}

bool CN3FXPMeshInstance::CollapseOne()
{
	if (m_pCollapseUpTo <= m_pFXPMesh->m_pCollapses)
		return false;

	m_pCollapseUpTo--;

	m_iNumIndices -= m_pCollapseUpTo->NumIndicesToLose;

	for (int* i = m_pFXPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges;
		i < m_pFXPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges
				+ m_pCollapseUpTo->NumIndicesToChange;
		i++)
	{
		m_pIndices[*i] = m_pCollapseUpTo->CollapseTo;
	}

	m_iNumVertices -= m_pCollapseUpTo->NumVerticesToLose;

	return true;
}

bool CN3FXPMeshInstance::SplitOne()
{
	if (m_pCollapseUpTo >= m_pFXPMesh->m_pCollapses + m_pFXPMesh->m_iNumCollapses)
		return false; // 이렇게 하면 포인터 하나가 삐져 나오게 된다..
	// 하지만 이렇게 다시 하는 이유는 아래 코드로 하면 마지막 폴리곤이 절대 그려지지 않는다.
	// 이렇게 해도 괜찮을 수 있도록 방어코드를 넣었다. m_pFXPMesh->m_pCollapses 를 할당할때 1개 더 할당하고 마지막 데이터를 초기값으로 넣었다.
	//	if (m_pCollapseUpTo >= m_pFXPMesh->m_pCollapses + m_pFXPMesh->m_iNumCollapses - 1) return false; // 이게 정상이다..

	m_iNumIndices  += m_pCollapseUpTo->NumIndicesToLose;
	m_iNumVertices += m_pCollapseUpTo->NumVerticesToLose;

	if (m_pFXPMesh->m_pAllIndexChanges)
	{
		for (int* i = m_pFXPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges;
			i < m_pFXPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges
					+ m_pCollapseUpTo->NumIndicesToChange;
			i++)
		{
			m_pIndices[*i] = m_iNumVertices - 1;
		}
	}

	m_pCollapseUpTo++;
	return true;
}

void CN3FXPMeshInstance::Render()
{
	if (m_pFXPMesh == nullptr)
		return;
	s_lpD3DDev->SetFVF(FVF_VNT1);

	const int iPCToRender = 1000; // primitive count to render
	if (m_iNumIndices > 3)
	{
		int iPC = m_iNumIndices / 3;

		int iLC = iPC / iPCToRender;
		int i   = 0;
		for (; i < iLC; ++i)
		{
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iPCToRender,
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pColorVertices,
				sizeof(__VertexXyzColorT1));
		}

		int iRPC = iPC % iPCToRender;
		if (iRPC > 0)
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iRPC,
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pColorVertices,
				sizeof(__VertexXyzColorT1));
	}
}

void CN3FXPMeshInstance::RenderTwoUV()
{
	if (nullptr == m_pFXPMesh)
		return;
	if (nullptr == m_pFXPMesh->GetVertices2())
	{
		m_pFXPMesh->GenerateSecondUV(); // 두번째 UV 가 없음 새로 만든다..
	}
	if (nullptr == m_pFXPMesh->GetVertices2())
		return;

	s_lpD3DDev->SetFVF(FVF_VNT2);

	const int iPCToRender = 1000; // primitive count to render
	if (m_iNumIndices > 3)
	{
		int iPC = m_iNumIndices / 3;

		int iLC = iPC / iPCToRender;
		int i   = 0;
		for (; i < iLC; ++i)
		{
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iPCToRender,
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pFXPMesh->m_pVertices2,
				sizeof(__VertexT2));
		}

		int iRPC = iPC % iPCToRender;
		if (iRPC > 0)
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iRPC,
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pFXPMesh->m_pVertices2,
				sizeof(__VertexT2));
	}
}

__VertexXyzColorT1* CN3FXPMeshInstance::GetVertices() const
{
	return m_pColorVertices;
}

void CN3FXPMeshInstance::SetColor(uint32_t dwColor)
{
	if (m_pColorVertices == nullptr)
		return;
	if (m_iNumVertices <= 0)
		return;

	for (int i = 0; i < m_iNumVertices; i++)
	{
		m_pColorVertices[i].color = dwColor;
	}
}
