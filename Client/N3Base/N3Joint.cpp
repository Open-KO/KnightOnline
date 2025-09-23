// N3Joint.cpp: implementation of the CN3Joint class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Joint.h"
#include "N3Mesh.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3Joint::CN3Joint()
{
	m_dwType |= OBJ_JOINT;
	
	m_qOrient.Identity();		// Joint Orient Quaternion
	m_pParent = nullptr;
}

CN3Joint::~CN3Joint()
{
	for(it_Joint it = m_Children.begin(), itEnd = m_Children.end(); it != itEnd; it++)
	{
		delete *it;
	}
	m_Children.clear();
}

void CN3Joint::Release()
{
	m_qOrient.Identity();		// Joint Orient Quaternion
	m_KeyOrient.Release();		// Joint Orient 키값... nullptr 이면 없는거다..
	
	for(it_Joint it = m_Children.begin(), itEnd = m_Children.end(); it != itEnd; it++)
	{
		delete *it;
	}
	m_Children.clear();
	m_pParent = nullptr;

	CN3Transform::Release();
}

bool CN3Joint::Load(HANDLE hFile)
{
	CN3Transform::Load(hFile);

	DWORD dwRWC = 0;
	int nL = 0;

	m_KeyOrient.Load(hFile); // Joint Orient...

	int nCC = 0;
	ReadFile(hFile, &nCC, 4, &dwRWC, nullptr);
	for(int i = 0; i < nCC; i++)
	{
		CN3Joint* pChild = new CN3Joint();
		this->ChildAdd(pChild);

		pChild->Load(hFile);
	}

	return true;
}

#ifdef _N3TOOL
bool CN3Joint::Save(HANDLE hFile)
{
	CN3Transform::Save(hFile);

	DWORD dwRWC = 0;
	
	m_KeyOrient.Save(hFile); // 

	int iSize = m_Children.size();
	WriteFile(hFile, &iSize, 4, &dwRWC, nullptr);

	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		pChild->Save(hFile);
	}

	return true;
}
#endif // end of _N3TOOL


#ifdef _N3TOOL
void CN3Joint::Render(const __Matrix44* pMtxParent, float fUnitSize)
{
	DWORD dwAlpha, dwFog, dwLight, dwZ;
	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	
	if(dwZ) s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	if(dwFog) s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if(dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if(dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	static __Matrix44 stm;
	static __Material smtl;
	static bool bInit = false;
	if(false == bInit)
	{
		stm.Identity();
		smtl.Init();
		bInit = true;
	}

	s_lpD3DDev->SetTransform(D3DTS_WORLD, &stm);
	s_lpD3DDev->SetMaterial(&smtl);
	s_lpD3DDev->SetTexture(0, nullptr);

	if(m_pParent) // 부모 관절과 이어주는 선..
	{
		static __Vector3 v[2];
		static __VertexColor vBone[2];
	
		v[0] = m_pParent->m_Matrix.Pos();
		v[1] = m_Matrix.Pos();
		if(pMtxParent)
		{
			D3DXMATRIX mmm;
			v[0] *= *pMtxParent;
			v[1] *= *pMtxParent;
		}

		vBone[0].Set(v[0], 0xff00ff00);
		vBone[1].Set(v[1], 0xff0000ff);
		s_lpD3DDev->SetFVF(FVF_CV);
		s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, vBone, sizeof(__VertexColor)); // 선그리기..
	}

	// 박스 그리기..
	static __VertexColor vBoxes[36];
	static __VertexColor vAxis[6];
	static bool bBoxCreated = false;
	if(false == bBoxCreated)
	{
		CN3Mesh mesh;
		mesh.Create_Cube(__Vector3(-0.5f, -0.5f, -0.5f), __Vector3(0.5f, 0.5f, 0.5f));
		__VertexT1* pVSrc = mesh.Vertices();
		for(int i = 0; i < 36; i++)
		{
			vBoxes[i].Set(pVSrc[i].x, pVSrc[i].y, pVSrc[i].z, 0xff00ff00);
		}

		float fL = 1.0f;
		vAxis[0].Set(0, 0, 0, 0xffff0000);
		vAxis[1].Set(fL, 0, 0, 0xffff0000);
		vAxis[2].Set(0, 0, 0, 0xff00ff00);
		vAxis[3].Set(0, fL, 0, 0xff00ff00);
		vAxis[4].Set(0, 0, 0, 0xff0000ff);
		vAxis[5].Set(0, 0, fL, 0xff0000ff);

		bBoxCreated = true;
	}

	__Matrix44 mtxAxis, mtxBox;
	mtxBox.Scale(fUnitSize, fUnitSize, fUnitSize);
	if(m_Children.size() > 0) mtxAxis.Scale(fUnitSize*2, fUnitSize*2, fUnitSize*2);
	else mtxAxis.Scale(fUnitSize*12, fUnitSize*12, fUnitSize*12);

	mtxBox *= m_Matrix;
	mtxAxis *= m_Matrix;

	s_lpD3DDev->SetFVF(FVF_CV);
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtxBox);
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, vBoxes, sizeof(__VertexColor)); // 박스 그리기..
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtxAxis);
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 3, vAxis, sizeof(__VertexColor)); // 축 그리기..

	if(dwZ) s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	if(dwFog) s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	if(dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	if(dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);

	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		pChild->Render(pMtxParent, fUnitSize);
	}
}
#endif // end of _N3TOOL

void CN3Joint::ChildAdd(CN3Joint *pChild)
{
	__ASSERT(pChild, "Child joint pointer is NULL!");

	CN3Joint* pChild2 = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild2 = *it;
		if(pChild2 == pChild) return;
	}

	m_Children.push_back(pChild);
	pChild->ParentSet(this);
}

void CN3Joint::ChildDelete(CN3Joint *pChild)
{
	if(nullptr == pChild) return;
	it_Joint it = m_Children.begin(), itEnd = m_Children.end();
	int iSize = m_Children.size();
	for(; it != itEnd; )
	{
		if(*it == pChild) 
		{
			it = m_Children.erase(it);
			pChild->ParentSet(nullptr);
			break;
		}
		else
		{
			it++;
		}
	}
}

void CN3Joint::ParentSet(CN3Joint* pParent)
{
	if (pParent == m_pParent)
		return;

	m_pParent = pParent;
	if (pParent != nullptr)
	{
		pParent->ChildAdd(this);
		m_iFileFormatVersion = pParent->m_iFileFormatVersion;
	}
}

void CN3Joint::NodeCount(int &nCount)
{
	nCount++;
	
	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		pChild->NodeCount(nCount);
	}
}

#ifdef _N3TOOL
BOOL CN3Joint::FindPointerByName(const std::string& szName, CN3Joint *&pJoint) // 이름을 넣으면 해당 노드의 포인터를 돌려준다..
{
	if(szName.empty())
	{
		pJoint = nullptr;
		return FALSE;
	}

	if(m_szName == szName) return TRUE;
	pJoint = this;
		
	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		if(TRUE == pChild->FindPointerByName(szName, pJoint)) return TRUE;
	}
	
	return FALSE;
}
#endif // end of _N3TOOL

BOOL CN3Joint::FindPointerByID(int nID, CN3Joint *&pJoint)
{
	static int stnID = 0;
	if(pJoint == nullptr) stnID = 0;

	pJoint = this;
	if(nID == stnID) return TRUE;
	stnID++;

	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		if(TRUE == pChild->FindPointerByID(nID, pJoint)) return TRUE;
	}
	
	return FALSE;
}

#ifdef _N3TOOL
BOOL CN3Joint::FindIndex(const std::string& szName, int &nIndex)
{
	if(szName.empty())
	{
		nIndex = -1;
		return FALSE;
	}

	if(m_szName == szName) return TRUE;
	nIndex++;
		
	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		if(TRUE == pChild->FindIndex(szName, nIndex)) return TRUE;
	}
	
	return FALSE;
}
#endif // end of _N3TOOL

void CN3Joint::Tick(float fFrm)
{
	m_KeyPos.DataGet(fFrm, m_vPos);
	m_KeyRot.DataGet(fFrm, m_qRot);
	m_KeyScale.DataGet(fFrm, m_vScale);
	m_KeyOrient.DataGet(fFrm, m_qOrient);

	CN3Joint::ReCalcMatrix();

	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;

		__ASSERT(pChild, "Child joint pointer is NULL!");
		pChild->Tick(fFrm);
	}
}

bool CN3Joint::TickAnimationKey(float fFrm)
{
	bool bNeedReCalcMatrix = CN3Transform::TickAnimationKey(fFrm);
	if(m_KeyOrient.DataGet(fFrm, m_qOrient)) bNeedReCalcMatrix = true;

	return bNeedReCalcMatrix;
}

void CN3Joint::ReCalcMatrix()
{
	static __Matrix44 mtx;
	static __Quaternion qt;
//	원래 행렬 계산 코드...
//	m_Matrix.Identity();
//	mtx.Rotation(m_vRot.x, m_vRot.y, m_vRot.z); m_Matrix *= mtx; // 부모 축 만큼 회전.
//	mtx.Scale(m_vScale); m_Matrix *= mtx; // 부모 축 만큼 회전.
//	mtx.Rotation(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z); m_Matrix *= mtx; // 부모 로컬 축 만큼 회전.
//	mtx.Identity(); mtx.PosSet(m_vPos); m_Matrix *= mtx; // 이동

//	if(m_RotSeq == ROT_SEQ_XYZ)
//	{
//		m_Matrix.Rotation(m_vRot.x, m_vRot.y, m_vRot.z); // 부모 축 만큼 회전. // XYZ 회전일때..
//	}
//	else if(ROT_SEQ_YXZ)
//	{
//		m_Matrix.Identity();
//		mtx.RotationY(m_vRot.y); m_Matrix *= mtx; // YXZ 회전일때
//		mtx.RotationX(m_vRot.x); m_Matrix *= mtx;
//		mtx.RotationZ(m_vRot.z); m_Matrix *= mtx;
//	}

	if(m_KeyOrient.Count() > 0) // Orient 키값이 있으면..
	{
		qt = m_qRot * m_qOrient;
		::D3DXMatrixRotationQuaternion(&m_Matrix, &qt); // 회전.. 쿼터니언 계산..
	}
	else
	{
		::D3DXMatrixRotationQuaternion(&m_Matrix, &m_qRot); // 회전.. 쿼터니언 계산..
	}

	if(1.0f != m_vScale.x || 1.0f != m_vScale.y || 1.0f != m_vScale.z) // 스케일 값이 있으면..
	{
		mtx.Scale(m_vScale); // 스케일
		m_Matrix *= mtx;
	}
	m_Matrix.PosSet(m_vPos);

	if(m_pParent) m_Matrix *= m_pParent->m_Matrix; // 부모 행렬
}

void CN3Joint::MatricesGet(__Matrix44 *pMtxs, int &nJointIndex)
{
	memcpy(&(pMtxs[nJointIndex]), &m_Matrix, sizeof(__Matrix44));
	nJointIndex++;

	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin();
	int iSize = m_Children.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pChild = *it;
		pChild->MatricesGet(pMtxs, nJointIndex);
	}
}

void CN3Joint::ReCalcMatrixBlended(float fFrm0, float fFrm1, float fWeight0)
{
//	__ASSERT(1.0f == (fWeight0 + fWeight1));

	static __Vector3 v1, v2;
	static __Quaternion qt1, qt2;
	int nKC = 0, nFrm0 = (int)fFrm0, nFrm1 = (int)fFrm1;
	float fWeight1 = 1.0f - fWeight0;

	bool bHaveKey1 = m_KeyPos.DataGet(fFrm0, v1);
	bool bHaveKey2 = m_KeyPos.DataGet(fFrm1, v2);
	if(bHaveKey1 && bHaveKey2) 
		m_vPos = (v1 * fWeight0) + (v2 * fWeight1);

	bHaveKey1 = m_KeyRot.DataGet(fFrm0, qt1);
	bHaveKey2 = m_KeyRot.DataGet(fFrm1, qt2);
	if(bHaveKey1 && bHaveKey2) 
		m_qRot.Slerp(qt1, qt2, fWeight1);

	bHaveKey1 = m_KeyScale.DataGet(fFrm0, v1);
	bHaveKey2 = m_KeyScale.DataGet(fFrm1, v2);
	if(bHaveKey1 && bHaveKey2) 
		m_vScale = (v1 * fWeight0) + (v2 * fWeight1);

	bHaveKey1 = m_KeyOrient.DataGet(fFrm0, qt1);
	bHaveKey2 = m_KeyOrient.DataGet(fFrm1, qt2);
	if(bHaveKey1 && bHaveKey2) 
		m_qOrient.Slerp(qt1, qt2, fWeight1);

	this->ReCalcMatrix(); // Matrix 계산...
}

#ifdef _N3TOOL
void CN3Joint::KeyDelete(CN3Joint *pJoint, int nKS, int nKE)
{
	CN3AnimKey* pAKs[3] = { &(pJoint->m_KeyPos), &(pJoint->m_KeyRot), &(pJoint->m_KeyScale) };

	int i;
	for(i = 0; i < 3; i++)
	{
		int nKC = pAKs[i]->Count();
		if(nKC > 0 && nKS > 0 && nKS < nKC && nKE > nKS && nKE < nKC)
		{
			int nKE2 = nKC - nKE - 1;

			__Vector3* pVs1 = new __Vector3[nKS];
			__Vector3* pVs2 = new __Vector3[nKE2];

			if(pAKs[i]->Type() == KEY_VECTOR3) 
			{
				memcpy(pVs1, &(((__Vector3*)pAKs[i]->m_pDatas)[0]), sizeof(__Vector3) * nKS);
				memcpy(pVs2, &(((__Vector3*)pAKs[i]->m_pDatas)[nKE+1]), sizeof(__Vector3) * nKE2);
			}
			else if(pAKs[i]->Type() == KEY_QUATERNION)
			{
				memcpy(pVs1, &(((__Quaternion*)pAKs[i]->m_pDatas)[0]), sizeof(__Quaternion) * nKS);
				memcpy(pVs2, &(((__Quaternion*)pAKs[i]->m_pDatas)[nKE+1]), sizeof(__Quaternion) * nKE2);
			}

			delete [] pAKs[i]->m_pDatas;
			pAKs[i]->m_nCount = 0;
			pAKs[i]->m_pDatas = nullptr;

			pAKs[i]->Alloc(nKS + nKE2, pAKs[i]->SamplingRate(), pAKs[i]->Type());

			if(pAKs[i]->Type() == KEY_VECTOR3) 
			{
				memcpy(&(((__Vector3*)pAKs[i]->m_pDatas)[0]), pVs1, sizeof(__Vector3) * nKS);
				memcpy(&(((__Vector3*)pAKs[i]->m_pDatas)[nKS]), pVs2, sizeof(__Vector3) * nKE2);
			}
			else if(pAKs[i]->Type() == KEY_QUATERNION)
			{
				memcpy(&(((__Quaternion*)pAKs[i]->m_pDatas)[0]), pVs1, sizeof(__Quaternion) * nKS);
				memcpy(&(((__Quaternion*)pAKs[i]->m_pDatas)[nKS]), pVs2, sizeof(__Quaternion) * nKE2);
			}

			delete [] pVs1;
			delete [] pVs2;
		}
	}

	// Child 를 다시 만들어 준다.
	int nCC = pJoint->ChildCount();
	for(i = 0; i < nCC; i++)
	{
		CN3Joint* pChild = pJoint->Child(i);
		pChild->KeyDelete(pChild, nKS, nKE); // 하위 조인트를 복사..
	}
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3Joint::AddKey(CN3Joint *pJSrc, int nIndexS, int nIndexE)
{
	m_KeyPos.Add(pJSrc->m_KeyPos, nIndexS, nIndexE);
	m_KeyRot.Add(pJSrc->m_KeyRot, nIndexS, nIndexE);
	m_KeyScale.Add(pJSrc->m_KeyScale, nIndexS, nIndexE);

	it_Joint it = pJSrc->m_Children.begin();
	it_Joint it2 = m_Children.begin();
	int iSize = pJSrc->m_Children.size();
	int iSize2 = m_Children.size();
	__ASSERT(iSize == iSize2, "can't copy animation key - because child count is different from each other.");
	for(int i = 0; i < iSize; i++, it++, it2++)
	{
		CN3Joint* pChildSrc = *it;
		CN3Joint* pChildDest = *it2;

		pChildDest->AddKey(pChildSrc, nIndexS, nIndexE); // 재귀호출
	}
}
#endif // end of _N3TOOL

// 회전값등을
#ifdef _N3TOOL
void CN3Joint::CopyExceptAnimationKey(CN3Joint *pJSrc)
{
	this->Release();

	m_szName = pJSrc->m_szName;
	m_szFileName = pJSrc->m_szFileName;

	m_vPos = pJSrc->m_vPos;
	m_qRot = pJSrc->m_qRot;
	m_vScale = pJSrc->m_vScale;

	m_Matrix = pJSrc->m_Matrix;
	m_fFrmCur = pJSrc->m_fFrmCur;
	m_fFrmWhole = pJSrc->m_fFrmWhole;

	if(m_KeyPos.Count() <= 0) m_KeyPos.m_eType = pJSrc->m_KeyPos.m_eType;
	if(m_KeyRot.Count() <= 0) m_KeyRot.m_eType = pJSrc->m_KeyRot.m_eType;
	if(m_KeyScale.Count() <= 0) m_KeyScale.m_eType = pJSrc->m_KeyScale.m_eType;

	CN3Joint* pChild = nullptr;
	it_Joint it = m_Children.begin(), itEnd = m_Children.end();
	for(; it != itEnd; )
	{
		pChild = *it;
		delete pChild;
		it = m_Children.erase(it);
	}
	m_Children.clear();

	it = pJSrc->m_Children.begin();
	itEnd = pJSrc->m_Children.end();
	for(; it != itEnd; it++)
	{
		CN3Joint* pChildDest = new CN3Joint();
		this->ChildAdd(pChildDest); // 자식 추가..

		CN3Joint* pChildSrc = *it;
		pChildDest->CopyExceptAnimationKey(pChildSrc); // 재귀호출
	}
}
#endif // end of _N3TOOL
