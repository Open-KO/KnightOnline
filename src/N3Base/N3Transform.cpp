// N3Obj.cpp: implementation of the CN3 class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Transform.h"

CN3Transform::CN3Transform()
{
	m_dwType |= OBJ_TRANSFORM;

	m_vPos.Set(0, 0, 0); // 위치, 스케일, 회전 벡터.
	m_vScale.Set(1, 1, 1);
	m_qRot.Identity();
	m_Matrix.Identity();

	// 에니메이션 키
	m_fFrmCur   = 0;
	m_fFrmWhole = 0;
}

CN3Transform::~CN3Transform()
{
}

void CN3Transform::Release()
{
	m_vPos.Set(0, 0, 0); // 위치, 스케일, 회전 벡터.
	m_vScale.Set(1, 1, 1);
	m_qRot.Identity();

	m_Matrix.Identity();

	// 에니메이션 키
	m_fFrmCur   = 0;
	m_fFrmWhole = 0;

	m_KeyPos.Release();
	m_KeyRot.Release();
	m_KeyScale.Release();

	CN3BaseFileAccess::Release();
}

bool CN3Transform::Load(File& file)
{
	CN3BaseFileAccess::Load(file);

	file.Read(&m_vPos, sizeof(__Vector3)); // 위치, 스케일, 회전 벡터.
	file.Read(&m_qRot, sizeof(__Quaternion));
	file.Read(&m_vScale, sizeof(__Vector3));

	// 에니메이션 키
	m_KeyPos.Load(file);
	m_KeyRot.Load(file);
	m_KeyScale.Load(file);

	m_fFrmCur       = 0;
	m_fFrmWhole     = 0;

	float fFrmWhole = 0;

	fFrmWhole       = m_KeyPos.Count() * m_KeyPos.SamplingRate() / 30.0f;
	if (fFrmWhole > m_fFrmWhole)
		m_fFrmWhole = fFrmWhole;

	fFrmWhole = m_KeyRot.Count() * m_KeyRot.SamplingRate() / 30.0f;
	if (fFrmWhole > m_fFrmWhole)
		m_fFrmWhole = fFrmWhole;

	fFrmWhole = m_KeyScale.Count() * m_KeyScale.SamplingRate() / 30.0f;
	if (fFrmWhole > m_fFrmWhole)
		m_fFrmWhole = fFrmWhole;

	ReCalcMatrix(); // 변환 행렬 계산..

	return true;
}

#ifdef _N3TOOL
bool CN3Transform::Save(File& file)
{
	CN3BaseFileAccess::Save(file);

	file.Write(&m_vPos, sizeof(__Vector3)); // 위치, 스케일, 회전 벡터.
	file.Write(&m_qRot, sizeof(__Quaternion));
	file.Write(&m_vScale, sizeof(__Vector3));

	// 에니메이션 키
	m_KeyPos.Save(file);
	m_KeyRot.Save(file);
	m_KeyScale.Save(file);

	return true;
}
#endif // end of _N3TOOL

void CN3Transform::Tick(float fFrm)
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
	{
		m_fFrmCur = fFrm;
	}

	bool bNdeedReCalcMatrix = TickAnimationKey(m_fFrmCur);

	if (m_dwType & OBJ_JOINT)
		return; // Joint 일 경우는 행렬을 계산하는 방법이 다르기 땜시 넘어간다..

	if (bNdeedReCalcMatrix)
		ReCalcMatrix();
}

void CN3Transform::ReCalcMatrix()
{
	m_Matrix.Scale(m_vScale);
	if (m_qRot.w != 0)
		m_Matrix *= m_qRot;
	m_Matrix.PosSet(m_vPos);
}

bool CN3Transform::TickAnimationKey(float fFrm)
{
	// 에니메이션 키
	int nKCP = m_KeyPos.Count();
	int nKCR = m_KeyRot.Count();
	int nKCS = m_KeyScale.Count();
	if (nKCP <= 0 && nKCR <= 0 && nKCS <= 0)
		return false;

	bool bNeedReCalcMatrix = false;
	if (m_KeyPos.DataGet(fFrm, m_vPos) == true)
		bNeedReCalcMatrix = true;
	if (m_KeyRot.DataGet(fFrm, m_qRot) == true)
		bNeedReCalcMatrix = true;
	if (m_KeyScale.DataGet(fFrm, m_vScale) == true)
		bNeedReCalcMatrix = true;

	return bNeedReCalcMatrix;
}

#ifdef _N3TOOL
void CN3Transform::Render(const __Matrix44* pMtxParent, float fUnitSize)
{
	// 축 그리기..
	static __Vector3 vAxis[9];
	static bool bAxisCreated = false;
	if (false == bAxisCreated)
	{
		__Vector3 v0(0, 0, 0), v1(1, 0, 0), v2(0.8f, 0.2f, 0);
		__Matrix44 mtxRot;
		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
			{
				mtxRot.Identity();
			} // X 축
			else if (i == 1)
			{
				mtxRot.RotationZ(DegreesToRadians(90.0f));
			} // Y 축
			else if (i == 2)
			{
				mtxRot.RotationY(DegreesToRadians(-90.0f));
			} // Z 축

			vAxis[i * 3 + 0] = v0 * mtxRot;
			vAxis[i * 3 + 1] = v1 * mtxRot;
			vAxis[i * 3 + 2] = v2 * mtxRot;
		}

		bAxisCreated = true;
	}

	__Matrix44 mtxBox;
	mtxBox.Scale(fUnitSize, fUnitSize, fUnitSize); // 관절부 박스에 스케일 적용
	mtxBox *= m_Matrix;

	s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxBox.toD3D());
	CN3Base::RenderLines(&(vAxis[0]), 2, 0xffff0000); // 선그리기..
	CN3Base::RenderLines(&(vAxis[3]), 2, 0xff00ff00); // 선그리기..
	CN3Base::RenderLines(&(vAxis[6]), 2, 0xff0000ff); // 선그리기..
}
#endif                                                // end of _N3TOOL

/*
#if _DEBUG 
// 선택 상자 만들기.
void CN3Transform::GenerateSelectBox(__Vector3 &vMin, __Vector3 &vMax)
{
	// Bounding Box Vertex Buffer 생성
	if(m_lpVBBox != nullptr) { m_lpVBBox->Release(); m_lpVBBox = nullptr; }
	HRESULT rval = m_lpDevRef->CreateVertexBuffer(54 * sizeof(__VertexColor), 0, FVF_CV, D3DPOOL_MANAGED, &m_lpVBBox);
	if(rval != D3D_OK)
	{
		rval = m_lpDevRef->CreateVertexBuffer(54 * sizeof(__VertexColor), 0, FVF_CV, D3DPOOL_MANAGED, &m_lpVBBox);
		if(rval != D3D_OK)
		{
			char szDebug[256]; D3DXGetErrorString(rval, szDebug, 256);
			MessageBox(s_hWndBase, szDebug, "VertexBuffer 생성 실패", MB_OK);
			return;
		}
	}

	float fW = (vMax.x - vMin.x)/5, fH = (vMax.y - vMin.y)/5, fL = (vMax.z - vMin.z)/5; // 길이 높이 너비 (x y z)

	__VertexColor* pVC;
	m_lpVBBox->Lock(0, 0, (uint8_t**)&pVC, nullptr);
	
	pVC[4] =  pVC[2] =  pVC[0] =  __VertexColor(vMin.x, vMax.y, vMin.z, 0xff7f7f7f);  // 정면 - 아래와 같은 선 리스트를 만든다.
	pVC[10] = pVC[8] =  pVC[6] =  __VertexColor(vMax.x, vMax.y, vMin.z, 0xff7f7f7f);  //  /          /
	pVC[16] = pVC[14] = pVC[12] = __VertexColor(vMax.x, vMin.y, vMin.z, 0xff7f7f7f);  // +--      --+
	pVC[22] = pVC[20] = pVC[18] = __VertexColor(vMin.x, vMin.y, vMin.z, 0xff7f7f7f);  // |          |
	pVC[28] = pVC[26] = pVC[24] = __VertexColor(vMax.x, vMax.y, vMax.z, 0xff7f7f7f);  // 
	pVC[34] = pVC[32] = pVC[30] = __VertexColor(vMin.x, vMax.y, vMax.z, 0xff7f7f7f);  // |/         |/
	pVC[40] = pVC[38] = pVC[36] = __VertexColor(vMin.x, vMin.y, vMax.z, 0xff7f7f7f);  // +--      --+
	pVC[46] = pVC[44] = pVC[42] = __VertexColor(vMax.x, vMin.y, vMax.z, 0xff7f7f7f);  // 

	pVC[1] =  pVC[0];  pVC[1].x  += fW;   pVC[3] =  pVC[2];  pVC[3].y  -= fH;   pVC[5] =  pVC[4];  pVC[5].z  += fL;
	pVC[7] =  pVC[6];  pVC[7].x  -= fW;   pVC[9] =  pVC[8];  pVC[9].y  -= fH;   pVC[11] = pVC[10]; pVC[11].z += fL;
	pVC[13] = pVC[12]; pVC[13].x -= fW;   pVC[15] = pVC[14]; pVC[15].y += fH;   pVC[17] = pVC[16]; pVC[17].z += fL;
	pVC[19] = pVC[18]; pVC[19].x += fW;   pVC[21] = pVC[20]; pVC[21].y += fH;   pVC[23] = pVC[22]; pVC[23].z += fL;
	
	pVC[25] = pVC[24]; pVC[25].x -= fW;   pVC[27] = pVC[26]; pVC[27].y -= fH;   pVC[29] = pVC[28]; pVC[29].z -= fL;
	pVC[31] = pVC[30]; pVC[31].x += fW;   pVC[33] = pVC[32]; pVC[33].y -= fH;   pVC[35] = pVC[34]; pVC[35].z -= fL;
	pVC[37] = pVC[36]; pVC[37].x += fW;   pVC[39] = pVC[38]; pVC[39].y += fH;   pVC[41] = pVC[40]; pVC[41].z -= fL;
	pVC[43] = pVC[42]; pVC[43].x -= fW;   pVC[45] = pVC[44]; pVC[45].y += fH;   pVC[47] = pVC[46]; pVC[47].z -= fL;

	__Vector3 vLength = (vMax - vMin);
	float fLength = vLength.Magnitude();
	__Vector3 vCenter = vMin + (vMax - vMin)/2.0f;
	pVC[48].Set(0, 0, 0, 0xffff0000); pVC[48].x -= vLength.x/2 + fLength/5 + 0.5f;// x 축 빨간색 -> 빨간색
	pVC[49].Set(0, 0, 0, 0xffff0000); pVC[49].x += vLength.x/2 + fLength/5 + 0.5f;// x 축 빨간색 -> 빨간색
	pVC[50].Set(0, 0, 0, 0xff00ff00); pVC[50].y -= vLength.y/2 + fLength/5 + 0.5f;// y 축 녹색 -> 녹색
	pVC[51].Set(0, 0, 0, 0xff00ff00); pVC[51].y += vLength.y/2 + fLength/5 + 0.5f;// y 축 녹색 -> 녹색
	pVC[52].Set(0, 0, 0, 0xff0000ff); pVC[52].z -= vLength.z/2 + fLength/5 + 0.5f;// z 축 파란색 -> 파란색
	pVC[53].Set(0, 0, 0, 0xff0000ff); pVC[53].z += vLength.z/2 + fLength/5 + 0.5f;// z 축 파란색 -> 파란색
	m_lpVBBox->Unlock();
}
#endif
*/
