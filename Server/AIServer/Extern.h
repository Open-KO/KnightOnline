#ifndef _EXTERN_H_
#define _EXTERN_H_

// -------------------------------------------------
// 전역 객체 변수
// -------------------------------------------------
extern bool	g_bNpcExit;

import AIServerModel;
namespace model = aiserver_model;

struct _PARTY_GROUP
{
	WORD wIndex;
	short uid[8];		// 하나의 파티에 8명까지 가입가능
	_PARTY_GROUP()
	{
		for (int i = 0; i < 8; i++)
			uid[i] = -1;
	}
};

struct _USERLOG
{
	CTime t;
	BYTE  byFlag;	// 
	BYTE  byLevel;
	char  strUserID[MAX_ID_SIZE + 1];		// 아이디(캐릭터 이름)
};

#endif
