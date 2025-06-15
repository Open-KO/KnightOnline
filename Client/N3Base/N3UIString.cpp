// N3UIString.cpp: implementation of the CN3UIString class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIString.h"

#ifndef _REPENT
#ifdef _N3GAME
#include "..\WarFare\N3UIWndBase.h"
#include "..\WarFare\UIInventory.h"
#endif 
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3UIString::CN3UIString()
{
	m_eType = UI_TYPE_STRING;
	m_pDFont = NULL;

	m_Color = 0xffffffff;
	ZeroMemory(&m_ptDrawPos, sizeof(m_ptDrawPos));
	m_iLineCount = 0;
	m_iStartLine = 0;
}

CN3UIString::~CN3UIString()
{
	delete m_pDFont;	m_pDFont = NULL;
}

void CN3UIString::Release()
{
	CN3UIBase::Release();

	m_szString = "";
	delete m_pDFont;	m_pDFont = NULL;
	m_Color = 0xffffffff;

	ZeroMemory(&m_ptDrawPos, sizeof(m_ptDrawPos));
	m_iLineCount = 0;
	m_iStartLine = 0;
	m_NewLineIndices.clear();
}

void CN3UIString::Render()
{
	if(!m_bVisible) return;
	
	__ASSERT(m_pDFont,"m_pDFont is NULL");
	m_pDFont->DrawText((float)m_ptDrawPos.x, (float)m_ptDrawPos.y, m_Color, 0 );

	CN3UIBase::Render();
}

void CN3UIString::Init(CN3UIBase* pParent)
{
	CN3UIBase::Init(pParent);

	if(m_pDFont) delete m_pDFont; m_pDFont = NULL;
	m_pDFont = new CDFont("굴림", 16);	// default 로 굴림 16으로 설정
	m_pDFont->InitDeviceObjects( CN3Base::s_lpD3DDev );
	m_pDFont->RestoreDeviceObjects();
}

void CN3UIString::SetString(const std::string& szString)
{
	m_szString = szString;

	if(m_szString.empty()) m_pDFont->SetText("");
	else WordWrap();
}

void CN3UIString::SetStringAsInt(int iVal)
{
	char szBuff[32] = "";
	sprintf(szBuff, "%d", iVal);
	this->SetString(szBuff);
}

void CN3UIString::SetString_NoWordWrap(const std::string& szString)
{
	// 왼쪽위 정렬해서 그린다.
	m_ptDrawPos.x = m_rcRegion.left;
	m_ptDrawPos.y = m_rcRegion.top;

	m_pDFont->SetText(szString.c_str());
}

void CN3UIString::SetFont(const std::string& szFontName, uint32_t dwHeight, BOOL bBold, BOOL bItalic)
{
	uint32_t dwFlag = 0;
	if (bBold) dwFlag |= D3DFONT_BOLD;
	if (bItalic) dwFlag |= D3DFONT_ITALIC;
	if (m_pDFont)
	{
		m_pDFont->SetFont(szFontName, dwHeight, dwFlag);
		WordWrap();
	}
}

void CN3UIString::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	WordWrap();
}

void CN3UIString::SetStyle(uint32_t dwStyle)
{
	CN3UIBase::SetStyle(dwStyle);
	WordWrap();
}

// 글씨찍는 위치도 바뀌어 준다.
BOOL CN3UIString::MoveOffset(int iOffsetX, int iOffsetY)
{
	if (FALSE == CN3UIBase::MoveOffset(iOffsetX, iOffsetY)) return FALSE;
	m_ptDrawPos.x += iOffsetX;	m_ptDrawPos.y += iOffsetY;
	return TRUE;
}

// 문자를 dwStyle에 맞게 정렬해준다.
void CN3UIString::WordWrap()
{
	m_iLineCount = 0;

	if (NULL == m_pDFont) return;
	int iStrLen = m_szString.size();

	if (0 == iStrLen)
	{
		m_pDFont->SetText("");

		// 글씨를 찍지는 않지만 edit의 경우 caret을 찍기때문에 글씨 그리는 좌표를 세팅해준다.
		if (m_dwStyle & UISTYLE_STRING_SINGLELINE)
		{
			if (m_dwStyle & UISTYLE_STRING_ALIGNCENTER)	m_ptDrawPos.x = m_rcRegion.left + (m_rcRegion.right - m_rcRegion.left)/2;
			else m_ptDrawPos.x = m_rcRegion.left;

			if (m_dwStyle & UISTYLE_STRING_ALIGNVCENTER)
			{
				// 문자열의 pixel 길이 측정
				SIZE sizeTmp = {0, 0};
				m_pDFont->GetTextExtent("가", 2, &sizeTmp);
				m_ptDrawPos.y = m_rcRegion.top + ((m_rcRegion.bottom - m_rcRegion.top-sizeTmp.cy)/2);
			}
			else m_ptDrawPos.y = m_rcRegion.top;
		}
		else
		{
			m_ptDrawPos.x = m_rcRegion.left;
			m_ptDrawPos.y = m_rcRegion.top;
		}

		return;
	}

	// 문자열의 pixel 길이 측정
	SIZE size;
	const std::string szString = GetString();
	iStrLen = szString.size();
	if (FALSE == m_pDFont->GetTextExtent(szString.c_str(), iStrLen, &size))
	{	// 길이를 측정할 수 없을경우(m_hDC가 생성되지 않았을경우)
		m_pDFont->SetText(szString);
		return;
	}

	int iRegionWidth = m_rcRegion.right - m_rcRegion.left;
	int iRegionHeight = m_rcRegion.bottom - m_rcRegion.top;

	if (m_dwStyle & UISTYLE_STRING_SINGLELINE)
	{
		// 임시 변수 잡기
		std::string szNewBuff;

		/*if (size.cy>iRegionHeight)	// 글자 높이가 해당 영역보다 큰 경우
		{	
			// NOTE: need to look into putting this back!!!

			m_ptDrawPos.y = m_rcRegion.top;
			//m_pDFont->SetText("글자 높이가 STRING control보다 큽니다.");
			m_pDFont->SetText("Temp1");
			return;
		}
		else*/ if (size.cx <= iRegionWidth)	// 글자 가로 길이가 영역 길이보다 작을경우
		{
			szNewBuff = m_szString;

			// 가로 정렬 
			if (m_dwStyle & UISTYLE_STRING_ALIGNRIGHT) m_ptDrawPos.x = m_rcRegion.right - size.cx;
			else if (m_dwStyle & UISTYLE_STRING_ALIGNCENTER) m_ptDrawPos.x = m_rcRegion.left + ((iRegionWidth-size.cx)/2);
			else m_ptDrawPos.x = m_rcRegion.left;
			// 세로 정렬
			if (m_dwStyle & UISTYLE_STRING_ALIGNBOTTOM) m_ptDrawPos.y = m_rcRegion.bottom-size.cy;
			else if (m_dwStyle & UISTYLE_STRING_ALIGNVCENTER) m_ptDrawPos.y = m_rcRegion.top + ((iRegionHeight-size.cy)/2);
			else m_ptDrawPos.y = m_rcRegion.top;
			m_iLineCount = 1;
		}
		else	// 글자 가로 길이가 영역 길이보다 클경우 영역안에 들어가는 글자만 표시하자.
		{
			// 가로 정렬은 왼쪽 정렬로.
			m_ptDrawPos.x = m_rcRegion.left;
			// 세로 정렬
			if (m_dwStyle & UISTYLE_STRING_ALIGNBOTTOM) m_ptDrawPos.y = m_rcRegion.bottom-size.cy;
			else if (m_dwStyle & UISTYLE_STRING_ALIGNVCENTER) m_ptDrawPos.y = m_rcRegion.top + ((iRegionHeight-size.cy)/2);
			else m_ptDrawPos.y = m_rcRegion.top;

			// 글자 자르는 코드
			int iCount = 0;
			while(iCount<iStrLen)
			{
				if ('\n' == szString[iCount])		// \n
				{
					break;
				}
				else if (0x80 & szString[iCount])	// 2BYTE 문자
				{
					BOOL bFlag = m_pDFont->GetTextExtent(szString.c_str(), iCount+2, &size);
					__ASSERT(bFlag, "cannot get size of dfont");
					if (size.cx>iRegionWidth) break;
					iCount += 2;
				}
				else								// 1BYTE 문자
				{
					BOOL bFlag = m_pDFont->GetTextExtent(szString.c_str(), iCount+1, &size);
					__ASSERT(bFlag, "cannot get size of dfont");
					if (size.cx>iRegionWidth) break;
					++iCount;
				}
			}
			// strDisplayString 에 iCount 만큼만 카피하기
			szNewBuff = m_szString.substr(0, iCount);	// +1은 맨 마지막에 NULL 넣기 위해
			m_iLineCount = 1;
		}
		m_pDFont->SetText(szNewBuff);

	}
	else
	{	// mutiline일때는 항상 왼쪽 정렬해서 그린다.
		m_ptDrawPos.x = m_rcRegion.left;
		m_ptDrawPos.y = m_rcRegion.top;

		m_NewLineIndices.clear();	// 새로운 라인 인덱스 클리어

		// 글자 자르는 코드, 영역 밖으로 벗어나는 글자는 찍지 않는다.
		int iCX=0;//, iCY=0;
		int iCount = 0;

		// 우선 맨 처음 한줄이 들어갈 수 있는 크기인지 체크하기
		BOOL bFlag = m_pDFont->GetTextExtent("최", 2, &size);
		__ASSERT(bFlag, "cannot get size of dfont");
//		iCY += size.cy;
//		if (iCY > iRegionHeight)
		if (size.cy > iRegionHeight)
		{
			// NOTE: need to look into returning this back !!!

			//m_pDFont->SetText("글자 높이가 STRING control보다 큽니다.");
			//m_pDFont->SetText("Temp2");
			//return;
		}

		m_iLineCount = 1;	// 여기까지 오면 1줄은 찍힌다.
		m_NewLineIndices.push_back(0);	// 맨 첨 시작은 0

		while(iCount<iStrLen)
		{
			if ('\n' == szString[iCount])		// \n
			{
//				szNewBuff += '\n';
//				iCY += size.cy;
//				if (iCY > iRegionHeight) break;	// 세로 범위가 넘으면 더이상 글자를 찍지 않는다.
				++iCount;
				iCX = 0;
				if (iCount<iStrLen-1)
				{
					++m_iLineCount;	// 마지막 글자가 아닐경우 한줄 더하기 
					m_NewLineIndices.push_back(iCount);
				}
			}
			else
			{
				int iCC=0;
				if (0x80 & szString[iCount])	iCC = 2;	// 2BYTE 문자
				else iCC = 1;	// 1BYTE 문자

				BOOL bFlag = m_pDFont->GetTextExtent(&(szString[iCount]), iCC, &size);
				__ASSERT(bFlag, "cannot get size of dfont");
				if ((iCX+size.cx) > iRegionWidth)	// 가로 길이가 넘었으면
				{
//					szNewBuff += '\n';	// 다음줄로 내린다.
					iCX = 0;
//					iCY += size.cy;
//					if (iCY > iRegionHeight) break;	// 세로 범위가 넘으면 더이상 글자를 찍지 않는다.
					if (iCount<iStrLen-1)
					{
						++m_iLineCount;	// 마지막 글자가 아닐경우 한줄 더하기 
						m_NewLineIndices.push_back(iCount);
					}
				}
				// 글자 카피
//				szNewBuff += szString.substr(iCount, iCC);
				
				iCount += iCC;
				iCX += size.cx;
			}
		}
		SetStartLine(0);
	}
	//m_pDFont->SetText(szNewBuff);
}

void CN3UIString::SetStartLine(int iLine)
{
	if ((m_dwStyle & UISTYLE_STRING_SINGLELINE) || iLine >= m_iLineCount) return;
	m_iStartLine = iLine;

	SIZE size = {0,0};
	BOOL bFlag = m_pDFont->GetTextExtent("최", 2, &size);
	__ASSERT(bFlag, "cannot get size of dfont");
	if (0 == size.cy) return;

	int iEndLine = m_iStartLine + ((m_rcRegion.bottom - m_rcRegion.top)/size.cy);
	bool bMoreLine = true;
	if (iEndLine >= m_iLineCount)
	{
		iEndLine = m_iLineCount;
		bMoreLine = false;
	}
	int i, iCC, iSize;
	std::string strNew;
	for (i=m_iStartLine; i<iEndLine-1; ++i)
	{
		iCC = m_NewLineIndices[i+1] - m_NewLineIndices[i];
		if (iCC>0)
		{
			strNew += m_szString.substr(m_NewLineIndices[i], iCC);
			iSize = strNew.size();
			if ((iSize>0) && ('\n' != strNew[iSize-1])) strNew += "\n";
		}
	}
	// 마지막줄 처리
	if (bMoreLine)
	{
		if (iEndLine > 0) {
			iCC = m_NewLineIndices[iEndLine] - m_NewLineIndices[iEndLine - 1];
			if (iCC > 0) strNew += m_szString.substr(m_NewLineIndices[i], iCC);
		}
	}
	else
	{
		if (iEndLine > 0) {
			iCC = m_szString.size() - m_NewLineIndices[iEndLine - 1];
			if (iCC > 0) strNew += m_szString.substr(m_NewLineIndices[i], iCC);
		}
	}
	m_pDFont->SetText(strNew);
}

bool CN3UIString::Load(HANDLE hFile)
{
	if (false == CN3UIBase::Load(hFile)) return false;
	DWORD dwNum;
	// font 정보
	int iStrLen = 0;
	ReadFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, NULL);			// font 이름 길이 
	if (iStrLen>0)
	{
		std::string szFontName(iStrLen, '?');
		ReadFile(hFile, &(szFontName[0]), iStrLen, &dwNum, NULL);				// string

		uint32_t dwFontFlags = 0, dwFontHeight = 0;
		ReadFile(hFile, &dwFontHeight, sizeof(dwFontHeight), &dwNum, NULL);	// font height
		ReadFile(hFile, &dwFontFlags, sizeof(dwFontFlags), &dwNum, NULL);	// font flag (bold, italic)

		SetFont(szFontName, dwFontHeight, dwFontFlags & D3DFONT_BOLD, dwFontFlags & D3DFONT_ITALIC);	// 글꼴 지정
	}
#ifdef _N3TOOL
	else
	{
		SetFont("굴림", 10, FALSE, FALSE);	// 임시로 글꼴 지정
		MessageBox(s_hWndBase, "폰트가 지정되지 않은 UIString이 있어서 굴림(10)으로 설정하였습니다.", "No font", MB_OK);
	}
#else
	__ASSERT(iStrLen>0, "No font name");
#endif

	// string
	ReadFile(hFile, &m_Color, sizeof(m_Color), &dwNum, NULL);			// 글자 색
	ReadFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, NULL);			// string 길이 
	if (iStrLen>0)
	{
		std::string szString(iStrLen, '?');
		ReadFile(hFile, &(szString[0]), iStrLen, &dwNum, NULL);				// string
		SetString(szString);
	}

	// NOTE: testing UI string
	if(m_iFileFormatVersion >= N3FORMAT_VER_1264) {
		//char temp[0xFF];

		//sprintf(temp, "\nm_dwReserved = %d\n", m_dwReserved);
		//OutputDebugString(temp);
		//sprintf(temp, "m_dwStyle = %d\n", m_dwStyle);
		//OutputDebugString(temp);

		int iIdk0;
		ReadFile(hFile, &iIdk0, sizeof(int), &dwNum, NULL);

		//sprintf(temp, "iIdk0 = %d\n", iIdk0);
		//OutputDebugString(temp);
	}

	return true;
}

void CN3UIString::operator = (const CN3UIString& other)
{
	CN3UIBase::operator = (other);

	m_ptDrawPos = other.m_ptDrawPos;		// 실제 화면에 표시될 글자의 제일 왼쪽 상단 좌표
	m_Color = other.m_Color;			// 글자 색

	// 폰트 설정
	uint32_t dwFontFlags = other.GetFontFlags();
	SetFont(other.GetFontName(), other.GetFontHeight(), dwFontFlags & D3DFONT_BOLD, dwFontFlags & D3DFONT_ITALIC);

	// 글씨 설정
	this->SetString(other.m_szString); // m_szString = other.m_szString;			// string buffer
}

#ifdef _N3TOOL
bool CN3UIString::Save(HANDLE hFile)
{
	if (false == CN3UIBase::Save(hFile)) return false;
	DWORD dwNum;
	// font 정보
	char* pszFontName = NULL;
	__ASSERT(m_pDFont, "no font");
	const std::string strFontName(m_pDFont->GetFontName());
	int iStrLen = strFontName.size();
	__ASSERT(iStrLen>0, "No font name");
	WriteFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, NULL);			// font 이름 길이 
	if (iStrLen>0)
	{
		WriteFile(hFile, strFontName.c_str(), iStrLen, &dwNum, NULL);				// string
		uint32_t dwFontFlags = 0, dwFontHeight = 0;
		if (m_pDFont)
		{
			dwFontHeight = m_pDFont->GetFontHeight();
			dwFontFlags = m_pDFont->GetFontFlags();
		}
		WriteFile(hFile, &dwFontHeight, sizeof(dwFontHeight), &dwNum, NULL);	// font height
		WriteFile(hFile, &dwFontFlags, sizeof(dwFontFlags), &dwNum, NULL);	// font flag (bold, italic)
	}

	// string
	WriteFile(hFile, &m_Color, sizeof(m_Color), &dwNum, NULL);			// 글자 색
	iStrLen = 0;
	iStrLen = m_szString.size();
	WriteFile(hFile, &iStrLen, sizeof(iStrLen), &dwNum, NULL);			// string 길이 
	if (iStrLen>0)
	{
		WriteFile(hFile, m_szString.c_str(), iStrLen, &dwNum, NULL);				// string
	}

	//note: to imitate original file structure, 00 00 00 00 after each string.
	char padding[4] = { 0, 0, 0, 0 };
	WriteFile(hFile, padding, 4, &dwNum, NULL);

	return true;
}

void CN3UIString::ChangeFont(const std::string& szFont)
{
	if(m_pDFont)
	{
		uint32_t dwFlag = m_pDFont->GetFontFlags();
		bool bBold = (dwFlag & D3DFONT_BOLD) ? true : false;
		bool bItalic = (dwFlag & D3DFONT_ITALIC) ? true : false;
		this->SetFont(szFont, m_pDFont->GetFontHeight(), bBold, bItalic);
	}

	CN3UIBase::ChangeFont(szFont);
}
#endif

int CN3UIString::GetStringRealWidth(int iNum)
{
	SIZE size;
	BOOL bFlag = m_pDFont->GetTextExtent("가", lstrlen("가"), &size);
	__ASSERT(bFlag, "cannot get size of dfont");
	//int iLength = iNum/2;
	//if (iLength == 0) return 0;
	//return (size.cx*iLength); 
	return (size.cx*iNum) / 2;
}

int CN3UIString::GetStringRealWidth(std::string& szText)
{
	SIZE size;
	BOOL bFlag = m_pDFont->GetTextExtent(szText, szText.length(), &size);
	__ASSERT(bFlag, "cannot get size of dfont");
	return size.cx;
}

void CN3UIString::SetStyle(uint32_t dwType, uint32_t dwStyleEx)
{
	if (dwType == UI_STR_TYPE_LINE)
	{
		uint32_t dwStyle = m_dwStyle;
		if (UISTYLE_STRING_SINGLELINE == dwStyleEx) dwStyle |= UISTYLE_STRING_SINGLELINE;
		else if (UISTYLE_STRING_MULTILINE == dwStyleEx) dwStyle &= (~UISTYLE_STRING_SINGLELINE);
		SetStyle(dwStyle);
	}
	else if (dwType == UI_STR_TYPE_HALIGN)
	{
		uint32_t dwStyle = m_dwStyle;
		dwStyle &= (~UISTYLE_STRING_ALIGNLEFT);
		dwStyle &= (~UISTYLE_STRING_ALIGNCENTER);
		dwStyle &= (~UISTYLE_STRING_ALIGNRIGHT);
		if (UISTYLE_STRING_ALIGNLEFT == dwStyleEx) dwStyle |= UISTYLE_STRING_ALIGNLEFT;
		else if (UISTYLE_STRING_ALIGNCENTER == dwStyleEx) dwStyle |= UISTYLE_STRING_ALIGNCENTER;
		else if (UISTYLE_STRING_ALIGNRIGHT == dwStyleEx) dwStyle |= UISTYLE_STRING_ALIGNRIGHT;
		SetStyle(dwStyle);
	}
	else if (dwType == UI_STR_TYPE_VALIGN)
	{
		uint32_t dwStyle = m_dwStyle;
		dwStyle &= (~UISTYLE_STRING_ALIGNTOP);
		dwStyle &= (~UISTYLE_STRING_ALIGNVCENTER);
		dwStyle &= (~UISTYLE_STRING_ALIGNBOTTOM);
		if (UISTYLE_STRING_ALIGNTOP == dwStyleEx) dwStyle |= UISTYLE_STRING_ALIGNTOP;
		else if (UISTYLE_STRING_ALIGNVCENTER == dwStyleEx) dwStyle |= UISTYLE_STRING_ALIGNVCENTER;
		else if (UISTYLE_STRING_ALIGNBOTTOM == dwStyleEx) dwStyle |= UISTYLE_STRING_ALIGNBOTTOM;
		SetStyle(dwStyle);
	}
}

uint32_t CN3UIString::MouseProc(uint32_t dwFlags, const POINT &ptCur, const POINT &ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;

#ifndef _REPENT
#ifdef _N3GAME
	if ( CN3UIWndBase::m_sRecoveryJobInfo.m_bWaitFromServer ) return dwRet;
#endif
#endif

	// 특정 이벤트에 대해 메시지 전송..
	if(IsIn(ptCur.x, ptCur.y) && (dwFlags & UI_MOUSE_LBCLICKED) )	
	{
		m_pParent->ReceiveMessage(this, UIMSG_STRING_LCLICK); // 부모에게 버튼 클릭 통지..
//		dwRet |= UI_MOUSEPROC_DONESOMETHING;
	}

	if(IsIn(ptCur.x, ptCur.y) && (dwFlags & UI_MOUSE_LBDBLCLK) )	
	{
		m_pParent->ReceiveMessage(this, UIMSG_STRING_LDCLICK); // 부모에게 버튼 클릭 통지..
//		dwRet |= UI_MOUSEPROC_DONESOMETHING;
	}

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}
