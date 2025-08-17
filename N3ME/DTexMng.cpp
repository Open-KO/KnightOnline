// DTexMng.cpp: implementation of the CDTexMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainFrm.h"
#include "n3me.h"
#include "LyTerrainDef.h"

#include "DTex.h"
#include <N3Base/N3Texture.h>
#include "DlgDTexGroupView.h"
#include "DTexMng.h"
#include "DTexGroupMng.h"
#include "ProgressBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDTexMng::CDTexMng()
{
	m_NextID = 0;
	m_pMainFrm = NULL;
}

CDTexMng::~CDTexMng()
{
	Release();
}


//
//	Release..
//
void CDTexMng::Release()
{
	it_DTex it = m_pDTex.begin();
	while(it!=m_pDTex.end())
	{
		CDTex* pTmpDTex = (*it);
		if(pTmpDTex)
		{
			pTmpDTex->Release();
			delete pTmpDTex;
		}
		it = m_pDTex.erase(it);
	}
	m_pDTex.clear();
}


//
//	Init..
//
void CDTexMng::Init(CMainFrame* pFrm)
{
	Release();

	m_NextID = 0;
	m_pMainFrm = pFrm;	
}


//
//
//
bool CDTexMng::AddDTex(CString FileName)
{
	if(IsInDTex(FileName)) return false;

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	char szModuleFilePath[_MAX_PATH];
	GetModuleFileName(NULL, szModuleFilePath, _MAX_PATH);

	char szNewPath[_MAX_PATH];
	_splitpath(szModuleFilePath, szDrive, szDir, NULL, NULL);
	_makepath(szNewPath, szDrive, szDir, NULL, NULL);

	std::string szNewFN = "dtex\\";
	szNewFN += FileName;

	CDTex* pDTex = new CDTex;
	pDTex->Init();
	pDTex->m_ID = m_NextID;

	std::string szOldPath = CN3Base::PathGet();
	pDTex->m_pTex->PathSet(szNewPath);
	pDTex->m_pTex->LoadFromFile(szNewFN);
	pDTex->m_pTex->PathSet(szOldPath);

	m_pDTex.push_back(pDTex);
	m_NextID++;

	return true;	
}


//
//
//
void CDTexMng::DelDTexByID(int id)
{
	it_DTex it;
	for(it = m_pDTex.begin(); it != m_pDTex.end(); it++)
	{
		CDTex* pDTex = (*it);
		if(pDTex->m_ID == id)
		{
			if((m_NextID-1)==id) m_NextID -= 1;

			pDTex->Release();
			delete pDTex;
			m_pDTex.erase(it);
			break;
		}
	}
}


//
//	Load..
//	DTex정보들과 실제 텍스쳐 소스들을 읽어들인다.
//
void CDTexMng::LoadFromFile(CString RealFileName)
{
	Init(m_pMainFrm);
	char szDTexInfoFileName[_MAX_PATH];
	wsprintf(szDTexInfoFileName, "%sDTEX\\%s.dtx", s_szPath.c_str(), (LPCTSTR)RealFileName);

	FILE* stream = fopen(szDTexInfoFileName, "r");
	if(!stream)
	{
		return;
	}

	int iCount;
	int result = fscanf(stream, "NumDTex = %d\n", &iCount);
	if(EOF == result) { MessageBox(::GetActiveWindow(), szDTexInfoFileName, "Invalid DTex Info File...", MB_OK); return; }

	m_NextID = 0;
	if(iCount>0)	// old version data 
	{
		CProgressBar ProgressBar;
		ProgressBar.Create("Load TileTex Info..", 50,  iCount);

		CDTexGroupMng* pDTexGroupMng = m_pMainFrm->GetDTexGroupMng();

		int i;
		char szDTexFileName[_MAX_PATH];
		DWORD dwRWC;
		for(i=0; i<iCount; i++)
		{
			result = fscanf(stream, "%s\n", szDTexFileName);
			__ASSERT(result != EOF, "Invalid DTex Info File...");

			ProgressBar.StepIt();

			//	실제 텍스쳐 소스를 읽고..
			CDTex* pDTex = new CDTex;
			pDTex->Init();
			pDTex->m_ID = i;
			pDTex->m_pTex->LoadFromFile(szDTexFileName);
			
			//	그에 관한 타일 정보들을 읽고..
			char szDir[_MAX_DIR], szFName[_MAX_FNAME];
			_splitpath(szDTexFileName, NULL, szDir, szFName, NULL);
			wsprintf(szDTexInfoFileName, "%s%s%s.dif", s_szPath.c_str(), szDir, szFName); // Texture Information file

			HANDLE hFile = CreateFile(szDTexInfoFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				for(int x=0; x<NUM_DTEXTILE; x++)
				{
					for(int y=0; y<NUM_DTEXTILE; y++)
					{
						ReadFile(hFile, &(pDTex->m_Attr[x][y]), sizeof(DTEXATTR), &dwRWC, NULL);
						DTEXTILEATTR tile;
						tile.TexID = i;
						tile.TileX = x;
						tile.TileY = y;
						pDTexGroupMng->SetTile(pDTex->m_Attr[x][y].Group, pDTex->m_Attr[x][y].Attr, tile);
					}
				}
			}
			CloseHandle(hFile);
			


			m_pDTex.push_back(pDTex);
		}	// end of for(i=0; i<iCount; i++)
		m_NextID = i;
	}	// end of if(iCount>0)

	if(iCount<0)	//new version...
	{
		int version;
		fscanf(stream, "Version = %d\n", &version);

		m_NextID = 0;
		iCount = -iCount;
		int id;

		CDTexGroupMng* pDTexGroupMng = m_pMainFrm->GetDTexGroupMng();
		int i;
		char szDTexFileName[_MAX_PATH];
		DWORD dwRWC;

		CProgressBar ProgressBar;
		ProgressBar.Create("Load TileTex Info..", 50,  iCount);

		for(i=0;i<iCount;i++)
		{
			ProgressBar.StepIt();

			result = fscanf(stream, "%s %d\n", szDTexFileName, &id);
			__ASSERT(result != EOF, "Invalid DTex Info File...");

			if(m_NextID <= id) m_NextID = id + 1;

			//	실제 텍스쳐 소스를 읽고..
			CDTex* pDTex = new CDTex;
			pDTex->Init();
			pDTex->m_ID = id;
			pDTex->m_pTex->LoadFromFile(szDTexFileName);

			if(version==2)
			{
				for(int y=0; y<NUM_DTEXTILE; y++)
				{
					int Group;
					fscanf(stream, "%d\n", &Group);
					for(int x=0; x<NUM_DTEXTILE; x++)
					{
						pDTex->m_Attr[x][y].Group = Group;
						pDTex->m_Attr[x][y].Attr = x;

						DTEXTILEATTR tile;
						tile.TexID = id;
						tile.TileX = x;
						tile.TileY = y;
						
						pDTexGroupMng->SetTile(pDTex->m_Attr[x][y].Group, pDTex->m_Attr[x][y].Attr, tile);
					}
				}
			}

			if(version==1)
			{
				//	그에 관한 타일 정보들을 읽고..
				char szDir[_MAX_DIR], szFName[_MAX_FNAME];
				_splitpath(szDTexFileName, NULL, szDir, szFName, NULL);
				wsprintf(szDTexInfoFileName, "%s%s%s.dif", s_szPath.c_str(), szDir, szFName); // Texture Information file

				HANDLE hFile = CreateFile(szDTexInfoFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if(hFile != INVALID_HANDLE_VALUE)
				{
					for(int x=0; x<NUM_DTEXTILE; x++)
					{
						for(int y=0; y<NUM_DTEXTILE; y++)
						{
							ReadFile(hFile, &(pDTex->m_Attr[x][y]), sizeof(DTEXATTR), &dwRWC, NULL);
							DTEXTILEATTR tile;
							tile.TexID = id;
							tile.TileX = x;
							tile.TileY = y;
							pDTexGroupMng->SetTile(pDTex->m_Attr[x][y].Group, pDTex->m_Attr[x][y].Attr, tile);
						}
					}
				}
				CloseHandle(hFile);
			}
			m_pDTex.push_back(pDTex);
		}
	}	//end of if(iCount<0)

	fclose(stream);
}


//
//
//
void CDTexMng::SaveToFile(CString RealFileName)
{
	char szDTexDir[_MAX_PATH];
	wsprintf(szDTexDir, "%sDTex", s_szPath.c_str());
	CreateDirectory("dtex", NULL); // 경로 만들고..

	char szDTexInfoFileName[_MAX_PATH];
	wsprintf(szDTexInfoFileName, "%sDTEX\\%s.dtx", s_szPath.c_str(), (LPCTSTR)RealFileName);

	FILE* stream = fopen(szDTexInfoFileName, "w");
	if(!stream)	return;

	int NumDTex = -static_cast<int>(m_pDTex.size());
	fprintf(stream, "NumDTex = %d\n", NumDTex);
	int version = N3ME_DTEX_DATA_VERSION;
	fprintf(stream, "Version = %d\n", version);

	CDTexGroupMng* pDTexGroupMng = m_pMainFrm->GetDTexGroupMng();

	int id;
	it_DTex it;
	char szDTexFileName[_MAX_PATH];
	//DWORD dwRWC;
	CDTex* pDTex;
	for(it=m_pDTex.begin(); it!=m_pDTex.end(); it++)
	{
		pDTex = (*it);
		sprintf(szDTexFileName, "%s", pDTex->m_pTex->FileName().c_str());
		id = pDTex->m_ID;

		fprintf(stream, "%s %d\n", szDTexFileName, id);

		for(int y=0; y<NUM_DTEXTILE; y++)
		{
			fprintf(stream, "%d\n", pDTex->m_Attr[0][y].Group);
		}

		/*
		//
		//	version1 저장방식...
		//	dif파일만들기...
		//
		char szDir[_MAX_DIR], szFName[_MAX_FNAME];
		
		_splitpath(szDTexFileName, NULL, szDir, szFName, NULL);
		wsprintf(szDTexInfoFileName, "%s%s%s.dif", s_szPath.c_str(), szDir, szFName); // Texture Information file

		//	그에 관한 타일 정보들을 읽고..
		HANDLE hFile = CreateFile(szDTexInfoFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			for(int x=0; x<NUM_DTEXTILE; x++)
			{
				for(int y=0; y<NUM_DTEXTILE; y++)
				{
					WriteFile(hFile, &(pDTex->m_Attr[x][y]), sizeof(DTEXATTR), &dwRWC, NULL);
				}
			}
		}
		CloseHandle(hFile);		
		*/
	}
	fclose(stream);
}


//
//	게임에서 쓸수 있는 타일 텍스쳐 포멧으로 변환후 저장..
//
void CDTexMng::SaveGameTile()
{
	D3DFORMAT	Format;
	int			Size = DTEX_SIZE / NUM_DTEXTILE;		//단위텍스쳐의 길이..
	D3DLOCKED_RECT d3dlr;

	HANDLE hFile;
	int ix, iz;
	int j;
	char* pSourceImg;
	char* pTargetImg;

	char szDTexDir[_MAX_PATH];
	wsprintf(szDTexDir, "%sDTex\\", s_szPath.c_str());
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME];
	_splitpath(szDTexDir, szDrive, szDir, NULL, NULL);

	CN3Texture* pTex;
	char szDTexGameFileName[_MAX_PATH];
	char szNewFName[_MAX_PATH];

	it_DTex it;
	//for(int i=0;i<MAX_TILETEXTURE;i++)
	for(it = m_pDTex.begin(); it != m_pDTex.end(); it++)
	{
		CDTex* pDTex = (*it);
		if(pDTex)
		{
			pTex = pDTex->m_pTex;
			if(NULL == pTex || NULL == pTex->Get())
			{
				MessageBox(::GetActiveWindow(), "Tile texture pointer is NULL!!!", "Save GameTile Data Error", MB_OK);
				continue;
			}

			//Source Info...
			Format = pTex->PixelFormat();
			pTex->Get()->LockRect( 0, &d3dlr, 0, 0 );
			int Bits = d3dlr.Pitch / DTEX_SIZE;

			CN3Texture TileTex;
			D3DLOCKED_RECT d3dlrTarget;
			for(iz=0; iz<NUM_DTEXTILE; iz++)
			{
				//file setting..
				_splitpath(pTex->FileName().c_str(), NULL, NULL, szFName, NULL);
				sprintf(szNewFName, "%s_%d",szFName, iz);

				_makepath(szDTexGameFileName, szDrive, szDir, szNewFName, ".gtt");
				hFile = CreateFile(szDTexGameFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				for(ix=0; ix<NUM_DTEXTILE; ix++)
				{
					//텍스쳐 서페이스 만들고, 텍스쳐 채우고, 형식 변환하고, 저장.
					TileTex.Create(Size, Size, Format, TRUE);
					TileTex.Get()->LockRect( 0, &d3dlrTarget, 0, 0 );
					pSourceImg = (char*)((char*)d3dlr.pBits + (ix*Size*Bits) + (iz*Size*d3dlr.Pitch));
					pTargetImg = (char*) d3dlrTarget.pBits;

					for(j=0;j<Size;j++)
					{
						memcpy( &(pTargetImg[j*Bits*Size]), &(pSourceImg[j* d3dlr.Pitch]), Bits*Size);
					}

					TileTex.Get()->UnlockRect(0);

					TileTex.Convert(D3DFMT_DXT1);
					TileTex.GenerateMipMap();
					TileTex.Save(hFile);
				}				
				CloseHandle(hFile);
			}			
			pTex->Get()->UnlockRect(0);
		}//endof if(pDTex)
	}
}


//
//
//
CDTex* CDTexMng::GetDTexByID(int id)
{
	it_DTex it;
	for(it = m_pDTex.begin(); it != m_pDTex.end(); it++)
	{
		if((*it)->m_ID == id) return (*it);
	}
	return NULL;
}


//
//
//
CDTex* CDTexMng::GetDTexByName(CString FileName)
{
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	char szModuleFilePath[_MAX_PATH];
	GetModuleFileName(NULL, szModuleFilePath, _MAX_PATH);

	char szNewPath[_MAX_PATH];
	_splitpath(szModuleFilePath, szDrive, szDir, NULL, NULL);
	_makepath(szNewPath, szDrive, szDir, NULL, NULL);

	CString NewFileName = "dtex\\";
	
	NewFileName += FileName;

	CString DTexName;
	CDTex* pDTex;
	
	it_DTex it;
	for(it = m_pDTex.begin(); it != m_pDTex.end(); it++)
	{
		pDTex = (*it);
		DTexName = pDTex->m_pTex->FileName().c_str();
		
		if(DTexName==NewFileName) 
			return pDTex;
	}
	return NULL;
}


//
//
//
bool CDTexMng::IsInDTex(CString FileName)
{
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	char szModuleFilePath[_MAX_PATH];
	GetModuleFileName(NULL, szModuleFilePath, _MAX_PATH);

	char szNewPath[_MAX_PATH];
	_splitpath(szModuleFilePath, szDrive, szDir, NULL, NULL);
	_makepath(szNewPath, szDrive, szDir, NULL, NULL);

	CString NewFileName = "dtex\\";
	NewFileName += FileName;

	CString DTexName;
	CDTex* pDTex;

	it_DTex it;
	for(it = m_pDTex.begin(); it != m_pDTex.end(); it++)
	{
		pDTex = (*it);
		DTexName = pDTex->m_pTex->FileName().c_str();

		if(DTexName==NewFileName) return true;
	}
	return false;
}
