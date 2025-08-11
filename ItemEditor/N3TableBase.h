#ifndef __N3TABLEBASE_H_
#define __N3TABLEBASE_H_

#include <vector>
#include <map>

//-----------------------------------------------------------------------------
std::wstring s2ws(const std::string& s);

//-----------------------------------------------------------------------------
template <typename Type> class CN3TableBase
{
public:
	CN3TableBase();
	virtual ~CN3TableBase();

// Attributes
protected:
	enum DATA_TYPE {DT_NONE, DT_CHAR, DT_BYTE, DT_SHORT, DT_WORD, DT_INT, DT_DWORD, DT_STRING, DT_FLOAT, DT_DOUBLE};
	typedef typename std::vector<DATA_TYPE>::iterator it_DataType;
	
	typedef typename std::map<uint32_t, Type>::iterator		it_Table;
	typedef typename std::map<uint32_t, Type>::value_type	val_Table;
	typedef typename std::pair<typename std::map<uint32_t, Type>::iterator, bool> pair_Table;

	typename std::vector<DATA_TYPE> m_DataTypes;	// 실제 사용되는 정보의 데이타 타입
	typename std::map<uint32_t, Type> m_Datas; // 실제 사용되는 정보

// Operations
public:
	void	Release();

	Type* Find(uint32_t dwID) // ID로 data 찾기
	{
		auto it = m_Datas.find(dwID);
		if (it == m_Datas.end())
			return nullptr; // 찾기에 실패 했다!~!!
		
		return &it->second;
	}

	int GetSize() const
	{
		return static_cast<int>(m_Datas.size());
	}

	Type* GetIndexedData(int index)	//index로 찾기..
	{
		if (index < 0
			|| index >= static_cast<int>(m_Datas.size()))
			return nullptr;
		
		auto it = m_Datas.begin();
		std::advance(it, index);
		return &it->second;
	}

	bool IDToIndex(uint32_t dwID, int* index) // 해당 ID의 Index 리턴..	Skill에서 쓴다..
	{
		auto it = m_Datas.find(dwID);
		if (it == m_Datas.end())
			return false; // 찾기에 실패 했다!~!!

		auto itSkill = m_Datas.begin();
		int iSize = static_cast<int>(m_Datas.size());
		for (int i = 0; i < iSize; i++, itSkill++)
		{
			if (itSkill == it)
			{
				*index = i;
				return true;
			}
		}
		return false;
	}

	BOOL	LoadFromFile(const std::string& szFN);

protected:
	BOOL	Load(HANDLE hFile);
	BOOL	WriteData(HANDLE hFile, DATA_TYPE DataType, const char* lpszData);
	BOOL	ReadData(HANDLE hFile, DATA_TYPE DataType, void* pData);

	int		SizeOf(DATA_TYPE DataType) const;
	BOOL	MakeOffsetTable(std::vector<int>& offsets);
};



// cpp파일에 있으니까 link에러가 난다. 왜 그럴까?

template <class Type>
CN3TableBase<Type>::CN3TableBase()
{
}

template <class Type>
CN3TableBase<Type>::~CN3TableBase()
{
	Release();
}

template <class Type>
void CN3TableBase<Type>::Release()
{
	m_DataTypes.clear(); // data type 저장한것 지우기
	m_Datas.clear(); // row 데이타 지우기
}

// 파일에 데이타 타입별로 쓰기..
template <class Type>
BOOL CN3TableBase<Type>::WriteData(HANDLE hFile, DATA_TYPE DataType, const char* lpszData)
{
	DWORD dwNum;
	switch(DataType)
	{
	case DT_CHAR:
		{
			char cWrite;
			if (isdigit(lpszData[0]))
			{
				int iTemp = atoi(lpszData);
				if (iTemp < -127 || iTemp > 128) return FALSE; // 범위가 벗어났어~
				cWrite = (char)iTemp;
			}
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &cWrite, sizeof(cWrite), &dwNum, NULL);
		}
		break;
	case DT_BYTE:
		{
			uint8_t byteWrite;
			if (isdigit(lpszData[0]))
			{
				int iTemp = atoi(lpszData);
				if (iTemp < 0 || iTemp > 255) return FALSE; // 범위가 벗어났어~
				byteWrite = (uint8_t)iTemp;
			}
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &byteWrite, sizeof(byteWrite), &dwNum, NULL);
		}
		break;
	case DT_SHORT:
		{
			int16_t iWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] )
			{
				int iTemp = atoi(lpszData);
				if (iTemp < -32767 || iTemp > 32768) return FALSE; // 범위가 벗어났어~
				iWrite = (int16_t)iTemp;
			}
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, NULL);
		}
		break;
	case DT_WORD:
		{
			uint16_t iWrite;
			if (isdigit(lpszData[0]) )
			{
				int iTemp = atoi(lpszData);
				if (iTemp < 0 || iTemp > 65535) return FALSE; // 범위가 벗어났어~
				iWrite = (int16_t)iTemp;
			}
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, NULL);
		}
		break;
	case DT_INT:
		{
			int iWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] )	iWrite = atoi(lpszData);
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, NULL);
		}
		break;
	case DT_DWORD:
		{
			uint32_t iWrite;
			if (isdigit(lpszData[0]) )	iWrite = strtoul(lpszData, NULL, 10);
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, NULL);
		}
		break;
	case DT_STRING:
		{
			std::string& szString = *((std::string*)lpszData);
			int iStrLen = szString.size();
			WriteFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, NULL);
			if (iStrLen>0) WriteFile(hFile, &(szString[0]), iStrLen, &dwNum, NULL);
		}
		break;
	case DT_FLOAT:
		{
			float fWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] ||
				'.' == lpszData[0] )	fWrite = (float)atof(lpszData);
			else return FALSE;	// 문자는 안되~!
			WriteFile(hFile, &fWrite, sizeof(fWrite), &dwNum, NULL);
		}
		break;
	case DT_DOUBLE:
		{
			double dWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] ||
				'.' == lpszData[0] )	dWrite = atof(lpszData);
			WriteFile(hFile, &dWrite, sizeof(dWrite), &dwNum, NULL);
		}
		break;

	case DT_NONE:
	default:
		__ASSERT(0,"");
	}
	return TRUE;
}

template <class Type>
BOOL CN3TableBase<Type>::ReadData(HANDLE hFile, DATA_TYPE DataType, void* pData)
{
	DWORD dwNum;
	switch(DataType)
	{
	case DT_CHAR:
		{
			ReadFile(hFile, pData, sizeof(char), &dwNum, NULL);
		}
		break;
	case DT_BYTE:
		{
			ReadFile(hFile, pData, sizeof(uint8_t), &dwNum, NULL);
		}
		break;
	case DT_SHORT:
		{
			ReadFile(hFile, pData, sizeof(int16_t), &dwNum, NULL);
		}
		break;
	case DT_WORD:
		{
			ReadFile(hFile, pData, sizeof(uint16_t), &dwNum, NULL);
		}
		break;
	case DT_INT:
		{
			ReadFile(hFile, pData, sizeof(int), &dwNum, NULL);
		}
		break;
	case DT_DWORD:
		{
			ReadFile(hFile, pData, sizeof(uint32_t), &dwNum, NULL);
		}
		break;
	case DT_STRING:
		{
			std::string& szString = *((std::string*)pData);
			
			int iStrLen = 0;
			ReadFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, NULL);

			szString = "";
			if (iStrLen>0)
			{
				szString.assign(iStrLen, ' ');
				ReadFile(hFile, &(szString[0]), iStrLen, &dwNum, NULL);
			}
		}
		break;
	case DT_FLOAT:
		{
			ReadFile(hFile, pData, sizeof(float), &dwNum, NULL);
		}
		break;
	case DT_DOUBLE:
		{
			ReadFile(hFile, pData, sizeof(double), &dwNum, NULL);
		}
		break;

	case DT_NONE:
	default:
		return FALSE;
	}
	return TRUE;
}

template <class Type>
BOOL CN3TableBase<Type>::LoadFromFile(const std::string& szFN)
{
	if(szFN.empty()) return FALSE;

	std::wstring wStr = s2ws(szFN);
	HANDLE hFile = ::CreateFile(wStr.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(INVALID_HANDLE_VALUE == hFile)
	{
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - Can't open file(read) File Handle error ({})", szFN);
#endif
		return FALSE;
	}


	
	
	
	
	
	
	
	
	// 파일 암호화 풀기.. .. 임시 파일에다 쓴다음 ..
	std::string szFNTmp = szFN + ".tmp";
	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
	if(dwSizeLow <= 0)
	{
		CloseHandle(hFile);
		::remove(szFNTmp.c_str()); // 임시 파일 지우기..
		return FALSE;
	}

	// 원래 파일을 읽고..
	uint8_t* pDatas = new uint8_t[dwSizeLow];
	DWORD dwRWC = 0;
	::ReadFile(hFile, pDatas, dwSizeLow, &dwRWC, NULL); // 암호화된 데이터 읽고..
	CloseHandle(hFile); // 원래 파일 닫고

// 테이블 만드는 툴에서 쓰는 키와 같은 키..
	uint16_t key_r = 0x0816;
	uint16_t key_c1 = 0x6081;
	uint16_t key_c2 = 0x1608;

//uint8_t Encrypt(uint8_t plain)
//{
//	uint8_t cipher;
//	cipher = (plain ^ (key_r>>8));
//	key_r = (cipher + key_r) * key_c1 + key_c2;
//	return cipher;
//}

//uint8_t Decrypt(uint8_t cipher)
//{
//	uint8_t plain;
//	plain = (cipher ^ (m_r>>8));
//	m_r = (cipher + m_r) * m_c1 + m_c2;
//	return plain;
//}

	// 암호화 풀고..
	for (uint32_t i = 0; i < dwSizeLow; i++)
	{
		uint8_t byData = (pDatas[i] ^ (key_r>>8));
		key_r = (pDatas[i] + key_r) * key_c1 + key_c2;
		pDatas[i] = byData;
	}

	// 임시 파일에 쓴다음.. 다시 연다..
	std::wstring wStr2 = s2ws(szFNTmp);
	hFile = ::CreateFile(wStr2.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	::WriteFile(hFile, pDatas, dwSizeLow, &dwRWC, NULL); // 임시파일에 암호화 풀린 데이터 쓰기
	CloseHandle(hFile); // 임시 파일 닫기
	delete [] pDatas; pDatas = NULL;

	hFile = ::CreateFile(wStr2.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // 임시 파일 읽기 모드로 열기.

	




	
	
	
	
	BOOL bResult = Load(hFile);

	CloseHandle(hFile);

	if (FALSE == bResult)
	{
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - incorrect table ({})", szFN);
#endif
	}


	// 임시 파일 지우기..
	::remove(szFNTmp.c_str());

	return bResult;
}

template <class Type>
BOOL CN3TableBase<Type>::Load(HANDLE hFile)
{
	Release();

	// data(column) 의 구조가 어떻게 되어 있는지 읽기
	DWORD dwNum;
	int i, j, iDataTypeCount = 0;
	ReadFile(hFile, &iDataTypeCount, 4, &dwNum, NULL);			// (엑셀에서 column 수)

	std::vector<int> offsets;
	if (iDataTypeCount>0)
	{
		m_DataTypes.insert(m_DataTypes.begin(), iDataTypeCount, DT_NONE);
		ReadFile(hFile, &(m_DataTypes[0]), sizeof(DATA_TYPE)*iDataTypeCount, &dwNum, NULL);	// 각각의 column에 해당하는 data type

		if(FALSE == MakeOffsetTable(offsets))
		{
			return FALSE;	// structure변수에 대한 offset table 만들어주기
		}

		int iSize = offsets[iDataTypeCount];	// MakeOffstTable 함수에서 리턴되는 값중 m_iDataTypeCount번째에 이 함수의 실제 사이즈가 들어있다.
		if (sizeof(Type) != iSize ||		// 전체 type의 크기와 실제 구조체의 크기와 다르거나 
			DT_DWORD != m_DataTypes[0] )	// 맨 처음의 데이타가 DT_DWORD형이 아닐때(맨처음은 고유한 ID이므로)
		{
			m_DataTypes.clear();
			return FALSE;
		}
	}

	// row 가 몇줄인지 읽기
	int iRC;
	ReadFile(hFile, &iRC, sizeof(iRC), &dwNum, NULL);
	Type Data;
	for (i=0; i<iRC; ++i)
	{
		for (j=0; j<iDataTypeCount; ++j)
		{
			ReadData(hFile, m_DataTypes[j], (char*)(&Data) + offsets[j]);
		}

		uint32_t dwKey = *((uint32_t*)(&Data));
		pair_Table pt = m_Datas.insert(val_Table(dwKey, Data));

	}
	return TRUE;
}

template <class Type>
int CN3TableBase<Type>::SizeOf(DATA_TYPE DataType) const
{
	switch(DataType)
	{
	case DT_CHAR:
		return sizeof(char);
	case DT_BYTE:
		return sizeof(uint8_t);
	case DT_SHORT:
		return sizeof(int16_t);
	case DT_WORD:
		return sizeof(uint16_t);
	case DT_INT:
		return sizeof(int);
	case DT_DWORD:
		return sizeof(uint32_t);
	case DT_STRING:
		return sizeof(std::string);
	case DT_FLOAT:
		return sizeof(float);
	case DT_DOUBLE:
		return sizeof(double);
	}
	return 0;
}

// structure는 4바이트 정렬하여서 메모리를 잡는다. 따라서 아래 함수가 필요하다.
// 아래 함수로 OffsetTable을 만들어 쓴 후에는 만드시 리턴값을 delete [] 를 해주어야 한다.
template <class Type>
BOOL CN3TableBase<Type>::MakeOffsetTable(std::vector<int>& offsets)
{	
	if (m_DataTypes.empty()) return false;

	int i, iDataTypeCount = m_DataTypes.size();
	offsets.clear();
	offsets.resize(iDataTypeCount+1);	// +1을 한 이유는 맨 마지막 값에 Type의 실제 사이즈를 넣기 위해서
	offsets[0] = 0;
	int iPrevDataSize = SizeOf(m_DataTypes[0]);
	for (i=1; i<iDataTypeCount; ++i)
	{
		int iCurDataSize = SizeOf(m_DataTypes[i]);
		if (1 == iCurDataSize%4)	// 현재 데이터가 1바이트면 그냥 이전 데이터가 몇바이트든 상관 없다.
		{
			offsets[i] = offsets[i-1] + iPrevDataSize;
		}
		else if (2 == iCurDataSize%4) // 현재 데이터가 2바이트면 짝수번지에 위치해야 한다.
		{
			if (0 == ((offsets[i-1]+iPrevDataSize) % 2))
				offsets[i] = offsets[i-1] + iPrevDataSize;
			else
				offsets[i] = offsets[i-1] + iPrevDataSize+1;
		}
		else if (0 == iCurDataSize%4) // 현재 데이터가 4바이트면 4의 배수번지에 위치해야 한다.
		{
			if (0 == ((offsets[i-1]+iPrevDataSize) % 4))
				offsets[i] = offsets[i-1] + iPrevDataSize;
			else
				offsets[i] = ((int)(offsets[i-1] + iPrevDataSize + 3)/4)*4;	// 4의 배수로 만들기
		}
		iPrevDataSize = iCurDataSize;
	}

	// 맨 마지막 값에 Type의 실제 사이즈를 넣자.
	offsets[iDataTypeCount] = ((int)(offsets[iDataTypeCount-1] + iPrevDataSize + 3)/4)*4;	// 4의 배수로 만들기

	return true;
}

#endif
