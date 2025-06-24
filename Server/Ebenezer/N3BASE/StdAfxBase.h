#if !defined(AFX_STDAFX_H__N3BASE__INCLUDED_)
#define AFX_STDAFX_H__N3BASE__INCLUDED_

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

// TODO: reference additional headers your program requires here
#include "AfxVer_.h"

#include "My_3DStruct.h"

/////////////////////////////////////////////////////////////////////////////
// Diagnostic memory management routines

// Low level sanity checks for memory blocks
BOOL AFXAPI AfxIsValidAddress(const void* lp,
			UINT nBytes, BOOL bReadWrite = TRUE);
BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, int nLength = -1);
BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, int nLength = -1);

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)

// Memory tracking allocation
void* AFX_CDECL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#if _MSC_VER >= 1200
void AFX_CDECL operator delete(void* p, LPCSTR lpszFileName, int nLine);
#endif

void* AFXAPI AfxAllocMemoryDebug(size_t nSize, BOOL bIsObject,
	LPCSTR lpszFileName, int nLine);
void AFXAPI AfxFreeMemoryDebug(void* pbData, BOOL bIsObject);

// Dump any memory leaks since program started
BOOL AFXAPI AfxDumpMemoryLeaks();

// Return TRUE if valid memory block of nBytes
BOOL AFXAPI AfxIsMemoryBlock(const void* p, UINT nBytes,
	LONG* plRequestNumber = nullptr);

// Return TRUE if memory is sane or print out what is wrong
BOOL AFXAPI AfxCheckMemory();

#define afxMemDF _crtDbgFlag

enum AfxMemDF // memory debug/diagnostic flags
{
	allocMemDF = 0x01,         // turn on debugging allocator
	delayFreeMemDF = 0x02,         // delay freeing memory
	checkAlwaysMemDF = 0x04          // AfxCheckMemory on every alloc/free
};

#ifdef _UNICODE
#define AfxOutputDebugString(lpsz) \
	do \
	{ \
		USES_CONVERSION; \
		_RPT0(_CRT_WARN, W2CA(lpsz)); \
	} while (0)
#else
#define AfxOutputDebugString(lpsz) _RPT0(_CRT_WARN, lpsz)
#endif

// turn on/off tracking for a short while
BOOL AFXAPI AfxEnableMemoryTracking(BOOL bTrack);

// Advanced initialization: for overriding default diagnostics
BOOL AFXAPI AfxDiagnosticInit(void);

// A failure hook returns whether to permit allocation
typedef BOOL(AFXAPI* AFX_ALLOC_HOOK)(size_t nSize, BOOL bObject, LONG lRequestNumber);

// Set new hook, return old (never NULL)
AFX_ALLOC_HOOK AFXAPI AfxSetAllocHook(AFX_ALLOC_HOOK pfnAllocHook);

// Debugger hook on specified allocation request - Obsolete
void AFXAPI AfxSetAllocStop(LONG lRequestNumber);

// Memory state for snapshots/leak detection
struct CMemoryState
{
// Attributes
	enum blockUsage
	{
		freeBlock,    // memory not used
		objectBlock,  // contains a CObject derived class object
		bitBlock,     // contains ::operator new data
		crtBlock,
		ignoredBlock,
		nBlockUseMax  // total number of usages
	};

	_CrtMemState m_memState;
	LONG m_lCounts[nBlockUseMax];
	LONG m_lSizes[nBlockUseMax];
	LONG m_lHighWaterCount;
	LONG m_lTotalCount;

	CMemoryState();

// Operations
	void Checkpoint();  // fill with current state
	BOOL Difference(const CMemoryState& oldState,
					const CMemoryState& newState);  // fill with difference
	void UpdateData();

	// Output to afxDump
	void DumpStatistics() const;
	void DumpAllObjectsSince() const;
};

// Enumerate allocated objects or runtime classes
//void AFXAPI AfxDoForAllObjects(void (AFX_CDECL *pfn)(CObject* pObject, void* pContext),
//	void* pContext);
//void AFXAPI AfxDoForAllClasses(void (AFX_CDECL *pfn)(const CRuntimeClass* pClass,
//	void* pContext), void* pContext);

#else

// non-_DEBUG_ALLOC version that assume everything is OK
#define DEBUG_NEW new
#define AfxCheckMemory() TRUE
#define AfxIsMemoryBlock(p, nBytes) TRUE
#define AfxEnableMemoryTracking(bTrack) FALSE
#define AfxOutputDebugString(lpsz) ::OutputDebugString(lpsz)

// diagnostic initialization
#ifndef _DEBUG
#define AfxDiagnosticInit() TRUE
#else
BOOL AFXAPI AfxDiagnosticInit(void);
#endif

#endif // _DEBUG






#if _DEBUG && !defined TRACE
void AFX_CDECL AfxTrace(const char* lpszFormat, ...);
#define TRACE ::AfxTrace
#else
#define TRACE  1 ? (void)0 : printf
#endif



#include <list>





#endif // !defined(AFX_STDAFX_H__N3BASE__INCLUDED_)
