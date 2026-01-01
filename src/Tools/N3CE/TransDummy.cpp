// TransDummy.cpp: implementation of the CTransDummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "n3me.h"
#include "TransDummy.h"

#include <N3Base/Pick.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTransDummy::CTransDummy()
{
	ZeroMemory(m_pSortedCubes, sizeof(m_pSortedCubes));

	const float fCubeOffset = 10.0f;
	InitDummyCube(DUMMY_CENTER, &(m_DummyCubes[DUMMY_CENTER]), __Vector3(0,0,0), D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0x00));
	InitDummyCube(DUMMY_X, &(m_DummyCubes[DUMMY_X]), __Vector3(fCubeOffset,0,0), D3DCOLOR_ARGB(0xff, 0xff, 0x00, 0x00));
	InitDummyCube(DUMMY_Y, &(m_DummyCubes[DUMMY_Y]), __Vector3(0,fCubeOffset,0), D3DCOLOR_ARGB(0xff, 0x00, 0xff, 0x00));
	InitDummyCube(DUMMY_Z, &(m_DummyCubes[DUMMY_Z]), __Vector3(0,0,fCubeOffset), D3DCOLOR_ARGB(0xff, 0x00, 0x00, 0xff));
	D3DCOLOR LineColor = D3DCOLOR_ARGB(0xff,0xaa,0xaa,0xaa);
	m_LineVertices[0].Set(0,0,0, LineColor);	m_LineVertices[1].Set(fCubeOffset,0,0, LineColor);
	m_LineVertices[2].Set(0,0,0, LineColor);	m_LineVertices[3].Set(0,fCubeOffset,0, LineColor);
	m_LineVertices[4].Set(0,0,0, LineColor);	m_LineVertices[5].Set(0,0,fCubeOffset, LineColor);
	m_vPrevScaleArray = nullptr;

	Release();
}

CTransDummy::~CTransDummy()
{
	Release();
}

void CTransDummy::Release()
{
	m_SelObjArray.RemoveAll();
	m_pSelectedCube = nullptr;
	m_vPrevPos.Set(0,0,0);
	m_qPrevRot.x = m_qPrevRot.y = m_qPrevRot.z = m_qPrevRot.w = 0;

	if (m_vPrevScaleArray) {delete [] m_vPrevScaleArray; m_vPrevScaleArray = nullptr;}
}

void CTransDummy::InitDummyCube(int iType, __DUMMYCUBE* pDummyCube, const __Vector3& vOffset, D3DCOLOR color)
{
	ASSERT(pDummyCube);
	const float fCubeSize = 1.0f;
	__Vector3	vCubeV[8], vCubeN[6];
	vCubeV[0].Set( fCubeSize, fCubeSize, fCubeSize);	vCubeV[1].Set( fCubeSize, fCubeSize,-fCubeSize);	vCubeV[2].Set(-fCubeSize, fCubeSize,-fCubeSize);
	vCubeV[3].Set(-fCubeSize, fCubeSize, fCubeSize);	vCubeV[4].Set( fCubeSize,-fCubeSize, fCubeSize);	vCubeV[5].Set( fCubeSize,-fCubeSize,-fCubeSize);
	vCubeV[6].Set(-fCubeSize,-fCubeSize,-fCubeSize);	vCubeV[7].Set(-fCubeSize,-fCubeSize, fCubeSize);
	vCubeN[0].Set( 1, 0, 0);	vCubeN[1].Set( 0, 1, 0);	vCubeN[2].Set( 0, 0, 1);
	vCubeN[3].Set(-1, 0, 0);	vCubeN[4].Set( 0,-1, 0);	vCubeN[5].Set( 0, 0,-1);

	pDummyCube->Vertices[0].Set(vCubeV[0], vCubeN[0], color);	pDummyCube->Vertices[1].Set(vCubeV[4], vCubeN[0], color);	pDummyCube->Vertices[2].Set(vCubeV[5], vCubeN[0], color);
	pDummyCube->Vertices[3].Set(vCubeV[1], vCubeN[0], color);	pDummyCube->Vertices[4].Set(vCubeV[0], vCubeN[0], color);	pDummyCube->Vertices[5].Set(vCubeV[5], vCubeN[0], color);
	pDummyCube->Vertices[6].Set(vCubeV[0], vCubeN[1], color);	pDummyCube->Vertices[7].Set(vCubeV[1], vCubeN[1], color);	pDummyCube->Vertices[8].Set(vCubeV[2], vCubeN[1], color);
	pDummyCube->Vertices[9].Set(vCubeV[0], vCubeN[1], color);	pDummyCube->Vertices[10].Set(vCubeV[2], vCubeN[1], color);	pDummyCube->Vertices[11].Set(vCubeV[3], vCubeN[1], color);
	pDummyCube->Vertices[12].Set(vCubeV[0], vCubeN[2], color);	pDummyCube->Vertices[13].Set(vCubeV[7], vCubeN[2], color);	pDummyCube->Vertices[14].Set(vCubeV[4], vCubeN[2], color);
	pDummyCube->Vertices[15].Set(vCubeV[0], vCubeN[2], color);	pDummyCube->Vertices[16].Set(vCubeV[3], vCubeN[2], color);	pDummyCube->Vertices[17].Set(vCubeV[7], vCubeN[2], color);
	pDummyCube->Vertices[18].Set(vCubeV[6], vCubeN[3], color);	pDummyCube->Vertices[19].Set(vCubeV[3], vCubeN[3], color);	pDummyCube->Vertices[20].Set(vCubeV[2], vCubeN[3], color);
	pDummyCube->Vertices[21].Set(vCubeV[6], vCubeN[3], color);	pDummyCube->Vertices[22].Set(vCubeV[7], vCubeN[3], color);	pDummyCube->Vertices[23].Set(vCubeV[3], vCubeN[3], color);
	pDummyCube->Vertices[24].Set(vCubeV[6], vCubeN[4], color);	pDummyCube->Vertices[25].Set(vCubeV[5], vCubeN[4], color);	pDummyCube->Vertices[26].Set(vCubeV[4], vCubeN[4], color);
	pDummyCube->Vertices[27].Set(vCubeV[6], vCubeN[4], color);	pDummyCube->Vertices[28].Set(vCubeV[4], vCubeN[4], color);	pDummyCube->Vertices[29].Set(vCubeV[7], vCubeN[4], color);
	pDummyCube->Vertices[30].Set(vCubeV[6], vCubeN[5], color);	pDummyCube->Vertices[31].Set(vCubeV[2], vCubeN[5], color);	pDummyCube->Vertices[32].Set(vCubeV[1], vCubeN[5], color);
	pDummyCube->Vertices[33].Set(vCubeV[6], vCubeN[5], color);	pDummyCube->Vertices[34].Set(vCubeV[1], vCubeN[5], color);	pDummyCube->Vertices[35].Set(vCubeV[5], vCubeN[5], color);

	int i;
	for (i=0; i<NUM_CUBEVERTEX; ++i)
	{
		pDummyCube->Vertices[i].x += vOffset.x;
		pDummyCube->Vertices[i].y += vOffset.y;
		pDummyCube->Vertices[i].z += vOffset.z;
	}
	pDummyCube->iType = iType;
	pDummyCube->vCenterPos = vOffset;
}

void CTransDummy::Tick()
{
	if (m_SelObjArray.GetSize()==0) return;
	// Scale 조정
	__Vector3 vL = s_CameraData.vEye - m_vPos;
	float fL = vL.Magnitude()*0.01f;
	m_vScale.Set(fL, fL, fL);

	CN3Transform::Tick(-1000.0f);
	ReCalcMatrix();

	// 거리에 따라 정렬
	int i;
	for (i=0; i<NUM_DUMMY; ++i)
	{
		__Vector3 vPos = m_DummyCubes[i].vCenterPos*m_Matrix;
		m_DummyCubes[i].fDistance = (vPos - s_CameraData.vEye).Magnitude();
	}
	for (i=0; i<NUM_DUMMY; ++i) m_pSortedCubes[i] = &(m_DummyCubes[i]);
	qsort(m_pSortedCubes, NUM_DUMMY, sizeof(__DUMMYCUBE*), SortCube);
}

int CTransDummy::SortCube(const void* pArg1, const void* pArg2)
{
	const __DUMMYCUBE* pObj1 = *(const __DUMMYCUBE**)pArg1;
	const __DUMMYCUBE* pObj2 = *(const __DUMMYCUBE**)pArg2;

	if (pObj1->fDistance == pObj2->fDistance) return 0;
	else if (pObj1->fDistance > pObj2->fDistance) return -1;
	else return 1;
}

void CTransDummy::Render()
{
	if (m_SelObjArray.GetSize()==0) return;

	HRESULT hr;

	// set transform
	hr = s_lpD3DDev->SetTransform(D3DTS_WORLD, m_Matrix.toD3D()); // 월드 행렬 적용..

	// set texture
	hr = s_lpD3DDev->SetTexture(0, nullptr);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	// backup state
	DWORD dwZEnable, dwLighting;
	hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	// set state
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 이어지 선 그리기
	hr = s_lpD3DDev->SetFVF(FVF_XYZCOLOR);
	hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 3, m_LineVertices, sizeof(__VertexXyzColor));

	// Cube 그리기
	hr = s_lpD3DDev->SetFVF(FVF_XYZNORMALCOLOR);
	int i;
	for (i=0; i<NUM_DUMMY; ++i)
	{
		ASSERT(m_pSortedCubes[i]);
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_pSortedCubes[i]->Vertices, sizeof(__VertexXyzNormalColor));
	}

	// restore
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
}

void CTransDummy::SetSelObj(CN3Transform* pObj)
{
	m_SelObjArray.RemoveAll();
	if (pObj)
	{
		m_SelObjArray.Add(pObj);
		m_vPos = pObj->Pos();
		m_qRot = pObj->Rot();
	}
}

void CTransDummy::AddSelObj(CN3Transform* pObj)
{
	_ASSERT(pObj);
	m_SelObjArray.Add(pObj);
}

__DUMMYCUBE* CTransDummy::Pick(int x, int y)
{
	CPick pick;
	pick.SetPickXY(x, y);

	int i, j;
	for (i=NUM_DUMMY-1; i>=0; --i)
	{
		for (j=0; j<12; ++j)
		{
			__Vector3 v1, v2, v3;
			__VertexXyzNormalColor* pVertex;

			ASSERT(m_pSortedCubes[i]);
			pVertex = m_pSortedCubes[i]->Vertices+j*3+0;
			v1.Set(pVertex->x, pVertex->y, pVertex->z);
			pVertex = m_pSortedCubes[i]->Vertices+j*3+1;
			v2.Set(pVertex->x, pVertex->y, pVertex->z);
			pVertex = m_pSortedCubes[i]->Vertices+j*3+2;
			v3.Set(pVertex->x, pVertex->y, pVertex->z);

			v1 *= m_Matrix;	v2 *= m_Matrix;	v3 *= m_Matrix;

			__Vector3 vPos; float t, u, v;
			if (pick.IntersectTriangle(v1, v2, v3, t, u, v, &vPos)) return m_pSortedCubes[i];
		}
	}

	return nullptr;
}

BOOL CTransDummy::MouseMsgFilter(LPMSG pMsg)
{
	int iSize = static_cast<int>(m_SelObjArray.GetSize());
	if (iSize == 0)
		return FALSE;

	switch(pMsg->message)
	{
	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			m_pSelectedCube = Pick(point.x, point.y);
			if (m_pSelectedCube)
			{
				CN3Transform* pSelObj0 = m_SelObjArray.GetAt(0);
				_ASSERT(pSelObj0);
				m_vPrevPos = pSelObj0->Pos();
				m_qPrevRot = pSelObj0->Rot();
				if (m_vPrevScaleArray) {delete [] m_vPrevScaleArray; m_vPrevScaleArray = nullptr;}
				m_vPrevScaleArray = new __Vector3[iSize];
				for (int i=0; i<iSize; ++i)	// 모든 선택된 객체의 스케일 저장
				{
					CN3Transform* pSelObj = m_SelObjArray.GetAt(i);
					_ASSERT(pSelObj);
					m_vPrevScaleArray[i] = pSelObj->Scale();
				}

				SetCapture(pMsg->hwnd);
				return TRUE;
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			if (m_pSelectedCube)
			{
				ReleaseCapture();
				m_pSelectedCube = nullptr;
				return TRUE;
			}
		}
		break;
	case WM_RBUTTONUP:	// 큐브 선택 취소 및 이번 드래그로 움직인것 되돌려 놓기
		{
			if (m_pSelectedCube)
			{
				__Vector3 vDiffPos = m_vPrevPos - m_vPos;
//				__Quaternion qDiffRot = m_qPrevRot - m_qRot;
				__Vector3 vDiffScale; vDiffScale.Set(1.0f, 1.0f, 1.0f);
//				TransDiff(&vDiffPos, &vDiffRot, &vDiffScale);
				TransDiff(&vDiffPos, nullptr, &vDiffScale);

				m_vPos = m_vPrevPos;
				m_qRot = m_qPrevRot;
				_ASSERT(m_vPrevScaleArray);
				m_vScale = m_vPrevScaleArray[0];


				ReleaseCapture();
				m_pSelectedCube = nullptr;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

void CTransDummy::GetPickRay(POINT point, __Vector3& vDir, __Vector3& vOrig)
{
	LPDIRECT3DDEVICE9 lpD3DDev = s_lpD3DDev;

    // Get the pick ray from the mouse position
    __Matrix44 matProj;
    lpD3DDev->GetTransform(D3DTS_PROJECTION, matProj.toD3D());

    // Compute the vector of the pick ray in screen space
    __Vector3 v;
    v.x =  ( ( ( 2.0f * point.x ) / (s_CameraData.vp.Width) ) - 1 ) / matProj.m[0][0];
    v.y = -( ( ( 2.0f * point.y ) / (s_CameraData.vp.Height) ) - 1 ) / matProj.m[1][1];
    v.z =  1.0f;

    // Get the inverse view matrix
    __Matrix44 matView, m;
    lpD3DDev->GetTransform(D3DTS_VIEW, matView.toD3D());
    m = matView.Inverse();

    // Transform the screen space pick ray into 3D space
    vDir.x  = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0];
    vDir.y  = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1];
    vDir.z  = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2];
    vOrig.x = m.m[3][0];
    vOrig.y = m.m[3][1];
    vOrig.z = m.m[3][2];
}

void CTransDummy::TransDiff(__Vector3* pvDiffPos, __Quaternion* pqDiffRot, __Vector3* pvDiffScale)
{
	int iSize = static_cast<int>(m_SelObjArray.GetSize());
	if (iSize <= 0)
		return;

	if (pvDiffPos != nullptr)
	{
		for (int i = 0; i < iSize; i++)
		{
			CN3Transform* pSelObj = m_SelObjArray.GetAt(i);
			_ASSERT(pSelObj);
			pSelObj->PosSet(pSelObj->Pos() + (*pvDiffPos));
		}
	}

	if (pqDiffRot != nullptr)
	{
		for (int i = 0; i < iSize; i++)
		{
			CN3Transform* pSelObj = m_SelObjArray.GetAt(i);
			_ASSERT(pSelObj);
			__Quaternion qtRot = pSelObj->Rot();
			qtRot *= (*pqDiffRot);
			pSelObj->RotSet(qtRot);
		}
	}

	if (pvDiffScale != nullptr)
	{
		for (int i = 0; i < iSize; i++)
		{
			CN3Transform* pSelObj = m_SelObjArray.GetAt(i);
			_ASSERT(pSelObj && m_vPrevScaleArray);
			__Vector3 vScale;
			vScale.x = m_vPrevScaleArray[i].x * pvDiffScale->x;
			vScale.y = m_vPrevScaleArray[i].y * pvDiffScale->y;
			vScale.z = m_vPrevScaleArray[i].z * pvDiffScale->z;
			pSelObj->ScaleSet(vScale);
		}
	}
}
