#ifndef __COMPRESS_H
#define __COMPRESS_H

typedef enum
{
	COMPRESSING = 1,
	UNCOMPRESSING
} FILEMODE;

class CCompressData : public CObject
{
private:
	char* dataPtr;

public:
	CCompressData()
	{
		dataPtr = nullptr;
	}

	virtual ~CCompressData()
	{
		if (dataPtr != nullptr)
		{
			delete[] dataPtr;
			dataPtr = nullptr;
		}
	}

	char* GetDataPtr() const {
		return dataPtr;
	}

	void SetDataPtr(char* dp) {
		dataPtr = dp;
	}
};

class CCompressManager
{
public:
	int adding_data_count;
	CObList* addList;
	long pnCompressedSize;
	PCHAR  pScratchPad;
	DWORD dwCrc;

	long OrgDataLength;

	char* CompressBufferPtr;
	char* ExtractBufferPtr;

	long nCompressBufferCount;
	long nUnCompressBufferCount;

	long CompressCurPos;
	long UncompressCurPos;

	int Mode;
	int ErrorOccurred;

public:
	CCompressManager();
	virtual ~CCompressManager();

	CObList* GetDataList() const {
		return addList;
	}

	int PreCompressWork();
	int PreUncompressWork(long nLen, long orgDataLen = 0);

	//Compress Part...
	int AddData(char* data, long nLen = 0L);
	int Compress();

	char* GetCompressionBufferPtr() const  {
		return CompressBufferPtr;
	}

	long GetCompressedDataCount() const {
		return UncompressCurPos;
	}

	long GetUnCompressDataLength() const {
		return OrgDataLength;
	}

	long GetCrcValue() const {
		return dwCrc;
	}

	void FlushAddData();

	//Extract Part...
	void SetCompressionData(char* b_ptr, long nLen = 0);
	int Extract();

	char* GetExtractedBufferPtr() const {
		return ExtractBufferPtr;
	}

	long GetExtractedBufferCount() const {
		return nUnCompressBufferCount;
	}

	long GetExtractedDataCount() const {
		return UncompressCurPos;
	}

	void FlushExtractedData();
};


#endif
