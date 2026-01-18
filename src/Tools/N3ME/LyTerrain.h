// LyTerrain.h: interface for the CLyTerrain class.
//	- 2001. 9.24.~ Map Editor용으로 변환...
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TERRAIN_H__7BCDE4B8_8FBC_432E_B4CB_4A56C5FC29F0__INCLUDED_)
#define AFX_N3TERRAIN_H__7BCDE4B8_8FBC_432E_B4CB_4A56C5FC29F0__INCLUDED_

#pragma once

#include <map>
#include <list>

#include "LyTerrainDef.h"
#include <N3Base/N3BaseFileAccess.h>

class CN3Texture;
class CQTNode;

class CDTexMng;
class CDTexGroup;
class CDTexGroupMng;
class CDlgSetLightMap;

////////////////////////////////////////////////
//	multimap사용
//savegamedata할때 쓸꺼얌..
//colormap 찍을때도 써..^^
//
typedef std::multimap<short, short> MMap;
typedef MMap::value_type MMValue;
typedef MMap::iterator MMIter;

typedef std::list<short> LList;
typedef LList::iterator LLIter;

typedef std::list<CQTNode*>::iterator it_QTNode;

class CLyTerrain : public CN3BaseFileAccess
{
protected:
	__Material m_Material;
	D3DFILLMODE m_FillMode;

	//quadtree..
	CQTNode* m_pRoot;
	CQTNode* m_pRefCurrNode;

	//지형 edit관련...
	__VertexXyzColor m_vBrushVertices[MAX_BRUSH_SIZE * MAX_BRUSH_SIZE
									  * 9]; // Brush의 모양을 나타내는 vertex buffer
	WORD m_wBrushIndices[MAX_BRUSH_SIZE * MAX_BRUSH_SIZE * 6
						 * 4];              // Brush의 모양을 나타내는 index buffer
	int m_iBrushIndexCount;                 // Brush의 모양을 나타내는 index의 숫자
	int m_iEditMode;                        // Edit의 모드인가?

	CDTexMng* m_pDTexMng;

	MMap m_ColorMapTileTree; //컬러맵 만들기전 타일들 갖고 있는 트리..

	int m_iDistanceTable[DISTANCE_TABLE_SIZE][DISTANCE_TABLE_SIZE];

public:
	float m_fBrushIntensityMap[MAX_BRUSH_SIZE][MAX_BRUSH_SIZE]; // Brush의 변화율을 표시한 맵
	float m_fHeightScale;
	float m_fFlatHeight;
	bool m_bFlaten;

	int m_iZoneID;

	//LightMap관련...
	CDlgSetLightMap* m_pDlgSetLightMap;
	LPDIRECT3DVERTEXBUFFER9 m_LightMapVB;
	CN3Texture*** m_ppLightMapTexture;
	bool** m_ppIsLightMap;
	int m_iNumLightMap;
	bool m_bDrawLineLightMap; //라이트 맵 그리는 옵션에서.. 라인형태로 그리는지..
	__VertexXyzColor
		m_vLineLightMap[2];   //라인형태로 그린다면 그 라인을 화면에 표시해야되겠지?..^^ 그점이야..

	//LOD관련..
	int m_iHeightLimit;
	int m_iDistLimit;

	std::list<CQTNode*> m_RenderNodes;

	LPDIRECT3DVERTEXBUFFER9 m_ColorMapVB; //	컬러맵쓸때..
	LPDIRECT3DVERTEXBUFFER9 m_TileVB;     //	하나의 타일과 하나의 라이트맵

	//common..
	MAPDATA** m_ppMapData;
	BOOL** m_ppRenderInfo;

	//texture..
	CN3Texture** m_pColorTexture;

	RECT m_VisibleRect;   //타일단위의 가시영역..
	POINT m_EyePos;       //타일단위..

	int m_iHeightMapSize; // Cell Count 와 비슷한 개념 -> 1 을 빼주어야 Cell Count 이다.
	int m_iMaxLevel;

	int m_iNumTileMap;    //현재 가지고 있는 타일맵의 수..

	int m_iColorMapTexSize;
	int m_iColorMapPixelPerUnitDistance;
	int m_iNumColorMap;

	//타일찍는 거 방향과 관련...
	float m_fTileDirU[8][4];
	float m_fTileDirV[8][4];
	float m_fTileDirUforColorMap[8][4];
	float m_fTileDirVforColorMap[8][4];

	// DTex관련...
	LPDIRECT3DVERTEXBUFFER9 m_pDTexVB;
	__VertexT1* m_pDTexVertices;
	int m_DTexFaceCnt;
	int m_DTexBuff[5000];
	int m_DTexAttrTable[9][14];
	int m_DTexDirTable[9][14];

	// ColorMap 관련...
	LPDIRECT3DVERTEXBUFFER9 m_pColorMapTmpVB;
	__VertexTransformedT2* m_pColorMapTmpVertices;

	//Brush 관련..
	BOOL m_bBrushFlat; //지형 높이고 내리는 브러쉬를 평평하게 할꺼야?

protected:
	int DetectRealLightMap(int sx, int sz, int range);
	void MakeDistanceTable();
	void FillLightMap(int x, int z, int lcx, int lcz, DWORD Color, float alpha);
	void SetLightMap(int x, int z, int lcx, int lcz);

	void PutColorMapTile(
		int x, int z); //컬러맵 만들 타일 넣기...(아직 안만들어진...그런 타일들 대기열에 넣는것..)
	void VerifyTexInfo(LPDTEXINFO pTex);
	void SetColorMap(int x, int y);
	void SetPertinentTile(CDTexGroupMng* pDTexGroupMng, LPDTEXINFO pDest1, LPDTEXINFO pDest2);

	inline int Log2(int x);
	inline void ClearRenderInfo();
	void SetVisibleRect(); //타일단위..

	//map editor 관련..
	//
	void SetDTexManual(int x, int y);             //수동찍기
	void SetDTex(int x, int y);                   //자동찍기.
	void Heighten(POINT ptCenter, float fHeight); // 지형을 높게하거나 낮게 하는 함수
	void Smooth(POINT ptCenter); // 지형을 주변 맵의 높이값과 비교해서 부드럽게 한다.
	void Flaten(POINT ptCenter);
	void SetApexHeight(int x, int z, float fHeight); // 높이맵의 높이값 지정
	float GetApexHeight(int x, int z) const;         // 높이맵의 높이값 얻기
	float GetFallOffValue(float fFallOff, float x);  // BrushIntensity 계산할때 쓰이는 식.

	//개선된 라이트맵관련..(2002. 11. 13..)
	void ConvertLightMapToolDataV2toV3();
	//

public:
	void UpdateBrushArea(POINT ptCenter); // Brush 영역 표시 vertex갱신
	void RenderBrushArea();               // Brush 영역 표시 그리기

	void ExportHeightBMP(const char* szPathName);
	void ImportHeightBMP(const char* szPathName);

	void MakeMoveTable(short** ppEvent);
	void Init(int HeightMapSize = 5);
	bool LoadFromFile(const char* lpszPath);
	bool SaveToFile(const char* lpszPath);
	bool SaveToFilePartition(const char* lpszPath, float psx, float psz, float width);
	void Tick();
	void Render();
	void Release();

	void MakeGameLightMap(char* szFullPathName);
	void MakeGameColorMap(char* szFullPathName);
	void GeneraterColorMap(bool bIsAll = false);
	void TilingAll(); //지형 전체를 선택된 타일로 깔기..

	void UpDateDistLimit();

	CN3Texture* GetTileTex(int id);
	void SetFillMode(D3DFILLMODE fillmode);
	float GetHeight(float x, float z);

	//map editor 관련..
	//
	SIZE GetPatchNum(float fSize);
	BOOL MouseMsgFilter(LPMSG pMsg); // 지형 고칠때 마우스 메세지 처리
	bool Pick(int x, int y, __Vector3* vec, POINT* pHeightMapPos = nullptr);
	void Import(LPCTSTR pFileName, float fSize);
	void ImportHeight(LPCTSTR pFileName);
	void SaveServerData(File& file);
	void SetEditMode(int iEditMode); // 지형 Edit모드로 변경
	void UpdateBrushIntensityMap(int iShape, int iSize,
		float fFallOff); // 브러쉬의 모양과 사이즈에 따라서 IntensityMap을 다시 구성한다.
	void SaveGameData(File& file);
	void ColorMapImport(LPCTSTR lpszPathName);
	void ColorMapExport(LPCTSTR lpszPathName);
	void GenerateMiniMap(LPCTSTR lpszPathName, int size);
	//

	CLyTerrain();
	virtual ~CLyTerrain();
};

#endif // !defined(AFX_N3TERRAIN_H__7BCDE4B8_8FBC_432E_B4CB_4A56C5FC29F0__INCLUDED_)

