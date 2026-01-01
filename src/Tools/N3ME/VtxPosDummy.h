// VtxPosDummy.h: interface for the CVtxPosDummy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VTXPOSDUMMY_H__FA661289_2E6D_4F3A_B490_3B50794389FC__INCLUDED_)
#define AFX_VTXPOSDUMMY_H__FA661289_2E6D_4F3A_B490_3B50794389FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TransDummy.h"

// 점을 이동시키는 기능을 가졌다.
class CVtxPosDummy : public CTransDummy
{
public:
	CVtxPosDummy();
	virtual ~CVtxPosDummy();

// Attributes
public:
protected:
	CTypedPtrArray<CPtrArray, __VertexXyzT1*> m_SelVtxArray;	// 선택된 점들

// Operations
public:
	virtual void Tick();
	virtual void Render();
	virtual void Release();
	virtual void SetSelObj(CN3Transform* pObj);		// 선택된 객체 바꾸기
	virtual void AddSelObj(CN3Transform* pObj);	// 선택된 객체 추가
	virtual BOOL MouseMsgFilter(LPMSG pMsg);			// 마우스 메세지 처리

	void SetSelVtx(__VertexXyzT1* pVtx);		// 선택된 점 바꾸기
	void AddSelVtx(__VertexXyzT1* pVtx);				// 선택된 점 추가

	void AddPos(__Vector3 vAddPos) {m_vPos+=vAddPos;}
	void PosRotate(__Matrix44 vRotate,__Vector3 vCenterPos);
protected:
	virtual void TransDiff(__Vector3* pvDiffPos, __Quaternion* pqDiffRot, __Vector3* pvDiffScale);		// 차이만큼 선택된 오므젝트들을 변형시킨다.
};

#endif // !defined(AFX_VTXPOSDUMMY_H__FA661289_2E6D_4F3A_B490_3B50794389FC__INCLUDED_)
