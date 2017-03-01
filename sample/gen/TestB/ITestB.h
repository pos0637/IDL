
#ifndef __ITESTB_H__
#define __ITESTB_H__

#include "AXP/cplusplus/xplatform/include/type.h"

namespace TestB
{
    class IB
    {
    public:

        VIRTUAL AXP::Int32 Bar(IN AXP::Int32 a) = 0;
    };
}

#endif // __ITESTB_H__