#pragma once

#include "threads/SingleLock.h"
#include <list>
#include "utils/log.h"

template<class T>
class PCQueue
{
public:
	PCQueue(int nMaxNumElements)
	{
		m_nMaxNumElements = nMaxNumElements;
	}

	~PCQueue(void)
	{
		RemoveAll();
	}

	bool AppendNewElement(const T& element)
	{
		if (m_nMaxNumElements != -1 && GetNumElements() >= m_nMaxNumElements)
		{
			return false;
		}

		CSingleLock lock(m_cs);	
		m_lstElements.push_back(element);

		return true;
	}

	bool RemoveFirstElement(T& element, DWORD dwTimeout = 0)
	{
		memset(&element, 0, sizeof(T));

		CSingleLock lock(m_cs);

		if (!m_lstElements.empty())
		{
			element = m_lstElements.front();
			m_lstElements.pop_front();

			return true;
		}

		return false;
	}

	void RemoveAll()
	{
		CSingleLock lock(m_cs);
		if (!m_lstElements.empty())
		{
			m_lstElements.clear();
		}
	}

	BOOL IsEmpty() const
	{
		CSingleLock lock(m_cs);
		return m_lstElements.empty();
	}

	int GetNumElements() const
	{
		CSingleLock lock(m_cs);
		return m_lstElements.size();
	}

	int GetMaxElementsCount() const
	{
		return m_nMaxNumElements;
	}

private:
	typedef std::list<T> ElemList;
	ElemList m_lstElements;
	int m_nMaxNumElements;
	CCriticalSection m_cs;
};

template<class T>
class GrowQueue
{
public:
	GrowQueue(int nInitElemCnt, int nMaxAllocElemCnt, int nMaxReserveElemCnt)
		: m_nInitElemCnt(nInitElemCnt)
		, m_nMaxAllocNumElements(nMaxAllocElemCnt)
		, m_nMaxReserveElemCnt(nMaxReserveElemCnt)
		, m_nAllocatedElemCnt(0)
	{
    for (int i = 0; i < nInitElemCnt; i++)
    {
      T* pElem = (T*)malloc(sizeof(T));
      if (pElem)
      {
        AppendNewElement(pElem);
        m_nAllocatedElemCnt++;
      }
    }
	}

	~GrowQueue()
	{
		RemoveAll();
	}

	bool AppendNewElement(const T* element)
	{
		CSingleLock lock(m_cs);	
		if (m_nMaxReserveElemCnt != -1 && m_lstElements.size() >= m_nMaxReserveElemCnt)
		{
			free((void*)element);
			m_nAllocatedElemCnt--;
      CLog::Log(LOGDEBUG, "~~~~~~~~~~~~~~exceed max reserved: %d, free it, now allocated: %d", m_nMaxReserveElemCnt, m_nAllocatedElemCnt);
			return false;
		}
		else
		{			
			m_lstElements.push_back((T*)element);
      CLog::Log(LOGDEBUG, "~~~~~~~~~~~~recycle, now size: %d, now allocated: %d", m_lstElements.size(), m_nAllocatedElemCnt);
			return true;
		}
	}

	bool RemoveFirstElement(T*& element)
	{
		CSingleLock lock(m_cs);
		
		if (m_lstElements.empty() && m_nMaxAllocNumElements != -1 && m_nAllocatedElemCnt < m_nMaxAllocNumElements)
		{
      void* buffer = malloc(sizeof(T));
      if (buffer)
      {
        element = (T*)buffer;
        m_nAllocatedElemCnt++;
        CLog::Log(LOGDEBUG, "~~~~~~~~~~~~~~buffer empty, allocate new, now allocated: %d", m_nAllocatedElemCnt);
        return true;
      }
		}		
		else if (!m_lstElements.empty())
		{
			element = m_lstElements.front();
			m_lstElements.pop_front();
      CLog::Log(LOGDEBUG, "~~~~~~~~~~~~~~buffer isn't empty, remove first one, now size: %d, now allocated: %d", m_lstElements.size(), m_nAllocatedElemCnt);

			return true;
		}

    CLog::Log(LOGDEBUG, "~~~~~~~~~~~~~~buffer empty, and allocated to max number: %d, return false", m_nMaxAllocNumElements);
		return false;
	}

	void RemoveAll()
	{
		CSingleLock lock(m_cs);
		if (!m_lstElements.empty())
		{
			typename ElemPtrList::iterator iter = m_lstElements.begin();
			for (; iter != m_lstElements.end(); ++iter)
			{
				free(*iter);
			}
		}

		m_lstElements.clear();
		m_nAllocatedElemCnt = 0;
	}

	bool IsEmpty() const
	{
		CSingleLock lock(m_cs);
		return m_lstElements.empty();
	}

	int GetNumElements() const
	{
		CSingleLock lock(m_cs);
		return m_lstElements.size();
	}

	int GetMaxElementsCount() const
	{
		return m_nMaxAllocNumElements;
	}
private:
	typedef std::list<T*> ElemPtrList;
	ElemPtrList m_lstElements;
	const int m_nInitElemCnt;
	const int m_nMaxAllocNumElements;
	const int m_nMaxReserveElemCnt;
	int m_nAllocatedElemCnt;
	//	HANDLE m_hSemaphore;
	CCriticalSection m_cs;
};