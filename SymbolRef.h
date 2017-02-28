
#ifndef __SYMBOL_REF_H__
#define __SYMBOL_REF_H__

#include "AXP/cplusplus/xplatform/include/astring.h"
#include "AXP/cplusplus/xplatform/include/stl/HashTable.h"
#include "IDLCDefine.h"

namespace IDLC
{
    class CSymbolRef : public AXP::CObject
    {
    public:

        STATIC AXP::Sp<CSymbolRef> Create(
            IN AXP::PCStr fileName,
            IN SymbolType symbolType,
            IN AXP::Int32 line);

    protected:

        AXP::ARESULT Initialize(
            IN AXP::PCStr fileName,
            IN SymbolType symbolType,
            IN AXP::Int32 line);

    public:

        AXP::Sp<AXP::String> mFileName;
        SymbolType mSymbolType;
        AXP::Int32 mLine;
        AXP::Sp<AXP::HashTable<AXP::PStr, CSymbolRef> > mSymbolTable;
    };

    AXP::ARESULT AddRef(
        IN AXP::PCStr namespacz,
        IN AXP::PCStr refName,
        IN AXP::PCStr fileName,
        IN SymbolType symbolType,
        IN AXP::Int32 line);
}

#endif // __SYMBOL_REF_H__
