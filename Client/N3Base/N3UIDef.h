// N3UIDef.h: 
//
//////////////////////////////////////////////////////////////////////

#if !defined __N3UIDEF_H__
#define __N3UIDEF_H__

#include <string>

#define DIR_UITEXTURE		("Texture\\UI\\")

const float UI_DEFAULT_Z = 0.9f;
const float UI_DEFAULT_RHW = 1.0f;

// type
enum eUI_TYPE	{	UI_TYPE_BASE = 0,		// none
					UI_TYPE_BUTTON,			// button
					UI_TYPE_STATIC,			// static (배경그림과 글자가 나오는 클래스)
					UI_TYPE_PROGRESS,		// progress
					UI_TYPE_IMAGE,			// image
					UI_TYPE_SCROLLBAR,		// scroll bar
					UI_TYPE_STRING,			// string
					UI_TYPE_TRACKBAR,		// track bar
					UI_TYPE_EDIT,			// edit
					
					//cerberus 01,12,29
					UI_TYPE_AREA,			// area
					UI_TYPE_TOOLTIP,		// tooltip

					// ecli666
					UI_TYPE_ICON,			// icon
					UI_TYPE_ICON_MANAGER,	// icon manager.. 

					// repent 전용
					UI_TYPE_ICONSLOT,		// icon slot
					UI_TYPE_LIST,			// Text List...
				};

// State
enum eUI_STATE	{	UI_STATE_COMMON_NONE = 0,				// 아무렇지도 않은 그냥 평범한 상태 혹은 아이콘을 가진 윈도우가 아이콘을 선택하지 않은 상태
					UI_STATE_COMMON_MOVE,					// 움직여야 하는 
					UI_STATE_BUTTON_NORMAL,					// 아무렇지도 않은 그냥 평범한 상태..
					UI_STATE_BUTTON_DOWN,					// 버튼이 눌린상태
					UI_STATE_BUTTON_DOWN_2CHECKDOWN,		// 버튼이 임시적으로 눌린 상태(체크 버튼시 사용, 진짜로 눌린 상태가 아니다) 다음에 down상태로
					UI_STATE_BUTTON_DOWN_2CHECKUP,			// 버튼이 임시적으로 눌린 상태(체크 버튼시 사용, 진짜로 눌린 상태가 아니다) 다음에 normal상태로
					UI_STATE_BUTTON_ON,						// 버튼이 켜진 상태.. 
					UI_STATE_BUTTON_DISABLE,				// 버튼이 비활성화된 상태
					UI_STATE_BUTTON_CLICK,					// 버튼이 눌렸다 떨어진 상태 - Click.
					UI_STATE_SCROLLBAR_NULL,				// 아무상태도 아님..
					UI_STATE_SCROLLBAR_TOPBUTTON_DOWN,		// 위로 올리는 버튼이 눌린 상태.
					UI_STATE_SCROLLBAR_BOTTOMBUTTON_DOWN,	// 아래로 내리는 버튼이 눌린상태.
					UI_STATE_EDTCTRL_ACTIVE,
					UI_STATE_EDTCTRL_UNACTIVE,
					UI_STATE_TRACKBAR_THUMBDRAG,			// Thumb를 드래그 하는 중이다.
					UI_STATE_LIST_ENABLE,					// List 에서 선택 가능
					UI_STATE_LIST_DISABLE,					// List 에서 선택 불가능

					// ecli666 (아이콘을 가진 윈도우.. 아이콘 자체는 아니다..)
//					UI_STATE_ICON_NONE,						// 아이콘을 가진 윈도우가 아이콘을 선택하지 않은 상태..
					UI_STATE_ICON_MOVING,					// 아이콘을 가진 윈도우가 아이콘을 움직이고 있는 상태..
//					UI_STATE_ICON_SELECTED,					// 아이콘을 가진 윈도우가 아이콘을 선택한 상태..
					UI_STATE_ICON_WAIT_FROM_SERVER,			// 아이콘을 가진 윈도우가 아이콘을 옮기고 서버로 부터 응답을 기다리는 상태..
					UI_STATE_ICON_DO_SUCCESS,				// 아이콘을 가진 윈도우가 서버로 부터 성공을 받은 상태..
					UI_STATE_ICON_DO_FAIL,					// 아이콘을 가진 윈도우가 서버로 부터 실패를 받은 상태..
					UI_STATE_ICON_DO_RECOVERY,				// 아이콘을 가진 윈도우가 서버로 부터 실패를 받아 복구해야 하는 상태..
				};

// message
const uint32_t UIMSG_BUTTON_CLICK		= 0x00000001;		// normal 버튼 클릭
const uint32_t UIMSG_TRACKBAR_POS		= 0x00000010;		// trackbar의 pos가 변경됨
const uint32_t UIMSG_SCROLLBAR_POS		= 0x00000100;		// scrollbar의 pos가 변경됨.
const uint32_t UIMSG_EDIT_RETURN		= 0x00001000;		// Edit에서 enter가 들어옴
const uint32_t UIMSG_EDIT_TAB			= 0x00002000;		// Edit에서 Tab이 들어옴 - NOTE: Officially this isn't triggered anymore
const uint32_t UIMSG_EDIT_ESCAPE		= 0x00004000;		// Edit에서 esc이 들어옴
const uint32_t UIMSG_ICON_DOWN_FIRST	= 0x00010000;		// Icon에 왼쪽 마우스 버튼 처음으로 다운..
const uint32_t UIMSG_ICON_DOWN			= 0x00020000;		// Icon에 왼쪽 마우스 버튼 다운..
const uint32_t UIMSG_ICON_UP			= 0x00040000;		// Icon에 왼쪽 마우스 버튼 업..
const uint32_t UIMSG_ICON_DBLCLK		= 0x00080000;		// Icon에 버튼 더블 클릭
// const uint32_t UIMSG_AREA_DOWN_FIRST	= 0x00100000;		// Area에 왼쪽 마우스 버튼 처음으로 다운.. - intended for the inventory only, but it's unused. Use UIMSG_BUTTON_CLICK instead.
const uint32_t UIMSG_LIST_SELCHANGE	= 0x00200000;		// List Selection Change..
const uint32_t UIMSG_LIST_DBLCLK		= 0x00400000;
const uint32_t UIMSG_ICON_RDOWN_FIRST	= 0x01000000;		// Icon에 오른쪽 마우스 버튼 처음으로 다운..
const uint32_t UIMSG_ICON_RDOWN		= 0x02000000;		// Icon에 오른쪽 마우스 버튼 다운..
const uint32_t UIMSG_ICON_RUP			= 0x04000000;		// Icon에 오른쪽 마우스 버튼 업..
const uint32_t UIMSG_ICON_RDBLCLK		= 0x08000000;		// Icon에 오른쪽 마우스 더블 클릭
const uint32_t UIMSG_STRING_LCLICK		= 0x10000000;		// string에 마우스가 왼쪽 클릭 되었을때...
const uint32_t UIMSG_STRING_LDCLICK	= 0x20000000;		// string에 마우스가 왼쪽 더블 클릭 되었을때...

// 리펜트용 메세지
const uint32_t UIMSG_ICONSLOT_LBCLICK	= 0x00010000;		// icon slot에 왼쪽 마우스 버튼 클릭
const uint32_t UIMSG_ICONSLOT_RBCLICK	= 0x00020000;		// icon slot에 오른쪽 마우스 버튼 클릭
const uint32_t UIMSG_ICONSLOT_MBCLICK	= 0x00040000;		// icon slot에 왼쪽 마우스 버튼 클릭
const uint32_t UIMSG_ICONSLOT_MOUSEOVER= 0x00080000;		// icon slot에 마우스가 걸쳐 있다.

// mouse flag (localinput.h의 값과 일치해야 한다.)
const uint32_t UI_MOUSE_LBCLICK	= 0x00000001;
const uint32_t UI_MOUSE_LBCLICKED	= 0x00000002;
const uint32_t UI_MOUSE_LBDOWN		= 0x00000004;
const uint32_t UI_MOUSE_MBCLICK	= 0x00000008;
const uint32_t UI_MOUSE_MBCLICKED	= 0x00000010;
const uint32_t UI_MOUSE_MBDOWN		= 0x00000020;
const uint32_t UI_MOUSE_RBCLICK	= 0x00000040;
const uint32_t UI_MOUSE_RBCLICKED	= 0x00000080;
const uint32_t UI_MOUSE_RBDOWN		= 0x00000100;
const uint32_t UI_MOUSE_LBDBLCLK	= 0x00000200;
const uint32_t UI_MOUSE_MBDBLCLK	= 0x00000400;
const uint32_t UI_MOUSE_RBDBLCLK	= 0x00000800;

// mouse procedure return value flag
const uint32_t UI_MOUSEPROC_NONE			= 0x00000000;	// 아무 일도 하지 않았다.
const uint32_t UI_MOUSEPROC_DONESOMETHING	= 0x00000001;	// 먼가 일을 했다.
const uint32_t UI_MOUSEPROC_CHILDDONESOMETHING	= 0x00000002;	// 자식이 먼가 일을 했다.(이 플래그가 설정되어 있으면 항상 UI_MOUSEPROC_DONESOMETHING도 설정되어있다.)
const uint32_t UI_MOUSEPROC_INREGION		= 0x00000004;	// 영역 안에 마우스 포인터가 있다.
const uint32_t UI_MOUSEPROC_PREVINREGION	= 0x00000008;	// 영역 안에 이전 틱의 마우스 포인터가 있었다.
const uint32_t UI_MOUSEPROC_DIALOGFOCUS	= 0x00000010;	// dialog가 포커스 받았다.


// ui_string type
const uint32_t UI_STR_TYPE_LINE			= 0x00000000;	// 스트링 라인 설정 싱글라인인지 멀티라인인지..
const uint32_t UI_STR_TYPE_HALIGN			= 0x00000001;	// 스트링 수평정렬
const uint32_t UI_STR_TYPE_VALIGN			= 0x00000002;	// 스트링 수직정렬

// ui style
const uint32_t UISTYLE_NONE				= 0x00000000;
const uint32_t UISTYLE_ALWAYSTOP			= 0x00000001;	// 항상 최상위에
const uint32_t UISTYLE_MODAL				= 0x00000002;	// modal dialog
const uint32_t UISTYLE_FOCUS_UNABLE		= 0x00000004;	// 포커스를 받을수 없는 유아이
const uint32_t UISTYLE_SHOW_ME_ALONE		= 0x00000008;	// 단지 자기 자신만 열릴수 있는 다른것은 닫혀야한다면..
const uint32_t UISTYLE_HIDE_UNABLE			= 0x00000010;	// 닫히지 않는 유아이
const uint32_t UISTYLE_USER_MOVE_HIDE		= 0x00000020;	// 유저가 움직이면 닫히는 유아이
const uint32_t UISTYLE_POS_LEFT			= 0x00000040;	// 왼쪽에 달린 유아이
const uint32_t UISTYLE_POS_RIGHT			= 0x00000080;	// 오른쪽에 달린 유아이

	// button
const uint32_t UISTYLE_BTN_NORMAL			= 0x00010000;	// 일반 버튼
const uint32_t UISTYLE_BTN_CHECK			= 0x00020000;	// 체크 버튼(toggle버튼)
	// image
const uint32_t UISTYLE_IMAGE_ANIMATE		= 0x00010000;	// 에니메이션 되는 이미지이다.
	// string
const uint32_t UISTYLE_STRING_MULTILINE	= 0x00000000;	// 여러줄로 표시된다.
const uint32_t UISTYLE_STRING_SINGLELINE	= 0x00100000;	// 한줄로만 표시된다.
const uint32_t UISTYLE_STRING_ALIGNLEFT	= 0x00200000;	// 왼쪽 정렬(default)
const uint32_t UISTYLE_STRING_ALIGNRIGHT	= 0x00400000;	// 오른쪽 정렬 (한줄일때만)
const uint32_t UISTYLE_STRING_ALIGNCENTER	= 0x00800000;	// 가운데 정렬 (한줄일때만)
const uint32_t UISTYLE_STRING_ALIGNTOP		= 0x01000000;	// 상단 정렬 (한줄일때만, default)
const uint32_t UISTYLE_STRING_ALIGNBOTTOM	= 0x02000000;	// 하단 정렬 (한줄일때만)
const uint32_t UISTYLE_STRING_ALIGNVCENTER	= 0x04000000;	// 수직가운데 정렬 (한줄일때만)
	// edit
const uint32_t UISTYLE_EDIT_PASSWORD		= 0x10000000;	// 암호를 입력받는 edit이다.
const uint32_t UISTYLE_EDIT_NUMBERONLY		= 0x20000000;
	// progress
const uint32_t UISTYLE_PROGRESS_LEFT2RIGHT	= 0x10000000;	// 왼쪽에서 오른쪽으로 증가(default)
const uint32_t UISTYLE_PROGRESS_RIGHT2LEFT	= 0x20000000;	// 오른쪽에서 왼쪽으로 증가
const uint32_t UISTYLE_PROGRESS_TOP2BOTTOM	= 0x40000000;	// 위쪽에서 아래쪽으로 증가
const uint32_t UISTYLE_PROGRESS_BOTTOM2TOP	= 0x80000000;	// 아래쪽에서 위쪽으로 증가
	// TrackBar
const uint32_t UISTYLE_TRACKBAR_HORIZONTAL	= 0x00010000;	// 가로(default)
const uint32_t UISTYLE_TRACKBAR_VERTICAL	= 0x00020000;	// 세로
	// ScrollBar
const uint32_t UISTYLE_SCROLLBAR_HORIZONTAL= 0x00010000;	// 가로(default)
const uint32_t UISTYLE_SCROLLBAR_VERTICAL	= 0x00020000;	// 세로

	// Icon type.. ecli666..
const uint32_t UISTYLE_ICON_ITEM			= 0x00000010;	// 아이템 아이콘..
const uint32_t UISTYLE_ICON_SKILL			= 0x00000020;	// 스킬 아이콘..
const uint32_t UISTYLE_ICON_CERTIFICATION_NEED		= 0X00000100;	// 서버로 부터 인증이 필요한 아이콘..
const uint32_t UISTYLE_ICON_CERTIFICATION_NONEED	= 0X00000200;	// 서버로 부터 인증이 불필요한 아이콘..
const uint32_t UISTYLE_ICON_HIGHLIGHT				= 0x00001000;	// No highlight Icon.. 
const uint32_t UISTYLE_DURABILITY_EXHAUST			= 0x00002000;	// Durability exhausted Icon.. 
const uint32_t UISTYLE_DISABLE_SKILL	= 0x00004000;	// Disable Skill Icon..
const uint32_t UISTYLE_ICON_NO_HIGHLIGHT			= 0x00000000;	// Highlight Icon..

// structures
struct __FLOAT_RECT
{
	float left, top, right, bottom;
	BOOL PtInRect2D(float x, float y) const {return ((x>=left && x<=right && y>=top && y<=bottom)?TRUE:FALSE);}
	BOOL PtInRect3D(float x, float y) const {return ((x>=left && x<=right && y>=bottom && y<=top)?TRUE:FALSE);}
};

#endif // #if !defined __N3UIDEF_H__
