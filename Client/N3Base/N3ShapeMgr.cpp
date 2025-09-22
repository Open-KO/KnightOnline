// N3ShapeMgr.cpp: implementation of the CN3ShapeMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _N3GAME
#include "StdAfxBase.h"
#endif // end of #ifndef _N3GAME

#include "N3ShapeMgr.h"

#ifndef _3DSERVER
#include "N3ShapeExtra.h"
#endif // end of #ifndef _3DSERVER

#include <shared/globals.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3ShapeMgr::CN3ShapeMgr()
{
	m_fMapWidth = 0.0f;
	m_fMapLength = 0.0f;
	m_nCollisionFaceCount = 0;
	m_pvCollisions = nullptr;
	memset(m_pCells, 0, sizeof(m_pCells));
}

CN3ShapeMgr::~CN3ShapeMgr()
{
#ifndef _3DSERVER
	for (CN3Shape* pShape : m_Shapes)
		delete pShape;
	m_Shapes.clear();
#endif // end of #ifndef _3DSERVER

	delete[] m_pvCollisions;
	m_pvCollisions = nullptr;

	for (int z = 0; z < MAX_CELL_MAIN; z++)
	{
		for (int x = 0; x < MAX_CELL_MAIN; x++)
			delete m_pCells[x][z];
	}
}

void CN3ShapeMgr::Release()
{
#ifndef _3DSERVER
	ReleaseShapes();
#endif // end of #ifndef _3DSERVER

	m_fMapWidth = 0.0f;
	m_fMapLength = 0.0f;
	m_nCollisionFaceCount = 0;

	delete[] m_pvCollisions;
	m_pvCollisions = nullptr;

	for (int z = 0; z < MAX_CELL_MAIN; z++)
	{
		for (int x = 0; x < MAX_CELL_MAIN; x++)
		{
			delete m_pCells[x][z];
			m_pCells[x][z] = nullptr;
		}
	}

	memset(m_pCells, 0, sizeof(MAX_CELL_MAIN));

#ifndef _3DSERVER
	m_ShapesToRender.clear();
#endif // end of #ifndef _3DSERVER

#ifdef _N3TOOL
	m_CollisionExtras.clear(); // 추가로 넣을 충돌체크 데이터
#endif // end of #ifedef _N3TOOL
}

#ifndef _3DSERVER
void CN3ShapeMgr::ReleaseShapes()
{
	for (CN3Shape* pShape : m_Shapes)
		delete pShape;
	m_Shapes.clear();
	m_ShapesHaveID.clear();
	m_ShapesToRender.clear();
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
bool CN3ShapeMgr::Load(HANDLE hFile)
{
	DWORD dwRWC;
	int nL = 0;

	if (m_iFileFormatVersion >= N3FORMAT_VER_1264)
	{
		int iIdk0;
		ReadFile(hFile, &iIdk0, sizeof(int), &dwRWC, nullptr);

		int iNL;
		ReadFile(hFile, &iNL, sizeof(int), &dwRWC, nullptr);
		if (iNL > 0)
		{
			m_szName.resize(iNL);
			ReadFile(hFile, &m_szName[0], iNL, &dwRWC, nullptr);
		}
	}

	if (!LoadCollisionData(hFile))
		return false;

	int iSC = 0;
	if (!m_Shapes.empty())
		ReleaseShapes();
	m_ShapesHaveID.clear();

	ReadFile(hFile, &iSC, 4, &dwRWC, nullptr); // Shape Count
	if (iSC > 0)
	{
		CN3Shape* pShape = nullptr;
		m_Shapes.reserve(iSC);

		uint32_t dwType = 0;
		for (int i = 0; i < iSC; i++)
		{
			ReadFile(hFile, &dwType, 4, &dwRWC, nullptr); // Shape Type

			// 성문등 확장된 Object 로 쓸경우..
			if (dwType & OBJ_SHAPE_EXTRA)
				pShape = new CN3ShapeExtra();
			else
				pShape = new CN3Shape();

			pShape->m_iFileFormatVersion = m_iFileFormatVersion;

			m_Shapes.push_back(pShape);

			// pShape->m_iEventID; 바인드 포인트 100~, 200~ 성문 1100~, 1200~ 레버 2100~, 2200~
			// pShape->m_iEventType; 0-바인드포인트 1-성문(좌우열림) 2-성문(상하열림) 3-레버(상하당김) 4-깃발(보임, 안보임)
			// pShape->m_iNPC_ID; 조종할 Object ID
			// pShape->m_iNPC_Status; toggle 0, 1

			pShape->Load(hFile);

			//  ID 가 있는 오브젝트 ... NPC 로 쓸수 있다..
			if (pShape->m_iEventID != 0)
			{
				m_ShapesHaveID.push_back(pShape);
				pShape->MakeCollisionMeshByPartsDetail(); // 현재 모습 그대로... 충돌 메시를 만든다...

				//TRACE(_T("  Load OBject Event : ID(%d) Type(%d) CtrlID(%d) Status(%d)\n"),
					//pShape->m_iEventID, pShape->m_iEventType, pShape->m_iNPC_ID, pShape->m_iNPC_Status);

				switch (pShape->m_iEventType)
				{
					case OBJECT_TYPE_BIND: // 좌우열림성문,
					case OBJECT_TYPE_WARP_GATE:
						pShape->m_bVisible = true;
						break;

					default:
						pShape->m_bVisible = false;
				}
			}

#ifdef _N3GAME
			// 강제 코딩... 각종 성문 열기..
//			if(dwType & OBJ_SHAPE_EXTRA)
//			{
//				CN3ShapeExtra* pSE = (CN3ShapeExtra*)pShape; // 성문등 확장된 Object 로 쓸경우..
//				pSE->RotateTo(0, __Vector3(0,1,0), 80, 1, true); // 바로 열기.
//				pSE->RotateTo(1, __Vector3(0,1,0), -80, 1, true); // 바로 열기.
//			}

			if (!(i % 64))
				UpdateLoadStatus(i, iSC);
#endif // end of #ifndef _N3GAME
		}
	}

	return true;
}
#endif // end of #ifndef _3DSERVER

bool CN3ShapeMgr::LoadCollisionData(HANDLE hFile)
{
	DWORD dwRWC;

	ReadFile(hFile, &m_fMapWidth, 4, &dwRWC, nullptr);
	ReadFile(hFile, &m_fMapLength, 4, &dwRWC, nullptr);

	if (!Create(m_fMapWidth, m_fMapLength))
		return false;

	// 충돌 체크 폴리곤 데이터 읽기..
	ReadFile(hFile, &m_nCollisionFaceCount, 4, &dwRWC, nullptr);

	delete[] m_pvCollisions;
	m_pvCollisions = nullptr;

	if (m_nCollisionFaceCount > 0)
	{
		m_pvCollisions = new __Vector3[m_nCollisionFaceCount * 3];
		ReadFile(hFile, m_pvCollisions, sizeof(__Vector3) * m_nCollisionFaceCount * 3, &dwRWC, nullptr);
	}

#if !defined(_3DSERVER)
	if (m_iFileFormatVersion == N3FORMAT_VER_HERO)
	{
		// NOTE(srmeier): for the "ah_hapbi_zone.opd" the jump seems to be specifically 0x338 bytes
		uint8_t* tmp = new uint8_t[0x338];
		ReadFile(hFile, tmp, 0x338, &dwRWC, nullptr);
		delete[] tmp;
	}
#endif

	// Cell Data 쓰기.
	int iExist = 0;
	int z = 0;
	for (float fZ = 0.0f; fZ < m_fMapLength; fZ += CELL_MAIN_SIZE, z++)
	{
		int x = 0;
		for (float fX = 0.0f; fX < m_fMapWidth; fX += CELL_MAIN_SIZE, x++)
		{
			delete m_pCells[x][z]; m_pCells[x][z] = nullptr;

			ReadFile(hFile, &iExist, 4, &dwRWC, nullptr); // 데이터가 있는 셀인지 쓰고..

			if (iExist == 0)
				continue;

			m_pCells[x][z] = new __CellMain;
			m_pCells[x][z]->Load(hFile);
		}
	}

	return true;
}

#ifdef _N3TOOL
bool CN3ShapeMgr::Save(HANDLE hFile)
{
	if (!SaveCollisionData(hFile))
		return false;

#ifndef _3DSERVER
	DWORD dwRWC;
	int iSC = static_cast<int>(m_Shapes.size());

	WriteFile(hFile, &iSC, 4, &dwRWC, nullptr); // Shape Count
	for (CN3Shape* pShape : m_Shapes)
	{
		uint32_t dwType = pShape->Type();
		if (pShape->m_iEventID != 0
			|| pShape->m_iEventType != 0
			|| pShape->m_iNPC_ID != 0
			|| pShape->m_iNPC_Status != 0)
			dwType |= OBJ_SHAPE_EXTRA; // NPC ID 가 있으면.. 확장 Shape 로 ...

		WriteFile(hFile, &dwType, 4, &dwRWC, nullptr); // Shape Type
		pShape->CollisionMeshSet(""); // 충돌 메시는 지워준다..
		pShape->ClimbMeshSet(""); // 충돌 메시는 지워준다..
		pShape->Save(hFile);
	}
#endif // end of #ifndef _3DSERVER
	return true;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
bool CN3ShapeMgr::SaveCollisionData(HANDLE hFile)
{
	DWORD dwRWC;

	WriteFile(hFile, &m_fMapWidth, 4, &dwRWC, nullptr); // 맵 실제 미터 단위 너비
	WriteFile(hFile, &m_fMapLength, 4, &dwRWC, nullptr); // 맵 실제 미터 단위 길이

	// 충돌 체크 폴리곤 데이터 쓰기..
	WriteFile(hFile, &m_nCollisionFaceCount, 4, &dwRWC, nullptr);
	if (m_nCollisionFaceCount > 0)
		WriteFile(hFile, m_pvCollisions, sizeof(__Vector3) * m_nCollisionFaceCount * 3, &dwRWC, nullptr);

	// Cell Data 쓰기.
	int z = 0;
	for (float fZ = 0.0f; fZ < m_fMapLength; fZ += CELL_MAIN_SIZE, z++)
	{
		int x = 0;
		for (float fX = 0.0f; fX < m_fMapWidth; fX += CELL_MAIN_SIZE, x++)
		{
			int iExist = 0;
			if (m_pCells[x][z] != nullptr)
				iExist = 1;

			// 데이터가 있는 셀인지 쓰고..
			WriteFile(hFile, &iExist, 4, &dwRWC, nullptr);

			if (m_pCells[x][z] != nullptr)
				m_pCells[x][z]->Save(hFile);
		}
	}

	return true;
}
#endif // end of _N3TOOL

// 맵의 너비와 높이를 미터 단위로 넣는다..
bool CN3ShapeMgr::Create(float fMapWidth, float fMapLength)
{
	if (fMapWidth <= 0.0f
		|| fMapWidth > MAX_CELL_MAIN * CELL_MAIN_SIZE
		|| fMapLength <= 0.0f
		|| fMapLength > MAX_CELL_MAIN * CELL_MAIN_SIZE)
		return false;

	m_fMapWidth = fMapWidth;
	m_fMapLength = fMapLength;

	return true;
}

#ifdef _N3TOOL
void CN3ShapeMgr::GenerateCollisionData()
{
	int nFC = 0;

	// Shape 에 있는 충돌 메시 만큼 생성.
	for (CN3Shape* pShape : m_Shapes)
	{
		CN3VMesh* pVM = pShape->CollisionMesh();
		if (pVM == nullptr)
			continue;

		int nIC = pVM->IndexCount();
		if (nIC > 0)
			nFC += nIC / 3;
		else
			nFC += pVM->VertexCount() / 3;
	}

	nFC += static_cast<int>(m_CollisionExtras.size()) / 3; // 추가 충돌 데이터..

	if (nFC <= 0)
		return;

	m_nCollisionFaceCount = nFC;

	delete[] m_pvCollisions;
	m_pvCollisions = new __Vector3[nFC * 3]; // 충돌 폴리곤 생성

	memset(m_pvCollisions, 0, sizeof(__Vector3) * nFC * 3);

	int nCPC = 0; // Collision Polygon Count

	// Shape 에 있는 충돌 메시 만큼 데이터 복사..
	for (CN3Shape* pShape : m_Shapes)
	{
		CN3VMesh* pVMesh = pShape->CollisionMesh();
		if (pVMesh == nullptr)
			continue;

		__Vector3* pVSrc = pVMesh->Vertices();
		int nIC = pVMesh->IndexCount();
		if (nIC > 0)
		{
			uint16_t* pwIs = pVMesh->Indices();
			for (int j = 0; j < nIC; j++)
				m_pvCollisions[nCPC++] = pVSrc[pwIs[j]] * pShape->m_Matrix; // 월드 위치이다.
		}
		else
		{
			int nVC = pVMesh->VertexCount();
			for (int j = 0; j < nVC; j++)
				m_pvCollisions[nCPC++] = pVSrc[j] * pShape->m_Matrix; // 월드 위치이다.
		}
	}

	// 추가 충돌 데이터 넣기..
	for (const __Vector3& vColExtra : m_CollisionExtras)
		m_pvCollisions[nCPC++] = vColExtra;

	if (nCPC != (nFC * 3))
	{
		__ASSERT(0, "충돌 체크 폴리곤의 점갯수와 면 갯수가 다릅니다.");
		Release();
		return;
	}

	// 각 선분이 셀에 걸쳐 있는지 혹은 포함되어 있는지 등등 판단해서 인덱스 생성을 한다.
	int xSMax = (int) (m_fMapWidth / CELL_SUB_SIZE);
	int zSMax = (int) (m_fMapLength / CELL_SUB_SIZE);
	for (int i = 0; i < nFC; i++)
	{
		__Vector3 vEdge[3][2];

		vEdge[0][0] = m_pvCollisions[i * 3];
		vEdge[0][1] = m_pvCollisions[i * 3 + 1];
		vEdge[1][0] = m_pvCollisions[i * 3 + 1];
		vEdge[1][1] = m_pvCollisions[i * 3 + 2];
		vEdge[2][0] = m_pvCollisions[i * 3 + 2];
		vEdge[2][1] = m_pvCollisions[i * 3];

		// 걸쳐 있는 메시 만큼 생성...
		for (int j = 0; j < 3; j++)
		{
			// 범위를 정하고..
			int xx1 = 0, xx2 = 0, zz1 = 0, zz2 = 0;

			if (vEdge[j][0].x < vEdge[j][1].x)
			{
				xx1 = (int) (vEdge[j][0].x / CELL_SUB_SIZE) - 1;
				xx2 = (int) (vEdge[j][1].x / CELL_SUB_SIZE) + 1;
			}
			else
			{
				xx1 = (int) (vEdge[j][1].x / CELL_SUB_SIZE) - 1;
				xx2 = (int) (vEdge[j][0].x / CELL_SUB_SIZE) + 1;
			}

			if (xx1 < 0)
				xx1 = 0;

			if (xx1 >= xSMax)
				xx1 = xSMax - 1;

			if (xx2 < 0)
				xx2 = 0;

			if (xx2 >= xSMax)
				xx2 = xSMax - 1;

			if (vEdge[j][0].z < vEdge[j][1].z)
			{
				zz1 = (int) (vEdge[j][0].z / CELL_SUB_SIZE) - 1;
				zz2 = (int) (vEdge[j][1].z / CELL_SUB_SIZE) + 1;
			}
			else
			{
				zz1 = (int) (vEdge[j][1].z / CELL_SUB_SIZE) - 1;
				zz2 = (int) (vEdge[j][0].z / CELL_SUB_SIZE) + 1;
			}

			if (zz1 < 0) zz1 = 0;
			if (zz1 >= zSMax) zz1 = zSMax - 1;
			if (zz2 < 0) zz2 = 0;
			if (zz2 >= zSMax) zz2 = zSMax - 1;

			// 범위만큼 처리..
			for (int z = zz1; z <= zz2; z++)
			{
				float fZMin = (float) (z * CELL_SUB_SIZE);
				float fZMax = (float) ((z + 1) * CELL_SUB_SIZE);

				for (int x = xx1; x <= xx2; x++)
				{
					float fXMin = (float) (x * CELL_SUB_SIZE);
					float fXMax = (float) ((x + 1) * CELL_SUB_SIZE);

					// Cohen thuderland algorythm
					uint32_t dwOC0 = 0, dwOC1 = 0; // OutCode 0, 1
					if (vEdge[j][0].z > fZMax)
						dwOC0 |= 0xf000;

					if (vEdge[j][0].z < fZMin)
						dwOC0 |= 0x0f00;

					if (vEdge[j][0].x > fXMax)
						dwOC0 |= 0x00f0;

					if (vEdge[j][0].x < fXMin)
						dwOC0 |= 0x000f;

					if (vEdge[j][1].z > fZMax)
						dwOC1 |= 0xf000;

					if (vEdge[j][1].z < fZMin)
						dwOC1 |= 0x0f00;

					if (vEdge[j][1].x > fXMax)
						dwOC1 |= 0x00f0;

					if (vEdge[j][1].x < fXMin)
						dwOC1 |= 0x000f;

					bool bWriteID = false;

					// 두 끝점이 같은 변의 외부에 있다.
					if (dwOC0 & dwOC1)
						bWriteID = false;
					// 선분이 사각형 내부에 있음
					else if (dwOC0 == 0 && dwOC1 == 0)
						bWriteID = true;
					// 선분 한점은 셀의 내부에 한점은 외부에 있음.
					else if ((dwOC0 == 0 && dwOC1 != 0)
						|| (dwOC0 != 0 && dwOC1 == 0))
						bWriteID = true;
					// 두 꿑점 모두 셀 외부에 있지만 판단을 다시 해야 한다.
					else if ((dwOC0 & dwOC1) == 0)
					{
						// 위의 변과의 교차점을 계산하고..
						float fXCross = vEdge[j][0].x + (fZMax - vEdge[j][0].z) * (vEdge[j][1].x - vEdge[j][0].x) / (vEdge[j][1].z - vEdge[j][0].z);
						if (fXCross < fXMin)
							bWriteID = false; // 완전히 외곽에 있다.
						else
							bWriteID = true; // 걸처있다.
					}

					// 만약 걸린게 없다면... 위에서 직선을 쏘아서 충돌인지 체크한다.
					if (!bWriteID)
					{
						__Vector3 vPos, vDir;
						vDir.Set(0, -1.0f, 0);

						float zz3 = static_cast<float>(z * CELL_SUB_SIZE),
							zz4 = static_cast<float>((z + 1) * CELL_SUB_SIZE);
						float xx3 = static_cast<float>(x * CELL_SUB_SIZE),
							xx4 = static_cast<float>((x + 1) * CELL_SUB_SIZE);
						for (float fZ = zz3; fZ <= zz4 && !bWriteID; fZ += 0.25f)
						{
							for (float fX = xx3; fX <= xx4 && !bWriteID; fX += 0.25f)
							{
								vPos.Set(fX, 10000.0f, fZ);

								// 폴리곤 충돌 체크..
								if (::_IntersectTriangle(vPos, vDir, m_pvCollisions[i * 3], m_pvCollisions[i * 3 + 1], m_pvCollisions[i * 3 + 2]))
									bWriteID = true; // 충돌 폴리곤 인덱스를 쓰게 만든다..
							}
						}
					}

					// 충돌 폴리곤 쓸일 없소..
					if (!bWriteID)
						continue;

					// 충돌 정보를 써야 한다..
					int nX = x / CELL_MAIN_DIVIDE;
					int nZ = z / CELL_MAIN_DIVIDE;
					if (nX < 0
						|| nX >= MAX_CELL_MAIN
						|| nZ < 0
						|| nZ >= MAX_CELL_MAIN)
						continue;

					if (m_pCells[nX][nZ] == nullptr)
						m_pCells[nX][nZ] = new __CellMain;

					int nXSub = x % CELL_MAIN_DIVIDE;
					int nZSub = z % CELL_MAIN_DIVIDE;

					__CellSub* pSubCell = &(m_pCells[nX][nZ]->SubCells[nXSub][nZSub]);
					int nCCPC = pSubCell->nCCPolyCount; // Collision Check Polygon Count

					bool bOverlapped = false;
					for (int k = 0; k < nCCPC; k++) // 중복 되는지 체크
					{
						if (i * 3 == pSubCell->pdwCCVertIndices[k * 3])
						{
							bOverlapped = true;
							break;
						}
					}

					// 겹친게 있는지 체크
					if (bOverlapped)
						continue;

					// 중복된게 없으면..
					if (0 == nCCPC) // 첨 생성 되었으면..
					{
						pSubCell->pdwCCVertIndices = new uint32_t[768];
						memset(pSubCell->pdwCCVertIndices, 0, 768 * 4);
					}
//					else // 이미 있으면..
//					{
//						uint32_t* pwBack = pSubCell->pdwCCVertIndices;
//						pSubCell->pdwCCVertIndices = new uint32_t[(nCCPC+1)*3];
//						memcpy(pSubCell->pdwCCVertIndices, pwBack, nCCPC * 3 * 4); // 점세개가 하나의 폴리곤이고 워드 인덱스이므로..
//						delete [] pwBack;
//					}

					if (nCCPC >= 256)
					{
						__ASSERT(0, "충돌 체크 폴리곤 수가 너무 많습니다");
						continue;
					}

					pSubCell->pdwCCVertIndices[nCCPC * 3 + 0] = i * 3 + 0; // 인덱스 저장..
					pSubCell->pdwCCVertIndices[nCCPC * 3 + 1] = i * 3 + 1; // 인덱스 저장..
					pSubCell->pdwCCVertIndices[nCCPC * 3 + 2] = i * 3 + 2; // 인덱스 저장..
					pSubCell->nCCPolyCount++; // Collision Check Polygon Count 를 늘린다.
				} // end of for(int x = xx1; x <= xx2; x++)
			} // end of for(int z = zz1; z <= zz2; z++) // 범위만큼 처리..
		} // end of for(int j = 0; j < 3; j++) // 걸쳐 있는 메시 만큼 생성...
	}
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
int CN3ShapeMgr::Add(CN3Shape* pShape)
{
	if (pShape == nullptr)
		return -1;

	__Vector3 vPos = pShape->Pos();
	int nX = (int) (vPos.x / CELL_MAIN_SIZE);
	int nZ = (int) (vPos.z / CELL_MAIN_SIZE);
	if (nX < 0
		|| nX >= MAX_CELL_MAIN
		|| nZ < 0
		|| nZ >= MAX_CELL_MAIN)
	{
		__ASSERT(0, "CN3ShapeMgr::Add - Shape Add Failed. Check position");
		return -1;
	}

	pShape->SaveToFile(); // 파일로 저장하고..

	CN3Shape* pShapeAdd = new CN3Shape();

	// 이 파일을 열은 다음
	if (!pShapeAdd->LoadFromFile(pShape->FileName()))
	{
		delete pShapeAdd;
		return -1;
	}

	if (m_pCells[nX][nZ] == nullptr)
		m_pCells[nX][nZ] = new __CellMain;

	int iSC = m_pCells[nX][nZ]->nShapeCount;

	// 첨 생성 되었으면..
	if (0 == iSC)
	{
		m_pCells[nX][nZ]->pwShapeIndices = new uint16_t[iSC + 1];
	}
	// 이미 있으면..
	else
	{
		uint16_t* pwBack = m_pCells[nX][nZ]->pwShapeIndices;
		m_pCells[nX][nZ]->pwShapeIndices = new uint16_t[iSC + 1];
		memcpy(m_pCells[nX][nZ]->pwShapeIndices, pwBack, iSC * 2);
		delete[] pwBack;
	}

	// 인덱스 저장..
	m_pCells[nX][nZ]->pwShapeIndices[iSC] = static_cast<uint16_t>(m_Shapes.size());

	m_Shapes.push_back(pShapeAdd); // 추가 한다..
	m_pCells[nX][nZ]->nShapeCount++; // Shape Count 를 늘린다.

	return m_Shapes.size() - 1;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
bool CN3ShapeMgr::AddCollisionTriangle(const __Vector3& v1, const __Vector3& v2, const __Vector3& v3)
{
	size_t count = m_CollisionExtras.size();
	m_CollisionExtras.push_back(v1); // 추가로 넣을 충돌체크 데이터
	m_CollisionExtras.push_back(v2); // 추가로 넣을 충돌체크 데이터
	m_CollisionExtras.push_back(v3); // 추가로 넣을 충돌체크 데이터

	if ((count + 3) == m_CollisionExtras.size())
		return true;

	return false;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3ShapeMgr::MakeMoveTable(int16_t** pMoveArray)
{
	int ArraySize = (MAX_CELL_MAIN * CELL_MAIN_DIVIDE) + 1;

	for (int bx = 0; bx < MAX_CELL_MAIN; bx++)
	{
		for (int bz = 0; bz < MAX_CELL_MAIN; bz++)
		{
			if (m_pCells[bx][bz] == nullptr)
				continue;

			for (int sx = 0; sx < CELL_MAIN_DIVIDE; sx++)
			{
				for (int sz = 0; sz < CELL_MAIN_DIVIDE; sz++)
				{
					if (m_pCells[bx][bz]->SubCells[sx][sz].nCCPolyCount <= 0)
						continue;

					int ix = (bx * CELL_MAIN_DIVIDE) + sx;
					int iz = (bz * CELL_MAIN_DIVIDE) + sz;
					pMoveArray[ix][iz] = 0;
				} //for(int sz=0; sz<CELL_MAIN_DIVIDE; sz++)
			} //for(int sx=0; sx<CELL_MAIN_DIVIDE; sx++)
		}
	}
}
#endif // end of _N3TOOL

#ifndef _3DSERVER
void CN3ShapeMgr::Tick()
{
	int xMainS = (int) ((s_CameraData.vEye.x - s_CameraData.fFP) / CELL_MAIN_SIZE);
	if (xMainS < 0)
		xMainS = 0;

	int xMainE = (int) ((s_CameraData.vEye.x + s_CameraData.fFP) / CELL_MAIN_SIZE);
	if (xMainE > MAX_CELL_MAIN)
		xMainE = MAX_CELL_MAIN;

	int zMainS = (int) ((s_CameraData.vEye.z - s_CameraData.fFP) / CELL_MAIN_SIZE);
	if (zMainS < 0)
		zMainS = 0;

	int zMainE = (int) ((s_CameraData.vEye.z + s_CameraData.fFP) / CELL_MAIN_SIZE);
	if (zMainE > MAX_CELL_MAIN)
		zMainE = MAX_CELL_MAIN;

	int iSC = static_cast<int>(m_Shapes.size());

	m_ShapesToRender.clear();

	// 렌더링 리스트 비우고..
	for (int zM = zMainS; zM < zMainE; zM++)
	{
		for (int xM = xMainS; xM < xMainE; xM++)
		{
			__CellMain* pCellCur = m_pCells[xM][zM];
			if (pCellCur == nullptr)
				continue;

			int iSCC = pCellCur->nShapeCount;
			for (int i = 0; i < iSCC; i++)
			{
				int iSIndex = pCellCur->pwShapeIndices[i];
				__ASSERT(iSIndex >= 0 && iSIndex < iSC, "Shape Index is invalid");

				CN3Shape* pShape = m_Shapes[iSIndex];
				
				pShape->Tick();
				if (pShape->m_bDontRender)
					continue;

				m_ShapesToRender.push_back(pShape);
			}
		}
	}
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
void CN3ShapeMgr::Render()
{
	for (CN3Shape* pShape : m_ShapesToRender)
	{
		__ASSERT(pShape, "Shape pointer is null!!!");

		pShape->Render();
#if _DEBUG
		pShape->RenderCollisionMesh();
#endif
	}
}
#endif // end of #ifndef _3DSERVER

bool CN3ShapeMgr::CheckCollision(
	const __Vector3& vPos,		// 충돌 위치
	const __Vector3& vDir,		// 방향 벡터
	float fSpeedPerSec,			// 초당 움직이는 속도
	__Vector3* pvCol,			// 충돌 지점 (crash position)
	__Vector3* pvNormal,		// 충돌한면의 법선벡터 (crash normal)
	__Vector3* pVec)			// 충돌한 면 의 폴리곤 __Vector3[3] (polygon/triangle of crash)
{
	// 움직이는 속도가 없거나 반대로 움직이면 넘어간다..
	if (fSpeedPerSec <= 0)
		return false;

	static __CellSub* ppCells[128];

	// 다음 위치
	__Vector3 vPosNext = vPos + (vDir * fSpeedPerSec);

	int iSubCellCount = 0;

	if (fSpeedPerSec < 4.0f)
	{
		__Vector3 vPos2 = vPos + (vDir * 4.0f);
		iSubCellCount = SubCellPathThru(vPos, vPos2, 128, ppCells); // 통과하는 서브셀을 가져온다..
	}
	else
	{
		iSubCellCount = SubCellPathThru(vPos, vPosNext, 128, ppCells); // 통과하는 서브셀을 가져온다..
	}

	// 없음 말자.
	if (iSubCellCount <= 0)
		return false;

	__Vector3 vColTmp(0, 0, 0);
	int nIndex0, nIndex1, nIndex2;
	static float fT, fU, fV;
	float fDistClosest = FLT_MAX;

	for (int i = 0; i < iSubCellCount; i++)
	{
		if (ppCells[i]->nCCPolyCount <= 0)
			continue;

		for (int j = 0; j < ppCells[i]->nCCPolyCount; j++)
		{
			nIndex0 = ppCells[i]->pdwCCVertIndices[j * 3];
			nIndex1 = ppCells[i]->pdwCCVertIndices[j * 3 + 1];
			nIndex2 = ppCells[i]->pdwCCVertIndices[j * 3 + 2];

			// NOTE: does the vector intersect the triangle?
			if (!::_IntersectTriangle(vPos, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp))
				continue;

			if (::_IntersectTriangle(vPosNext, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2]))
				continue;

			float fDistTmp = (vPos - vColTmp).Magnitude(); // 거리를 재보고..
			if (fDistTmp < fDistClosest)
			{
				fDistClosest = fDistTmp;

				// 충돌이다..
				if (pvCol != nullptr)
					*pvCol = vColTmp;

				if (pvNormal != nullptr)
				{
					pvNormal->Cross(
						m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0],
						m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
					pvNormal->Normalize();
				}

				if (pVec != nullptr)
				{
					pVec[0] = m_pvCollisions[nIndex0];
					pVec[1] = m_pvCollisions[nIndex1];
					pVec[2] = m_pvCollisions[nIndex2];
				}
			}
		}
	}

	if (fDistClosest != FLT_MAX)
		return true;

#ifndef _3DSERVER
	// 눈에 보이는것만 대상으로 해서...
	if (!m_ShapesToRender.empty())
	{
		// 거리순으로 정렬..
		std::vector<CN3Shape*> Shapes;
		Shapes.reserve(m_ShapesToRender.size());

		for (CN3Shape* pShape : m_ShapesToRender)
		{
			if (pShape->CollisionMesh() == nullptr)
				continue;

			// 멀리 떨어진거면 지나간다..
			if ((pShape->Pos() - vPos).Magnitude() > pShape->Radius() * 2)
				continue;

			Shapes.push_back(pShape);
		}

		if (Shapes.empty())
			return false;

		// 카메라 거리에 따라 정렬하고..
		if (Shapes.size() > 1)
			qsort(&Shapes[0], Shapes.size(), sizeof(CN3Shape*), SortByCameraDistance);

		for (CN3Shape* pShape : Shapes)
		{
			CN3VMesh* pVMesh = pShape->CollisionMesh();
			if (pVMesh->CheckCollision(pShape->m_Matrix, vPos, vPosNext, pvCol, pvNormal))
				return true;
		}
	}
#endif // end of #ifndef _3DSERVER

	return false;
}

#ifndef _3DSERVER
bool CN3ShapeMgr::CheckCollisionCamera(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	__Vector3 vDir = vEyeResult - vAt;
	float fD = vDir.Magnitude();
	vDir.Normalize();
	__Vector3 vCol(0, 0, 0);
	if (!CheckCollision(vAt, vDir, fD, &vCol))
		return false;

	// 충돌점과 처다보는 거리를 재보고..
	float fDelta = (vEyeResult - vCol).Magnitude();

	// 너무 가까이 붙으면 돌아간다..
	if (fDelta < fNP * 2.0f)
		return false;

	vEyeResult -= vDir * fDelta;
	return true;
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
void CN3ShapeMgr::RenderCollision(const __Vector3& vPos)
{
	int x = (int) (vPos.x / CELL_MAIN_SIZE);
	int z = (int) (vPos.z / CELL_MAIN_SIZE);

	__CellSub* ppCell[9] = {};
	SubCell(vPos, ppCell);

	__Matrix44 mtxWorld;
	mtxWorld.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtxWorld);

	DWORD dwFillPrev;
	s_lpD3DDev->GetRenderState(D3DRS_FILLMODE, &dwFillPrev);
	s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	DWORD dwLight;
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
//	DWORD dwZ;
//	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
//	s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	s_lpD3DDev->SetTexture(0, nullptr);

	for (int i = 0; i < 9; i++)
	{
		if (ppCell[i] == nullptr
			|| ppCell[i]->nCCPolyCount <= 0)
			continue;

		int nFC = ppCell[i]->nCCPolyCount;
		int n0, n1, n2;

		__VertexColor vCols[4];
		__VertexColor vNormalDir[2];
		__Vector3 vDir;

		for (int j = 0; j < nFC; j++)
		{
			n0 = ppCell[i]->pdwCCVertIndices[j * 3 + 0];
			n1 = ppCell[i]->pdwCCVertIndices[j * 3 + 1];
			n2 = ppCell[i]->pdwCCVertIndices[j * 3 + 2];

			vCols[0].Set(m_pvCollisions[n0], 0xffff0000);
			vCols[1].Set(m_pvCollisions[n1], 0xff00ff00);
			vCols[2].Set(m_pvCollisions[n2], 0xff0000ff);
//			vCols[3] = vCols[0]; vCols[3].color = 0xffffffff;

			vDir.Cross(
				(m_pvCollisions[n1] - m_pvCollisions[n0]),
				(m_pvCollisions[n2] - m_pvCollisions[n1]));
			
			vDir.Normalize();

			vNormalDir[0] = (m_pvCollisions[n0] + m_pvCollisions[n1] + m_pvCollisions[n2]) / 3.0f;
			vNormalDir[1] = vNormalDir[0] + vDir;
			vNormalDir[0].color = 0xffff0000;
			vNormalDir[1].color = 0xffffffff;

			s_lpD3DDev->SetFVF(FVF_CV);
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, vCols, sizeof(__VertexColor));
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, vNormalDir, sizeof(__VertexColor));
		}
	}

	s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, dwFillPrev);
	s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
//	s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
CN3Shape* CN3ShapeMgr::Pick(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick)
{
	if (m_ShapesToRender.empty())
		return nullptr;

	__Vector3 vPos, vDir;
	::_Convert2D_To_3DCoordinate(iXScreen, iYScreen, s_CameraData.mtxView, s_CameraData.mtxProjection, s_CameraData.vp, vPos, vDir);

	// 눈에 보이는것만 대상으로 해서...
	int iSC = m_ShapesToRender.size();

	// 거리순으로 정렬..
	std::vector<CN3Shape*> Shapes(m_ShapesToRender.begin(), m_ShapesToRender.end());

	if (Shapes.size() > 1)
		qsort(&Shapes[0], iSC, sizeof(CN3Shape*), SortByCameraDistance);

	for (CN3Shape* pShape : Shapes)
	{
		// 이벤트가 있어야 한다면...
		if (bMustHaveEvent
			&& pShape->m_iEventID <= 0)
			continue;

		if (pShape->CheckCollisionPrecisely(false, vPos, vDir, pvPick) >= 0)
			return pShape;
	}

	return nullptr;
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
CN3Shape* CN3ShapeMgr::PickMovable(int iXScreen, int iYScreen, __Vector3* pvPick)
{
	if (m_ShapesToRender.empty())
		return nullptr;

	__Vector3 vPos, vDir;
	::_Convert2D_To_3DCoordinate(iXScreen, iYScreen, s_CameraData.mtxView, s_CameraData.mtxProjection, s_CameraData.vp, vPos, vDir);

	// 눈에 보이는것만 대상으로 해서...
	// 거리순으로 정렬..
	std::vector<CN3Shape*> Shapes(m_ShapesToRender.begin(), m_ShapesToRender.end());

	if (Shapes.size() > 1)
		qsort(&Shapes[0], Shapes.size(), sizeof(CN3Shape*), SortByCameraDistance);

	for (CN3Shape* pShape : Shapes)
	{
		if (pShape->CheckCollisionPrecisely(false, vPos, vDir, pvPick) >= 0)
			return pShape;
	}

	return nullptr;
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
CN3Shape* CN3ShapeMgr::ShapeGetByID(int iID)
{
	for (CN3Shape* pShape : m_ShapesHaveID)
	{
		if (pShape->m_iEventID == iID)
			return pShape;
	}

	return nullptr;
}
#endif // end of #ifndef _3DSERVER

#ifndef _3DSERVER
int CN3ShapeMgr::SortByCameraDistance(const void* pArg1, const void* pArg2)
{
	CN3Shape* pShape1 = *((CN3Shape**) pArg1);
	CN3Shape* pShape2 = *((CN3Shape**) pArg2);

	float fDist1 = (CN3Base::s_CameraData.vEye - pShape1->Pos()).Magnitude();
	float fDist2 = (CN3Base::s_CameraData.vEye - pShape2->Pos()).Magnitude();

	if (fDist1 < fDist2)
		return -1; // 가까우면 true
	else if (fDist1 > fDist2)
		return 1;

	return 0;
}
#endif // end of #ifndef _3DSERVER

int CN3ShapeMgr::SubCellPathThru(const __Vector3& vFrom, const __Vector3& vAt, int iMaxSubCell, __CellSub** ppSubCells) // 벡터 사이에 걸친 셀포인터 돌려준다..
{
	if (ppSubCells == nullptr)
		return 0;

	// 범위를 정하고..
	int xx1 = 0, xx2 = 0, zz1 = 0, zz2 = 0;

	if (vFrom.x < vAt.x)
	{
		xx1 = (int) (vFrom.x / CELL_SUB_SIZE);
		xx2 = (int) (vAt.x / CELL_SUB_SIZE);
	}
	else
	{
		xx1 = (int) (vAt.x / CELL_SUB_SIZE);
		xx2 = (int) (vFrom.x / CELL_SUB_SIZE);
	}

	if (vFrom.z < vAt.z)
	{
		zz1 = (int) (vFrom.z / CELL_SUB_SIZE);
		zz2 = (int) (vAt.z / CELL_SUB_SIZE);
	}
	else
	{
		zz1 = (int) (vAt.z / CELL_SUB_SIZE);
		zz2 = (int) (vFrom.z / CELL_SUB_SIZE);
	}

	bool bPathThru;
	float fZMin, fZMax, fXMin, fXMax;
	int iSubCellCount = 0;
	for (int z = zz1; z <= zz2; z++) // 범위만큼 처리..
	{
		fZMin = (float) (z * CELL_SUB_SIZE);
		fZMax = (float) ((z + 1) * CELL_SUB_SIZE);

		for (int x = xx1; x <= xx2; x++)
		{
			fXMin = (float) (x * CELL_SUB_SIZE);
			fXMax = (float) ((x + 1) * CELL_SUB_SIZE);

			// Cohen thuderland algorythm
			uint32_t dwOC0 = 0, dwOC1 = 0; // OutCode 0, 1
			if (vFrom.z > fZMax)
				dwOC0 |= 0xf000;

			if (vFrom.z < fZMin)
				dwOC0 |= 0x0f00;

			if (vFrom.x > fXMax)
				dwOC0 |= 0x00f0;

			if (vFrom.x < fXMin)
				dwOC0 |= 0x000f;

			if (vAt.z > fZMax)
				dwOC1 |= 0xf000;

			if (vAt.z < fZMin)
				dwOC1 |= 0x0f00;

			if (vAt.x > fXMax)
				dwOC1 |= 0x00f0;

			if (vAt.x < fXMin)
				dwOC1 |= 0x000f;

			bPathThru = false;

			// 두 끝점이 같은 변의 외부에 있다.
			if (dwOC0 & dwOC1)
				bPathThru = false;
			// 선분이 사각형 내부에 있음
			else if (dwOC0 == 0 && dwOC1 == 0)
				bPathThru = true;
			// 선분 한점은 셀의 내부에 한점은 외부에 있음.
			else if ((dwOC0 == 0 && dwOC1 != 0)
				|| (dwOC0 != 0 && dwOC1 == 0))
				bPathThru = true;
			// 두 L점 모두 셀 외부에 있지만 판단을 다시 해야 한다.
			else if ((dwOC0 & dwOC1) == 0)
			{
				// 위의 변과의 교차점을 계산하고..
				float fXCross = vFrom.x + (fZMax - vFrom.z) * (vAt.x - vFrom.x) / (vAt.z - vFrom.z);
				if (fXCross < fXMin)
					bPathThru = false; // 완전히 외곽에 있다.
				else
					bPathThru = true; // 걸처있다.
			}

			if (!bPathThru)
				continue;

			// 충돌 정보를 써야 한다..
			int nX = x / CELL_MAIN_DIVIDE;
			int nZ = z / CELL_MAIN_DIVIDE;

			// 메인셀바깥에 있음 지나간다.
			if (nX < 0
				|| nX >= MAX_CELL_MAIN
				|| nZ < 0
				|| nZ >= MAX_CELL_MAIN)
				continue;

			// 메인셀이 널이면 지나간다..
			if (m_pCells[nX][nZ] == nullptr)
				continue;

			int nXSub = x % CELL_MAIN_DIVIDE;
			int nZSub = z % CELL_MAIN_DIVIDE;

			// NOTE: the check on nX and nZ isn't good enough because
			//       "z/CELL_MAIN_DIVIDE" will round a small neg "z" to zero
			//       and we'll run into an error!!!!!
			if (nXSub < 0
				|| nXSub >= (MAX_CELL_MAIN % CELL_MAIN_DIVIDE)
				|| nZSub < 0
				|| nZSub >= (MAX_CELL_MAIN % CELL_MAIN_DIVIDE))
				continue;

			ppSubCells[iSubCellCount++] = &m_pCells[nX][nZ]->SubCells[nXSub][nZSub];

			if (iSubCellCount >= iMaxSubCell)
				return iMaxSubCell;
		} // end of for(int x = xx1; x <= xx2; x++)
	} // end of for(int z = zz1; z <= zz2; z++) // 범위만큼 처리..

	return iSubCellCount; // 걸친 셀 포인터 돌려주기..
}

float CN3ShapeMgr::GetHeightNearstPos(const __Vector3& vPos, float fDist, __Vector3* pvNormal) // 가장 가까운 높이값을 돌려준다. 없으면 -FLT_MAX 을 돌려준다.
{
	__CellSub* pCell = SubCell(vPos.x, vPos.z); // 서브셀을 가져온다..

	// 없음 말자.
	if (pCell == nullptr
		|| pCell->nCCPolyCount <= 0)
		return -FLT_MAX;

	// 꼭대기에 위치를 하고..
	__Vector3 vPosV = vPos;
	vPosV.y = 5000.0f;

	__Vector3 vDir(0, -1, 0); // 수직 방향 벡터
	__Vector3 vColTmp(0, 0, 0); // 최종적으로 가장 가까운 충돌 위치..

	int nIndex0, nIndex1, nIndex2;
	float fT, fU, fV;
	float fNearst = FLT_MAX, fHeight = -FLT_MAX;		// 일단 최소값을 큰값으로 잡고..

	for (int i = 0; i < pCell->nCCPolyCount; i++)
	{
		nIndex0 = pCell->pdwCCVertIndices[i * 3];
		nIndex1 = pCell->pdwCCVertIndices[i * 3 + 1];
		nIndex2 = pCell->pdwCCVertIndices[i * 3 + 2];

		// 충돌된 점이 있으면..
		if (!::_IntersectTriangle(vPosV, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp))
			continue;

		float fMinTmp = (vColTmp - vPos).Magnitude();

		// 가장 가까운 충돌 위치를 찾기 위한 코드..
		if (fMinTmp < fNearst)
		{
			fNearst = fMinTmp;
			fHeight = vColTmp.y; // 높이값.

			if (pvNormal != nullptr)
			{
				pvNormal->Cross(
					m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0],
					m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
				pvNormal->Normalize();
			}
		}
	}

	return fHeight;
}

// 가장 가까운 높이값을 돌려준다. 없으면 -FLT_MAX 을 돌려준다.
float CN3ShapeMgr::GetHeightNearstPos(const __Vector3& vPos, __Vector3* pvNormal)
{
	__CellSub* pCell = SubCell(vPos.x, vPos.z); // 서브셀을 가져온다..

	// 없음 말자.
	if (pCell == nullptr
		|| pCell->nCCPolyCount <= 0)
		return -FLT_MAX;

	// 꼭대기에 위치를 하고..
	__Vector3 vPosV = vPos;
	vPosV.y = 5000.0f;

	__Vector3 vDir(0, -1, 0); // 수직 방향 벡터
	__Vector3 vColTmp(0, 0, 0); // 최종적으로 가장 가까운 충돌 위치..

	int nIndex0, nIndex1, nIndex2;
	float fT, fU, fV;
	float fNearst = FLT_MAX, fHeight = -FLT_MAX;		// 일단 최소값을 큰값으로 잡고..

	for (int i = 0; i < pCell->nCCPolyCount; i++)
	{
		nIndex0 = pCell->pdwCCVertIndices[i * 3];
		nIndex1 = pCell->pdwCCVertIndices[i * 3 + 1];
		nIndex2 = pCell->pdwCCVertIndices[i * 3 + 2];

		// 충돌된 점이 있으면..
		if (!::_IntersectTriangle(vPosV, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp))
			continue;

		float fMinTmp = (vColTmp - vPos).Magnitude();

		// 가장 가까운 충돌 위치를 찾기 위한 코드..
		if (fMinTmp < fNearst)
		{
			fNearst = fMinTmp;
			fHeight = vColTmp.y; // 높이값.

			if (pvNormal != nullptr)
			{
				pvNormal->Cross(
					m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0],
					m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
				pvNormal->Normalize();
			}
		}
	}

	return fHeight;
}

float CN3ShapeMgr::GetHeight(float fX, float fZ, __Vector3* pvNormal) // 가장 높은 곳을 돌려준다.. 없으면 -FLT_MAX값을 돌려준다.
{
	__CellSub* pCell = SubCell(fX, fZ); // 서브셀을 가져온다..

	// 없음 말자.
	if (pCell == nullptr
		|| pCell->nCCPolyCount <= 0)
		return -FLT_MAX;

	__Vector3 vPosV(fX, 5000.0f, fZ); // 꼭대기에 위치를 하고..
	__Vector3 vDir(0, -1, 0); // 수직 방향 벡터
	__Vector3 vColTmp(0, 0, 0); // 최종적으로 가장 가까운 충돌 위치..

	float fT, fU, fV;
	float fMaxTmp = -FLT_MAX;

	for (int i = 0; i < pCell->nCCPolyCount; i++)
	{
		int nIndex0 = pCell->pdwCCVertIndices[i * 3];
		int nIndex1 = pCell->pdwCCVertIndices[i * 3 + 1];
		int nIndex2 = pCell->pdwCCVertIndices[i * 3 + 2];

		// 충돌된 점이 있으면..
		if (!::_IntersectTriangle(vPosV, vDir, m_pvCollisions[nIndex0], m_pvCollisions[nIndex1], m_pvCollisions[nIndex2], fT, fU, fV, &vColTmp))
			continue;

		if (vColTmp.y > fMaxTmp)
		{
			fMaxTmp = vColTmp.y;

			if (pvNormal != nullptr)
			{
				pvNormal->Cross(
					m_pvCollisions[nIndex1] - m_pvCollisions[nIndex0],
					m_pvCollisions[nIndex2] - m_pvCollisions[nIndex0]);
				pvNormal->Normalize();
			}
		}
	}

	return fMaxTmp;
}

// 해당 위치의 셀 포인터를 돌려준다.
void CN3ShapeMgr::SubCell(const __Vector3& vPos, __CellSub** ppSubCell)
{
	int x = (int) (vPos.x / CELL_MAIN_SIZE);
	int z = (int) (vPos.z / CELL_MAIN_SIZE);

	__ASSERT(x >= 0 && x < MAX_CELL_MAIN && z >= 0 && z < MAX_CELL_MAIN, "Invalid cell number");

	int xx = (((int) vPos.x) % CELL_MAIN_SIZE) / CELL_SUB_SIZE;
	int zz = (((int) vPos.z) % CELL_MAIN_SIZE) / CELL_SUB_SIZE;

	// 2, 3, 4
	// 1, 0, 5
	// 8, 7, 6
	for (int i = 0; i < 9; i++)
	{
		switch (i)
		{
			case 0:
				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx][zz];
				else
					ppSubCell[i] = nullptr;
				break;

			case 1:
				if ((x == 0)
					&& (xx == 0))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if ((x != 0)
					&& (xx == 0))
				{
					if (m_pCells[x - 1][z] != nullptr)
						ppSubCell[i] = &m_pCells[x - 1][z]->SubCells[CELL_MAIN_DIVIDE - 1][zz];
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx - 1][zz];
				else
					ppSubCell[i] = nullptr;
				break;

			case 2:
				if (x == 0
					&& xx == 0)
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (z == (CELL_MAIN_SIZE - 1)
					&& zz == (CELL_MAIN_DIVIDE - 1))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				// x 감소, z 증가.
				if (x != 0
					&& xx == 0)
				{
					if ((z != (MAX_CELL_MAIN - 1))
						&& (zz == (CELL_MAIN_DIVIDE - 1)))
					{
						if (m_pCells[x - 1][z + 1] != nullptr)
							ppSubCell[i] = &m_pCells[x - 1][z + 1]->SubCells[CELL_MAIN_DIVIDE - 1][0];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x - 1][z] != nullptr)
							ppSubCell[i] = &m_pCells[x - 1][z]->SubCells[CELL_MAIN_DIVIDE - 1][zz + 1];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				// x 감소, z 증가.
				if (z != (MAX_CELL_MAIN - 1)
					&& zz == (CELL_MAIN_DIVIDE - 1))
				{
					if (x != 0
						&& xx == 0)
					{
						if (m_pCells[x - 1][z + 1] != nullptr)
							ppSubCell[i] = &m_pCells[x - 1][z + 1]->SubCells[CELL_MAIN_DIVIDE - 1][0];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x][z + 1] != nullptr)
							ppSubCell[i] = &m_pCells[x][z + 1]->SubCells[xx - 1][0];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx - 1][zz + 1];
				else
					ppSubCell[i] = nullptr;
				break;

			// z 증가.
			case 3:
				if (z == (MAX_CELL_MAIN - 1)
					&& zz == (CELL_MAIN_DIVIDE - 1))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (z != (MAX_CELL_MAIN - 1)
					&& zz == (CELL_MAIN_DIVIDE - 1))
				{
					if (m_pCells[x - 1][z] != nullptr)
						ppSubCell[i] = &m_pCells[x - 1][z]->SubCells[xx][0];
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx][zz + 1];
				else
					ppSubCell[i] = nullptr;
				break;

			// x 증가, z 증가.
			case 4:
				if (x == (MAX_CELL_MAIN - 1)
					&& xx == (CELL_MAIN_DIVIDE - 1))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (z == (MAX_CELL_MAIN - 1)
					&& zz == (CELL_MAIN_DIVIDE - 1))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (x != (MAX_CELL_MAIN - 1)
					&& xx == (CELL_MAIN_DIVIDE - 1))
				{
					if (z != (MAX_CELL_MAIN - 1)
						&& zz == (CELL_MAIN_DIVIDE - 1))
					{
						if (m_pCells[x + 1][z + 1] != nullptr)
							ppSubCell[i] = &m_pCells[x + 1][z + 1]->SubCells[0][0];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x + 1][z] != nullptr)
							ppSubCell[i] = &m_pCells[x + 1][z]->SubCells[0][zz + 1];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (z != (MAX_CELL_MAIN - 1)
					&& zz == (CELL_MAIN_DIVIDE - 1))
				{
					if (x != (MAX_CELL_MAIN - 1)
						&& xx == (CELL_MAIN_DIVIDE - 1))
					{
						if (m_pCells[x + 1][z + 1] != nullptr)
							ppSubCell[i] = &m_pCells[x + 1][z + 1]->SubCells[0][0];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x][z + 1] != nullptr)
							ppSubCell[i] = &m_pCells[x][z + 1]->SubCells[xx + 1][0];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx + 1][zz + 1];
				else
					ppSubCell[i] = nullptr;
				break;

			// x 증가.
			case 5:
				if (x == (MAX_CELL_MAIN - 1)
					&& xx == (CELL_MAIN_DIVIDE - 1))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (x != (MAX_CELL_MAIN - 1)
					&& xx == (CELL_MAIN_DIVIDE - 1))
				{
					if (m_pCells[x + 1][z] != nullptr)
						ppSubCell[i] = &m_pCells[x + 1][z]->SubCells[0][zz];
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx + 1][zz];
				else
					ppSubCell[i] = nullptr;
				break;

			// x 증가. z 감소.
			case 6:
				if (x == (MAX_CELL_MAIN - 1)
					&& xx == (CELL_MAIN_DIVIDE - 1))
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (z == 0
					&& zz == 0)
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (x != (MAX_CELL_MAIN - 1)
					&& xx == (CELL_MAIN_DIVIDE - 1))
				{
					if (z != 0
						&& zz == 0)
					{
						if (m_pCells[x + 1][z - 1] != nullptr)
							ppSubCell[i] = &m_pCells[x + 1][z - 1]->SubCells[0][CELL_MAIN_DIVIDE - 1];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x + 1][z] != nullptr)
							ppSubCell[i] = &m_pCells[x + 1][z]->SubCells[0][zz - 1];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (z != 0
					&& zz == 0)
				{
					if (x != (CELL_MAIN_SIZE - 1)
						&& xx == (CELL_MAIN_DIVIDE - 1))
					{
						if (m_pCells[x + 1][z - 1] != nullptr)
							ppSubCell[i] = &m_pCells[x + 1][z - 1]->SubCells[0][CELL_MAIN_DIVIDE - 1];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x][z - 1] != nullptr)
							ppSubCell[i] = &m_pCells[x][z - 1]->SubCells[xx + 1][CELL_MAIN_DIVIDE - 1];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx + 1][zz - 1];
				else
					ppSubCell[i] = nullptr;
				break;

			// z 감소.
			case 7:
				if (z == 0
					&& zz == 0)
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (z != 0
					&& zz == 0)
				{
					if (m_pCells[x][z - 1] != nullptr)
						ppSubCell[i] = &m_pCells[x][z - 1]->SubCells[xx][CELL_MAIN_DIVIDE - 1];
					else
						ppSubCell[i] = nullptr;
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx][zz - 1];
				else
					ppSubCell[i] = nullptr;
				break;

			// x 감소, z 감소.
			case 8:
				if (x == 0
					&& xx == 0)
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (z == 0
					&& zz == 0)
				{
					ppSubCell[i] = nullptr;
					break;
				}

				if (x != 0
					&& xx == 0)
				{
					if (z != 0
						&& zz == 0)
					{
						if (m_pCells[x - 1][z - 1] != nullptr)
							ppSubCell[i] = &m_pCells[x - 1][z - 1]->SubCells[CELL_MAIN_DIVIDE - 1][CELL_MAIN_DIVIDE - 1];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x - 1][z] != nullptr)
							ppSubCell[i] = &m_pCells[x - 1][z]->SubCells[CELL_MAIN_DIVIDE - 1][zz - 1];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (z != 0
					&& zz == 0)
				{
					if (x != 0
						&& xx == 0)
					{
						if (m_pCells[x - 1][z - 1] != nullptr)
							ppSubCell[i] = &m_pCells[x - 1][z - 1]->SubCells[CELL_MAIN_DIVIDE - 1][CELL_MAIN_DIVIDE - 1];
						else
							ppSubCell[i] = nullptr;
					}
					else
					{
						if (m_pCells[x][z - 1] != nullptr)
							ppSubCell[i] = &m_pCells[x][z - 1]->SubCells[xx - 1][CELL_MAIN_DIVIDE - 1];
						else
							ppSubCell[i] = nullptr;
					}
					break;
				}

				if (m_pCells[x][z] != nullptr)
					ppSubCell[i] = &m_pCells[x][z]->SubCells[xx - 1][zz - 1];
				else
					ppSubCell[i] = nullptr;
				break;
		}	// switch
	}	// for 
}
