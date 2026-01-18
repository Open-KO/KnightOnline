// SoundMgr.cpp: implementation of the CEventMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "SoundMgr.h"
#include "MapMng.h"
#include "SoundCell.h"
#include "LyTerrain.h"
#include "DlgSetSound.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSoundMgr::CSoundMgr()
{
	m_iVersion = 1;
	m_pSound.clear();
	m_pCurrSound = nullptr;

	m_MapSize    = 0;

	m_pRefMapMng = nullptr;
	m_bActive    = false;

	m_pDlgSound  = new CDlgSetSound;
	m_pDlgSound->Create(IDD_EDIT_BGM);
	m_pDlgSound->ShowWindow(FALSE);
	m_pDlgSound->m_pRefSoundMgr = this;
}

CSoundMgr::~CSoundMgr()
{
	std::list<CSoundCell*>::iterator it;
	for (it = m_pSound.begin(); it != m_pSound.end(); it++)
	{
		delete (*it);
		(*it) = nullptr;
	}
	m_pSound.clear();

	if (m_pCurrSound)
	{
		delete m_pCurrSound;
		m_pCurrSound = nullptr;
	}

	if (m_pDlgSound)
	{
		m_pDlgSound->DestroyWindow();
		delete m_pDlgSound;
		m_pDlgSound = nullptr;
	}

	m_MapSize    = 0;
	m_pRefMapMng = nullptr;
	m_bActive    = false;
}

void CSoundMgr::DelSound(CSoundCell* pSound)
{
	std::list<CSoundCell*>::iterator itSound;

	for (itSound = m_pSound.begin(); itSound != m_pSound.end(); itSound++)
	{
		if (pSound == (*itSound))
		{
			delete pSound;
			m_pSound.erase(itSound);
			return;
		}
	}
}

void CSoundMgr::SetCurrSound(CSoundCell* pSound)
{
	if (m_pCurrSound)
	{
		delete m_pCurrSound;
		m_pCurrSound = nullptr;
	}

	m_pCurrSound = pSound;

	std::list<CSoundCell*>::iterator itSound;

	for (itSound = m_pSound.begin(); itSound != m_pSound.end(); itSound++)
	{
		if (pSound == (*itSound))
		{
			m_pSound.erase(itSound);
			return;
		}
	}
}

void CSoundMgr::UpdateSound()
{
	m_pSound.push_back(m_pCurrSound);

	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
	CSoundCell* pSound      = new CSoundCell(pRefTerrain);
	m_pCurrSound            = pSound;
}

BOOL CSoundMgr::MouseMsgFilter(LPMSG pMsg)
{
	if (!m_pRefMapMng)
		return FALSE;
	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
	if (!m_bActive || !pRefTerrain)
		return FALSE;

	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN:
		{
			POINT point = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };

			__Vector3 vec;
			if (!pRefTerrain->Pick(point.x, point.y, &vec, nullptr))
				break;

			m_pCurrSound->InitRect(vec);
		}
			return TRUE;
		case WM_LBUTTONUP:
		{
			POINT point = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };

			__Vector3 vec;
			if (!pRefTerrain->Pick(point.x, point.y, &vec, nullptr))
				break;

			m_pCurrSound->AddRect(vec);
		}
			return TRUE;
		case WM_MOUSEMOVE:
		{
			DWORD_PTR nFlags = pMsg->wParam;
			POINT point      = { short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam)) };
			if (nFlags & MK_LBUTTON)
			{
				__Vector3 vec;
				if (!pRefTerrain->Pick(point.x, point.y, &vec, nullptr))
					break;
				m_pCurrSound->AddRect(vec);
			}
		}
			return TRUE;
	}
	return FALSE;
}

void CSoundMgr::SetActive(bool active)
{
	if (m_bActive == active)
		return;

	m_bActive = active;
	if (active)
	{
		m_pDlgSound->ShowWindow(TRUE);

		CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
		CSoundCell* pSound      = new CSoundCell(pRefTerrain);
		if (m_pCurrSound)
		{
			delete m_pCurrSound;
			m_pCurrSound = nullptr;
		}
		m_pCurrSound = pSound;
	}
	else
	{
		m_pDlgSound->ShowWindow(FALSE);

		if (m_pCurrSound)
		{
			delete m_pCurrSound;
			m_pCurrSound = nullptr;
		}
	}
}

void CSoundMgr::Render()
{
	HRESULT hr;

	__Matrix44 mtx;
	mtx.Identity();

	hr = s_lpD3DDev->SetTransform(D3DTS_WORLD, mtx.toD3D()); // 월드 행렬 적용..

	// set texture
	hr = s_lpD3DDev->SetTexture(0, nullptr);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	// backup state
	DWORD dwZEnable, dwLighting, dwCullMode;
	hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
	hr = s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCullMode);

	// set state
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	hr = s_lpD3DDev->SetFVF(FVF_XYZCOLOR);

	//이미 만들어진 길 그리기...
	std::list<CSoundCell*>::iterator itSound;

	CSoundCell* pSound;
	for (itSound = m_pSound.begin(); itSound != m_pSound.end(); itSound++)
	{
		pSound = (*itSound);
		if (!pSound)
			continue;

		pSound->Render(0xff0000ff);
	}

	//대화상자에서 선택된 길 그리기.
	if (m_pDlgSound->m_pSelSound)
		m_pDlgSound->m_pSelSound->Render(0xff00ff00);

	//만들고 있는 길 & 영역 그리기..
	m_pCurrSound->Render(0xffff0000);

	// restore
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCullMode);
}

bool CSoundMgr::Load(File& file)
{
	//dlg 클리어..
	m_pDlgSound->Clear();

	file.Read(&m_iVersion, sizeof(int));
	if (!m_pDlgSound->LoadSoundGroup(file))
		return false;

	// m_pSound클리어...
	for (CSoundCell* pSoundCell : m_pSound)
		delete pSoundCell;
	m_pSound.clear();

	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();

	int cnt                 = 0;
	file.Read(&cnt, sizeof(int));
	for (int i = 0; i < cnt; i++)
	{
		CSoundCell* pSoundCell = new CSoundCell(pRefTerrain);
		pSoundCell->Load(file);

		m_pSound.push_back(pSoundCell);
		//dlg에 추가...
		m_pDlgSound->AddSoundInfo(pSoundCell);
	}
	m_pRefMapMng->Invalidate();
	return true;
}

bool CSoundMgr::Save(File& file)
{
	file.Write(&m_iVersion, sizeof(int));

	if (!m_pDlgSound->SaveSoundGroup(file))
		return false;

	int cnt = static_cast<int>(m_pSound.size());
	file.Write(&cnt, sizeof(int));

	for (CSoundCell* pSoundCell : m_pSound)
		pSoundCell->Save(file);

	return true;
}

void CSoundMgr::SaveGameData(File& file)
{
	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
	m_MapSize               = pRefTerrain->m_iHeightMapSize;

	char* pSound            = (char*) GlobalAlloc(GMEM_FIXED, sizeof(char) * m_MapSize * m_MapSize);
	memset(pSound, -1, sizeof(char) * m_MapSize * m_MapSize);

	//sound cell들을 면적순으로(큰게 앞으로 오게..)정렬하고...
	//면적순으로 정리하면서 아이디정렬도 하고...
	//
	//
	SCSort();

	std::list<int> tmpList;
	for (CSoundCell* pSoundCell : m_pSound)
		tmpList.push_back(pSoundCell->m_dwSoundGroupID);

	tmpList.sort();
	tmpList.unique();

	std::map<int, int> tmpMap;
	std::list<int>::iterator it_int = tmpList.begin();
	int cnt                         = static_cast<int>(tmpList.size());

	file.Write(&cnt, sizeof(int));
	for (int i = 0; i < cnt; i++)
	{
		int dwID = (*it_int);
		tmpMap.insert(std::make_pair(dwID, i));
		it_int++;

		LPSOUNDINFO pSI = m_pDlgSound->GetSoundGroup(dwID);
		if (pSI == nullptr)
		{
			AfxMessageBox("Sound Group이 유효하지 않습니다.ㅠ.ㅠ");
			return;
		}

		// sound group을 어케 저장한담?
		for (int j = 0; j < 4; j++)
		{
			int str_size = 0;
			std::string str;

			str      = pSI->szBGM[j];
			str_size = static_cast<int>(str.size());
			file.Write(&str_size, sizeof(int));
			file.Write(str.c_str(), str_size);
			file.Write(&(pSI->fBGMRegenTime[j]), sizeof(float));

			str      = pSI->szBGE[j];
			str_size = static_cast<int>(str.size());
			file.Write(&str_size, sizeof(int));
			file.Write(str.c_str(), str_size);
			file.Write(&(pSI->fBGERegenTime[j]), sizeof(float));
		}
	}

	// 타일에 Sound Info 셋팅하고 저장...
	for (CSoundCell* pSoundCell : m_pSound)
	{
		for (int x = pSoundCell->m_Rect.left; x <= pSoundCell->m_Rect.right; x++)
		{
			for (int z = pSoundCell->m_Rect.top; z <= pSoundCell->m_Rect.bottom; z++)
				pSound[x + z * (m_MapSize)] = (char) pSoundCell->m_dwSoundGroupID;
		}
	}

	file.Write(pSound, sizeof(char) * m_MapSize * m_MapSize);

	GlobalFree(pSound);
	pSound = nullptr;
}

//
///////////////////////////////////////////////////////////////////////////////////////////////
// related sort list...
// list의 sort함수 베꼈당..-.-
// 제대로 동작 안하더라..ㅠ.ㅠ
//

void CSoundMgr::SCSort()
{
	if (2 <= m_pSound.size())
	{
		const size_t _MAXN = 15;
		std::list<CSoundCell*> _X, _A[_MAXN + 1];
		size_t _N = 0;
		while (!m_pSound.empty())
		{
			_X.splice(_X.begin(), m_pSound, m_pSound.begin());
			size_t _I;
			for (_I = 0; _I < _N && !_A[_I].empty(); ++_I)
			{
				SCMerge(_A[_I], _X); //_A[_I].merge(_X, _Pr);
				_A[_I].swap(_X);
			}
			if (_I == _MAXN)
				SCMerge(_A[_I], _X); // _A[_I].merge(_X, _Pr);
			else
			{
				_A[_I].swap(_X);
				if (_I == _N)
					++_N;
			}
		}
		while (0 < _N)
			SCMerge(m_pSound, _A[--_N]); //m_pVBList_Alive.merge(_A[--_N], _Pr);
	}
}

void CSoundMgr::SCMerge(std::list<CSoundCell*>& l1, std::list<CSoundCell*>& l2)
{
	if (&l1 != &l2)
	{
		std::list<CSoundCell*>::iterator _F1 = l1.begin(), _L1 = l1.end();
		std::list<CSoundCell*>::iterator _F2 = l2.begin(), _L2 = l2.end();
		while (_F1 != _L1 && _F2 != _L2)
			if (SCComp(*_F2, *_F1))
			{
				std::list<CSoundCell*>::iterator _Mid2 = _F2;
				l1.splice(_F1, l2, _F2, ++_Mid2);
				_F2 = _Mid2;
			}
			else
				++_F1;
		if (_F2 != _L2)
			l1.splice(_L1, l2, _F2, _L2);
	}
}

bool CSoundMgr::SCComp(CSoundCell* pP1, CSoundCell* pP2)
{
	CRect rt1, rt2;
	rt1 = pP1->m_Rect;
	rt2 = pP2->m_Rect;

	int dim1, dim2;
	dim1 = (rt1.Height() + 1) * (rt1.Width() + 1);
	dim2 = (rt2.Height() + 1) * (rt2.Width() + 1);

	if (dim1 < dim2)
		return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
