#pragma once

#include <map>

template <
	class ValueType,
	class KeyType = int>
class CSTLMap
{
public:
	using MapType = std::map<KeyType, ValueType*>;
	MapType m_UserTypeMap;

	int GetSize() const {
		return (int) m_UserTypeMap.size(); 
	}

	bool IsExist(
		const KeyType& key)
		const
	{
		return m_UserTypeMap.find(key) != m_UserTypeMap.end(); 
	}

	bool IsEmpty() const { 
		return m_UserTypeMap.empty(); 
	}

	bool PutData(
		const KeyType& key_value,
		ValueType* pData) 
	{
		return m_UserTypeMap.insert(
			std::make_pair(key_value, pData)).second; 
	}

	ValueType* GetData(
		const KeyType& key_value)
	{
		auto itr = m_UserTypeMap.find(key_value);
		return (itr != m_UserTypeMap.end() ? itr->second : nullptr);
	}

	const ValueType* GetData(
		const KeyType& key_value)
		const
	{
		auto itr = m_UserTypeMap.find(key_value);
		return (itr != m_UserTypeMap.end() ? itr->second : nullptr);
	}

	void DeleteData(
		const KeyType& key_value)
	{
		auto itr = m_UserTypeMap.find(key_value);
		if (itr!= m_UserTypeMap.end())
		{
			delete itr->second;
			m_UserTypeMap.erase(itr);
		}
	}

	void DeleteAllData()
	{
		if (m_UserTypeMap.empty())
			return;

		for (auto itr = m_UserTypeMap.begin(); itr != m_UserTypeMap.end(); ++itr)
			delete itr->second;

		m_UserTypeMap.clear();
	}

	~CSTLMap()
	{
		DeleteAllData();
	}

	typename MapType::const_iterator begin() const {
		return m_UserTypeMap.begin();
	}

	typename MapType::const_iterator end() const {
		return m_UserTypeMap.end();
	}

	typename MapType::iterator begin() {
		return m_UserTypeMap.begin();
	}

	typename MapType::iterator end() {
		return m_UserTypeMap.end();
	}

	typename MapType::const_reverse_iterator rbegin() const {
		return m_UserTypeMap.rbegin();
	}

	typename MapType::const_reverse_iterator rend() const {
		return m_UserTypeMap.rend();
	}

	typename MapType::reverse_iterator rbegin() {
		return m_UserTypeMap.rbegin();
	}

	typename MapType::reverse_iterator rend() {
		return m_UserTypeMap.rend();
	}
};
