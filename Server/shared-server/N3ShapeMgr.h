#pragma once

#include "My_3DStruct.h"

constexpr int CELL_MAIN_DIVIDE = 4; // 메인셀은 4 X 4 의 서브셀로 나뉜다..
constexpr int CELL_SUB_SIZE = 4; // 4 Meter 가 서브셀의 사이즈이다..
constexpr int CELL_MAIN_SIZE = CELL_MAIN_DIVIDE * CELL_SUB_SIZE; // 메인셀 크기는 서브셀갯수 X 서브셀 크기이다.
constexpr int MAX_CELL_MAIN = 4096 / CELL_MAIN_SIZE; // 메인셀의 최대 갯수는 지형크기 / 메인셀크기 이다.
constexpr int MAX_CELL_SUB = MAX_CELL_MAIN * CELL_MAIN_DIVIDE; // 서브셀 최대 갯수는 메인셀 * 메인셀나눔수 이다.

class CN3ShapeMgr
{
public:
	// 하위 셀 데이터
	struct __CellSub
	{
		int 		nCCPolyCount;		// Collision Check Polygon Count
		uint32_t*	pdwCCVertIndices;	// Collision Check Polygon Vertex Indices - wCCPolyCount * 3 만큼 생성된다.

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

		__CellSub()
		{
			memset(this, 0, sizeof(__CellSub));
		}

		~__CellSub()
		{
			delete[] pdwCCVertIndices;
		}
	};

	// 기본 셀 데이터
	struct __CellMain
	{
		int			nShapeCount;	// Shape Count;
		uint16_t*	pwShapeIndices;	// Shape Indices
		__CellSub	SubCells[CELL_MAIN_DIVIDE][CELL_MAIN_DIVIDE];

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
	float					m_fMapWidth;	// 맵 너비.. 미터 단위
	float					m_fMapLength;	// 맵 길이.. 미터 단위
	int						m_nCollisionFaceCount;
	__CellMain* m_pCells[MAX_CELL_MAIN][MAX_CELL_MAIN];

public:
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
	float Width() const
	{
		return m_fMapWidth;
	}

	// 맵의 너비. 단위는 미터이다.
	float Height() const
	{
		return m_fMapWidth;
	}

	bool		CheckCollision(
		const __Vector3& vPos,			// 충돌 위치
		const __Vector3& vDir,			// 방향 벡터
		float fSpeedPerSec,				// 초당 움직이는 속도
		__Vector3* pvCol = nullptr,		// 충돌 지점
		__Vector3* pvNormal = nullptr,	// 충돌한면의 법선벡터
		__Vector3* pVec = nullptr);		// 충돌한 면 의 폴리곤 __Vector3[3]

	bool		Create(float fMapWidth, float fMapLength); // 맵의 너비와 높이를 미터 단위로 넣는다..
	bool		LoadCollisionData(HANDLE hFile);

	void Release();
	CN3ShapeMgr();
	virtual ~CN3ShapeMgr();
};
