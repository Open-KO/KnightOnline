﻿#ifndef _PACKET_H_
#define _PACKET_H_

const BYTE INFO_MODIFY			=	1;
const BYTE INFO_DELETE			= 	2;

// 상태이상 정보
#define _ABNORMAL_DB			5

const BYTE ABNORMAL_NONE		=	0;
const BYTE ABNORMAL_POISON		=	1;
const BYTE ABNORMAL_CONFUSION	=	2;
const BYTE ABNORMAL_PARALYSIS	=	3;
const BYTE ABNORMAL_BLIND		=	4;
const BYTE ABNORMAL_LIGHT		=	5;
const BYTE ABNORMAL_FIRE		=	6;
const BYTE ABNORMAL_COLD		=	7;
const BYTE ABNORMAL_HASTE		=	8;
const BYTE ABNORMAL_SHIELD		=	9;
const BYTE ABNORMAL_INFRAVISION =	10;

const int TYPE_MONEY_SID		=	900000000;	// 아이템 과 돈을 구분하기위해 sid를 크게 잡았다.

#define SERVER_INFO_START			0X01
#define SERVER_INFO_END				0X02

#endif
