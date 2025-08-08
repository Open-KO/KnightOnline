#pragma once

#include <string>
#include <map>
#include <vector>

#include <N3Base/N3TableBase.h>

using DATA_TYPE = TBL_DATA_TYPE;

class CTblEditorBase
{
public:
	inline std::map<int, std::vector<CStringA>>& GetRows() {
		return m_Rows;
	}

	inline const std::map<int, std::vector<CStringA>>& GetRows() const {
		return m_Rows;
	}

	inline const std::vector<DATA_TYPE>& GetColumnTypes() const {
		return m_DataTypes;
	}
	
	CTblEditorBase();
	virtual ~CTblEditorBase();
	void Release();
	bool LoadFile(const CString& path, CString& errorMsg);
	bool SaveFile(const CString& path, const std::map<int, std::vector<CStringA>>& newData);
	CString GetColumnDefault(int iColNo) const;
	CString GetColumnName(int iColNo) const;
	CString GetFullColumnName(int iColNo) const;
	DATA_TYPE GetColumnType(int iColNo) const;

protected:
	bool LoadRowData(HANDLE hFile);

protected:
	std::map<int, std::vector<CStringA>> m_Rows;
	std::vector<DATA_TYPE> m_DataTypes;
};
