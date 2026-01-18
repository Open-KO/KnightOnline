// N3PMeshInstance.cpp: implementation of the CN3PMeshInstance class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3PMeshInstance.h"

//
// an instance of a mesh. Each version of each mesh that is rendered at a
// separate level of detail needs one of these.

// Each instance is tied to the original mesh that it represents.
CN3PMeshInstance::CN3PMeshInstance()
{
	m_pPMesh        = nullptr;
	m_pIndices      = nullptr;
	m_iNumVertices  = 0;
	m_iNumIndices   = 0;
	m_pCollapseUpTo = nullptr;
}

CN3PMeshInstance::CN3PMeshInstance(CN3PMesh* pN3PMesh)
{
	m_pPMesh        = nullptr;

	m_pIndices      = nullptr;
	m_iNumVertices  = 0;
	m_iNumIndices   = 0;
	m_pCollapseUpTo = nullptr;

	CN3PMeshInstance::Create(pN3PMesh);
}

CN3PMeshInstance::CN3PMeshInstance(const std::string& szFN)
{
	m_pPMesh        = nullptr;

	m_pIndices      = nullptr;
	m_iNumVertices  = 0;
	m_iNumIndices   = 0;
	m_pCollapseUpTo = nullptr;

	this->Create(szFN);
}

CN3PMeshInstance::~CN3PMeshInstance()
{
	delete[] m_pIndices;
	m_pIndices = nullptr;
	s_MngPMesh.Delete(&m_pPMesh);
}

void CN3PMeshInstance::Release()
{
	delete[] m_pIndices;
	m_pIndices = nullptr;

	s_MngPMesh.Delete(&m_pPMesh);
	m_pCollapseUpTo = nullptr;
	m_iNumVertices  = 0;
	m_iNumIndices   = 0;
}

bool CN3PMeshInstance::Create(CN3PMesh* pN3PMesh)
{
	if (pN3PMesh == nullptr)
	{
		CN3PMeshInstance::Release();
		return false;
	}

	m_pPMesh           = pN3PMesh;
	m_szName           = pN3PMesh->m_szName;

	// And setup my instance-specific data.
	// We start with the lowest level of detail.
	int iMaxNumIndices = m_pPMesh->GetMaxNumIndices();
	if (iMaxNumIndices > 0)
	{
		delete[] m_pIndices;
		m_pIndices = new uint16_t[m_pPMesh->m_iMaxNumIndices];
		__ASSERT(m_pIndices, "Failed to create index buffer");
		CopyMemory(m_pIndices, m_pPMesh->m_pIndices, m_pPMesh->m_iMaxNumIndices * sizeof(uint16_t));
	}

	// lowest level of detail right now.
	m_iNumVertices  = m_pPMesh->GetMinNumVertices();
	m_iNumIndices   = m_pPMesh->GetMinNumIndices();

	// So far, we're at the last collapse
	m_pCollapseUpTo = m_pPMesh->m_pCollapses;

	return true;
}

bool CN3PMeshInstance::Create(const std::string& szFN)
{
	if (m_pPMesh != nullptr && m_pPMesh->FileName() == szFN)
		return true; // 파일 이름이 같으면 새로 만들지 않고 리턴하자

	CN3PMeshInstance::Release();

	CN3PMesh* pN3PMesh = s_MngPMesh.Get(szFN);
	return Create(pN3PMesh);
}

void CN3PMeshInstance::SetLODByNumVertices(int iNumVertices)
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

void CN3PMeshInstance::SetLOD(float value)
{
#define _USE_LODCONTROL_VALUE
#ifdef _USE_LODCONTROL_VALUE
	// value는 distance * FOV이다.
	if (m_pPMesh == nullptr)
		return;

	if (m_pPMesh->m_iLODCtrlValueCount == 0)
	{ // LODCtrlValue가 없으면 모두 그린다.
		SetLODByNumVertices(0x7fffffff);
		return;
	}

	__ASSERT(m_pPMesh->m_pLODCtrlValues, "LOD control value is NULL!");

	CN3PMesh::__LODCtrlValue* pTmpLODCV = m_pPMesh->m_pLODCtrlValues
										  + m_pPMesh->m_iLODCtrlValueCount - 1;

	if (value < m_pPMesh->m_pLODCtrlValues[0].fDist)
	{ // 최소 기준치보다 가까우므로 가장 많은 면으로 그린다.
		SetLODByNumVertices(m_pPMesh->m_pLODCtrlValues[0].iNumVertices);
	}
	else if (pTmpLODCV->fDist < value)
	{ // 최대 기준치보다 멀리 있으므로 가장 적은 면으로 그린다.
		SetLODByNumVertices(pTmpLODCV->iNumVertices);
	}
	else
	{ // 중간 값에 맞게 조정된 면 수로 그린다.
		for (int i = 1; i < m_pPMesh->m_iLODCtrlValueCount; ++i)
		{
			if (value < m_pPMesh->m_pLODCtrlValues[i].fDist)
			{
				CN3PMesh::__LODCtrlValue* pHiValue  = m_pPMesh->m_pLODCtrlValues + i;
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
	if (m_pCollapseUpTo == nullptr || m_pPMesh == nullptr)
		return;

	const int iLODCtrlValueCount = 5;
	__PMLODCtrlValue LODCtrlValues[iLODCtrlValueCount];
	__PMLODCtrlValue* pTmpLODCV = &(LODCtrlValues[iLODCtrlValueCount - 1]);

	int iMaxNumVertices         = m_pPMesh->GetMaxNumVertices();
	int iMinNumVertices         = m_pPMesh->GetMinNumVertices();
	int iDiff                   = iMaxNumVertices - iMinNumVertices;

	float fVolume               = m_pPMesh->GetVolume();
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

bool CN3PMeshInstance::CollapseOne()
{
	if (m_pCollapseUpTo <= m_pPMesh->m_pCollapses)
		return false;

	m_pCollapseUpTo--;

	m_iNumIndices -= m_pCollapseUpTo->NumIndicesToLose;

	for (int* i = m_pPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges;
		i < m_pPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges
				+ m_pCollapseUpTo->NumIndicesToChange;
		i++)
	{
		m_pIndices[*i] = m_pCollapseUpTo->CollapseTo;
	}

	m_iNumVertices -= m_pCollapseUpTo->NumVerticesToLose;

	return true;
}

bool CN3PMeshInstance::SplitOne()
{
	if (m_pCollapseUpTo >= m_pPMesh->m_pCollapses + m_pPMesh->m_iNumCollapses)
		return false; // 이렇게 하면 포인터 하나가 삐져 나오게 된다..
	// 하지만 이렇게 다시 하는 이유는 아래 코드로 하면 마지막 폴리곤이 절대 그려지지 않는다.
	// 이렇게 해도 괜찮을 수 있도록 방어코드를 넣었다. m_pPMesh->m_pCollapses 를 할당할때 1개 더 할당하고 마지막 데이터를 초기값으로 넣었다.
	//	if (m_pCollapseUpTo >= m_pPMesh->m_pCollapses + m_pPMesh->m_iNumCollapses - 1) return false; // 이게 정상이다..

	m_iNumIndices  += m_pCollapseUpTo->NumIndicesToLose;
	m_iNumVertices += m_pCollapseUpTo->NumVerticesToLose;

	if (m_pPMesh->m_pAllIndexChanges)
	{
		for (int* i = m_pPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges;
			i < m_pPMesh->m_pAllIndexChanges + m_pCollapseUpTo->iIndexChanges
					+ m_pCollapseUpTo->NumIndicesToChange;
			i++)
		{
			m_pIndices[*i] = m_iNumVertices - 1;
		}
	}

	m_pCollapseUpTo++;
	return true;
}

void CN3PMeshInstance::Render()
{
	if (m_pPMesh == nullptr)
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
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pPMesh->m_pVertices,
				sizeof(__VertexT1));
		}

		int iRPC = iPC % iPCToRender;
		if (iRPC > 0)
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iRPC,
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pPMesh->m_pVertices,
				sizeof(__VertexT1));
	}
}

void CN3PMeshInstance::RenderTwoUV()
{
	if (nullptr == m_pPMesh)
		return;
	if (nullptr == m_pPMesh->GetVertices2())
	{
		m_pPMesh->GenerateSecondUV(); // 두번째 UV 가 없음 새로 만든다..
	}
	if (nullptr == m_pPMesh->GetVertices2())
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
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pPMesh->m_pVertices2,
				sizeof(__VertexT2));
		}

		int iRPC = iPC % iPCToRender;
		if (iRPC > 0)
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iRPC,
				m_pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pPMesh->m_pVertices2,
				sizeof(__VertexT2));
	}
}

__VertexT1* CN3PMeshInstance::GetVertices() const
{
	if (m_pPMesh == nullptr)
		return nullptr;
	return m_pPMesh->m_pVertices;
}

//	By : Ecli666 ( On 2002-08-06 오후 4:33:04 )
//
void CN3PMeshInstance::PartialRender(int iCount, uint16_t* pIndices)
{
	if (m_pPMesh == nullptr)
		return;
	s_lpD3DDev->SetFVF(FVF_VNT1);
	const int iPCToRender = 1000; // primitive count to render

								  /*	if(m_iNumIndices > 3)
	{
		int iPC = m_iNumIndices / 3;

		int iLC = iPC / iPCToRender;
		int i;
		for (i=0; i<iLC; ++i)
		{
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iPCToRender, m_pIndices + i*iPCToRender*3, D3DFMT_INDEX16, m_pPMesh->m_pVertices, sizeof(__VertexT1));
		}

		int iRPC = iPC%iPCToRender;
		if(iRPC > 0) s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iRPC, m_pIndices + i*iPCToRender*3, D3DFMT_INDEX16, m_pPMesh->m_pVertices, sizeof(__VertexT1));
	}
*/

	if (iCount > 3)
	{
		int iPC = iCount / 3;
		int iLC = iPC / iPCToRender;
		int i   = 0;
		for (; i < iLC; ++i)
		{
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iPCToRender,
				pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pPMesh->m_pVertices,
				sizeof(__VertexT1));
		}

		int iRPC = iPC % iPCToRender;
		if (iRPC > 0)
			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_iNumVertices, iRPC,
				pIndices + i * iPCToRender * 3, D3DFMT_INDEX16, m_pPMesh->m_pVertices,
				sizeof(__VertexT1));
	}
}

int CN3PMeshInstance::GetIndexByiOrder(int iOrder)
{
	if (iOrder >= GetNumIndices())
		return 0;

	return m_pIndices[iOrder];
}

__Vector3 CN3PMeshInstance::GetVertexByIndex(int iIndex)
{
	if (iIndex < 0 || iIndex >= GetNumVertices())
		return __Vector3 { 0, 0, 0 };

	CN3PMesh* mesh = GetMesh();
	if (mesh == nullptr)
		return __Vector3 { 0, 0, 0 };

	__VertexT1& vtx = mesh->m_pVertices[iIndex];
	return __Vector3 { vtx.x, vtx.y, vtx.z };
}
