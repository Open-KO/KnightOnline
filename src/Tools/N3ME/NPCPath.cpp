// NPCPath.cpp: implementation of the CNPCPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "NPCPath.h"
#include "LyTerrain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNPCPath::CNPCPath()
{
	m_iNPCID  = 0;
	m_iNumNPC = 0;
	ZeroMemory(m_strNPCName, 80);
	ZeroMemory(m_strPathName, 255);
	m_pRefTerrain = nullptr;
	m_dwColor     = 0xff0000ff;
	m_iVersion    = 4;

	m_LTActVertex.Set(0.0f, 0.0f, 0.0f);
	m_RBActVertex.Set(0.0f, 0.0f, 0.0f);

	m_cAttr_Create = 0;
	m_cAttr_Regen  = 0;
	m_cAttr_Group  = 0;
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
	if (m_Path.size() > 0)
		m_Path.pop_back();
}

bool CNPCPath::GetPath(int idx, __Vector3* pPos)
{
	if (idx < 0 || idx >= static_cast<int>(m_Path.size()))
		return false;

	std::list<__Vector3>::iterator it;
	it = m_Path.begin();

	for (int i = 0; i < idx; i++)
	{
		it++;
	}
	(*pPos) = (*it);
	return true;
}

void CNPCPath::Load(File& file)
{
	file.Read(&m_strPathName, 256);
	file.Read(&m_iActType, sizeof(int));
	file.Read(&m_iNPCID, sizeof(int));
	file.Read(&m_iNumNPC, sizeof(int));
	file.Read(&m_iRegenTime, sizeof(int));
	file.Read(&m_iZoneID, sizeof(int));

	file.Read(&m_LTStartVertex, sizeof(__Vector3));
	file.Read(&m_RBStartVertex, sizeof(__Vector3));
	file.Read(m_strNPCName, 80);

	m_LTStartVertex.y = m_pRefTerrain->GetHeight(m_LTStartVertex.x, m_LTStartVertex.z);
	m_RBStartVertex.y = m_pRefTerrain->GetHeight(m_RBStartVertex.x, m_RBStartVertex.z);

	int size;
	file.Read(&size, sizeof(int));

	m_iVersion = size / 1000;
	size       = size % 1000;

	__Vector3 Vertex;
	m_Path.clear();
	for (int i = 0; i < size; i++)
	{
		file.Read(&Vertex, sizeof(__Vector3));
		Vertex.y = m_pRefTerrain->GetHeight(Vertex.x, Vertex.z);
		m_Path.push_back(Vertex);
	}
	if (m_iVersion >= 1)
	{
		file.Read(&m_dwColor, sizeof(DWORD));
	}
	if (m_iVersion == 2)
	{
		int tmp;
		file.Read(&tmp, sizeof(int));
		file.Read(&m_LTActVertex, sizeof(__Vector3));
		file.Read(&m_RBActVertex, sizeof(__Vector3));
	}
	if (m_iVersion >= 3)
	{
		file.Read(&m_LTActVertex, sizeof(__Vector3));
		file.Read(&m_RBActVertex, sizeof(__Vector3));

		file.Read(&m_cAttr_Create, sizeof(unsigned char));
		file.Read(&m_cAttr_Regen, sizeof(unsigned char));
		file.Read(&m_cAttr_Group, sizeof(unsigned char));
	}
	if (m_iVersion >= 4)
	{
		file.Read(&m_cAttr_Option, sizeof(unsigned char));
	}
}

void CNPCPath::Save(File& file)
{
	file.Write(&m_strPathName, 256);
	file.Write(&m_iActType, sizeof(int));
	file.Write(&m_iNPCID, sizeof(int));
	file.Write(&m_iNumNPC, sizeof(int));
	file.Write(&m_iRegenTime, sizeof(int));
	file.Write(&m_iZoneID, sizeof(int));

	file.Write(&m_LTStartVertex, sizeof(__Vector3));
	file.Write(&m_RBStartVertex, sizeof(__Vector3));
	file.Write(m_strNPCName, 80);

	m_iVersion = 4;
	int size   = (m_iVersion * 1000) + static_cast<int>(m_Path.size());
	file.Write(&size, sizeof(int));

	std::list<__Vector3>::iterator itVertex;

	__Vector3 Vertex;
	for (itVertex = m_Path.begin(); itVertex != m_Path.end(); itVertex++)
	{
		Vertex = (*itVertex);
		file.Write(&Vertex, sizeof(__Vector3));
	}

	if (m_iVersion >= 1)
	{
		file.Write(&m_dwColor, sizeof(DWORD));
	}

	file.Write(&m_LTActVertex, sizeof(__Vector3));
	file.Write(&m_RBActVertex, sizeof(__Vector3));

	file.Write(&m_cAttr_Create, sizeof(unsigned char));
	file.Write(&m_cAttr_Regen, sizeof(unsigned char));
	file.Write(&m_cAttr_Group, sizeof(unsigned char));
	file.Write(&m_cAttr_Option, sizeof(unsigned char));
}

void CNPCPath::TransPos(float x, float z)
{
	std::list<__Vector3>::iterator itVertex;

	std::list<__Vector3> tmp;

	__Vector3 Vertex;
	for (itVertex = m_Path.begin(); itVertex != m_Path.end(); itVertex++)
	{
		Vertex    = (*itVertex);
		Vertex.x += x;
		Vertex.z += z;
		//if(m_pRefTerrain) Vertex.y = m_pRefTerrain->GetHeight(Vertex.x, Vertex.z);
		tmp.push_back(Vertex);
	}
	m_Path.clear();
	m_Path             = tmp;

	m_LTStartVertex.x += x;
	m_LTStartVertex.z += z;
	//if(m_pRefTerrain) m_LTStartVertex.y = m_pRefTerrain->GetHeight(m_LTStartVertex.x, m_LTStartVertex.z);

	m_RBStartVertex.x += x;
	m_RBStartVertex.z += z;
	//if(m_pRefTerrain) m_RBStartVertex.y = m_pRefTerrain->GetHeight(m_RBStartVertex.x, m_RBStartVertex.z);

	m_LTActVertex.x   += x;
	m_LTActVertex.z   += z;

	m_RBActVertex.x   += x;
	m_RBActVertex.z   += z;
}

bool CNPCPath::CheckValid()
{
	std::list<__Vector3>::iterator itVertex;
	__Vector3 Vertex;

	float MapSize = (m_pRefTerrain->m_iHeightMapSize - 1) * TERRAIN_CELL_SIZE;
	for (itVertex = m_Path.begin(); itVertex != m_Path.end(); itVertex++)
	{
		Vertex = (*itVertex);
		if (Vertex.x > MapSize || Vertex.z > MapSize || Vertex.x < 0 || Vertex.z < 0)
			return false;
	}

	if (m_LTStartVertex.x < 0 || m_LTStartVertex.x > MapSize || m_LTStartVertex.z < 0
		|| m_LTStartVertex.z > MapSize)
		return false;

	if (m_RBStartVertex.x < 0 || m_RBStartVertex.x > MapSize || m_RBStartVertex.z < 0
		|| m_RBStartVertex.z > MapSize)
		return false;

	if (m_LTActVertex.x < 0 || m_LTActVertex.x > MapSize || m_LTActVertex.z < 0
		|| m_LTActVertex.z > MapSize)
		return false;

	if (m_RBActVertex.x < 0 || m_RBActVertex.x > MapSize || m_RBActVertex.z < 0
		|| m_RBActVertex.z > MapSize)
		return false;

	return true;
}
