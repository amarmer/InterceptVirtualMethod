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

// 'ITest' interface is known.
struct ITest {
  virtual int foo(const string& str1, string& str2) = 0;
};

// 'Test' source code is unavailable. Only available 'Test*'.
struct Test: public ITest {
  int foo(const string& str1, string& str2) override {
    cout << "Test::foo 'str1 + str2': " << str1 + str2 << endl;

    return (int)(str1 + str2).size();
  }
};

static VTABLE_FUNCTION_TYPE(&ITest::foo) s_fTest_foo;

// This function overrides 'foo' function in struct 'Test'.
static int fooIntercept(ITest* pTest, const string& str1, string& str2) {
  cout << "fooIntercept" << endl;

  // Calls Test::foo
  auto ret = s_fTest_foo(pTest, str1, str2);

  str2 = "QWERTY";

  return ret;
}

void main() {
  ITest* pTest = new Test;

  // 'SetVTableFunction' overrides 'foo' function in struct 'Test' with 'fooIntercept' function.
  // 's_fTest_foo' can be used to call 'Test::foo'.
  s_fTest_foo = SetVTableFunction(pTest, &ITest::foo, fooIntercept);

  // Calls fooIntercept
  string s = "QAZ";
  auto res = pTest->foo("ABC", s);

  cout << "s: " << s << " res: " << res << endl;

  delete pTest;
}

```
