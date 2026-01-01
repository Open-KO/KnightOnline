#pragma once

#include "TblEditorBase.h"

class CTblEditorDoc : public CDocument
{
protected: // create from serialization only
	CTblEditorDoc();
	DECLARE_DYNCREATE(CTblEditorDoc)

public:
	static constexpr LPARAM HINT_DOCUMENT_LOADED = 0x100000;

	inline bool IsLoaded() const {
		return !GetPathName().IsEmpty();
	}

	inline CTblEditorBase& GetTbl() {
		return m_Tbl;
	}

	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	BOOL OnSaveDocument(LPCTSTR lpszPathName) override;
	BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE) override;

protected:
	CTblEditorBase	m_Tbl;
};
