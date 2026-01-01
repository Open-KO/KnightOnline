#ifndef _DEFINE_H
#define _DEFINE_H

#define SM_EVENTS_RECEIVE			0
#define SM_EVENTS_SEND				1

#define PS_1						0XAA
#define PS_2						0X55
#define PE_1						0X55
#define PE_2						0XAA

#define SM_PACKETPENDING_SEND		0
#define	SM_PACKETPENDING_RECEIVE	1

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_DELETES(p) { if(p) { delete[] (p);	  (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
#define INIT_PTR(p)		{ (p) = nullptr;}

#define WM_PACKET_ARRIVED			(WM_USER+1)
#define WM_MESSAGE_RESTOREALL		(WM_USER+2)
#define WM_PACKET_PROCESS_PENDING	(WM_USER+100)

#define CONNECT_PORT		15100

#endif