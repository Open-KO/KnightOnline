// N3IME.h: interface for the CN3IME class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3IME_H__87E26E3B_B5EB_403B_8126_2884169AFEE9__INCLUDED_)
#define AFX_N3IME_H__87E26E3B_B5EB_403B_8126_2884169AFEE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <imm.h>
#include <string>
using namespace std;

const uint32_t IME_IN_COMPOSITION = 0x00000001;
const uint32_t IME_IN_CHOSECAND = 0x00000002;

#define MAX_LISTCAND 32
#define MAX_CHARS_PER_LINE 10
#define MAX_COMP_STRING_LEN 10
#define DEFAULT_CAND_NUM_PER_PAGE 5

// char	CANDWND_CLASS[] = "HanjaCandidate";

class CN3IME
{
  public:
    CN3IME();
    virtual ~CN3IME();

    // Attributes
  public:
  protected:
    HWND m_hWnd;
    HIMC m_hIMC;
    HKL m_hKeyLayout;
    uint32_t m_dwProperty;
    uint32_t m_dwState;

    // Operations
  public:
    BOOL InitIME(HWND hwnd);
    LRESULT IMEMessageProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

  protected:
    inline BOOL Enter()
    {
        if (m_hWnd != nullptr)
            return (BOOL)(m_hIMC = ImmGetContext(m_hWnd));
        else
            return FALSE;
    };
    inline void Leave()
    {
        ImmReleaseContext(m_hWnd, m_hIMC);
    };
    inline BOOL Check()
    {
        return (!(m_dwProperty & IME_PROP_SPECIAL_UI) && (m_dwProperty & IME_PROP_AT_CARET));
    }

    // for Message
    LRESULT OnInputLangChange(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMESetContext(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMEStartComposition(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMEComposition(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMEEndComposition(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMECompositionFull(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMENotify(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnIMEControl(WPARAM dwCommand, LPARAM dwData);
    LRESULT OnChar(WPARAM wParam);
    LRESULT OnKeyDown(WPARAM wParam);

    BOOL OpenCandidate(int32_t lParam);
    BOOL CloseCandidate(int32_t CandList);
    BOOL ChangeCandidate(int32_t CandList);
    BOOL SetOpenStatus();

    void ClearData();
    BOOL GetResultString();
    BOOL GetCompString(int32_t flag);
};

#endif // !defined(AFX_N3IME_H__87E26E3B_B5EB_403B_8126_2884169AFEE9__INCLUDED_)
