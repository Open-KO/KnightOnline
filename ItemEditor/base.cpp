/*
*/

#include "base.h"

//-----------------------------------------------------------------------------
char               pTexName[0xFF];
_N3TexHeader       HeaderOrg;
uint8_t*     compTexData;
int                compTexSize;
int                iPixelSize;
uint16_t*    m_pIndices0;
_N3VertexT1*       m_pVertices0;
int                m_iMaxNumIndices0;
int                m_iMaxNumVertices0;
int                m_nVC;
int                m_nFC;
int                m_nUVC;
__VertexSkinned*   m_pSkinVertices;
__VertexXyzNormal* m_pVertices;
uint16_t*              m_pwVtxIndices;
float*             m_pfUVs;
uint16_t*              m_pwUVsIndices;
aiScene            m_Scene;
e_ItemType         eType;

//-----------------------------------------------------------------------------
std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

//-----------------------------------------------------------------------------
void N3LoadTexture(const char* szFN) {

	size_t last_slash = 0;
	size_t last_point = 0;

	memset(pTexName, 0x00, 0xFF);

	size_t len = strlen(szFN);

	while (szFN[last_slash++] != '\\' && last_slash < len)
		;

	while (szFN[last_point++] != '.' && last_point < len)
		;

	for (size_t i = last_slash; i < last_point - 1; ++i)
	{
		pTexName[i - last_slash] = szFN[i];
	}

	strcat(pTexName, ".bmp");
	printf("Texture: %s (%s)\n", pTexName, szFN);

	FILE* fpTexture = fopen(szFN, "rb");
	if(fpTexture == NULL) {
		fprintf(stderr, "ERROR: Missing texture %s\n", szFN);
		return;
	}

	int nL0 = 0;
	fread(&nL0, sizeof(int), 1, fpTexture);

	char m_szName0[NAME_LENGTH] = {};

	if(nL0 > 0) {
		memset(&m_szName0, 0, (nL0+1));
		fread(&m_szName0, sizeof(char), nL0, fpTexture);
	}

	fread(&HeaderOrg, sizeof(_N3TexHeader), 1, fpTexture);

	printf("\nTexName: %s\n", m_szName0);
	printf("H.szID    -> %c%c%c%hu\n",
		HeaderOrg.szID[0],
		HeaderOrg.szID[1],
		HeaderOrg.szID[2],
		(uint8_t) HeaderOrg.szID[3]
	);
	printf("H.nWidth  -> %d\n", HeaderOrg.nWidth);
	printf("H.nHeight -> %d\n", HeaderOrg.nHeight);
	printf("H.Format  -> %d\n", HeaderOrg.Format);
	printf("H.bMipMap -> %d\n\n", HeaderOrg.bMipMap);

	iPixelSize = 0;
	switch(HeaderOrg.Format) {
		case D3DFMT_DXT1: {
			compTexSize = (HeaderOrg.nWidth*HeaderOrg.nHeight/2);
		} break;
		case D3DFMT_DXT3: {
			compTexSize = (HeaderOrg.nWidth*HeaderOrg.nHeight);
		} break;
		case D3DFMT_DXT5: {
			compTexSize = (HeaderOrg.nWidth*HeaderOrg.nHeight);
		} break;
		case D3DFMT_A1R5G5B5:
		case D3DFMT_A4R4G4B4: {
			iPixelSize = 2;
		} break;
		case D3DFMT_R8G8B8: {
			iPixelSize = 3;
		} break;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8: {
			iPixelSize = 4;
		} break;
		default: {
			fprintf(stderr,
				"\nERROR: Unknown texture format %d. (need to implement this)\n",
				HeaderOrg.Format
			);
			return;
		} break;
	}

	if(iPixelSize == 0) {
		if(compTexData) {
			delete compTexData;
			compTexData = NULL;
		}

		compTexData = new uint8_t[compTexSize];
		fread(compTexData, sizeof(uint8_t), compTexSize, fpTexture);

	} else {
		if(compTexData) {
			delete compTexData;
			compTexData = NULL;
		}

		compTexSize = (HeaderOrg.nWidth*HeaderOrg.nHeight*iPixelSize);
		compTexData = new uint8_t[compTexSize];
		fread(compTexData, sizeof(uint8_t), compTexSize, fpTexture);
	}

	fclose(fpTexture);
}

//-----------------------------------------------------------------------------
void N3LoadMesh(const char* szFN) {
	FILE* fpMesh = fopen(szFN, "rb");
	if(fpMesh == NULL) {
		fprintf(stderr, "\nERROR: Missing mesh %s\n", szFN);
		return;
	}

	int nL0 = 0;
	fread(&nL0, sizeof(int), 1, fpMesh);

	char m_szName0[NAME_LENGTH] = {};

	if(nL0 > 0) {
		memset(&m_szName0, 0, (nL0+1));
		fread(&m_szName0, sizeof(char), nL0, fpMesh);
	}

	int m_iNumCollapses0;
	int m_iTotalIndexChanges0;
	int m_iMinNumVertices0;
	int m_iMinNumIndices0;

	fread(&m_iNumCollapses0, sizeof(int), 1, fpMesh);
	fread(&m_iTotalIndexChanges0, sizeof(int), 1, fpMesh);
	fread(&m_iMaxNumVertices0, sizeof(int), 1, fpMesh);
	fread(&m_iMaxNumIndices0, sizeof(int), 1, fpMesh);
	fread(&m_iMinNumVertices0, sizeof(int), 1, fpMesh);
	fread(&m_iMinNumIndices0, sizeof(int), 1, fpMesh);

	if(m_pVertices0) {
		delete[] m_pVertices0;
		m_pVertices0 = NULL;
	}

	if(m_iMaxNumVertices0 > 0) {
		m_pVertices0 = new _N3VertexT1[m_iMaxNumVertices0];
		memset(m_pVertices0, 0, sizeof(_N3VertexT1)*m_iMaxNumVertices0);
		fread(m_pVertices0, sizeof(_N3VertexT1), m_iMaxNumVertices0, fpMesh);
	}

	if(m_pIndices0) {
		delete m_pIndices0;
		m_pIndices0 = NULL;
	}

	if(m_iMaxNumIndices0 > 0) {
		m_pIndices0 = new uint16_t[m_iMaxNumIndices0];
		memset(m_pIndices0, 0, sizeof(uint16_t)*m_iMaxNumIndices0);
		fread(m_pIndices0, sizeof(uint16_t), m_iMaxNumIndices0, fpMesh);
	}

	// NOTE: read in the "collapses" (I think this is used to set the vertices
	// based on how close the player is to the object)
	_N3EdgeCollapse* m_pCollapses = new _N3EdgeCollapse[(m_iNumCollapses0+1)];
	memset(&m_pCollapses[m_iNumCollapses0], 0, sizeof(_N3EdgeCollapse));

	if(m_iNumCollapses0 > 0) {
		fread(m_pCollapses, sizeof(_N3EdgeCollapse), m_iNumCollapses0, fpMesh);
		memset(&m_pCollapses[m_iNumCollapses0], 0, sizeof(_N3EdgeCollapse));
	}

	int* m_pAllIndexChanges = new int[m_iTotalIndexChanges0];

	if(m_iTotalIndexChanges0 > 0) {
		fread(m_pAllIndexChanges, sizeof(int), m_iTotalIndexChanges0, fpMesh);
	}

	int m_iLODCtrlValueCount0;
	fread(&m_iLODCtrlValueCount0, sizeof(int), 1, fpMesh);

	_N3LODCtrlValue* m_pLODCtrlValues = new _N3LODCtrlValue[m_iLODCtrlValueCount0];

	if(m_iLODCtrlValueCount0) {
		fread(m_pLODCtrlValues, sizeof(_N3LODCtrlValue), m_iLODCtrlValueCount0, fpMesh);
	}

	if(m_pAllIndexChanges && m_iLODCtrlValueCount0) {
		int m_iNumIndices = 0;
		int m_iNumVertices = 0;

		int c = 0;
		int LOD = 0;

		// TEMP HACK
		/*
		if(m_szName0[0] == 'a') {
			LOD = 1;
		}
		*/

		int iDiff = m_pLODCtrlValues[LOD].iNumVertices - m_iNumVertices;

		while(m_pLODCtrlValues[LOD].iNumVertices > m_iNumVertices) {
			if(c >= m_iNumCollapses0) break;
			if(m_pCollapses[c].NumVerticesToLose+m_iNumVertices > m_pLODCtrlValues[LOD].iNumVertices)
				break;

			m_iNumIndices += m_pCollapses[c].NumIndicesToLose;
			m_iNumVertices += m_pCollapses[c].NumVerticesToLose;
		
			int tmp0 = m_pCollapses[c].iIndexChanges;
			int tmp1 = tmp0+m_pCollapses[c].NumIndicesToChange;

			for(int i=tmp0; i<tmp1; i++) {
				m_pIndices0[m_pAllIndexChanges[i]] = m_iNumVertices-1;
			}

			c++;

			m_iMaxNumIndices0 = m_iNumIndices;
			m_iMaxNumVertices0 = m_iNumVertices;
		}

		// NOTE: if we break on a collapse that isn't intended to be one we
		// should collapse up to then keep collapsing until we find one
		while(m_pCollapses[c].bShouldCollapse) {

			if(c >= m_iNumCollapses0) break;

			m_iNumIndices += m_pCollapses[c].NumIndicesToLose;
			m_iNumVertices += m_pCollapses[c].NumVerticesToLose;
		
			int tmp0 = m_pCollapses[c].iIndexChanges;
			int tmp1 = tmp0+m_pCollapses[c].NumIndicesToChange;

			for(int i=tmp0; i<tmp1; i++) {
				m_pIndices0[m_pAllIndexChanges[i]] = m_iNumVertices-1;
			}

			c++;

			m_iMaxNumIndices0 = m_iNumIndices;
			m_iMaxNumVertices0 = m_iNumVertices;
		}
	}

	delete[] m_pLODCtrlValues;
	delete[] m_pAllIndexChanges;
	delete[] m_pCollapses;
	
	printf("\nMeshName: %s\n", m_szName0);
	printf("m_iNumCollapses      -> %d\n", m_iNumCollapses0);
	printf("m_iTotalIndexChanges -> %d\n", m_iTotalIndexChanges0);
	printf("m_iMaxNumVertices    -> %d\n", m_iMaxNumVertices0);
	printf("m_iMaxNumIndices     -> %d\n", m_iMaxNumIndices0);
	printf("m_iMinNumVertices    -> %d\n", m_iMinNumVertices0);
	printf("m_iMinNumIndices     -> %d\n", m_iMinNumIndices0);
	printf("m_iLODCtrlValueCount -> %d\n", m_iLODCtrlValueCount0);

	fclose(fpMesh);
}

//-----------------------------------------------------------------------------
bool CN3BaseFileAccess_Load(FILE* hFile) {
	std::string m_szName = "";

	int nL = 0;
	fread(&nL, sizeof(int), 1, hFile);
	if(nL > 0)  {
		std::vector<char> buffer(nL+1, NULL);
		fread(&buffer[0], sizeof(char), nL, hFile);
		m_szName = &buffer[0];
	}

	return true;
}

//-----------------------------------------------------------------------------
bool CN3CPlugBase_Load(FILE* hFile) {
	CN3BaseFileAccess_Load(hFile);

	int nL = 0;
	char szFN[512] = "";

	e_PlugType m_ePlugType;
	fread(&m_ePlugType, sizeof(e_PlugType), 1, hFile);
	if(m_ePlugType > PLUGTYPE_MAX) {
		m_ePlugType = PLUGTYPE_NORMAL;
	}

	int m_nJointIndex;
	fread(&m_nJointIndex, sizeof(int), 1, hFile);

	glm::vec3 m_vPosition;
	fread(&m_vPosition, sizeof(glm::vec3), 1, hFile);
	_N3Matrix44 m_MtxRot;
	fread(&m_MtxRot, sizeof(_N3Matrix44), 1, hFile);
	glm::vec3 m_vScale;
	fread(&m_vScale, sizeof(glm::vec3), 1, hFile);

	_N3Material m_Mtl;
	fread(&m_Mtl, sizeof(_N3Material), 1, hFile);

	fread(&nL, sizeof(int), 1, hFile);
	if(nL > 0) {
		fread(szFN, sizeof(char), nL, hFile);
		N3LoadMesh(szFN);
	}

	fread(&nL, sizeof(int), 1, hFile);
	if(nL > 0) {
		fread(szFN, sizeof(char), nL, hFile); szFN[nL] = '\0';
		N3LoadTexture(szFN);
	}	

	return 0;
}

//-----------------------------------------------------------------------------
bool CN3CPlug_Load(FILE* hFile) {
	CN3CPlugBase_Load(hFile);
	return true;
}

//-----------------------------------------------------------------------------
bool CN3IMesh_Create(int nFC, int nVC, int nUVC) {
	if(nFC<=0 || nVC<=0) return false;

	m_nFC = nFC;
	m_nVC = nVC;

	if(m_pVertices) delete m_pVertices;
	if(m_pwVtxIndices) delete m_pwVtxIndices;
	if(m_pfUVs) delete m_pfUVs;
	if(m_pwUVsIndices) delete m_pwUVsIndices;

	m_pVertices = new __VertexXyzNormal[nVC]; memset(m_pVertices, 0, sizeof(__VertexXyzNormal) * nVC);
	m_pwVtxIndices = new uint16_t[nFC*3]; memset(m_pwVtxIndices, 0, 2 * nFC * 3); // uint16_t

	if(nUVC > 0) {
		m_nUVC = nUVC;
		m_pfUVs = new float[nUVC*2]; memset(m_pfUVs, 0, 8 * nUVC);
		m_pwUVsIndices = new uint16_t[nFC*3]; memset(m_pwUVsIndices, 0, 2 * nFC * 3); // uint16_t
	}

	return true;
}

//-----------------------------------------------------------------------------
bool CN3Skin_Create(int nFC, int nVC, int nUVC) {
	if(false == CN3IMesh_Create(nFC, nVC, nUVC)) return false;

	delete [] m_pSkinVertices;
	m_pSkinVertices = new __VertexSkinned[nVC];
	memset(m_pSkinVertices, 0, sizeof(__VertexSkinned)*nVC);

	return true;
}

//-----------------------------------------------------------------------------
bool CN3IMesh_Load(FILE* hFile) {
	CN3BaseFileAccess_Load(hFile);

	int nFC = 0, nVC = 0, nUVC = 0;

	fread(&nFC, sizeof(int), 1, hFile);
	fread(&nVC, sizeof(int), 1, hFile);
	fread(&nUVC, sizeof(int), 1, hFile);

	if(nFC > 0 && nVC > 0) {
		CN3Skin_Create(nFC, nVC, nUVC);

		fread(m_pVertices, sizeof(__VertexXyzNormal), nVC, hFile);
		fread(m_pwVtxIndices, 2*3, nFC, hFile);
	}

	if(m_nUVC > 0) {
		fread(m_pfUVs, 8, nUVC, hFile);
		fread(m_pwUVsIndices, 2*3, nFC, hFile);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool CN3Skin_Load(FILE* hFile) {
	CN3IMesh_Load(hFile);
	return true;
}

//-----------------------------------------------------------------------------
bool CN3CPart_Load(FILE* hFile) {
	CN3BaseFileAccess_Load(hFile);

	int nL = 0;
	char szFN[256] = "";

	int m_dwReserved;
	fread(&m_dwReserved, sizeof(int), 1, hFile);
	_N3Material m_MtlOrg;
	fread(&m_MtlOrg, sizeof(_N3Material), 1, hFile);

	fread(&nL, sizeof(int), 1, hFile);
	if(nL > 0) {
		fread(szFN, sizeof(char), nL, hFile);
		N3LoadTexture(szFN);
	}

	fread(&nL, sizeof(int), 1, hFile);
	if(nL > 0) {
		fread(szFN, sizeof(char), nL, hFile); szFN[nL] = '\0';

		FILE* fpSkin = fopen(szFN, "rb");
		if(fpSkin == NULL) {
			fprintf(stderr, "\nERROR: Missing skin %s\n", szFN);
			return false;
		}

		CN3BaseFileAccess_Load(fpSkin);
		CN3Skin_Load(fpSkin);
	}

	return true;
}

//-----------------------------------------------------------------------------
e_ItemType MakeResrcFileNameForUPC(
	__TABLE_ITEM_BASIC* pItem, std::string* pszResrcFN,
	std::string* pszIconFN, e_PartPosition& ePartPosition,
	e_PlugPosition& ePlugPosition, e_Race eRace
) {
	ePartPosition = PART_POS_UNKNOWN;
	ePlugPosition = PLUG_POS_UNKNOWN;

	if(pszResrcFN) *pszResrcFN = "";
	if(pszIconFN) *pszIconFN = "";

	if(NULL == pItem) return ITEM_TYPE_UNKNOWN;

	e_ItemType eType = ITEM_TYPE_UNKNOWN;
	e_ItemPosition ePos = (e_ItemPosition)pItem->byAttachPoint;

	int iPos = 0;
	std::string szExt;

	if(ePos>=ITEM_POS_DUAL && ePos<=ITEM_POS_TWOHANDLEFT) {
		if(ITEM_POS_DUAL==ePos||ITEM_POS_RIGHTHAND==ePos||ITEM_POS_TWOHANDRIGHT==ePos)
			ePlugPosition = PLUG_POS_RIGHTHAND;
		else if(ITEM_POS_LEFTHAND==ePos||ITEM_POS_TWOHANDLEFT==ePos)
			ePlugPosition = PLUG_POS_LEFTHAND;
		eType = ITEM_TYPE_PLUG;
		szExt = ".n3cplug";
	} else if(ePos>=ITEM_POS_UPPER && ePos<=ITEM_POS_SHOES) {
		if(ITEM_POS_UPPER == ePos)
			ePartPosition = PART_POS_UPPER;
		else if(ITEM_POS_LOWER == ePos)
			ePartPosition = PART_POS_LOWER;
		else if(ITEM_POS_HEAD == ePos)
			ePartPosition = PART_POS_HAIR_HELMET;
		else if(ITEM_POS_GLOVES == ePos)
			ePartPosition = PART_POS_HANDS;
		else if(ITEM_POS_SHOES == ePos)
			ePartPosition = PART_POS_FEET;
		eType = ITEM_TYPE_PART;
		szExt = ".n3cpart";
		iPos = ePartPosition + 1;
	} else if(ePos>=ITEM_POS_EAR && ePos<=ITEM_POS_INVENTORY) {
		eType = ITEM_TYPE_ICONONLY;
		szExt = ".dxt";
	} else if(ePos == ITEM_POS_GOLD) {
		eType = ITEM_TYPE_GOLD;
		szExt = ".dxt";
	} else if(ePos == ITEM_POS_SONGPYUN) {
		eType = ITEM_TYPE_SONGPYUN;
		szExt = ".dxt";
	} else printf("Invalid Item Position\n");

	std::vector<char> buffer(256, NULL);
	if(pszResrcFN) {
		if(pItem->dwIDResrc) {
			if(eRace!=RACE_UNKNOWN && ePos>=/*ITEM_POS_DUAL*/ITEM_POS_UPPER && ePos<=ITEM_POS_SHOES) {
				// NOTE: no idea but perhaps this will work for now
				sprintf(&buffer[0], "Item\\%.1d_%.4d_%.2d_%.1d%s",
					(pItem->dwIDResrc / 10000000),
					((pItem->dwIDResrc / 1000) % 10000) + eRace,
					(pItem->dwIDResrc / 10) % 100,
					pItem->dwIDResrc % 10,
					szExt.c_str()
				);
			} else {
				sprintf(&buffer[0], "Item\\%.1d_%.4d_%.2d_%.1d%s",
					(pItem->dwIDResrc / 10000000),
					(pItem->dwIDResrc / 1000) % 10000,
					(pItem->dwIDResrc / 10) % 100,
					pItem->dwIDResrc % 10,
					szExt.c_str()
				);
			}

			*pszResrcFN = &buffer[0];
		} else {
			*pszResrcFN = "";
		}
	}

	if(pszIconFN) {
		sprintf(&buffer[0],	"UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt",
			(pItem->dwIDIcon / 10000000), 
			(pItem->dwIDIcon / 1000) % 10000, 
			(pItem->dwIDIcon / 10) % 100, 
			pItem->dwIDIcon % 10
		);

		*pszIconFN = &buffer[0];
	}

	return eType;
}

//-----------------------------------------------------------------------------
void GenerateScene(void) {

	float* vertices = NULL;
	uint32_t* elements = NULL;

	int iVC = 0;
	int iFC = 0;

	if(eType == ITEM_TYPE_PLUG) {
		vertices = new float[5*m_iMaxNumVertices0];
		memset(vertices, 0, 5*m_iMaxNumVertices0);

		for(int i=0; i<m_iMaxNumVertices0; i++) {
			vertices[5*i+0] = m_pVertices0[i].x;
			vertices[5*i+1] = m_pVertices0[i].y;
			vertices[5*i+2] = m_pVertices0[i].z;

			vertices[5*i+3] = m_pVertices0[i].tu;
			vertices[5*i+4] = m_pVertices0[i].tv;
		}

		iVC = m_iMaxNumVertices0;
		iFC = m_iMaxNumIndices0/3;

		elements = new uint32_t[m_iMaxNumIndices0];
		memset(elements, 0, m_iMaxNumIndices0*sizeof(uint32_t));
		for(int i=0; i<m_iMaxNumIndices0; i++) {
			elements[i] = (uint32_t) m_pIndices0[i];
		}

	} else if(eType == ITEM_TYPE_PART) {
		vertices = new float[5*3*m_nFC];
		memset(vertices, 0, 5*3*m_nFC*sizeof(float));

		for(int i=0; i<3*m_nFC; i++) {
			vertices[5*i+0] = m_pVertices[m_pwVtxIndices[i]].x;
			vertices[5*i+1] = m_pVertices[m_pwVtxIndices[i]].y;
			vertices[5*i+2] = m_pVertices[m_pwVtxIndices[i]].z;

			vertices[5*i+3] = m_pfUVs[2*m_pwUVsIndices[i]+0];
			vertices[5*i+4] = m_pfUVs[2*m_pwUVsIndices[i]+1];
		}

		iVC = 3*m_nFC;
		iFC = m_nFC;

		elements = new uint32_t[3*m_nFC];
		memset(elements, 0, 3*m_nFC*sizeof(uint32_t));
		for(int i=0; i<3*m_nFC; i++) {
			elements[i] = (uint32_t) (i);
		}

	} else if(eType==ITEM_TYPE_ICONONLY || eType==ITEM_TYPE_GOLD || eType==ITEM_TYPE_SONGPYUN) {
		// NOTE: need to offset the position to get the icon to display in the center
		float _vertices[] = {
			-0.25f+0.25f/4.0f,  0.25f-0.25f/4.0f, -0.1f, 0.0f, 0.0f, // Top-left
			 0.25f+0.25f/4.0f,  0.25f-0.25f/4.0f, -0.1f, 1.0f, 0.0f, // Top-right
			 0.25f+0.25f/4.0f, -0.25f-0.25f/4.0f, -0.1f, 1.0f, 1.0f, // Bottom-right
			-0.25f+0.25f/4.0f, -0.25f-0.25f/4.0f, -0.1f, 0.0f, 1.0f  // Bottom-left
		};

		vertices = new float[5*3*2];
		memset(vertices, 0, 5*3*2*sizeof(float));
		for(int i=0; i<5*3*2; i++) {
			vertices[i] = _vertices[i];
		}

		iVC = 3*2;
		iFC = 2;

		uint32_t _elements[] = {
			0, 1, 2,
			2, 3, 0
		};

		elements = new uint32_t[3*2];
		memset(elements, 0, 3*2*sizeof(uint32_t));
		for(int i=0; i<3*2; i++) {
			elements[i] = _elements[i];
		}
	}

	//----

	m_Scene.mRootNode = new aiNode();

	m_Scene.mMaterials = new aiMaterial*[1];
	m_Scene.mMaterials[0] = NULL;
	m_Scene.mNumMaterials = 1;

	m_Scene.mMaterials[0] = new aiMaterial();

	aiString strTex(pTexName);
	m_Scene.mMaterials[0]->AddProperty(
		&strTex, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0)
	);

	if(vertices!=NULL && elements!=NULL) {

		m_Scene.mMeshes = new aiMesh*[1];
		m_Scene.mMeshes[0] = NULL;
		m_Scene.mNumMeshes = 1;

		m_Scene.mMeshes[0] = new aiMesh();
		m_Scene.mMeshes[0]->mMaterialIndex = 0;

		m_Scene.mRootNode->mMeshes = new uint32_t[1];
		m_Scene.mRootNode->mMeshes[0] = 0;
		m_Scene.mRootNode->mNumMeshes = 1;

		aiMesh* pMesh = m_Scene.mMeshes[0];

		pMesh->mVertices = new aiVector3D[iVC];
		pMesh->mNumVertices = iVC;

		pMesh->mTextureCoords[0] = new aiVector3D[iVC];
		pMesh->mNumUVComponents[0] = iVC;

		for (int i = 0; i < iVC; ++i)
		{
			Vertex v;
			v.x = vertices[5*i+0];
			v.y = vertices[5*i+1];
			v.z = vertices[5*i+2];
			v.u = vertices[5*i+3];
			v.v = vertices[5*i+4];

			pMesh->mVertices[i] = aiVector3D(v.x, v.y, v.z);
			pMesh->mTextureCoords[0][i] = aiVector3D(v.u, (1.0f-v.v), 0);
		}

		pMesh->mFaces = new aiFace[iFC];
		pMesh->mNumFaces = iFC;

		for (int i = 0; i < iFC; ++i)
		{
			aiFace& face = pMesh->mFaces[i];

			face.mIndices = new uint32_t[3];
			face.mNumIndices = 3;

			face.mIndices[0] = elements[3*i+0];
			face.mIndices[1] = elements[3*i+1];
			face.mIndices[2] = elements[3*i+2];
		}
	} else {
		printf("Failed!\n");
		printf("\nERROR: Mesh data missing!\n");
		system("pause");
		exit(-1);
	}

	delete vertices;
	delete elements;

	printf("Success!\n");
}

//-----------------------------------------------------------------------------
bool N3MeshConverter::Convert(char* filename) {
	Assimp::Exporter* pExporter = new Assimp::Exporter();

	int len_fn = strlen(filename);
	char* exten = &filename[len_fn-7];

	if(!strcmp(exten, "n3cplug")) {

		eType = ITEM_TYPE_PLUG;

		char tmp[0xFF] = "";
		sprintf(tmp, "./item/%s", filename);
		FILE* pFile = fopen(tmp, "rb");
		if(pFile == NULL) {
			fprintf(stderr, "ERROR: Missing N3Plug %s\n", tmp);
			delete pExporter;
			return false;
		}

		CN3CPlug_Load(pFile);
		fclose(pFile);
	} else if(!strcmp(exten, "n3cpart")) {

		eType = ITEM_TYPE_PART;

		char tmp[0xFF] = "";
		sprintf(tmp, "./item/%s", filename);
		FILE* pFile = fopen(tmp, "rb");
		if(pFile == NULL) {
			fprintf(stderr, "ERROR: Missing N3Part %s\n", tmp);
			delete pExporter;
			return false;
		}

		CN3CPart_Load(pFile);
		fclose(pFile);
	}

	printf("\n\"%s\"", filename);
	printf("\nGenerating scene... ");
	GenerateScene();
	printf("\nExporting to %s... ", "obj");

	char output_fn[0xFFFF] = {};
	filename[len_fn-8] = '\0';
	sprintf(output_fn, "./Item_output/%s.obj", filename);

	aiReturn ret = pExporter->Export(&m_Scene, "obj", output_fn);
	if(ret == aiReturn_SUCCESS) {
		printf("Done!\n");
	} else {
		printf("Failed!\n");
		delete pExporter;
		return false;
	}

	delete pExporter;
	return true;
}
