#include "StdAfx.h"
#include "TblEditorBase.h"
#include "resource.h"

#include <vector>
#include <map>

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr int MAX_COLUMN_COUNT	= 10'000;
static constexpr int MAX_ROW_COUNT		= 100'000;
static constexpr int MAX_STRING_LENGTH	= 50'000;

struct WriteBuffer : std::vector<uint8_t>
{
	template <typename T>
	void append(const T* value)
	{
		append(value, sizeof(T));
	}

	void append(const void* value, size_t length)
	{
		insert(
			end(),
			reinterpret_cast<const uint8_t*>(value),
			reinterpret_cast<const uint8_t*>(value) + length);
	}
};

CTblEditorBase::CTblEditorBase()
{
}

CTblEditorBase::~CTblEditorBase()
{
}

void CTblEditorBase::Release()
{
	m_Rows.clear();
	m_DataTypes.clear();
}

bool CTblEditorBase::LoadFile(const CString& path, CString& errorMsg)
{
	if (path.IsEmpty())
	{
		TRACE("Path is empty");
		errorMsg.LoadString(IDS_ERROR_INVALID_FILE_PATH);
		return false;
	}

	std::error_code ec;

	FileReader encryptedFile;
	if (!encryptedFile.OpenExisting(path.GetString()))
	{
		TRACE(_T("Failed to open file: '%s'\n"), path);
		errorMsg.Format(IDS_ERROR_FAILED_TO_OPEN_FILE, path);
		return false;
	}

	CString TemporaryPath = path + _T(".tmp");
	TRACE(_T("Temporary Path: '%s'\n"), TemporaryPath);

	size_t encryptedFileSize = static_cast<size_t>(encryptedFile.Size());
	if (encryptedFileSize == 0)
	{
		TRACE("Error: Filesize is empty\n");

		encryptedFile.Close();
		std::filesystem::remove(TemporaryPath.GetString(), ec);

		errorMsg.LoadString(IDS_ERROR_INVALID_FILE_SIZE);
		return false;
	}

	// define pDatas with respect to dwSizeLow ( max size of document in bytes )
	uint8_t* pDatas = new uint8_t[encryptedFileSize];
	encryptedFile.Read(pDatas, encryptedFileSize);
	encryptedFile.Close(); // close original file

	// same key with the one used in table creator 
	uint16_t key_r = 0x0816;
	uint16_t key_c1 = 0x6081;
	uint16_t key_c2 = 0x1608;

	// decrypt
	for (uint32_t i = 0; i < encryptedFileSize; i++)
	{
		uint8_t byData = (pDatas[i] ^ (key_r >> 8));
		key_r = (pDatas[i] + key_r) * key_c1 + key_c2;
		pDatas[i] = byData;
	}

	// Open temporary file for writing the decrypted data to.
	{
		FileWriter tmpFileWriter;
		if (!tmpFileWriter.Create(TemporaryPath.GetString()))
		{
			delete[] pDatas;
			errorMsg.Format(IDS_ERROR_FAILED_TO_OPEN_TEMP_FILE_FOR_WRITING, path);
			return false;
		}

		tmpFileWriter.Write(pDatas, encryptedFileSize); // write encrypted data into temporary file
		tmpFileWriter.Close(); // 임시 파일 닫기

		delete[] pDatas;
		pDatas = nullptr;
	}

	bool ret = false;

	// Open temporary file again for reading, so we can read the decrypted data back.
	FileReader decryptedFile;
	if (decryptedFile.OpenExisting(TemporaryPath.GetString()))
	{
		ret = LoadRowData(decryptedFile);

		if (!ret)
			errorMsg.LoadString(IDS_ERROR_UNSUPPORTED_TABLE);

		decryptedFile.Close();
	}

	// delete temporary file
	std::filesystem::remove(TemporaryPath.GetString(), ec);

	return ret;
}

bool CTblEditorBase::LoadRowData(File& file)
{
	size_t bytesRead = 0;

	// Reading how the structure of the data (column) is organized.
	int iDataTypeCount = 0;

	// Read first 4 bytes = number of columns in the table (datatype count)
	if (!file.Read(&iDataTypeCount, sizeof(int), &bytesRead)
		|| bytesRead != sizeof(int))
	{
		TRACE("Failed to read datatype count.\n");
		return false;
	}

	TRACE("Columns: %d\n", iDataTypeCount);

	if (iDataTypeCount <= 0
		|| iDataTypeCount > MAX_COLUMN_COUNT)
	{
		TRACE("Invalid column count.\n");
		return false;
	}

	// Read the data types
	m_DataTypes.clear();
	m_DataTypes.resize(iDataTypeCount);

	for (int i = 0; i < iDataTypeCount; i++)
	{
		int iDataType = 0;
		if (!file.Read(&iDataType, sizeof(int), &bytesRead)
			|| bytesRead != sizeof(int))
		{
			TRACE("Failed to read data types.\n");
			return false;
		}

		m_DataTypes[i] = static_cast<DATA_TYPE>(iDataType);

		// Optional: Print data types for debug
		TRACE("DataType[%d] = %d\n", i, iDataType);
	}

	// Now read the row count (4 bytes after the dataTypes array)
	int iRowCount = 0;
	if (!file.Read(&iRowCount, sizeof(int), &bytesRead)
		|| bytesRead != sizeof(int))
	{
		TRACE("Failed to read row count.\n");
		return false;
	}

	TRACE("Row count: %d\n", iRowCount);

	if (iRowCount < 0
		|| iRowCount > MAX_ROW_COUNT)
	{
		TRACE("Invalid row count.\n");
		return false;
	}

	// Now that we've read the datatypes and the row count, we can read the row data.
	m_Rows.clear();

	CStringA szValue;
	for (int iRowNo = 0; iRowNo < iRowCount; iRowNo++)
	{
		std::vector<CStringA> row;
		row.reserve(iDataTypeCount);

		// Read each column's data for this row
		for (int iColNo = 0; iColNo < iDataTypeCount; iColNo++)
		{
			DATA_TYPE dataType = static_cast<DATA_TYPE>(m_DataTypes[iColNo]);

			szValue.Empty();
			switch (dataType)
			{
				case DT_CHAR:
				{
					char val = '\0';
					if (!file.Read(&val, sizeof(char), &bytesRead)
						|| bytesRead != sizeof(char))
					{
						TRACE("Failed to read DT_CHAR data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%c", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_CHAR = %c\n", iRowNo, iColNo, val);
					break;
				}

				case DT_BYTE:
				{
					uint8_t val = 0;
					if (!file.Read(&val, sizeof(uint8_t), &bytesRead)
						|| bytesRead != sizeof(uint8_t))
					{
						TRACE("Failed to read DT_BYTE data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%u", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_BYTE = %u\n", iRowNo, iColNo, val);
					break;
				}

				case DT_SHORT:
				{
					int16_t val = 0;
					if (!file.Read(&val, sizeof(int16_t), &bytesRead)
						|| bytesRead != sizeof(int16_t))
					{
						TRACE("Failed to read DT_SHORT data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%d", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_SHORT = %d\n", iRowNo, iColNo, val);
					break;
				}

				case DT_WORD:
				{
					uint16_t val = 0;
					if (!file.Read(&val, sizeof(uint16_t), &bytesRead)
						|| bytesRead != sizeof(uint16_t))
					{
						TRACE("Failed to read DT_WORD data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%u", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_WORD = %u\n", iRowNo, iColNo, val);
					break;
				}

				case DT_INT:
				{
					int32_t val = 0;
					if (!file.Read(&val, sizeof(int32_t), &bytesRead)
						|| bytesRead != sizeof(int32_t))
					{
						TRACE("Failed to read DT_INT data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%d", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_INT = %d\n", iRowNo, iColNo, val);
					break;
				}

				case DT_DWORD:
				{
					uint32_t val = 0;
					if (!file.Read(&val, sizeof(uint32_t), &bytesRead)
						|| bytesRead != sizeof(uint32_t))
					{
						TRACE("Failed to read DT_DWORD data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%u", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_DWORD = %u\n", iRowNo, iColNo, val);
					break;
				}

				case DT_STRING:
				{
					// Read 32-bit string length
					int32_t iStrLen = 0;
					if (!file.Read(&iStrLen, sizeof(int32_t), &bytesRead)
						|| bytesRead != sizeof(int32_t))
					{
						TRACE("Failed to read string length at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					TRACE("Row %d, Column %d: String length = %d\n", iRowNo, iColNo, iStrLen);

					// Invalid string length
					if (iStrLen < 0
						|| iStrLen > MAX_STRING_LENGTH)
					{
						TRACE("Row %d, Column %d: DT_STRING = (invalid string)\n", iRowNo, iColNo);
						return false;
					}

					// Empty string (but still valid)
					if (iStrLen == 0)
					{
						TRACE("Row %d, Column %d: DT_STRING = (empty string)\n", iRowNo, iColNo);
					}
					else
					{
						if (!file.Read(szValue.GetBuffer(iStrLen), iStrLen, &bytesRead)
							|| bytesRead != iStrLen)
						{
							szValue.ReleaseBuffer();
							TRACE("Failed to read string data at row %d, column %d\n", iRowNo, iColNo);
							return false;
						}

						szValue.ReleaseBuffer();

						TRACE(_T("Row %d, Column %d: DT_STRING = %s\n"), iRowNo, iColNo, szValue);
					}

					row.push_back(szValue);
					break;
				}

				case DT_FLOAT:
				{
					float val = 0.0f;
					if (!file.Read(&val, sizeof(float), &bytesRead)
						|| bytesRead != sizeof(float))
					{
						TRACE("Failed to read DT_FLOAT data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%f", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_FLOAT = %f\n", iRowNo, iColNo, val);
					break;
				}

				case DT_DOUBLE:
				{
					double val = 0.0;
					if (!file.Read(&val, sizeof(double), &bytesRead)
						|| bytesRead != sizeof(double))
					{
						TRACE("Failed to read DT_DOUBLE data at row %d, column %d\n", iRowNo, iColNo);
						return false;
					}

					szValue.Format("%f", val);
					row.push_back(szValue);

					TRACE("Row %d, Column %d: DT_DOUBLE = %f\n", iRowNo, iColNo, val);
					break;
				}

				default:
					TRACE("Unknown data type at row %d, column %d\n", iRowNo, iColNo);
					return false;
			}
		}

		m_Rows.insert(std::make_pair(iRowNo, row));
	}

	return true;
}

bool CTblEditorBase::SaveFile(const CString& path, const std::map<int, std::vector<CStringA>>& newData)
{
	WriteBuffer writeBuffer;

	// 1. iDataTypeCount (4 bytes)
	int32_t iDataTypeCount = static_cast<int32_t>(m_DataTypes.size());

	TRACE("iDataTypeCount = %d", iDataTypeCount);
	writeBuffer.append<int32_t>(&iDataTypeCount);

	// 2. stored datatypes (4 bytes each)
	for (int32_t dataType : m_DataTypes)
		writeBuffer.append<int32_t>(&dataType);

	// 3. row count
	int32_t iRowCount = static_cast<int32_t>(newData.size());
	writeBuffer.append<int32_t>(&iRowCount);

	// 4. row data
	for (const auto& rowItr : newData)
	{
		const auto& row = rowItr.second;

		for (int iColNo = 0; iColNo < iDataTypeCount; iColNo++)
		{
			DATA_TYPE dataType = m_DataTypes[iColNo];
			const CStringA& value = row[iColNo];

			switch (dataType)
			{
				case DT_CHAR:
				{
					char val = static_cast<char>(atoi(value));
					writeBuffer.append<char>(&val);
					break;
				}

				case DT_BYTE:
				{
					uint8_t val = static_cast<uint8_t>(atoi(value));
					writeBuffer.append<uint8_t>(&val);
					break;
				}

				case DT_SHORT:
				{
					int16_t val = static_cast<int16_t>(atoi(value));
					writeBuffer.append<int16_t>(&val);
					break;
				}

				case DT_WORD:
				{
					uint16_t val = static_cast<uint16_t>(atoi(value));
					writeBuffer.append<uint16_t>(&val);
					break;
				}

				case DT_INT:
				{
					int32_t val = atoi(value);
					writeBuffer.append<int32_t>(&val);
					break;
				}

				case DT_DWORD:
				{
					uint32_t val = static_cast<uint32_t>(strtoul(value, nullptr, 10));
					writeBuffer.append<uint32_t>(&val);
					break;
				}

				case DT_STRING:
				{
					int32_t len = value.GetLength();
					writeBuffer.append<int32_t>(&len);
					writeBuffer.append(value.GetString(), len);
					break;
				}

				case DT_FLOAT:
				{
					float val = strtof(value, nullptr);
					writeBuffer.append<float>(&val);
					break;
				}

				case DT_DOUBLE:
				{
					double val = strtod(value, nullptr);
					writeBuffer.append<double>(&val);
					break;
				}
			}
		}
	}

	if (path.IsEmpty())
		return false;

	// Create or open the file for writing
	FileWriter file;
	if (!file.Create(path.GetString()))
		return false;

	// Encryption key as defined earlier
	uint16_t key_r = 0x0816;
	uint16_t key_c1 = 0x6081;
	uint16_t key_c2 = 0x1608;

	// Encrypt the data before writing it to the file
	size_t dataSize = writeBuffer.size();
	uint8_t* encryptedData = new uint8_t[dataSize];

	for (size_t i = 0; i < dataSize; i++)
	{
		uint8_t cipher = (writeBuffer[i] ^ (key_r >> 8));
		key_r = (cipher + key_r) * key_c1 + key_c2;
		encryptedData[i] = cipher;
	}

	TRACE("First encrypted byte: %02X\n", encryptedData[0]);
	TRACE("Data size to be written: %zu\n", dataSize);

	// Write encrypted data to the file
	size_t bytesWritten = 0;
	bool bResult = file.Write(encryptedData, dataSize, &bytesWritten);
	file.Close();

	TRACE("Actual number of bytes written: %zu\n", bytesWritten);

	// Clean up
	delete[] encryptedData;
	return (bResult && bytesWritten == dataSize);
}

CString CTblEditorBase::GetColumnDefault(int iColNo) const
{
	DATA_TYPE dataType = m_DataTypes[iColNo];

	CString szDefault;
	switch (dataType)
	{
		case DT_CHAR:
		case DT_BYTE:
		case DT_SHORT:
		case DT_WORD:
		case DT_INT:
		case DT_DWORD:
			szDefault = _T("0");
			break;

		case DT_STRING:
			/* leave empty */
			break;

		case DT_FLOAT:
		case DT_DOUBLE:
			szDefault = _T("0.0");
			break;
	}

	return szDefault;
}

CString CTblEditorBase::GetColumnName(int iColNo) const
{
	DATA_TYPE dataType = m_DataTypes[iColNo];
	switch (dataType)
	{
		case DT_CHAR:
			return _T("SByte");

		case DT_BYTE:
			return _T("Byte");

		case DT_SHORT:
			return _T("Int16");

		case DT_WORD:
			return _T("UInt16");

		case DT_INT:
			return _T("Int32");

		case DT_DWORD:
			return _T("UInt32");

		case DT_STRING:
			return _T("String");

		case DT_FLOAT:
			return _T("Float");

		case DT_DOUBLE:
			return _T("Double");

		default:
		{
			CString szName;
			szName.Format(_T("%d"), dataType);
			return szName;
		}
	}
}

CString CTblEditorBase::GetFullColumnName(int iColNo) const
{
	CString szName;
	szName.Format(
		_T("%d (%s)"),
		iColNo + 1,
		GetColumnName(iColNo));
	return szName;
}

DATA_TYPE CTblEditorBase::GetColumnType(int iColNo) const
{
	return m_DataTypes[iColNo];
}
