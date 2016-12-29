#ifndef TEST_APP_INTRO_H
#define TEST_APP_INTRO_H

#include "TestAppIntroPrivate.h"
#include "../../../tools/generator_utils.h"

#include <iostream>
#include <cstdio>

using namespace std;

class TestAppIntro :
  public DBusCpp::Test::Com::Intro_proxy,
  public DBus::IntrospectableProxy,
  public DBus::PropertiesProxy,
  public DBus::ObjectProxy
{
public:
  TestAppIntro(DBus::Connection &connection, pthread_cond_t &condition, bool &testResult) :
    DBus::ObjectProxy(connection, "/DBusCpp/Test/Com/Intro", "DBusCpp.Test.Com.Intro"),
    mCondition(condition),
    mTestResult(testResult)
  {}

  void test1Result()
  {
    cout << "Test1Result" << endl;
    mTestResult = true;
    pthread_cond_signal(&mCondition);
  }

  void testByteResult(const uint8_t &Byte)
  {
    printf("TestByteResult: %d\n", Byte);
    mTestResult = true;
    pthread_cond_signal(&mCondition);
  }

  void PropertiesChanged(const std::string& interface,
                         const std::map<std::string, DBus::Variant>& changed_properties,
                         const std::vector<std::string>& invalidated_properties)
  {
    cout << "PropertiesChanged on interface " << interface;
    std::map<std::string, DBus::Variant>::const_iterator it = changed_properties.find("testProperty");
    if (changed_properties.end() != it &&
        changed_properties.size() == 1 &&
        invalidated_properties.empty())
    {
      cout << " with value " << it->second.operator std::string();
      mTestResult = true;
    }
    cout << endl;
    pthread_cond_signal(&mCondition);
  }

private:
  pthread_cond_t &mCondition;
  bool &mTestResult;
};

#endif // TEST_COM_INTRO_H
