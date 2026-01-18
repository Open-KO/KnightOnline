#include "StdAfxBase.h"
#include "WinCrypt.h"
#include "N3Base.h"

#include <FileIO/File.h>

CWinCrypt::CWinCrypt()
{
	m_bIsLoaded      = false;
	m_hCryptProvider = 0;
	m_hCryptHash     = 0;
	m_hCryptKey      = 0;
}

bool CWinCrypt::Load()
{
	Release();

	// Try to acquire an existing key context
	// NOTE: Officially this passes 0, but this will require access to the persistent keystore used for
	// private keys.
	// If we use CRYPT_VERIFYCONTEXT instead, we don't need access to private keys, so we can avoid
	// requiring the extra privs.
	if (!CryptAcquireContext(
			&m_hCryptProvider, nullptr, Provider, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)
		// Create a new key context instead
		&& !CryptAcquireContext(
			&m_hCryptProvider, nullptr, Provider, PROV_RSA_FULL, CRYPT_NEWKEYSET))
		return false;

	if (!CryptCreateHash(m_hCryptProvider, CALG_SHA, 0, 0, &m_hCryptHash))
	{
		CryptReleaseContext(m_hCryptProvider, 0);
		return false;
	}

	if (!CryptHashData(m_hCryptHash, reinterpret_cast<const BYTE*>(Cipher), sizeof(Cipher) - 1, 0)
		|| !CryptDeriveKey(m_hCryptProvider, CALG_RC4, m_hCryptHash, 0x800000u, &m_hCryptKey))
	{
		CryptDestroyHash(m_hCryptHash);
		CryptReleaseContext(m_hCryptProvider, 0);
		return false;
	}

	m_bIsLoaded = TRUE;
	return true;
}

void CWinCrypt::Release()
{
	if (m_bIsLoaded)
	{
		CryptDestroyKey(m_hCryptKey);
		CryptDestroyHash(m_hCryptHash);
		CryptReleaseContext(m_hCryptProvider, 0);
	}

	m_bIsLoaded      = false;
	m_hCryptProvider = 0;
	m_hCryptHash     = 0;
	m_hCryptKey      = 0;
}

bool CWinCrypt::ReadFile(
	File& file, void* buffer, size_t bytesToRead, size_t* bytesRead /*= nullptr*/)
{
	if (!file.Read(buffer, bytesToRead, bytesRead))
		return false;

	if (IsLoaded())
	{
		DWORD dwDataLen = static_cast<DWORD>(bytesToRead);
		if (!CryptDecrypt(m_hCryptKey, 0, TRUE, 0, static_cast<BYTE*>(buffer), &dwDataLen))
			return false;
	}

	return true;
}

CWinCrypt::~CWinCrypt()
{
	Release();
}
