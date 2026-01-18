// N3FXShape.cpp: implementation of the CN3FXShape class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXShape.h"

CN3FXSPart::CN3FXSPart()
{
	m_vPivot.Set(0, 0, 0);
	m_WorldMtx.Identity();

	m_bOutOfCameraRange = TRUE;

	m_fTexFPS           = 10.0f;
	m_fTexIndex         = 0;
	m_TexRefs.clear();

	m_bTexLoop = true;

	m_Mtl.Init();

	//	m_pPM = nullptr;

	m_pRefShape = nullptr;
}

CN3FXSPart::~CN3FXSPart()
{
	for (size_t i = 0; i < m_TexRefs.size(); i++)
		s_MngTex.Delete(&m_TexRefs[i]);

	//	if(m_pPM) { m_pPM->Release(); delete m_pPM; m_pPM = nullptr; }
}

void CN3FXSPart::Release()
{
	m_vPivot.Set(0, 0, 0);       // Local 축
	m_WorldMtx.Identity();       // World Matrix.. Shape Loading 때 미리 계산해야 좋다..
	m_bOutOfCameraRange = TRUE;  // Camera 범위 바깥에 있음...

	m_fTexFPS           = 10.0f; // Texture Animation Interval;
	m_fTexIndex         = 0;     // Current Texture Index.. Animation 시킬때 필요한 인덱스이다..

	for (size_t i = 0; i < m_TexRefs.size(); i++)
		s_MngTex.Delete(&m_TexRefs[i]);
	m_TexRefs.clear();

	//	if(m_pPM) { m_pPM->Release(); delete m_pPM; m_pPM = nullptr; }
	m_FXPMInst.Release();
}

////////////////////////////////// tex ///////////////////////////////////////////

void CN3FXSPart::TexAlloc(int nCount)
{
	if (nCount <= 0)
		return;

	for (size_t i = 0; i < m_TexRefs.size(); i++)
		s_MngTex.Delete(&m_TexRefs[i]);
	m_TexRefs.clear();

	m_TexRefs.assign(nCount, nullptr);
}

CN3Texture* CN3FXSPart::Tex(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_TexRefs.size()))
		return nullptr;

	return m_TexRefs[iIndex];
}

CN3Texture* CN3FXSPart::TexSet(int iIndex, const std::string& szFN)
{
	if (iIndex >= static_cast<int>(m_TexRefs.size()))
		return nullptr;

	s_MngTex.Delete(&m_TexRefs[iIndex]);
	m_TexRefs[iIndex] = s_MngTex.Get(szFN);
	return m_TexRefs[iIndex];
}

// timeGetTime 으로 얻은 값을 넣으면 Texture Animation 을 컨트롤 한다..
void CN3FXSPart::Tick(const __Matrix44& mtxParent)
{
	CN3FXPMesh* pFXPMesh = m_FXPMInst.GetMesh();
	if (nullptr == pFXPMesh)
		return;

	m_bOutOfCameraRange = FALSE;

	m_WorldMtx.Identity();
	m_WorldMtx.PosSet(m_vPivot);
	m_WorldMtx    *= mtxParent;

	////////////////////////////////////////////////////////////////////////////
	// 카메라와 멀리 떨어지면 지나간다..
	float fDist    = (m_WorldMtx.Pos() - s_CameraData.vEye).Magnitude();
	float fRadius  = Radius();
	if (s_CameraData.IsOutOfFrustum(
			this->m_WorldMtx.Pos(), fRadius * 3.0f)) // 카메라 사면체 바깥이면 지나간다..
	{
		m_bOutOfCameraRange = TRUE;
		return;
	}
	//
	//////////////////////////////////////////////////////////////////////////////////

	// 카메라 거리에 따라 LOD 수준을 조절한다.
	float fLOD = fDist * s_CameraData.fFOV;
	m_FXPMInst.SetLOD(fLOD);

	if (m_TexRefs.size() > 1)
		m_fTexIndex += CN3Base::s_fSecPerFrm * m_fTexFPS;
}

void CN3FXSPart::Render()
{
	if (m_bOutOfCameraRange)
		return;
	if (m_bOutOfCameraRange || m_FXPMInst.GetNumVertices() <= 0)
		return;

#ifdef _DEBUG
	CN3Base::s_RenderInfo.nShape_Part++; // Rendering Information Update...
#endif

	LPDIRECT3DTEXTURE9 lpTex = nullptr;
	int iTC                  = static_cast<int>(m_TexRefs.size());
	if (iTC > 0)
	{
		int iTexIndex = (int) m_fTexIndex;
		if (m_bTexLoop)
			iTexIndex = (int) m_fTexIndex % iTC;

		if (iTexIndex >= 0 && iTexIndex < iTC && m_TexRefs[iTexIndex] != nullptr)
			lpTex = m_TexRefs[iTexIndex]->Get();
		else
			return;
	}

	if (m_Mtl.nRenderFlags & RF_ALPHABLENDING) // Alpha 사용
	{
		__AlphaPrimitive* pAP = s_AlphaMgr.Add();
		if (pAP)
		{
			pAP->bUseVB          = FALSE;
			pAP->dwBlendDest     = m_Mtl.dwDestBlend;
			pAP->dwBlendSrc      = m_Mtl.dwSrcBlend;
			pAP->dwFVF           = FVF_XYZCOLORT1;
			pAP->dwPrimitiveSize = sizeof(__VertexXyzColorT1);
			pAP->fCameraDistance = (s_CameraData.vEye
									- (this->Min() + (this->Max() - this->Min()) * 0.5f))
									   .Magnitude();
			pAP->lpTex           = lpTex;
			pAP->ePrimitiveType  = D3DPT_TRIANGLELIST;
			pAP->nPrimitiveCount = m_FXPMInst.GetNumIndices() / 3;
			//pAP->nRenderFlags		= RF_ALPHABLENDING | RF_NOTUSEFOG | RF_DIFFUSEALPHA | RF_NOTUSELIGHT | RF_DOUBLESIDED;
			pAP->nRenderFlags    = m_Mtl.nRenderFlags;
			pAP->nVertexCount    = m_FXPMInst.GetNumVertices();
			pAP->MtxWorld        = m_WorldMtx;
			pAP->pVertices       = m_FXPMInst.GetVertices();
			pAP->pwIndices       = m_FXPMInst.GetIndices();
		}
		return;                      // 렌더링 안하지롱.
	}

	s_lpD3DDev->SetMaterial(&m_Mtl); // 재질 설정..
	s_lpD3DDev->SetTexture(0, lpTex);
	if (nullptr != lpTex)
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	}
	else
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	}

	__Matrix44 mtx;
	s_lpD3DDev->GetTransform(D3DTS_WORLD, mtx.toD3D());
	s_lpD3DDev->SetTransform(D3DTS_WORLD, m_WorldMtx.toD3D());

	DWORD dwCullMode = 0, dwZWriteEnable = 0, dwZBufferEnable = 0, dwLight = 0;
	s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWriteEnable);
	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZBufferEnable);
	s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCullMode);
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);

	if (m_pRefShape->m_dwZEnable != dwZBufferEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, m_pRefShape->m_dwZEnable);
	if (m_pRefShape->m_dwZWrite != dwZWriteEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, m_pRefShape->m_dwZWrite);
	if (m_pRefShape->m_dwLight != dwLight)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, m_pRefShape->m_dwLight);
	if (m_pRefShape->m_dwDoubleSide != dwCullMode)
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, m_pRefShape->m_dwDoubleSide);

	m_FXPMInst.Render();

	if (m_pRefShape->m_dwZEnable != dwZBufferEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZBufferEnable);
	if (m_pRefShape->m_dwZWrite != dwZWriteEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable);
	if (m_pRefShape->m_dwLight != dwLight)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
	if (m_pRefShape->m_dwDoubleSide != dwCullMode)
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCullMode);

	s_lpD3DDev->SetTransform(D3DTS_WORLD, mtx.toD3D());
}

bool CN3FXSPart::Load(File& file)
{
	int nL = 0;
	char szFN[256];

	file.Read(&m_vPivot, sizeof(__Vector3));

	file.Read(&nL, 4); // Mesh FileName
	file.Read(szFN, nL);
	szFN[nL] = '\0';   // 메시 파일 이름..

	//m_pRefShape의 경로와 읽어들인 파일명을 합쳐라...
	char szPath[_MAX_PATH];
	char szFName[_MAX_FNAME], szExt[_MAX_EXT];
	char szDir[_MAX_DIR];
	_splitpath(m_pRefShape->FileName().c_str(), nullptr, szDir, nullptr, nullptr);
	_splitpath(szFN, nullptr, nullptr, szFName, szExt);
	_makepath(szPath, nullptr, szDir, szFName, szExt);

	if (!this->MeshSet(szPath))
		return false;

	file.Read(&m_Mtl, sizeof(__Material)); // 재질

	int iTC = 0;
	file.Read(&iTC, 4);
	file.Read(&m_fTexFPS, 4);
	m_TexRefs.clear();
	this->TexAlloc(iTC);          // Texture Pointer Pointer 할당..
	for (int j = 0; j < iTC; j++) // Texture Count 만큼 파일 이름 읽어서 텍스처 부르기..
	{
		file.Read(&nL, 4);
		if (nL > 0)
		{
			file.Read(szFN, nL);
			szFN[nL] = '\0'; // 텍스처 파일 이름..

			_splitpath(szFN, nullptr, nullptr, szFName, szExt);
			_makepath(szPath, nullptr, szDir, szFName, szExt);
			m_TexRefs[j] = s_MngTex.Get(szPath);
		}
	}

	return true;
}

void CN3FXSPart::Duplicate(CN3FXSPart* pSrc)
{
	m_vPivot = pSrc->m_vPivot;
	if (pSrc->Mesh())
		MeshSet(pSrc->Mesh()->FileName());

	m_Mtl     = pSrc->m_Mtl;

	int iTC   = 0;
	iTC       = pSrc->TexCount();
	m_fTexFPS = pSrc->m_fTexFPS;

	m_TexRefs.clear();

	TexAlloc(iTC);
	for (int j = 0; j < iTC; j++)
	{
		CN3Texture* pTex = pSrc->Tex(j);
		if (pTex != nullptr)
			m_TexRefs[j] = s_MngTex.Get(pTex->FileName());
	}
	return;
}

bool CN3FXSPart::Save(File& /*file*/)
{
	return true;
}

bool CN3FXSPart::MeshSet(const std::string& szFN)
{
	m_FXPMInst.Create(szFN);
	return true;
}

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

///////////////////////////////////
// CN3Shape
CN3FXShape::CN3FXShape()
{
	m_dwType |= OBJ_SHAPE;
	m_mtxParent.Identity();
	m_mtxFinalTransform.Identity();

	m_dwSrcBlend   = D3DBLEND_ONE;
	m_dwDestBlend  = D3DBLEND_ONE;
	m_bAlpha       = TRUE;

	m_dwZEnable    = D3DZB_TRUE;
	m_dwZWrite     = TRUE;
	m_dwLight      = FALSE;
	m_dwDoubleSide = D3DCULL_NONE;
}

CN3FXShape::~CN3FXShape()
{
	for (CN3FXSPart* pPart : m_Parts)
	{
		pPart->Release();
		delete pPart;
	}
	m_Parts.clear();
}

void CN3FXShape::Release()
{
	for (CN3FXSPart* pPart : m_Parts)
	{
		pPart->Release();
		delete pPart;
	}
	m_Parts.clear();

	CN3TransformCollision::Release();
}

void CN3FXShape::Tick(float fFrm)
{
	CN3TransformCollision::Tick(fFrm);
	m_mtxFinalTransform = m_Matrix * m_mtxParent;

	for (CN3FXSPart* pPart : m_Parts)
		pPart->Tick(m_mtxFinalTransform);
}

// 카메라 위치, 카메라 평면(관찰 절두체 평면) -> 12개의 벡터 배열로 되어 있다.
// [0][1]:카메라 위치와 벡터, [2][3]:카메라 범위 위치와 방향 벡터, [4][5] ~ [10][11]:상하좌우평면벡터
void CN3FXShape::Render()
{
	for (CN3FXSPart* pPart : m_Parts)
		pPart->Render();
}

bool CN3FXShape::Load(File& file)
{
	CN3TransformCollision::Load(file); // 기본정보 읽기...

	for (CN3FXSPart* pPart : m_Parts)
		delete pPart;
	m_Parts.clear();

	int iPC = 0;
	file.Read(&iPC, 4); // Part Count
	if (iPC > 0)
	{
		m_Parts.assign(iPC, nullptr);
		for (int i = 0; i < iPC; i++)
		{
			m_Parts[i]              = new CN3FXSPart();
			m_Parts[i]->m_pRefShape = this;
			if (!m_Parts[i]->Load(file))
				return false;

			//m_Parts[i]->ReCalcMatrix(m_Matrix); // Part Matrix 계산
		}
	}

	uint32_t dwTmp = 0;
	file.Read(&dwTmp, 4); // 소속
	file.Read(&dwTmp, 4); // 속성 0
	file.Read(&dwTmp, 4); // 속성 1
	file.Read(&dwTmp, 4); // 속성 2
	file.Read(&dwTmp, 4); // 속성 3

	FindMinMax();

	return true;
}

bool CN3FXShape::Save(File& /*file*/)
{
	/*
	CN3TransformCollision::Save(file); // 기본정보 읽기...
	
	int nL = 0;
	
	CN3SPart* pPD = nullptr;
	int iPC = m_Parts.size();
	file.Write(&iPC, 4); // Mesh FileName
	for(int i = 0; i < iPC; i++)
	{
		m_Parts[i]->Save(file);
	}

	file.Write(&m_iBelong, 4); // 소속
	file.Write(&m_iAttr0, 4); // 속성 0
	file.Write(&m_iAttr1, 4); // 속성 1
	file.Write(&m_iAttr2, 4); // 속성 2
	file.Write(&m_iAttr3, 4); // 속성 3
	*/
	return true;
}

void CN3FXShape::PartDelete(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_Parts.size()))
		return;

	auto it = m_Parts.begin();
	std::advance(it, iIndex);

	delete *it;
	m_Parts.erase(it);
}

__Vector3 CN3FXShape::CenterPos()
{
	FindMinMax();
	return (m_vMin + (m_vMax - m_vMin) * 0.5f);
}

void CN3FXShape::FindMinMax()
{
	__Vector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
	__Vector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	__Vector3 vMinTmp(0, 0, 0);
	__Vector3 vMaxTmp(0, 0, 0);

	// 가장 큰 지점찾기..
	__Matrix44 mtxWI = m_mtxFinalTransform.Inverse(); // World Matrix Inverse
	for (CN3FXSPart* pPart : m_Parts)
	{
		//pPart->ReCalcMatrix(m_mtxFinalTransform);
		vMinTmp = pPart->Min() * mtxWI; // 월드 상의 최소값을 로컬 좌표로 바꾸어준다..
		vMaxTmp = pPart->Max() * mtxWI; // 월드 상의 최대값을 로컬 좌표로 바꾸어준다..

		if (vMinTmp.x < vMin.x)
			vMin.x = vMinTmp.x;
		if (vMinTmp.y < vMin.y)
			vMin.y = vMinTmp.y;
		if (vMinTmp.z < vMin.z)
			vMin.z = vMinTmp.z;
		if (vMaxTmp.x > vMax.x)
			vMax.x = vMaxTmp.x;
		if (vMaxTmp.y > vMax.y)
			vMax.y = vMaxTmp.y;
		if (vMaxTmp.z > vMax.z)
			vMax.z = vMaxTmp.z;
	}

	// 최대 최소값을 저장
	m_vMin    = vMin * m_mtxFinalTransform;
	m_vMax    = vMax * m_mtxFinalTransform;

	// 최대 최소값을 갖고 반지름 계산한다..
	m_fRadius = (m_vMax - m_vMin).Magnitude() * 0.5f;
}

void CN3FXShape::Duplicate(CN3FXShape* pSrc)
{
	if (!pSrc)
		return;

	m_dwSrcBlend   = pSrc->m_dwSrcBlend;
	m_dwDestBlend  = pSrc->m_dwDestBlend;
	m_bAlpha       = pSrc->m_bAlpha;

	m_dwZEnable    = pSrc->m_dwZEnable;
	m_dwZWrite     = pSrc->m_dwZWrite;
	m_dwLight      = pSrc->m_dwLight;
	m_dwDoubleSide = pSrc->m_dwDoubleSide;

	//CN3TransformCollision::Load(file); // 기본정보 읽기...
	//transform collision...
	SetRadius(pSrc->Radius());
	SetMin(pSrc->Min());
	SetMax(pSrc->Max());

	if (pSrc->CollisionMesh())
		SetMeshCollision(pSrc->CollisionMesh()->FileName());
	if (pSrc->ClimbMesh())
		SetMeshClimb(pSrc->ClimbMesh()->FileName());

	//transform....
	ScaleSet(pSrc->Scale());
	PosSet(pSrc->m_vPos);
	RotSet(pSrc->Rot());

	//basefileaccess
	FileNameSet(pSrc->FileName());

	m_Matrix = pSrc->m_Matrix;
	//
	//
	m_KeyPos.Duplicate(&(pSrc->m_KeyPos));
	m_KeyRot.Duplicate(&(pSrc->m_KeyRot));
	m_KeyScale.Duplicate(&(pSrc->m_KeyScale));

	m_fFrmWhole = pSrc->m_fFrmWhole;
	m_fFrmCur   = 0.0f;

	for (CN3FXSPart* pPart : m_Parts)
		delete pPart;
	m_Parts.clear();

	size_t partCount = pSrc->m_Parts.size();
	if (partCount > 0)
	{
		m_Parts.assign(partCount, nullptr);
		for (size_t i = 0; i < partCount; i++)
		{
			m_Parts[i]              = new CN3FXSPart();
			m_Parts[i]->m_pRefShape = this;
			m_Parts[i]->Duplicate(pSrc->m_Parts[i]);
			//m_Parts[i]->ReCalcMatrix(m_Matrix); // Part Matrix 계산
		}
	}

	return;
}

void CN3FXShape::SetCurrFrm(float fFrm)
{
	if (FRAME_SELFPLAY == fFrm)
	{
		m_fFrmCur += s_fSecPerFrm;
		if (m_fFrmCur < 0)
			m_fFrmCur = 0.0f;
		if (m_fFrmCur >= m_fFrmWhole)
			m_fFrmCur = 0.0f;
	}
	else
		m_fFrmCur = fFrm;
}

float CN3FXShape::GetCurrFrm()
{
	return m_fFrmCur;
}

void CN3FXShape::SetPartsMtl(BOOL bAlpha, uint32_t dwSrcBlend, uint32_t dwDestBlend,
	uint32_t dwZEnable, uint32_t dwZWrite, uint32_t dwLight, uint32_t dwDoubleSide)
{
	m_dwSrcBlend          = dwSrcBlend;
	m_dwDestBlend         = dwDestBlend;
	m_bAlpha              = bAlpha;

	m_dwZEnable           = dwZEnable;
	m_dwZWrite            = dwZWrite;
	m_dwLight             = dwLight;
	m_dwDoubleSide        = dwDoubleSide;

	uint32_t dwRenderFlag = RF_ALPHABLENDING | RF_NOTUSEFOG | RF_DIFFUSEALPHA | RF_NOTUSELIGHT
							| RF_DOUBLESIDED | RF_NOTZWRITE | RF_NOTZBUFFER;

	if (static_cast<D3DZBUFFERTYPE>(m_dwZEnable) == D3DZB_TRUE)
		dwRenderFlag ^= RF_NOTZBUFFER;

	if (m_dwZWrite == TRUE)
		dwRenderFlag ^= RF_NOTZWRITE;

	if (m_dwDoubleSide != D3DCULL_NONE)
		dwRenderFlag ^= RF_DOUBLESIDED;

	if (m_dwLight == TRUE)
		dwRenderFlag ^= RF_NOTUSELIGHT;

	if (m_bAlpha != TRUE)
		dwRenderFlag ^= RF_ALPHABLENDING;

	for (CN3FXSPart* pPart : m_Parts)
		pPart->m_Mtl.nRenderFlags = dwRenderFlag;
}

void CN3FXShape::SetMaxLOD()
{
	for (CN3FXSPart* pPart : m_Parts)
	{
		pPart->m_bOutOfCameraRange = FALSE;
		pPart->m_FXPMInst.SetLOD(0);
	}
}
