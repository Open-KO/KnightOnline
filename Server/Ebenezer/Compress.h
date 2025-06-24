#ifndef __COMPRESS_H
#define __COMPRESS_H

typedef enum
{
	COMPRESSING = 1,
	UNCOMPRESSING
} FILEMODE;

#define E	0x00
#define R	0x01
#define W	0x02
#define WR	0x03

class CCompressMng
{
public:
	CCompressMng();
	virtual ~CCompressMng();

	PCHAR	m_pScratchPad;
	DWORD	m_dwCrc;
	DWORD	m_dwThreadID;

	int		m_nOrgDataLength;

	char*	m_pInputBuffer;
	char*	m_pOutputBuffer;

	int		m_nInputBufferCount;
	int		m_nOutputBufferCount;

	int		m_nInputBufferCurPos;
	int		m_nOutputBufferCurPos;

	int		m_nBufferStatus;
	int		m_nMode;
	int		m_nErrorOccurred;

	void Initialize();

	// Compress Part...
	int PreCompressWork(const char* OriginData, int OriginSize);
	int Compress();

	// Extract Part...
	int PreUncompressWork(const char* pCompData, int nCompLen, int orgDataLen = 0);
	int Extract();
};

#endif
