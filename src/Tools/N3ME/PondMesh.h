// PondMesh.h: interface for the CPondMesh class.
//
// 단지 가장큰 사각형의 영역과 땅의 지형이될 영역만 있음 (4m 씩의 점간의 거리)
// client에서 구현
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PONDMESH_H__314B2191_7B76_461C_AFD2_FDBC105914FC__INCLUDED_)
#define AFX_PONDMESH_H__314B2191_7B76_461C_AFD2_FDBC105914FC__INCLUDED_

#pragma once

#define MAX_PONDMESH_LINE   200
#define MAX_PONDMESH_VERTEX 200 * 4
//#define MAX_PONDMESH_INDEX		199*18
//#define INDEXBUFFER_PERLINE		18

#include <N3Base/N3BaseFileAccess.h>
#include <list>

class CLyTerrain;

class CPondMesh : public CN3BaseFileAccess
{
public:
	//	기존함수
	CN3Texture* TexGet() const
	{
		return m_pTexture;
	}
	int VertexCount() const
	{
		return m_iVC;
	}
	int RectVCount() const
	{
		return m_iRectVC;
	}
	int IndexCount() const
	{
		return m_iIC;
	}
	int LastVertexCount() const
	{
		return GetWaterScaleWidht();
	}

	void Rotation(__Matrix44 mRot, __Vector3 vCenter);

protected:
	//	선택된 점제어
	struct __SELECT_PO
	{
		int ix; //	m_iWaterScaleWidth 상의 숫자
		int iz; //	m_iWaterScaleHeight 상의 숫자
	};

	typedef std::list<__SELECT_PO*>::iterator it_SelVtx;
	std::list<__SELECT_PO*> m_vSelect; //	선택된 점들의 관리

	//	기본 데이타
	int m_iPondID;
	float m_fWaterHeight;                 //	물높이
	int m_iWaterScaleWidth;               //	가로방향의 점의 갯수
	int m_iWaterScaleHeight;              //	세로방향의 점의 갯수
	DWORD m_dwPondAlpha;                  //	물의 알파
	float m_fTU;
	float m_fTV;
	float m_fWaterScaleX, m_fWaterScaleZ; //	정렬했을시 점간의 거리

	int m_iBackUpWidht;
	int m_iBackUpHeight;

	__Vector3 m_pVertices[MAX_PONDMESH_VERTEX];    //	저장,백업용으로 쓰이는 값
	__VertexXyzT2 m_pViewVts[MAX_PONDMESH_VERTEX]; //	화면에 뿌려지는 값
	WORD* m_pdwIndex;                              //[MAX_PONDMESH_LINE*MAX_PONDMESH_LINE*6];
	int m_iIC;                                     // Index Buffer Count.
	int m_iVC;                                     // Vertex Count.
	CN3Texture* m_pTexture;

	BOOL m_bUVState;                               //	TRUE 점에 관여하여, FALSE 평편하게 (uv잡음)

	//	연못영역에 관한 데이타
	__Vector3 m_vDrawBox[4];        //	화면상에 나올 영역(왼쪽위부터 시계방향으로)
	__VertexXyzColor m_ViewRect[5]; //	화면에 그릴선
	int m_iRectVC;
	__VertexXyzT2 m_pRectVts[100];  //	영역줄에 그려질 점들

	__Vector3 m_vSelectBox[2];      //	점들을 선택했을시 점들의 영역(점들이 화면영역을 벋어났을시)

	//	지형정보 참고
	CLyTerrain* m_pTerrain;

public:
	CPondMesh();
	~CPondMesh() override;

	void Release() override;
	bool Load(File& file) override;
	virtual bool Load1000(File& file);
	virtual bool Load1001(File& file);
	bool Save(File& file) override;
	void Render();
	void RenderVertexPoint(); // 잘보이게 점만 다시 그리기

	__Vector3 GetCenter();
	float SettingDrawPos(
		__Vector3 vPos1, __Vector3 vPos2, __Vector3 vPos3, __Vector3 vPos4, int iLinePosNum);
	__VertexXyzT2* const GetVertex(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_iVC)
			return nullptr;
		return m_pViewVts + iIndex;
	}
	__VertexXyzT2* const GetRcVertex(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_iRectVC)
			return nullptr;
		return m_pRectVts + iIndex;
	}

	BOOL SetTextureName(LPCTSTR pszFName);

public:
	//	새로 추가한거나 고친거
	void SetTerrain(CLyTerrain* pCTerrain)
	{
		m_pTerrain = pCTerrain;
	}

	void MakeDrawRect(__Vector3* pvPos);      //	화면에 보일 연못의 영역테두리 만든다
	void UpdateDrawPos();                     //	영역줄에 그려질 점들

	void UpdateWaterHeight();                 //	현재의 물의 높이로 세팅
	void UpdateMovePos(__Vector3 vMovingPos); //	이동

	void MakePondPos();                       //	현재 박스에서 연못을 만든다
	void MakeIndex();
	void ReCalcUV();                          //	점에 상관없이 평편하게 뿌린다
	void ReCalcVexUV();                       //	점에 관여하여 뿌린다

	void EstimatePos();                       //	줄의 크기에 따라 점들의 갯수를 임의로 지정////

	void ClearSelectPos();                    //	선택점에 관한 데이터 초기화
	void InputSelectPos(float fX, float fY, float fZ, int iVC = -1); //	선택한 점들에 대한 자료입력
	BOOL InputDummyMovingPos(
		__Vector3 vDummyMovingPos, BOOL bMovePond); //	더미가 움직였을시 나머지점을 처리하기 위해

protected:
	void MovingPos();
	void CalcuWidth(int iSx, int iSy, int iEx = -1, int iEy = -1); //	처음 좌우로의 계산을 한다
	void SetAllPos(
		int iSx, int iSy, int iEx = -1, int iEy = -1); //	처음 계산한점들외에 점들 좌표 계산
	void ReInputBackPos();                             //	새로 옮긴좌표를 백업에 넣어줌

public:                                                //	다이얼로그 상자에서 쓰이는 함수
	void SetPondID(int iPondID)
	{
		m_iPondID = iPondID;
	}
	void SetWaterHeight(float fHeight);
	void SetWaterScaleWidht(int iWidhtNum)
	{
		m_iWaterScaleWidth = iWidhtNum;
		UpdateDrawPos();
		MakePondPos();
	}
	void SetWaterScaleHeight(int iHeightNum)
	{
		m_iWaterScaleHeight = iHeightNum;
		UpdateDrawPos();
		MakePondPos();
	}
	void SetAlphaFactor(DWORD dwAlpha)
	{
		m_dwPondAlpha = dwAlpha;
	}
	void SetTU(float fTU)
	{
		m_fTU = fTU;
		ReCalcUV();
	}
	void SetTV(float fTV)
	{
		m_fTV = fTV;
		ReCalcUV();
	}
	void SetLeft(float fLeft);
	void SetTop(float fTop);
	void SetRight(float fRight);
	void SetBottom(float fBottom);
	void SetChangUVState()
	{
		m_bUVState ^= 1;
	}

	int GetPondID() const
	{
		return m_iPondID;
	}
	float GetWaterHeight() const
	{
		return m_fWaterHeight;
	}
	int GetWaterScaleWidht() const
	{
		return m_iWaterScaleWidth;
	}
	int GetWaterScaleHeight() const
	{
		return m_iWaterScaleHeight;
	}
	DWORD GetAlphaFactor() const
	{
		return m_dwPondAlpha;
	}
	float GetTU() const
	{
		return m_fTU;
	}
	float GetTV() const
	{
		return m_fTV;
	}
	float GetLeft() const
	{
		return m_vDrawBox[0].x;
	}
	float GetTop() const
	{
		return m_vDrawBox[0].z;
	}
	float GetRight() const
	{
		return m_vDrawBox[2].x;
	}
	float GetBottom() const
	{
		return m_vDrawBox[2].z;
	}
	float GetWaterWidthLength() const
	{
		return m_fWaterScaleX;
	}
	float GetWaterHeightLength() const
	{
		return m_fWaterScaleZ;
	}
	BOOL GetChangUVState() const
	{
		return m_bUVState;
	}
};

#endif // !defined(AFX_PONDMESH_H__314B2191_7B76_461C_AFD2_FDBC105914FC__INCLUDED_)
