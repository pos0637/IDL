
#ifndef __ITESTA_H__
#define __ITESTA_H__

#include "AXP/cplusplus/xplatform/include/type.h"
#include "AXP/cplusplus/xplatform/include/nullable.h"
#include "AXP/cplusplus/xplatform/include/astring.h"
#include "AXP/cplusplus/xplatform/include/list.h"
#include "IPC/gen/TestB/TestB.h"
#include "IPC/gen/NA/test.h"

namespace TestA
{
    class IA
    {
    public:

        VIRTUAL AXP::Int32 Foo(IN AXP::Int32 a, IN CONST AXP::Sp<NA::CList> & b) = 0;
        VIRTUAL AXP::Void SetListener(IN CONST AXP::Sp<TestB::B> & bb) = 0;
    };
}

#endif // __ITESTA_H__