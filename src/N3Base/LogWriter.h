// LogWriter.h: interface for the CLogWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGWRITER_H__3299921C_04C3_40D6_848B_52F0CE5C9352__INCLUDED_)
#define AFX_LOGWRITER_H__3299921C_04C3_40D6_848B_52F0CE5C9352__INCLUDED_

#pragma once

#include <string>
#include <string_view>

#include <spdlog/fmt/bundled/format.h>

class CLogWriter
{
protected:
	//	static HANDLE s_hFile;
	static std::string s_szFileName;

public:
	static void Open(const std::string& szFN);
	static void Close();
	static void Write(const std::string_view message);

	template <typename... Args>
	static inline void Write(fmt::format_string<Args...> fmt, Args&&... args)
	{
		std::string message = fmt::format(fmt, std::forward<Args>(args)...);
		Write(message);
	}

	CLogWriter();
	virtual ~CLogWriter();
};

#endif // !defined(AFX_LOGWRITER_H__3299921C_04C3_40D6_848B_52F0CE5C9352__INCLUDED_)
