// SowSeedMng.cpp: implementation of the CSowSeedMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainFrm.h"
#include "MapMng.h"
#include <N3Base/N3Shape.h>

#include "SowSeedMng.h"
#include "DlgSowSeed.h"
#include "DlgShapeList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSowSeedMng::CSowSeedMng()
{
	Grass_Group.clear();
	bActive = FALSE;
	ptSelHeightMapPos.x = 0;
	ptSelHeightMapPos.y = 0;
	Grass = nullptr;
	Grass_Count = 0;
	Render_Grass = FALSE;
	Select_Group_Id = -1;
	Add_List_Count = 0;

	m_pVertices = new __VertexColor[8];
};
CSowSeedMng::~CSowSeedMng()
{
	Release();
};

void CSowSeedMng::Release()
{
	it_Grass_Group it = Grass_Group.begin();
	int i;
	for (i = 0; i < static_cast<int>(Grass_Group.size()); i++, it++)
	{
		LPGRASS_GROUP group = (LPGRASS_GROUP)*it;
		it_Grass it_grass = group->grass.begin();
		for (int j = 0; j < static_cast<int>(group->grass.size()); j++, it_grass++)
		{
			LPGRASS grass = *it_grass;
			if( grass != nullptr)
				delete[] grass;
		}
		group->grass.clear();
		if( group != nullptr)
			delete group;
	}
	Grass_Group.clear();


	if( Grass != nullptr)
	{
		for( i = 0; i <Grass_Count ; i ++)
		{
				if(Grass[i].Sub_Grass != nullptr)
					delete Grass[i].Sub_Grass;
		}
		delete Grass;
	}

	Obj_Name.clear();


};

void CSowSeedMng::SetActive(BOOL flage)
{
	bActive = flage;
};
BOOL CSowSeedMng::MouseMessage(LPMSG pMsg)
{
	if( bActive == FALSE)
		return FALSE;

	static int	iPrevScreenY = 0;
	const float fDelta = 0.10f;
	static int	iSumOfEditedHeight=0;	// 이번 드래그로 변화된 지형높이의 합
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	switch(pMsg->message)
	{
	case WM_MOUSEMOVE:
		{
			DWORD_PTR nFlags = pMsg->wParam;
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			if( pFrame->m_pDlgSowSeed->Sow_Select_Flage == CS_SOW)
			{
				// 브러시 업데이트 
				if(pFrame->GetMapMng()->GetTerrain()->Pick(point.x, point.y, nullptr, &ptSelHeightMapPos))
					pFrame->GetMapMng()->GetTerrain()->UpdateBrushArea(ptSelHeightMapPos);
			}
			else
			if( pFrame->m_pDlgSowSeed->Sow_Select_Flage == CS_SELECT)
			{
			}

		}
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			if( pFrame->m_pDlgSowSeed->Sow_Select_Flage == CS_SOW)
			{
				Render_Grass = FALSE;
				Add_Grass();
			}
			else
			if( pFrame->m_pDlgSowSeed->Sow_Select_Flage == CS_SELECT)
			{
				Select_Group_Id = SelectSeed(point);
				if( Select_Group_Id >= 0)
				{
					SetListPos();
				}

			}
		}
		return TRUE;
	case WM_LBUTTONUP:
		{
		}
		return TRUE;
	}
	return TRUE;
};

void CSowSeedMng::Create_Grass(int Num)
{
	if( Grass != nullptr)
		delete[] Grass;

	Grass = (LPGRASS) new GRASS[Num];
	Grass_Count = Num;

	for( int i = 0 ; i < Grass_Count ;i++)
	{
		Grass[i].Tile_x = -1;
		Grass[i].Tile_z = -1;
		Grass[i].Pos = __Vector3(0.0f,0.0f,0.0f);
		Grass[i].Sub_Grass = nullptr;
		Grass[i].Sub_Grass_Count = 0;
	}
};

void CSowSeedMng::Random_Grass(void)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	for (int i = 0,Num = 0 ; i < MAX_BRUSH_SIZE ; ++i)
	{
		for (int j = 0 ; j < MAX_BRUSH_SIZE ; ++j)
		{
			if(pFrame->GetMapMng()->GetTerrain()->m_fBrushIntensityMap[i][j] <= 0) 
				continue;

//			if(pFrame->m_pDlgSowSeed->Sow_Array[i][j] == 0)
//				continue;

//			if( (pFrame->m_pDlgSowSeed->Sow_Array[i][j] == 2) || (pFrame->m_pDlgSowSeed->Sow_Array[i][j] == 1))
			{
				if( Grass[Num].Sub_Grass != nullptr)
				{
					delete[] Grass[Num].Sub_Grass;
					Grass[Num].Sub_Grass = nullptr;
				}

				Grass[Num].Sub_Grass_Count = rand() % pFrame->m_pDlgSowSeed->SowDetail;
				Grass[Num].Sub_Grass = (LPGRASS) new GRASS[Grass[Num].Sub_Grass_Count];
				for( int k = 0 ; k < Grass[Num].Sub_Grass_Count ; k ++)
				{
					int rad = rand() % 4;
					if( rad == 0)
					{
						Grass[Num].Sub_Grass[k].Detx = (rand()%20)*0.1f;
						Grass[Num].Sub_Grass[k].Detz = (rand()%20)*0.1f;
						Grass[Num].roll = rand() % 360;
					}
					else
					if( rad == 1)
					{
						Grass[Num].Sub_Grass[k].Detx =-(rand()%20)*0.1f;
						Grass[Num].Sub_Grass[k].Detz = (rand()%20)*0.1f;
						Grass[Num].roll = rand() % 360;
					}
					else
					if( rad == 2)
					{
						Grass[Num].Sub_Grass[k].Detx =-(rand()%20)*0.1f;
						Grass[Num].Sub_Grass[k].Detz =-(rand()%20)*0.1f;
						Grass[Num].roll = rand() % 360;
					}
					else
					if( rad == 3)
					{
						Grass[Num].Sub_Grass[k].Detx = (rand()%20)*0.1f;
						Grass[Num].Sub_Grass[k].Detz =-(rand()%20)*0.1f;
						Grass[Num].roll = rand() % 360;
					}

					Grass[Num].Sub_Grass[k].Sub_Grass = nullptr;
				}
				Num++;
			}

		}
	}
	Render_Grass = TRUE;
};

void CSowSeedMng::Render(LPDIRECT3DDEVICE9 lpD3DDevice)
{
	if( bActive == FALSE)
	{
		return;
	}
	int i,j,Num=0;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();


	if( pFrame->m_pDlgSowSeed->Sow_Select_Flage == CS_SOW)
	{
		// 브러시에 선택된 풀 
		if( Render_Grass == TRUE)
		{
			pFrame->GetMapMng()->GetTerrain()->RenderBrushArea();
			for (i = 0,Num = 0 ; i < MAX_BRUSH_SIZE ; ++i)
			{
				for (j = 0 ; j < MAX_BRUSH_SIZE ; ++j)
				{
					if(pFrame->GetMapMng()->GetTerrain()->m_fBrushIntensityMap[i][j] <= 0) 
						continue;

	//				if(pFrame->m_pDlgSowSeed->Sow_Array[i][j] == 0)
	//					continue;

	//				if( (pFrame->m_pDlgSowSeed->Sow_Array[i][j] == 2) || (pFrame->m_pDlgSowSeed->Sow_Array[i][j] == 1) )
					{
						if( Grass != nullptr)
						{
							Grass[Num].Tile_x = ptSelHeightMapPos.x - BRUSH_CENTER + i;
							Grass[Num].Tile_z = ptSelHeightMapPos.y - BRUSH_CENTER + j;

							if (Grass[Num].Tile_x < 0 || Grass[Num].Tile_z < 0 ) continue;

							float pos_x = (float)(Grass[Num].Tile_x*TERRAIN_CELL_SIZE);
							float pos_z = (float)(Grass[Num].Tile_z*TERRAIN_CELL_SIZE);


							Grass[Num].Pos.Set(pos_x,
								pFrame->GetMapMng()->GetTerrain()->m_ppMapData[Grass[Num].Tile_x][Grass[Num].Tile_z].fHeight,
								pos_z);

							if( ((CN3Shape*)pFrame->GetMapMng()->GetSelSourceObj()))
							{
								((CN3Shape*)pFrame->GetMapMng()->GetSelSourceObj())->PosSet(Grass[Num].Pos);
								((CN3Shape*)pFrame->GetMapMng()->GetSelSourceObj())->Render();
							}

							if( Grass[Num].Sub_Grass != nullptr)
							{
								for( int k = 0 ; k < Grass[Num].Sub_Grass_Count ; k ++)
								{
									Grass[Num].Sub_Grass[k].Tile_x = ptSelHeightMapPos.x - BRUSH_CENTER + i;
									Grass[Num].Sub_Grass[k].Tile_z = ptSelHeightMapPos.y - BRUSH_CENTER + j;

									Grass[Num].Sub_Grass[k].Pos.Set(pos_x+Grass[Num].Sub_Grass[k].Detx,
										pFrame->GetMapMng()->GetTerrain()->m_ppMapData[Grass[Num].Tile_x][Grass[Num].Tile_z].fHeight,
										pos_z+Grass[Num].Sub_Grass[k].Detz);


									if( ((CN3Shape*)pFrame->GetMapMng()->GetSelSourceObj()))
									{
										((CN3Shape*)pFrame->GetMapMng()->GetSelSourceObj())->PosSet(Grass[Num].Sub_Grass[k].Pos);
										((CN3Shape*)pFrame->GetMapMng()->GetSelSourceObj())->Render();
									}
								}
							}
							Num ++;
						}
					}
				}
			} 
		}
	}

	// 맵에 찍힌 풀 그리기 
	if( Render_Grass == TRUE)
	{
		it_Grass_Group it = Grass_Group.begin();
		for (i = 0; i < static_cast<int>(Grass_Group.size()); i++, it++)
		{
			LPGRASS_GROUP group = *it;
			it_Grass it_grass = group->grass.begin();

			if( Select_Group_Id != i)
			{
				for (j = 0; j < static_cast<int>(group->grass.size()); j++, it_grass++)
				{
					LPGRASS grass = *it_grass;

					CN3Shape* pShape = (CN3Shape*)pFrame->GetMapMng()->m_pDlgSourceList->m_ListShape.GetItemDataPtr(group->Obj_ID);
					if( (CN3Shape*)pShape )
					{
						pShape->PosSet(grass->Pos);
						pShape->Tick();
						if( pShape->m_bDontRender == false)
						{
							pShape->Render();
						}
					}
				}
			}
			else
			if( Select_Group_Id == i )  // 선택된 그룹 
			{
				Render_Box(lpD3DDevice,group->Pos);
				for (j = 0; j < static_cast<int>(group->grass.size()); j++, it_grass++)
				{
					LPGRASS grass = *it_grass;

					CN3Shape* pShape = (CN3Shape*)pFrame->GetMapMng()->m_pDlgSourceList->m_ListShape.GetItemDataPtr(group->Obj_ID);
					if( (CN3Shape*)pShape )
					{
						pShape->PosSet(grass->Pos);
						pShape->Tick();
						if( pShape->m_bDontRender == false)
						{
							pShape->Render();
						}
					}
				}
			}
		}
	}

};

void CSowSeedMng::Add_Grass(void)
{
	if( Grass != nullptr)
	{

		__Vector3 Max = __Vector3(-99999.0f,-999999.0f,-999999.0f);
		__Vector3 Min = __Vector3( 99999.0f, 999999.0f, 999999.0f);
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->m_pDlgSowSeed->m_CB_Seed.ResetContent();

		bool add_list_group = false;
		bool add_list = false;

		int Grass_ID = Add_List_Count;
		LPGRASS_GROUP temp = new GRASS_GROUP;

		temp->grass.clear();
		temp->Group_id = Grass_ID  ;
		temp->Obj_ID = pFrame->m_pDlgSowSeed->Obj_Index;
		temp->b_size = pFrame->m_pDlgSowSeed->Brush_Size;

		char Name[256];

		CN3Shape* pShape = (CN3Shape*)pFrame->GetMapMng()->m_pDlgSourceList->m_ListShape.GetItemDataPtr(temp->Obj_ID);
		strcpy(temp->FileName,pShape->FileName().c_str());

//		temp->Pos = Grass[0].Pos;
		int i; int listCount;
		for(i = 0,listCount = 0; i < Grass_Count; i++)
		{
			LPGRASS grass = new GRASS;
			grass->Sub_Grass = nullptr;
			grass->Tile_x = Grass[i].Tile_x;
			grass->Tile_z = Grass[i].Tile_z;
			grass->Pos = Grass[i].Pos;

			if( Max.x < grass->Pos.x) Max.x = grass->Pos.x;
			if( Max.y < grass->Pos.y) Max.y = grass->Pos.y;
			if( Max.z < grass->Pos.z) Max.z = grass->Pos.z;

			if( Min.x > grass->Pos.x) Min.x = grass->Pos.x;
			if( Min.y > grass->Pos.y) Min.y = grass->Pos.y;
			if( Min.z > grass->Pos.z) Min.z = grass->Pos.z;

			add_list_group = true;
			add_list = true;

			it_Grass_Group it = Grass_Group.begin();
			for (int ii = 0; ii < static_cast<int>(Grass_Group.size()); ii++, it++)
			{
				LPGRASS_GROUP group_list = *it;
				it_Grass it_grass = group_list->grass.begin();
				for (int jj = 0; jj < static_cast<int>(group_list->grass.size()); jj++, it_grass++)
				{
					LPGRASS grass_list = *it_grass;
					if( grass->Pos == grass_list->Pos)
					{
						add_list_group = false;
						break;
					}
				}
			}


			if( (add_list_group == true) && (grass->Tile_x > 0) && (grass->Tile_z > 0) )
			{
				temp->grass.push_back(grass);

				char text[256];
				sprintf(text,"SubID:%d, x :%f, y :%f, z :%f",listCount,grass->Pos.x,grass->Pos.y,grass->Pos.z);
				pFrame->m_pDlgSowSeed->m_CB_Seed.AddString(text);
				listCount++;
			}

			if( Grass[i].Sub_Grass != nullptr)
			{
				for( int j = 0; j < Grass[i].Sub_Grass_Count ; j++)
				{
					LPGRASS grass_sub = new GRASS;
					grass_sub->Sub_Grass = nullptr;
					grass_sub->Tile_x = Grass[i].Sub_Grass[j].Tile_x;
					grass_sub->Tile_z = Grass[i].Sub_Grass[j].Tile_z;
					grass_sub->Pos = Grass[i].Sub_Grass[j].Pos;
					grass_sub->Detx = Grass[i].Sub_Grass[j].Detx;
					grass_sub->Detz = Grass[i].Sub_Grass[j].Detz;

					if( Max.x < grass_sub->Pos.x) Max.x = grass_sub->Pos.x;
					if( Max.y < grass_sub->Pos.y) Max.y = grass_sub->Pos.y;
					if( Max.z < grass_sub->Pos.z) Max.z = grass_sub->Pos.z;

					if( Min.x > grass_sub->Pos.x) Min.x = grass_sub->Pos.x;
					if( Min.y > grass_sub->Pos.y) Min.y = grass_sub->Pos.y;
					if( Min.z > grass_sub->Pos.z) Min.z = grass_sub->Pos.z;


					add_list = true;
					it_Grass_Group it = Grass_Group.begin();
					for (int ii = 0; ii < static_cast<int>(Grass_Group.size()); ii++, it++)
					{
						LPGRASS_GROUP group_list = *it;
						it_Grass it_grass = group_list->grass.begin();
						for (int jj = 0; jj < static_cast<int>(group_list->grass.size()); jj++, it_grass++)
						{
							LPGRASS grass_list = *it_grass;
							if(grass_sub->Pos == grass_list->Pos)
							{
								add_list = false;
								break;
							}
						}
					}

					if( (add_list == true) && (grass_sub->Tile_x > 0) && ( grass_sub->Tile_z > 0) )
					{
						temp->grass.push_back(grass_sub);
						char text[256];
						sprintf(text,"SubID:%d, x :%f, y :%f, z :%f",listCount,grass_sub->Pos.x,grass_sub->Pos.y,grass_sub->Pos.z);
						pFrame->m_pDlgSowSeed->m_CB_Seed.AddString(text);
						listCount++;
					}
				}
			}
			int CurPos = pFrame->m_pDlgSowSeed->m_CB_Seed.GetCount();
			pFrame->m_pDlgSowSeed->m_CB_Seed.SetCurSel(CurPos);
		}

		if( (add_list == true) || (add_list_group == true))
		{
			__Vector3 mid = (Max + Min) /2;
			temp->Pos.Set(mid.x,mid.y,mid.z); 
			Grass_Group.push_back(temp);
		}
		

		int Count = 0;
		for( i = 0 ; i < MAX_BRUSH_SIZE ; i++)
		{
			for( int j = 0 ; j < MAX_BRUSH_SIZE ;j++)
			{
				if( pFrame->GetMapMng()->GetTerrain()->m_fBrushIntensityMap[i][j] <= 0)
					continue;

				pFrame->m_pDlgSowSeed->Sow_Array[i][j] = rand()%3;
				Count ++;
			}
		}

		Create_Grass(Count);
		Random_Grass();
		Render_Grass = TRUE;

		pFrame->m_pDlgSowSeed->m_CB_TileGroup.GetLBText(0,Name);
		if( strcmp(Name,"NONE") == 0)
			pFrame->m_pDlgSowSeed->m_CB_TileGroup.DeleteString(0);

		pFrame->GetMapMng()->m_pDlgSourceList->m_ListShape.GetText(temp->Obj_ID,Name);

		char text[256];
		sprintf(text,"풀 ID: %d , 파일명:%s", Grass_ID,Name);
		int CurPos = pFrame->m_pDlgSowSeed->m_CB_TileGroup.GetCount();

		pFrame->m_pDlgSowSeed->m_CB_TileGroup.AddString(text);
		pFrame->m_pDlgSowSeed->m_CB_TileGroup.SetCurSel(CurPos);

		Add_List_Count ++;
	}
};

int CSowSeedMng::SelectSeed(POINT point)
{
	__Vector3 vPos;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	if (pFrame->GetMapMng()->GetTerrain()->Pick(point.x, point.y, &vPos))
	{
		it_Grass_Group it = Grass_Group.begin();
		for (int i = 0; i < static_cast<int>(Grass_Group.size()); i++, it++)
		{
			LPGRASS_GROUP group = *it;
			it_Grass it_grass = group->grass.begin();
			for (int j = 0; j < static_cast<int>(group->grass.size()); j++, it_grass++)
			{
				LPGRASS grass = *it_grass;

				CN3Shape* pShape = (CN3Shape*)pFrame->GetMapMng()->m_pDlgSourceList->m_ListShape.GetItemDataPtr(group->Obj_ID);
				if( (CN3Shape*)pShape )
				{
					pShape->PosSet(grass->Pos);
					pShape->Tick();
					if( pShape->m_bDontRender == false)
					{
						int check_id = pShape->CheckCollisionPrecisely(true,point.x,point.y);
						if( check_id >= 0)
						{
							CreateBox(__Vector3(group->b_size*TERRAIN_CELL_SIZE/2,1.0f,group->b_size*TERRAIN_CELL_SIZE/2),
								__Vector3(-(group->b_size*TERRAIN_CELL_SIZE/2),0.0f,-(group->b_size*TERRAIN_CELL_SIZE/2)));
							return i;
						}
					}
				}
			}
		}
	}
	return -1;
};

void CSowSeedMng::SetListPos(void)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (Grass_Group.size() > static_cast<size_t>(Select_Group_Id))
	{
		pFrame->m_pDlgSowSeed->m_CB_Seed.ResetContent();

		it_Grass_Group it = Grass_Group.begin();
		for (int i = 0, listCount = 0; i < static_cast<int>(Grass_Group.size()); i++, it++)
		{
			if( i == Select_Group_Id)
			{
				LPGRASS_GROUP group = *it;
				it_Grass it_grass = group->grass.begin();
				for (int j = 0; j < static_cast<int>(group->grass.size()); j++, it_grass++)
				{
					LPGRASS grass = *it_grass;
					char text[256];
					sprintf(text,"SubID:%d, x :%f, y :%f, z :%f",listCount,grass->Pos.x,grass->Pos.y,grass->Pos.z);
					pFrame->m_pDlgSowSeed->m_CB_Seed.AddString(text);
					listCount++;
				}
				pFrame->m_pDlgSowSeed->m_CB_TileGroup.SetCurSel(Select_Group_Id);
				return ;
			}
		}



		pFrame->m_pDlgSowSeed->m_CB_TileGroup.SetCurSel(Select_Group_Id);
	}
		
};

void CSowSeedMng::CreateBox(__Vector3 vMax,__Vector3 vMin)
{
	m_pVertices[0].Set(vMin.x, vMax.y, vMin.z,0xffff0000); 
	m_pVertices[1].Set(vMax.x, vMax.y, vMin.z,0xffff0000); 
	m_pVertices[2].Set(vMax.x, vMin.y, vMin.z,0xffff0000); 
	m_pVertices[3].Set(vMin.x, vMin.y, vMin.z,0xffff0000); 
	m_pVertices[4].Set(vMax.x, vMax.y, vMax.z,0xffff0000); 
	m_pVertices[5].Set(vMin.x, vMax.y, vMax.z,0xffff0000); 
	m_pVertices[6].Set(vMin.x, vMin.y, vMax.z,0xffff0000); 
	m_pVertices[7].Set(vMax.x, vMin.y, vMax.z,0xffff0000); 

};

void CSowSeedMng::Render_Box(LPDIRECT3DDEVICE9 lpD3DDevice,__Vector3 Pos)
{

	WORD m_Index[26] = {0,1 ,1,4 ,4,5 ,5,0, 3,2, 2,7, 7,6, 6,3, 1,2 ,0,3,
						1,2 ,4,7 ,5,6};
	DWORD dwLight;
	lpD3DDevice->GetRenderState(D3DRS_LIGHTING, &dwLight);
	if(dwLight)
		lpD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	__Matrix44 WorldMtx;
	WorldMtx.Identity();
	WorldMtx.PosSet(Pos);
	lpD3DDevice->SetTransform(D3DTS_WORLD, &WorldMtx);

	lpD3DDevice->SetTexture(0, nullptr);
 	lpD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST,0,8,13,m_Index,D3DFMT_INDEX16,m_pVertices,sizeof(__VertexColor));

	if(dwLight)
		lpD3DDevice->SetRenderState(D3DRS_LIGHTING, dwLight);

}
void CSowSeedMng::SaveData(void)
{
	// Seed List 읽어 오기..
	DWORD dwFlags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(FALSE, "tgi", nullptr, dwFlags, "Grass Info File(*.tgi)|*.tgi||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	int size = static_cast<int>(Grass_Group.size());
	if (size > 0)
	{
		FILE* fp = fopen((LPCTSTR) dlg.GetPathName(), "w");
	// 그룹의 총겟수 
		fwrite(&size, sizeof(int), 1, fp);
		it_Grass_Group it = Grass_Group.begin();
		for (int i = 0; i < size; i++, it++)
		{
			LPGRASS_GROUP group = *it;
			// 브러시 크기 
			fwrite(&group->b_size, sizeof(group->b_size), 1, fp);
			// 그룹 아이디
			fwrite(&group->Group_id, sizeof(group->Group_id), 1, fp);
			// 오브젝트 아이디
			fwrite(&group->Obj_ID, sizeof(group->Obj_ID), 1, fp);
			// 그룹 위치 
			fwrite(group->Pos, sizeof(group->Pos), 1, fp);
			// 서브 그룹 크기 
			int grass_size = static_cast<int>(group->grass.size());
			fwrite(&grass_size, sizeof(grass_size), 1, fp);

			// 파일명 쓰기 
			int len = static_cast<int>(strlen(group->FileName));
			fwrite(&len, sizeof(int), 1, fp);
			fwrite(group->FileName, len, 1, fp);


			it_Grass it_grass = group->grass.begin();
			for (int j = 0; j < grass_size; j++, it_grass++)
			{
				LPGRASS grass = *it_grass;
				// 풀의 위치 
				fwrite(grass->Pos, sizeof(grass->Pos), 1, fp);
				// 풀의 타일 번호 
				fwrite(&grass->Tile_x, sizeof(grass->Tile_x), 1, fp);
				fwrite(&grass->Tile_z, sizeof(grass->Tile_z), 1, fp);
			}
		}
		fclose(fp);
	}

//	Test_GameDataSave();

};
void CSowSeedMng::LoadData(void)
{

	char szOldPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szOldPath);

	// Seed List 읽어 오기..
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "tgi", nullptr, dwFlags, "Grass Info File(*.tgi)|*.tgi||", nullptr);

	if(dlg.DoModal() == IDCANCEL) return;

	// 리스트 초기화 
	if( Grass_Group.size() > 0)
	{
		Render_Grass = FALSE;
		it_Grass_Group it = Grass_Group.begin();
		for (int i = 0; i < static_cast<int>(Grass_Group.size()); i++, it++)
		{
			LPGRASS_GROUP group = (LPGRASS_GROUP)*it;
			it_Grass it_grass = group->grass.begin();
			for (int j = 0; j < static_cast<int>(group->grass.size()); j++, it_grass++)
			{
				LPGRASS grass = *it_grass;
				if( grass != nullptr)
					delete[] grass;
			}
			group->grass.clear();
			if( group != nullptr)
				delete group;
		}
		Grass_Group.clear();
	}
// 로딩 시작 
	FILE* fp = fopen((LPCTSTR)dlg.GetPathName(),"r");

	int size = 0;

	fread(&size,sizeof(size),1,fp);

	for( int i = 0; i < size ; i++)
	{
		LPGRASS_GROUP group = new GRASS_GROUP;
		fread(&group->b_size,sizeof( group->b_size),1,fp);
		fread(&group->Group_id,sizeof(group->Group_id),1,fp);
		fread(&group->Obj_ID,sizeof(group->Obj_ID),1,fp);
		fread(group->Pos,sizeof(group->Pos),1,fp);
		int grass_sub_size = 0;
		fread(&grass_sub_size ,sizeof(grass_sub_size),1,fp);

		// 파일명 읽기 
		int len =0;
		fread(&len,sizeof(int),1,fp);
		fread(group->FileName,len,1,fp);

		for( int j = 0; j < grass_sub_size ; j++)
		{

			LPGRASS grass = new GRASS;
			// 풀의 위치 
			fread(grass->Pos,sizeof(grass->Pos),1,fp);
			// 풀의 타일 번호 
			fread(&grass->Tile_x,sizeof(grass->Tile_x),1,fp);
			fread(&grass->Tile_z,sizeof(grass->Tile_z),1,fp);
			group->grass.push_back(grass);
		}

		Grass_Group.push_back(group);
	}

	SetCurrentDirectory(szOldPath);

	fclose(fp);

//	Test_GameDataLoad();

};


void CSowSeedMng::SaveDataGame(void)
{


	CMainFrame* m_pRefFrm= (CMainFrame*)AfxGetMainWnd();

	if(m_pRefFrm->m_SeedFileName[0] == '\0')
	{
		::MessageBox(nullptr,"맵 파일 부터 게임용으로 저장하세요","확인",MB_OK);
		return;
	}

	char szOldPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szOldPath);

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	char szModuleFilePath[_MAX_PATH];
	GetModuleFileName(nullptr, szModuleFilePath, _MAX_PATH);
	char szCurrDir[MAX_PATH];
	char szNewDir[MAX_PATH];
	_splitpath(szModuleFilePath, szDrive, szDir, nullptr, nullptr);
	_makepath(szCurrDir, szDrive, szDir, nullptr, nullptr);
	SetCurrentDirectory(szCurrDir);

	CreateDirectory("Seeds", nullptr);
	sprintf(szNewDir,"%s\\Seeds",szCurrDir);
	SetCurrentDirectory(szNewDir);

	char szNewPath[_MAX_PATH];
	//char szNewFName[_MAX_PATH];
	//_splitpath(szModuleFilePath, szDrive, szDir, nullptr, nullptr);
	_makepath(szNewPath, nullptr, nullptr, m_pRefFrm->m_SeedFileName, "grs");

	
	HANDLE hFile = CreateFile(szNewPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	DWORD dwRWC;

	char Buff[80];
	sprintf(Buff,"GrassInfoFile");
	WriteFile(hFile, Buff, 80, &dwRWC, nullptr);

	// 그룹 크기 
	Obj_Name.clear();
	it_Grass_Group it = Grass_Group.begin();
	for (int i = 0, Object_ID = 0; i < static_cast<int>(Grass_Group.size()); i++, it++)
	{
		LPGRASS_GROUP group = *it;
		BOOL back_push = TRUE;
		it_Obj_Name it_Obj = Obj_Name.begin();
		for (int j = 0; j < static_cast<int>(Obj_Name.size()); j++, it_Obj++)
		{
			LPOBJ_NAME Obj = *it_Obj;
			if( strcmp( group->FileName,Obj->FileName) == 0 )
				back_push = FALSE;
		}
		if( back_push == TRUE)
		{
			LPOBJ_NAME File_Name = new OBJ_NAME;
			strcpy(File_Name->FileName,group->FileName);
			File_Name->Id = Object_ID;
			Obj_Name.push_back(File_Name);
			Object_ID ++;
		}

	}

	int iNum = static_cast<int>(Obj_Name.size());
	WriteFile(hFile, &iNum, sizeof(int), &dwRWC, nullptr);

	it_Obj_Name it_Obj = Obj_Name.begin();
	for (int j = 0; j < static_cast<int>(Obj_Name.size()); j++, it_Obj++)
	{
		LPOBJ_NAME Obj = *it_Obj;
		int len = static_cast<int>(strlen(Obj->FileName));
		WriteFile(hFile, &len, sizeof(int), &dwRWC, nullptr);
		WriteFile(hFile, Obj->FileName, len, &dwRWC, nullptr);
	}

	CloseHandle(hFile);

	SetCurrentDirectory(szOldPath);
}


void CSowSeedMng::Test_GameDataSave(void)
{

	DWORD dwRWC = 0;
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(FALSE, "tst", nullptr, dwFlags, "Grass Info File(*.tst)|*.tst||", nullptr);

	if(dlg.DoModal() == IDCANCEL) return;

	HANDLE hFile = CreateFile(dlg.GetPathName(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);


	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	int Map_Size = pFrm->GetMapMng()->GetTerrain()->m_iHeightMapSize;
	//타일에 풀 속성 저장..
	LPSEEDGROUP SeedAttr = new SEEDGROUP[Map_Size * Map_Size];
	ZeroMemory(SeedAttr, sizeof(unsigned char) * Map_Size * Map_Size);

	for (int i = 0; i < Map_Size * Map_Size; i++)
	{
		SeedAttr[i].Obj_Id = 0;
		SeedAttr[i].Seed_Count = 0;
		SeedAttr[i].SeedGroup_Sub = nullptr;
		SeedAttr[i].sub_flage = 0;
	}

	for (LPGRASS_GROUP group : pFrm->GetMapMng()->m_SowSeedMng.Grass_Group)
	{
		for (LPGRASS grass : group->grass)
		{
			int index = grass->Tile_z + (grass->Tile_x * Map_Size);

			if (SeedAttr[index].Obj_Id == 0)
			{
				SeedAttr[index].Obj_Id = group->Group_id + 1;

				if (SeedAttr[index].Seed_Count < 15)
					SeedAttr[index].Seed_Count += 1;
			}
			else
			{
				if (SeedAttr[index].Obj_Id == group->Group_id + 1)
				{
					if (SeedAttr[index].Seed_Count < 15)
						SeedAttr[index].Seed_Count += 1;
				}
				else
				{
					if (SeedAttr[index].SeedGroup_Sub == nullptr)
					{
						SeedAttr[index].sub_flage = 1;
						SeedAttr[index].SeedGroup_Sub = new SEEDGROUP;
					}

					SeedAttr[index].SeedGroup_Sub->Obj_Id = group->Group_id + 1;
					if (SeedAttr[index].SeedGroup_Sub->Seed_Count < 15)
						SeedAttr[index].SeedGroup_Sub->Seed_Count += 1;
				}
			}
		}
	}

	for (int i = 0; i < Map_Size * Map_Size; i++)
	{
		WriteFile(hFile, &SeedAttr[i], sizeof(unsigned char), &dwRWC, nullptr);
		if (SeedAttr[i].sub_flage == 1)
			WriteFile(hFile, SeedAttr[i].SeedGroup_Sub, sizeof(unsigned char), &dwRWC, nullptr);
	}


	CloseHandle(hFile);

}

void CSowSeedMng::Test_GameDataLoad(void)
{
	DWORD dwRWC = 0;
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "tst", nullptr, dwFlags, "Grass Info File(*.tst)|*.tst||", nullptr);

	if(dlg.DoModal() == IDCANCEL) return;

//	HANDLE hFile = CreateFile(dlg.GetPathName(), GENERIC_READ, 0, nullptr, 
//		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	FILE* fp = fopen((LPCTSTR)dlg.GetPathName(),"r");


	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	int Map_Size = pFrm->GetMapMng()->GetTerrain()->m_iHeightMapSize;
	//타일에 풀 속성 저장..
	LPSEEDGROUP SeedAttr = new SEEDGROUP[Map_Size*Map_Size];
	ZeroMemory(SeedAttr, sizeof(unsigned char)*Map_Size*Map_Size);

	int  i;
	for(i = 0 ; i < Map_Size*Map_Size ; i++)
	{
		SeedAttr[i].Obj_Id = 0;
		SeedAttr[i].Seed_Count = 0;
		SeedAttr[i].SeedGroup_Sub = nullptr;
		SeedAttr[i].sub_flage = 0;
	}

	for( i = 0 ; i < Map_Size*Map_Size; i++)
	{
//		ReadFile(hFile,&SeedAttr[i],sizeof(unsigned char),&dwRWC,nullptr);
		fread(&SeedAttr[i],sizeof(unsigned char),1,fp);
		if( SeedAttr[i].sub_flage == 1)
		{
			SeedAttr[i].SeedGroup_Sub = new SEEDGROUP;
//			ReadFile(hFile,SeedAttr[i].SeedGroup_Sub,sizeof(unsigned char),&dwRWC,nullptr);
			fread(SeedAttr[i].SeedGroup_Sub,sizeof(unsigned char),1,fp);
		}
	}

	// 텍스트파일로 함 뽑아보자..
	FILE* stream = fopen("c:\\grass.txt", "w");
	for(int z=0; z<Map_Size;z++)
	{
		for(int x=0; x<Map_Size;x++)
		{
			SEEDGROUP v = SeedAttr[z + (x*Map_Size)];
			fprintf(stream, "%d,%d\t",v.Obj_Id,v.Seed_Count );

			if( v.SeedGroup_Sub !=nullptr)
				fprintf(stream, "서브 %d,%d\t",v.SeedGroup_Sub->Obj_Id,v.SeedGroup_Sub->Seed_Count );


			fprintf(stream, "\n");
		}
	}
	fclose(stream);

//	CloseHandle(hFile);
	fclose(fp);




}
