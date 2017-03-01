
#include "_TestB.h"
#include "IPC/gen/TestB/TestBObjectHolder.h"

using namespace AXP;
using namespace IPC;
using namespace TestB;

AXP::Sp<_B> _B::Create()
{
    return new _B();
}

AXP::Int32 _B::Bar(IN AXP::Int32 a)
{
    return 0;
}

