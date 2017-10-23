# InterceptVirtualMethod

InterceptVirtualMethod is a small framework which allows to intercept virtual methods.

It is used in Windows mostly for COM interfaces methods interception. 

Bellow is an example how it can be used.

```C++
#include <tchar.h>
#include <string>
#include <iostream>
#include "InterceptVirtualMethod.h"

using namespace std;

struct ITest {
  virtual int foo(const string& str1, string& str2) = 0;
};

struct Test: public ITest {
  int foo(const string& str1, string& str2) override {
    auto str = str1 + str2;
    cout << "Test::foo 'str1 + str2': " << str << endl;

    return (int)str.size();
  }
};

static VTABLE_FUNCTION_TYPE(&ITest::foo) s_fTest_foo;

static int fooOverride(ITest* pTest, const string& str1, string& str2) {
  cout << "fooOverride" << endl;

  // Calls Test::foo
  auto ret = s_fTest_foo(pTest, str1, str2);

  str2 = "QWERTY";

  return ret;
}

void main() {
  ITest* pTest = new Test;

  s_fTest_foo = OverrideVTableFunction(pTest, &ITest::foo, fooOverride);

  // 'pTest->foo' calls 'fooOverride'.
  string s = "QAZ";
  auto res = pTest->foo("ABC", s);

  cout << "s: " << s << " res: " << res << endl;

  delete pTest;
}

```
