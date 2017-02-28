
#include "SymbolRef.h"

using namespace AXP;

namespace IDLC
{
    Sp<HashTable<PStr, CSymbolRef> > gSymbolTable = new HashTable<PStr, CSymbolRef>(100);

    Sp<CSymbolRef> CSymbolRef::Create(
        IN PCStr fileName,
        IN SymbolType symbolType,
        IN Int32 line)
    {
        Sp<CSymbolRef> obj = new CSymbolRef();
        if (obj == NULL)
            return NULL;

        if (AFAILED(obj->Initialize(fileName, symbolType, line)))
            return NULL;

        return obj;
    }

    ARESULT CSymbolRef::Initialize(
        IN PCStr fileName,
        IN SymbolType symbolType,
        IN Int32 line)
    {
        mFileName = String::Create(fileName);
        if (mFileName == NULL)
            return AE_FAIL;

        mSymbolType = symbolType;
        mLine = line;

        return AS_OK;
    }

    ARESULT AddRef(
        IN PCStr namespacz,
        IN PCStr refName,
        IN PCStr fileName,
        IN SymbolType symbolType,
        IN Int32 line)
    {
        Sp<CSymbolRef> obj = CSymbolRef::Create(fileName, symbolType, line);
        if (obj == NULL)
            return AE_FAIL;

        PStr key = (PStr)malloc(strlen(namespacz) + strlen(refName) + 2);
        if (!key)
            return AE_FAIL;

        sprintf(key, "%s.%s", namespacz, refName);
        if (!gSymbolTable->InsertUnique(key, obj)) {
            free(key);
            return AE_FAIL;
        }

        free(key);

        return AS_OK;
    }
}