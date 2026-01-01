// ScaleDummy.cpp: implementation of the CScaleDummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "ScaleDummy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScaleDummy::CScaleDummy()
{
}

CScaleDummy::~CScaleDummy()
{

}

BOOL CScaleDummy::MouseMsgFilter(LPMSG pMsg)
{
	if (m_SelObjArray.GetSize() == 0)
		return FALSE;

	static POINT ptStartCursor;
	switch (pMsg->message)
	{
	case WM_MOUSEMOVE:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			DWORD_PTR nFlags = pMsg->wParam;

			int iDiff = point.x - ptStartCursor.x;
			if (m_pSelectedCube && (nFlags & MK_LBUTTON))
			{
				float fFactor = 1.0f + 0.01f*iDiff;
				//if (fFactor<0.0f) fFactor = 0.0001f;
				switch(m_pSelectedCube->iType)
				{
				case DUMMY_CENTER:
					{
						__Vector3 vDiffScale;	vDiffScale.Set(fFactor, fFactor, fFactor);
						TransDiff(nullptr, nullptr, &vDiffScale);
					}
					break;
				case DUMMY_X:
					{
						__Vector3 vDiffScale;	vDiffScale.Set(fFactor, 1, 1);
						TransDiff(nullptr, nullptr, &vDiffScale);
					}
					break;
				case DUMMY_Y:
					{
						__Vector3 vDiffScale;	vDiffScale.Set(1, fFactor, 1);
						TransDiff(nullptr, nullptr, &vDiffScale);
					}
					break;
				case DUMMY_Z:
					{
						__Vector3 vDiffScale;	vDiffScale.Set(1, 1, fFactor);
						TransDiff(nullptr, nullptr, &vDiffScale);
					}
					break;
				}
				return TRUE;
			}
		}
	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			ptStartCursor.x = point.x;
			ptStartCursor.y = point.y;
		}
		break;
	}

	return CTransDummy::MouseMsgFilter(pMsg);
}
