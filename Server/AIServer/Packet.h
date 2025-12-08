#ifndef SERVER_AISERVER_PACKET_H
#define SERVER_AISERVER_PACKET_H

#pragma once

const uint8_t INFO_MODIFY			=	1;
const uint8_t INFO_DELETE			= 	2;

// 상태이상 정보
#define _ABNORMAL_DB			5

const uint8_t ABNORMAL_NONE			= 0;
const uint8_t ABNORMAL_POISON		= 1;
const uint8_t ABNORMAL_CONFUSION	= 2;
const uint8_t ABNORMAL_PARALYSIS	= 3;
const uint8_t ABNORMAL_BLIND		= 4;
const uint8_t ABNORMAL_LIGHT		= 5;
const uint8_t ABNORMAL_FIRE			= 6;
const uint8_t ABNORMAL_COLD			= 7;
const uint8_t ABNORMAL_HASTE		= 8;
const uint8_t ABNORMAL_SHIELD		= 9;
const uint8_t ABNORMAL_INFRAVISION	= 10;

const int TYPE_MONEY_SID			= 900000000;	// 아이템 과 돈을 구분하기위해 sid를 크게 잡았다.

#define SERVER_INFO_START			0X01
#define SERVER_INFO_END				0X02

#endif // SERVER_AISERVER_PACKET_H
