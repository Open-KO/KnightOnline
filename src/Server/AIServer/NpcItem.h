#ifndef SERVER_AISERVER_NPCITEM_H
#define SERVER_AISERVER_NPCITEM_H

#pragma once

namespace AIServer
{

class CNpcItem
{
public:
	int** m_ppItem;
	int m_nRow;
	int m_nField;

	CNpcItem();
	~CNpcItem();
};

} // namespace AIServer

#endif // SERVER_AISERVER_NPCITEM_H
