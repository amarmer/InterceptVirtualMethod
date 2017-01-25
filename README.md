# InterceptVirtualMethod

InterceptVirtualMethod is a small framework which allows to intercept virtual methods.

It is used in Windows mostly for COM interfaces methods interception. 

Bellow is example how it can be used.

```C++
#include <tchar.h>
#include <string>
#include "InterceptVirtualMethod.h"

using namespace std;

struct ITest
{
    virtual int STDMETHODCALLTYPE foo(const string& str) = 0;
};

struct Test: public ITest
{
    int STDMETHODCALLTYPE foo(const string& str) override
    {
        printf("Test::foo str: %s%\n", str.c_str());

        return str.size();
    }
};


static VTABLE_FUNCTION_TYPE(ITest, foo) s_fTest_foo;

static int STDMETHODCALLTYPE fooIntercept(ITest* pTest, const string& str)
{
    printf("fooIntercept\n");

    // Calls Test::foo
    return s_fTest_foo(pTest, str);
}

int _tmain(int argc, _TCHAR* argv[])
{
    ITest* pTest = new Test;

    // s_fQAZ_Test == pointer to QAZ::Test 
    s_fTest_foo = SetVTableFunction(pTest, &ITest::foo, fooIntercept);

    // Calls fooIntercept
    auto res = pTest->foo("ABC");

    printf("res %d\n", res);

    delete pTest;

	return 0;
}
```
