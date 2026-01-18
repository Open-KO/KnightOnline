#ifndef SERVER_AISERVER_PACKET_H
#define SERVER_AISERVER_PACKET_H

#pragma once

namespace AIServer
{

inline constexpr uint8_t INFO_MODIFY = 1;
inline constexpr uint8_t INFO_DELETE = 2;

inline constexpr int TYPE_MONEY_SID  = 900000000; // 아이템 과 돈을 구분하기위해 sid를 크게 잡았다.

enum e_ServerInfoOpcode : uint8_t
{
	SERVER_INFO_START = 1,
	SERVER_INFO_END   = 2
};

} // namespace AIServer

#endif // SERVER_AISERVER_PACKET_H
