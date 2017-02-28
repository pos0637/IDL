
#ifndef __IDLC_PARSER_H__
#define __IDLC_PARSER_H__

#include "IDLCDefine.h"
#include "AXP/cplusplus/xplatform/include/aresult.h"
#include "AXP/cplusplus/xplatform/include/stl/hashTable.h"

namespace IDLC {
    int Initialize();

    int AddItem(
        SymbolType symbolType,
        const char * content);

    AXP::ARESULT RemovePrevItem();

    class MapTable
    {
    public:

        STATIC AXP::Boolean SetParcelFunctionName();
        STATIC AXP::Sp<AXP::HashTable<AXP::Int32, AXP::String> > GetWriteToParcelNameOfBasicType();
        STATIC AXP::Sp<AXP::HashTable<AXP::Int32, AXP::String> > GetReadFromParcelNameOfBasicType();

    private:

        STATIC AXP::Sp<AXP::HashTable<AXP::Int32, AXP::String> > sWriteToParcelOfBasicType;
        STATIC AXP::Sp<AXP::HashTable<AXP::Int32, AXP::String> > sReadFromParcelOfBasicType;
    };
} // namespace IDLC

#endif // __IDLC_PARSER_H__
