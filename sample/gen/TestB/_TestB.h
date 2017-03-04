
#ifndef ___TESTB_TESTB_H__
#define ___TESTB_TESTB_H__

#include "AXP/cplusplus/xplatform/include/type.h"
#include "AXP/cplusplus/xplatform/include/object.h"
#include "./ITestB.h"

namespace TestB
{
    class _B : public AXP::CObject, public IB
    {
    public:

        STATIC AXP::Sp<_B> Create();

    public:

        AXP::Int32 Bar(IN AXP::Int32 a);

    };
}

#endif // ___TESTB_TESTB_H__