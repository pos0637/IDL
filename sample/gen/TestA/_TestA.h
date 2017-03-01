
#ifndef ___TESTA_TESTA_H__
#define ___TESTA_TESTA_H__

#include "AXP/cplusplus/xplatform/include/type.h"
#include "AXP/cplusplus/xplatform/include/nullable.h"
#include "AXP/cplusplus/xplatform/include/astring.h"
#include "AXP/cplusplus/xplatform/include/list.h"
#include "AXP/cplusplus/xplatform/include/object.h"
#include "./ITestA.h"

namespace TestA
{
    class _A : public AXP::CObject, public IA
    {
    public:

        STATIC AXP::Sp<_A> Create();

    public:

        AXP::Int32 Foo(IN AXP::Int32 a, IN CONST AXP::Sp<NA::CList> & b);

        AXP::Void SetListener(IN CONST AXP::Sp<TestB::B> & bb);

    };
}

#endif // ___TESTA_TESTA_H__