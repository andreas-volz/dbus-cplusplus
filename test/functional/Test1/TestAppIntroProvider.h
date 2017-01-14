#ifndef TEST_APP_INTRO_PROVIDER_H
#define TEST_APP_INTRO_PROVIDER_H

#include "TestAppIntroProviderPrivate.h"
#include "TestAppIntro.h"
#include "../../../tools/generator_utils.h"

#include <iostream>

using namespace std;

class TestAppIntroProvider :
  public DBusCpp::Test::Com::Intro_adaptor,
  public DBus::IntrospectableAdaptor,
  public DBus::PropertiesAdaptor,
  public DBus::ObjectAdaptor
{
public:
  TestAppIntroProvider(DBus::Connection &connection) :
    DBus::ObjectAdaptor(connection, "/DBusCpp/Test/Com/Intro")
  {}

  void test1()
  {
    cout << "Test1" << endl;
    this->test1Result();
  }

  void testByte(const uint8_t &Byte)
  {
    printf("TestByte: %d\n", Byte);
    this->testByteResult(Byte);
  }

  void testPropChanged(const std::string& val)
  {
    cout << "testPropChanged" << endl;
    testProperty = val;
    std::map<std::string, DBus::Variant> changed_props;
    changed_props["testProperty"] = testProperty.value();
    this->PropertiesChanged(this->DBusCpp::Test::Com::Intro_adaptor::name(),
                            changed_props,
                            std::vector<std::string>());
  }
};

#endif // TEST_COM_INTRO_PROVIDER_H

