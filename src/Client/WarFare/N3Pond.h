// N3Pond.h: interface for the CN3Pond class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Pond_H__B9A59A74_B468_4552_8D80_E8AF3FE586E0__INCLUDED_)
#define AFX_N3Pond_H__B9A59A74_B468_4552_8D80_E8AF3FE586E0__INCLUDED_

#pragma once

inline constexpr int MAX_PONDMESH_LINE   = 200;
inline constexpr int MAX_PONDMESH_VERTEX = 200 * 4;
inline constexpr int MAX_POND_TEX        = 32;

#include <N3Base/N3BaseFileAccess.h>

#include <vector>

class CN3Pond : public CN3BaseFileAccess
{
public:
	CN3Pond();
	~CN3Pond() override;

	struct __VertexPond
	{
	public:
		float x, y, z;
		float nx, ny, nz;
		D3DCOLOR color;
		float u, v, u2, v2;
		void Set(float sx, float sy, float sz, float snx, float sny, float snz, D3DCOLOR scolor, float su, float sv, float su2, float sv2)
		{
			x = sx, y = sy, z = sz;
			nx = snx, ny = sny, nz = snz;
			color = scolor;
			u = su, v = sv;
			u2 = su2, v2 = sv2;
		}
	};

	class CPondMesh
	{
	public:
		CN3Texture* m_pTexWave;
		BOOL m_bTick2Rand;             //	시야에 들어와 tick과rend를 실행결정
		__VertexPond* m_pVertices;     //	Vertices
		float* m_pfVelocityArray;      //	계산 저장
		float m_pfMaxHeight;           //	물결이 어느정도 이상 올라가지 못하게 함
		uint16_t* m_wpIndex;           //	그림을 그릴 순서
		int m_iIC;                     // Index Buffer Count.
		int m_iVC;                     // Vertex Count.

		int m_iWidthVtx, m_iHeightVtx; // 계산에 필요
		float m_fmin, m_fmax, m_fmaxcal, m_fmincal;

		__Vector3 m_vCenterPo;         //	연못의 중간지점
		float m_fRadius;               //	연못의 지름

		CPondMesh()
		{
			m_bTick2Rand      = FALSE;
			m_pVertices       = nullptr;
			m_wpIndex         = nullptr;
			m_pfVelocityArray = nullptr;
			m_pTexWave        = nullptr;
			m_iWidthVtx       = 0;
			m_iHeightVtx      = 0;
			m_vCenterPo       = {};
			m_fRadius         = 0.0f;
			m_iIC             = 0;
			m_pfMaxHeight     = 0.0f;
			m_iVC             = 0;
			m_fmin            = 0.0f;
			m_fmincal         = 0.0f;
			m_fmaxcal         = 0.0f;
			m_fmax            = 0.0f;
		}

		~CPondMesh()
		{
			delete[] m_pVertices;
			m_pVertices = nullptr;

			delete[] m_wpIndex;
			m_wpIndex = nullptr;

			delete[] m_pfVelocityArray;
			m_pfVelocityArray = nullptr;

			CN3Base::s_MngTex.Delete(&m_pTexWave);
			m_pTexWave = nullptr;
		};
	};

public:
	std::vector<CPondMesh> m_PondMeshes; //	연못의 정보

	CN3Texture* m_pTexPond[MAX_POND_TEX];
	float m_fTexIndex;

	int m_iMaxVtxNum;  //	가장 많은 vertices수
	float* m_pfMaxVtx; //	물결높이 계산을 위한 임시

public:
	void Release() override;
	bool Load(File& file) override;
	void Render();
	void Tick();

private:
	void UpdateWaterPositions();
};

#endif // !defined(AFX_N3Pond_H__B9A59A74_B468_4552_8D80_E8AF3FE586E0__INCLUDED_)
