// Compress.cpp : implementation of the CMainFrame class
//
#include "stdafx.h"
//#include "server.h"
#include <shared/pkware/pklib.h>
#include "Compress.h"

#define DO_CRC_INSTREAM          1
#define DO_CRC_OUTSTREAM         2

UINT DataType = CMP_BINARY;          // GLOBAL FOR DATA TYPE FOR COMPRESSION
UINT DictSize = 4096;               // GLOBAL FOR DICTIONARY SIZE FOR COMPRESSION

CCompressManager::CCompressManager()
{
	adding_data_count = 0;
	addList = nullptr;
	CompressBufferPtr = nullptr;
	ExtractBufferPtr = nullptr;
	pScratchPad = new char[CMP_BUFFER_SIZE];
}

CCompressManager::~CCompressManager()
{
	delete pScratchPad;
	pScratchPad = nullptr;

	FlushAddData();
	FlushExtractedData();
}

//Compress Part...
int CCompressManager::AddData(char* add_data, long nLen)
{
	short l_len;

	if (addList == nullptr)
		addList = new CObList;

	char* dataPtr;
	if (nLen == 0)
		l_len = strlen(add_data);
	else
		l_len = (short) nLen;

	CCompressData* pData;

	pData = new CCompressData;
	dataPtr = new char[l_len + 1 + sizeof(short)];
	memcpy(dataPtr, &l_len, sizeof(short));
	memcpy(&dataPtr[sizeof(short)], add_data, l_len);
	dataPtr[l_len + sizeof(short)] = '\0';
	pData->SetDataPtr(dataPtr);

	addList->AddTail(pData);
	adding_data_count++;

	return true;
}

/*********************************************************************
 *
 * Function:   ReadBuffer()
 *
 * Purpose:    To handle calls from the Data Compression Library for
 *             read requests. If compressing, then the data read is
 *             in uncompressed form.  If compressing, then the data
 *             read is data that was previously compressed. This
 *             function is called until zero is returned.
 *
 * Parameters: buffer ->   Address of buffer to read the data into
 *             iSize ->    Number of bytes to read into buffer
 *             dwParam ->  User-defined parameter, in this case a
 *                         pointer to the DATABLOCK
 *
 * Returns:    Number of bytes actually read, or zero on EOF
 *
 *********************************************************************/
UINT ReadBuffer(PCHAR buffer, UINT* iSize, void* pParam)
{
	CCompressManager* pCM;
	UINT iRead;

	pCM = (CCompressManager*) pParam;

	if (pCM == nullptr)
		return 0;

	if (pCM->CompressCurPos < pCM->nCompressBufferCount)
	{
		UINT BytesLeft = pCM->nCompressBufferCount - pCM->CompressCurPos;

		if (BytesLeft < *iSize)
			*iSize = BytesLeft;

		memcpy(buffer, (pCM->CompressBufferPtr + pCM->CompressCurPos), *iSize);
		pCM->CompressCurPos += *iSize;

		iRead = *iSize;
	}
	else
	{
		iRead = 0;
	}

	if (pCM->Mode == COMPRESSING)
		pCM->dwCrc = crc32pk(buffer, &iRead, &pCM->dwCrc);

	return iRead;
}

/*********************************************************************
 *
 * Function:   WriteBuffer()
 *
 * Purpose:    To handle calls from the Data Compression Library for
 *             write requests.
 *
 * Parameters: buffer ->   Address of buffer to write data from
 *             iSize ->    Number of bytes to write
 *             dwParam ->  User-defined parameter, in this case a
 *                         pointer to the DATABLOCK
 *
 * Returns:    Zero, the return value is not used by the Data
 *             Compression Library
 *
 *********************************************************************/
void WriteBuffer(PCHAR buffer, UINT* iSize, void* pParam)
{
	CCompressManager* pCM;

	pCM = (CCompressManager*) pParam;
	if (pCM == nullptr)
		return;

	if (pCM->UncompressCurPos < pCM->nUnCompressBufferCount)
	{
		if ((pCM->nUnCompressBufferCount - pCM->UncompressCurPos) < (int) (*iSize))
		{
			//MessageBox( nullptr, "Out of buffer space - #1", "Compression Error", MB_OK );
			pCM->ErrorOccurred = 1;
			return;
		}

		memcpy((pCM->ExtractBufferPtr + pCM->UncompressCurPos), buffer, *iSize);
		pCM->UncompressCurPos += *iSize;
	}
	else
	{
		//MessageBox( nullptr, "Out of buffer space - #2", "Compression Error", MB_OK );
		pCM->ErrorOccurred = 2;
		return;
	}

	if (pCM->Mode == UNCOMPRESSING)
		pCM->dwCrc = crc32pk(buffer, iSize, &pCM->dwCrc);

	return;
}

int CCompressManager::PreCompressWork()
{
	short strLength;
	long bytes_count = 0;
	POSITION pos;
	CCompressData* pData;

	if (addList == nullptr)
	{
		nCompressBufferCount = 0;
		OrgDataLength = 0;
		return false;
	}

	for (pos = addList->GetHeadPosition(); pos != nullptr; addList->GetNext(pos))
	{
		pData = (CCompressData*) addList->GetAt(pos);
		memcpy((char*) (&strLength), pData->GetDataPtr(), sizeof(short));
		bytes_count += strLength;
	}

	if (CompressBufferPtr != nullptr)
		delete CompressBufferPtr;

	CompressBufferPtr = new char[bytes_count + 1];
	nCompressBufferCount = bytes_count;
	OrgDataLength = bytes_count;

	CompressCurPos = 0;

	for (pos = addList->GetHeadPosition(); pos != nullptr; addList->GetNext(pos))
	{
		pData = (CCompressData*) addList->GetAt(pos);
		memcpy(&strLength, pData->GetDataPtr(), sizeof(short));
		bytes_count = strLength;
		memcpy((CompressBufferPtr + CompressCurPos), (pData->GetDataPtr() + sizeof(short)), bytes_count);
		CompressCurPos += bytes_count;
	}

	bytes_count = CompressCurPos;
	CompressCurPos = 0;

	if (ExtractBufferPtr != nullptr)
		delete[] ExtractBufferPtr;

	if (bytes_count < 1024)
		bytes_count = bytes_count * 2;

	ExtractBufferPtr = new char[bytes_count + 1];
	nUnCompressBufferCount = bytes_count;
	UncompressCurPos = 0;

	dwCrc = ~((DWORD) 0); // Pre-condition CRC
	Mode = COMPRESSING;
	ErrorOccurred = 0;

	return true;
}

int CCompressManager::Compress()
{
	int iStatus;

	if (OrgDataLength == 0)
		return true;

	// COMPRESS THE FILE
	iStatus = implode(ReadBuffer, WriteBuffer, pScratchPad, this, &DataType, &DictSize);
	if (iStatus != 0)
		return false;

	return true;
}

void CCompressManager::FlushAddData()
{
	POSITION pos;
	CCompressData* pData;

	if (addList != nullptr)
	{
		if (addList->GetCount() > 0)
		{
			for (pos = addList->GetHeadPosition(); pos != nullptr; )
			{
				pData = (CCompressData*) addList->GetNext(pos);
				delete pData;
			}

			addList->RemoveAll();
		}

		delete addList;
		addList = nullptr;
	}
	adding_data_count = 0;

	delete[] CompressBufferPtr;
	CompressBufferPtr = nullptr;

	delete[] ExtractBufferPtr;
	ExtractBufferPtr = nullptr;

	nCompressBufferCount = 0;
	nUnCompressBufferCount = 0;
	CompressCurPos = 0;
	UncompressCurPos = 0;
	ErrorOccurred = 0;

	if (pScratchPad != nullptr)
	{
		for (int i = 0; i < CMP_BUFFER_SIZE; i++)
			pScratchPad[i] = 0;
	}
}

//Extract Part...
void CCompressManager::SetCompressionData(char* b_ptr, long nLen)
{
	size_t l_len;
	if (nLen == 0)
		l_len = strlen(b_ptr);
	else
		l_len = static_cast<size_t>(nLen);

	memcpy(CompressBufferPtr, b_ptr, l_len);
}

int CCompressManager::PreUncompressWork(long nLen, long orgDataLen)
{
	delete[] CompressBufferPtr;
	CompressBufferPtr = new char[nLen + 1];
	nCompressBufferCount = nLen + 1;
	CompressCurPos = 0;

	delete[] ExtractBufferPtr;

	if (orgDataLen == 0)
	{
		if (DataType == CMP_ASCII)
		{
			ExtractBufferPtr = new char[nLen * 100];
			nUnCompressBufferCount = nLen * 100;
		}
		else
		{
			ExtractBufferPtr = new char[nLen * 10];
			nUnCompressBufferCount = nLen * 10;
		}
	}
	else
	{
		ExtractBufferPtr = new char[orgDataLen + 1];
		nUnCompressBufferCount = orgDataLen + 1;
	}

	OrgDataLength = orgDataLen;
	dwCrc = ~((DWORD) 0);
	Mode = UNCOMPRESSING;
	ErrorOccurred = 0;
	UncompressCurPos = 0;

	return true;
}

int CCompressManager::Extract()
{
	int iStatus;

	if (OrgDataLength == 0)
		return true;

	iStatus = explode(ReadBuffer, WriteBuffer, pScratchPad, this);
	if (iStatus != 0)
		return false;

	return true;
}

void CCompressManager::FlushExtractedData()
{
	delete[] CompressBufferPtr;
	CompressBufferPtr = nullptr;

	nCompressBufferCount = 0;
	CompressCurPos = 0;

	delete[] ExtractBufferPtr;
	ExtractBufferPtr = nullptr;

	nUnCompressBufferCount = 0;
	ErrorOccurred = 0;
	UncompressCurPos = 0;

	if (pScratchPad != nullptr)
	{
		for (int i = 0; i < CMP_BUFFER_SIZE; i++)
			pScratchPad[i] = 0;
	}
}
