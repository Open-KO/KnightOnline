// VtxPosDummy.cpp: implementation of the CVtxPosDummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "VtxPosDummy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVtxPosDummy::CVtxPosDummy()
{

}

CVtxPosDummy::~CVtxPosDummy()
{
}

void CVtxPosDummy::Release()
{
	CTransDummy::Release();
	m_SelVtxArray.RemoveAll();
}	

void CVtxPosDummy::Tick()
{
	if (m_SelVtxArray.GetSize()==0) return;

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
	qsort(m_pSortedCubes, sizeof(__DUMMYCUBE*), NUM_DUMMY, SortCube);
}

void CVtxPosDummy::Render()
{
	if (m_SelVtxArray.GetSize()==0) return;

	HRESULT hr;

	// set transform
	hr = s_lpD3DDev->SetTransform(D3DTS_WORLD, &m_Matrix); // 월드 행렬 적용..

	// set texture
	hr = s_lpD3DDev->SetTexture(0, NULL);
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
		if (m_pSortedCubes[i]->iType == DUMMY_CENTER) continue;	// 가운데 큐브는 그리지 않는다.
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_pSortedCubes[i]->Vertices, sizeof(__VertexXyzNormalColor));
	}

	// restore
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
}

void CVtxPosDummy::SetSelObj(CN3Transform* pObj)
{
	ASSERT(0);
}

void CVtxPosDummy::AddSelObj(CN3Transform* pObj)
{
	ASSERT(0);
}

void CVtxPosDummy::SetSelVtx(__VertexXyzT1* pVtx)			// 선택된 점 바꾸기
{
	m_SelVtxArray.RemoveAll();
	if (pVtx)
	{
		m_SelVtxArray.Add(pVtx);
		m_vPos = *pVtx;
		m_qRot.x = m_qRot.y = m_qRot.z = m_qRot.w = 0;
	}
}

void CVtxPosDummy::AddSelVtx(__VertexXyzT1* pVtx)			// 선택된 점 추가
{
	_ASSERT(pVtx);
	m_SelVtxArray.Add(pVtx);
}

BOOL CVtxPosDummy::MouseMsgFilter(LPMSG pMsg)				// 마우스 메세지 처리
{
	int iSize = m_SelVtxArray.GetSize();
	if (iSize == 0) return FALSE;

	switch(pMsg->message)
	{
	case WM_MOUSEMOVE:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			DWORD nFlags = pMsg->wParam;
			if (m_pSelectedCube && (nFlags & MK_LBUTTON))
			{
				__Vector3 vRayDir, vRayOrig;	// 화면 중앙(시점)과 마우스 포인터를 이은 직선의 방향과 원점
				__Vector3 vPN, vPV;	// 평면의 법선과 포함된 점
				__Vector3 vPos;	// 위의 평면과 직선의 만나는 점(구할 점)
				__Vector3 vCameraDir = s_CameraData.vAt - s_CameraData.vEye;	vCameraDir.Normalize();
				GetPickRay(point, vRayDir, vRayOrig);
				vPV = m_vPrevPos;
				__Matrix44 mat = m_Matrix;	mat.PosSet(0,0,0);

				switch(m_pSelectedCube->iType)
				{
				case DUMMY_CENTER:
					{
						// XZ평면 위로 움직이게..
						vPN.Set(0,1,0);

						__Vector3 vTmp = vPV - vRayOrig;
						float fT = D3DXVec3Dot(&vPN, &vTmp) / D3DXVec3Dot(&vPN, &vRayDir);
						vPos = vRayOrig + vRayDir*fT;

						__Vector3 vDiffPos = vPos - m_vPos;
						TransDiff(&vDiffPos, NULL, NULL);
						m_vPos = vPos;
					}
					break;
				case DUMMY_X:
					{
						vPN.Set(0, vCameraDir.y, vCameraDir.z);					
						vPN.Normalize();

						__Vector3 vTmp = vPV - vRayOrig;
						float fT = D3DXVec3Dot(&vPN, &vTmp) / D3DXVec3Dot(&vPN, &vRayDir);
						vPos = vRayOrig + vRayDir*fT;
						vPos += ((m_pSelectedCube->vCenterPos*(-1.0f))*mat);

						__Vector3 vDiffPos;	vDiffPos.Set(vPos.x - m_vPos.x, 0, 0);
						TransDiff(&vDiffPos, NULL, NULL);
						m_vPos.x = vPos.x;
					}
					break;
				case DUMMY_Y:
					{
						vPN.Set(vCameraDir.x, 0, vCameraDir.z);
						vPN.Normalize();

						__Vector3 vTmp = vPV - vRayOrig;
						float fT = D3DXVec3Dot(&vPN, &vTmp) / D3DXVec3Dot(&vPN, &vRayDir);
						vPos = vRayOrig + vRayDir*fT;
						vPos += ((m_pSelectedCube->vCenterPos*(-1.0f))*mat);

						__Vector3 vDiffPos;	vDiffPos.Set(0, vPos.y - m_vPos.y, 0);
						TransDiff(&vDiffPos, NULL, NULL);
						m_vPos.y = vPos.y;
					}
					break;
				case DUMMY_Z:
					{
						vPN.Set(vCameraDir.x, vCameraDir.y, 0);
						vPN.Normalize();

						__Vector3 vTmp = vPV - vRayOrig;
						float fT = D3DXVec3Dot(&vPN, &vTmp) / D3DXVec3Dot(&vPN, &vRayDir);
						vPos = vRayOrig + vRayDir*fT;
						vPos += ((m_pSelectedCube->vCenterPos*(-1.0f))*mat);

						__Vector3 vDiffPos;	vDiffPos.Set(0, 0, vPos.z - m_vPos.z);
						TransDiff(&vDiffPos, NULL, NULL);
						m_vPos.z = vPos.z;
					}
					break;
				}
				return TRUE;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			m_pSelectedCube = Pick(point.x, point.y);
			if (m_pSelectedCube)
			{
				__VertexXyzT1* pSelVtx0 = m_SelVtxArray.GetAt(0);
				_ASSERT(pSelVtx0);
				m_vPrevPos = *pSelVtx0;

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
				m_pSelectedCube = NULL;
				return TRUE;
			}
		}
		break;
	case WM_RBUTTONDOWN:	// 큐브 선택 취소 및 이번 드래그로 움직인것 되돌려 놓기
		{
			if (m_pSelectedCube)
			{
				__Vector3 vDiffPos = m_vPrevPos - m_vPos;
				TransDiff(&vDiffPos, NULL, NULL);

				m_vPos = m_vPrevPos;

				ReleaseCapture();
				m_pSelectedCube = NULL;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

void CVtxPosDummy::TransDiff(__Vector3* pvDiffPos, __Quaternion* pqDiffRot, __Vector3* pvDiffScale)		// 차이만큼 선택된 오브젝트들을 변형시킨다.
{
	int i, iSize = m_SelVtxArray.GetSize();
	if (iSize<=0) return;
	if (pvDiffPos)
	{
		for (i=0; i<iSize; ++i)
		{
			__VertexXyzT1* pSelVtx = m_SelVtxArray.GetAt(i);
			_ASSERT(pSelVtx);
			pSelVtx->x += pvDiffPos->x;
			pSelVtx->y += pvDiffPos->y;
			pSelVtx->z += pvDiffPos->z;
		}
	}
}

void CVtxPosDummy::PosRotate(__Matrix44 vRotate,__Vector3 vCenterPos)
{
	__Vector3 vDummyPos;
	vDummyPos = Pos();
	vDummyPos -= vCenterPos;
	vDummyPos *= vRotate;
	vDummyPos += vCenterPos;
	PosSet(vDummyPos);
}
