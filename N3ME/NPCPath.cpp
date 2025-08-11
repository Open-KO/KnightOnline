// NPCPath.cpp: implementation of the CNPCPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "NPCPath.h"
#include "LyTerrain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNPCPath::CNPCPath()
{
	m_iNPCID = 0;
	m_iNumNPC = 0;
	ZeroMemory(m_strNPCName, 80);
	ZeroMemory(m_strPathName, 255);
	m_pRefTerrain = NULL;
	m_dwColor = 0xff0000ff;
	m_iVersion = 4;

	m_LTActVertex.Set(0.0f, 0.0f, 0.0f);
	m_RBActVertex.Set(0.0f, 0.0f, 0.0f);

	m_cAttr_Create = 0;
	m_cAttr_Regen = 0;
	m_cAttr_Group = 0;
	m_cAttr_Option = 0;


	m_Path.clear();
}

CNPCPath::~CNPCPath()
{
	m_Path.clear();
}


void CNPCPath::AddPos(__Vector3 Path)
{
	m_Path.push_back(Path);
}

void CNPCPath::DelPrevPos()
{
	if(m_Path.size()>0) m_Path.pop_back();
}

bool CNPCPath::GetPath(int idx, __Vector3* pPos)
{
	if (idx < 0
		|| idx >= static_cast<int>(m_Path.size()))
		return false;

	std::list<__Vector3>::iterator it;
	it = m_Path.begin();

	for(int i=0; i<idx; i++)
	{
		it++;
	}
	(*pPos) = (*it);
	return true;
}

void CNPCPath::Load(HANDLE hFile)
{
	DWORD dwRWC;
	ReadFile(hFile, &m_strPathName, 256, &dwRWC, NULL);
	ReadFile(hFile, &m_iActType, sizeof(int), &dwRWC, NULL);
	ReadFile(hFile, &m_iNPCID, sizeof(int), &dwRWC, NULL);
	ReadFile(hFile, &m_iNumNPC, sizeof(int), &dwRWC, NULL);
	ReadFile(hFile, &m_iRegenTime, sizeof(int), &dwRWC, NULL);
	ReadFile(hFile, &m_iZoneID, sizeof(int), &dwRWC, NULL);

	ReadFile(hFile, &m_LTStartVertex, sizeof(__Vector3), &dwRWC, NULL);
	ReadFile(hFile, &m_RBStartVertex, sizeof(__Vector3), &dwRWC, NULL);
	ReadFile(hFile, m_strNPCName, 80, &dwRWC, NULL);

	m_LTStartVertex.y = m_pRefTerrain->GetHeight(m_LTStartVertex.x, m_LTStartVertex.z);
	m_RBStartVertex.y = m_pRefTerrain->GetHeight(m_RBStartVertex.x, m_RBStartVertex.z);

	int size;
	ReadFile(hFile, &size, sizeof(int), &dwRWC, NULL);

	m_iVersion = size / 1000;
	size = size % 1000;
	
	__Vector3 Vertex;
	m_Path.clear();
	for(int i=0;i<size;i++)
	{
		ReadFile(hFile, &Vertex, sizeof(__Vector3), &dwRWC, NULL);
		Vertex.y = m_pRefTerrain->GetHeight(Vertex.x, Vertex.z);
		m_Path.push_back(Vertex);
	}
	if(m_iVersion>=1)
	{
		ReadFile(hFile, &m_dwColor, sizeof(DWORD), &dwRWC, NULL);
	}
	if(m_iVersion==2)
	{
		int tmp;
		ReadFile(hFile, &tmp, sizeof(int), &dwRWC, NULL);
		ReadFile(hFile, &m_LTActVertex, sizeof(__Vector3), &dwRWC, NULL);
		ReadFile(hFile, &m_RBActVertex, sizeof(__Vector3), &dwRWC, NULL);
	}
	if(m_iVersion>=3)
	{
		ReadFile(hFile, &m_LTActVertex, sizeof(__Vector3), &dwRWC, NULL);
		ReadFile(hFile, &m_RBActVertex, sizeof(__Vector3), &dwRWC, NULL);

		ReadFile(hFile, &m_cAttr_Create, sizeof(unsigned char), &dwRWC, NULL);
		ReadFile(hFile, &m_cAttr_Regen, sizeof(unsigned char), &dwRWC, NULL);
		ReadFile(hFile, &m_cAttr_Group, sizeof(unsigned char), &dwRWC, NULL);
	}
	if(m_iVersion>=4)
	{
		ReadFile(hFile, &m_cAttr_Option, sizeof(unsigned char), &dwRWC, NULL);
	}
}

void CNPCPath::Save(HANDLE hFile)
{
	DWORD dwRWC;
	WriteFile(hFile, &m_strPathName, 256, &dwRWC, NULL);
	WriteFile(hFile, &m_iActType, sizeof(int), &dwRWC, NULL);
	WriteFile(hFile, &m_iNPCID, sizeof(int), &dwRWC, NULL);
	WriteFile(hFile, &m_iNumNPC, sizeof(int), &dwRWC, NULL);
	WriteFile(hFile, &m_iRegenTime, sizeof(int), &dwRWC, NULL);
	WriteFile(hFile, &m_iZoneID, sizeof(int), &dwRWC, NULL);

	WriteFile(hFile, &m_LTStartVertex, sizeof(__Vector3), &dwRWC, NULL);
	WriteFile(hFile, &m_RBStartVertex, sizeof(__Vector3), &dwRWC, NULL);
	WriteFile(hFile, m_strNPCName, 80, &dwRWC, NULL);

	m_iVersion = 4;
	int size = (m_iVersion*1000) + m_Path.size();
	WriteFile(hFile, &size, sizeof(int), &dwRWC, NULL);

	std::list<__Vector3>::iterator itVertex;

	__Vector3 Vertex;
	for(itVertex = m_Path.begin(); itVertex != m_Path.end(); itVertex++)
	{
		Vertex = (*itVertex);
		WriteFile(hFile, &Vertex, sizeof(__Vector3), &dwRWC, NULL);
	}

	if(m_iVersion>=1)
	{
		WriteFile(hFile, &m_dwColor, sizeof(DWORD), &dwRWC, NULL);
	}

	WriteFile(hFile, &m_LTActVertex, sizeof(__Vector3), &dwRWC, NULL);
	WriteFile(hFile, &m_RBActVertex, sizeof(__Vector3), &dwRWC, NULL);

	WriteFile(hFile, &m_cAttr_Create, sizeof(unsigned char), &dwRWC, NULL);
	WriteFile(hFile, &m_cAttr_Regen, sizeof(unsigned char), &dwRWC, NULL);
	WriteFile(hFile, &m_cAttr_Group, sizeof(unsigned char), &dwRWC, NULL);
	WriteFile(hFile, &m_cAttr_Option, sizeof(unsigned char), &dwRWC, NULL);	
}

void CNPCPath::TransPos(float x, float z)
{
	std::list<__Vector3>::iterator itVertex;

	std::list<__Vector3> tmp; 

	__Vector3 Vertex;
	for(itVertex = m_Path.begin(); itVertex != m_Path.end(); itVertex++)
	{
		Vertex = (*itVertex);
		Vertex.x += x;
		Vertex.z += z;
		//if(m_pRefTerrain) Vertex.y = m_pRefTerrain->GetHeight(Vertex.x, Vertex.z);
		tmp.push_back(Vertex);
	}
	m_Path.clear();
	m_Path = tmp;

	m_LTStartVertex.x += x;
	m_LTStartVertex.z += z;
	//if(m_pRefTerrain) m_LTStartVertex.y = m_pRefTerrain->GetHeight(m_LTStartVertex.x, m_LTStartVertex.z);

	m_RBStartVertex.x += x;
	m_RBStartVertex.z += z;
	//if(m_pRefTerrain) m_RBStartVertex.y = m_pRefTerrain->GetHeight(m_RBStartVertex.x, m_RBStartVertex.z);

	m_LTActVertex.x += x;
	m_LTActVertex.z += z;

	m_RBActVertex.x += x;
	m_RBActVertex.z += z;
}

bool CNPCPath::CheckValid()
{
	std::list<__Vector3>::iterator itVertex;
	__Vector3 Vertex;

	float MapSize = (m_pRefTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE;
	for(itVertex = m_Path.begin(); itVertex != m_Path.end(); itVertex++)
	{
		Vertex = (*itVertex);
		if(Vertex.x > MapSize || Vertex.z > MapSize || Vertex.x < 0 || Vertex.z < 0) return false;
	}

	if(m_LTStartVertex.x < 0 || m_LTStartVertex.x > MapSize || 
		m_LTStartVertex.z < 0 || m_LTStartVertex.z > MapSize ) return false;

	if(m_RBStartVertex.x < 0 || m_RBStartVertex.x > MapSize || 
		m_RBStartVertex.z < 0 || m_RBStartVertex.z > MapSize ) return false;

	if(m_LTActVertex.x < 0 || m_LTActVertex.x > MapSize || 
		m_LTActVertex.z < 0 || m_LTActVertex.z > MapSize ) return false;

	if(m_RBActVertex.x < 0 || m_RBActVertex.x > MapSize || 
		m_RBActVertex.z < 0 || m_RBActVertex.z > MapSize ) return false;

	return true;
}
