// DTexGroupMng.cpp: implementation of the CDTexGroupMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "DTexGroupMng.h"
#include "DTexGroup.h"
#include "MainFrm.h"
#include "DTexMng.h"
#include "DlgDTexGroupView.h"
#include "ProgressBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDTexGroupMng::CDTexGroupMng()
{
	m_pGroupView = nullptr;
	m_pMainFrm = nullptr;

	memset(&m_SelectedDTex, 0, sizeof(m_SelectedDTex));
	memset(&m_SelectedDTexTile, 0, sizeof(m_SelectedDTexTile));
}

CDTexGroupMng::~CDTexGroupMng()
{
	if (m_pGroupView != nullptr)
	{
		m_pGroupView->DestroyWindow();
		delete m_pGroupView;
		m_pGroupView = nullptr;
	}

	for (CDTexGroup* pDTG : m_Groups)
		delete pDTG;
}


//
//	Init..
//
void CDTexGroupMng::Init(CWnd* pWndParent)
{
	Release();

	m_pMainFrm = (CMainFrame*)pWndParent;

	m_pGroupView = new CDlgDTexGroupView;
	m_pGroupView->Create(IDD_DTEX_GROUP);
	m_pGroupView->ShowWindow(FALSE);
	m_pGroupView->SetGroupMng(this);
	
	SetGroupID("NONE", 0);
}


//
//	Release..
//
void CDTexGroupMng::Release()
{
	if (m_pGroupView != nullptr)
	{
		m_pGroupView->DestroyWindow();
		delete m_pGroupView;
		m_pGroupView = nullptr;
	}

	for (CDTexGroup* pDTG : m_Groups)
		delete pDTG;
	m_Groups.clear();
}


//
//
//
int CDTexGroupMng::GetBrushSize()
{
	if(m_pGroupView)
	{
		return m_pGroupView->m_SliderBrushSize.GetPos();
	}
	return 0;
}


//
//	SetGroup..
//
void CDTexGroupMng::SetGroup(const char* pName)
{
	//^^
	//셋팅하려는 그룹이 있으면 걍 리턴해버려..
	for (CDTexGroup* pDTG : m_Groups)
	{
		if (strcmp(pDTG->m_Name, pName) == 0)
			return;
	}
	//
	//^^

	CDTexGroup* pGroup = new CDTexGroup();
	pGroup->Init();

	wsprintf(pGroup->m_Name, pName);

	//새로운 idx를 지정하기 위해서 일단 정렬을 하자..
	auto it = m_Groups.begin();
	int iSize = static_cast<int>(m_Groups.size());
	int* ArrayIdx = new int[iSize];
	for (int i = 0; i < iSize; i++, it++)
	{
		CDTexGroup* pDTG = *it;
		ArrayIdx[i] = pDTG->m_ID;
	}

	qsort( ArrayIdx, iSize, sizeof(int), this->CompareIdx );

	int idx = 0;
	for (int i = 0; i < iSize; i++)
	{
		if (idx == ArrayIdx[i])
			idx++;
	}
	delete ArrayIdx;

	pGroup->m_ID = idx;
	
	m_Groups.push_back(pGroup);
	m_pGroupView->AddGroup(pGroup);
}


//
//	SetGroupID..
//	ID로 그룹 넣기..
//
void CDTexGroupMng::SetGroupID(const char* pName, int id)
{
	CDTexGroup* pGroup = new CDTexGroup;
	
	pGroup->Init();

	wsprintf(pGroup->m_Name, pName);
	pGroup->m_ID = id;
	
	m_Groups.push_back(pGroup);
	m_pGroupView->AddGroup(pGroup);	
}


//
//	qsort를 위해 정렬하는 함수...
//
int CDTexGroupMng::CompareIdx( const void *arg1, const void *arg2 )
{
	int a,b;
	a = (*((int*)arg1));
	b = (*((int*)arg2));

	if(a<b) return -1;
	if(a==b) return 0;
	return 1;
}


//
//	그룹 지우기..
//
void CDTexGroupMng::DelGroup(int ID)
{
	for (auto it = m_Groups.begin(); it != m_Groups.end(); ++it)
	{
		CDTexGroup* pDTG = *it;
		if (pDTG == nullptr)
			continue;

		if (pDTG->m_ID == ID)
		{
			pDTG->ClearDTex();
			delete pDTG;
			it = m_Groups.erase(it);
			m_pGroupView->ResetAll();
			return;
		}
	}
}


//
//	그룹안에 타일 넣기..
//
void CDTexGroupMng::SetTile(int ID, int attr, __DTexTileAttr tile)
{
	for (CDTexGroup* pDTG : m_Groups)
	{
		if (pDTG != nullptr && pDTG->m_ID == ID)
		{
			pDTG->SetAttr(attr, tile);
			m_pGroupView->ResetAll();
			return;
		}
	}
}

//
//
//
void CDTexGroupMng::DelTile(int ID, int attr, __DTexTileAttr tile)
{
	for (CDTexGroup* pDTG : m_Groups)
	{
		if (pDTG != nullptr && pDTG->m_ID == ID)
		{
			pDTG->DelAttr(attr, tile);
			m_pGroupView->ResetAll();
			return;
		}
	}
}

//
//
//
void CDTexGroupMng::DelTileByDTexID(int DTexID)
{
	for (CDTexGroup* pDTG : m_Groups)
		pDTG->DelAttrByDTexID(DTexID);
	m_pGroupView->ResetAll();
}

//
//
//
int CDTexGroupMng::GetID2Index(int id)
{
	auto it = m_Groups.begin();
	auto itEnd = m_Groups.end();
	int i = 0;
	for (; it != itEnd; ++it, ++i)
	{
		CDTexGroup* pDTG = *it;
		if (pDTG != nullptr && pDTG->m_ID == id)
			return i;
	}
	return 0;
}

//
//
//
int CDTexGroupMng::GetIndex2ID(int idx)
{
	if (idx < 0
		|| idx >= static_cast<int>(m_Groups.size()))
		return 0;

	auto it = m_Groups.begin();
	std::advance(it, idx);

	CDTexGroup* pDTG = *it;
	if (pDTG != nullptr)
		return pDTG->m_ID;
	
	return 0;
}


//
//
//
__DTexTileAttr CDTexGroupMng::GetTileAttr(int groupID, int attr)
{
	__DTexTileAttr DTileAttr;
	DTileAttr.TexID = -1;

	if (groupID == 0)
		return DTileAttr;

	CDTexGroup* pGroup = nullptr;
	for (CDTexGroup* pDTG : m_Groups)
	{
		if (pDTG != nullptr && pDTG->m_ID == groupID)
			pGroup = pDTG;
	}

	if (pGroup != nullptr)
	{
		size_t attrCount = pGroup->m_Attributes[attr].size();
		if (attrCount != 0)
		{
			size_t AttrIdx = static_cast<size_t>(rand()) % attrCount;
			it_DTexTileAttr it = pGroup->m_Attributes[attr].begin();
			std::advance(it, AttrIdx);
			DTileAttr = *(*it);
		}
	}

	return DTileAttr;
}

//
//
//
__DTexTileAttr CDTexGroupMng::GetTileAttrManuel(int groupID, int attr, int AttrIdx)
{
	__DTexTileAttr DTileAttr;
	DTileAttr.TexID = -1;

	CDTexGroup* pGroup = nullptr;
	for (CDTexGroup* pDTG : m_Groups)
	{
		if (pDTG != nullptr && pDTG->m_ID == groupID)
			pGroup = pDTG;
	}

	int attrCount = static_cast<int>(pGroup->m_Attributes[attr].size());
	if (AttrIdx >= 0 && AttrIdx < attrCount)
	{
		auto it = pGroup->m_Attributes[attr].begin();
		std::advance(it, AttrIdx);
		DTileAttr = *(*it);
	}
	return DTileAttr;
}

//
//
//
char* CDTexGroupMng::GetGroupName(int id)
{
	for (CDTexGroup* pDTG : m_Groups)
	{
		if (pDTG != nullptr && pDTG->m_ID == id)
			return pDTG->m_Name;
	}

	return nullptr;
}

//
//
//
bool CDTexGroupMng::LoadFromFile(CString RealFileName)
{
 	Init(m_pMainFrm);
	SetCurrentDirectory(CN3Base::s_szPath.c_str());

	char szDTexInfoFileName[_MAX_PATH];
	sprintf(szDTexInfoFileName,"dtex\\%s.tgx", (LPCTSTR)RealFileName);

	FILE* stream = fopen(szDTexInfoFileName, "r");
	if(stream)
	{
		int iCount;
		int result = fscanf(stream, "NumGroup = %d\n", &iCount);
		if(EOF == result) { MessageBox(::GetActiveWindow(), szDTexInfoFileName, "Invalid DTex Info File...", MB_OK); return false; }

		CProgressBar ProgressBar;
		ProgressBar.Create("Load TileGroup Info..", 50,  iCount);

		int i;
		int id;
		char szDTexGroupName[40];
		for(i=0; i<iCount; i++)
		{
			result = fscanf(stream, "%s %d\n", szDTexGroupName, &id);
			//result = fscanf(stream, "%s", szDTexGroupName);
			//result = fscanf(stream, "%d\n", &id);
			if(EOF == result) { MessageBox(::GetActiveWindow(), szDTexInfoFileName, "Invalid DTex Info File...", MB_OK); return false; }

			ProgressBar.StepIt();

			SetGroupID(szDTexGroupName, id);
		}
		fclose(stream);
		return true;
	}

	return false;
}


//
//
//
bool CDTexGroupMng::SaveToFile(CString RealFileName)
{
	SetCurrentDirectory(CN3Base::s_szPath.c_str());

	CreateDirectory("dtex", nullptr);

	char szDTexInfoFileName[_MAX_PATH];
	sprintf(szDTexInfoFileName,"dtex\\%s.tgx", (LPCTSTR)RealFileName);

	FILE* stream = fopen(szDTexInfoFileName, "w");
	if (stream == nullptr)
		return false;

	int iCount = static_cast<int>(m_Groups.size()) - 1;
	fprintf(stream, "NumGroup = %d\n", iCount);

	CProgressBar ProgressBar;
	ProgressBar.Create("Save TileGroup Info..", 50, iCount);

	int id;
	char szDTexGroupName[40];
	auto it = m_Groups.begin();
	for (int i = 0; i <= iCount; i++, it++)
	{
		ProgressBar.StepIt();

		CDTexGroup* pGroup = *it;
		sprintf(szDTexGroupName, pGroup->m_Name);
		id = pGroup->m_ID;

		if (id == 0)
			continue;

		fprintf(stream, "%s %d\n", szDTexGroupName, id);
	}

	fclose(stream);
	return true;
}
