#ifndef _EXTERN_H_
#define _EXTERN_H_

// -------------------------------------------------
// 전역 객체 변수
// -------------------------------------------------
extern BOOL	g_bNpcExit;

struct	_PARTY_GROUP
{
	WORD wIndex;
	short uid[8];		// 하나의 파티에 8명까지 가입가능
	_PARTY_GROUP()
	{
		for (int i = 0;i < 8;i++)
			uid[i] = -1;
	}
};

struct _MAKE_WEAPON
{
	BYTE	byIndex;		// 몹의 레벨 기준
	short	sClass[MAX_UPGRADE_WEAPON];		// 1차무기 확률
	_MAKE_WEAPON()
	{
		for (int i = 0;i < MAX_UPGRADE_WEAPON;i++)
			sClass[i] = 0;
	}
};

struct _MAKE_ITEM_GRADE_CODE
{
	BYTE	byItemIndex;		// item grade
	short	sGrade_1;			// 단계별 확률
	short	sGrade_2;
	short	sGrade_3;
	short	sGrade_4;
	short	sGrade_5;
	short	sGrade_6;
	short	sGrade_7;
	short	sGrade_8;
	short	sGrade_9;
};

struct _MAKE_ITEM_LARE_CODE
{
	BYTE	byItemLevel;			// item level 판단 
	short	sLareItem;				// lareitem 나올 확률
	short	sMagicItem;				// magicitem 나올 확률
	short	sGereralItem;			// gereralitem 나올 확률
};

struct _MAGIC_TABLE
{
	int		iNum;
	BYTE	bMoral;
	BYTE	bSkillLevel;
	short	sSkill;
	short	sMsp;
	short   sHP;
	BYTE	bItemGroup;
	int		iUseItem;
	BYTE	bCastTime;
	BYTE	bReCastTime;
	BYTE	bSuccessRate;
	BYTE	bType1;
	BYTE	bType2;
	short   sRange;
	BYTE	bEtc;
};

struct _MAGIC_TYPE1
{
	int		iNum;
	BYTE	bHitType;
	short	sHitRate;
	short	sHit;
	BYTE	bDelay;
	BYTE	bComboType;
	BYTE	bComboCount;
	short	sComboDamage;
	short	sRange;
};

struct _MAGIC_TYPE2
{
	int     iNum;
	BYTE    bHitType;
	short   sHitRate;
	short	sAddDamage;
	short   sAddRange;
	BYTE    bNeedArrow;
};

struct _MAGIC_TYPE3
{
	int		iNum;
	//BYTE	bDistance;
	BYTE	bRadius;
	short	sAngle;
	short	sFirstDamage;
	short	sEndDamage;
	short	sTimeDamage;
	BYTE	bDirectType;
	short	sDuration;
	BYTE	bAttribute;
};

struct _MAGIC_TYPE4
{
	int     iNum;
	short   sMSP;
	BYTE    bBuffType;
	BYTE    bRadius;
	short   sDuration;
	BYTE    bAttackSpeed;
	BYTE    bSpeed;
	short   sAC;
	BYTE    bAttack;
	short   sMaxHP;
	BYTE    bHitRate;
	short   sAvoidRate;
	BYTE    bStr;
	BYTE    bSta;
	BYTE    bDex;
	BYTE    bIntel;
	BYTE    bCha;
	BYTE    bFireR;
	BYTE    bColdR;
	BYTE    bLightningR;
	BYTE    bMagicR;
	BYTE    bDiseaseR;
	BYTE    bPoisonR;
};

struct	_USERLOG
{
	CTime t;
	BYTE  byFlag;	// 
	BYTE  byLevel;
	char  strUserID[MAX_ID_SIZE + 1];		// 아이디(캐릭터 이름)
};

#endif