#include "StdAfxBase.h"
#include "N3TableBaseImpl.h"

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>

#ifdef _N3GAME
#include "LogWriter.h"
#endif

CN3TableBaseImpl::CN3TableBaseImpl()
{
}

CN3TableBaseImpl::~CN3TableBaseImpl()
{
}

bool CN3TableBaseImpl::LoadFromFile(const std::string& szFN)
{
	if (szFN.empty())
		return false;

	FileReader encryptedFile;
	if (!encryptedFile.OpenExisting(szFN))
	{
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - Can't open file(read) File Handle error ({})", szFN);
#endif
		return false;
	}

	std::error_code ec;

	// 파일 암호화 풀기.. .. 임시 파일에다 쓴다음 ..
	std::string szFNTmp      = szFN + ".tmp";
	size_t encryptedFileSize = static_cast<size_t>(encryptedFile.Size());
	if (encryptedFileSize == 0)
	{
		encryptedFile.Close();
		std::filesystem::remove(szFNTmp, ec); // 임시 파일 지우기..
		return false;
	}

	// 원래 파일을 읽고..
	uint8_t* pDatas = new uint8_t[encryptedFileSize];
	encryptedFile.Read(pDatas, encryptedFileSize); // 암호화된 데이터 읽고..
	encryptedFile.Close();                         // 원래 파일 닫고

												   // 테이블 만드는 툴에서 쓰는 키와 같은 키..
	uint16_t key_r  = 0x0816;
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
	for (uint32_t i = 0; i < encryptedFileSize; i++)
	{
		uint8_t byData = (pDatas[i] ^ (key_r >> 8));
		key_r          = (pDatas[i] + key_r) * key_c1 + key_c2;
		pDatas[i]      = byData;
	}

	// TODO: Rather than write to file to read it back again, we should just read it from a memory stream.

	// 임시 파일에 쓴다음.. 다시 연다..
	{
		FileWriter tmpFileWriter;
		if (!tmpFileWriter.Create(szFNTmp))
		{
			tmpFileWriter.Close();
			delete[] pDatas;
			return false;
		}

		tmpFileWriter.Write(pDatas, encryptedFileSize); // 임시파일에 암호화 풀린 데이터 쓰기
	}

	delete[] pDatas;
	pDatas = nullptr;

	// 임시 파일 읽기 모드로 열기.
	FileReader decryptedFile;
	if (!decryptedFile.OpenExisting(szFNTmp))
	{
		std::filesystem::remove(szFNTmp, ec);
		return false;
	}

	bool bResult = Load(decryptedFile);
	decryptedFile.Close();

	if (!bResult)
	{
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - incorrect table ({})", szFN);
#endif
	}

	// 임시 파일 지우기..
	std::filesystem::remove(szFNTmp, ec);

	return bResult;
}

bool CN3TableBaseImpl::ReadData(File& file, DATA_TYPE DataType, void* pData)
{
	switch (DataType)
	{
		case DT_CHAR:
			file.Read(pData, sizeof(char));
			break;

		case DT_BYTE:
			file.Read(pData, sizeof(uint8_t));
			break;

		case DT_SHORT:
			file.Read(pData, sizeof(int16_t));
			break;

		case DT_WORD:
			file.Read(pData, sizeof(uint16_t));
			break;

		case DT_INT:
			file.Read(pData, sizeof(int));
			break;

		case DT_DWORD:
			file.Read(pData, sizeof(uint32_t));
			break;

		case DT_STRING:
		{
			std::string& szString = *((std::string*) pData);

			int iStrLen           = 0;
			file.Read(&iStrLen, sizeof(iStrLen));

			szString.clear();
			if (iStrLen > 0)
			{
				szString.assign(iStrLen, ' ');
				file.Read(&szString[0], iStrLen);
			}
		}
		break;

		case DT_FLOAT:
			file.Read(pData, sizeof(float));
			break;

		case DT_DOUBLE:
			file.Read(pData, sizeof(double));
			break;

		case DT_NONE:
		default:
			__ASSERT(0, "");
			return false;
	}

	return true;
}

int CN3TableBaseImpl::SizeOf(DATA_TYPE DataType) const
{
	switch (DataType)
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

		default:
			break;
	}

	__ASSERT(0, "");
	return 0;
}
