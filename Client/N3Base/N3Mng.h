// N3Mng.h: interface for the CN3Mng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Mng_h__INCLUDED_)
#define AFX_N3Mng_h__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _N3GAME
#include "LogWriter.h"
#endif

#include <string>
#include <map>

template <class T> class CN3Mng
{
protected:
	
	typedef typename std::map<std::string, T*>::iterator		it_Data;
	typedef typename std::map<std::string, T*>::value_type	val_Data;

	typedef typename std::map<T*, int>::iterator				it_Ref;
	typedef typename std::map<T*, int>::value_type			val_Ref;

	typename std::map<std::string, T*>	m_Datas;
	typename std::map<T*, int>			m_Refs;

public:
	int		Count() { return m_Datas.size(); }

	int Add(T* pData)
	{
		if(NULL == pData)
		{ 
#ifdef _N3GAME
			CLogWriter::Write("CN3Mng::Add - Null object pointer");
#endif
			return -1;
		}
		if(pData->FileName().empty())
		{
#ifdef _N3GAME
			CLogWriter::Write("CN3Mng::Add - Null object file name");
#endif
			return -1;
		}

		std::pair<it_Ref, bool> pairRef = m_Refs.insert(val_Ref(pData, 1));
		if(false == pairRef.second)
		{
#ifdef _N3GAME
			CLogWriter::Write("CN3Mng::Add - Duplicated object pointer");
#endif
			return -1;
		}

		std::pair<it_Data, bool> pairData = m_Datas.insert(val_Data(pData->FileName(), pData));
		if(false == pairData.second)
		{
#ifdef _N3GAME
			CLogWriter::Write("CN3Mng::Add - duplicated object's file name.");
#endif
			m_Refs.erase(pairRef.first); // 참조 카운트 지우고..
			return -1;
		}

		return m_Datas.size() - 1;
	}

	T* 	Get(int index, bool bIncreaseRefCount = TRUE)
	{
		if(index < 0 || index >= m_Datas.size()) return NULL;

		if(bIncreaseRefCount)
		{
			it_Ref it = m_Refs.begin();
			for(int i = 0; i < index; i++, it++);
			it->second++;
		}

		it_Data it = m_Datas.begin();
		for(int i = 0; i < index; i++, it++);

		return it->second;
	}

	T* 	Get(const std::string& szFN, bool bIncreaseRefCount = TRUE, int iLOD = 0)
	{
		if(szFN.empty()) return NULL;
		std::string szFN2 = szFN;
		CharLower(&(szFN2[0]));

		T* pData = NULL;
		it_Data it = m_Datas.find(szFN2);
		if(it == m_Datas.end()) // 못 찾았다..
		{
			pData = new T();
			pData->m_iLOD = iLOD; // 로딩시 LOD 적용

			if(false == pData->LoadFromFile(szFN2)) // 파일 읽기에 실패했다!!
			{
				delete pData;
				pData = NULL;
			}
			else 
			{
				int reChk = Add(pData); // 읽기 성공하면 추가..
				//	asm
				if(reChk==-1)	//	추가시 전에 데이터가 있어 참조 카운트를 하나 더한다 
				{
					T* pBakData = pData;	//	같은 파일중 전 데이타를 받아 리턴(새로운 그림으로 바뀌지 않을수 있다)
					it_Data it = m_Datas.find(pBakData->FileName());
					pData = (*it).second;

					if(bIncreaseRefCount)
					{						
						it_Ref it2 = m_Refs.find(pData);
						if(it2 != m_Refs.end()) // 참조 카운트 찾기..
						{
							((*it2).second)++;
						}
					}

					delete pBakData;	//	이번에 읽은 데이타는 필요가 없으므로 지움
					pBakData = NULL;
				}
				//	asm
			}
		}
		else //  찾았다..!!
		{
			pData = (*it).second;

			if(bIncreaseRefCount)
			{
				it_Ref it2 = m_Refs.find(pData);
				if(it2 != m_Refs.end()) // 참조 카운트 찾기..
				{
					((*it2).second)++;
				}
			}
		}

		return pData;
	}

	bool IsExist(const std::string& szFN)
	{
		if(szFN.empty()) return false;

		if(m_Datas.find(szFN) != m_Datas.end()) return true;
		else return false;
	} // 있나 없나~

	void Delete(T** ppData, bool bReleaseOrg = true)
	{
#ifndef _N3GAME
		bReleaseOrg = true; // 툴에서는 무조건 
#endif
		if(NULL == ppData || NULL == *ppData) return;

		it_Data it = m_Datas.find((*ppData)->FileName());
		if(it == m_Datas.end()) return; // 못 찾았다..
		else //  찾았다..!!
		{
			it_Ref it2 = m_Refs.find(*ppData);
			if(bReleaseOrg && it2 != m_Refs.end()) // 참조 카운트 찾기..
			{
				((*it2).second)--;
				if(0 == (*it2).second)
				{
					delete (*ppData);
					m_Datas.erase(it);
					m_Refs.erase(it2);
				}
			}
			*ppData = NULL;
		}
	}

#ifdef _N3TOOL
	void SaveToFiles()
	{
		it_Data it = m_Datas.begin(), itEnd = m_Datas.end();
		for(; it != itEnd; it++) (*it).second->SaveToFile();
	}
#endif // end of _N3TOOL
	
	void Release()
	{
		it_Data it = m_Datas.begin(), itEnd = m_Datas.end();
		for(; it != itEnd; it++) delete ((*it).second);

		m_Datas.clear();
		m_Refs.clear();
	}

	CN3Mng()
	{
	}
	
	~CN3Mng()
	{
		Release();
	}

};

#endif // !defined(AFX_N3Mng_h__INCLUDED_)
