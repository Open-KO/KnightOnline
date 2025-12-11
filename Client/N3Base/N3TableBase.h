// N3TableBase.h: interface for the CN3TableBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TABLEBASE_H__DD4F005E_05B0_49E3_883E_94BE6C8AC7EF__INCLUDED_)
#define AFX_N3TABLEBASE_H__DD4F005E_05B0_49E3_883E_94BE6C8AC7EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <map>

#include "My_3DStruct.h" // _ASSERT

#ifdef _N3GAME
#include "LogWriter.h"
#endif

enum TBL_DATA_TYPE {DT_NONE, DT_CHAR, DT_BYTE, DT_SHORT, DT_WORD, DT_INT, DT_DWORD, DT_STRING, DT_FLOAT, DT_DOUBLE};

template <typename Type> class CN3TableBase
{
public:
	using DATA_TYPE = TBL_DATA_TYPE;
	using MAP_TYPE = std::map<uint32_t, Type>;

	CN3TableBase();
	virtual ~CN3TableBase();

// Attributes
protected:
	std::vector<DATA_TYPE> m_DataTypes;	// 실제 사용되는 정보의 데이타 타입
	MAP_TYPE m_Datas; // 실제 사용되는 정보

// Operations
public:
	inline const MAP_TYPE& GetMap() const {
		return m_Datas;
	}

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

			WriteFile(hFile, &cWrite, sizeof(cWrite), &dwNum, nullptr);
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

			WriteFile(hFile, &byteWrite, sizeof(byteWrite), &dwNum, nullptr);
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

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, nullptr);
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

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, nullptr);
		}
		break;
	case DT_INT:
		{
			int iWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] )	iWrite = atoi(lpszData);
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, nullptr);
		}
		break;
	case DT_DWORD:
		{
			uint32_t iWrite;
			if (isdigit(lpszData[0]) )	iWrite = strtoul(lpszData, nullptr, 10);
			else return FALSE;		// 문자는 안되~!

			WriteFile(hFile, &iWrite, sizeof(iWrite), &dwNum, nullptr);
		}
		break;
	case DT_STRING:
		{
			std::string& szString = *((std::string*)lpszData);
			int iStrLen = szString.size();
			WriteFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, nullptr);
			if (iStrLen>0) WriteFile(hFile, &(szString[0]), iStrLen, &dwNum, nullptr);
		}
		break;
	case DT_FLOAT:
		{
			float fWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] ||
				'.' == lpszData[0] )	fWrite = (float)atof(lpszData);
			else return FALSE;	// 문자는 안되~!
			WriteFile(hFile, &fWrite, sizeof(fWrite), &dwNum, nullptr);
		}
		break;
	case DT_DOUBLE:
		{
			double dWrite;
			if (isdigit(lpszData[0]) || '-' == lpszData[0] ||
				'.' == lpszData[0] )	dWrite = atof(lpszData);
			WriteFile(hFile, &dWrite, sizeof(dWrite), &dwNum, nullptr);
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
			ReadFile(hFile, pData, sizeof(char), &dwNum, nullptr);
		}
		break;
	case DT_BYTE:
		{
			ReadFile(hFile, pData, sizeof(uint8_t), &dwNum, nullptr);
		}
		break;
	case DT_SHORT:
		{
			ReadFile(hFile, pData, sizeof(int16_t), &dwNum, nullptr);
		}
		break;
	case DT_WORD:
		{
			ReadFile(hFile, pData, sizeof(uint16_t), &dwNum, nullptr);
		}
		break;
	case DT_INT:
		{
			ReadFile(hFile, pData, sizeof(int), &dwNum, nullptr);
		}
		break;
	case DT_DWORD:
		{
			ReadFile(hFile, pData, sizeof(uint32_t), &dwNum, nullptr);
		}
		break;
	case DT_STRING:
		{
			std::string& szString = *((std::string*)pData);
			
			int iStrLen = 0;
			ReadFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, nullptr);

			szString = "";
			if (iStrLen>0)
			{
				szString.assign(iStrLen, ' ');
				ReadFile(hFile, &(szString[0]), iStrLen, &dwNum, nullptr);
			}
		}
		break;
	case DT_FLOAT:
		{
			ReadFile(hFile, pData, sizeof(float), &dwNum, nullptr);
		}
		break;
	case DT_DOUBLE:
		{
			ReadFile(hFile, pData, sizeof(double), &dwNum, nullptr);
		}
		break;

	case DT_NONE:
	default:
		__ASSERT(0,"");
		return FALSE;
	}
	return TRUE;
}

template <class Type>
BOOL CN3TableBase<Type>::LoadFromFile(const std::string& szFN)
{
	if(szFN.empty()) return FALSE;

	HANDLE hFile = ::CreateFile(szFN.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

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
	::ReadFile(hFile, pDatas, dwSizeLow, &dwRWC, nullptr); // 암호화된 데이터 읽고..
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
	for(uint32_t i = 0; i < dwSizeLow; i++)
	{
		uint8_t byData = (pDatas[i] ^ (key_r>>8));
		key_r = (pDatas[i] + key_r) * key_c1 + key_c2;
		pDatas[i] = byData;
	}

	// 임시 파일에 쓴다음.. 다시 연다..
	hFile = ::CreateFile(szFNTmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	::WriteFile(hFile, pDatas, dwSizeLow, &dwRWC, nullptr); // 임시파일에 암호화 풀린 데이터 쓰기
	CloseHandle(hFile); // 임시 파일 닫기
	delete [] pDatas; pDatas = nullptr;

	hFile = ::CreateFile(szFNTmp.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr); // 임시 파일 읽기 모드로 열기.

	




	
	
	
	
	BOOL bResult = Load(hFile);

	CloseHandle(hFile);

	if (!bResult)
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
	ReadFile(hFile, &iDataTypeCount, 4, &dwNum, nullptr);			// (엑셀에서 column 수)

	std::vector<int> offsets;
	__ASSERT(iDataTypeCount>0, "Data Type 이 0 이하입니다.");
	if (iDataTypeCount>0)
	{
		m_DataTypes.insert(m_DataTypes.begin(), iDataTypeCount, DT_NONE);
		ReadFile(hFile, &(m_DataTypes[0]), sizeof(DATA_TYPE)*iDataTypeCount, &dwNum, nullptr);	// 각각의 column에 해당하는 data type

		if(FALSE == MakeOffsetTable(offsets))
		{
			__ASSERT(0, "can't make offset table");
			return FALSE;	// structure변수에 대한 offset table 만들어주기
		}

		int iSize = offsets[iDataTypeCount];	// MakeOffstTable 함수에서 리턴되는 값중 m_iDataTypeCount번째에 이 함수의 실제 사이즈가 들어있다.
		if (sizeof(Type) != iSize ||		// 전체 type의 크기와 실제 구조체의 크기와 다르거나 
			DT_DWORD != m_DataTypes[0] )	// 맨 처음의 데이타가 DT_DWORD형이 아닐때(맨처음은 고유한 ID이므로)
		{
			m_DataTypes.clear();
			__ASSERT(0, "DataType is mismatch or DataSize is incorrect!!");
			return FALSE;
		}
	}

	// row 가 몇줄인지 읽기
	int iRC;
	ReadFile(hFile, &iRC, sizeof(iRC), &dwNum, nullptr);
	Type Data;
	for (i=0; i<iRC; ++i)
	{
		for (j=0; j<iDataTypeCount; ++j)
		{
			ReadData(hFile, m_DataTypes[j], (char*)(&Data) + offsets[j]);
		}

		uint32_t dwKey = *((uint32_t*)(&Data));
		auto pt = m_Datas.insert(std::make_pair(dwKey, Data));

		__ASSERT(pt.second, "CN3TableBase<Type> : Key 중복 경고.");
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
	__ASSERT(0,"");
	return 0;
}

// structure는 4바이트 정렬하여서 메모리를 잡는다. 따라서 아래 함수가 필요하다.
// 아래 함수로 OffsetTable을 만들어 쓴 후에는 만드시 리턴값을 delete [] 를 해주어야 한다.
template <class Type>
BOOL CN3TableBase<Type>::MakeOffsetTable(std::vector<int>& offsets)
{
	if (m_DataTypes.empty())
		return false;

	static constexpr int StructAlignment = alignof(Type);

	int iDataTypeCount = (int) m_DataTypes.size();

	offsets.clear();
	offsets.resize(iDataTypeCount + 1);
	offsets[0] = 0;

	int iPrevDataSize = SizeOf(m_DataTypes[0]);
	for (int i = 1; i < iDataTypeCount; i++)
	{
		int iCurDataSize = SizeOf(m_DataTypes[i]);
		int iPreviousOffset = offsets[i - 1];

		int modulo = (iCurDataSize % StructAlignment);
		if (0 == modulo)
		{
			modulo = (iPreviousOffset + iPrevDataSize) % StructAlignment;
			if (0 == modulo)
				offsets[i] = iPreviousOffset + iPrevDataSize;
			else
				offsets[i] = ((int) (iPreviousOffset + iPrevDataSize + (StructAlignment - 1)) / StructAlignment) * StructAlignment;
		}
		else if (1 == modulo)
		{
			offsets[i] = iPreviousOffset + iPrevDataSize;
		}
		else if (2 == modulo)
		{
			modulo = ((iPreviousOffset + iPrevDataSize) % 2);
			if (0 == modulo)
				offsets[i] = iPreviousOffset + iPrevDataSize;
			else
				offsets[i] = iPreviousOffset + iPrevDataSize + 1; // NOTE: Effectively this is (2 - modulo), but modulo can only be 1 here.
		}
		else if (4 == modulo)
		{
			modulo = ((iPreviousOffset + iPrevDataSize) % 4);
			if (0 == modulo)
				offsets[i] = iPreviousOffset + iPrevDataSize;
			else
				offsets[i] = iPreviousOffset + iPrevDataSize + (4 - modulo);
		}
		else
		{
			__ASSERT(0, "");
		}

		iPrevDataSize = iCurDataSize;
	}

	offsets[iDataTypeCount] = ((int) (offsets[iDataTypeCount - 1] + iPrevDataSize + (StructAlignment - 1)) / StructAlignment) * StructAlignment;
	return true;
}

#endif // !defined(AFX_N3TABLEBASE_H__DD4F005E_05B0_49E3_883E_94BE6C8AC7EF__INCLUDED_)
