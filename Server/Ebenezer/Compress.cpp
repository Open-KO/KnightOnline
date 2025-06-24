// Compress.cpp : implementation of the CMainFrame class
//
#include "stdafx.h"
#include <shared/pkware/pklib.h>
#include "Compress.h"

#define DO_CRC_INSTREAM          1
#define DO_CRC_OUTSTREAM         2

UINT DataType = CMP_BINARY;          // GLOBAL FOR DATA TYPE FOR COMPRESSION
UINT DictSize = 4096;               // GLOBAL FOR DICTIONARY SIZE FOR COMPRESSION

CCompressMng::CCompressMng()
{
	m_pInputBuffer = nullptr;
	m_pOutputBuffer = nullptr;
	m_nBufferStatus = 0;
	m_nErrorOccurred = 0;
	m_pScratchPad = new char[CMP_BUFFER_SIZE];
}

CCompressMng::~CCompressMng()
{
	delete[] m_pScratchPad;
	m_pScratchPad = nullptr;

	delete[] m_pInputBuffer;
	m_pInputBuffer = nullptr;


	delete[] m_pOutputBuffer;
	m_pOutputBuffer = nullptr;
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
 * Parameters: buffer ->   Address of buffer to read the data into1
 *             iSize ->    Number of bytes to read into buffer
 *             dwParam ->  User-defined parameter, in this case a
 *                         pointer to the DATABLOCK
 *
 * Returns:    Number of bytes actually read, or zero on EOF
 *
 *********************************************************************/
UINT ReadBuffer(PCHAR buffer, UINT* iSize, void* pParam)
{
	CCompressMng* pCM;
	UINT iRead;

	pCM = (CCompressMng*) pParam;

	if (pCM == nullptr)
		return 0;

	if (pCM->m_nInputBufferCurPos < pCM->m_nInputBufferCount)
	{
		UINT BytesLeft = pCM->m_nInputBufferCount - pCM->m_nInputBufferCurPos;

		if (BytesLeft < *iSize)
			*iSize = BytesLeft;

		memcpy(buffer, (pCM->m_pInputBuffer + pCM->m_nInputBufferCurPos), *iSize);
		pCM->m_nInputBufferCurPos += *iSize;

		iRead = *iSize;
	}
	else
	{
		iRead = 0;
	}

	if (pCM->m_nMode == COMPRESSING)
		pCM->m_dwCrc = crc32pk(buffer, &iRead, &pCM->m_dwCrc);

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
	CCompressMng* pCM;

	pCM = (CCompressMng*) pParam;
	if (pCM == nullptr)
		return;

	if (pCM->m_nOutputBufferCurPos < pCM->m_nOutputBufferCount)
	{
		if ((pCM->m_nOutputBufferCount - pCM->m_nOutputBufferCurPos) < (int) (*iSize))
		{
			//MessageBox( nullptr, "Out of buffer space - #1", "Compression Error", MB_OK );
			pCM->m_nErrorOccurred = 1;
			return;
		}

		memcpy((pCM->m_pOutputBuffer + pCM->m_nOutputBufferCurPos), buffer, *iSize);
		pCM->m_nOutputBufferCurPos += *iSize;
	}
	else
	{
		//MessageBox( nullptr, "Out of buffer space - #2", "Compression Error", MB_OK );
		pCM->m_nErrorOccurred = 2;
		return;
	}

	if (pCM->m_nMode == UNCOMPRESSING)
		pCM->m_dwCrc = crc32pk(buffer, iSize, &pCM->m_dwCrc);
}

int CCompressMng::PreCompressWork(const char* OriginData, int OriginSize)
{
	if (OriginSize == 0)
	{
		m_nInputBufferCount = 0;
		m_nOrgDataLength = 0;
		return false;
	}

	delete[] m_pInputBuffer;
	m_pInputBuffer = new char[OriginSize + 1];
	m_nOrgDataLength = m_nInputBufferCount = OriginSize;

	memcpy(m_pInputBuffer, OriginData, OriginSize);
	m_nInputBufferCurPos = 0;

	delete[] m_pOutputBuffer;
	m_pOutputBuffer = nullptr;

	if (OriginSize < 1024)
		OriginSize = OriginSize * 2;

	m_pOutputBuffer = new char[OriginSize + 1];
	m_nOutputBufferCount = OriginSize;
	m_nOutputBufferCurPos = 0;

	m_dwCrc = ~((DWORD) 0); // Pre-condition CRC
	m_nMode = COMPRESSING;
	m_nErrorOccurred = 0;

	return true;
}

int CCompressMng::Compress()
{
	int iStatus;

	if (m_nOrgDataLength == 0)
		return true;

	// COMPRESS THE FILE
	iStatus = implode(ReadBuffer, WriteBuffer, m_pScratchPad, this, &DataType, &DictSize);

	if (iStatus != 0)
		return false;

	return true;
}

//Extract Part...

int CCompressMng::PreUncompressWork(const char* pCompData, int nCompLen, int orgDataLen)
{
	delete[] m_pInputBuffer;
	m_pInputBuffer = new char[nCompLen + 1];
	m_nInputBufferCount = nCompLen + 1;
	m_nInputBufferCurPos = 0;

	memcpy(m_pInputBuffer, pCompData, nCompLen);

	delete[] m_pOutputBuffer;
	m_pOutputBuffer = nullptr;

	if (orgDataLen == 0)
	{
		if (DataType == CMP_ASCII)
		{
			m_pOutputBuffer = new char[nCompLen * 100];
			m_nOutputBufferCount = nCompLen * 100;
		}
		else
		{
			m_pOutputBuffer = new char[nCompLen * 10];
			m_nOutputBufferCount = nCompLen * 10;
		}
	}
	else
	{
		m_pOutputBuffer = new char[orgDataLen + 1];
		m_nOutputBufferCount = orgDataLen + 1;
	}

	m_nOrgDataLength = orgDataLen;
	m_dwCrc = ~((DWORD) 0);
	m_nMode = UNCOMPRESSING;
	m_nErrorOccurred = 0;
	m_nOutputBufferCurPos = 0;

	return true;
}

int CCompressMng::Extract()
{
	int iStatus;

	if (m_nOrgDataLength == 0)
		return true;

	iStatus = explode(ReadBuffer, WriteBuffer, m_pScratchPad, this);
	if (iStatus != 0)
		return false;

	return true;
}

void CCompressMng::Initialize()
{
	delete[] m_pInputBuffer;
	m_pInputBuffer = nullptr;

	delete[] m_pOutputBuffer;
	m_pOutputBuffer = nullptr;

	m_nBufferStatus = E;
	m_nInputBufferCount = 0;
	m_nOutputBufferCount = 0;
	m_nInputBufferCurPos = 0;
	m_nOutputBufferCurPos = 0;
	m_nErrorOccurred = 0;

	memset(m_pScratchPad, 0, CMP_BUFFER_SIZE);
/*	if ( pScratchPad ) {
		for (int i = 0; i <CMP_BUFFER_SIZE; i++)
			pScratchPad[i] = 0;
	}
*/
}
