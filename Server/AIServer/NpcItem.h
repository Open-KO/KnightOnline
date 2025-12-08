#ifndef SERVER_AISERVER_NPCITEM_H
#define SERVER_AISERVER_NPCITEM_H

#pragma once

class CNpcItem
{
public:
	int** m_ppItem;
	int m_nRow;
	int m_nField;

	CNpcItem();
	~CNpcItem();
};

#endif // SERVER_AISERVER_NPCITEM_H
