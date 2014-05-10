#pragma once

#include <string>
#include "utils/TimeUtils.h"
#include "utils/log.h"

#define BEGIN_THIS_FUNCTION() \
  CLog::Log(LOGINFO, "++++++++++++++%s begin", __FUNCTION__)

#define END_THIS_FUNCTION() \
  CLog::Log(LOGINFO, "++++++++++++++%s end", , __FUNCTION__)

#define BEGIN_FUNCTION(_func) \
  CLog::Log(LOGINFO, "++++++++++++++%s begin", _func)

#define END_FUNCTION(_func) \
  CLog::Log(LOGINFO, "++++++++++++++%s end", _func)

#define BEGIN_MEMBER_FUNCTION(_class, _func) \
  CLog::Log(LOGINFO, "++++++++++++++%s::%s begin", _class, _func)

#define END_MEMBER_FUNCTION(_class, _func) \
  CLog::Log(LOGINFO, "++++++++++++++%s::%s end", _class, _func)

class TestBase
{
public:
  TestBase(const CStdString& strClass, const CStdString& strFunc)
    : m_strClass(strClass)
    , m_strFunc(strFunc)
  {
    if (!m_strClass.IsEmpty())
    {
      BEGIN_MEMBER_FUNCTION(m_strClass.c_str(), m_strFunc.c_str());
    }
    else if (!m_strFunc.IsEmpty())
    {
      BEGIN_FUNCTION(m_strFunc.c_str());
    }
  }

  ~TestBase()
  {
    if (!m_strClass.IsEmpty())
    {
      END_MEMBER_FUNCTION(m_strClass.c_str(), m_strFunc.c_str());
    }
    else if (!m_strFunc.IsEmpty())
    {
      END_FUNCTION(m_strFunc.c_str());
    }
  }

protected:
  CStdString m_strClass;
  CStdString m_strFunc;
};

class NormalTest : public TestBase
{
public:
  NormalTest(const CStdString& strClass, const CStdString& strFunc) : TestBase(strClass, strFunc)
  {

  }

  ~NormalTest()
  {

  }
};

class ElapseTest : public NormalTest
{
public:
  ElapseTest(const CStdString& strClass, const CStdString& strFunc) : NormalTest(strClass, strFunc)
  {
    m_start = CurrentHostCounter();
  }

  ~ElapseTest()
  {
    int64_t end, freq;
    end = CurrentHostCounter();
    freq = CurrentHostFrequency();
    CLog::Log(LOGINFO,"*************Elapse: %.2fms", 1000.f * (end - m_start) / freq);
  }

protected:
  int64_t m_start;
};

#if defined __VDM_OPEN_TEST__
#define BEGIN_NORMAL_TEST(_func) \
{ \
  NormalTest("", #_func);

#define END_NORMAL_TEST_() \
}

#define BEGIN_ELAPSE_TEST(_func) \
{ \
  ElapseTest("", #_func);

#define END_ELAPSE_TEST_() \
}
#else
#define BEGIN_NORMAL_TEST(_func)
#define END_NORMAL_TEST_()
#define BEGIN_ELAPSE_TEST(_func)
#define END_ELAPSE_TEST_()
#endif