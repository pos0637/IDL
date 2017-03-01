
#include "_TestA.h"
#include "IPC/gen/TestA/TestAObjectHolder.h"

using namespace AXP;
using namespace IPC;
using namespace TestA;

AXP::Sp<_A> _A::Create()
{
    return new _A();
}

AXP::Int32 _A::Foo(IN AXP::Int32 a, IN CONST AXP::Sp<NA::CList> & b)
{
    return 0;
}

AXP::Void _A::SetListener(IN CONST AXP::Sp<TestB::B> & bb)
{

}

