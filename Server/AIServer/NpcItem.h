// NpcItem.h: interface for the CNpcItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCITEM_H__56D8779F_271E_4B39_92CF_3DA33366FBA8__INCLUDED_)
#define AFX_NPCITEM_H__56D8779F_271E_4B39_92CF_3DA33366FBA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNpcItem
{
public:

	int** m_ppItem;
	int m_nRow;
	int m_nField;

	CNpcItem();
	~CNpcItem();
};

#endif // !defined(AFX_NPCITEM_H__56D8779F_271E_4B39_92CF_3DA33366FBA8__INCLUDED_)
