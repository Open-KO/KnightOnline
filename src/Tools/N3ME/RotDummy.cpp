// RotDummy.cpp: implementation of the CRotDummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "RotDummy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRotDummy::CRotDummy()
{
}

CRotDummy::~CRotDummy()
{
}

BOOL CRotDummy::MouseMsgFilter(LPMSG pMsg)
{
	if (m_SelObjArray.GetSize() == 0)
		return FALSE;

	static POINT ptPrevCursor;
	switch (pMsg->message)
	{
		case WM_MOUSEMOVE:
		{
			POINT point        = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
			DWORD_PTR nFlags   = pMsg->wParam;

			int iDiff          = point.x - ptPrevCursor.x;
			const float fDelta = 0.005f;
			ptPrevCursor       = point;
			if (m_pSelectedCube && (nFlags & MK_LBUTTON))
			{
				switch (m_pSelectedCube->iType)
				{
					case DUMMY_CENTER:
					{
					}
					break;
					case DUMMY_X:
					{
						__Quaternion q;
						q.RotationAxis(1, 0, 0, fDelta * iDiff);
						//						m_qRot *= q;

						this->TransDiff(nullptr, &q, nullptr);
					}
					break;
					case DUMMY_Y:
					{
						__Quaternion q;
						q.RotationAxis(0, 1, 0, fDelta * iDiff);
						//						m_qRot *= q;

						this->TransDiff(nullptr, &q, nullptr);
					}
					break;
					case DUMMY_Z:
					{
						__Quaternion q;
						q.RotationAxis(0, 0, 1, fDelta * iDiff);
						//						m_qRot *= q;

						this->TransDiff(nullptr, &q, nullptr);
					}
					break;
				}
				return TRUE;
			}
		}
		case WM_LBUTTONDOWN:
		{
			POINT point    = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
			ptPrevCursor.x = point.x;
			ptPrevCursor.y = point.y;
		}
		break;
	}

	return CTransDummy::MouseMsgFilter(pMsg);
}

void CRotDummy::SetSelObj(CN3Transform* pObj)
{
	m_SelObjArray.RemoveAll();
	if (pObj)
	{
		m_SelObjArray.Add(pObj);
		m_vPos = pObj->Pos();

		//		m_qRot = pObj->Rot();
		m_qRot.Identity(); // Quaternion 회전이기 땜시... 초기화를 해준다..
	}
}
