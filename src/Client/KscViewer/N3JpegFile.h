// N3JpegFile.h: interface for the CN3JpegFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3JPEGFILE_H__680B8929_B03D_45EF_8986_51561B4BDC79__INCLUDED_)
#define AFX_N3JPEGFILE_H__680B8929_B03D_45EF_8986_51561B4BDC79__INCLUDED_

#pragma once

#include <JpegFile/JpegFile.h>

class CN3JpegFile : public CJpegFile
{
private:
	BYTE* m_pImageBuf;
	UINT m_uiWidth;
	UINT m_uiHeight;

public:
	void DrawImage(HDC hDC);
	BOOL LoadJpegFile(const std::string& csJpeg);
	UINT GetWidth()
	{
		return m_uiWidth;
	}
	UINT GetHeight()
	{
		return m_uiHeight;
	}
	BYTE* GetImageBuf()
	{
		return m_pImageBuf;
	}
	void Release();

	CN3JpegFile();
	virtual ~CN3JpegFile();
};

#endif // !defined(AFX_N3JPEGFILE_H__680B8929_B03D_45EF_8986_51561B4BDC79__INCLUDED_)
