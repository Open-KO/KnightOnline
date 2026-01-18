// RiverMesh.h: interface for the CRiverMesh class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RIVERMESH_H__473FE46A_9CD7_4D5C_8196_45F0BFA94F04__INCLUDED_)
#define AFX_RIVERMESH_H__473FE46A_9CD7_4D5C_8196_45F0BFA94F04__INCLUDED_

#pragma once

//const int MAXNUM_RIVERMESH_VERTEX = 2048;
#define MAX_RIVERMESH_LINE   200
#define MAX_RIVERMESH_VERTEX 200 * 4
#define MAX_RIVERMESH_INDEX  199 * 18
#define INDEXBUFFER_PERLINE  18

#include <N3Base/N3BaseFileAccess.h>

class CRiverMesh : public CN3BaseFileAccess
{
public:
	CRiverMesh();
	~CRiverMesh() override;

	// Attributes
public:
	int GetRiverID() const
	{
		return m_iRiverID;
	}
	void SetRiverID(int iRiverID)
	{
		m_iRiverID = iRiverID;
	}
	float GetSpeed1() const
	{
		return m_fSpeed1;
	}
	float GetSpeed2() const
	{
		return m_fSpeed2;
	}
	void SetSpeed1(float fSpeed)
	{
		m_fSpeed1 = fSpeed;
	}
	void SetSpeed2(float fSpeed)
	{
		m_fSpeed2 = fSpeed;
	}
	float GetMeterPerU() const
	{
		return m_fMeterPerU;
	}
	void SetMeterPerU(float fMeterPerU)
	{
		m_fMeterPerU = fMeterPerU;
	}
	float GetMeterPerV() const
	{
		return m_fMeterPerV;
	}
	void SetMeterPerV(float fMeterPerV)
	{
		m_fMeterPerV = fMeterPerV;
	}
	float GetMeterPerU2() const
	{
		return m_fMeterPerU2;
	}
	void SetMeterPerU2(float fMeterPerU)
	{
		m_fMeterPerU2 = fMeterPerU;
	}
	float GetMeterPerV2() const
	{
		return m_fMeterPerV2;
	}
	void SetMeterPerV2(float fMeterPerV)
	{
		m_fMeterPerV2 = fMeterPerV;
	}
	CN3Texture* TexGet() const
	{
		return m_pTexture;
	}
	int VertexCount() const
	{
		return m_iVC;
	}
	int IndexCount() const
	{
		return m_iIC;
	}
	int GetAnimTexCount() const
	{
		return m_iAnimTextureCount;
	}
	CN3Texture* AnimTexGet(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= m_iAnimTextureCount)
			return nullptr;
		return m_pAnimTextures[iIndex];
	}
	float GetAnimTexFPS() const
	{
		return m_fAnimTexFPS;
	}
	void SetAnimTexFPS(float fFPS)
	{
		m_fAnimTexFPS = fFPS;
	}
	DWORD GetAlphaFactor() const
	{
		return m_dwAlphaFactor;
	}
	void SetAlphaFactor(DWORD dwFactor)
	{
		m_dwAlphaFactor = dwFactor;
	}

protected:
	int m_iRiverID;
	__VertexXyzT2 m_pVertices[MAX_RIVERMESH_VERTEX];
	WORD m_wIndex[MAX_RIVERMESH_INDEX];
	int m_iIC;                    // Index Buffer Count.
	int m_iVC;                    // Vertex Count.
	CN3Texture* m_pTexture;
	int m_iAnimTextureCount;      // Animation되는 텍스쳐 갯수
	CN3Texture** m_pAnimTextures; // Animation되는 텍스쳐 포인터들..
	float m_fSpeed1;              // 유속 : 초당 v좌표의 변화량
	float m_fSpeed2;              // 유속 : 초당 v2좌표의 변화량
	float m_fMeterPerU;           // U좌표 1.0에 해당하는 강의 길이
	float m_fMeterPerV;           // V좌표 1.0에 해당하는 강의 길이
	float m_fMeterPerU2;          // U2좌표 1.0에 해당하는 강의 길이
	float m_fMeterPerV2;          // V2좌표 1.0에 해당하는 강의 길이
	float m_fAnimTexFPS;          // AnimTexture의 초당 frame 수;
	DWORD m_dwAlphaFactor;        // 강을 투명하게 하기 위한 알파값

								  // Operations
public:
	void Release() override;
	bool Load(File& file) override;
	bool Save(File& file) override;
	void Render();
	void RenderVertexPoint(); // 잘보이게 점만 다시 그리기

	int AddVertex(__Vector3& vPos1, __Vector3& vPos2, __Vector3& vPos3, __Vector3& vPos4);
	int AddVertex();
	int DeleteVertex(int iIndex); // 쌍으로 지우고 뒤에 있는 것들을 앞으로 두개씩 이동
	__VertexXyzT2* GetVertex(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_iVC)
			return nullptr;
		return m_pVertices + iIndex;
	}
	__Vector3 GetCenter();

	BOOL SetTextureName(LPCTSTR pszFName);
	BOOL SetAnimTextureName(LPCTSTR pszFName, LPCTSTR pszExt, int iCount);
	void ReCalcUV();

protected:
	void ReleaseAnimTextures();
};

#endif // !defined(AFX_RIVERMESH_H__473FE46A_9CD7_4D5C_8196_45F0BFA94F04__INCLUDED_)
