// N3Camera.cpp: implementation of the CN3Camera class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Camera.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CN3Camera::CN3Camera()
{
	m_dwType |= OBJ_CAMERA;

	m_Data.Release();
	m_Data.vEye = m_vPos = __Vector3(15,5,-15);
	m_Data.vAt = m_vAt = __Vector3(0,0,0);
	m_Data.vUp = m_vScale = __Vector3(0,1,0);

	m_Data.fFOV = D3DXToRadian(55.0f); // 기본값 55 도
	m_Data.fNP = 0.7f;
	m_Data.fFP = 512.0f;

	m_bFogUse = TRUE;
	m_FogColor = D3DCOLOR_ARGB(255,255,255,255);
//	m_fFogDensity = 1.0 / m_Data.fFP;
//	m_fFogStart = m_Data.fFP * 0.75f;
//	m_fFogEnd = m_Data.fFP;

	m_fRadianX = 0.0f;
	m_bOrtho = false;
}

CN3Camera::~CN3Camera()
{
}

void CN3Camera::Release()
{
	m_Data.Release();
	m_Data.vEye = m_vPos = __Vector3(15,5,-15);
	m_Data.vAt = m_vAt = __Vector3(0,0,0);
	m_Data.vUp = m_vScale = __Vector3(0,1,0);

	m_Data.fFOV = D3DXToRadian(55.0f); // 기본값 55 도
	m_Data.fNP = 0.7f;
	m_Data.fFP = 512.0f;

	m_bFogUse = FALSE;
	m_FogColor = D3DCOLOR_ARGB(255,255,255,255);
//	m_fFogDensity = 1.0f / m_Data.fFP;
//	m_fFogStart = m_Data.fFP * 0.75f;
//	m_fFogEnd = m_Data.fFP;

	m_fRadianX = 0.0f;

	CN3Transform::Release();
}

bool CN3Camera::Load(HANDLE hFile)
{
	CN3Transform::Load(hFile);

	DWORD dwRWC = 0;
	ReadFile(hFile, &m_vAt, sizeof(__Vector3), &dwRWC, NULL); // At position
	ReadFile(hFile, &m_Data, sizeof(__CameraData), &dwRWC, NULL); // CameraData
	ReadFile(hFile, &m_bFogUse, 4, &dwRWC, NULL);
	ReadFile(hFile, &m_FogColor, 4, &dwRWC, NULL);
//	ReadFile(hFile, &m_fFogDensity, 4, &dwRWC, NULL);
//	ReadFile(hFile, &m_fFogStart, 4, &dwRWC, NULL);
//	ReadFile(hFile, &m_fFogEnd, 4, &dwRWC, NULL);

	return true;
}

#ifdef _N3TOOL
BOOL CN3Camera::MoveByWindowMessage(MSG* pMsg)
{
//	static int iButtonDownCount = 0;
	if (pMsg->message == WM_MOUSEWHEEL)
	{
		//int16_t zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		int16_t zDelta = (int16_t)((pMsg->wParam>>16)&0x0000ffff);
		float fD = (m_vPos - m_vAt).Magnitude();
		float fzD = fD * zDelta * 0.001f;
		bool bSmall = false;
#ifdef _N3INDOOR
		if (0.0f < fzD && fzD < 0.6f)
		{
			fzD = 0.6f;
			bSmall = true;
		}
		if (0.0f > fzD && fzD > -0.6f)
		{
			fzD = -0.6f;
			bSmall = true;
		}
#endif 
		this->MoveStraight(fzD, bSmall);
		this->Apply();
		return TRUE;
	}
	else if (pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
		case VK_UP: { __Vector3 vDir = m_vAt - m_vPos; vDir.Normalize(); this->Move(vDir*10.0f); this->Apply(); return TRUE; }
			break;
		case VK_DOWN: { __Vector3 vDir = m_vAt - m_vPos; vDir.Normalize(); this->Move(vDir*-10.0f); this->Apply(); return TRUE; }
			break;
		case VK_RIGHT: { this->Rotate(0, -0.1f); this->Apply(); return TRUE; }
			break;
		case VK_LEFT: { this->Rotate(0, 0.1f); this->Apply(); return TRUE; }
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				static __Vector3 vEyes[10] = { m_vPos, m_vPos, m_vPos, m_vPos, m_vPos, m_vPos, m_vPos, m_vPos, m_vPos, m_vPos };
				static __Vector3 vAts[10] = { m_vAt, m_vAt, m_vAt, m_vAt, m_vAt, m_vAt, m_vAt, m_vAt, m_vAt, m_vAt};
				static __Vector3 vUps[10] = { m_vScale, m_vScale, m_vScale, m_vScale, m_vScale, m_vScale, m_vScale, m_vScale, m_vScale, m_vScale };
				
				int nCamIndex = pMsg->wParam - '0';

				if (::_IsKeyDown(VK_CONTROL))
				{
					vEyes[nCamIndex] = m_vPos;
					vAts[nCamIndex] = m_vAt;
					vUps[nCamIndex] = m_vScale;
					return FALSE;
				}
				else
				{
					this->EyePosSet(vEyes[nCamIndex]);
					this->AtPosSet(vAts[nCamIndex]);
					this->UpVectorSet(vUps[nCamIndex]);
					this->Apply();
					return TRUE;
				}
			}
			break;
		}
	}
	else if(::_IsKeyDown(VK_MENU))
	{
		static POINT ptPrev;
	
		POINT point = { (int16_t)LOWORD(pMsg->lParam), (int16_t)HIWORD(pMsg->lParam) };
		POINT ptDelta = { point.x - ptPrev.x, point.y - ptPrev.y };

		switch(pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
//				iButtonDownCount++;
//				::SetCapture(pMsg->hwnd);
				ptPrev.x = int16_t(LOWORD(pMsg->lParam));
				ptPrev.y = int16_t(HIWORD(pMsg->lParam));
				return TRUE;
			}
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			{
//				iButtonDownCount--;
//				if (iButtonDownCount <= 0) 
//					ReleaseCapture();
				return TRUE;
			}
			break;
		case WM_MOUSEMOVE:
			{
				uint32_t nFlags = pMsg->wParam;
				ptPrev = point;

				if(	(nFlags & MK_LBUTTON) && (nFlags & MK_MBUTTON) ) // Alt + LB + MB
				{
					float fZoom = (float)(ptDelta.x)/500.0f;
					this->Zoom(fZoom);
					this->Apply();
					return TRUE;
				}
				else if((nFlags & MK_LBUTTON) ) // Alt + LB
				{
					float fRX = (float)(ptDelta.y)/200.0f;
					float fRY = -(float)(ptDelta.x)/200.0f;
					this->LookAround(fRX, -fRY);
					this->Apply();
					return TRUE;
				}
				else if((nFlags & MK_RBUTTON) ) // Alt + RB
				{
					float fRX = (float)(ptDelta.y)/200.0f;
					float fRY = (float)(ptDelta.x)/200.0f;
					this->Rotate(fRX, -fRY);
					this->Apply();
					return TRUE;
				}
				else if((nFlags & MK_MBUTTON) ) // Alt + MB
				{
					__Vector3 vDelta = m_vPos - m_vAt;
					float fDelta = vDelta.Magnitude() * 0.002f / m_Data.fFOV;
					__Vector3 vD1;
					vD1.Set(-(float)(ptDelta.x), (float)(ptDelta.y), 0 );

					vD1 *= fDelta;

					this->MovePlane(vD1.x, -vD1.y);
					this->Apply();
					return TRUE;
				}
			}
		default:	// 마우스 메세지가 아닐경우 카메라 움직임이 아니다.
			return FALSE;
		}
	}

	return FALSE;
}
#endif // end of #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3Camera::Zoom(float fDelta)
{
	__Vector3 vD = m_vAt - m_vPos;

	float fD2 = sqrtf(vD.Magnitude());
	fDelta *= fD2 / 5.0f;

	__Vector3 vPos = m_vPos + vD * fDelta;
	float fDist = (vPos - m_Data.vAt).Magnitude();
#ifndef _N3INDOOR
	if(fDist < 0.3f) return; // 너무 가까우면 적용하지 않는다..
	else if(fDist > m_Data.fFP * 2.0f) return; // 너무 멀면 적용하지 않는다.
#endif

	m_vPos = vPos;
#ifdef _N3INDOOR
	if (fDist <6.6f)
		m_vAt = m_vAt + vD * fDelta;
#endif	
}
#endif // end of #ifdef _N3TOOL

void CN3Camera::Rotate(float fRadianX, float fRadianY)
{
	//static __Matrix44 mtx;
	__Matrix44 mtx;		//by lynus...
	mtx.RotationY(fRadianY);

	__Vector3 v1 = m_vAt - m_vPos, v2; // Rotation 은 LookAt Position 처럼, Scale 은 UpVector 처럼 쓴다..
	v1 *= mtx;
	m_vScale *= mtx;

	v2.Cross(v1, m_vScale);
	__Quaternion qt;
	qt.RotationAxis(v2, fRadianX);
	mtx = qt;

	v1 *= mtx;
	m_vScale *= mtx;	//by lynus...
/*
#ifndef _N3_CLIENT_
	m_vScale *= mtx;
#endif
*/
	m_vAt = m_vPos + v1;
}

#ifdef _N3TOOL
void CN3Camera::LookAround(float fRadianX, float fRadianY)		//At Postion을 중심으로 카메라가 돈다..고로 위치가 바뀐다..
{
	//static __Matrix44 mtx;
	//static __Vector3 v1, v2;
	//static __Quaternion qt;
	
	__Matrix44 mtx;		//by lynus...
	__Vector3 v1, v2;
	__Quaternion qt;

	v1 = m_vPos - m_vAt;
	mtx.RotationY(fRadianY);
	v1 *= mtx;
	m_vScale *= mtx;

	v2.Cross(v1, m_vScale);
	qt.RotationAxis(v2, fRadianX);
	mtx = qt;
	
	v1 *= mtx;
	m_vScale *= mtx;	//by lynus...
/*
#ifdef _N3_CLIENT_
	m_vScale *= mtx;
#endif
*/
	m_vPos = m_vAt + v1; // Rotation 은 LookAt Position 처럼, Scale 은 UpVector 처럼 쓴다..
}
#endif // end of #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3Camera::Move(const __Vector3& vDelta)
{
	m_vPos += vDelta;
	m_vAt += vDelta;
}
#endif // end of #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3Camera::MoveStraight(float fDistance, bool bSmall)
{
	__Vector3 vDelta = m_vAt - m_vPos;
	vDelta.Normalize();
	m_vPos += vDelta * fDistance;
#ifdef _N3INDOOR
	if (bSmall)
		m_vAt += vDelta * fDistance;
#endif 
}
#endif // end of #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3Camera::MovePlane(float fX, float fY)
{
	__Vector3 vDir = m_vAt - m_vPos; // Rotation 은 LookAt Position 처럼, Scale 은 UpVector 처럼 쓴다..
	vDir.Normalize();

	__Vector3 vHoriz;
	vHoriz.Cross(m_vScale, vDir);
	vHoriz.Normalize();
	
	__Vector3 vDown;
	vDown.Cross(vHoriz, vDir);
	vDown.Normalize();
	m_vScale = vDown*(-1);

	__Vector3 vMove = (vHoriz*fX) + (vDown*fY);
	
	m_vPos += vMove;
	m_vAt += vMove; // Rotation 은 LookAt Position 처럼, Scale 은 UpVector 처럼 쓴다..
}
#endif // end of #ifdef _N3TOOL

#ifdef _N3TOOL
bool CN3Camera::Save(HANDLE hFile)
{
	CN3Transform::Save(hFile);

	DWORD dwRWC = 0;
	WriteFile(hFile, &m_vAt, sizeof(__Vector3), &dwRWC, NULL); // At position
	WriteFile(hFile, &m_Data, sizeof(__CameraData), &dwRWC, NULL); // CameraData
	WriteFile(hFile, &m_bFogUse, 4, &dwRWC, NULL);
	WriteFile(hFile, &m_FogColor, 4, &dwRWC, NULL);
//	WriteFile(hFile, &m_fFogStart, 4, &dwRWC, NULL);
//	WriteFile(hFile, &m_fFogEnd, 4, &dwRWC, NULL);
//	WriteFile(hFile, &m_fFogDensity, 4, &dwRWC, NULL);

	return true;
}
#endif // end of #ifdef _N3TOOL

void CN3Camera::Apply()
{
	s_lpD3DDev->SetTransform(D3DTS_VIEW, &m_Data.mtxView);
	s_lpD3DDev->SetTransform(D3DTS_PROJECTION, &m_Data.mtxProjection); // Projection Matrix Setting
	memcpy(&(CN3Base::s_CameraData), &m_Data, sizeof(__CameraData)); // Static Data Update...

	// 안개 색깔 맞추기..
	s_lpD3DDev->SetRenderState( D3DRS_FOGENABLE, m_bFogUse);
	s_lpD3DDev->SetRenderState( D3DRS_FOGCOLOR,  m_FogColor);

	s_lpD3DDev->SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_EXP2);
	s_lpD3DDev->SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_EXP2);
//	s_lpD3DDev->SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_NONE);
//	s_lpD3DDev->SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR);

	s_lpD3DDev->SetRenderState( D3DRS_RANGEFOGENABLE, TRUE);

	// Range Fog : 장점 - 거리기반으로 fog가 적용된다. 단점 - poligon단위로 같은 fog값이 적용된다.(큰 폴리곤이 있을경우 어색한 fog가 될 수 있다.)
	// range fog = FALSE로 했을때는 depth버퍼 기반으로 fog가 계산되어 적용된다.

	// vertex fog 와 pixel fog(table fog)의 차이 - Dino씀..
	// vertex fog : vertex의 depth값을 기반으로 vertex사이의 보간으로 계산되는 것 같다.
	//				따라서 카메라를 돌리다보면 vertex주위를 중심으로 fog가 변하는것이 관찰된다.
	// pixel fog : pixel의 depth값을 기반으로 fog를 계산하는것 같다.

	// 위의 모든 fog의 차이를 보려면 큰판을 하나 그려서 fog를 넣어보면 쉽게 관찰할 수 있다.

//	s_lpD3DDev->SetRenderState( D3DRS_FOGSTART,   *(uint32_t*)&m_fFogStart);
//	s_lpD3DDev->SetRenderState( D3DRS_FOGEND,     *(uint32_t*)&m_fFogEnd);
//	s_lpD3DDev->SetRenderState( D3DRS_FOGDENSITY, *(uint32_t*)&m_fFogDensity);



	float fFogDensity = 1.0f / (0.37f*m_Data.fFP);//0.33f/23.0f;
	float fFogStart = m_Data.fFP * 0.75f;
	float fFogEnd = m_Data.fFP;

	s_lpD3DDev->SetRenderState( D3DRS_FOGSTART,   *(uint32_t*)&fFogStart);
	s_lpD3DDev->SetRenderState( D3DRS_FOGEND,     *(uint32_t*)&fFogEnd);
	s_lpD3DDev->SetRenderState( D3DRS_FOGDENSITY, *(uint32_t*)&fFogDensity);
}

void CN3Camera::Render(float fUnitSize)
{
}

void CN3Camera::Tick(float fFrm)
{
	CN3Transform::Tick(fFrm);

	////////////////////////////////////////////////////////////////////////
	// View Matrix 및 Projection Matrix Setting
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 	__Vector3 m_vEye, m_vUp ->> m_vPos, m_vScale 로 대신한다.. 중요!!
	m_Data.vEye = m_vPos;
	m_Data.vAt  = m_vAt;
	m_Data.vUp  = m_vScale; // Up Vector 처럼 쓴다.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

//	m_Data.fInverse_SineHalfOfFOV = 1.0f/sinf(m_Data.fFOV*0.5f);

	::D3DXMatrixLookAtLH(&m_Data.mtxView, &m_Data.vEye, &m_Data.vAt, &m_Data.vUp); // Look At 적용
	::D3DXMatrixInverse(&m_Data.mtxViewInverse, NULL, &m_Data.mtxView); // View Inverse 행렬 구하기..
	CN3Base::s_lpD3DDev->GetViewport(&m_Data.vp); // View port 가져오기...
	
	m_Data.fAspect = (float)m_Data.vp.Width / (float)m_Data.vp.Height; // 종횡비
	if(m_bOrtho)
	{
		float fL = (m_Data.vAt - m_Data.vEye).Magnitude() / 2.0f;
		::D3DXMatrixOrthoLH(&m_Data.mtxProjection, fL, fL / m_Data.fAspect, m_Data.fNP * (1.0f + fL / 1000.0f), m_Data.fFP);  // Projection Matrix Setting
//		::D3DXMatrixOrthoLH(&m_Data.mtxProjection, 2.0f, 2.0f, m_Data.fNP, m_Data.fFP);  // Projection Matrix Setting
	}
	else
	{
		::D3DXMatrixPerspectiveFovLH(&m_Data.mtxProjection, m_Data.fFOV, m_Data.fAspect, m_Data.fNP, m_Data.fFP); // Projection Matrix Setting
	}

	__Matrix44 mtx = m_Data.mtxView * m_Data.mtxProjection;
	float frustum [6][4];
	frustum[0][0] = mtx._14 - mtx._11;
	frustum[0][1] = mtx._24 - mtx._21;
	frustum[0][2] = mtx._34 - mtx._31;
	frustum[0][3] = mtx._44 - mtx._41;

	// Normalize the result 
	float t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
	frustum[0][0] /= t;
	frustum[0][1] /= t;
	frustum[0][2] /= t;
	frustum[0][3] /= t;

	// Extract the numbers for the LEFT plane 
	frustum[1][0] = mtx._14 + mtx._11;
	frustum[1][1] = mtx._24 + mtx._21;
	frustum[1][2] = mtx._34 + mtx._31;
	frustum[1][3] = mtx._44 + mtx._41;

	// Normalize the result 
	t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
	frustum[1][0] /= t;
	frustum[1][1] /= t;
	frustum[1][2] /= t;
	frustum[1][3] /= t;

	// Extract the BOTTOM plane 
	frustum[2][0] = mtx._14 + mtx._12;
	frustum[2][1] = mtx._24 + mtx._22;
	frustum[2][2] = mtx._34 + mtx._32;
	frustum[2][3] = mtx._44 + mtx._42;

	// Normalize the result 
	t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
	frustum[2][0] /= t;
	frustum[2][1] /= t;
	frustum[2][2] /= t;
	frustum[2][3] /= t;

	// Extract the TOP plane 
	frustum[3][0] = mtx._14 - mtx._12;
	frustum[3][1] = mtx._24 - mtx._22;
	frustum[3][2] = mtx._34 - mtx._32;
	frustum[3][3] = mtx._44 - mtx._42;

	// Normalize the result 
	t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
	frustum[3][0] /= t;
	frustum[3][1] /= t;
	frustum[3][2] /= t;
	frustum[3][3] /= t;

	// Extract the FAR plane
	frustum[4][0] = mtx._14 - mtx._13;
	frustum[4][1] = mtx._24 - mtx._23;
	frustum[4][2] = mtx._34 - mtx._33;
	frustum[4][3] = mtx._44 - mtx._43;

	// Normalize the result 
	t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
	frustum[4][0] /= t;
	frustum[4][1] /= t;
	frustum[4][2] /= t;
	frustum[4][3] /= t;

	// Extract the NEAR plane
	frustum[5][0] = mtx._14 + mtx._13;
	frustum[5][1] = mtx._24 + mtx._23;
	frustum[5][2] = mtx._34 + mtx._33;
	frustum[5][3] = mtx._44 + mtx._43;

	// Normalize the result 
	t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
	frustum[5][0] /= t;
	frustum[5][1] /= t;
	frustum[5][2] /= t;
	frustum[5][3] /= t;

	memcpy(m_Data.fFrustum, frustum, sizeof(float)*6*4);
}




void CN3Camera::LookAt(const __Vector3& vEye, const __Vector3& vAt, const __Vector3& vUp)
{
	m_vPos = vEye;
	m_vAt = vAt;
	m_vScale = vUp;
}
