// Ground.cpp 
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Ground.h"
#include <N3Base/N3Texture.h>

CGround::CGround()
{
	m_iSize = 128;
	m_iTileSize = 4;
	m_pVB = new __VertexT1[(m_iSize/m_iTileSize) * (m_iSize/m_iTileSize) * 6];

	int NumTile = m_iSize/m_iTileSize;
	for(int x=0;x<NumTile;x++)
	{
		for(int z=0;z<NumTile;z++)
		{
			float y = 0.0f;
			//if(x==0 || z==0 || x==NumTile-1 || z==NumTile-1) y = ((float)(rand()%300)) / 100.0f;
			int idx = (z + NumTile*x) * 6;
			__Vector3 vLB;
			vLB.Set((x*m_iTileSize) - (m_iSize/2), y, (z*m_iTileSize) - (m_iSize/2));
			m_pVB[idx].Set(vLB.x, vLB.y, vLB.z, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
			m_pVB[idx+1].Set(vLB.x, vLB.y, vLB.z+m_iTileSize, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
			m_pVB[idx+2].Set(vLB.x+m_iTileSize, vLB.y, vLB.z+m_iTileSize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
			m_pVB[idx+3] = m_pVB[idx];
			m_pVB[idx+4] = m_pVB[idx+2];
			m_pVB[idx+5].Set(vLB.x+m_iTileSize, vLB.y, vLB.z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
		}
	}

	m_pTile = CN3Base::s_MngTex.Get("tile.bmp");
}

CGround::~CGround()
{
	if(m_pTile) CN3Base::s_MngTex.Delete(&m_pTile);

	if(m_pVB) delete[] m_pVB;
	m_pVB = nullptr;
}

void CGround::Render()
{
	CN3Base::s_lpD3DDev->SetFVF(FVF_VNT1);

	DWORD dwAlpha, dwZEnable;
	
	s_lpD3DDev->GetRenderState( D3DRS_ZWRITEENABLE, &dwZEnable );
	s_lpD3DDev->GetRenderState( D3DRS_ALPHABLENDENABLE, &dwAlpha );

	s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	s_lpD3DDev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

	if(m_pTile) s_lpD3DDev->SetTexture(0, m_pTile->Get());
	else s_lpD3DDev->SetTexture(0, nullptr);

	s_lpD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_iSize/m_iTileSize*m_iSize/m_iTileSize*2, m_pVB, sizeof(__VertexT1));

	s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, dwAlpha );
	s_lpD3DDev->SetRenderState( D3DRS_ZWRITEENABLE, dwZEnable );
}

void CGround::SetTex(const char* szPath)
{
	if(m_pTile) CN3Base::s_MngTex.Delete(&m_pTile);
	m_pTile = CN3Base::s_MngTex.Get(szPath);
}