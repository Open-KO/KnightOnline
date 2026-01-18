// N3EngTool.h: interface for the CN3EngTool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_D3DENGINETEST_H__4DE7DD27_A9BC_43C5_9D67_E99031ED38B5__INCLUDED_)
#define AFX_D3DENGINETEST_H__4DE7DD27_A9BC_43C5_9D67_E99031ED38B5__INCLUDED_

#pragma once

#include "N3Eng.h"

struct __EXPORT_OPTION
{
	char szID[8];                  // ID "N3Scene1"
	char szRemark[64];             // 설명..

	int nNodeCount;                // 전체 노드 카운트

	BOOL bAnimationKey;
	int nFrmStart;                 // 에니메이션 시작 프레임
	int nFrmEnd;                   // 에니메이션 끝 프레임
	float fSamplingRate;           // Key Sampling - 30.0f 가 표준..

	D3DCOLORVALUE dcvBackground;   // 배경 색
	D3DCOLORVALUE dcvAmbientLight; // 기본 조명 색

								   //	int nCameraCount;	// scene 내의 카메라 갯수
	//	int nMaterialCount;	// scene 내의 재질 갯수
	//	int nTextureCount;	// scene 내의 텍스처 갯수
	//	int nLightCount;	// scene 내의 조명 갯수

	BOOL bExportCamera;   // 카메라 데이터를 갖고 있다.
	BOOL bExportLight;    // 라이트 데이터를 갖고 있다.
	BOOL bExportGeometry; // 지오메트리 데이터를 갖고 있다.

	// 도우미 오브젝트(??? - 실제 겜에서는 필요없고 개발시에만 필요한 오브젝트를 말한다)데이터를 갖고 있다.
	BOOL bExportDummy;

	// 도우미 오브젝트(??? - 실제 겜에서는 필요없고 개발시에만 필요한 오브젝트를 말한다)데이터를 갖고 있다.
	BOOL bExportCharacter;

	BOOL bExportSelectedOnly;   // 선택된 것만 ???

	BOOL bGenerateFileName;     // 파일 이름을 0_0000_00_0 포맷으로 바꾼다..??
	BOOL bGenerateSmoothNormal; // 부드럽게 보이도록 법선 벡터들을 재 계산한다.

	//	BOOL bGenerateProgressiveMesh; // Progressive Mesh 생성

	// 텍스처 파일을 자동으로 최적화 시켜서 생성 Direct3D 의 포맷에 맞게 2의 제곱수 단위로 맞추어서 "OBM" 비트맵 파일로 저장.
	BOOL bGenerateHalfSizeTexture;
	BOOL bGenerateCompressedTexture; // Texture 압축 사용

	char szSubDir[_MAX_DIR];         // export 할때 저장하는 sub폴더를 지정해준다.
};

class CN3EngTool : public CN3Eng
{
public:
	int m_nGridLineCount;              // 그리드 라인 카운트..
	__VertexColor* m_pVGrids;          // 그리드 렌더링 용

	__VertexColor m_VAxis[60];         // 축 렌더링 용
	__VertexColor m_VDir[6];           // 방향 표시 용
	__VertexTransformed m_VPreview[6]; // 텍스처 프리뷰 용

									   //	LPDIRECT3DDEVICE8	m_lpD3DDevExtra;

public:
	//	bool CreateExtraDevice(int nWidth, int nHeight);
	void GridCreate(int nWidth, int nHeight);
	void RenderTexturePreview(CN3Texture* pTex, HWND hDlgWndDiffuse, RECT* pRCSrc = nullptr);
	void RenderGrid(const __Matrix44& mtxWorld);
	void RenderAxis(bool bShowDir = false);

	CN3EngTool();
	virtual ~CN3EngTool();
};

#endif // !defined(AFX_D3DENGINETEST_H__4DE7DD27_A9BC_43C5_9D67_E99031ED38B5__INCLUDED_)
