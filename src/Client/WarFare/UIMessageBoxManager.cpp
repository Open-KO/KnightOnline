// UIMessageBoxManager.cpp: implementation of the CUIMessageBoxManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIMessageBoxManager.h"
#include "GameProcedure.h"
#include "UIManager.h"

CUIMessageBoxManager::CUIMessageBoxManager()
{
	m_pMsgBoxLatestRef = nullptr;
}

CUIMessageBoxManager::~CUIMessageBoxManager()
{
	CUIMessageBoxManager::Release();
}

int CUIMessageBoxManager::GetCount() const
{
	return static_cast<int>(m_UBMs.size());
}

std::string CUIMessageBoxManager::MessageBoxPost(const std::string& szMsg, const std::string& szTitle, int iStyle, e_Behavior eBehavior)
{
	CUIMessageBox* pMB       = nullptr;
	__TABLE_UI_RESRC* pTblUI = nullptr;

	it_UBM it, it_e;

	it   = m_UBMs.find(szMsg);
	it_e = m_UBMs.end();

	if (it == it_e)
	{
		pMB = new CUIMessageBox();
		if (pMB == nullptr)
			return szMsg;

		pTblUI = CGameBase::s_pTbl_UI.Find(NATION_ELMORAD);

		pMB->LoadFromFile(pTblUI->szMessageBox);
		pMB->SetVisible(false);
		m_UBMs.insert(val_UBM(szMsg, pMB));
	}
	else
	{
		pMB = it->second;
	}

	RECT rt = pMB->GetRegion();
	POINT pt;
	pt.x = (CN3Base::s_CameraData.vp.Width - (rt.right - rt.left)) / 2;
	pt.y = (CN3Base::s_CameraData.vp.Height - (rt.bottom - rt.top)) / 2;

	pMB->SetBoxStyle(iStyle);
	if (BEHAVIOR_DELETE_CHR == eBehavior)
		pMB->SetVisibleEditControl(true); // Edit Control Visible
	else
		pMB->SetVisibleEditControl(false);

	pMB->m_eBehavior = eBehavior;
	pMB->SetText(szMsg);
	pMB->SetTitle(szTitle);

	pMB->SetPos(pt.x, pt.y);
	pMB->SetVisible(true);

	m_pMsgBoxLatestRef = pMB; // 마지막에 띄운 대화상자.. Z 정렬을 위해서 이 포인터를 세팅해준다..

	it                 = m_UBMs.begin();
	it_e               = m_UBMs.end();
	while (it != it_e)
	{
		pMB = it->second;
		if (pMB && !pMB->IsVisible())
		{
			delete pMB;
			it = m_UBMs.erase(it);
			continue;
		}
		it++;
	}

	return szMsg;
}

void CUIMessageBoxManager::MessageBoxClose(const std::string& szMsg)
{
	it_UBM it_f, it_e;
	it_f = m_UBMs.find(szMsg);
	it_e = m_UBMs.end();

	if (it_f != it_e)
	{
		if (m_pMsgBoxLatestRef == it_f->second)
			m_pMsgBoxLatestRef = nullptr;

		CUIMessageBox* pMB = it_f->second;
		if (pMB)
			pMB->SetVisible(false);
	}
}

void CUIMessageBoxManager::Render()
{
	it_UBM it = m_UBMs.begin(), it_e = m_UBMs.end();
	for (; it != it_e; it++)
	{
		CUIMessageBox* pMB = it->second;
		if (pMB == nullptr || pMB == m_pMsgBoxLatestRef)
			continue;
		if (!pMB->IsVisible())
			continue;
		CUIManager::RenderStateSet();
		pMB->Render(); // 메시지 박스 렌더링..
		CUIManager::RenderStateRestore();
	}

	if (m_pMsgBoxLatestRef && m_pMsgBoxLatestRef->IsVisible())
	{
		CUIManager::RenderStateSet();
		m_pMsgBoxLatestRef->Render(); // 메시지 박스 렌더링..
		CUIManager::RenderStateRestore();
	}
}

void CUIMessageBoxManager::MessageBoxCloseAll()
{
	it_UBM it = m_UBMs.begin(), it_e = m_UBMs.end();
	for (; it != it_e; it++)
	{
		CUIMessageBox* pMB = it->second;
		if (pMB)
			pMB->SetVisible(false);
	}

	m_pMsgBoxLatestRef = nullptr;
}

uint32_t CUIMessageBoxManager::MouseProcAndTick(uint32_t& dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = 0;

	if (m_pMsgBoxLatestRef != nullptr && m_pMsgBoxLatestRef->IsVisible())
	{
		dwRet = m_pMsgBoxLatestRef->MouseProc(dwFlags, ptCur, ptOld);
		if (dwRet != UI_MOUSEPROC_NONE)
			dwFlags = 0;

		// NOTE: This seems redundant, but MouseProc() can cause it to be reset.
		if (m_pMsgBoxLatestRef != nullptr)
			m_pMsgBoxLatestRef->Tick();
	}

	for (auto& [_, pMB] : m_UBMs)
	{
		if (pMB == nullptr)
			continue;

		if (!pMB->IsVisible())
			continue;

		if (m_pMsgBoxLatestRef == pMB)
			continue;

		dwRet = pMB->MouseProc(dwFlags, ptCur, ptOld);
		if (dwRet != UI_MOUSEPROC_NONE)
			dwFlags = 0;
		pMB->Tick();
	}

	return dwRet;
}

void CUIMessageBoxManager::Release()
{
	it_UBM it = m_UBMs.begin(), it_e = m_UBMs.end();
	for (; it != it_e; it++)
		delete it->second;
	m_UBMs.clear();

	m_pMsgBoxLatestRef = nullptr;
}

CUIMessageBox* CUIMessageBoxManager::ReFocusMsgBox()
{
	for (rit_UBM itor = m_UBMs.rbegin(); m_UBMs.rend() != itor; ++itor)
	{
		CUIMessageBox* pMsgBox = itor->second;
		if (pMsgBox && pMsgBox->IsVisible())
		{
			m_pMsgBoxLatestRef = pMsgBox;
			return m_pMsgBoxLatestRef;
		}
	}

	m_pMsgBoxLatestRef = nullptr;
	return nullptr;
}
