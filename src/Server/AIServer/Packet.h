#ifndef SERVER_AISERVER_PACKET_H
#define SERVER_AISERVER_PACKET_H

#pragma once

namespace AIServer
{

constexpr uint8_t INFO_MODIFY          = 1;
constexpr uint8_t INFO_DELETE          = 2;

constexpr uint8_t ABNORMAL_NONE        = 0;
constexpr uint8_t ABNORMAL_POISON      = 1;
constexpr uint8_t ABNORMAL_CONFUSION   = 2;
constexpr uint8_t ABNORMAL_PARALYSIS   = 3;
constexpr uint8_t ABNORMAL_BLIND       = 4;
constexpr uint8_t ABNORMAL_LIGHT       = 5;
constexpr uint8_t ABNORMAL_FIRE        = 6;
constexpr uint8_t ABNORMAL_COLD        = 7;
constexpr uint8_t ABNORMAL_HASTE       = 8;
constexpr uint8_t ABNORMAL_SHIELD      = 9;
constexpr uint8_t ABNORMAL_INFRAVISION = 10;

constexpr int TYPE_MONEY_SID = 900000000; // 아이템 과 돈을 구분하기위해 sid를 크게 잡았다.

enum e_ServerInfoOpcode : uint8_t
{
	SERVER_INFO_START = 1,
	SERVER_INFO_END   = 2
};

} // namespace AIServer

#endif // SERVER_AISERVER_PACKET_H
