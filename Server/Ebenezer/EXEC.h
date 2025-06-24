// EXEC.h: interface for the EXEC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXEC_H__7514FC25_511B_11D3_BE41_00105A6B97E2__INCLUDED_)
#define AFX_EXEC_H__7514FC25_511B_11D3_BE41_00105A6B97E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class EXEC
{
public:
	void Init();
	BYTE m_Exec;
	int m_ExecInt[MAX_EXEC_INT];

	void Parse(char* pBuf);
	EXEC();
	virtual ~EXEC();

};

#endif // !defined(AFX_EXEC_H__7514FC25_511B_11D3_BE41_00105A6B97E2__INCLUDED_)
