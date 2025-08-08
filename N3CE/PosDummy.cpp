// PosDummy.cpp: implementation of the CPosDummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "n3me.h"
#include "PosDummy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPosDummy::CPosDummy()
{
}

CPosDummy::~CPosDummy()
{

}

void CPosDummy::SetSelObj(CN3Transform* pObj)
{
	m_SelObjArray.RemoveAll();
	if (pObj)
	{
		m_SelObjArray.Add(pObj);
		m_vPos = pObj->Pos();
//		m_qRot.x = m_qRot.y = m_qRot.z = m_qRot.w = 0;
	}
}

void CPosDummy::SetVertex(__Vector3* pVertex)
{
	if(pVertex) m_vPos = (*pVertex);
}

BOOL CPosDummy::MouseMsgFilter(LPMSG pMsg)
{
	if (m_SelObjArray.GetSize() == 0) return FALSE;

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
				__Vector3 vTmp;
				__Vector3 vCameraDir = s_CameraData.vAt - s_CameraData.vEye;	vCameraDir.Normalize();
				GetPickRay(point, vRayDir, vRayOrig);
				vPV = m_vPrevPos;
				__Matrix44 mat = m_Matrix;	mat.PosSet(0,0,0);

				switch(m_pSelectedCube->iType)
				{
				case DUMMY_CENTER:
					{
						vPN = vCameraDir;

						vTmp = vPV - vRayOrig;
						float fT = D3DXVec3Dot(&vPN, &vTmp) / D3DXVec3Dot(&vPN, &vRayDir);
						vPos = vRayOrig + vRayDir*fT;

						__Vector3 vDiffPos = vPos - m_vPos;
						TransDiff(&vDiffPos, NULL, NULL);
						m_vPos = vPos;
					}
					break;
				case DUMMY_X:
					{
						vPN.Set(0, vCameraDir.y, vCameraDir.z);		vPN.Normalize();

						vTmp = vPV - vRayOrig;
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
						vPN.Set(vCameraDir.x, 0, vCameraDir.z);		vPN.Normalize();

						vTmp = vPV - vRayOrig;
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
						vPN.Set(vCameraDir.x, vCameraDir.y, 0);		vPN.Normalize();

						vTmp = vPV - vRayOrig;
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
	}

	return CTransDummy::MouseMsgFilter(pMsg);
}
