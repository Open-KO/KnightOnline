// N3FXShape.h: interface for the CN3FXShape class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __N3FXSHAPE_H__
#define __N3FXSHAPE_H__

#pragma once

#include "N3TransformCollision.h"
#include "N3Texture.h"
#include "N3FXPMesh.h"
#include "N3FXPMeshInstance.h"

#include <vector>

class CN3FXSPart : public CN3BaseFileAccess
{
	friend class CN3FXShape;

public:
	__Material m_Mtl;         // Material
	__Vector3 m_vPivot;       // Local 축
	__Matrix44 m_WorldMtx;    // World Matrix.. Shape Loading 때 미리 계산해야 좋다..
	BOOL m_bOutOfCameraRange; // Camera 범위 바깥에 있음...

	float m_fTexFPS;          // Texture Animation Interval;
	bool m_bTexLoop;

	CN3FXShape* m_pRefShape;

protected:
	std::vector<CN3Texture*> m_TexRefs; // Texture Reference Pointers
	CN3FXPMeshInstance m_FXPMInst;      // Progressive Mesh Instance

	float
		m_fTexIndex; // Current Texture Index.. Animation 시킬때 필요한 인덱스이다.. float 로 해서 텍스처 에니메이션 제어한다.

public:
	bool Load(File& file) override;
	bool Save(File& file) override;
	void Duplicate(CN3FXSPart* pSrc);

	int TexCount() const
	{
		return static_cast<int>(m_TexRefs.size());
	}

	CN3Texture* Tex(int iIndex);
	void TexAlloc(int nCount);
	CN3Texture* TexSet(int iIndex, const std::string& szFN);

	__Vector3 Min()
	{
		if (m_FXPMInst.GetMesh())
			return m_FXPMInst.GetMesh()->Min() * m_WorldMtx;
		else
			return __Vector3(0, 0, 0);
	} // 월드 상의 최소값
	__Vector3 Max()
	{
		if (m_FXPMInst.GetMesh())
			return m_FXPMInst.GetMesh()->Max() * m_WorldMtx;
		else
			return __Vector3(0, 0, 0);
	} // 월드 상의 최대값
	float Radius()
	{
		if (m_FXPMInst.GetMesh())
			return m_FXPMInst.GetMesh()->Radius();
		else
			return 0.0f;
	}

	CN3FXPMesh* Mesh()
	{
		return m_FXPMInst.GetMesh();
	}
	__VertexXyzColorT1* GetColorVertices()
	{
		return m_FXPMInst.GetVertices();
	}
	void SetColor(uint32_t dwColor = 0xffffffff)
	{
		m_FXPMInst.SetColor(dwColor);
	}
	bool MeshSet(const std::string& szFN);
	void Tick(const __Matrix44& mtxParent);
	void Render();
	void Release() override;

	CN3FXSPart();
	~CN3FXSPart() override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CN3FXShape
//
class CN3FXShape : public CN3TransformCollision
{
public:
	std::vector<CN3FXSPart*> m_Parts; // Part Data Pointer Linked List

	__Matrix44 m_mtxParent;
	__Matrix44 m_mtxFinalTransform;

	uint32_t m_dwSrcBlend;
	uint32_t m_dwDestBlend;
	BOOL m_bAlpha;

	uint32_t m_dwZEnable;
	uint32_t m_dwZWrite;
	uint32_t m_dwLight;
	uint32_t m_dwDoubleSide;

public:
	void SetPartsMtl(BOOL bAlpha, uint32_t dwSrcBlend, uint32_t dwDestBlend, uint32_t dwZEnable,
		uint32_t dwZWrite, uint32_t dwLight, uint32_t dwDoubleSide);
	__Vector3 CenterPos();

	void SetCurrFrm(float fFrm);
	float GetCurrFrm();
	float GetWholeFrm()
	{
		return m_fFrmWhole;
	}

	void FindMinMax() override;

	int PartCount() const
	{
		return static_cast<int>(m_Parts.size());
	}

	CN3FXSPart* Part(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_Parts.size()))
			return nullptr;

		return m_Parts[iIndex];
	}

	CN3FXSPart* PartAdd()
	{
		CN3FXSPart* pPart = new CN3FXSPart();
		m_Parts.push_back(pPart);
		return pPart;
	}

	void PartDelete(int iIndex);

	bool Load(File& file) override;
	bool Save(File& file) override;
	void Duplicate(CN3FXShape* pSrc);

	void Tick(float fFrm = FRAME_SELFPLAY) override;
	void Render();
	void Release() override;

public:
	void SetMaxLOD();
	CN3FXShape();
	~CN3FXShape() override;
};

#endif // !defined(AFX_N3Shape_h__INCLUDED_)
