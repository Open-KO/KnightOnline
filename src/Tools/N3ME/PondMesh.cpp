// PondMesh.cpp: implementation of the CPondMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "PondMesh.h"
#include "LyTerrain.h"
#include <N3Base/N3Texture.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPondMesh::CPondMesh()
{
	m_pdwIndex = nullptr;
	m_pTexture = nullptr;

	Release();
}

CPondMesh::~CPondMesh()
{
	Release();
}

void CPondMesh::Release()
{
	m_iPondID = -1;
	m_fWaterHeight = 0.0f;	//	물높이
	m_iWaterScaleWidth = 4;	//	가로방향의 점의 갯수	
	m_iWaterScaleHeight = 6;//	세로방향의 점의 갯수	
	m_iBackUpWidht = 0;
	m_iBackUpHeight = 0;
	m_dwPondAlpha = 0x88ffffff;		//	물의 알파	
	m_fTU = 50.0f;	
	m_fTV = 50.0f;	
	
	m_bUVState = TRUE;
	m_pTerrain = nullptr;

	m_iRectVC = 0;
	ZeroMemory(m_pRectVts, 100*sizeof(__VertexXyzT2));
	
	m_iVC = 0;
	m_iIC = 0;
	m_fWaterScaleX = 1.0f,m_fWaterScaleZ=1.0f;

	ZeroMemory(m_pVertices, MAX_PONDMESH_VERTEX*sizeof(__Vector3));
	ZeroMemory(m_pViewVts, MAX_PONDMESH_VERTEX*sizeof(__VertexXyzT2));
	
	ClearSelectPos();

	if (m_pTexture) { s_MngTex.Delete(&m_pTexture); m_pTexture = nullptr;}
	if (m_pdwIndex) { delete []m_pdwIndex;m_pdwIndex = nullptr;}
}

void CPondMesh::Render()
{
	if (0 > m_iVC) return;

	__Matrix44 matWorld;	matWorld.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, matWorld.toD3D());

	//	영역 상자 그리기
	{
		s_lpD3DDev->SetTexture(0, nullptr);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		s_lpD3DDev->SetFVF(FVF_XYZCOLOR);

		s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, m_ViewRect, sizeof(__VertexXyzColor));
	}


	s_lpD3DDev->SetFVF(FVF_XYZT2);
	// Texture state
	if (m_pTexture && m_iVC>2  && m_iIC>0)
	{
		// backup state
		DWORD dwAlphaBlend, dwDestAlpha, dwSrcAlpha, dwTextureFactor;
		s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
		s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestAlpha);
		s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcAlpha);
		s_lpD3DDev->GetRenderState(D3DRS_TEXTUREFACTOR, &dwTextureFactor);
		DWORD dwAlphaOP, dwAlphaArg1;
		s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOP);
		s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);

		if ((m_dwPondAlpha & 0xff000000) != 0xff000000)	// alpha factor 설정하기
		{
			// render state 세팅
			s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			s_lpD3DDev->SetRenderState(D3DRS_TEXTUREFACTOR, m_dwPondAlpha);	// alpha factor 설정
			// texture state 세팅(alpha)
			s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			s_lpD3DDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		}

		// set texture state(color)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		s_lpD3DDev->SetTexture(0, m_pTexture->Get());
		s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		s_lpD3DDev->SetTexture(1, nullptr);
		

		// render
		s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST
										  , 0
										  , m_iVC
										  , m_iIC
										  , m_pdwIndex
										  , D3DFMT_INDEX16
										  , m_pViewVts//m_pVertices
										  , sizeof(__VertexXyzT2));

		// restore
		s_lpD3DDev->SetTexture(1, nullptr);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOP);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg1);
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestAlpha);
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcAlpha);
		s_lpD3DDev->SetRenderState(D3DRS_TEXTUREFACTOR, dwTextureFactor);
	}
	else
	{
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		s_lpD3DDev->SetTexture(0, nullptr);

		// backup state
		__Material BackupMtrl;
		s_lpD3DDev->GetMaterial(&BackupMtrl);

		// set material
		D3DCOLORVALUE color; color.a = 1.0f; color.r = color.g = 0.0f; color.b = 1.0f;
		__Material mtrl; mtrl.Init(color);
		s_lpD3DDev->SetMaterial(&mtrl);

		// render
		s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, m_iWaterScaleWidth, m_pViewVts, sizeof(__VertexXyzT2));

		// restore
		s_lpD3DDev->SetMaterial(&BackupMtrl);
	}
}

void CPondMesh::RenderVertexPoint()	// 잘보이게 점만 다시 그리기
{
	if (m_iVC == 0 && m_iRectVC==0) return;
	HRESULT hr;

	// set texture
	hr = s_lpD3DDev->SetTexture(0, nullptr);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	// transform
	__Matrix44 matView, matProj, matVP;
	s_lpD3DDev->GetTransform(D3DTS_VIEW, matView.toD3D());
	s_lpD3DDev->GetTransform(D3DTS_PROJECTION, matProj.toD3D());
	matVP = matView * matProj;
	D3DVIEWPORT9 vp = s_CameraData.vp;

	__VertexTransformedColor Vertices[4];
	D3DCOLOR clr = D3DCOLOR_ARGB(0xff, 0xff, 0x00, 0x00);
	s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);

	int i;
	__Vector4 v;
	//	화면상에 빨간점
	for (i=0; i<=m_iVC; ++i)
	{
		v.Transform(m_pViewVts[i], matVP);

		float fScreenZ = (v.z/v.w);
		if (fScreenZ>1.0 || fScreenZ<0.0) continue;

		int iScreenX = int(((v.x/v.w)+1.0f)*(vp.Width)/2.0f);
		int iScreenY = int((1.0f-(v.y/v.w))*(vp.Height)/2.0f);
		if (iScreenX >= (int)vp.X && iScreenX <= (int)vp.Width &&
			iScreenY >= (int)vp.Y && iScreenY <= (int)vp.Height)
		{
			// set X (점을 찍으면 1픽셀밖에 안찍으므로 X표시를 그린다.
			Vertices[0].Set(float(iScreenX-2), float(iScreenY-2), 0.5f, 0.5f, clr);
			Vertices[1].Set(float(iScreenX+2), float(iScreenY+2), 0.5f, 0.5f, clr);
			Vertices[2].Set(float(iScreenX+2), float(iScreenY-2), 0.5f, 0.5f, clr);
			Vertices[3].Set(float(iScreenX-2), float(iScreenY+2), 0.5f, 0.5f, clr);
			// render
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 2, Vertices, sizeof(__VertexTransformedColor));
		}
	}

	//	영역을 나타내는 점
	for(i=0;i<m_iRectVC;++i)
	{
		v.Transform(m_pRectVts[i], matVP);

		float fScreenZ = (v.z/v.w);
		if (fScreenZ>1.0 || fScreenZ<0.0) continue;

		int iScreenX = int(((v.x/v.w)+1.0f)*(vp.Width)/2.0f);
		int iScreenY = int((1.0f-(v.y/v.w))*(vp.Height)/2.0f);
		if (iScreenX >= (int)vp.X && iScreenX <= (int)vp.Width &&
			iScreenY >= (int)vp.Y && iScreenY <= (int)vp.Height)
		{
			// set X (점을 찍으면 1픽셀밖에 안찍으므로 X표시를 그린다.
			Vertices[0].Set(float(iScreenX-2), float(iScreenY-2), 0.5f, 0.5f, clr);
			Vertices[1].Set(float(iScreenX+2), float(iScreenY+2), 0.5f, 0.5f, clr);
			Vertices[2].Set(float(iScreenX+2), float(iScreenY-2), 0.5f, 0.5f, clr);
			Vertices[3].Set(float(iScreenX-2), float(iScreenY+2), 0.5f, 0.5f, clr);
			// render
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 2, Vertices, sizeof(__VertexTransformedColor));
		}
	}
}

void CPondMesh::SetWaterHeight(float fHeight)
{
	if(m_fWaterHeight == fHeight) return;
	m_fWaterHeight = fHeight;

	for(int i=0;i<5;++i)
		m_vDrawBox[i].y = fHeight;

	MakeDrawRect(m_vDrawBox);	//	외곽선 다시 만듬
	UpdateWaterHeight();	//	각점들을 지정한 높이로 올림
}

void CPondMesh::UpdateWaterHeight()
{
	for(int i=0;i<m_iVC;++i)
	{
		m_pVertices[i].y = m_fWaterHeight;
		m_pViewVts[i].y = m_fWaterHeight;
	}
//	ReCalcUV();
}

void CPondMesh::UpdateMovePos(__Vector3 vMovingPos)
{
	for(int i=0;i<m_iVC;++i)
	{
		m_pVertices[i] += vMovingPos;
//		m_pViewVts[i] += vMovingPos;
	}
}

void CPondMesh::MakeDrawRect(__Vector3* p4vPos)	//	화면에 보일 연못의 영역테두리 만든다
{
	if(p4vPos==nullptr) return;

	memcpy(m_vDrawBox,p4vPos,sizeof(__Vector3)*4);
	m_fWaterHeight = p4vPos[0].y;	//	물의 높이를 받는다
	p4vPos = nullptr;	//	메모리에서 이상한 결과과 나올까봐..

	//----------------------------------------------------------------------------
	DWORD color = 0xffffff00;
	m_ViewRect[0].Set(m_vDrawBox[0],color);	//	위점
	m_ViewRect[1].Set(m_vDrawBox[1],color);	//	위점
	m_ViewRect[2].Set(m_vDrawBox[2],color);	//	아래점
	m_ViewRect[3].Set(m_vDrawBox[3],color);	//	아래점
	m_ViewRect[4] = m_ViewRect[0];
	//----------------------------------------------------------------------------

	UpdateDrawPos();
}

void CPondMesh::UpdateDrawPos()
{
	m_iRectVC = 0;
	//	위,아래점들 세팅
	m_fWaterScaleX = SettingDrawPos(m_vDrawBox[0],m_vDrawBox[1],m_vDrawBox[3],m_vDrawBox[2],m_iWaterScaleWidth);
	//	좌우점들 세팅
	m_fWaterScaleZ = SettingDrawPos(m_vDrawBox[0],m_vDrawBox[3],m_vDrawBox[1],m_vDrawBox[2],m_iWaterScaleHeight);
}

float CPondMesh::SettingDrawPos(__Vector3 vPos1,__Vector3 vPos2,__Vector3 vPos3,__Vector3 vPos4,int iLinePosNum)
{
	if(iLinePosNum==0) return 0.0f;

	__Vector3 LinePos;
	float fLength;

	LinePos = vPos2-vPos1;
	fLength = LinePos.Magnitude(); fLength /= (iLinePosNum+1);	//	길이구하고,끝에넣을 점빼고 위치할 점들 계산
	LinePos.Normalize(); LinePos *= fLength;	//	길이1m인 백터를 구하고,점간의 거리를 입력

	//	입력시작
	for (int i=1;i<iLinePosNum+1;++i)
	{
		vPos2 = vPos1 + LinePos*(float)i;
		m_pRectVts[m_iRectVC+i-1].Set(vPos2, 0,0,0,0);

		vPos4 = vPos3 + LinePos*(float)i;
		m_pRectVts[m_iRectVC+i+iLinePosNum-1].Set(vPos4, 0,0,0,0);
	}
	m_iRectVC += iLinePosNum*2;

	return fLength;
}

void CPondMesh::MakePondPos()
{
	if(m_iWaterScaleWidth*m_iWaterScaleHeight<=0) return;


	if(m_pdwIndex == nullptr || (m_iBackUpWidht != m_iWaterScaleWidth || m_iBackUpHeight != m_iWaterScaleHeight)) 
	{
		delete []m_pdwIndex; 
		m_pdwIndex = new WORD [m_iWaterScaleWidth*m_iWaterScaleHeight*6];

		m_iBackUpWidht = m_iWaterScaleWidth;
		m_iBackUpHeight = m_iWaterScaleHeight;
	}

	ASSERT(MAX_PONDMESH_VERTEX>m_iWaterScaleWidth*m_iWaterScaleHeight);	//	앞으로 만들어질 점의숫자가 max에 달하는지 확인
	int ix,iz,itemp;

	//	방향의 점들의 위치를 받은뒤
	float *fX,fY,*fZ;
	fX = new float [m_iWaterScaleWidth];
	for(ix=0; ix<m_iWaterScaleWidth; ++ix) fX[ix] = m_pRectVts[ix].x;

	fZ = new float [m_iWaterScaleHeight];
	itemp = m_iWaterScaleWidth+m_iWaterScaleWidth;
	for(iz=0; iz<m_iWaterScaleHeight; ++iz) fZ[iz] = m_pRectVts[iz + itemp].z;

//	fY = m_pRectVts[0].y;
	fY = m_fWaterHeight;


	__Vector3 vPos;
	m_iVC = 0 , m_iIC = 0;	//	초기화
	//	일단 점들을 쫙 뿌린다
	for(iz=0; iz<m_iWaterScaleHeight; ++iz)
	{
		for(ix=0; ix<m_iWaterScaleWidth; ++ix)
		{
			vPos.Set(fX[ix],fY,fZ[iz]);
			m_pVertices[m_iVC] = vPos;
			m_pViewVts[m_iVC].Set(vPos,0,0,0,0);
			++m_iVC;
		}
	}

	m_iIC = (m_iWaterScaleWidth-1)*2*(m_iWaterScaleHeight-1);

	delete []fX;
	delete []fZ;

//	m_iLastVertexNum = m_iWaterScaleWidth;
	
	MakeIndex();
	ReCalcUV();
}

BOOL CPondMesh::SetTextureName(LPCTSTR pszFName)
{
	if (m_pTexture)
	{
		if (lstrcmpi(pszFName, m_pTexture->FileName().c_str()) == 0) return TRUE;
		s_MngTex.Delete(&m_pTexture);
	}
	m_pTexture = s_MngTex.Get(pszFName, TRUE);
	return m_pTexture ? TRUE:FALSE;
}

void CPondMesh::MakeIndex()
{
	if(m_pdwIndex==nullptr) 
	{
		m_pdwIndex = new WORD [m_iWaterScaleWidth*m_iWaterScaleHeight*6];
	}

	int j,k;
	int m = m_iWaterScaleWidth;	//	다음줄
	int x=0,y=m;
	WORD* indexPtr = m_pdwIndex;	//	삼각형을 부를 위치 설정

	--m;
	for (j=0; j<m_iWaterScaleHeight; j++)
	{
		for (k=0; k<m; k++)
		{
			indexPtr[0] = x;
			indexPtr[1] = x+1;
			indexPtr[2] = y;
			indexPtr[3] = y;
			indexPtr[4] = x+1;
			indexPtr[5] = y+1;

			indexPtr += 6;
			++x;
			++y;
		}
		++x;
		++y;
	}
}

void CPondMesh::ReCalcUV()
{
	if (m_iVC<2) return;

	const float fTu = m_fTU,fTv = m_fTV;
	int i,j;

	__Vector3* pVertices= m_pVertices;
	__VertexXyzT2* ptmpVertices = m_pViewVts;
	
	//	줄에 대한 변경(x,z에 대해)
	for (i=0;i<m_iWaterScaleHeight;++i)
	{	
		ptmpVertices->tu = ptmpVertices->x/fTu;
		ptmpVertices->tv = ptmpVertices->z/fTv;
		ptmpVertices->tu2 = ptmpVertices->tu;
		ptmpVertices->tv2 = ptmpVertices->tv;			

		for (j=0;j<m_iWaterScaleWidth;++j)
		{	
			(ptmpVertices+j)->tu = (ptmpVertices+j)->x/fTu;
			(ptmpVertices+j)->tv = (ptmpVertices+j)->z/fTv;
			(ptmpVertices+j)->tu2 = (ptmpVertices+j)->tu;
			(ptmpVertices+j)->tv2 = (ptmpVertices+j)->tv;			
		}
		ptmpVertices += m_iWaterScaleWidth;
		pVertices += m_iWaterScaleWidth;
	}
}

void CPondMesh::ReCalcVexUV()
{
	__Vector3		pBakVertices[MAX_PONDMESH_VERTEX];	//	저장,백업용으로 쓰이는 값
	__VertexXyzT2	pBakViewVts[MAX_PONDMESH_VERTEX];	//	화면에 뿌려지는 값

	int i;
	for(i=0;i<m_iVC;++i)
	{
		pBakVertices[i].x  = m_pVertices[i].x;
		pBakVertices[i].y  = m_pVertices[i].y;
		pBakVertices[i].z  = m_pVertices[i].z;

		pBakViewVts[i].x  = m_pViewVts[i].x;
		pBakViewVts[i].y  = m_pViewVts[i].y;
		pBakViewVts[i].z  = m_pViewVts[i].z;
	}
	
	MakePondPos();

	for(i=0;i<m_iVC;++i)
	{
		m_pVertices[i].x  = pBakVertices[i].x;
		m_pVertices[i].y  = pBakVertices[i].y;
		m_pVertices[i].z  = pBakVertices[i].z;

		m_pViewVts[i].x  = pBakViewVts[i].x;
		m_pViewVts[i].y  = pBakViewVts[i].y;
		m_pViewVts[i].z  = pBakViewVts[i].z;
	}
}

void CPondMesh::ClearSelectPos()
{
	memset(m_vSelectBox, 0, sizeof(__Vector3) * 2);

	for (__SELECT_PO* pSel : m_vSelect)
		delete pSel;
	m_vSelect.clear();
}

void CPondMesh::InputSelectPos(float fX, float fY, float fZ, int iVC)
{
	if (m_vSelectBox[0].x == 0.0f || m_vSelectBox[0].z == 0.0f)
	{
		m_vSelectBox[0].x = fX, m_vSelectBox[0].y = fY, m_vSelectBox[0].z = fZ;
		m_vSelectBox[1].x = fX, m_vSelectBox[1].y = fY, m_vSelectBox[1].z = fZ;
	}
	else
	{
		//	선택한 점들의 최대영역을 잡고
		if (m_vSelectBox[0].x > fX) m_vSelectBox[0].x = fX;
		if (m_vSelectBox[1].x < fX) m_vSelectBox[1].x = fX;
		if (m_vSelectBox[0].z > fZ) m_vSelectBox[0].z = fZ;
		if (m_vSelectBox[1].z < fZ) m_vSelectBox[1].z = fZ;
	}

	//	높이가 틀려지면 높이를 재 설정
	if (m_vSelectBox[0].y != fY)
	{
		SetWaterHeight(fY);
		m_vSelectBox[0].y = fY;
	}

	if (iVC > -1)	//	전체 선택이 아닌 일부선택시
	{
		int iHeight = iVC / m_iWaterScaleWidth;
		int iWidth = iVC % m_iWaterScaleWidth;

		for (__SELECT_PO* pSelpo : m_vSelect)
		{
			if (pSelpo != nullptr
				&& pSelpo->ix == iWidth
				&& pSelpo->iz == iHeight)
			{
				pSelpo = nullptr;
				return;
			}
		}

		//	선택한 점들을 임시로 가지고 있음
		__SELECT_PO* pSelpo = new __SELECT_PO;
		pSelpo->ix = iWidth;
		pSelpo->iz = iHeight;

		m_vSelect.push_back(pSelpo);
	}
}

BOOL CPondMesh::InputDummyMovingPos(__Vector3 vDummyMovingPos,BOOL bMovePond)
{
	//	움직여야할 이유가 없다
	if(vDummyMovingPos.x == 0.0f && vDummyMovingPos.y == 0.0f && vDummyMovingPos.z == 0.0f) return FALSE;
	BOOL bDrawBoxMove = FALSE;

	//	선택한 점들영역의 움직임
	m_vSelectBox[0] += vDummyMovingPos;
	m_vSelectBox[1] += vDummyMovingPos;

	if(vDummyMovingPos.y!=0)
		SetWaterHeight(m_vSelectBox[0].y);	//	높이를 체크

	//	연못 영역줄 처리
	if(bMovePond)
	{		
		for(int i=0;i<4;++i) m_vDrawBox[i] += vDummyMovingPos;
		MakeDrawRect(m_vDrawBox);	//	가장자리 점들 재 계산
		UpdateMovePos(vDummyMovingPos);	//	백업용 계산 점들 재 계산
	}
	else
	{
		if(m_vSelectBox[0].x < m_vDrawBox[2].x) { SetLeft(m_vSelectBox[0].x); bDrawBoxMove = TRUE;}
		else if(m_vSelectBox[1].x > m_vDrawBox[0].x) { SetRight(m_vSelectBox[1].x); bDrawBoxMove = TRUE;}

		if(m_vSelectBox[0].z < m_vDrawBox[0].z) { SetTop(m_vSelectBox[0].z); bDrawBoxMove = TRUE;}
		else if(m_vSelectBox[1].z > m_vDrawBox[2].z) { SetBottom(m_vSelectBox[1].z); bDrawBoxMove = TRUE;}

		if(vDummyMovingPos.x!=0 || vDummyMovingPos.z!=0)
			MovingPos();	//	더미의 움직임에 따라 점들 움직임
	}	

	return bDrawBoxMove;
}

void CPondMesh::SetLeft(float fLeft)		{m_vDrawBox[1].x = fLeft  ,m_vDrawBox[2].x = fLeft;  MakeDrawRect(m_vDrawBox);}
void CPondMesh::SetTop(float fTop)			{m_vDrawBox[0].z = fTop	  ,m_vDrawBox[1].z = fTop;   MakeDrawRect(m_vDrawBox);}
void CPondMesh::SetRight(float fRight)		{m_vDrawBox[0].x = fRight ,m_vDrawBox[3].x = fRight; MakeDrawRect(m_vDrawBox);}
void CPondMesh::SetBottom(float fBottom)	{m_vDrawBox[2].z = fBottom,m_vDrawBox[3].z = fBottom;MakeDrawRect(m_vDrawBox);}

//	기본적으로 선택된 점들은 이미 움직인 상태로 원본이라 할수있는 m_pVertices참조해서 움직일 예상점을 계산
void CPondMesh::MovingPos()
{
	for (__SELECT_PO* pSelpo : m_vSelect)
	{
		if (pSelpo != nullptr)
			CalcuWidth(pSelpo->ix, pSelpo->iz);
	}

	ReInputBackPos();
}

void CPondMesh::CalcuWidth(int iSx,int iSy,int iEx,int iEy)
{
	if(iSx<0 || iSx>m_iWaterScaleWidth || iSy<0 || iSy >m_iWaterScaleHeight) return;

	int iHeightNum = iSy*m_iWaterScaleWidth;
	//	----------------------------------------------------------------------------------
	//	계산해야할 첫점을 구한다
	__Vector3 vBakPick = *(m_pVertices + iHeightNum + iSx);	//	백업된 전의 좌표(참조하여 새로운 좌표 계산)
	__Vector3 vNowPick = *(m_pViewVts + iHeightNum + iSx);	//	현재 지정된 점(나중에 여러개 지정되게)
	__Vector3* pLRVertices = m_pVertices + iHeightNum;		//	계산할 첫 점(백업용)
	__VertexXyzT2* pLRViewVer = m_pViewVts + iHeightNum;	//	계산할 첫 점(현재용)
	//	----------------------------------------------------------------------------------

	//	----------------------------------------------------------------------------------
	// 위치에 해당하는양쪽 끝의점을 가진다
	__Vector3 *pvLeft,*pvRight,*pvTop,*pvBottom;
	pvRight = pLRVertices;	//	처음시작이 오른쪽부터이다
	pvLeft = pvRight + m_iWaterScaleWidth-1;
	pvTop = m_pVertices + iSx;
	pvBottom = m_pVertices + m_iVC - m_iWaterScaleWidth + 1;
	//	----------------------------------------------------------------------------------

	//	새로운 좌표 계산
	int iIntervalNum,j;
	float fx1,fx2,fnx1,fnx2;
	float fny1,fny2;
	float ftemp;
	
	//	----------------------------------------------------------------------------------
	iIntervalNum = iSx-1;	//	좌우 각 한점씩 빼기에
	if(iIntervalNum>0)
	{
		++pLRVertices, ++pLRViewVer;	//	오른쪽
		fny2 = vNowPick.z-vBakPick.z;
		for(j=0 ; j<iIntervalNum ; ++j,++pLRVertices,++pLRViewVer,++pvTop,++pvBottom)
		{
			// 사이의 점마다 거리비율을 구해 새로운 위치를 구한다
			fx2 = pvRight->x - vBakPick.x;	//	좌우의 비율구함
			if(fx2!=0)
			{
				fx1 = pvRight->x - pLRVertices->x;
				ftemp = fx1/fx2;

				//	오른끝점x-백업점x : 오른끝점x-현재계산하는점x =  오른끝점x-새로운점x : 구할점x
				fnx2 = pvRight->x - vNowPick.x;
				fnx1 = fnx2*ftemp;
				pLRViewVer->x = pvRight->x - fnx1;	//	x의 새로운점 입력

				//	오른끝점x-백업점x : 오른끝점x-현재계산하는점x =  새로운점z-백업점z : 구할변화량z
				fny1 = fny2*ftemp;
				pLRViewVer->z += fny1;				//	z의 변화량 더함
			}
		}
	}
	//	----------------------------------------------------------------------------------

	//	----------------------------------------------------------------------------------
	iIntervalNum = m_iWaterScaleWidth - iSx-2;	//	좌우 각 한점씩 빼기에
	if(iIntervalNum>0)
	{
		++pLRVertices, ++pLRViewVer;	//	왼쪽
		fny2 = vNowPick.z-vBakPick.z;
		for(j=0 ; j<iIntervalNum ; ++j,++pLRVertices,++pLRViewVer,++pvTop,++pvBottom)
		{
			fx2 = vBakPick.x - pvLeft->x;
			if(fx2!=0)
			{
				fx1 = pLRVertices->x - pvLeft->x; 
				ftemp = fx1/fx2;

				fnx2 = vNowPick.x - pvLeft->x;
				fnx1 = fnx2*ftemp;
				pLRViewVer->x = pvLeft->x + fnx1;		//	x의 새로운점 입력

				fny1 = fny2*ftemp;
				pLRViewVer->z += fny1;					//	z의 변화량 더함
			}
		}
	}
	//	----------------------------------------------------------------------------------

	for(j=0;j<m_iWaterScaleWidth;++j)
		SetAllPos(j,iSy,iEx,iEy);
}

void CPondMesh::SetAllPos(int iSx,int iSy,int iEx,int iEy)
{
	if(iSx<0 || iSx>m_iWaterScaleWidth || iSy<0 || iSy >m_iWaterScaleHeight) return;

	//	계산해야할 첫점을 구한다
	__Vector3* pTBVertices = m_pVertices + iSx;
	__VertexXyzT2* pTBViewVer = m_pViewVts + iSx;

	// 위치에 해당하는양쪽 끝의점을 가진다
	__Vector3 vTop,vBottom,vCenter,vNowCenter;
	vTop = *pTBVertices;	
	vBottom = *(m_pVertices + m_iVC - m_iWaterScaleWidth + iSx);
	vCenter = *(m_pVertices + m_iWaterScaleWidth*iSy + iSx);	//	현재 지정된 점(나중에 여러개 지정되게)
	vNowCenter = *(m_pViewVts + m_iWaterScaleWidth*iSy + iSx);	//	백업된 전의 좌표(참조하여 새로운 좌표 계산)

	//	새로운 좌표 계산
	int iIntervalNum,j;
	float fy1,fy2,fny1,fny2;
	float fnx1,fnx2;
	float ftemp;

	iIntervalNum = iSy-1;
	if(iIntervalNum>0)
	{
		fy2 = vCenter.z - vTop.z;
		if(fy2!=0)
		{
			pTBVertices += m_iWaterScaleWidth, pTBViewVer += m_iWaterScaleWidth;
			fnx2 = vNowCenter.x - vCenter.x;
			fny2 = vTop.z - vNowCenter.z;
			for(j=0 ; j<iIntervalNum ; ++j,pTBVertices+=m_iWaterScaleWidth,pTBViewVer+=m_iWaterScaleWidth)
			{
				fy1 = pTBVertices->z - vTop.z; 	//	좌우의 비율구함
				ftemp = fy1/fy2;

				fnx1 = fnx2*ftemp;
				pTBViewVer->x += fnx1;			//	x의 변화량 더함

				fny1 = fny2*ftemp;
				pTBViewVer->z = vTop.z - fny1;	//	z의 새로운점 입력
			}
		}
	}

	iIntervalNum = m_iWaterScaleHeight - iSy-2;
	if(iIntervalNum>0)
	{
		fy2 = vBottom.z - vCenter.z;
		if(fy2!=0)
		{
			pTBVertices += m_iWaterScaleWidth, pTBViewVer += m_iWaterScaleWidth;
			fnx2 = vNowCenter.x - vCenter.x;
			fny2 = vNowCenter.z - vBottom.z;
			for(j=0 ; j<iIntervalNum ; ++j,pTBVertices+=m_iWaterScaleWidth,pTBViewVer+=m_iWaterScaleWidth)
			{
				fy1 = vBottom.z - pTBVertices->z; 
				ftemp = fy1/fy2;

				fnx1 = fnx2*ftemp;
				pTBViewVer->x += fnx1;				//	x의 변화량 더함

				fny1 = fny2*ftemp;
				pTBViewVer->z = vBottom.z + fny1;	//	z의 새로운점 입력
			}

		}
	}
}

void CPondMesh::ReInputBackPos()
{
	for(int i=0;i<m_iVC;++i)
	{
		m_pVertices[i].x  = m_pViewVts[i].x;
		m_pVertices[i].y  = m_pViewVts[i].y;
		m_pVertices[i].z  = m_pViewVts[i].z;
	}
}

__Vector3 CPondMesh::GetCenter()
{
	__Vector3 vCenter;
	vCenter = m_vDrawBox[2]-m_vDrawBox[0];
	float fLength = vCenter.Magnitude();
	fLength /= 2;
	vCenter.Normalize();
	vCenter = m_vDrawBox[0] + vCenter*fLength;
	return vCenter;
}

bool CPondMesh::Load1001(File& file)
{
	Release();

	int iLen;
	char szTextueFName[_MAX_PATH];

	file.Read(&m_iPondID, sizeof(m_iPondID));		// 연못 번호
	file.Read(&m_dwPondAlpha, sizeof(m_dwPondAlpha));		// 연못 알파
	file.Read(&m_fWaterHeight, sizeof(m_fWaterHeight));
	file.Read(&m_iWaterScaleWidth, sizeof(m_iWaterScaleWidth));
	file.Read(&m_iWaterScaleHeight, sizeof(m_iWaterScaleHeight));
	file.Read(&m_fTU, sizeof(m_fTU));
	file.Read(&m_fTV, sizeof(m_fTV));
	file.Read(&m_fWaterScaleX, sizeof(m_fWaterScaleX));
	file.Read(&m_fWaterScaleZ, sizeof(m_fWaterScaleZ));
	file.Read(&m_bUVState, sizeof(m_bUVState));

	file.Read(m_vDrawBox, sizeof(m_vDrawBox));				// 한줄에 있는 점 갯수

	file.Read(&m_iVC, sizeof(m_iVC));				// 점 갯수
	if (m_iVC > 0)
	{
		file.Read(m_pViewVts, m_iVC * sizeof(__VertexXyzT2));	// vertex buffer
		ReInputBackPos();	//	백업용에 새로좌표입력
	}
	file.Read(&m_iIC, sizeof(m_iIC));				// IndexBuffer Count.

	file.Read(&iLen, sizeof(iLen));				// texture file name length
	if (iLen > 0)
	{
		file.Read(szTextueFName, iLen);			// texture name
		szTextueFName[iLen] = '\0';
		m_pTexture = s_MngTex.Get(szTextueFName, TRUE);				// load texture
	}

	MakeIndex();	//	인덱스를 다시 계산
	MakeDrawRect(m_vDrawBox);	//	영역라인 설정

	m_iBackUpWidht = m_iWaterScaleWidth;
	m_iBackUpHeight = m_iWaterScaleHeight;

	return 0;
}

bool CPondMesh::Load1000(File& file)
{
	Release();

	int iLen;
	char szTextueFName[_MAX_PATH];
	float fScaleTemp;

	file.Read(&m_iPondID, sizeof(m_iPondID));			// 연못 번호
	file.Read(&m_dwPondAlpha, sizeof(m_dwPondAlpha));	// 연못 알파

	file.Read(&m_iWaterScaleWidth, sizeof(m_iWaterScaleWidth));	// 한줄에 있는 점 갯수

	file.Read(&fScaleTemp, sizeof(fScaleTemp));
	file.Read(&fScaleTemp, sizeof(fScaleTemp));

	file.Read(&m_iVC, sizeof(m_iVC));			// 점 갯수
	if (m_iVC > 0)
		file.Read(m_pVertices, m_iVC * sizeof(__Vector3));	// vertex buffer
	file.Read(&m_iIC, sizeof(m_iIC));			// IndexBufferCount.
	file.Read(&iLen, sizeof(iLen));				// texture name length
	if (iLen > 0)
	{
		file.Read(szTextueFName, iLen);			// texture name
		szTextueFName[iLen] = '\0';
		m_pTexture = s_MngTex.Get(szTextueFName, TRUE);				// load texture
	}

	//	---------------------------------------------------------------------------------------------------------
	m_fWaterHeight = m_pVertices[0].y;
	m_iWaterScaleHeight = m_iVC / m_iWaterScaleWidth;
	m_iIC = (m_iWaterScaleWidth - 1) * 2 * (m_iWaterScaleHeight - 1);
	m_fTU = 50.0f;
	m_fTV = 50.0f;

	MakeIndex();
	ReCalcUV();
	//	---------------------------------------------------------------------------------------------------------

	//	---------------------------------------------------------------------------------------------------------
	int i;
	for (i = 0; i < 4; ++i) m_vDrawBox[i] = m_pVertices[0];
	for (i = 0; i < m_iVC; ++i)
	{
		m_pViewVts[i].x = m_pVertices[i].x, m_pViewVts[i].y = m_pVertices[i].y, m_pViewVts[i].z = m_pVertices[i].z;

		if (m_vDrawBox[0].x < m_pVertices[i].x) m_vDrawBox[0].x = m_pVertices[i].x, m_vDrawBox[3].x = m_pVertices[i].x;
		if (m_vDrawBox[1].x > m_pVertices[i].x) m_vDrawBox[1].x = m_pVertices[i].x, m_vDrawBox[2].x = m_pVertices[i].x;
		if (m_vDrawBox[0].z > m_pVertices[i].z) m_vDrawBox[0].z = m_pVertices[i].z, m_vDrawBox[3].z = m_pVertices[i].z;
		if (m_vDrawBox[1].z < m_pVertices[i].z) m_vDrawBox[1].z = m_pVertices[i].z, m_vDrawBox[2].z = m_pVertices[i].z;
	}
	m_vDrawBox[0].x += 1.0f, m_vDrawBox[3].x += 1.0f, m_vDrawBox[1].x -= 1.0f, m_vDrawBox[2].x -= 1.0f;
	m_vDrawBox[0].z -= 1.0f, m_vDrawBox[3].z -= 1.0f, m_vDrawBox[1].z += 1.0f, m_vDrawBox[2].z += 1.0f;

	MakeDrawRect(m_vDrawBox);	//	외곽선 다시 만듬
	//	---------------------------------------------------------------------------------------------------------

	return 0;
}

bool CPondMesh::Load(File& file)
{
	Release();

	int iLen;
	char szTextueFName[_MAX_PATH];
	float fScaleTemp;

	file.Read(&m_iPondID, sizeof(m_iPondID));		// 연못 번호

	m_dwPondAlpha = 0xddffffff;

	file.Read(&m_iWaterScaleWidth, sizeof(int));				// 한줄에 있는 점 갯수

	file.Read(&fScaleTemp, sizeof(fScaleTemp));
	file.Read(&fScaleTemp, sizeof(fScaleTemp));

	file.Read(&m_iVC, sizeof(m_iVC));			// 점 갯수
	if (m_iVC > 0)
	{
		file.Read(m_pViewVts, m_iVC * sizeof(__VertexXyzT2));	// vertex buffer
		ReInputBackPos();	//	백업용에 새로좌표입력
	}
	file.Read(&m_iIC, sizeof(m_iIC));			// IndexBufferCount.
	file.Read(&iLen, sizeof(iLen));				// texture name length
	if (iLen > 0)
	{
		file.Read(szTextueFName, iLen);	// texture name
		szTextueFName[iLen] = '\0';
		m_pTexture = s_MngTex.Get(szTextueFName, TRUE);				// load texture
	}

	//	---------------------------------------------------------------------------------------------------------
	m_fWaterHeight = m_pVertices[0].y;
	m_iWaterScaleHeight = m_iVC / m_iWaterScaleWidth;
	m_iIC = (m_iWaterScaleWidth - 1) * 2 * (m_iWaterScaleHeight - 1);
	m_fTU = 50.0f;
	m_fTV = 50.0f;

	MakeIndex();
	//	---------------------------------------------------------------------------------------------------------

	//	---------------------------------------------------------------------------------------------------------
	int i;
	for (i = 0; i < 4; ++i) m_vDrawBox[i] = m_pVertices[0];
	for (i = 0; i < m_iVC; ++i)
	{
		if (m_vDrawBox[0].x < m_pVertices[i].x) m_vDrawBox[0].x = m_pVertices[i].x, m_vDrawBox[3].x = m_pVertices[i].x;
		if (m_vDrawBox[1].x > m_pVertices[i].x) m_vDrawBox[1].x = m_pVertices[i].x, m_vDrawBox[2].x = m_pVertices[i].x;
		if (m_vDrawBox[0].z > m_pVertices[i].z) m_vDrawBox[0].z = m_pVertices[i].z, m_vDrawBox[3].z = m_pVertices[i].z;
		if (m_vDrawBox[1].z < m_pVertices[i].z) m_vDrawBox[1].z = m_pVertices[i].z, m_vDrawBox[2].z = m_pVertices[i].z;
	}
	m_vDrawBox[0].x += 1.0f, m_vDrawBox[3].x += 1.0f, m_vDrawBox[1].x -= 1.0f, m_vDrawBox[2].x -= 1.0f;
	m_vDrawBox[0].z -= 1.0f, m_vDrawBox[3].z -= 1.0f, m_vDrawBox[1].z += 1.0f, m_vDrawBox[2].z += 1.0f;

	MakeDrawRect(m_vDrawBox);	//	외곽선 다시 만듬
	//	---------------------------------------------------------------------------------------------------------

	return 0;
}

bool CPondMesh::Save(File& file)
{
	file.Write(&m_iPondID, sizeof(m_iPondID));		// 연못 번호
	file.Write(&m_dwPondAlpha, sizeof(m_dwPondAlpha));		// 연못 알파
	file.Write(&m_fWaterHeight, sizeof(m_fWaterHeight));
	file.Write(&m_iWaterScaleWidth, sizeof(m_iWaterScaleWidth));
	file.Write(&m_iWaterScaleHeight, sizeof(m_iWaterScaleHeight));
	file.Write(&m_fTU, sizeof(m_fTU));
	file.Write(&m_fTV, sizeof(m_fTV));
	file.Write(&m_fWaterScaleX, sizeof(m_fWaterScaleX));
	file.Write(&m_fWaterScaleZ, sizeof(m_fWaterScaleZ));
	file.Write(&m_bUVState, sizeof(m_bUVState));

	file.Write(m_vDrawBox, sizeof(m_vDrawBox));				// 한줄에 있는 점 갯수

	file.Write(&m_iVC, sizeof(m_iVC));						// 점 갯수
	if (m_iVC > 0)
		file.Write(m_pViewVts, m_iVC * sizeof(__VertexXyzT2));	// vertex buffer
	file.Write(&m_iIC, sizeof(m_iIC));						// IndexBuffer Count.

	int iLen = 0;
	if (m_pTexture != nullptr)
		iLen = static_cast<int>(m_pTexture->FileName().size());

	file.Write(&iLen, sizeof(iLen));							// texture file name length
	if (iLen > 0)
		file.Write(m_pTexture->FileName().c_str(), iLen);	// texture file name

	return 0;
}
//	==================================================================================

void CPondMesh::Rotation(__Matrix44 mRot,__Vector3 vCenter)
{
	//	백업(되돌아감)을 할 수있다.
//	if(m_bThisBackup==FALSE) m_bThisBackup = TRUE;

	//	중간값을 중심으로 회전시킨다
	int nCount;
	__VertexXyzT2* pViewVts = m_pViewVts;

	__Vector3 vPos,vStPos;
//	vStPos = pViewVts[(m_iVC+m_iLastVertexNum)/2].v;	//	현재 연못의 중간 위치구함
	vStPos = vCenter;
	for(nCount = 0 ;nCount<m_iVC ; nCount++)
	{
		vPos = *pViewVts;
		vPos -= vStPos;
		vPos *= mRot;
		vPos += vStPos;
		*pViewVts = vPos;
		pViewVts++;
	}
}
/*
void CPondMesh::SetVtx()
{
	if(m_bThisBackup==FALSE) return;

	__Vector3* pVtx = m_pVertices;
	__VertexXyzT2* pViewVtx = m_pViewVts;

	float tempX = m_fWaterScaleX;
	float tempZ = m_fWaterScaleZ;

	int i,j;
	int iNumVertex = m_iVC/m_iWaterScaleWidth;

	//	 처음 한 줄에 대해 x방향만 변함
	pVtx->Set(pViewVtx->x,pViewVtx->y,pViewVtx->z);

	for (j=1;j<m_iWaterScaleWidth;j++)
	{	
		(pVtx+j)->x = (pVtx+j-1)->x + ((pViewVtx+j)->x-(pViewVtx+j-1)->x) / tempX;
		(pVtx+j)->y = (pViewVtx+j)->y;
		(pVtx+j)->z = (pViewVtx+j)->z;
	}
	pVtx += m_iWaterScaleWidth;
	pViewVtx += m_iWaterScaleWidth;
	
	//	줄에 대한 x,z에 대한 변환
	for(i = 1 ;i<iNumVertex ; i++)
	{
		pVtx->x = (pVtx-m_iWaterScaleWidth)->x + (pViewVtx->x-(pViewVtx-m_iWaterScaleWidth)->x) / tempX;
		pVtx->y = pViewVtx->y;
		pVtx->z = (pVtx-m_iWaterScaleWidth)->z + (pViewVtx->z-(pViewVtx-m_iWaterScaleWidth)->z) / tempZ;

		for (j=1;j<m_iWaterScaleWidth;j++)
		{	
			(pVtx+j)->x = (pVtx+j-1)->x + ((pViewVtx+j)->x-(pViewVtx+j-1)->x) / tempX;
			(pVtx+j)->y = (pViewVtx+j)->y;
			(pVtx+j)->z = (pVtx+j-m_iWaterScaleWidth)->z + ((pViewVtx+j)->z-(pViewVtx+j-m_iWaterScaleWidth)->z) / tempZ;
		}
		pVtx += m_iLastVertexNum;
		pViewVtx += m_iLastVertexNum;
	}


	//	재 조정(화면에 정확히 나오게 하기 위해)
	ReCalcUV();

	m_bThisFixPosion = TRUE;
	m_bThisBackup = FALSE;
}

void CPondMesh::ReSetVtx()
{
	if(m_bThisBackup==FALSE) return;

	m_fWaterScaleX=m_fBakScaleX , m_fWaterScaleZ=m_fBakScaleZ;
	ReCalcUV();

	m_bThisFixPosion = TRUE;
	m_bThisBackup = FALSE;
}


__Vector3 CPondMesh::GetRightVct()
{
	__Vector3 reVet = m_pViewVts[1];
	reVet -= m_pViewVts[0];

	reVet.Normalize();

	reVet.x = reVet.x<0 ? -reVet.x:reVet.x;
	return reVet;
}

__Vector3 CPondMesh::GetDownVct()
{
	__Vector3 reVet = m_pViewVts[m_iLastVertexNum];
	reVet -= m_pViewVts[0];

	reVet.Normalize();
	reVet.z = reVet.z<0 ? reVet.z:-reVet.z;
	return reVet;
}
*/
/*
int CPondMesh::AddVertex(__Vector3* pvPos)
{
//	ASSERT(MAX_PONDMESH_VERTEX>m_iVC+2 && (m_iVC%2) == 0);
//	ASSERT(m_iVC<2);
	ASSERT(MAX_PONDMESH_VERTEX>m_iVC+m_iWaterScaleWidth*m_iWaterScaleHeight);	//	앞으로 만들어질 점의숫자가 max에 달하는지 확인

	//	첫 시작점과 두번째점으로 4m씩 끊어 넣음
	m_pVertices[m_iVC] = vPos1;
	m_pViewVts[m_iVC].Set(vPos1,0,0,0,0);

	__Vector3 LinePos = vPos2-vPos1;
	float LineLength = LinePos.Magnitude();
	int LinePosNum = (int)(LineLength/4.0f);
	if(LineLength > (float)LinePosNum*4.0f) LinePosNum++;
	ASSERT(MAX_PONDMESH_VERTEX>m_iVC+LinePosNum);

	LinePos.Normalize();
	LinePos*=4.0f;
	for(int i=1;i<LinePosNum+1;i++)
	{
		vPos2 = vPos1 + LinePos*((float)i);
		m_pVertices[m_iVC+i] = vPos2;

		vPos2 = vPos1 + LinePos*m_fWaterScaleX*((float)i);
		m_pViewVts[m_iVC+i].Set(vPos2, 0,0,0,0);
	}
	m_iLastVertexNum=LinePosNum+1;
	m_iVC += m_iLastVertexNum;

	MakeIndex();
	ReCalcUV();
	
	return m_iVC;
}

int CPondMesh::AddVertex()
{
	ASSERT(MAX_PONDMESH_VERTEX>m_iVC+m_iLastVertexNum);
	if (MAX_PONDMESH_VERTEX<=m_iVC+m_iLastVertexNum) return m_iVC;
	ASSERT(m_iVC>=m_iLastVertexNum);

	if (m_iVC<m_iLastVertexNum)
	{
		m_pVertices[m_iVC+0].Set(0,0,0);
		m_pVertices[m_iVC+1].Set(0,0,1);
		
		m_pViewVts[m_iVC+0].Set(0, 0, 0, 0, 0, 0, 0);
		m_pViewVts[m_iVC+1].Set(0, 0, 1, 0, 0, 0, 0);
		return m_iVC;
	}

	// 기존 마지막 두점과 직각인 방향 구하기(4m의 거리로 찍인다)
	__Vector3 v1, v2, v3, vDir, vDiff, vScaleDiff;
	v1 = m_pVertices[m_iVC-2];	v2 = m_pVertices[m_iVC-1];

	v3 = v2 - v1;
	__Matrix44 mat;	mat.RotationY(DegreesToRadians(-90.0f));
	vDir = v3*mat;	vDir.Normalize();
	vDiff = vDir*4.0f;
	vScaleDiff = vDiff;
	vScaleDiff.x *= m_fWaterScaleX;
	vScaleDiff.z *= m_fWaterScaleZ;

	for(int i=0;i<m_iLastVertexNum;i++)
	{
		vDir = m_pVertices[m_iVC-m_iLastVertexNum+i];
		m_pVertices[m_iVC+i] = vDir - vDiff;

		vDir = m_pViewVts[m_iVC-m_iLastVertexNum+i];
		m_pViewVts[m_iVC+i] = vDir - vScaleDiff;
	}
	m_iVC += m_iLastVertexNum;

	// IndexBuffer Setting.	
	m_iIC += (m_iLastVertexNum-1)*2;
	ReCalcUV();

	return m_iVC;
}

int CPondMesh::DeleteVertex(int iIndex)
{
	if (iIndex<0 || iIndex>=m_iVC) return m_iVC;
	int i, iStart;
	iStart = (iIndex/m_iLastVertexNum);

	// Vertext Buffer delete
	for (i=iStart*m_iLastVertexNum; i<m_iVC-m_iLastVertexNum; ++i)
	{
		m_pVertices[i] = m_pVertices[i+m_iLastVertexNum];
		m_pViewVts[i] = m_pViewVts[i+m_iLastVertexNum];
	}
	m_iVC -= m_iLastVertexNum;

	// need not indexing Index Buffer again. ^^
	m_iIC -= (m_iLastVertexNum-1)*2;

	return m_iVC;
}
*/
