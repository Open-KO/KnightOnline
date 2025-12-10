#pragma once

#ifdef _3DSERVER
#include "My_3DStruct.h"
#else
#include "N3BaseFileAccess.h"
#endif // end of #ifndef _3DSERVER

constexpr int CELL_MAIN_DIVIDE = 4; // 메인셀은 4 X 4 의 서브셀로 나뉜다..
constexpr int CELL_SUB_SIZE = 4; // 4 Meter 가 서브셀의 사이즈이다..
constexpr int CELL_MAIN_SIZE = CELL_MAIN_DIVIDE * CELL_SUB_SIZE; // 메인셀 크기는 서브셀갯수 X 서브셀 크기이다.
constexpr int MAX_CELL_MAIN = 4096 / CELL_MAIN_SIZE; // 메인셀의 최대 갯수는 지형크기 / 메인셀크기 이다.
constexpr int MAX_CELL_SUB = MAX_CELL_MAIN * CELL_MAIN_DIVIDE; // 서브셀 최대 갯수는 메인셀 * 메인셀나눔수 이다.

#ifdef _3DSERVER
class CN3ShapeMgr
#else
#include <list>
#include <vector>
class CN3Shape;
class CN3ShapeMgr : public CN3BaseFileAccess
#endif // end of #ifndef _3DSERVER
{
public:
	struct __CellSub // 하위 셀 데이터
	{
		int 		nCCPolyCount; // Collision Check Polygon Count
		uint32_t* pdwCCVertIndices; // Collision Check Polygon Vertex Indices - wCCPolyCount * 3 만큼 생성된다.

		void Load(HANDLE hFile)
		{
			DWORD dwRWC = 0;

			ReadFile(hFile, &nCCPolyCount, 4, &dwRWC, nullptr);

			if (nCCPolyCount > 0)
			{
				delete[] pdwCCVertIndices;
				pdwCCVertIndices = new uint32_t[nCCPolyCount * 3];
				__ASSERT(pdwCCVertIndices, "New memory failed");

				ReadFile(hFile, pdwCCVertIndices, nCCPolyCount * 3 * 4, &dwRWC, nullptr);

				// TRACE(_T("CollisionCheckPolygon : %d\n"), nCCPolyCount);
			}
		}

#ifdef _N3TOOL
		void Save(HANDLE hFile)
		{
			DWORD dwRWC = 0;
			WriteFile(hFile, &nCCPolyCount, 4, &dwRWC, nullptr);
			if (nCCPolyCount > 0)
				WriteFile(hFile, pdwCCVertIndices, nCCPolyCount * 3 * 4, &dwRWC, nullptr);
		}
#endif // end of _N3TOOL

		__CellSub()
		{
			nCCPolyCount = 0;
			pdwCCVertIndices = nullptr;
		}

		~__CellSub()
		{
			delete[] pdwCCVertIndices;
		}
	};

	struct __CellMain // 기본 셀 데이터
	{
		int		nShapeCount; // Shape Count;
		uint16_t* pwShapeIndices; // Shape Indices
		__CellSub SubCells[CELL_MAIN_DIVIDE][CELL_MAIN_DIVIDE];

		void Load(HANDLE hFile)
		{
			DWORD dwRWC = 0;

			ReadFile(hFile, &nShapeCount, 4, &dwRWC, nullptr);

			if (nShapeCount > 0)
			{
				delete[] pwShapeIndices;
				pwShapeIndices = new uint16_t[nShapeCount];
				ReadFile(hFile, pwShapeIndices, nShapeCount * 2, &dwRWC, nullptr);
			}

			for (int z = 0; z < CELL_MAIN_DIVIDE; z++)
			{
				for (int x = 0; x < CELL_MAIN_DIVIDE; x++)
					SubCells[x][z].Load(hFile);
			}
		}

#ifdef _N3TOOL
		void Save(HANDLE hFile)
		{
			DWORD dwRWC = 0;
			WriteFile(hFile, &nShapeCount, 4, &dwRWC, nullptr);
			if (nShapeCount > 0)
				WriteFile(hFile, pwShapeIndices, nShapeCount * 2, &dwRWC, nullptr);

			for (int z = 0; z < CELL_MAIN_DIVIDE; z++)
			{
				for (int x = 0; x < CELL_MAIN_DIVIDE; x++)
					SubCells[x][z].Save(hFile);
			}
		}
#endif // end of _N3TOOL

		__CellMain()
		{
			nShapeCount = 0;
			pwShapeIndices = nullptr;
		}

		~__CellMain()
		{
			delete[] pwShapeIndices;
		}
	};

	__Vector3* m_pvCollisions;

protected:
#ifndef _3DSERVER
	std::vector<CN3Shape*>	m_Shapes;			// 리스트로 안 만든 이유는... 배열이 훨씬 효율적이기 때문이다.
	std::list<CN3Shape*>	m_ShapesToRender;	// Tick 을 호출하면 렌더링할 것만 추린다..
	std::list<CN3Shape*>	m_ShapesHaveID;		// ID 를 갖고 있어 NPC 가 될수 있는 Shapes....
#endif // end of #ifndef _3DSERVER

	float					m_fMapWidth;	// 맵 너비.. 미터 단위
	float					m_fMapLength;	// 맵 길이.. 미터 단위
	int						m_nCollisionFaceCount;
	__CellMain*				m_pCells[MAX_CELL_MAIN][MAX_CELL_MAIN];

#ifdef _N3TOOL
	std::list<__Vector3>	m_CollisionExtras; // 추가로 넣을 충돌체크 데이터
#endif // end of #ifedef _N3TOOL

public:
#ifndef _3DSERVER
	// 고유 ID 를 가진 오브젝트... NPC 로 쓸수 있는 오브젝트를 검색해서 돌려준다..
	CN3Shape* ShapeGetByID(int iID);

	// 위치를 돌려준다..
	CN3Shape* Pick(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = nullptr);

	CN3Shape* PickMovable(int iXScreen, int iYScreen, __Vector3* pvPick);
#endif // end of #ifndef _3DSERVER
	void SubCell(const __Vector3& vPos, __CellSub** ppSubCell);

	// 해당 위치의 셀 포인터를 돌려준다.
	__CellSub* SubCell(float fX, float fZ)
	{
		int x = (int) (fX / CELL_MAIN_SIZE);
		int z = (int) (fZ / CELL_MAIN_SIZE);

		__ASSERT(x >= 0 && x < MAX_CELL_MAIN && z >= 0 && z < MAX_CELL_MAIN, "Invalid cell number");
		if (x < 0
			|| x >= MAX_CELL_MAIN
			|| z < 0
			|| z >= MAX_CELL_MAIN)
			return nullptr;

		if (nullptr == m_pCells[x][z])
			return nullptr;

		int xx = (((int) fX) % CELL_MAIN_SIZE) / CELL_SUB_SIZE;
		int zz = (((int) fZ) % CELL_MAIN_SIZE) / CELL_SUB_SIZE;

		return &m_pCells[x][z]->SubCells[xx][zz];
	}

	// 가장 가까운 높이을 돌려준다. 없으면 -FLT_MAX 을 돌려준다.
	float		GetHeightNearstPos(const __Vector3& vPos, float fDist, __Vector3* pvNormal = nullptr);

	// 가장 가까운 높이을 돌려준다. 없으면 -FLT_MAX 을 돌려준다.
	float		GetHeightNearstPos(const __Vector3& vPos, __Vector3* pvNormal = nullptr);

	// 현재 지점에서 제일 높은 값을 돌려준다. 없으면 -FLT_MAX 을 돌려준다.
	float		GetHeight(float fX, float fZ, __Vector3* pvNormal = nullptr);

	// 벡터 사이에 걸친 셀포인터 돌려준다..
	int			SubCellPathThru(const __Vector3& vFrom, const __Vector3& vAt, int iMaxSubCell, __CellSub** ppSubCells);

	// 맵의 너비. 단위는 미터이다.
	float Width() const {
		return m_fMapWidth;
	}

	// 맵의 너비. 단위는 미터이다.
	float Height() const {
		return m_fMapWidth;
	}

#ifdef _N3GAME
	virtual void UpdateLoadStatus(int iLoadedShapes, int iTotalShapes) = 0;
#endif

#ifndef _3DSERVER
	void		ReleaseShapes();

	// 넣은 위치에 있는 충돌 메시를 그려준다.. 디버깅용...
	void		RenderCollision(const __Vector3& vPos);

	void		Tick();
	void		Render();
	bool		Load(HANDLE hFile);
	bool		CheckCollisionCamera(__Vector3& vEye, const __Vector3& vAt, float fNP);
	static int SortByCameraDistance(const void* pArg1, const void* pArg2);
#endif // end of #ifndef _3DSERVER

	bool		CheckCollision(
		const __Vector3& vPos,			// 충돌 위치
		const __Vector3& vDir,			// 방향 벡터
		float fSpeedPerSec,				// 초당 움직이는 속도
		__Vector3* pvCol = nullptr,		// 충돌 지점
		__Vector3* pvNormal = nullptr,	// 충돌한면의 법선벡터
		__Vector3* pVec = nullptr);		// 충돌한 면 의 폴리곤 __Vector3[3]

	bool		Create(float fMapWidth, float fMapLength); // 맵의 너비와 높이를 미터 단위로 넣는다..
	bool		LoadCollisionData(HANDLE hFile);

#ifdef _N3TOOL
	//지형에서 shape가 있는 타일은 1, 없는 타일은 0으로 셋팅한 테이블을 만든다.
	void		MakeMoveTable(int16_t** pMoveArray);

	int			Add(CN3Shape* pShape);
	bool		AddCollisionTriangle(const __Vector3& v1, const __Vector3& v2, const __Vector3& v3);
	void		GenerateCollisionData();
	bool		Save(HANDLE hFile);
	bool		SaveCollisionData(HANDLE hFile);
#endif // end of _N3TOOL

	void Release();
	CN3ShapeMgr();
	virtual ~CN3ShapeMgr();
};
