// N3Shape.h: interface for the C3DObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Shape_h__INCLUDED_)
#define AFX_N3Shape_h__INCLUDED_

#pragma once

#include "N3TransformCollision.h"
#include "N3PMeshInstance.h"
#include "N3Texture.h"

#include <vector>

typedef std::vector<CN3Texture*> it_pTex;

class CN3SPart : public CN3BaseFileAccess
{
	friend class CN3Shape;

public:
	__Vector3 m_vPivot;       // Local 축
	__Matrix44 m_Matrix;      // World Matrix.. Shape Loading 때 미리 계산해야 좋다..
	BOOL m_bOutOfCameraRange; // Camera 범위 바깥에 있음...

	__Material m_Mtl;         // Material
	float m_fTexFPS;          // Texture Animation Interval;

	//	__Vector3	m_vWindFactorCur;		// 현재 바람 부는 값.. 이값으로 회전을 시킨다..
	//	__Vector3	m_vWindFactorToReach;	// 바람 부는 값..
	float m_fTimeToSetWind;             // 바람 부는 값을 바꾸기 위한 시간..
	float m_fWindFactorToReach;         // 현재 바람 부는 값.. 이값으로 회전을 시킨다..
	float m_fWindFactorCur;             // 바람 부는 값..

protected:
	std::vector<CN3Texture*> m_TexRefs; // Texture Reference Pointers
	CN3PMeshInstance m_PMInst;          // Progressive Mesh Instance

	float
		m_fTexIndex; // Current Texture Index.. Animation 시킬때 필요한 인덱스이다.. float 로 해서 텍스처 에니메이션 제어한다.

public:
	bool Load(File& file) override;
#ifdef _N3TOOL
	bool Save(File& file) override;
#endif // end of _N3TOOL

	int TexCount() const
	{
		return static_cast<int>(m_TexRefs.size());
	}

	CN3Texture* Tex(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_TexRefs.size()))
			return nullptr;

		return m_TexRefs[iIndex];
	}

	void TexAlloc(int nCount);

	CN3Texture* TexSet(int iIndex, const std::string& szFN)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_TexRefs.size()))
			return nullptr;

		s_MngTex.Delete(&m_TexRefs[iIndex]);
		m_TexRefs[iIndex] = s_MngTex.Get(szFN, true, s_Options.iTexLOD_Shape);
		return m_TexRefs[iIndex];
	}

	void TexSet(int iIndex, CN3Texture* pTex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_TexRefs.size()))
			return;

		s_MngTex.Delete(&m_TexRefs[iIndex]);
		m_TexRefs[iIndex] = pTex;
	}

	CN3PMeshInstance* MeshInstance()
	{
		return &m_PMInst;
	}

	CN3PMesh* Mesh()
	{
		return m_PMInst.GetMesh();
	}

	const CN3PMesh* Mesh() const
	{
		return m_PMInst.GetMesh();
	}

	void MeshSet(const std::string& szFN);

	void ReCalcMatrix(const __Matrix44& mtxParent)
	{
		m_Matrix.Identity();
		m_Matrix.PosSet(m_vPivot);
		m_Matrix *= mtxParent;
	}

	// 부모 행렬 즉 Shape 행렬, 회전쿼터니언 을 넣는다.
	void Tick(const __Matrix44& mtxParent, const __Quaternion& qRot, float fScale);
	void Render();
#ifdef _N3TOOL
	void RenderSelected(bool bWireFrame);
	void RenderAxis();
#endif // end of _N3TOOL

	// 월드 상의 최소값
	__Vector3 Min() const
	{
		const CN3PMesh* mesh = Mesh();
		if (mesh != nullptr)
			return mesh->Min() * m_Matrix;
		else
			return __Vector3(0, 0, 0);
	}

	// 월드 상의 최대값
	__Vector3 Max() const
	{
		const CN3PMesh* mesh = Mesh();
		if (mesh != nullptr)
			return mesh->Max() * m_Matrix;
		else
			return __Vector3(0, 0, 0);
	}

	float Radius() const
	{
		const CN3PMesh* mesh = Mesh();
		if (mesh != nullptr)
			return mesh->Radius();
		else
			return 0.0f;
	}

	void Release() override;

	CN3SPart();
	~CN3SPart() override;

	void PartialRender(int iCount, uint16_t* pIndices);
};

typedef std::vector<CN3SPart*>::iterator it_SPart;

class CN3Shape : public CN3TransformCollision
{
public:
	int m_iBelong;                  // 소속 - 0:소속 없음 1:엘모라드 2:카루스 3:?? ....
	int m_iEventID;                 // Event ID
	int m_iEventType;               // Event Type
	int m_iNPC_ID;                  // NPC 로 쓰는 오브젝트일 경우 NPC ID
	int m_iNPC_Status;              // NPC 로 쓰는 오브젝트일 경우 Default Status

	bool m_bDontRender;             // 카메라 거리에 따라 이플래그가 설정되면 렌더링하지 않는다..
	bool m_bVisible;                // ..

	std::vector<CN3SPart*> m_Parts; // Part Data Pointer Linked List

public:
#ifdef _N3TOOL
	bool SaveToSameFolderAndMore(const std::string& szFullPath, const std::string& szRelativePath);
	bool SaveToSameFolder(const std::string& szFullPath);
	void RemoveRenderFlags(int nFlags = -1);
	void MakeDefaultMaterial();
#endif // end of _N3TOOL
	// 정밀하게 폴리곤 단위로 체크 - 먼저 박스 체크후 다시 정밀 체크..
	int CheckCollisionPrecisely(bool bIgnoreBoxCheck, int ixScreen, int iyScreen,
		__Vector3* pVCol = nullptr, __Vector3* pVNormal = nullptr) override;
	// 정밀하게 폴리곤 단위로 체크 - 먼저 박스 체크후 다시 정밀 체크..
	int CheckCollisionPrecisely(bool bIgnoreBoxCheck, const __Vector3& vPos, const __Vector3& vDir,
		__Vector3* pVCol = nullptr, __Vector3* pVNormal = nullptr);
	bool MakeCollisionMeshByParts();       // 충돌 메시를 박스 형태로 다시 만든다...
	bool MakeCollisionMeshByPartsDetail(); // 현재 모습 그대로... 충돌 메시를 만든다...

	void FindMinMax() override;
	void ReCalcMatrix() override;
	void ReCalcPartMatrix();
	void Tick(float fFrm = FRAME_SELFPLAY) override;
	virtual void Render();

	int PartCount() const
	{
		return static_cast<int>(m_Parts.size());
	}

	CN3SPart* Part(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_Parts.size()))
			return nullptr;

		return m_Parts[iIndex];
	}

	CN3SPart* PartAdd()
	{
		CN3SPart* pPart = new CN3SPart();
		m_Parts.push_back(pPart);
		return pPart;
	}
	void PartDelete(int iIndex);

	bool Load(File& file) override;
#ifdef _N3TOOL
	bool Save(File& file) override;
	void RenderSelected(bool bWireFrame);
	void RenderSelected(int iPart, bool bWireFrame);
	bool IsPMeshProcessed();
#endif // end of _N3TOOL

	void Release() override;
	CN3Shape();
	~CN3Shape() override;

	//	By : Ecli666 ( On 2002-08-06 오후 4:33:04 )
	//
	void SetMaxLOD();
	__Matrix44 GetPartMatrix(int iPartIndex) const;
	void PartialRender(int iPartIndex, int iCount, uint16_t* pIndices);
	int GetIndexbufferCount(int iPartIndex);
	int GetIndexByiOrder(int iPartIndex, int iOrder);
	__Vector3 GetVertexByIndex(int iPartIndex, int iIndex);
	int GetColIndexbufferCount();
	int GetColIndexByiOrder(int iOrder);
	__Vector3 GetColVertexByIndex(int iIndex);
	void PartialColRender(int iCount, int* piIndices);
	void PartialGetCollision(int iIndex, __Vector3& vec);
	bool LoadTransformOnly(File& file);
	//	~(By Ecli666 On 2002-08-06 오후 4:33:04 )
};

#endif // !defined(AFX_N3Shape_h__INCLUDED_)
