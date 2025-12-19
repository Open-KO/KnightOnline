// N3Pond.h: interface for the CN3Pond class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Pond_H__B9A59A74_B468_4552_8D80_E8AF3FE586E0__INCLUDED_)
#define AFX_N3Pond_H__B9A59A74_B468_4552_8D80_E8AF3FE586E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_PONDMESH_LINE		200
#define MAX_PONDMESH_VERTEX		200*4
#define MAX_POND_TEX	32


#include <N3Base/N3BaseFileAccess.h>

class CN3Pond : public CN3BaseFileAccess
{
public:
	CN3Pond();
	virtual		~CN3Pond();

	struct __VertexPond
	{
	public:
		float x,y,z;
		float nx,ny,nz;
		D3DCOLOR	color;
		float u,v, u2,v2;
		void Set(float sx, float sy,float sz, float snx, float sny, float snz, D3DCOLOR scolor, float su, float sv, float su2, float sv2)
		{
			x = sx, y = sy, z = sz;
			nx = snx, ny = sny, nz = snz;
			color = scolor;
			u = su, v = sv;
			u2 = su2, v2 = sv2;
		}
	};

	class CPongMesh
	{
	public:
		CN3Texture*		m_pTexWave;
		BOOL			m_bTick2Rand;	//	시야에 들어와 tick과rend를 실행결정
		__VertexPond*	m_pVertices;	//	Vertices
		float*			m_pfVelocityArray;	//	계산 저장
		float			m_pfMaxHeight;	//	물결이 어느정도 이상 올라가지 못하게 함
		uint16_t*			m_wpIndex;		//	그림을 그릴 순서
		int				m_iIC;			// Index Buffer Count.
		int				m_iVC;			// Vertex Count.

		int				m_iWidthVtx,m_iHeightVtx;		// 계산에 필요
		float			m_fmin,m_fmax,m_fmaxcal,m_fmincal;

		__Vector3		m_vCenterPo;	//	연못의 중간지점
		float			m_fRadius;		//	연못의 지름

		CPongMesh() {m_bTick2Rand=FALSE;m_pVertices=nullptr,m_wpIndex=nullptr;m_pfVelocityArray=nullptr;m_pTexWave=nullptr;};
		virtual ~CPongMesh() 
		{
			if(m_pVertices) delete []m_pVertices;m_pVertices=nullptr;
			if(m_wpIndex) delete []m_wpIndex;m_wpIndex=nullptr;
			if(m_pfVelocityArray) delete []m_pfVelocityArray;m_pfVelocityArray=nullptr;
			if(m_pTexWave)	{CN3Base::s_MngTex.Delete(&m_pTexWave); m_pTexWave = nullptr;}
		};
	};

public:
	int				m_iPondMeshNum;	//	전체 연못의 갯수
	CPongMesh*		m_pCPondMesh;	//	연못의 정보

	CN3Texture*		m_pTexPond[MAX_POND_TEX];	
	float			m_fTexIndex;

	int				m_iMaxVtxNum;	//	가장 많은 vertices수
	float*			m_pfMaxVtx;		//	물결높이 계산을 위한 임시

public:
	void		Release() override;
	bool		Load(File& file) override;
	void		Render();
	void		Tick();

private:
	void		CheckHeight(float& ChkHeight)
	{
		if ( ChkHeight < -0.01f )	ChkHeight +=  0.01f;
		else if ( ChkHeight >  0.01f )	ChkHeight += -0.01f;
	};

	void		UpdateWaterPositions();
};

#endif // !defined(AFX_N3Pond_H__B9A59A74_B468_4552_8D80_E8AF3FE586E0__INCLUDED_)
