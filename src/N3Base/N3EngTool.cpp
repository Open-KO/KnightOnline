// N3EngTool.cpp: implementation of the CN3EngTool class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Light.h"
#include "N3EngTool.h"
#include "N3Texture.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3EngTool::CN3EngTool()
{
	// ½ÊÀÚ¼± »ý¼º..
	int i = 0;
	for(i = -10; i < 10; i++)
	{
		m_VAxis[ 0 + i + 10].Set(i * 500.0f, 0, 0, 0xffff0000); // X
		m_VAxis[20 + i + 10].Set(0, i * 500.0f, 0, 0xff00ff00); // Y
		m_VAxis[40 + i + 10].Set(0, 0, i * 500.0f, 0xff0000ff); // Z
	}

	m_VDir[0].Set(10, 0, 0, 0xffff0000);
	m_VDir[1].Set(7, 3, 0, 0xffff0000);

	m_VDir[2].Set(0, 10, 0, 0xff00ff00);
	m_VDir[3].Set(0, 7, -3, 0xff00ff00);

	m_VDir[4].Set(0, 0, 10, 0xff0000ff);
	m_VDir[5].Set(0, 3, 7, 0xff0000ff);
	
	m_pVGrids = nullptr;
	m_nGridLineCount = 0;
	
//	m_lpD3DDevExtra = nullptr;
}

CN3EngTool::~CN3EngTool()
{
	delete [] m_pVGrids; m_pVGrids = nullptr;
//	if(m_lpD3DDevExtra) m_lpD3DDevExtra->Release();
//	m_lpD3DDevExtra = nullptr;
}

// ¿ùµå Ãà ±×¸®±â..
void CN3EngTool::RenderGrid(const __Matrix44& mtxWorld)
{
	DWORD dwAlpha, dwFog, dwLight;
	s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	
	if(dwFog) s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if(dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if(dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	
	static __Material smtl;
	static bool bInit = false;
	if(false == bInit)
	{
		smtl.Init();
		bInit = true;
	}

	s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());
	s_lpD3DDev->SetTexture(0, nullptr);

	s_lpD3DDev->SetFVF(FVF_CV); //s_lpD3DDev->SetVertexShader(FVF_CV);

	if(m_pVGrids) // ±×¸®µå ±×¸®±â..
	{
		s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, m_nGridLineCount, m_pVGrids, sizeof(__VertexColor)); // X
	}

	// Ãà ±×¸®±â..
//	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 19, &m_VAxis[0], sizeof(__VertexColor)); // X
//	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 19, &m_VAxis[20], sizeof(__VertexColor)); // Y
//	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 19, &m_VAxis[40], sizeof(__VertexColor)); // Z

	if(dwFog) s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	if(dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	if(dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
}

void CN3EngTool::RenderAxis(bool bShowDir)
{
	DWORD dwAlpha, dwFog, dwLight;
	s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	
	if(dwFog) s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if(dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	if(dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	
	static __Matrix44 stm;
	static __Material smtl;
	static bool bInit = false;
	if(false == bInit)
	{
		stm.Identity();
		smtl.Init();
		bInit = true;
	}

	s_lpD3DDev->SetTransform(D3DTS_WORLD, stm.toD3D());
	s_lpD3DDev->SetTexture(0, nullptr);

	s_lpD3DDev->SetFVF(FVF_CV); //s_lpD3DDev->SetVertexShader(FVF_CV);

	// Ãà ±×¸®±â..
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 19, &m_VAxis[0], sizeof(__VertexColor)); // X
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 19, &m_VAxis[20], sizeof(__VertexColor)); // Y
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 19, &m_VAxis[40], sizeof(__VertexColor)); // Z

	if(bShowDir) s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 3, &m_VDir[0], sizeof(__VertexColor)); // Z

	if(dwFog) s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	if(dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	if(dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
}


//
// pTex==NULLÀÏ¶§ °ËÁ¤»öÀ» Âï±â À§ÇØ ¼öÁ¤ÇßÀ½...by lynus 2001. 12. 7.
//
void CN3EngTool::RenderTexturePreview(CN3Texture *pTex, HWND hWndDiffuse, RECT* pRCSrc)
{
	//if(pTex == nullptr) return;
	if(hWndDiffuse == nullptr) return;
	
	if(pTex == nullptr)
	{
		RECT rcPreview;
		::GetClientRect(hWndDiffuse, &rcPreview);

		int nW = rcPreview.right - rcPreview.left;
		int nH = rcPreview.bottom - rcPreview.top;

		D3DRECT rcClear = { rcPreview.left, rcPreview.top, rcPreview.right, rcPreview.bottom };
		s_lpD3DDev->Clear(1, &rcClear, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, D3DCOLOR_ARGB(255,64,64,64), 1.0f, 0);

		RECT rcTex = rcPreview;
		float fRatioDest = (float)nW / (float)nH;
		
		if(fRatioDest > 1.0f) // °¡·Î°¡ ±ä°æ¿ì
		{
			int nDec = (nW - nH) / 2;
			rcTex.left += nDec;
			rcTex.right -= nDec;
		}
		else if(fRatioDest < 1.0f) // ¼¼·Î°¡ ±ä°æ¿ì
		{
			int nDec = (nH - nW) / 2;
			rcTex.top += nDec;
			rcTex.bottom -= nDec;
		}

		float fU_Left = 0.0f, fV_Top = 0.0f, fU_Right = 1.0f, fV_Bottom = 1.0f; // ±âº» UV ÁÂÇ¥
	
		s_lpD3DDev->BeginScene();

		m_VPreview[0].Set((float)rcTex.left, (float)rcTex.top, 0.5f, 1.0f, 0xff000000, fU_Left, fV_Top);
		m_VPreview[1].Set((float)rcTex.right, (float)rcTex.top, 0.5f, 1.0f, 0xff000000, fU_Right, fV_Top);
		m_VPreview[2].Set((float)rcTex.right, (float)rcTex.bottom, 0.5f, 1.0f, 0xff000000, fU_Right, fV_Bottom);
		m_VPreview[3] = m_VPreview[0];
		m_VPreview[4] = m_VPreview[2];
		m_VPreview[5].Set((float)rcTex.left, (float)rcTex.bottom, 0.5f, 1.0f, 0xff000000, fU_Left, fV_Bottom);

		s_lpD3DDev->SetTexture(0, nullptr);
		DWORD ColorOP;
		s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &ColorOP);

		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);		

		s_lpD3DDev->SetFVF(FVF_TRANSFORMED); //s_lpD3DDev->SetVertexShader(FVF_TRANSFORMED);
		s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, &m_VPreview, sizeof(__VertexTransformed));

		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, ColorOP);

		s_lpD3DDev->EndScene();
		s_lpD3DDev->Present(&rcPreview, &rcPreview, hWndDiffuse, nullptr);
		return;
	}
	
//	if(hWndDiffuse != nullptr)
	{
		// ÅØ½ºÃ³ ÇÁ¸®ºä
		RECT rcPreview;
		HRESULT hr;
		::GetClientRect(hWndDiffuse, &rcPreview);

		int nW = rcPreview.right - rcPreview.left;
		int nH = rcPreview.bottom - rcPreview.top;

		D3DRECT rcClear = { rcPreview.left, rcPreview.top, rcPreview.right, rcPreview.bottom };
		hr = s_lpD3DDev->Clear(1, &rcClear, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, D3DCOLOR_ARGB(255,64,64,64), 1.0f, 0);

		if(pTex->Get() != nullptr)
		{
			RECT rcTex = rcPreview;
			float fRatioDest = (float)nW / (float)nH;
			float fRatioSrc = (float)(pTex->Width()) / (float)(pTex->Height());

			if(fRatioDest > 1.0f) // °¡·Î°¡ ±ä°æ¿ì
			{
				int nDec = (nW - nH) / 2;
				rcTex.left += nDec;
				rcTex.right -= nDec;
			}
			else if(fRatioDest < 1.0f) // ¼¼·Î°¡ ±ä°æ¿ì
			{
				int nDec = (nH - nW) / 2;
				rcTex.top += nDec;
				rcTex.bottom -= nDec;
			}


			float fU_Left = 0.0f, fV_Top = 0.0f, fU_Right = 1.0f, fV_Bottom = 1.0f; // ±âº» UV ÁÂÇ¥
			// ¸¸¾à Rect °¡ ÀÖÀ¸¸é..
			if(pRCSrc)
			{
				float fW = (float)(pTex->Width());
				float fH = (float)(pTex->Height());

				fU_Left = pRCSrc->left / fW;
				fV_Top = pRCSrc->top / fH;
				fU_Right = pRCSrc->right / fW;
				fV_Bottom = pRCSrc->bottom / fH;
			}


			s_lpD3DDev->BeginScene();

			m_VPreview[0].Set((float)rcTex.left, (float)rcTex.top, 0.5f, 1.0f, 0xffffffff, fU_Left, fV_Top);
			m_VPreview[1].Set((float)rcTex.right, (float)rcTex.top, 0.5f, 1.0f, 0xffffffff, fU_Right, fV_Top);
			m_VPreview[2].Set((float)rcTex.right, (float)rcTex.bottom, 0.5f, 1.0f, 0xffffffff, fU_Right, fV_Bottom);
			m_VPreview[3] = m_VPreview[0];
			m_VPreview[4] = m_VPreview[2];
			m_VPreview[5].Set((float)rcTex.left, (float)rcTex.bottom, 0.5f, 1.0f, 0xffffffff, fU_Left, fV_Bottom);

			DWORD dwAlpha, dwFog, dwLight;
			s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
			s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
			s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);

			if(	pTex->PixelFormat() != D3DFMT_DXT1 && 
				pTex->PixelFormat() != D3DFMT_X8R8G8B8 )
			{
				hr = s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				hr = s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA);
				hr = s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA);
			}
			hr = s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
			hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

			hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			hr = s_lpD3DDev->SetTexture(0, pTex->Get());

			hr = s_lpD3DDev->SetFVF(FVF_TRANSFORMED);//s_lpD3DDev->SetVertexShader(FVF_TRANSFORMED);
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, &m_VPreview, sizeof(__VertexTransformed));

			hr = s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
			hr = s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
			hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
			
			hr = s_lpD3DDev->EndScene();
		}
		
		hr = s_lpD3DDev->Present(&rcPreview, &rcPreview, hWndDiffuse, nullptr);
	}
}


void CN3EngTool::GridCreate(int nWidth, int nHeight)
{
	if(nWidth <= 0 || nHeight <= 0) return;

	delete [] m_pVGrids;
	m_nGridLineCount = (nWidth+1) + (nHeight+1);
	m_pVGrids = new __VertexColor[m_nGridLineCount * 2];

	// ±×¸®µå »ý¼º..
	D3DCOLOR GridColor = 0xff808080;
	int xx = nWidth/2, zz = nHeight/2;
	int nSeq = 0;
	for(int x = -xx; x <= xx; x++)
	{
		m_pVGrids[nSeq++].Set((float)x, 0, (float) zz, GridColor);
		m_pVGrids[nSeq++].Set((float)x, 0, (float)-zz, GridColor);
	}
	for(int z = -zz; z <= zz; z++)
	{
		m_pVGrids[nSeq++].Set((float) xx, 0, (float)z, GridColor);
		m_pVGrids[nSeq++].Set((float)-xx, 0, (float)z, GridColor);
	}
}
