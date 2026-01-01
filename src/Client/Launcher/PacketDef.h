#ifndef __PACKET_DEF_H_
#define __PACKET_DEF_H_

///////////////////////////////////////////////////////////////
// Packet Define 
///////////////////////////////////////////////////////////////
#define VERSION_REQ				0x01	// Send 
#define DOWNLOAD_INFO_REQ		0x02	// Send - s1(현재버전) | Recv FTP
#define LOGIN_REQ				0x03	// Send - s1(ID길이) str1(ID문자열:20바이트이하) s1(PW길이) str1(PW문자열:12바이트이하) | Recv - b1(0:실패 1:성공 2:ID없음 3:PW틀림 4:서버점검중)
#define MGAME_LOGIN_REQ			0x04	// !!! LOGIN_REQ !!! 와 동일
#define SERVER_LIST				0x05	// Recv - b1(ServerCount) loop(	s1(IP길이), str1(IP문자열), s1(서버이름길이), str1(서버이름문자열), s1(서버접속인원) )
///////////////////////////////////////////////////////////////

#endif // end of #ifndef __PACKET_DEF_H_
