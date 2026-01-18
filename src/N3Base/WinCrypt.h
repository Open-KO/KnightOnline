#ifndef N3BASE_WINCRYPT_H
#define N3BASE_WINCRYPT_H

#pragma once

#include <wincrypt.h>

class File;
class CWinCrypt
{
public:
	constexpr const static TCHAR Provider[] = MS_ENHANCED_PROV;
	constexpr const static char Cipher[]    = "owsd9012%$1as!wpow1033b%!@%12";

	inline bool IsLoaded() const
	{
		return m_bIsLoaded;
	}

	CWinCrypt();
	~CWinCrypt();
	bool Load();
	void Release();
	bool ReadFile(File& file, void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr);

protected:
	bool m_bIsLoaded;
	HCRYPTPROV m_hCryptProvider;
	HCRYPTHASH m_hCryptHash;
	HCRYPTKEY m_hCryptKey;
};

#endif // #define N3BASE_WINCRYPT_H
