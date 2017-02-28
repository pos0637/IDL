
#ifndef __IDLC_SYMBOL_H__
#define __IDLC_SYMBOL_H__

#include "AXP/cplusplus/xplatform/include/type.h"
#include "AXP/cplusplus/xplatform/include/astring.h"
#include "IDLCDefine.h"

namespace IDLC {
    class CSymbol : public AXP::CObject
    {
    public:

        AXP::Int32 mLevel;
        SymbolType mSymbolType;
        AXP::Sp<AXP::String> mContent;
    };
} // namespace IDLC

#endif // __IDLC_SYMBOL_H__
