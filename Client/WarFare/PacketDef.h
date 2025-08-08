#ifndef __PACKET_DEF_H_
#define __PACKET_DEF_H_

const int SOCKET_PORT_GAME = 15001;
const int SOCKET_PORT_LOGIN = 15100;

#include "shared/types.h"
#include "shared/packets.h"

#define N3_ACCOUNT_LOGIN			0xf3
#define N3_ACCOUNT_LOGIN_MGAME		0xf4	// !!! LOGIN_REQ !!! 와 동일
#define N3_GAMESERVER_GROUP_LIST	0xf5	// Recv - b1(ServerCount) loop(	s1(IP길이), str1(IP문자열), s1(서버이름길이), str1(서버이름문자열), s1(서버접속인원) )
#define N3_NEWS						0xf6	//notice messages on login screen

	enum e_ChatMode {	N3_CHAT_NORMAL = 1,
						N3_CHAT_PRIVATE,
						N3_CHAT_PARTY,
						N3_CHAT_FORCE,
						N3_CHAT_SHOUT,
						N3_CHAT_CLAN,
						N3_CHAT_PUBLIC,
						N3_CHAT_WAR,
						N3_CHAT_TITLE,						//맨윗줄에 갈 공지
						N3_CHAT_TITLE_DELETE,				//맨윗줄에 있는 공지 삭제
						N3_CHAT_CONTINUE,					//지속 공지
						N3_CHAT_CONTINUE_DELETE,			//지속 공지 삭제
						N3_CHAT_UNKNOWN = 0xffffffff };

	enum e_SubPacket_Trade		{	N3_SP_TRADE_BUY  = 0x01,
									N3_SP_TRADE_SELL = 0x02,
									N3_SP_TRADE_MOVE = 0x03	};

	enum e_SubPacket_State {	N3_SP_STATE_CHANGE_SITDOWN = 0x01,
								N3_SP_STATE_CHANGE_RECRUIT_PARTY = 0x02,
								N3_SP_STATE_CHANGE_SIZE = 0x03,
								N3_SP_STATE_CHANGE_ACTION = 0x04,			// 1 - 인사, 11 - 도발
								N3_SP_STATE_CHANGE_VISIBLE = 0x05 };		// 투명 0 ~ 255

	// Sub Packet
	enum e_SubPacket_Party {	N3_SP_PARTY_OR_FORCE_CREATE			= 0x01,	// Send - s1(ID)	| Recv b1(YesNoErr)
							N3_SP_PARTY_OR_FORCE_PERMIT			= 0x02,	// Send - b1(YesNo) | Recv - s1(ID)
							N3_SP_PARTY_OR_FORCE_INSERT			= 0x03,	// Send - s1(ID) | Recv - s3(ID, HPMax, HP) b2(Level, Class) - 문자열은 ID 로 알아낸다.. ID 가 -1 이면.. 파티에 들어오는것을 상대방이 거절한거다..
							N3_SP_PARTY_OR_FORCE_REMOVE			= 0x04,	// Send - s1(ID) | Recv - s1(ID) - 자기 자신이면 파티를 깨야 한다..
							N3_SP_PARTY_OR_FORCE_DESTROY			= 0x05,	// Send
							N3_SP_PARTY_OR_FORCE_HP_CHANGE		= 0x06,	// Recv - s3(ID, HPMax, HP)
							N3_SP_PARTY_OR_FORCE_LEVEL_CHANGE	= 0x07,	// Recv - s1(ID), b1(Level)
							N3_SP_PARTY_OR_FORCE_CLASS_CHANGE	= 0x08,	// Recv - s1(ID), b1(Class)드물지만 전직할때...
							N3_SP_PARTY_OR_FORCE_STATUS_CHANGE	= 0x09 };	// Recv - s1(ID), b1(Status)독, 저주, 지속성마법, 축복

	// Sub Packet
	enum e_SubPacket_PerTrade {	N3_SP_PER_TRADE_REQ =			0x01, 
								N3_SP_PER_TRADE_AGREE =			0x02,
								N3_SP_PER_TRADE_ADD =			0x03,
								N3_SP_PER_TRADE_OTHER_ADD =		0x04,
								N3_SP_PER_TRADE_DECIDE	=		0x05,
								N3_SP_PER_TRADE_OTHER_DECIDE =	0x06,
								N3_SP_PER_TRADE_DONE =			0x07,
								N3_SP_PER_TRADE_CANCEL =		0x08 };	

	// Sub Packet
	enum e_SubPacket_Magic {	N3_SP_MAGIC_CASTING =	0x01,
								N3_SP_MAGIC_FLYING =	0x02,
								N3_SP_MAGIC_EFFECTING =	0x03,
								N3_SP_MAGIC_FAIL = 		0x04,
								N3_SP_MAGIC_TYPE4BUFFTYPE = 0x05,
								N3_SP_MAGIC_CANCEL = 0x06								
	};

	// Sub Packet..
	enum e_SubPacket_ClassChange_Main {
		N3_SP_CLASS_CHANGE_PURE = 0x01,		// Class Change Real..
		N3_SP_CLASS_CHANGE_REQ  = 0x02,		// Class Change Req..
		N3_SP_CLASS_ALL_POINT   = 0x03,		// Class Change All Point..
		N3_SP_CLASS_SKILL_POINT = 0x04,
		N3_SP_CLASS_POINT_CHANGE_PRICE_QUERY = 0x05,
		N3_SP_CLASS_PROMOTION = 0x06	};

	enum e_SubPacket_ClassChange {	N3_SP_CLASS_CHANGE_SUCCESS	= 0x01,		// Success..
									N3_SP_CLASS_CHANGE_NOT_YET	= 0x02,		// Not yet..
									N3_SP_CLASS_CHANGE_ALREADY = 0x03,		// Already..
									N3_SP_CLASS_CHANGE_ITEM_IN_SLOT = 0x04,		// Item in Slot..
									N3_SP_CLASS_CHANGE_FAILURE	= 0x00 };	// Failure..

	enum e_SubPacket_Knights {	N3_SP_KNIGHTS_CREATE =				0x01, // 생성 Send - s1(Name Length) str1 | Recv - b1(1:성공 0:실패)
								N3_SP_KNIGHTS_JOIN =				0x02, // 가입 Send - s1(Knights ID) | Recv - b1(1:성공 0:실패)
								N3_SP_KNIGHTS_WITHDRAW =			0x03, // 탈퇴 Send - | Recv - b1(1:성공 0:실패)
								N3_SP_KNIGHTS_MEMBER_REMOVE =		0x04, // 멤버 삭제 - 
								N3_SP_KNIGHTS_DESTROY	=			0x05, // 뽀개기 Send - | Recv - b1(1:성공 0:실패)
								N3_SP_KNIGHTS_MEMBER_JOIN_ADMIT =	0x06, // 멤버 가입 허가 Send - s1(Knights ID) | Recv - b1(1:성공 0:실패)
								N3_SP_KNIGHTS_MEMBER_JOIN_REJECT =	0x07, // 멤버 가입 거절 Send - s1(Knights ID) | Recv - b1(1:성공 0:실패)
								N3_SP_KNIGHTS_MEMBER_PUNISH =		0x08, // 멤버 징계 - 가입과 같음
								N3_SP_KNIGHTS_APPOINT_CHIEF =		0x09, // 단장 임명 - 가입과 같음
								N3_SP_KNIGHTS_APPOINT_VICECHIEF =	0x0A, // 부단장 임명 - 가입과 같음
								N3_SP_KNIGHTS_APPOINT_OFFICER =		0x0B, // 장교임명 - 가입과 같음
								N3_SP_KNIGHTS_GRADE_CHANGE_ALL =	0x0C, // 전 기사단의 등급 및 순위 변경 Recv - s1(count) Loop { s1(Knights ID) b2(Grade Rank) }
								N3_SP_KNIGHTS_MEMBER_INFO_ALL =		0x0D, // 모든 멤버 요청 Send - s1(page) | s1(Member Count) Loop { s1(Name Length) str1 (Name) }
								N3_SP_KNIGHTS_MEMBER_INFO_ONLINE =	0x0E, // 현재 접속 리스트 Send - s1(page) | s1(Member Count) Loop { s1(Name Length) str1 (Name) }
								N3_SP_KNIGHTS_STASH =				0x0F, // 기사단 창고
								N3_SP_KNIGHTS_DUTY_CHANGE =			0x10, // 멤버의 직위 변경.. 해당 멤버에게 간다.. Recv - s1(Knights ID) b1(직위);
								N3_SP_KNIGHTS_JOIN_REQ =			0x11, // 기사단 인덱스
								N3_SP_KNIGHTS_UNKNOWN };

	enum e_SubPacket_KNights_Create {	N3_SP_KNIGHTS_CREATE_FAIL_DBFAIL =			0x00,
										N3_SP_KNIGHTS_CREATE_SUCCESS =				0x01,
										N3_SP_KNIGHTS_CREATE_FAIL_LOWLEVEL =		0x02,
										N3_SP_KNIGHTS_CREATE_FAIL_DUPLICATEDNAME =	0x03,
										N3_SP_KNIGHTS_CREATE_FAIL_LOWMONEY =		0x04,
										N3_SP_KNIGHTS_CREATE_FAIL_ALREADYJOINED =	0x05,
										N3_SP_KNIGHTS_CREATE_FAIL_UNKNOWN =			0x06,
										N3_SP_KNIGHTS_CREATE_FAIL_INVALIDDAY =		0x07,
										N3_SP_KNIGHTS_CREATE_FAIL_INVALIDSERVER =	0x08 };

	enum e_SubPacket_KNights_Common {	N3_SP_KNIGHTS_COMMON_DBFAIL =				0x00,	//DB검색 실패..
										N3_SP_KNIGHTS_COMMON_SUCCESS =				0x01,	//성공
										N3_SP_KNIGHTS_COMMON_FAIL_NONE_USER =		0x02,	//없는 유저..
										N3_SP_KNIGHTS_COMMON_FAIL_DEAD_USER =		0x03,	//상대유저가 죽어 있음..
										N3_SP_KNIGHTS_COMMON_FAIL_ENEMY_USER =		0x04,	//상대유저의 국가가 다름..
										N3_SP_KNIGHTS_COMMON_FAIL_OTHER_CLAN_USER =	0x05,	//상대유저가 이미 다른 클랜이나 기사단에 가입되어 있음..
										N3_SP_KNIGHTS_COMMON_FAIL_INVALIDRIGHT =	0x06,	//권한이 없음..
										N3_SP_KNIGHTS_COMMON_FAIL_NONE_CLAN =		0x07,	//존재하지 않는 기사단..									
										N3_SP_KNIGHTS_COMMON_FAIL_FULL =			0x08,	//인원이 풀..
										N3_SP_KNIGHTS_COMMON_FAIL_ME =				0x09,	//자기자신을 선택한 경우..
										N3_SP_KNIGHTS_COMMON_FAIL_NOT_JOINED =		0x0A,	//기사단이나 클랜에 가입되어 있지 않음...
										N3_SP_KNIGHTS_COMMON_FAIL_REJECT =			0x0B,	//상대편에서 거절...
										N3_SP_KNIGHTS_COMMON_FAIL_BATTLEZONE =		0x0C	// 전쟁존에서의 거부..
								};	// join, appoint, leave, withdraw 모두 쓴다.....
	
	enum e_SubPacket_KnightsList {	N3_SP_KNIGHTS_LIST_BASIC_ALL = 0x01,  // Receive - s1(knights Count) { s21(id, 이름길이), str1(이름) }
									N3_SP_KNIGHTS_LIST_BASIC_INSERT = 0x02, // Receive - s2(id, 이름길이), str1(이름)
									N3_SP_KNIGHTS_LIST_BASIC_REMOVE = 0x03 }; // Receive - s1(id)

	enum e_SubPacket_Administrator { N3_SP_ADMINISTRATOR_ARREST = 0x01, // 그 유저 있는 곳으로 존체인지.. Send b1(Type) s1(유저이름길이), str1(유저이름)
									N3_SP_ADMINISTRATOR_FORBID_CONNECT = 0x02, // 접속금지 및 쫓아내기.. Send b1(Type) s1(유저이름길이), str1(유저이름)
									N3_SP_ADMINISTRATOR_CHAT_FORBID = 0x03, // 채팅금지 Send b1(Type) s1(유저이름길이), str1(유저이름)
									N3_SP_ADMINISTRATOR_CHAT_PERMIT = 0x04 }; // 채팅허가 Send b1(Type) s1(유저이름길이), str1(유저이름)
									
	enum e_SubPacket_WareHouse {	N3_SP_WARE_OPEN = 				0x01,	// Open..
									N3_SP_WARE_GET_IN =				0x02,	// 堅?.
									N3_SP_WARE_GET_OUT =			0X03,	// 빼기..
									N3_SP_WARE_WARE_MOVE =			0x04,	// 이동..
									N3_SP_WARE_INV_MOVE =			0x05,	// 이동..
									N3_SP_WARE_INN =				0x10,	// 맨처음..여관주인임을 받음..
									N3_SP_WARE_UNKNOWN };

	enum e_SubPacket_NoahChange {	N3_SP_NOAH_GET = 0x01,	// 노아 습득,
									N3_SP_NOAH_LOST	= 0x02,	// 노아 분실..
									N3_SP_NOAH_SPEND = 0x03	// 노아 소비..
								};		

	enum e_Party_BBS {	N3_SP_PARTY_REGISTER						= 0x01,		// 파티 게시판에 등록
						N3_SP_PARTY_REGISTER_CANCEL					= 0x02,		// 파티 게시판에서 등록 취소
						N3_SP_PARTY_BBS_DATA						= 0x03};	// 파티 게시 정보 요구

	enum e_Trade_BBS_Kind { N3_SP_TRADE_BBS_BUY						= 0x01,		// 사는 물건 목록
							N3_SP_TRADE_BBS_SELL					= 0x02};		// 파는 물건 목록

	enum e_Trade_BBS_Sub  { N3_SP_TYPE_REGISTER						= 0x01,		// 물건 등록하기
							N3_SP_TYPE_REGISTER_CANCEL				= 0x02,		// 등록 해제하기
							N3_SP_TYPE_BBS_DATA						= 0x03,		// 게시판 정보 요구
							N3_SP_TYPE_BBS_OPEN						= 0x04,		// 상거래 게시판 열기
							N3_SP_TYPE_BBS_TRADE					= 0x05};	// 게시판에서 거래 신청하기

#endif // end of #ifndef __PACKET_DEF_H_
