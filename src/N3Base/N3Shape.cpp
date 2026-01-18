// N3Shape.cpp: implementation of the C3DObject class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Shape.h"

/////////////////////////////
// CN3Shape Part ....
CN3SPart::CN3SPart()
{
	m_dwType |= OBJ_SHAPE_PART;

	m_vPivot.Set(0, 0, 0);        // Local 축
	m_Matrix.Identity();          // World Matrix.. Shape Loading 때 미리 계산해야 좋다..
	m_bOutOfCameraRange = TRUE;   // Camera 범위 바깥에 있음...
	m_Mtl.Init();                 // Material
	m_fTexFPS            = 10.0f; // Texture Animation Interval;
	m_fTexIndex          = 0;     // Current Texture Index.. Animation 시킬때 필요한 인덱스이다..

	//	m_vWindFactorCur.Zero();		// 현재 바람 부는 값.. 이값으로 회전을 시킨다..
	//	m_vWindFactorToReach.Zero();	// 바람 부는 값..
	m_fTimeToSetWind     = 0; // 바람 부는 값을 바꾸기 위한 시간..
	m_fWindFactorCur     = 0; // 현재 바람 부는 값.. 이값으로 회전을 시킨다..
	m_fWindFactorToReach = 0; // 바람 부는 값..
}

CN3SPart::~CN3SPart()
{
	for (size_t i = 0; i < m_TexRefs.size(); i++)
		s_MngTex.Delete(&m_TexRefs[i]);
}

void CN3SPart::Release()
{
	m_vPivot.Set(0, 0, 0);        // Local 축
	m_Matrix.Identity();          // World Matrix.. Shape Loading 때 미리 계산해야 좋다..
	m_bOutOfCameraRange = TRUE;   // Camera 범위 바깥에 있음...
	m_Mtl.Init();                 // Material
	m_fTexFPS            = 10.0f; // Texture Animation Interval;
	m_fTexIndex          = 0;     // Current Texture Index.. Animation 시킬때 필요한 인덱스이다..

	//	m_vWindFactorCur.Zero();		// 현재 바람 부는 값.. 이값으로 회전을 시킨다..
	//	m_vWindFactorToReach.Zero();	// 바람 부는 값..
	m_fTimeToSetWind     = 0; // 바람 부는 값을 바꾸기 위한 시간..
	m_fWindFactorCur     = 0; // 현재 바람 부는 값.. 이값으로 회전을 시킨다..
	m_fWindFactorToReach = 0; // 바람 부는 값..

	for (size_t i = 0; i < m_TexRefs.size(); i++)
		s_MngTex.Delete(&m_TexRefs[i]);

	m_TexRefs.clear();
	m_PMInst.Release();
}

void CN3SPart::MeshSet(const std::string& szFN)
{
	m_PMInst.Create(szFN);
}

void CN3SPart::TexAlloc(int nCount)
{
	if (nCount <= 0)
		return;

	for (size_t i = 0; i < m_TexRefs.size(); i++)
		s_MngTex.Delete(&m_TexRefs[i]);
	m_TexRefs.clear();

	m_TexRefs.assign(nCount, nullptr);
}

void CN3SPart::Tick(const __Matrix44& mtxParent, const __Quaternion& qRot,
	float fScale) // timeGetTime 으로 얻은 값을 넣으면 Texture Animation 을 컨트롤 한다..
{
	CN3PMesh* pPMesh = m_PMInst.GetMesh();
	if (nullptr == pPMesh)
		return;

	m_bOutOfCameraRange = FALSE;

	// 카메라와 멀리 떨어지면 지나간다..
	__Vector3 vCenter   = (this->Min() + this->Max()) * 0.5f;
	if (s_CameraData.IsOutOfFrustum(
			vCenter, this->Radius() * fScale)) // 카메라 사면체 바깥이면 지나간다..
	{
		m_bOutOfCameraRange = TRUE;
		return;
	}

	float fDist = (vCenter - s_CameraData.vEye).Magnitude();
	float fLOD  = fDist * s_CameraData.fFOV / fScale;
	//	float fLOD = fDist + fDist * (s_CameraData.fFOV - 1.0f) / 3.0f;
	//	float fLOD = fDist * s_CameraData.fFOV * (512.0f / s_CameraData.fFP);

	// 카메라 거리에 따라 LOD 수준을 조절한다.
	//	fLOD *= 256.0f / s_CameraData.fFP;
	m_PMInst.SetLOD(fLOD);

	int iTC = static_cast<int>(m_TexRefs.size());
	if (iTC > 1) // 텍스처 에니메이션
	{
		m_fTexIndex += CN3Base::s_fSecPerFrm * m_fTexFPS;
		if (m_fTexIndex >= iTC)
			m_fTexIndex -= (iTC * m_fTexIndex)
						   / iTC;        // 정수로 나누면 소숫점만 남기게 된다??(하여튼 비슷해~)
	}

	if (m_Mtl.nRenderFlags & RF_BOARD_Y) // 카메라를 바라봐야하는 거면..
	{
		__Vector3 vPos = m_vPivot * mtxParent;
		__Vector3 vDir = s_CameraData.vEye - vPos;
		if (vDir.x > 0.0f)
			m_Matrix.RotationY(-atanf(vDir.z / vDir.x) - (__PI * 0.5f));
		else
			m_Matrix.RotationY(-atanf(vDir.z / vDir.x) + (__PI * 0.5f));

		// 부모 회전과 반대로 회전을 시킨다..
		float fAngle = 0.0f;
		qRot.AxisAngle(vDir, fAngle);
		if (fAngle != 0)
		{
			__Quaternion qRotI;
			qRotI.RotationAxis(vDir, -fAngle);
			m_Matrix *= qRotI;
		}

		m_Matrix *= mtxParent;
		m_Matrix.PosSet(vPos);
	}

	if (m_Mtl.nRenderFlags & RF_WINDY) // 바람에 살짝 날려야 하면..
	{
		m_fTimeToSetWind -= CN3Base::s_fSecPerFrm;

		if (m_fTimeToSetWind <= 0)
		{
			//			m_vWindFactorToReach.x = 0.05f - (0.1f * (rand()%100) / 100.0f);
			//			m_vWindFactorToReach.y = 0.05f - (0.1f * (rand()%100) / 100.0f); // 위아래로는 조금만 불게 한다.
			//			m_vWindFactorToReach.z = 0.05f - (0.1f * (rand()%100) / 100.0f);

			m_fWindFactorToReach = (rand() % 100) / 100.0f;
			m_fTimeToSetWind     = 3.0f * ((rand() % 100) / 100.0f); // 바람이 지속될 값..
		}
		else if (m_fWindFactorToReach != m_fWindFactorCur)
		//		else if(m_vWindFactorToReach != m_vWindFactorCur)
		{
			//			float fFactor = s_fSecPerFrm * (m_vWindFactorToReach - m_vWindFactorCur).Magnitude();
			float fFactor = s_fSecPerFrm * std::abs(m_fWindFactorToReach - m_fWindFactorCur);

			//			if(m_vWindFactorCur.x < m_vWindFactorToReach.x) m_vWindFactorCur.x += fFactor;
			//			if(m_vWindFactorCur.x > m_vWindFactorToReach.x) m_vWindFactorCur.x -= fFactor;
			//			if(m_vWindFactorCur.y < m_vWindFactorToReach.y) m_vWindFactorCur.y += fFactor;
			//			if(m_vWindFactorCur.y > m_vWindFactorToReach.y) m_vWindFactorCur.y -= fFactor;
			//			if(m_vWindFactorCur.z < m_vWindFactorToReach.z) m_vWindFactorCur.z += fFactor;
			//			if(m_vWindFactorCur.z > m_vWindFactorToReach.z) m_vWindFactorCur.z -= fFactor;
			if (m_fWindFactorCur < m_fWindFactorToReach)
				m_fWindFactorCur += fFactor;
			if (m_fWindFactorCur > m_fWindFactorToReach)
				m_fWindFactorCur -= fFactor;

			//			if(std::abs(m_vWindFactorToReach.x - m_vWindFactorCur.x) < fFactor) m_vWindFactorCur.x = m_vWindFactorToReach.x;
			//			if(std::abs(m_vWindFactorToReach.y - m_vWindFactorCur.y) < fFactor) m_vWindFactorCur.y = m_vWindFactorToReach.y;
			//			if(std::abs(m_vWindFactorToReach.z - m_vWindFactorCur.z) < fFactor) m_vWindFactorCur.z = m_vWindFactorToReach.z;
			if (std::abs(m_fWindFactorToReach - m_fWindFactorCur) < fFactor)
				m_fWindFactorCur = m_fWindFactorToReach;

			__Vector3 vPos = m_vPivot * mtxParent;
			//			m_Matrix.Rotation(CN3Base::s_vWindFactor * m_fWindFactorCur);
			m_Matrix.Rotation(__Vector3(0.05f, 0.02f, 0.05f) * m_fWindFactorCur);
			m_Matrix *= mtxParent;
			m_Matrix.PosSet(vPos);
		}
	}
}

void CN3SPart::Render()
{
	if (m_bOutOfCameraRange || m_PMInst.GetNumVertices() <= 0)
		return;

#ifdef _DEBUG
	s_RenderInfo.nShape_Part++; // Rendering Information Update...
#endif

	LPDIRECT3DTEXTURE9 lpTex = nullptr;
	int iTC                  = static_cast<int>(m_TexRefs.size());
	if (iTC > 0)
	{
		int iTexIndex = (int) m_fTexIndex;
		if (iTexIndex >= 0 && iTexIndex < iTC && m_TexRefs[iTexIndex])
			lpTex = m_TexRefs[iTexIndex]->Get();
	}

	if (m_Mtl.nRenderFlags & RF_ALPHABLENDING) // Alpha 사용
	{
		__AlphaPrimitive* pAP = s_AlphaMgr.Add();
		if (pAP)
		{
			pAP->bUseVB          = FALSE;
			pAP->dwBlendDest     = m_Mtl.dwDestBlend;
			pAP->dwBlendSrc      = m_Mtl.dwSrcBlend;
			pAP->dwFVF           = FVF_VNT1;
			pAP->dwPrimitiveSize = sizeof(__VertexT1);
			pAP->fCameraDistance = (s_CameraData.vEye - m_Matrix.Pos()).Magnitude();
			pAP->lpTex           = lpTex;
			pAP->ePrimitiveType  = D3DPT_TRIANGLELIST;
			pAP->nPrimitiveCount = m_PMInst.GetNumIndices() / 3;
			pAP->nRenderFlags    = m_Mtl.nRenderFlags;
			pAP->nVertexCount    = m_PMInst.GetNumVertices();
			pAP->MtxWorld        = m_Matrix;
			pAP->pVertices       = m_PMInst.GetVertices();
			pAP->pwIndices       = m_PMInst.GetIndices();
		}

		return; // 렌더링 안하지롱.
	}

	//	static uint32_t dwAlpha, dwFog, dwCull;
	DWORD dwFog = 0, dwCull = 0;
	//	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	//	if(TRUE != dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND,   m_Mtl.dwSrcBlend);
	//	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND,  m_Mtl.dwDestBlend);

#ifdef _DEBUG
	// Rendering Information Update...
	s_RenderInfo.nShape_Polygon += m_PMInst.GetNumIndices() / 3;
#endif

	if (m_Mtl.nRenderFlags & RF_NOTUSEFOG) // Fog 무시..
	{
		s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
		if (TRUE == dwFog)
			s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	}

	if (m_Mtl.nRenderFlags & RF_DOUBLESIDED) // Render Flags -
	{
		s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCull);
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}

	s_lpD3DDev->SetMaterial(&m_Mtl); // 재질 설정..
	s_lpD3DDev->SetTexture(0, lpTex);
	if (lpTex != nullptr)
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, m_Mtl.dwColorOp);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, m_Mtl.dwColorArg1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, m_Mtl.dwColorArg2);
	}
	else
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	}

	// 로딩할때 미리 계산해 놓은 월드 행렬 적용..
	s_lpD3DDev->SetTransform(D3DTS_WORLD, m_Matrix.toD3D());

	m_PMInst.Render();

	//	if((m_Mtl.nRenderFlags & RF_ALPHABLENDING) && FALSE == dwAlpha)	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if ((m_Mtl.nRenderFlags & RF_NOTUSEFOG) && TRUE == dwFog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, TRUE); // 안개 사용하지 않는다..

	if ((m_Mtl.nRenderFlags & RF_DOUBLESIDED) && D3DCULL_NONE != dwCull)
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCull);
}

#ifdef _N3TOOL
void CN3SPart::RenderSelected(bool bWireFrame)
{
	if (m_bOutOfCameraRange || m_PMInst.GetNumVertices() <= 0)
		return;

#ifdef _DEBUG
	CN3Base::s_RenderInfo.nShape_Part++; // Rendering Information Update...
	CN3Base::s_RenderInfo.nShape_Polygon += m_PMInst.GetNumIndices() / 3;
#endif _DEBUG

	LPDIRECT3DTEXTURE9 lpTex = nullptr;
	int iTC                  = static_cast<int>(m_TexRefs.size());
	if (iTC > 0)
	{
		int iTexIndex = (int) m_fTexIndex;
		if (iTexIndex >= 0 && iTexIndex < iTC && m_TexRefs[iTexIndex])
			lpTex = m_TexRefs[iTexIndex]->Get();
	}

	s_lpD3DDev->SetTexture(0, lpTex);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	if (lpTex != nullptr)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	else
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	s_lpD3DDev->SetTransform(D3DTS_WORLD, m_Matrix.toD3D());
	if (bWireFrame)
	{
		__Vector3 vLines[4];

		int nFC             = m_PMInst.GetNumIndices() / 3;
		uint16_t* pwIndices = m_PMInst.GetIndices();
		__VertexT1* pVs     = m_PMInst.GetVertices();

		for (int i = 0; i < nFC; i++)
		{
			vLines[0] = pVs[pwIndices[i * 3 + 0]];
			vLines[1] = pVs[pwIndices[i * 3 + 1]];
			vLines[2] = pVs[pwIndices[i * 3 + 2]];
			vLines[3] = vLines[0];
			CN3Base::RenderLines(vLines, 3, 0xff00ff00);
		}
	}
	else
		m_PMInst.Render();
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3SPart::RenderAxis()
{
	// 축그리기..
	float fRadius = 1.0f;
	if (m_PMInst.GetMesh())
		fRadius = m_PMInst.GetMesh()->Radius();
	static CN3Transform TTmp;
	TTmp.m_Matrix = m_Matrix;
	TTmp.Render(&m_Matrix, fRadius * 1.5f);
}
#endif // end of _N3TOOL

bool CN3SPart::Load(File& file)
{
	int nL = 0;
	char szFN[256];

	file.Read(&m_vPivot, sizeof(__Vector3));

	file.Read(&nL, 4);                     // Mesh FileName
	file.Read(szFN, nL);
	szFN[nL] = '\0';                       // 메시 파일 이름..
	this->MeshSet(szFN);

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
			szFN[nL]     = '\0'; // 텍스처 파일 이름..
			m_TexRefs[j] = s_MngTex.Get(szFN, true, s_Options.iTexLOD_Shape);
		}
	}

	return true;
}

#ifdef _N3TOOL
bool CN3SPart::Save(File& file)
{
	file.Write(&m_vPivot, sizeof(__Vector3));

	CN3PMesh* pPMesh = m_PMInst.GetMesh();
	__ASSERT(pPMesh, "Progressive mesh pointer is NULL!");
	int nL = 0;
	if (pPMesh != nullptr)
		nL = static_cast<int>(pPMesh->FileName().size());
	else
		MessageBox(s_hWndBase,
			"Progressive mesh pointer is NULL! : object가 제대로 보이지 않을 수 있습니다.(리소스 "
			"파일이 Load되지 않았을 가능성이 큼)",
			"warning", MB_OK);

	file.Write(&nL, 4); // Mesh FileName
	if (nL > 0)
	{
		//		if(-1 == pPMesh->FileName().find("object\\")) // 임시로 경로를 바꾸려고 넣었다.. 나중에 필요없음 지운다..
		//		{
		//			std::string szFNTmp = fmt::format("Object\\{}.N3PMesh", pPMesh->Name());
		//			pPMesh->FileNameSet(szFNTmp);
		//
		//			SetFilePointer(hFile, -4, 0, FILE_CURRENT);
		//			nL = pPMesh->FileName().size();
		//			file.Write(&nL, 4); // Mesh FileName
		//		}

		file.Write(pPMesh->FileName().c_str(), nL); // 메시 파일 이름..
	}

	file.Write(&m_Mtl, sizeof(__Material));         // 재질

	int iTC = static_cast<int>(m_TexRefs.size());
	file.Write(&iTC, 4);
	file.Write(&m_fTexFPS, 4);
	for (int j = 0; j < iTC; j++) // Texture File 이름 쓰기...
	{
		if (m_TexRefs[j])
			nL = static_cast<int>(m_TexRefs[j]->FileName().size());
		else
			nL = 0;

		file.Write(&nL, 4);
		if (nL > 0)
		{
			//			if(-1 == m_TexRefs[j]->FileName().find("object\\")) // 임시로 경로를 바꾸려고 넣었다.. 나중에 필요없음 지운다..
			//			{
			//				// 폴더 이름을 분리하고..
			//				char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
			//				_splitpath(m_TexRefs[j]->FileName(), szDrive, szDir, szFName, szExt);
			//
			//				std::string szFNTmp = fmt::format("Object\\{}.DXT", szFName);
			//				m_TexRefs[j]->FileNameSet(szFNTmp);
			//
			//				SetFilePointer(hFile, -4, 0, FILE_CURRENT);
			//				nL = lstrlen(m_TexRefs[j]->FileName());
			//				file.Write(&nL, 4); // Mesh FileName
			//			}

			file.Write(m_TexRefs[j]->FileName().c_str(), nL); // 택스처 파일 이름..
		}
	}

	return true;
}
#endif // end of _N3TOOL

void CN3SPart::PartialRender(int iCount, uint16_t* pIndices)
{
	if (m_bOutOfCameraRange || m_PMInst.GetNumVertices() <= 0)
		return;

#ifdef _DEBUG
	CN3Base::s_RenderInfo.nShape_Part++; // Rendering Information Update...
#endif

	LPDIRECT3DTEXTURE9 lpTex = nullptr;
	int iTC                  = static_cast<int>(m_TexRefs.size());
	if (iTC > 0)
	{
		int iTexIndex = (int) m_fTexIndex;
		if (iTexIndex >= 0 && iTexIndex < iTC && m_TexRefs[iTexIndex])
			lpTex = m_TexRefs[iTexIndex]->Get();
	}

	if (m_Mtl.nRenderFlags & RF_ALPHABLENDING) // Alpha 사용
	{
		__AlphaPrimitive* pAP = s_AlphaMgr.Add();
		if (pAP)
		{
			pAP->bUseVB          = FALSE;
			pAP->dwBlendDest     = m_Mtl.dwDestBlend;
			pAP->dwBlendSrc      = m_Mtl.dwSrcBlend;
			pAP->dwFVF           = FVF_VNT1;
			pAP->dwPrimitiveSize = sizeof(__VertexT1);
			pAP->fCameraDistance = (s_CameraData.vEye - m_Matrix.Pos()).Magnitude();
			pAP->lpTex           = lpTex;
			pAP->ePrimitiveType  = D3DPT_TRIANGLELIST;
			pAP->nPrimitiveCount = m_PMInst.GetNumIndices() / 3;
			pAP->nRenderFlags    = m_Mtl.nRenderFlags;
			pAP->nVertexCount    = m_PMInst.GetNumVertices();
			pAP->MtxWorld        = m_Matrix;
			pAP->pVertices       = m_PMInst.GetVertices();
			pAP->pwIndices       = m_PMInst.GetIndices();
		}

		return; // 렌더링 안하지롱.
	}

	DWORD dwFog = 0, dwCull = 0;

#ifdef _DEBUG
	// Rendering Information Update...
	s_RenderInfo.nShape_Polygon += m_PMInst.GetNumIndices() / 3;
#endif

	if (m_Mtl.nRenderFlags & RF_NOTUSEFOG) // Fog 무시..
	{
		s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
		if (TRUE == dwFog)
			s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	}
	if (m_Mtl.nRenderFlags & RF_DOUBLESIDED) // Render Flags -
	{
		s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCull);
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}

	s_lpD3DDev->SetMaterial(&m_Mtl); // 재질 설정..
	s_lpD3DDev->SetTexture(0, lpTex);
	if (nullptr != lpTex)
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, m_Mtl.dwColorOp);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, m_Mtl.dwColorArg1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, m_Mtl.dwColorArg2);
	}
	else
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	}

	// 로딩할때 미리 계산해 놓은 월드 행렬 적용..
	s_lpD3DDev->SetTransform(D3DTS_WORLD, m_Matrix.toD3D());

	m_PMInst.PartialRender(iCount, pIndices);

	if ((m_Mtl.nRenderFlags & RF_NOTUSEFOG) && TRUE == dwFog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, TRUE); // 안개 사용하지 않는다..

	if ((m_Mtl.nRenderFlags & RF_DOUBLESIDED) && D3DCULL_NONE != dwCull)
		s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCull);
}

// CN3Shape Part ....
///////////////////////////////////

///////////////////////////////////
// CN3Shape
CN3Shape::CN3Shape()
{
	m_dwType      |= OBJ_SHAPE;

	m_bDontRender  = false;
	m_bVisible     = true;

	m_iBelong      = 0;
	m_iEventID     = 0; // Event ID
	m_iEventType   = 0; // Event Type
	m_iNPC_ID      = 0; // NPC 로 쓰는 오브젝트일 경우 NPC ID
	m_iNPC_Status  = 0; // NPC 로 쓰는 오브젝트일 경우 NPC Status
}

CN3Shape::~CN3Shape()
{
	for (CN3SPart* pPart : m_Parts)
		delete pPart;
	m_Parts.clear();
}

void CN3Shape::Release()
{
	m_bDontRender = false;
	m_bVisible    = true;

	for (CN3SPart* pPart : m_Parts)
		delete pPart;
	m_Parts.clear();

	m_iBelong     = 0;
	m_iEventID    = 0; // Event ID
	m_iEventType  = 0; // Event Type - 바인드포인트,성문,레버 등등...
	m_iNPC_ID     = 0; // NPC 로 쓰는 오브젝트일 경우 NPC ID
	m_iNPC_Status = 0; // NPC 로 쓰는 오브젝트일 경우 NPC Status

	CN3TransformCollision::Release();
}

void CN3Shape::Tick(float fFrm)
{
	if (false == m_bVisible)
		return;

	// 만약 가까운 거리면.. 좀더 컬링을 느슨하게 한다..
	// 가장 큰 스케일 값을 찾아서..
	float fScale = m_vScale.x;
	if (fScale < m_vScale.y)
		fScale = m_vScale.y;
	if (fScale < m_vScale.z)
		fScale = m_vScale.z;

	// 카메라와 멀리 떨어지면 지나간다..
	float fDist = (m_vPos - s_CameraData.vEye).Magnitude();
	if (fDist > s_CameraData.fFP + m_fRadius * fScale * 2.0f)
	{
		m_bDontRender = true;
		return;
	}

	//	float fDelta = 2.4f;
	//	if(fDist < 64.0f)
	//		fDelta += (64.0f - fDist)/24.0f;
	__Vector3 vCenter = (this->Min() + this->Max()) * 0.5f;
	if (s_CameraData.IsOutOfFrustum(vCenter, this->Radius()))
	{
		m_bDontRender = true;
		return;
	}

	m_bDontRender = false;

	CN3TransformCollision::Tick(fFrm);

	for (CN3SPart* pPart : m_Parts)
	{
		if (pPart != nullptr)
			pPart->Tick(m_Matrix, m_qRot, fScale);
	}
}

// 카메라 위치, 카메라 평면(관찰 절두체 평면) -> 12개의 벡터 배열로 되어 있다.
// [0][1]:카메라 위치와 벡터, [2][3]:카메라 범위 위치와 방향 벡터, [4][5] ~ [10][11]:상하좌우평면벡터
void CN3Shape::Render()
{
	if (!m_bVisible)
		return;

	if (m_bDontRender)
		return;

#ifdef _DEBUG
	CN3Base::s_RenderInfo.nShape++;
#endif

	for (CN3SPart* pPart : m_Parts)
	{
		if (pPart != nullptr)
			pPart->Render();
	}
}

#ifdef _N3TOOL
void CN3Shape::RenderSelected(bool bWireFrame)
{
	// 축그리기..
	CN3Transform::Render(nullptr, m_fRadius * 3.0f);

	for (CN3SPart* pPart : m_Parts)
		pPart->RenderSelected(bWireFrame);
}
#endif // end of _N3TOOL

bool CN3Shape::Load(File& file)
{
	CN3TransformCollision::Load(file); // 기본정보 읽기...

	for (CN3SPart* pPart : m_Parts)
		delete pPart;
	m_Parts.clear();

	int iPC = 0;
	file.Read(&iPC, 4); // Part Count
	if (iPC > 0)
	{
		m_Parts.assign(iPC, nullptr);
		for (int i = 0; i < iPC; i++)
		{
			m_Parts[i] = new CN3SPart();
			m_Parts[i]->Load(file);
			m_Parts[i]->ReCalcMatrix(m_Matrix); // Part Matrix 계산
		}
	}

	file.Read(&m_iBelong, 4);     // 소속
	file.Read(&m_iEventID, 4);    // Event ID
	file.Read(&m_iEventType, 4);  // Event Type - 바인드 포인트, 성문, 레버 등등...
	file.Read(&m_iNPC_ID, 4);     // NPC 로 쓰는 오브젝트일 경우 NPC ID
	file.Read(&m_iNPC_Status, 4); // NPC 로 쓰는 오브젝트일 경우 NPC Status

	this->FindMinMax();

	return true;
}

bool CN3Shape::LoadTransformOnly(File& file)
{
	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	return CN3Transform::Load(file); // 기본정보 읽기...
}

#ifdef _N3TOOL
bool CN3Shape::Save(File& file)
{
	CN3TransformCollision::Save(file); // 기본정보 읽기...

	int nL  = 0;

	int iPC = static_cast<int>(m_Parts.size());
	file.Write(&iPC, 4); // Mesh FileName
	for (int i = 0; i < iPC; i++)
		m_Parts[i]->Save(file);

	file.Write(&m_iBelong, 4);     // 소속
	file.Write(&m_iEventID, 4);    // Event ID
	file.Write(&m_iEventType, 4);  // Event Type - 바인드 포인트, 성문, 레버 등등...
	file.Write(&m_iNPC_ID, 4);     // NPC 로 쓰는 오브젝트일 경우 NPC ID
	file.Write(&m_iNPC_Status, 4); // NPC 로 쓰는 오브젝트일 경우 NPC Status

	return true;
}
#endif // end of _N3TOOL

void CN3Shape::PartDelete(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_Parts.size()))
		return;

	auto it = m_Parts.begin();
	std::advance(it, iIndex);
	delete *it;
	m_Parts.erase(it);
}

#ifdef _N3TOOL
void CN3Shape::RenderSelected(int iPart, bool bWireFrame)
{
	if (iPart < 0 || iPart >= static_cast<int>(m_Parts.size()))
		return;

	m_Parts[iPart]->RenderSelected(bWireFrame);
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
bool CN3Shape::IsPMeshProcessed()
{
	if (m_Parts.empty())
		return false;

	for (CN3SPart* pPart : m_Parts)
	{
		if (pPart == nullptr || pPart->MeshInstance() == nullptr)
			continue;

		if (!pPart->MeshInstance()->IsLOD())
			return false;
	}

	return true;
}
#endif // end of _N3TOOL

void CN3Shape::ReCalcMatrix()
{
	CN3Transform::ReCalcMatrix(); // Transfomr Matrix 를 계산 해주고..

	// 각 파트의 매트릭스를 다시 계산해 준다..
	ReCalcPartMatrix();
}

void CN3Shape::ReCalcPartMatrix()
{
	// 각 파트의 매트릭스를 다시 계산해 준다..
	for (CN3SPart* pPart : m_Parts)
		pPart->ReCalcMatrix(m_Matrix);
}

void CN3Shape::FindMinMax()
{
	m_vMin.Zero();
	m_vMax.Zero();
	m_fRadius = 0;

	if (m_Parts.empty())
		return;

	m_vMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
	m_vMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	__Vector3 vMinTmp(0, 0, 0);
	__Vector3 vMaxTmp(0, 0, 0);

	// 가장 큰 지점찾기..
	__Matrix44 mtxWI = m_Matrix.Inverse(); // World Matrix Inverse
	for (CN3SPart* pPart : m_Parts)
	{
		vMinTmp = pPart->Min() * mtxWI;    // 월드 상의 최소값을 로컬 좌표로 바꾸어준다..
		vMaxTmp = pPart->Max() * mtxWI;    // 월드 상의 최대값을 로컬 좌표로 바꾸어준다..

		if (vMinTmp.x < m_vMin.x)
			m_vMin.x = vMinTmp.x;
		if (vMinTmp.y < m_vMin.y)
			m_vMin.y = vMinTmp.y;
		if (vMinTmp.z < m_vMin.z)
			m_vMin.z = vMinTmp.z;
		if (vMaxTmp.x > m_vMax.x)
			m_vMax.x = vMaxTmp.x;
		if (vMaxTmp.y > m_vMax.y)
			m_vMax.y = vMaxTmp.y;
		if (vMaxTmp.z > m_vMax.z)
			m_vMax.z = vMaxTmp.z;
	}

	// 최대 최소값을 갖고 반지름 계산한다..
	m_fRadius = (m_vMax - m_vMin).Magnitude() * 0.5f;
}

int CN3Shape::CheckCollisionPrecisely(
	bool bIgnoreBoxCheck, int ixScreen, int iyScreen, __Vector3* pVCol, __Vector3* pVNormal)
{
	// Compute the vector of the pick ray in screen space
	__Vector3 vTmp;

	vTmp.x = (((2.0f * ixScreen) / (CN3Base::s_CameraData.vp.Width)) - 1)
			 / CN3Base::s_CameraData.mtxProjection.m[0][0];
	vTmp.y = -(((2.0f * iyScreen) / (CN3Base::s_CameraData.vp.Height)) - 1)
			 / CN3Base::s_CameraData.mtxProjection.m[1][1];
	vTmp.z             = 1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44* pMtxVI = &CN3Base::s_CameraData.mtxViewInverse;
	__Vector3 vPos, vDir;
	vDir.x = vTmp.x * pMtxVI->m[0][0] + vTmp.y * pMtxVI->m[1][0] + vTmp.z * pMtxVI->m[2][0];
	vDir.y = vTmp.x * pMtxVI->m[0][1] + vTmp.y * pMtxVI->m[1][1] + vTmp.z * pMtxVI->m[2][1];
	vDir.z = vTmp.x * pMtxVI->m[0][2] + vTmp.y * pMtxVI->m[1][2] + vTmp.z * pMtxVI->m[2][2];
	vPos   = pMtxVI->Pos();

	return this->CheckCollisionPrecisely(bIgnoreBoxCheck, vPos, vDir, pVCol, pVNormal);
}

int CN3Shape::CheckCollisionPrecisely(bool bIgnoreBoxCheck, const __Vector3& vPos,
	const __Vector3& vDir, __Vector3* pVCol, __Vector3* pVNormal)
{
	// 박스 체크 먼저한다..
	if (!bIgnoreBoxCheck && !_CheckCollisionByBox(vPos, vDir, m_vMin * m_Matrix, m_vMax * m_Matrix))
		return -1;

	__Vector3 vPos2 = vPos, vDir2 = vDir;
	int partCount = static_cast<int>(m_Parts.size());
	for (int i = 0; i < partCount; i++)
	{
		CN3SPart* pPart        = m_Parts[i];

		CN3PMeshInstance* pPMI = pPart->MeshInstance();
		if (pPMI == nullptr)
			continue;

		__VertexT1* pVs = pPMI->GetVertices();
		uint16_t* pwIs  = pPMI->GetIndices();
		int nIndexCount = pPMI->GetNumIndices();

		int nFC         = nIndexCount / 3; // Face Count

		// Face 수가 24 개보다 많은 경우 일단 박스체크를 한다..
		if (nFC > 64 && !::_CheckCollisionByBox(vPos, vDir, pPart->Min(), pPart->Max()))
			continue;

		__Matrix44 mtxWI = pPart->m_Matrix.Inverse(); // World Matrix Inverse

		vPos2            = vPos * mtxWI;
		mtxWI.PosSet(0, 0, 0);
		vDir2    = vDir * mtxWI;      // 역행렬로 회전..

		int nCI0 = 0, nCI1 = 0, nCI2 = 0;
		for (int j = 0; j < nFC; j++) // 각각의 Face 마다 충돌체크..
		{
			nCI0 = pwIs[j * 3 + 0];
			nCI1 = pwIs[j * 3 + 1];
			nCI2 = pwIs[j * 3 + 2];

			if (!_IntersectTriangle(vPos2, vDir2, pVs[nCI0], pVs[nCI1], pVs[nCI2]))
				continue;

			float fT = 0.0f, fU = 0.0f, fV = 0.0f;
			::_IntersectTriangle(vPos2, vDir2, pVs[nCI0], pVs[nCI1], pVs[nCI2], fT, fU, fV, pVCol);
			if (pVCol != nullptr)
				(*pVCol) *= pPart->m_Matrix;

			if (pVNormal != nullptr)
			{
				pVNormal->Cross(pVs[nCI1] - pVs[nCI0], pVs[nCI2] - pVs[nCI1]);
				pVNormal->Normalize();

				mtxWI = pPart->m_Matrix.Inverse(); // World Matrix Inverse
				mtxWI.PosSet(0, 0, 0);             // 역행렬로 회전..
				*pVNormal *= mtxWI;                // 역행렬로 회전..
			}

			return i;
		}
	}

	return -1;
}

bool CN3Shape::MakeCollisionMeshByParts() // 충돌 메시를 박스로 만든다...
{
	int iVC = 0, iIC = 0;
	for (CN3SPart* pPart : m_Parts)
	{
		CN3PMesh* pPMesh = pPart->Mesh();
		if (pPMesh == nullptr)
			continue;

		iVC += 8;
		iIC += 36;
	}

	if (iVC <= 0 || iIC <= 0)
		return false;

	CN3VMesh* pVMesh = new CN3VMesh();
	pVMesh->CreateVertices(iVC);
	pVMesh->CreateIndex(iIC);

	__Vector3* pVDest = pVMesh->Vertices();
	uint16_t* pwIDest = pVMesh->Indices();

	__Matrix44 mtxI   = m_Matrix.Inverse();
	CN3VMesh VMTmp;

	iVC = 0;
	iIC = 0;
	for (CN3SPart* pPart : m_Parts)
	{
		CN3PMesh* pPMesh = pPart->Mesh();
		if (pPMesh == nullptr)
			continue;

		VMTmp.CreateCube(pPMesh->Min(), pPMesh->Max());

		__Vector3* pVSrc = VMTmp.Vertices();
		uint16_t* pwISrc = VMTmp.Indices();

		pPart->Tick(m_Matrix, m_qRot, 1.0f);

		__Matrix44 mtxPart  = pPart->m_Matrix;
		mtxPart            *= mtxI;

		for (int j = 0; j < 8; j++)
			pVDest[iVC + j] = pVSrc[j] * mtxPart;

		for (int j = 0; j < 36; j++)
			pwIDest[iIC + j] = pwISrc[j] + iVC;

		iVC += 8;
		iIC += 36;
	}

	if (iVC <= 0 || iIC <= 0)
	{
		delete pVMesh;
		return false;
	}

	int iCount       = CN3Base::s_MngVMesh.Count();
	std::string buff = fmt::format(
		"{}_collision_{}.n3vmesh", m_szFileName.c_str(), iCount); // 임시로 이름일 짓고..

	pVMesh->FileNameSet(buff);
	CN3Base::s_MngVMesh.Delete(&m_pMeshCollision);                // 전의 거 지우고..
	CN3Base::s_MngVMesh.Add(pVMesh);
	m_pMeshCollision = s_MngVMesh.Get(pVMesh->FileName());

	FindMinMax();

	return true;
}

bool CN3Shape::MakeCollisionMeshByPartsDetail() // 현재 모습 그대로... 충돌 메시를 만든다...
{
	int iMaxNumVertices = 0, iMaxNumIndices = 0;
	int iVC = 0, iIC = 0;
	for (CN3SPart* pPart : m_Parts)
	{
		CN3PMesh* pPMesh       = pPart->Mesh();
		CN3PMeshInstance* pPMI = pPart->MeshInstance();
		if (pPMesh == nullptr || pPMI == nullptr)
			continue;

		iMaxNumVertices  = pPMesh->GetMaxNumVertices();
		iMaxNumIndices   = pPMesh->GetMaxNumIndices();

		iVC             += iMaxNumVertices;
		iIC             += iMaxNumIndices;
	}

	if (iVC <= 0 || iIC <= 0)
		return false;

	CN3VMesh* pVMesh = new CN3VMesh();
	pVMesh->CreateVertices(iVC);
	pVMesh->CreateIndex(iIC);

	__Vector3* pVDest = pVMesh->Vertices();
	uint16_t* pwIDest = pVMesh->Indices();

	__VertexT1* pVSrc = nullptr;
	uint16_t* pwISrc  = nullptr;

	iVC               = 0;
	iIC               = 0;
	__Matrix44 mtxI   = m_Matrix.Inverse();

	for (CN3SPart* pPart : m_Parts)
	{
		CN3PMesh* pPMesh       = pPart->Mesh();
		CN3PMeshInstance* pPMI = pPart->MeshInstance();
		if (pPMesh == nullptr || pPMI == nullptr)
			continue;

		iMaxNumVertices = pPMesh->GetMaxNumVertices();
		iMaxNumIndices  = pPMesh->GetMaxNumIndices();
		pPMI->SetLODByNumVertices(iMaxNumVertices);

		pVSrc  = pPMesh->GetVertices();
		pwISrc = pPMI->GetIndices();
		if (pVSrc == nullptr || pwISrc == nullptr)
			continue;

		pPart->Tick(m_Matrix, m_qRot, 1.0f);

		__Matrix44 mtxPart  = pPart->m_Matrix;
		mtxPart            *= mtxI;

		for (int j = 0; j < iMaxNumVertices; j++)
			pVDest[iVC + j] = pVSrc[j] * mtxPart;

		for (int j = 0; j < iMaxNumIndices; j++)
			pwIDest[iIC + j] = pwISrc[j] + iVC;

		iVC += iMaxNumVertices;
		iIC += iMaxNumIndices;
	}

	if (iVC <= 0 || iIC <= 0)
	{
		delete pVMesh;
		return false;
	}

	int iCount       = CN3Base::s_MngVMesh.Count();
	std::string buff = fmt::format(
		"{}_collision_{}.n3vmesh", m_szFileName.c_str(), iCount); // 임시로 이름일 짓고..

	pVMesh->FileNameSet(buff);
	CN3Base::s_MngVMesh.Delete(&m_pMeshCollision);                // 전의 거 지우고..
	CN3Base::s_MngVMesh.Add(pVMesh);
	m_pMeshCollision = s_MngVMesh.Get(pVMesh->FileName());

	FindMinMax();

	return true;
}

#ifdef _N3TOOL
void CN3Shape::MakeDefaultMaterial()
{
	// 각 파트의 재질을 기본적인 흰색으로 해준다..
	__Material mtlBasic;
	mtlBasic.Init();

	for (CN3SPart* pPart : m_Parts)
		memcpy(&pPart->m_Mtl, &mtlBasic, sizeof(_D3DMATERIAL9));
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3Shape::RemoveRenderFlags(int nFlags)
{
	// 각 파트의 매트릭스를 다시 계산해 준다..
	for (CN3SPart* pPart : m_Parts)
		pPart->m_Mtl.nRenderFlags &= ~nFlags; // 기본 흰색..
}
#endif                                        // end of _N3TOOL

#ifdef _N3TOOL
bool CN3Shape::SaveToSameFolder(const std::string& szFullPath)
{
	if (szFullPath.empty())
		return false;

	std::string szPath = szFullPath;
	for (int i = static_cast<int>(szFullPath.size()) - 1; i >= 0; i--)
	{
		if ('\\' == szPath[i] || '/' == szPath[i])
		{
			szPath = szPath.substr(0, i + 1);
			break;
		}
	}

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	std::string szNameTmp, szOldFN;

	std::vector<std::string> OldPartFNs;
	std::vector<std::string> OldTexFNs;
	for (CN3SPart* pPart : m_Parts)
	{
		szOldFN = pPart->Mesh()->FileName();
		OldPartFNs.push_back(szOldFN); // 파일 이름 보관..

		_splitpath(szOldFN.c_str(), szDrive, szDir, szFName, szExt);
		szNameTmp = szPath + szFName + szExt;
		pPart->Mesh()->SaveToFile(szNameTmp);

		int iTC = pPart->TexCount();
		for (int j = 0; j < iTC; j++)
		{
			CN3Texture* pTex = pPart->Tex(j);

			szOldFN          = pTex->FileName();
			OldTexFNs.push_back(szOldFN); // 파일 이름 보관..

			_splitpath(szOldFN.c_str(), szDrive, szDir, szFName, szExt);
			szNameTmp = szPath + szFName + szExt;
			pTex->SaveToFile(szNameTmp);
		}
	}

	szOldFN = m_szFileName;
	_splitpath(m_szFileName.c_str(), szDrive, szDir, szFName, szExt);
	szNameTmp = szPath + szFName + szExt;
	SaveToFile(szNameTmp);
	m_szFileName = szOldFN;

	// 원래대로 파일 이름 돌려놓기..
	int iSeq     = 0;
	for (size_t i = 0; i < m_Parts.size(); i++)
	{
		CN3SPart* pPart = m_Parts[i];
		pPart->Mesh()->FileNameSet(OldPartFNs[i]);

		int iTC = pPart->TexCount();
		for (int j = 0; j < iTC; j++)
		{
			pPart->Tex(j)->FileNameSet(OldTexFNs[iSeq]);
			iSeq++;
		}
	}

	//	By : Ecli666 ( On 2002-10-16 오전 11:44:19 )
	//
	// TODO(srmeier): m_pvCollisions is undefined, if I need this I'll have to revisit this
	/*
	szOldFN = m_pvCollisions->FileName();
	_splitpath(CollisionMesh()->FileName().c_str(), szDrive, szDir, szFName, szExt);
	szNameTmp = szPath + szFName + szExt;
	CollisionMesh()->SaveToFile(szNameTmp);
	CollisionMesh()->FileNameSet(szOldFN);	
	*/

	//	~(By Ecli666 On 2002-10-16 오전 11:44:19 )

	return true;
}

bool CN3Shape::SaveToSameFolderAndMore(
	const std::string& szFullPath, const std::string& szRelativePath)
{
	if (szFullPath.empty())
		return false;

	std::string szPath = szFullPath;
	for (int i = static_cast<int>(szFullPath.size()) - 1; i >= 0; i--)
	{
		if ('\\' == szPath[i] || '/' == szPath[i])
		{
			szPath = szPath.substr(0, i + 1);
			break;
		}
	}

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	std::string szNameTmp, szOldFN;

	for (CN3SPart* pPart : m_Parts)
	{
		szOldFN = pPart->Mesh()->FileName();

		_splitpath(szOldFN.c_str(), szDrive, szDir, szFName, szExt);
		szNameTmp = szPath + szFName + szExt;
		pPart->Mesh()->SaveToFile(szNameTmp);
		pPart->Mesh()->FileNameSet(szRelativePath + szFName + szExt);

		int iTC = pPart->TexCount();
		for (int j = 0; j < iTC; j++)
		{
			CN3Texture* pTex = pPart->Tex(j);

			szOldFN          = pTex->FileName();

			_splitpath(szOldFN.c_str(), szDrive, szDir, szFName, szExt);
			szNameTmp = szPath + szFName + szExt;
			pTex->SaveToFile(szNameTmp);
			pTex->FileNameSet(szRelativePath + szFName + szExt);
		}
	}

	_splitpath(m_szFileName.c_str(), szDrive, szDir, szFName, szExt);
	szNameTmp = szPath + szFName + szExt;
	SaveToFile(szNameTmp);
	m_szFileName = szRelativePath + szFName + szExt;

	//	By : Ecli666 ( On 2002-10-16 오전 11:44:19 )
	//
	if (CollisionMesh())
	{
		_splitpath(CollisionMesh()->FileName().c_str(), szDrive, szDir, szFName, szExt);
		szNameTmp = szPath + szFName + szExt;
		CollisionMesh()->SaveToFile(szNameTmp);
		szOldFN  = szFName;
		szOldFN += szExt;
		CollisionMesh()->FileNameSet(szRelativePath + szFName + szExt);
	}
	//	~(By Ecli666 On 2002-10-16 오전 11:44:19 )

	return true;
}

#endif // end of _N3TOOL

//	By : Ecli666 ( On 2002-08-06 오후 4:33:32 )
//
void CN3Shape::SetMaxLOD()
{
	m_bDontRender = false;

	for (auto itr = m_Parts.begin(); itr != m_Parts.end(); ++itr)
	{
		auto part                 = *itr;
		part->m_bOutOfCameraRange = FALSE;
		part->m_PMInst.SetLOD(0);
	}
}

__Matrix44 CN3Shape::GetPartMatrix(int iPartIndex) const
{
	return m_Parts[iPartIndex]->m_Matrix;
}

void CN3Shape::PartialRender(int iPartIndex, int iCount, uint16_t* pIndices)
{
	if (iPartIndex < 0 || iPartIndex >= static_cast<int>(m_Parts.size()))
		return;

	m_Parts[iPartIndex]->PartialRender(iCount, pIndices);
}

int CN3Shape::GetIndexbufferCount(int iPartIndex)
{
	if (iPartIndex < 0 || iPartIndex >= static_cast<int>(m_Parts.size()))
		return 0;

	return m_Parts[iPartIndex]->MeshInstance()->GetNumIndices();
}

int CN3Shape::GetIndexByiOrder(int iPartIndex, int iOrder)
{
	return m_Parts[iPartIndex]->MeshInstance()->GetIndexByiOrder(iOrder);
}

__Vector3 CN3Shape::GetVertexByIndex(int iPartIndex, int iIndex)
{
	return m_Parts[iPartIndex]->MeshInstance()->GetVertexByIndex(iIndex);
}

int CN3Shape::GetColIndexbufferCount()
{
	if (CollisionMesh())
		if (CollisionMesh()->IndexCount())
			return CollisionMesh()->IndexCount();
		else
			return CollisionMesh()->VertexCount();
	else
		return -1;
}

int CN3Shape::GetColIndexByiOrder(int iOrder)
{
	return CollisionMesh()->GetColIndexByiOrder(iOrder);
}

__Vector3 CN3Shape::GetColVertexByIndex(int iIndex)
{
	return CollisionMesh()->GetColVertexByIndex(iIndex);
}

void CN3Shape::PartialColRender(int iCount, int* piIndices)
{
	CollisionMesh()->PartialColRender(iCount, piIndices);
}

void CN3Shape::PartialGetCollision(int iIndex, __Vector3& vec)
{
	CollisionMesh()->PartialGetCollision(iIndex, vec);
}

//	~(By Ecli666 On 2002-08-06 오후 4:33:32 )

