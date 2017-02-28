
#include <stdlib.h>
#include "AXP/cplusplus/xplatform/include/stl/tree.h"
#include "AXP/cplusplus/xplatform/include/stl/hashtable.h"
#include "IDLCParser.h"
#include "Common.h"

using namespace AXP;
using namespace AXP::STL;
using namespace IDLC;

namespace IDLC
{
    EXTERN Int32 gIDLFlag;
    EXTERN Sp<List<String> > gHeadListCplusplus;
    EXTERN Sp<String> gRootOutputDir;
    EXTERN Sp<String> GetOrignalFileName();
    EXTERN CMappingArea * gShareAreaCpp;
    EXTERN Sp<HashTable<PCWStr, CMappingInfo> > gMappingCpp;
    EXTERN Sp<String> gNamespacz;
    STATIC pthread_key_t sKey;

    STATIC Void WriteClass(IN TreeNode<CSymbol> * node);

    STATIC Void WriteFunCodeMembers(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("\r\nprivate:\r\n\r\n");

        Int32 count = 0;
        Boolean hasConstruction = FALSE;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Construction)
                hasConstruction = TRUE;

            if ((obj->GetValue()->mSymbolType == SymbolType_Construction)
                || (obj->GetValue()->mSymbolType == SymbolType_Function)) {
                Sp<String> funCode = GetConsOrFuncId(obj);
                if (funCode == NULL)
                    return;

                WRITE_STRING_TO_FILE(funCode->Length() + 77, L"STATIC CONST AXP::Int32 %ls = %d;\r\n", (PCWStr)*funCode, count++);
            }
        }

        if (!hasConstruction) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 77, L"STATIC CONST AXP::Int32 %ls = %d;\r\n", (PCWStr)*defConsId, count);
        }
    }

    STATIC Void WriteVarToParcel(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return;

        Sp<String> varId;
        Sp<String> failedMsg;
        if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter)) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varId = node->GetChildren()->Get(1)->GetValue()->mContent;
            if (node->GetValue()->mSymbolType == SymbolType_Member)
                failedMsg = String::Create(L"return AXP::AE_FAIL;\r\n");
            else
                failedMsg = String::Create(L"throw std::exception();\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
            failedMsg = String::Create(L"return NULL;\r\n");
        }
        else
            varId = NULL;

        if (varId == NULL)
            return;

        if (failedMsg == NULL)
            return;

        Sp<CSymbol> varSymbol = GetVarSymbol(node);
        if (varSymbol == NULL) {
            DEBUG_PRINT("symbol not found!\n");
            return;
        }

        FILE * file = (FILE*)pthread_getspecific(sKey);
        Sp<HashTable<Int32, String> > table = MapTable::GetWriteToParcelNameOfBasicType();
        if (table == NULL)
            return;

        if (varSymbol->mSymbolType == SymbolType_Class) {
            WRITE_STRING_TO_FILE(varId->Length() + 97,
                L"if (AXP::AFAILED(AXP::Libc::Common::ClassLoader::WriteObjectToParcel(parcel, %ls)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            WRITE_STRING_TO_FILE(varId->Length() + 97,
                L"if (AXP::AFAILED(parcel->WriteInt64(%ls->GetRemoteRef())))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
        }
        else if ((varSymbol->mSymbolType == SymbolType_String)
            || (varSymbol->mSymbolType == SymbolType_ByteArray)
            || (varSymbol->mSymbolType == SymbolType_Int8)
            || (varSymbol->mSymbolType == SymbolType_Byte)
            || (varSymbol->mSymbolType == SymbolType_UInt8)
            || (varSymbol->mSymbolType == SymbolType_Int16)
            || (varSymbol->mSymbolType == SymbolType_UInt16)
            || (varSymbol->mSymbolType == SymbolType_Int32)
            || (varSymbol->mSymbolType == SymbolType_UInt32)
            || (varSymbol->mSymbolType == SymbolType_Int64)
            || (varSymbol->mSymbolType == SymbolType_UInt64)
            || (varSymbol->mSymbolType == SymbolType_Boolean)
            || (varSymbol->mSymbolType == SymbolType_Float)
            || (varSymbol->mSymbolType == SymbolType_Double)
            || (varSymbol->mSymbolType == SymbolType_Int8_NULL)
            || (varSymbol->mSymbolType == SymbolType_Byte_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt8_NULL)
            || (varSymbol->mSymbolType == SymbolType_Int16_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt16_NULL)
            || (varSymbol->mSymbolType == SymbolType_Int32_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt32_NULL)
            || (varSymbol->mSymbolType == SymbolType_Int64_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt64_NULL)
            || (varSymbol->mSymbolType == SymbolType_Boolean_NULL)
            || (varSymbol->mSymbolType == SymbolType_Float_NULL)
            || (varSymbol->mSymbolType == SymbolType_Double_NULL)) {
            Sp<String> name = table->GetValue(varSymbol->mSymbolType);
            if (name == NULL)
                return;

            WRITE_STRING_TO_FILE(name->Length() + varId->Length() + 97,
                L"if (AXP::AFAILED(parcel->%ls(%ls)))\r\n", (PCWStr)*name, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");

                return;
            }

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                WRITE_STRING_TO_FILE(varId->Length() + 97,
                    L"if (AXP::AFAILED(AXP::Libc::Common::ClassLoader::WriteListOfObjectToParcel(parcel, %ls)))\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
            }
            else if ((elementSymbol->mSymbolType == SymbolType_String)
                || (elementSymbol->mSymbolType == SymbolType_ByteArray)
                || (elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Byte_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt8_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Int16_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt16_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Int32_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt32_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Int64_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt64_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Boolean_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Float_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Double_NULL)) {
                Sp<String> elementType = GetVarType(node->GetChildren()->Get(0)->GetChildren()->Get(0),
                    IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                if (elementType == NULL)
                    return;

                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 108,
                    L"AXP::Int32 length;\r\nif (%ls == NULL)\r\nlength = 0;\r\nelse\r\nlength = %ls->GetCount();\r\n\r\n",
                    (PCWStr)*varId, (PCWStr)*varId);
                FWRITE("AXP::Int8 type = 'L';\r\nif (AXP::AFAILED(parcel->Write((AXP::PCByte)&type, sizeof(type))))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (AXP::AFAILED(parcel->Write((AXP::PCByte)&length, sizeof(length))))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 17, L"if (%ls) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 70,
                    L"Foreach(%ls, obj, %ls) {\r\nif (obj == NULL)\r\n", (PCWStr)*elementType, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                Sp<String> functionName = table->GetValue(elementSymbol->mSymbolType);
                if (functionName == NULL)
                    return;

                if ((elementSymbol->mSymbolType == SymbolType_String)
                    || (elementSymbol->mSymbolType == SymbolType_ByteArray))
                    WRITE_STRING_TO_FILE(functionName->Length() + 71,
                    L"if (AXP::AFAILED(parcel->%ls(obj)))\r\n", (PCWStr)*functionName);
                else
                    WRITE_STRING_TO_FILE(functionName->Length() + 71,
                    L"if (AXP::AFAILED(parcel->%ls(obj->GetValue())))\r\n", (PCWStr)*functionName);

                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("}\r\n}\r\n}\r\n");
            }
            else {
                DEBUG_PRINT(L"not support list type!");
                return;
            }
        }
        else {
            DEBUG_PRINT(L"not support type!");
            return;
        }
    }

    STATIC Void ReadVarFromParcel(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return;

        Sp<String> varId;
        Sp<String> failedMsg;
        if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter)) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varId = node->GetChildren()->Get(1)->GetValue()->mContent;
            if (varId == NULL)
                return;

            varId = String::Create(varId->Length() + 17, L"%ls", (PCWStr)*varId);

            if (node->GetValue()->mSymbolType == SymbolType_Member)
                failedMsg = String::Create(L"return AXP::AE_FAIL;\r\n");
            else
                failedMsg = String::Create(L"return NULL;\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
            failedMsg = String::Create(L"throw std::exception();\r\n");
        }
        else
            varId = NULL;

        if (varId == NULL)
            return;

        if (failedMsg == NULL)
            return;

        Sp<CSymbol> varSymbol = GetVarSymbol(node);
        if (varSymbol == NULL) {
            DEBUG_PRINT("symbol not found!\n");

            return;
        }

        FILE * file = (FILE*)pthread_getspecific(sKey);
        Sp<HashTable<Int32, String> > table = MapTable::GetReadFromParcelNameOfBasicType();
        if (table == NULL)
            return;

        if (varSymbol->mSymbolType == SymbolType_Class) {
            WRITE_STRING_TO_FILE(varId->Length() + 97,
                L"if (AXP::AFAILED(AXP::Libc::Common::ClassLoader::ReadObjectFromParcel(parcel, %ls)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"AXP::Int64 _%ls;\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AXP::AFAILED(parcel->ReadInt64(_%ls)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if ((varSymbol->mSymbolType == SymbolType_String)
            || (varSymbol->mSymbolType == SymbolType_ByteArray)
            || (varSymbol->mSymbolType == SymbolType_Int8)
            || (varSymbol->mSymbolType == SymbolType_Byte)
            || (varSymbol->mSymbolType == SymbolType_UInt8)
            || (varSymbol->mSymbolType == SymbolType_Int16)
            || (varSymbol->mSymbolType == SymbolType_UInt16)
            || (varSymbol->mSymbolType == SymbolType_Int32)
            || (varSymbol->mSymbolType == SymbolType_UInt32)
            || (varSymbol->mSymbolType == SymbolType_Int64)
            || (varSymbol->mSymbolType == SymbolType_UInt64)
            || (varSymbol->mSymbolType == SymbolType_Boolean)
            || (varSymbol->mSymbolType == SymbolType_Float)
            || (varSymbol->mSymbolType == SymbolType_Double)
            || (varSymbol->mSymbolType == SymbolType_Int8_NULL)
            || (varSymbol->mSymbolType == SymbolType_Byte_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt8_NULL)
            || (varSymbol->mSymbolType == SymbolType_Int16_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt16_NULL)
            || (varSymbol->mSymbolType == SymbolType_Int32_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt32_NULL)
            || (varSymbol->mSymbolType == SymbolType_Int64_NULL)
            || (varSymbol->mSymbolType == SymbolType_UInt64_NULL)
            || (varSymbol->mSymbolType == SymbolType_Boolean_NULL)
            || (varSymbol->mSymbolType == SymbolType_Float_NULL)
            || (varSymbol->mSymbolType == SymbolType_Double_NULL)) {
            Sp<String> functionName = table->GetValue(varSymbol->mSymbolType);
            if (functionName == NULL)
                return;

            WRITE_STRING_TO_FILE(varId->Length() + functionName->Length() + 97,
                L"if (AXP::AFAILED(parcel->%ls(%ls)))\r\n", (PCWStr)*functionName, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                WRITE_STRING_TO_FILE(varId->Length() + 97,
                    L"if (AXP::AFAILED(AXP::Libc::Common::ClassLoader::ReadListOfObjectFromParcel(parcel, %ls)))\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
            }
            else if ((elementSymbol->mSymbolType == SymbolType_String)
                || (elementSymbol->mSymbolType == SymbolType_ByteArray)
                || (elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Byte_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt8_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Int16_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt16_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Int32_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt32_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Int64_NULL)
                || (elementSymbol->mSymbolType == SymbolType_UInt64_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Boolean_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Float_NULL)
                || (elementSymbol->mSymbolType == SymbolType_Double_NULL)) {
                Sp<String> functionName = table->GetValue(elementSymbol->mSymbolType);
                if (functionName == NULL)
                    return;

                Sp<String> elementType = GetVarType(
                    node->GetChildren()->Get(0)->GetChildren()->Get(0),
                    IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                if (elementType == NULL)
                    return;

                FWRITE("{\r\n");
                FWRITE("AXP::Int8 type;\r\nif (AXP::AFAILED(parcel->Read((AXP::PByte)&type, sizeof(type), sizeof(type))))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (type != 'L')\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("AXP::Int32 length;\r\nif (AXP::AFAILED(parcel->Read((AXP::PByte)&length, sizeof(length), sizeof(length))))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() * 2 + elementType->Length() + 57,
                    L"%ls = new AXP::List<%ls>();\r\nif (%ls == NULL)\r\n", (PCWStr)*varId, (PCWStr)*elementType, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("for (AXP::Int32 i = 0; i < length; i++) {\r\n");
                if ((elementSymbol->mSymbolType == SymbolType_String)
                    || (elementSymbol->mSymbolType == SymbolType_ByteArray))
                    WRITE_STRING_TO_FILE(elementType->Length() + 27, L"AXP::Sp<%ls> obj;\r\n", (PCWStr)*elementType);
                else
                    WRITE_STRING_TO_FILE(elementType->Length() + 27, L"%ls obj;\r\n", (PCWStr)*elementType);

                WRITE_STRING_TO_FILE(functionName->Length() + 40,
                    L"if (AXP::AFAILED(parcel->%ls(obj)))\r\n", (PCWStr)*functionName);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                if ((elementSymbol->mSymbolType == SymbolType_String)
                    || (elementSymbol->mSymbolType == SymbolType_ByteArray))
                    WRITE_STRING_TO_FILE(varId->Length() + 37,
                    L"if (!%ls->PushBack(obj))\r\n", (PCWStr)*varId);
                else
                    WRITE_STRING_TO_FILE(varId->Length() + elementType->Length() + 37,
                    L"if (!%ls->PushBack(new %ls(obj)))\r\n", (PCWStr)*varId, (PCWStr)*elementType);

                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("}\r\n}\r\n\r\n");
            }
            else {
                DEBUG_PRINT(L"not support list type!");
                return;
            }
        }
        else {
            DEBUG_PRINT(L"not support type!");

            return;
        }
    }

    STATIC Void ReadParametersFromParcel(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Parameter)
                ReadVarFromParcel(obj);
        }
    }

    STATIC Sp<String> GetFormParameters(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return NULL;

        Sp<String> parameterList = String::Create(L"");
        if (parameterList == NULL)
            return NULL;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if ((obj == NULL) || (obj->GetValue() == NULL) || (obj->GetValue()->mSymbolType != SymbolType_Parameter))
                return NULL;

            Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_FORM_PARAMETER);
            if (varType == NULL)
                return NULL;

            if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                return NULL;

            Sp<String> varId = obj->GetChildren()->Get(1)->GetValue()->mContent;
            if (varId == NULL)
                return NULL;

            if (parameterList->Equals(L""))
                parameterList = String::Create(varType->Length() + varId->Length() + 7, L"%ls %ls", (PCWStr)*varType, (PCWStr)*varId);
            else
                parameterList = String::Create(parameterList->Length() + varType->Length() + varId->Length() + 37,
                L"%ls, %ls %ls", (PCWStr)*parameterList, (PCWStr)*varType, (PCWStr)*varId);

            if (parameterList == NULL)
                return NULL;
        }

        return parameterList;
    }

    STATIC Sp<String> GetRealParameters(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return NULL;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> realParameterList;
        Foreach(TreeNode<CSymbol>, tmp, node->GetChildren()) {
            if (tmp->GetValue() && tmp->GetValue()->mSymbolType == SymbolType_Parameter) {
                if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                    return NULL;

                Sp<String> var = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                if (var == NULL)
                    return NULL;

                if (realParameterList == NULL)
                    realParameterList = String::Create(var->Length(), L"%ls", (PCWStr)*var);
                else
                    realParameterList = String::Create(realParameterList->Length() + var->Length() + 7,
                    L"%ls, %ls", (PCWStr)*realParameterList, (PCWStr)*var);

                if (realParameterList == NULL)
                    return NULL;
            }
        }

        if (realParameterList == NULL)
            realParameterList = String::Create(L"");

        return realParameterList;
    }

    STATIC Void WriteObjectHolderStubFunction(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Int32 countofFunction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_Function)) {
                if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> functionName = obj->GetChildren()->Get(1)->GetValue()->mContent;
                if (functionName == NULL)
                    return;

                Sp<String> funCodeName = GetConsOrFuncId(obj);
                if (funCodeName == NULL)
                    return;

                WRITE_STRING_TO_FILE(funCodeName->Length() + 39,
                    L"%lsif (funCode == %ls) {\r\n",
                    countofFunction++ == 0 ? L"" : L"else ", (PCWStr)*funCodeName);

                Boolean flag = FALSE;
                if (obj->GetChildren()->Get(2) == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()->Get(2)->GetChildren()) {
                    if (tmp->GetValue()->mSymbolType != SymbolType_Parameter)
                        continue;

                    Sp<String> varType = GetVarType(tmp->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                    if (varType == NULL)
                        return;

                    if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                        return;

                    Sp<String> varId = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                    if (varId == NULL)
                        return;

                    Sp<CSymbol> varSymbol = GetVarSymbol(tmp);
                    if (varSymbol == NULL) {
                        DEBUG_PRINT("symbol not found!\n");
                        return;
                    }

                    if (varSymbol->mSymbolType == SymbolType_Interface) {
                        Sp<String> className = GetTypeReferenceList(tmp->GetChildren()->Get(0), IDL_LANG_CPP);
                        if (className == NULL)
                            return;

                        flag = TRUE;
                        ReadVarFromParcel(tmp);
                        FWRITE("AXP::Sp<AXP::String> token;\r\nif (AXP::AFAILED(parcel->ReadString(token)))\r\nreturn NULL;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 17, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                        FWRITE("try {\r\n");
                        WRITE_STRING_TO_FILE(className->Length() + varId->Length() * 2 + 51,
                            L"%ls = %ls::Create(_%ls, token);\r\n",
                            (PCWStr)*varId, (PCWStr)*className, (PCWStr)*varId);
                        FWRITE("}\r\ncatch (std::exception & e) {\r\nreturn NULL;\r\n}\r\n\r\n");
                        break;
                    }
                    else {
                        WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 17, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);

                        ReadVarFromParcel(tmp);
                    }
                }

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(2));
                if (realParameterList == NULL)
                    return;

                if (obj->GetChildren()->Get(0) == NULL)
                    return;

                Sp<CSymbol> returnSymbol = obj->GetChildren()->Get(0)->GetValue();
                if (returnSymbol == NULL)
                    return;

                if (returnSymbol->mSymbolType == SymbolType_Void) {
                    WRITE_STRING_TO_FILE(42 + functionName->Length() + realParameterList->Length(),
                        L"mService->%ls(%ls);\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);

                    FWRITE("parcel->Reset();\r\n");
                    FWRITE("if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))\r\nreturn NULL;\r\n");
                }
                else {
                    Sp<String> returnType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                    if (returnType == NULL)
                        return;

                    WRITE_STRING_TO_FILE(77 + returnType->Length() + functionName->Length() + realParameterList->Length(),
                        L"%ls ret = mService->%ls(%ls);\r\n",
                        (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*realParameterList);

                    FWRITE("parcel->Reset();\r\n");
                    FWRITE("if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))\r\nreturn NULL;\r\n\r\n");

                    WriteVarToParcel(obj);
                }

                FWRITE("}\r\n");
            }
        }
    }

    STATIC Void WriteParametersToParcel(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Parameter) {
                WriteVarToParcel(obj);
                FWRITE("\r\n");
            }
        }
    }

    STATIC Void WriteObjectHolderOnTransact(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("AXP::Sp<AXP::CParcel> OnTransact(\r\n"
            "IN CONST AXP::Sp<AXP::CParcel> & parcel,\r\n"
            "IN CONST AXP::Sp<AXP::String> & uri)\r\n{\r\n");
        FWRITE("if (parcel == NULL)\r\nreturn NULL;\r\n\r\n");
        FWRITE("AXP::Int32 funCode;\r\nif (AXP::AFAILED(parcel->ReadInt32(funCode)))\r\nreturn NULL;\r\n\r\n");

        Int32 countOfConstruction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_Construction)) {
                Sp<String> consCodeId = GetConsOrFuncId(obj);
                if (consCodeId == NULL)
                    return;

                WRITE_STRING_TO_FILE(consCodeId->Length() + 40, L"%lsif (funCode == %ls) {\r\n",
                    countOfConstruction++ == 0 ? L"" : L"else ", (PCWStr)*consCodeId);
                FWRITE("if (mService == NULL) {\r\n");

                Foreach(TreeNode<CSymbol>, param, obj->GetChildren()->Get(1)->GetChildren()) {
                    if ((param == NULL) || (param->GetValue() == NULL))
                        return;

                    Sp<String> varType = GetVarType(param->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                    Sp<String> varId = GetVarId(param);
                    if ((varType == NULL) || (varId == NULL))
                        return;

                    WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 7, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                }

                ReadParametersFromParcel(obj->GetChildren()->Get(1));

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                FWRITE("Synchronized (&mLock) {\r\nif (mService == NULL) {\r\n");
                WRITE_STRING_TO_FILE(className->Length() + realParameterList->Length() + 33,
                    L"mService = _%ls::Create(%ls);\r\n", (PCWStr)*className, (PCWStr)*realParameterList);
                FWRITE("if (mService == NULL)\r\nreturn NULL;\r\n}\r\n}\r\n}\r\n\r\n");
            }
        }

        if (countOfConstruction == 0) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 40, L"if (funCode == %ls) {\r\n", (PCWStr)*defConsId);
            FWRITE("if (mService == NULL) {\r\n");
            FWRITE("Synchronized (&mLock) {\r\nif (mService == NULL) {\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 33, L"mService = _%ls::Create();\r\n", (PCWStr)*className);
            FWRITE("if (mService == NULL)\r\nreturn NULL;\r\n}\r\n}\r\n}\r\n\r\n");
        }

        FWRITE("parcel->Reset();\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))\r\nreturn NULL;\r\n\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)this)))\r\nreturn NULL;\r\n\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 97,
            L"if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)(I%ls*)mService.Get())))\r\nreturn NULL;\r\n}\r\n", (PCWStr)*className);
        FWRITE("else {\r\nif (mService == NULL) {\r\nparcel->Reset();\r\nif (AXP::AFAILED(parcel->"
            "WriteInt32((AXP::Int32)IPC::RemoteRefException)))\r\nreturn NULL;\r\n\r\nreturn parcel;\r\n}\r\n\r\n");

        WriteObjectHolderStubFunction(node);

        FWRITE("}\r\n\r\nreturn parcel;\r\n}\r\n\r\n");
    }

    STATIC Void WriteProxyCreates(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        if ((node->GetParent() == NULL)
            || (node->GetParent()->GetChildren()->Get(0) == NULL)
            || (node->GetParent()->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> namespacz = node->GetParent()->GetChildren()->Get(0)->GetValue()->mContent;
        if (namespacz == NULL)
            return;

        Boolean hasConstructions = FALSE;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Construction) {
                hasConstructions = TRUE;
                Sp<String> formParameterList = GetFormParameters(obj->GetChildren()->Get(1));
                if (formParameterList == NULL)
                    return;

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 87,
                    L"STATIC AXP::Sp<%ls> Create(%ls)\r\n", (PCWStr)*className, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(className->Length() + realParameterList->Length() + 37,
                    L"return new %ls(%ls);\r\n", (PCWStr)*className, (PCWStr)*realParameterList);
                FWRITE("}\r\n\r\n");
            }
        }

        if (!hasConstructions) {
            WRITE_STRING_TO_FILE(className->Length() + 77, L"STATIC AXP::Sp<%ls> Create()\r\n", (PCWStr)*className);
            FWRITE("{\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 37,
                L"return new %ls();\r\n", (PCWStr)*className);
            FWRITE("}\r\n\r\n");
        }

        WRITE_STRING_TO_FILE(className->Length() + 19,
            L"STATIC AXP::Sp<%ls>", (PCWStr)*className);
        FWRITE(" Create(IN AXP::Int64 objRef, IN CONST AXP::Sp<AXP::String> & token)\r\n{\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 87, L"return new %ls(objRef, token);\r\n", (PCWStr)*className);
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyConstructions(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("protected:\r\n\r\n");
        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        Boolean hasConstructions = FALSE;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Construction) {
                hasConstructions = TRUE;
                Sp<String> formParameterList = GetFormParameters(obj->GetChildren()->Get(1));
                if (formParameterList == NULL)
                    return;

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                Sp<String> consCodeId = GetConsOrFuncId(obj);
                if (consCodeId == NULL)
                    return;

                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 7,
                    L"%ls(%ls)\r\n", (PCWStr)*className, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                FWRITE("mTag = (AXP::Int8)0x83;\r\nmInterface = NULL;\r\n");
                FWRITE("AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 97,
                    L"AXP::Sp<IPC::IStub> stub = IPC::ServiceManager::GetService(%ls);\r\n", (PCWStr)*descptorName);
                FWRITE("if (stub == NULL) {\r\n");
                FWRITE("mIsRemote = TRUE;\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 117,
                    L"mConn = IPC::CServerConnection::Create(%ls);\r\nif (mConn == NULL)\r\nthrow std::exception();\r\n\r\n",
                    (PCWStr)*descptorName);
                FWRITE("if (AXP::AFAILED(parcel->WriteInt8(mTag)))\r\nthrow std::exception();\r\n\r\n"
                    "if (AXP::AFAILED(parcel->WriteString(mToken)))\r\nthrow std::exception();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 88,
                    L"if (AXP::AFAILED(parcel->WriteString(%ls)))\r\nthrow std::exception();\r\n", (PCWStr)*descptorName);
                FWRITE("}\r\nelse {\r\n");
                FWRITE("mIsRemote = FALSE;\r\nmConn = stub;\r\n");
                FWRITE("}\r\n\r\n");
                FWRITE("if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CREATE)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (AXP::AFAILED(parcel->WriteBoolean(mIsRemote)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (mIsRemote) {\r\n");
                FWRITE("if (AXP::AFAILED(parcel->WriteString(IPC::ServiceManager::sServerAddress)))\r\nthrow std::exception();\r\n");
                FWRITE("}\r\n\r\n");
                WRITE_STRING_TO_FILE(consCodeId->Length() + 117, L"if (AXP::AFAILED(parcel->WriteInt32(%ls)))\r\n", (PCWStr)*consCodeId);
                FWRITE("throw std::exception();\r\n\r\n");

                WriteParametersToParcel(obj->GetChildren()->Get(1));

                FWRITE("parcel->Reset();\r\n");
                FWRITE("parcel = mConn->Transact(parcel);\r\nif (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
                FWRITE("parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::"
                    "AFAILED(parcel->ReadInt32(code)))\r\nthrow std::exception();\r\n\r\nIPC::ReadException(code);\r\n");
                FWRITE("if (AXP::AFAILED(parcel->ReadInt64(mRef)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (!mIsRemote) {\r\n");
                FWRITE("AXP::Int64 serviceObject;\r\nif (AXP::AFAILED(parcel->ReadInt64(serviceObject)))\r\nthrow std::exception();\r\n\r\n");
                WRITE_STRING_TO_FILE(className->Length() + 38, L"mInterface = (I%ls*)serviceObject;\r\n", (PCWStr)*className);
                FWRITE("}\r\n");
                FWRITE("}\r\n\r\n");
            }
        }

        if (!hasConstructions) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(className->Length() + 7,
                L"%ls()\r\n", (PCWStr)*className);
            FWRITE("{\r\n");
            FWRITE("mTag = (AXP::Int8)0x83;\r\nmInterface = NULL;\r\n");
            FWRITE("AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 97,
                L"AXP::Sp<IPC::IStub> stub = IPC::ServiceManager::GetService(%ls);\r\n", (PCWStr)*descptorName);
            FWRITE("if (stub == NULL) {\r\n");
            FWRITE("mIsRemote = TRUE;\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 117,
                L"mConn = IPC::CServerConnection::Create(%ls);\r\nif (mConn == NULL)\r\nthrow std::exception();\r\n\r\n",
                (PCWStr)*descptorName);
            FWRITE("if (AXP::AFAILED(parcel->WriteInt8(mTag)))\r\nthrow std::exception();\r\n\r\n"
                "if (AXP::AFAILED(parcel->WriteString(mToken)))\r\nthrow std::exception();\r\n\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 88,
                L"if (AXP::AFAILED(parcel->WriteString(%ls)))\r\nthrow std::exception();\r\n", (PCWStr)*descptorName);
            FWRITE("}\r\nelse {\r\n");
            FWRITE("mIsRemote = FALSE;\r\nmConn = stub;\r\n");
            FWRITE("}\r\n\r\n");
            FWRITE("if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CREATE)))\r\nthrow std::exception();\r\n\r\n");
            FWRITE("if (AXP::AFAILED(parcel->WriteBoolean(mIsRemote)))\r\nthrow std::exception();\r\n\r\n");
            FWRITE("if (mIsRemote) {\r\n");
            FWRITE("if (AXP::AFAILED(parcel->WriteString(IPC::ServiceManager::sServerAddress)))\r\nthrow std::exception();\r\n");
            FWRITE("}\r\n\r\n");
            WRITE_STRING_TO_FILE(defConsId->Length() + 70, L"if (AXP::AFAILED(parcel->WriteInt32(%ls)))\r\n", (PCWStr)*defConsId);
            FWRITE("throw std::exception();\r\n\r\n");
            FWRITE("parcel->Reset();\r\n");
            FWRITE("parcel = mConn->Transact(parcel);\r\nif (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
            FWRITE("parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::"
                "AFAILED(parcel->ReadInt32(code)))\r\nthrow std::exception();\r\n\r\nIPC::ReadException(code);\r\n");
            FWRITE("if (AXP::AFAILED(parcel->ReadInt64(mRef)))\r\nthrow std::exception();\r\n\r\n");
            FWRITE("if (!mIsRemote) {\r\n");
            FWRITE("AXP::Int64 serviceObject;\r\nif (AXP::AFAILED(parcel->ReadInt64(serviceObject)))\r\nthrow std::exception();\r\n\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 38, L"mInterface = (I%ls*)serviceObject;\r\n", (PCWStr)*className);
            FWRITE("}\r\n");
            FWRITE("}\r\n\r\n");
        }

        WRITE_STRING_TO_FILE(className->Length() + 88, L"%ls(IN AXP::Int64 objRef, IN CONST AXP::Sp<AXP::String> & token)\r\n", (PCWStr)*className);
        FWRITE("{\r\n");
        FWRITE("mTag = (AXP::Int8)0x84;\r\nmToken = token;\r\nmInterface = NULL;\r\nmIsRemote = TRUE;\r\nmRef = objRef;\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 110,
            L"mConn = IPC::CServerConnection::Create(%ls);\r\nif (mConn == NULL)\r\nthrow std::exception();\r\n\r\n", (PCWStr)*descptorName);
        /*        FWRITE("AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 70,
                L"if (AXP::AFAILED(parcel->Write(%ls)))\r\nthrow std::exception();\r\n\r\n", (PCWStr)*descptorName);
                FWRITE("if (AXP::AFAILED(parcel->Write(IPC::CommandCode::COMMAND_CALLBACK)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (AXP::AFAILED(parcel->Write(IPC::ServiceManager::sServerAddress)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (AXP::AFAILED(parcel->Write(mRef)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("parcel->Reset();\r\n");
                FWRITE("parcel = mConn->Transact(parcel);\r\n");
                FWRITE("if (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
                FWRITE("parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(parcel->Read(code)))\r\n"
                "throw std::exception();\r\n\r\nIPC::ReadException(code);\r\n");
                FWRITE("AXP::Int64 obj;\r\nif (AXP::AFAILED(parcel->Read(obj)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (obj != mRef)\r\nthrow std::exception();\r\n");*/
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyDestruction(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("public:\r\n\r\n");

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        WRITE_STRING_TO_FILE(className->Length() + 30, L"VIRTUAL ~%ls()\r\n", (PCWStr)*className);
        FWRITE("{\r\n");
        FWRITE("if (mToken)\r\nreturn;\r\n");
        FWRITE("try {\r\n");
        FWRITE("AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();\r\n");
        FWRITE("if (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
        FWRITE("if (mIsRemote) {\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteInt8(mTag)))\r\nthrow std::exception();\r\n\r\n"
            "if (AXP::AFAILED(parcel->WriteString(mToken)))\r\nthrow std::exception();\r\n\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 117,
            L"if (AXP::AFAILED(parcel->WriteString(%ls)))\r\nthrow std::exception();\r\n", (PCWStr)*descptorName);
        FWRITE("}\r\n\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_RELEASE)))\r\nthrow std::exception();\r\n\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteBoolean(mIsRemote)))\r\nthrow std::exception();\r\n\r\n");
        FWRITE("if (mIsRemote)\r\n {\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteString(IPC::ServiceManager::sServerAddress)))\r\nthrow std::exception();\r\n");
        FWRITE("}\r\n\r\n");
        FWRITE("if (AXP::AFAILED(parcel->WriteInt64(mRef)))\r\nthrow std::exception();\r\n\r\n");
        FWRITE("parcel->Reset();\r\n");
        FWRITE("parcel = mConn->Transact(parcel);\r\n");
        FWRITE("if ((mTag & 0x0F) == 0x04)\r\nreturn;\r\n\r\n");
        FWRITE("if (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
        FWRITE("parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(parcel->ReadInt32(code)))\r\n"
            "throw std::exception();\r\n\r\nIPC::ReadException(code);\r\n");
        FWRITE("}\r\ncatch(...){\r\n}\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void WriteProxyFunctions(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("\r\npublic:\r\n\r\n");
        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        if ((node->GetParent() == NULL)
            || (node->GetParent()->GetChildren()->Get(0) == NULL)
            || (node->GetParent()->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> namespacz = node->GetParent()->GetChildren()->Get(0)->GetValue()->mContent;
        if (namespacz == NULL)
            return;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        Boolean hasConstructions = FALSE;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Function) {
                Sp<String> returnType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                if (returnType == NULL)
                    return;

                if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> functionName = obj->GetChildren()->Get(1)->GetValue()->mContent;
                if (functionName == NULL)
                    return;

                Sp<String> funcId = GetConsOrFuncId(obj);
                if (funcId == NULL)
                    return;

                Sp<String> formParameterList = GetFormParameters(obj->GetChildren()->Get(2));
                if (formParameterList == NULL)
                    return;

                if (obj->GetChildren()->Get(2) == NULL)
                    return;

                Sp<String> addRemoteRef = String::Create(L"");
                if (addRemoteRef == NULL)
                    return;

                if (obj->GetChildren()->Get(2)->GetChildren()->Get(0)) {
                    Sp<CSymbol> varSymbol = GetVarSymbol(obj->GetChildren()->Get(2)->GetChildren()->Get(0));
                    if (varSymbol && (varSymbol->mSymbolType == SymbolType_Interface)) {
                        if (obj->GetChildren()->Get(2)->GetChildren()->Get(0)->GetChildren()->Get(1)
                            || obj->GetChildren()->Get(2)->GetChildren()->Get(0)->GetChildren()->Get(1)->GetValue()
                            || obj->GetChildren()->Get(2)->GetChildren()->Get(0)->GetChildren()->Get(1)->GetValue()->mContent) {
                            Sp<String> varId = obj->GetChildren()->Get(2)->GetChildren()->Get(0)->GetChildren()->Get(1)->GetValue()->mContent;
                            addRemoteRef = String::Create(varId->Length() + descptorName->Length() + 197,
                                L"%ls->AddRemoteRef(IPC::ServiceManager::GetProxyAddr((AXP::PCWStr)*%ls));\r\n",
                                (PCWStr)*varId, (PCWStr)*descptorName);
                            if (addRemoteRef == NULL)
                                return;
                        }
                    }
                }

                WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length() + formParameterList->Length() + 25,
                    L"%ls %ls(%ls)\r\n", (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                FWRITE("if (mIsRemote) {\r\n");
                FWRITE("AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (AXP::AFAILED(parcel->WriteInt8(mTag)))\r\nthrow std::exception();\r\n\r\n"
                    "if (AXP::AFAILED(parcel->WriteString(mToken)))\r\nthrow std::exception();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 98,
                    L"if (AXP::AFAILED(parcel->WriteString(%ls)))\r\nthrow std::exception();\r\n\r\n", (PCWStr)*descptorName);
                FWRITE("if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CALL)))\r\nthrow std::exception();\r\n\r\n");
                FWRITE("if (AXP::AFAILED(parcel->WriteInt64(mRef)))\r\nthrow std::exception();\r\n\r\n");
                WRITE_STRING_TO_FILE(funcId->Length() + 88, L"if (AXP::AFAILED(parcel->WriteInt32(%ls)))\r\n", (PCWStr)*funcId);
                FWRITE("throw std::exception();\r\n\r\n");

                WriteParametersToParcel(obj->GetChildren()->Get(2));

                FWRITE("parcel->Reset();\r\n");
                FWRITE("parcel = mConn->Transact(parcel);\r\n");
                FWRITE("if ((mTag & 0x0F) == 0x04)\r\n");
                if (obj->GetChildren()->Get(0)
                    && obj->GetChildren()->Get(0)->GetValue()
                    && (obj->GetChildren()->Get(0)->GetValue()->mSymbolType != SymbolType_Void)) {
                    FWRITE("return 0;\r\n\r\n");
                    FWRITE("if (parcel == NULL)\r\nthrow std::exception();\r\n\r\nparcel->Reset();\r\n"
                        "AXP::Int32 code;\r\nif (AXP::AFAILED(parcel->ReadInt32(code)))\r\nthrow"
                        " std::exception();\r\n\r\nIPC::ReadException(code);\r\n");
                    WRITE_STRING_TO_FILE(addRemoteRef);
                    WRITE_STRING_TO_FILE(returnType->Length() + 7, L"%ls ret;\r\n", (PCWStr)*returnType);
                    ReadVarFromParcel(obj);
                    FWRITE("return ret;\r\n");
                }
                else {
                    FWRITE("return;\r\n\r\n");
                    FWRITE("if (parcel == NULL)\r\nthrow std::exception();\r\n\r\nparcel->Reset();\r\n"
                        "AXP::Int32 code;\r\nif (AXP::AFAILED(parcel->ReadInt32(code)))\r\nthrow"
                        " std::exception();\r\n\r\nIPC::ReadException(code);\r\n");
                    WRITE_STRING_TO_FILE(addRemoteRef);
                    FWRITE("return;\r\n");
                }

                FWRITE("}\r\nelse {\r\n");
                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(2));
                if (realParameterList == NULL)
                    return;

                if (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_Void)
                    WRITE_STRING_TO_FILE(functionName->Length() + realParameterList->Length() + 37,
                    L"mInterface->%ls(%ls);\r\nreturn;\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);
                else
                    WRITE_STRING_TO_FILE(functionName->Length() + realParameterList->Length() + 37,
                    L"return mInterface->%ls(%ls);\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);

                FWRITE("}\r\n");
                FWRITE("}\r\n\r\n");
            }
        }
    }

    STATIC Void WriteProxyGetRemoteRef(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("public:\r\n\r\n");
        FWRITE("AXP::Int64 GetRemoteRef()\r\n{\r\nreturn mRef;\r\n}\r\n\r\n");
    }

    STATIC Void WriteProxyMembers(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;


        if ((node->GetParent()->GetChildren()->Get(0) == NULL)
            || (node->GetParent()->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> namespacz = node->GetParent()->GetChildren()->Get(0)->GetValue()->mContent;
        if (namespacz == NULL)
            return;

        if ((node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL)
            || (node->GetChildren()->Get(1)->GetValue()->mContent == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("private:\r\n\r\n");
        FWRITE("AXP::Int8 mTag;\r\nAXP::Boolean mIsRemote;\r\n");
        FWRITE("AXP::Int64 mRef;\r\n");
        FWRITE("AXP::Sp<IPC::IStub> mConn;\r\nAXP::Sp<AXP::String> mToken;\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 38, L"AXP::Wp<I%ls> mInterface;\r\n", (PCWStr)*className);

        WriteFunCodeMembers(node);

        WRITE_STRING_TO_FILE(descptorName->Length() + namespacz->Length() + className->Length() + 87,
            L"CONST AXP::Sp<AXP::String> %ls = AXP::String::Create(L\"%ls.%ls\");\r\n",
            (PCWStr)*descptorName, (PCWStr)*namespacz, (PCWStr)*className);
    }

    STATIC Void WriteIncludeFile(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Int32 mode = GetMode(node);
        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_FileBegin) {
                Sp<String> fileName = obj->GetValue()->mContent;
                if (fileName == NULL)
                    return;

                Int32 index1 = fileName->LastIndexOf(L'/');
                Int32 index2 = fileName->LastIndexOfString(L".idl");
                if (index2 == -1)
                    return;

                if (index1 > 1) {
                    Sp<String> obj = fileName->SubString(0, index2);
                    if (obj == NULL)
                        return;

                    WRITE_STRING_TO_FILE(obj->Length() + 22, L"#include \"%ls.h\"\r\n", (PCWStr)*obj);
                    continue;
                }

                Sp<String> fileNameWithoutSuffix = fileName->SubString(index1 + 1, index2 - index1 - 1);
                if (fileNameWithoutSuffix == NULL)
                    return;

                Int32 modeFlag = GetMode(obj);
                Sp<String> tmp = GetNameSpace(obj);
                if (tmp == NULL)
                    return;

                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + sizeof("#include \"IPC/gen/%ls/%ls.h\"\r\n"),
                    L"#include \"IPC/gen/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                /*
                if (mode & IDL_MODE_IPC) {
                if (modeFlag & IDL_MODE_IPC)
                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                L"#include \"../../proxy/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                else
                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                L"#include \"../../models/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                }
                else if (mode & IDL_MODE_MODELS) {
                if (namespacz->Equals(tmp)) {
                if (modeFlag & IDL_MODE_IPC)
                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                L"#include \"../proxy/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                else
                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 37,
                L"#include \"%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);
                }
                else {
                if (modeFlag & IDL_MODE_IPC)
                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                L"#include \"../proxy/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                else
                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                L"#include \"../%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                }
                }
                */
            }
        }
    }

    STATIC Void WriteUsing(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<TreeNode<CSymbol> > obj = node->GetChildren()->Get(0);
        if ((obj == NULL) || (obj->GetValue() == NULL) || (obj->GetValue()->mContent == NULL))
            return;

        if (obj->GetChildren()->Get(0) == NULL) {
            WRITE_STRING_TO_FILE(obj->GetValue()->mContent->Length() + 22,
                L"#include \"%ls.h\"\r\n", (PCWStr)*obj->GetValue()->mContent);

            return;
        }

        FWRITE("#include \"");

        while (true) {
            if ((obj == NULL) || (obj->GetValue() == NULL) || (obj->GetValue()->mContent == NULL))
                return;

            if (obj->GetChildren()->Get(0)) {
                WRITE_STRING_TO_FILE(obj->GetValue()->mContent->Length() + 22,
                    L"%ls/", (PCWStr)*obj->GetValue()->mContent);

                obj = obj->GetChildren()->Get(0);
            }
            else {
                WRITE_STRING_TO_FILE(obj->GetValue()->mContent->Length() + 22,
                    L"%ls.h\"\r\n", (PCWStr)*obj->GetValue()->mContent);

                break;
            }
        }
    }

    STATIC Void WriteUsingList(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            WriteUsing(obj);
        }
    }

    STATIC Void WriteStubFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> stub = String::Create(L"Stub");
        if (stub == NULL)
            return;

        Sp<String> objectHolder = String::Create(L"ObjectHolder");
        if (objectHolder == NULL)
            return;

        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*szNamespace);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + stub->Length() + 15,
            L"%ls/%ls%ls.cpp", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix, (PCWStr)*stub);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        FWRITE("#include \"IPC/cplusplus/lib/include/ServiceManager.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/IpcException.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/UriManager.h\"\r\n\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if (((obj->GetChildren()->Get(0) == NULL)) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                WRITE_STRING_TO_FILE(namespacz->Length() + 22, L"namespace %ls\r\n{\r\n", (PCWStr)*namespacz);

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && tmp->GetValue()->mSymbolType == SymbolType_Interface) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + objectHolder->Length() + 88,
                            L"EXTERN AXP::Sp<IPC::CObjectHolder> Create%ls%ls();\r\n\r\n",
                            (PCWStr)*className, (PCWStr)*objectHolder);
                        WRITE_STRING_TO_FILE(className->Length() + stub->Length() + 38,
                            L"class %ls%ls : public IPC::IStub\r\n", (PCWStr)*className, (PCWStr)*stub);
                        FWRITE("{\r\npublic:\r\n\r\n");
                        WRITE_STRING_TO_FILE(className->Length() + stub->Length() + 18, L"%ls%ls()\r\n",
                            (PCWStr)*className, (PCWStr)*stub);
                        FWRITE("{\r\nmServiceList = new AXP::HashTable<AXP::Int64, IPC::CObjectHolder>(50);\r\n}\r\n\r\n");
                        FWRITE("AXP::Void AddRemoteRef(IN CONST AXP::Sp<AXP::String> & uri, IN AXP::Int64 objRef)\r\n"
                            "{\r\nif (mServiceList == NULL)\r\nreturn;\r\n\r\nAXP::Sp<IPC::CObjectHolder> obj"
                            " = mServiceList->GetValue(objRef);\r\nif (obj && uri){\r\nIPC::UriManager::StartThread(uri);\r\n"
                            "obj->mUriList->PushBack(uri);\r\nobj->AddRemoteRef();\r\n}\r\n}\r\n\r\n");
                        FWRITE("AXP::Sp<AXP::CParcel> Transact(IN CONST AXP::Sp<AXP::CParcel> & bundle)\r\n");
                        FWRITE("{\r\n");
                        FWRITE("if (bundle == NULL)\r\nreturn NULL;\r\n\r\n");
                        FWRITE("AXP::Int32 code;\r\n");
                        FWRITE("if (AXP::AFAILED(bundle->ReadInt32(code)))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("AXP::Sp<AXP::String> uri = NULL;\r\nswitch (code) {\r\ncase IPC::CommandCode::COMMAND_CREATE:\r\n"
                            "case IPC::CommandCode::COMMAND_RELEASE:\r\nAXP::Boolean isRemote;\r\nif (AXP::AFAILED(bundle->ReadBoolean(isRemote)))\r\n"
                            "return NULL;\r\n\r\nif (isRemote) {\r\nif (AXP::AFAILED(bundle->ReadString(uri)))\r\nreturn NULL;\r\n}\r\n\r\nbreak;\r\n"
                            "case IPC::CommandCode::COMMAND_CALLBACK:\r\nif (AXP::AFAILED(bundle->ReadString(uri)))\r\nreturn NULL;\r\n\r\nbreak;\r\n"
                            "case IPC::CommandCode::COMMAND_CALL:\r\nbreak;\r\ndefault:\r\nreturn NULL;\r\n}\r\n\r\n");
                        FWRITE("AXP::Sp<AXP::CParcel> parcel;\r\n");
                        FWRITE("AXP::Sp<IPC::CObjectHolder> objectHolder = CreateOrGetObjectHolder(code, uri, bundle, parcel);\r\n");
                        FWRITE("if (objectHolder == NULL)\r\nreturn parcel;\r\n\r\n");
                        FWRITE("if (code == IPC::CommandCode::COMMAND_RELEASE) {\r\n");
                        FWRITE("if (AXP::AFAILED(ReleaseObjectHolder(objectHolder, uri, FALSE)))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nreturn NULL;\r\n\r\n");
                        FWRITE("if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("return parcel;\r\n");
                        FWRITE("}\r\n");
                        FWRITE("else if (code == IPC::CommandCode::COMMAND_CALLBACK) {\r\n");
                        FWRITE("parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nreturn NULL;\r\n\r\n");
                        FWRITE("if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)objectHolder.Get())))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("return parcel;\r\n");
                        FWRITE("}\r\n");
                        FWRITE("else {\r\nparcel = objectHolder->OnTransact(bundle, uri);\r\n"
                            "if ((parcel == NULL) && (code == IPC::CommandCode::COMMAND_CREATE))\r\n"
                            "ReleaseObjectHolder(objectHolder, uri, FALSE);\r\n\r\nreturn parcel;\r\n}\r\n");
                        FWRITE("}\r\n\r\n");

                        FWRITE("AXP::Void OnDeath(IN CONST AXP::Sp<AXP::String> & uri)\r\n");
                        FWRITE("{\r\n");
                        FWRITE("if (mServiceList == NULL)\r\nreturn;\r\n\r\n");
                        FWRITE("if (uri == NULL)\r\nreturn;\r\n\r\nSynchronized(&mServiceList->mLock) {\r\n"
                            "AXP::Sp<AXP::List<IPC::CObjectHolder> > list = mServiceList->GetValues();\r\n"
                            "if (list == NULL)\r\nreturn;\r\n\r\nForeach(IPC::CObjectHolder, obj, list) {\r\n"
                            "ReleaseObjectHolder(obj, uri, TRUE);\r\n}\r\n}\r\n");
                        FWRITE("}\r\n\r\n");

                        FWRITE("private:\r\n\r\n");
                        FWRITE("AXP::Sp<IPC::CObjectHolder> CreateOrGetObjectHolder(\r\n"
                            "IN CONST AXP::Int32 code,\r\nIN CONST AXP::Sp<AXP::String> & uri,\r\n"
                            "IN CONST AXP::Sp<AXP::CParcel> & bundle,\r\nOUT AXP::Sp<AXP::CParcel> & parcel)\r\n");
                        FWRITE("{\r\n");
                        FWRITE("if ((mServiceList == NULL) || (bundle == NULL))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("Synchronized (&mServiceList->mLock) {\r\n");
                        FWRITE("AXP::Sp<IPC::CObjectHolder> obj;\r\n");
                        FWRITE("switch (code) {\r\n");
                        FWRITE("case IPC::CommandCode::COMMAND_CREATE:\r\n");

                        if ((tmp->GetChildren()->Get(0) == NULL) || (tmp->GetChildren()->Get(0)->GetValue() == NULL))
                            return;

                        if (tmp->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_Singleton) {
                            FWRITE("if (!mServiceList->Empty()) {\r\n");
                            FWRITE("AXP::Sp<AXP::List<IPC::CObjectHolder> > valueList = mServiceList->GetValues();\r\n");
                            FWRITE("if (valueList == NULL)\r\nreturn NULL;\r\n\r\n");
                            FWRITE("obj = valueList->Get(0);\r\n}\r\nelse {\r\n");
                            WRITE_STRING_TO_FILE(className->Length() + objectHolder->Length() + 77,
                                L"obj = Create%ls%ls();\r\n", (PCWStr)*className, (PCWStr)*objectHolder);
                            FWRITE("if (obj == NULL)\r\nreturn NULL;\r\n\r\n");
                            FWRITE("if (!mServiceList->InsertUnique((AXP::Int64)obj.Get(), obj))\r\nreturn NULL;\r\n}\r\n\r\n");
                        }
                        else {
                            WRITE_STRING_TO_FILE(className->Length() + objectHolder->Length() + 77,
                                L"obj = Create%ls%ls();\r\n", (PCWStr)*className, (PCWStr)*objectHolder);
                            FWRITE("if (obj == NULL)\r\nreturn NULL;\r\n\r\n");
                            FWRITE("if (!mServiceList->InsertUnique((AXP::Int64)obj.Get(), obj))\r\nreturn NULL;\r\n\r\n");
                        }

                        FWRITE("break;\r\n");
                        FWRITE("case IPC::CommandCode::COMMAND_CALLBACK:\r\ncase IPC::CommandCode::COMMAND_CALL:\r\n"
                            "case IPC::CommandCode::COMMAND_RELEASE:\r\n");
                        FWRITE("AXP::Int64 objRef;\r\nif (AXP::AFAILED(bundle->ReadInt64(objRef)))\r\nreturn NULL;\r\n\r\n");
                        FWRITE("obj = mServiceList->GetValue(objRef);\r\nif (obj == NULL) {\r\n"
                            "parcel = new AXP::CParcel();\r\nif (parcel == NULL)\r\nreturn NULL;\r\n\r\n"
                            "if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::RemoteRefException)))\r\nreturn NULL;\r\n\r\n"
                            "return NULL;\r\n}\r\n\r\n");
                        FWRITE("break;\r\ndefault:\r\nreturn NULL;\r\n}\r\n\r\n");
                        FWRITE("if ((code == IPC::CommandCode::COMMAND_CREATE) || (code == IPC::CommandCode::"
                            "COMMAND_CALLBACK)) {\r\nif (uri != NULL) {\r\nIPC::UriManager::StartThread(uri);\r\n"
                            "obj->mUriList->PushBack(uri);\r\n}\r\n\r\nobj->AddRemoteRef();\r\n}\r\n\r\n");
                        FWRITE("return obj;\r\n}\r\n\r\n");
                        FWRITE("return NULL;\r\n}\r\n\r\n");

                        FWRITE("AXP::ARESULT ReleaseObjectHolder(\r\nIN CONST AXP::Sp<IPC::CObjectHolder> & objectHolder,\r\n"
                            "IN CONST AXP::Sp<AXP::String> & uri,\r\nIN AXP::Boolean delAll) {\r\n");
                        FWRITE("if ((mServiceList == NULL) || (objectHolder == NULL))\r\nreturn AXP::AE_INVALIDARG;\r\n\r\n");
                        FWRITE("Synchronized(&mServiceList->mLock) {\r\n");
                        FWRITE("if (uri == NULL) {\r\nif (objectHolder->DecreaseRemoteRef() == 0) {\r\n");
                        FWRITE("if (!mServiceList->Remove((AXP::Int64)objectHolder.Get()))\r\nreturn AXP::AE_FAIL;\r\n"
                            "}\r\n}\r\nelse {\r\nForeach(AXP::String, obj, objectHolder->mUriList) {\r\n"
                            "if (uri->Equals(obj)) {\r\nobjectHolder->mUriList->Detach(obj);\r\n"
                            "if (objectHolder->DecreaseRemoteRef() == 0) {\r\nif (!mServiceList->Remove((AXP::Int64)objectHolder.Get()))\r\n"
                            "return AXP::AE_FAIL;\r\n}\r\n\r\nif (!delAll)\r\nbreak;\r\n}\r\n}\r\n}\r\n\r\n"
                            "return AXP::AS_OK;\r\n}\r\n\r\nreturn AXP::AE_FAIL;\r\n");
                        FWRITE("}\r\n\r\n");

                        FWRITE("private:\r\n\r\n");
                        FWRITE("AXP::Sp<AXP::HashTable<AXP::Int64, IPC::CObjectHolder> > mServiceList;\r\n");
                        FWRITE("};\r\n\r\n");

                        FWRITE("STATIC AXP::Boolean RegisterService()\r\n");
                        FWRITE("{\r\n");
                        WRITE_STRING_TO_FILE(namespacz->Length() * 2 + className->Length() + stub->Length() + 97,
                            L"return AXP::ASUCCEEDED(IPC::ServiceManager::RegisterService(L\"%ls.%ls\", new %ls::%ls%ls()));\r\n",
                            (PCWStr)*namespacz, (PCWStr)*className, (PCWStr)*namespacz, (PCWStr)*className, (PCWStr)*stub);
                        FWRITE("}\r\n\r\n");
                        FWRITE("STATIC AXP::Boolean sIsRegister = RegisterService();\r\n");
                    }
                }

                FWRITE("}\r\n");
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    STATIC Void WriteObjectHolderFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> modifier = String::Create(L"ObjectHolder");
        if (modifier == NULL)
            return;

        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*szNamespace);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + modifier->Length() + 15,
            L"%ls/%ls%ls.h", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix, (PCWStr)*modifier);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        Sp<String> macro = String::Create(fileNameWithoutSuffix->Length() + modifier->Length() + 15,
            L"__%ls_%ls_H__", (PCWStr)*fileNameWithoutSuffix, (PCWStr)*modifier);
        if (macro == NULL)
            return;

        for (PWStr pwstr = (PWStr)macro->GetPayload(); *pwstr; ++pwstr)
        if ((*pwstr >= 0x61) && (*pwstr <= 0x7a))
            *pwstr -= 0x20;

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#ifndef %ls\r\n", (PCWStr)*macro);
        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#define %ls\r\n\r\n", (PCWStr)*macro);

        FWRITE("#include \"IPC/cplusplus/lib/include/IpcException.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/ObjectHolder.h\"\r\n");

        Boolean hasObjectType = HasObjectTypeInInterface(node);
        if (hasObjectType)
            FWRITE("#include \"AXP/cplusplus/libc/include/Common/ClassLoader.h\"\r\n");

        WRITE_STRING_TO_FILE(szNamespace->Length() + fileNameWithoutSuffix->Length() + 77,
            L"#include \"IPC/gen/%ls/I%ls.h\"\r\n", (PCWStr)*szNamespace, (PCWStr)*fileNameWithoutSuffix);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if (((obj->GetChildren()->Get(0) == NULL)) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                WRITE_STRING_TO_FILE(namespacz->Length() + 22, L"namespace %ls\r\n{\r\n", (PCWStr)*namespacz);
                Sp<String> dump = String::Create(L"");
                if (dump == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + modifier->Length() + 50,
                            L"class %ls%ls : public IPC::CObjectHolder\r\n",
                            (PCWStr)*className, (PCWStr)*modifier);

                        FWRITE("{\r\n");
                        FWRITE("public:\r\n\r\n");

                        WriteObjectHolderOnTransact(tmp);

                        FWRITE("private:\r\n\r\n");
                        WRITE_STRING_TO_FILE(className->Length() + 50, L"AXP::Sp<_%ls> mService;\r\n", (PCWStr)*className);
                        WriteFunCodeMembers(tmp);
                        FWRITE("};\r\n");

                        dump = String::Create(dump->Length() + className->Length() * 2 + modifier->Length() * 2 + 104,
                            L"%ls\r\nAXP::Sp<IPC::CObjectHolder> Create%ls%ls()\r\n{\r\nreturn new %ls%ls();}\r\n",
                            (PCWStr)*dump, (PCWStr)*className, (PCWStr)*modifier, (PCWStr)*className, (PCWStr)*modifier);
                        if (dump == NULL)
                            return;
                    }
                }

                WRITE_STRING_TO_FILE(dump);
                FWRITE("}\r\n");
            }
        }

        WRITE_STRING_TO_FILE(macro->Length() + 50, L"\r\n#endif // %ls\r\n", (PCWStr)*macro);
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    STATIC Void WriteInterfaceFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/I%ls.h", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        Sp<String> macro = String::Create(fileNameWithoutSuffix->Length() + 17, L"__I%ls_H__", (PCWStr)*fileNameWithoutSuffix);
        if (macro == NULL)
            return;

        for (PWStr pwstr = (PWStr)macro->GetPayload(); *pwstr; ++pwstr)
        if ((*pwstr >= 0x61) && (*pwstr <= 0x7a))
            *pwstr -= 0x20;

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#ifndef %ls\r\n", (PCWStr)*macro);
        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#define %ls\r\n\r\n", (PCWStr)*macro);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class)) {
                        FWRITE("#include \"AXP/cplusplus/xplatform/include/parcelable.h\"\r\n");
                        break;
                    }
                }
            }
        }

        Foreach(String, tmpStr, gHeadListCplusplus) {
            WRITE_STRING_TO_FILE(tmpStr);
        }

        WriteIncludeFile(node);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                WRITE_STRING_TO_FILE(namespacz->Length() + 17, L"namespace %ls\r\n", (PCWStr)*namespacz);
                FWRITE("{\r\n");
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 77, L"class I%ls\r\n", (PCWStr)*className);
                        FWRITE("{\r\n");
                        FWRITE("public:\r\n\r\n");

                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                                if (returnType == NULL)
                                    return;

                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(2));
                                if (formParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length() + formParameterList->Length() + 27,
                                    L"VIRTUAL %ls %ls(%ls) = 0;\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                            }
                        }

                        FWRITE("};\r\n");
                    }
                    else if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class))
                        WriteClass(tmp);
                    else
                        continue;

                    if (!tmp->IsLastChild())
                        FWRITE("\r\n");
                }

                FWRITE("}\r\n");
            }
        }

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"\r\n#endif // %ls", (PCWStr)*macro);
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    STATIC Void WriteServiceHeadFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/_%ls.h", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        if (!ACCESS((PStr)fullFileName->GetBytes()->GetPayload(), 0))
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        Sp<String> macro = String::Create(namespacz->Length() + fileNameWithoutSuffix->Length() + 37, L"___%ls_%ls_H__",
            (PCWStr)*namespacz, (PCWStr)*fileNameWithoutSuffix);
        if (macro == NULL)
            return;

        for (PWStr pwstr = (PWStr)macro->GetPayload(); *pwstr; ++pwstr)
        if ((*pwstr >= 0x61) && (*pwstr <= 0x7a))
            *pwstr -= 0x20;

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#ifndef %ls\r\n", (PCWStr)*macro);
        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#define %ls\r\n\r\n", (PCWStr)*macro);

        Foreach(String, tmpStr, gHeadListCplusplus) {
            WRITE_STRING_TO_FILE(tmpStr);
        }

        FWRITE("#include \"AXP/cplusplus/xplatform/include/object.h\"\r\n");
        WRITE_STRING_TO_FILE(namespacz->Length() + fileNameWithoutSuffix->Length() + 50,
            L"#include \"./I%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                WRITE_STRING_TO_FILE(namespacz->Length() + 17, L"namespace %ls\r\n", (PCWStr)*namespacz);
                FWRITE("{\r\n");
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> originclassName = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (originclassName == NULL)
                            return;

                        Sp<String> className = String::Create(originclassName->Length() + 7, L"_%ls", (PCWStr)*originclassName);
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + originclassName->Length() + 97,
                            L"class %ls : public AXP::CObject, public I%ls\r\n", (PCWStr)*className, (PCWStr)*originclassName);
                        FWRITE("{\r\n");
                        FWRITE("public:\r\n\r\n");

                        Int32 count = 0;
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1));
                                if (formParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 77,
                                    L"STATIC AXP::Sp<%ls> Create(%ls);\r\n\r\n",
                                    (PCWStr)*className, (PCWStr)*formParameterList);

                                count++;
                            }
                        }

                        if (count == 0)
                            WRITE_STRING_TO_FILE(className->Length() + 77,
                            L"STATIC AXP::Sp<%ls> Create();\r\n\r\n", (PCWStr)*className);

                        if (count > 0) {
                            FWRITE("protected:\r\n\r\n");
                            Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                                if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                    Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1));
                                    if (formParameterList == NULL)
                                        return;

                                    WRITE_STRING_TO_FILE(className->Length() * 2 + formParameterList->Length() + 77,
                                        L"AXP::Sp<%ls> %ls(%ls);\r\n\r\n",
                                        (PCWStr)*className, (PCWStr)*className, (PCWStr)*formParameterList);
                                }
                            }
                        }

                        FWRITE("public:\r\n\r\n");
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                                if (returnType == NULL)
                                    return;

                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(2));
                                if (formParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length() + formParameterList->Length() + 37,
                                    L"%ls %ls(%ls);\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                            }
                        }

                        FWRITE("};\r\n");
                    }
                    else
                        continue;
                }

                FWRITE("}\r\n");
            }
        }

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"\r\n#endif // %ls", (PCWStr)*macro);
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    STATIC Void WriteServiceImplementFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/_%ls.cpp", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        if (!ACCESS((PStr)fullFileName->GetBytes()->GetPayload(), 0))
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 50,
            L"#include \"_%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);

        WRITE_STRING_TO_FILE(namespacz->Length() + fileNameWithoutSuffix->Length() + 87,
            L"#include \"IPC/gen/%ls/%lsObjectHolder.h\"\r\n", (PCWStr)*namespacz, (PCWStr)*fileNameWithoutSuffix);
        FWRITE("\r\n");

        FWRITE("using namespace AXP;\r\nusing namespace IPC;\r\n");
        WRITE_STRING_TO_FILE(namespacz->Length() + 77, L"using namespace %ls;\r\n\r\n", (PCWStr)*namespacz);
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        className = String::Create(className->Length() + 7, L"_%ls", (PCWStr)*className);
                        if (className == NULL)
                            return;

                        Int32 count = 0;
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                if ((var->GetChildren()->Get(0) == NULL) || (var->GetChildren()->Get(0)->GetValue() == NULL))
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1));
                                if (formParameterList == NULL)
                                    return;

                                Sp<String> realParameterList = GetRealParameters(var->GetChildren()->Get(1));
                                if (realParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(className->Length() * 3 + formParameterList->Length() + realParameterList->Length() + 97,
                                    L"AXP::Sp<%ls> %ls::Create(%ls) {\r\nreturn new %ls(%ls);\r\n}\r\n\r\n",
                                    (PCWStr)*className, (PCWStr)*className, (PCWStr)*formParameterList,
                                    (PCWStr)*className, (PCWStr)*realParameterList);

                                count++;
                            }
                        }

                        if (count == 0)
                            WRITE_STRING_TO_FILE(className->Length() * 3 + 77,
                            L"AXP::Sp<%ls> %ls::Create() {\r\nreturn new %ls();\r\n}\r\n\r\n",
                            (PCWStr)*className, (PCWStr)*className, (PCWStr)*className);

                        if (count > 0) {
                            Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                                if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                    if ((var->GetChildren()->Get(0) == NULL) || (var->GetChildren()->Get(0)->GetValue() == NULL))
                                        return;

                                    Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1));
                                    if (formParameterList == NULL)
                                        return;

                                    WRITE_STRING_TO_FILE(className->Length() * 3 + formParameterList->Length() + 97,
                                        L"AXP::Sp<%ls> %ls::%ls(%ls) {\r\n\r\n}\r\n\r\n",
                                        (PCWStr)*className, (PCWStr)*className, (PCWStr)*className, (PCWStr)*formParameterList);
                                }
                            }
                        }

                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                                if (var->GetChildren()->Get(0) == NULL)
                                    return;

                                Sp<CSymbol> retSymbol = var->GetChildren()->Get(0)->GetValue();
                                if (retSymbol == NULL)
                                    return;

                                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                                if (returnType == NULL)
                                    return;

                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(2));
                                if (formParameterList == NULL)
                                    return;

                                if (retSymbol->mSymbolType == SymbolType_Void)
                                    WRITE_STRING_TO_FILE(returnType->Length() + className->Length() + functionName->Length()
                                    + formParameterList->Length() + 77,
                                    L"%ls %ls::%ls(%ls)\r\n{\r\n\r\n}\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*className, (PCWStr)*functionName, (PCWStr)*formParameterList);
                                else
                                    WRITE_STRING_TO_FILE(returnType->Length() + className->Length() + functionName->Length()
                                    + formParameterList->Length() + 77,
                                    L"%ls %ls::%ls(%ls)\r\n{\r\nreturn 0;\r\n}\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*className, (PCWStr)*functionName, (PCWStr)*formParameterList);
                            }
                        }
                    }
                    else
                        continue;
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    STATIC Void WriteServiceFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        WriteServiceHeadFile(node, fileNameWithoutSuffix);
        WriteServiceImplementFile(node, fileNameWithoutSuffix);
    }

    STATIC Sp<String> GetBaseClassName(IN TreeNode<CSymbol> * node)
    {
        if ((node->GetChildren()->Get(2) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue() == NULL))
            return NULL;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        if (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue()->mContent == NULL)
            return String::Create(L"public AXP::IParcelable, public AXP::CObject");
        else {
            Sp<String> baseClassName = GetTypeReferenceList(node->GetChildren()->Get(2)->GetChildren()->Get(0), IDL_LANG_CPP);
            if (baseClassName == NULL)
                return NULL;

            return String::Create(baseClassName->Length() + 17, L"public %ls", (PCWStr)*baseClassName);
        }
    }

    STATIC Void WriteMembersToParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (scopeOfClass == NULL)
            return;

        FWRITE("AXP::ARESULT WriteToParcel(IN CONST AXP::Sp<AXP::CParcel> & parcel)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (parcel == NULL)\r\nreturn AXP::AE_FAIL;\r\n\r\n");
        WRITE_STRING_TO_FILE(scopeOfClass->Length() + 100,
            L"if (AXP::AFAILED(parcel->WriteString(AXP::String::Create(L\"%ls\"))))\r\nreturn AXP::AE_FAIL;\r\n\r\n",
            (PCWStr)*scopeOfClass);

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode
            && baseClassNode->GetChildren()->Get(0)
            && baseClassNode->GetChildren()->Get(0)->GetValue()
            && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);
                return;
            }

            Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_OBJC);
            if (baseClassName == NULL)
                return;

            if (symbol->mSymbolType == SymbolType_Class)
                WRITE_STRING_TO_FILE(baseClassName->Length() + 78,
                L"if (AXP::AFAILED(%ls::WriteToParcel(parcel)))\r\nreturn AXP::AE_FAIL;\r\n\r\n",
                (PCWStr)*baseClassName);
        }

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                WriteVarToParcel(obj);
                FWRITE("\r\n");
            }
        }

        FWRITE("return AXP::AS_OK;\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void ReadMembersFromParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (scopeOfClass == NULL)
            return;

        FWRITE("AXP::ARESULT ReadFromParcel(IN CONST AXP::Sp<AXP::CParcel> & parcel)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (parcel == NULL)\r\nreturn AXP::AE_FAIL;\r\n\r\n");
        FWRITE("AXP::Sp<AXP::String> className;\r\n");
        FWRITE("if (AXP::AFAILED(parcel->ReadString(className)))\r\nreturn AXP::AE_FAIL;\r\n\r\n");

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode
            && baseClassNode->GetChildren()->Get(0)
            && baseClassNode->GetChildren()->Get(0)->GetValue()
            && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);
                return;
            }

            Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_OBJC);
            if (baseClassName == NULL)
                return;

            if (symbol->mSymbolType == SymbolType_Class)
                WRITE_STRING_TO_FILE(baseClassName->Length() + 78,
                L"if (AXP::AFAILED(%ls::ReadFromParcel(parcel)))\r\nreturn AXP::AE_FAIL;\r\n\r\n", (PCWStr)*baseClassName);
        }

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member)
                ReadVarFromParcel(obj);
        }

        FWRITE("return AXP::AS_OK;\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void WriteCopyFunction(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        WRITE_STRING_TO_FILE(className->Length() + 50, L"AXP::Void Copy(IN CONST AXP::Sp<%ls> & obj)\r\n", (PCWStr)*className);
        FWRITE("{\r\n");
        FWRITE("if (obj == NULL)\r\nreturn;\r\n\r\n");

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode
            && baseClassNode->GetChildren()->Get(0)
            && baseClassNode->GetChildren()->Get(0)->GetValue()
            && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);
                return;
            }

            Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_OBJC);
            if (baseClassName == NULL)
                return;

            if (symbol->mSymbolType == SymbolType_Class)
                WRITE_STRING_TO_FILE(baseClassName->Length() + 23, L"%ls::Copy(obj);\r\n\r\n", (PCWStr)*baseClassName);
        }

        Int32 index = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            index++;
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType != SymbolType_Member)
                continue;

            Sp<String> varId = GetVarId(obj);
            if (varId == NULL)
                return;

            if (obj->GetChildren()->Get(0) == NULL)
                return;

            Sp<CSymbol> symbol = obj->GetChildren()->Get(0)->GetValue();
            if (symbol == NULL)
                return;

            if (symbol->mSymbolType == SymbolType_List) {
                Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                if (varType == NULL)
                    return;

                if (obj->GetChildren()->Get(0)->GetChildren()->Get(0) == NULL)
                    return;

                Sp<CSymbol> elementSymbol = obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue();
                if (elementSymbol == NULL)
                    return;

                Sp<String> elementType = GetVarType(obj->GetChildren()->Get(0)->GetChildren()->Get(0),
                    IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                if (elementType == NULL)
                    return;

                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (obj->%ls != NULL) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 40,
                    L"%ls = new %ls();\r\n", (PCWStr)*varId, (PCWStr)*varType);
                WRITE_STRING_TO_FILE(varId->Length() + 36, L"if (%ls == NULL)\r\nreturn;\r\n\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 50, L"Foreach (%ls, var, obj->%ls) {\r\n",
                    (PCWStr)*elementType, (PCWStr)*varId);

                if (elementSymbol->mSymbolType == SymbolType_TypeReference) {
                    WRITE_STRING_TO_FILE(elementType->Length() * 2 + 39, L"AXP::Sp<%ls> tmp = new %ls();\r\n",
                        (PCWStr)*elementType, (PCWStr)*elementType);
                    FWRITE("if (tmp == NULL)\r\nreturn;\r\n\r\n");
                    FWRITE("tmp->Copy(var);\r\n");
                }
                else if (elementSymbol->mSymbolType == SymbolType_String) {
                    FWRITE("AXP::Sp<AXP::String> tmp = AXP::String::Create(var);\r\n");
                    FWRITE("if (tmp == NULL)\r\nreturn;\r\n\r\n");
                }
                else if (elementSymbol->mSymbolType == SymbolType_ByteArray) {
                    FWRITE("AXP::Sp<AXP::ByteArray> tmp = var->Clone();\r\n");
                    FWRITE("if (tmp == NULL)\r\nreturn;\r\n\r\n");
                }
                else if ((elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_Int16_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_Int32_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_Int64_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_UInt8_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_UInt16_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_UInt32_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_UInt64_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_Float_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_Double_NULL)
                    || (elementSymbol->mSymbolType == SymbolType_Boolean_NULL)) {
                    WRITE_STRING_TO_FILE(elementType->Length() * 2 + 61,
                        L"AXP::Sp<%ls> tmp = new %ls(var->GetValue());\r\n", (PCWStr)*elementType, (PCWStr)*elementType);
                    FWRITE("if (tmp == NULL)\r\nreturn;\r\n\r\n");
                }
                else {
                    DEBUG_PRINT(L"not support list type!");

                    return;
                }

                WRITE_STRING_TO_FILE(varId->Length() + 39, L"if (!%ls->PushBack(tmp))\r\nreturn;\r\n", (PCWStr)*varId);
                FWRITE("}\r\n");
                FWRITE("}\r\nelse {\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 24, L"%ls = NULL;\r\n}\r\n", (PCWStr)*varId);
                if (index < node->GetChildren()->GetCount() - 1)
                    FWRITE("\r\n");
            }
            else if (symbol->mSymbolType == SymbolType_TypeReference) {
                Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                if (varType == NULL)
                    return;

                WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (obj->%ls != NULL) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 37, L"%ls = new %ls();\r\n", (PCWStr)*varId, (PCWStr)*varType);
                WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == NULL)\r\nreturn;\r\n\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 37, L"%ls->Copy(obj->%ls);\r\n", (PCWStr)*varId, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() + 37, L"}\r\nelse {\r\n%ls = NULL;}\r\n", (PCWStr)*varId);
                if (index < node->GetChildren()->GetCount() - 1)
                    FWRITE("\r\n");
            }
            else
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 20, L"%ls = obj->%ls;\r\n", (PCWStr)*varId, (PCWStr)*varId);
        }

        FWRITE("}\r\n");
    }

    STATIC Void WriteSetNullFunction(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("AXP::Void SetNull()\r\n");
        FWRITE("{\r\n");

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode
            && baseClassNode->GetChildren()->Get(0)
            && baseClassNode->GetChildren()->Get(0)->GetValue()
            && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);
                return;
            }

            Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_OBJC);
            if (baseClassName == NULL)
                return;

            if (symbol->mSymbolType == SymbolType_Class)
                WRITE_STRING_TO_FILE(baseClassName->Length() + 23, L"%ls::SetNull();\r\n\r\n", (PCWStr)*baseClassName);
        }

        Int32 index = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            index++;
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType != SymbolType_Member)
                continue;

            Sp<String> varId = GetVarId(obj);
            if (varId == NULL)
                return;

            if (obj->GetChildren()->Get(0) == NULL)
                return;

            Sp<CSymbol> symbol = obj->GetChildren()->Get(0)->GetValue();
            if (symbol == NULL)
                return;

            if ((symbol->mSymbolType == SymbolType_List)
                || (symbol->mSymbolType == SymbolType_ByteArray)
                || (symbol->mSymbolType == SymbolType_String)) {
                WRITE_STRING_TO_FILE(varId->Length() + 15, L"%ls = NULL;\r\n", (PCWStr)*varId);
            }
            else if (symbol->mSymbolType == SymbolType_TypeReference) {
                WRITE_STRING_TO_FILE(varId->Length() + 20, L"if (%ls != NULL)\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() + 23, L"%ls->SetNull();\r\n", (PCWStr)*varId);
                if (index < node->GetChildren()->GetCount() - 1)
                    FWRITE("\r\n");
            }
            else if ((symbol->mSymbolType == SymbolType_Int8_NULL)
                || (symbol->mSymbolType == SymbolType_Int16_NULL)
                || (symbol->mSymbolType == SymbolType_Int32_NULL)
                || (symbol->mSymbolType == SymbolType_Int64_NULL)
                || (symbol->mSymbolType == SymbolType_UInt8_NULL)
                || (symbol->mSymbolType == SymbolType_UInt16_NULL)
                || (symbol->mSymbolType == SymbolType_UInt32_NULL)
                || (symbol->mSymbolType == SymbolType_UInt64_NULL)
                || (symbol->mSymbolType == SymbolType_Float_NULL)
                || (symbol->mSymbolType == SymbolType_Double_NULL)
                || (symbol->mSymbolType == SymbolType_Boolean_NULL)) {
                WRITE_STRING_TO_FILE(varId->Length() + 18, L"%ls.SetNull();\r\n", (PCWStr)*varId);
            }
            else {
                WRITE_STRING_TO_FILE(varId->Length() + 12, L"%ls = 0;\r\n", (PCWStr)*varId);
            }
        }

        FWRITE("}\r\n");
    }

    STATIC Void WriteToString(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("VIRTUAL AXP::Sp<AXP::String> ToString()\r\n");
        FWRITE("{\r\n");

        Int32 count = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member)
                ++count;
        }

        if (count <= 0)
            return;

        if (count == 1) {
            Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
                if (obj->GetValue() == NULL)
                    return;

                if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                    Sp<CSymbol> symbol = GetVarSymbol(obj);
                    if (symbol == NULL)
                        return;

                    Sp<String> varId = GetVarId(obj);
                    if (varId == NULL)
                        return;

                    if (symbol->mSymbolType == SymbolType_Class) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == NULL)\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 58, L"return %ls->ToString();\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_List) {
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (%ls == NULL)\r\nreturn NULL;\r\n\r\n", (PCWStr)*varId);
                        FWRITE("AXP::Sp<AXP::String> json = AXP::String::Create(L\"[\");\r\n");
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"for(AXP::Int32 i = 0; i < %ls->GetCount(); ++i) {\r\n", (PCWStr)*varId);

                        Sp<CSymbol> elementSymbol = GetVarSymbol(obj->GetChildren()->Get(0));
                        if (elementSymbol == NULL)
                            return;

                        Sp<String> elementType = GetVarType(obj->GetChildren()->Get(0)->GetChildren()->Get(0),
                            IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 47, L"AXP::Sp<%ls> obj = (*%ls)[i];\r\n",
                            (PCWStr)*elementType, (PCWStr)*varId);
                        FWRITE("AXP::PCWStr comma;\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (i < %ls->GetCount() - 1)\r\n", (PCWStr)*varId);
                        FWRITE("comma = L\",\";\r\nelse\r\ncomma = L\"\";\r\n\r\n");

                        if (elementSymbol->mSymbolType == SymbolType_Class) {
                            Boolean isComplex;
                            if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0)->GetChildren()->Get(0), isComplex)))
                                return;

                            FWRITE("if (obj == NULL)\r\nreturn NULL;\r\n\r\n");
                            FWRITE("AXP::Sp<AXP::String> str = obj->ToString();\r\n");
                            FWRITE("if (str == NULL)\r\nreturn NULL;\r\n\r\n");

                            if (isComplex)
                                FWRITE("json = AXP::String::Create(json->Length() + str->Length() + 39,"
                                " L\"%ls%ls%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*str, comma);\r\n");
                            else
                                FWRITE("json = AXP::String::Create(json->Length() + str->Length() + 39,"
                                " L\"%ls\\\"%ls\\\"%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*str, comma);\r\n");

                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_String) {
                            FWRITE("if (obj == NULL)\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + obj->Length() + 39,"
                                " L\"%ls\\\"%ls\\\"%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*obj, comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_ByteArray) {

                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Int16_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Int32_NULL)) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 47,"
                                " L\"%ls\\\"%d\\\"%ls\", (AXP::PCWStr)*json, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_UInt8_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_UInt16_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_UInt32_NULL)) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 47,"
                                " L\"%ls\\\"%u\\\"%ls\", (AXP::PCWStr)*json, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Int64_NULL) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 47,"
                                " L\"%ls\\\"%lld\\\"%ls\", (AXP::PCWStr)*json, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_UInt64_NULL) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 47,"
                                " L\"%ls\\\"%llu\\\"%ls\", (AXP::PCWStr)*json, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_Float_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Double_NULL)) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 47,"
                                " L\"%ls\\\"%.2f\\\"%ls\", (AXP::PCWStr)*json, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Boolean_NULL) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("json = AXP::String::Create(json->Length() + 47,"
                                " L\"%ls\\\"%ls\\\"%ls\", (AXP::PCWStr)*json, obj->GetValue() ? L\"true\" : L\"false\", comma);\r\n\r\n");
                            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n");
                        }
                        else {
                            DEBUG_PRINT("not support list type!");
                            return;
                        }

                        FWRITE("}\r\n\r\n");
                        FWRITE("return AXP::String::Create(json->Length() + 7, L\"%ls]\", (AXP::PCWStr)*json);\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == NULL)\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 58, L"return %ls;\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_ByteArray) {

                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8_NULL)
                        || (symbol->mSymbolType == SymbolType_Byte_NULL)
                        || (symbol->mSymbolType == SymbolType_Int16_NULL)
                        || (symbol->mSymbolType == SymbolType_Int32_NULL)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue())\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"return AXP::String::Create(16, L\"%%d\", %ls.GetValue());\r\n", (PCWStr)*varId);
                    }
                    else if ((symbol->mSymbolType == SymbolType_UInt8_NULL)
                        || (symbol->mSymbolType == SymbolType_UInt16_NULL)
                        || (symbol->mSymbolType == SymbolType_UInt32_NULL)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue())\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"return AXP::String::Create(16, L\"%%u\", %ls.GetValue());\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_Int64_NULL) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue())\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"return AXP::String::Create(32, L\"%%lld\", %ls.GetValue());\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_UInt64_NULL) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue())\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87, L"return AXP::String::Create(32, L\"%%llu\","
                            " %ls.GetValue());\r\n", (PCWStr)*varId);
                    }
                    else if ((symbol->mSymbolType == SymbolType_Float_NULL)
                        || (symbol->mSymbolType == SymbolType_Double_NULL)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue())\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87, L"return AXP::String::Create(32, L\"%%.2f\","
                            " %ls.GetValue());\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_Boolean_NULL) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue())\r\n", (PCWStr)*varId);
                        FWRITE("return AXP::String::Create(L\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87, L"return AXP::String::Create(7, L\"%%ls\","
                            " %ls.GetValue() ? L\"true\" : L\"false\");\r\n", (PCWStr)*varId);
                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8)
                        || (symbol->mSymbolType == SymbolType_Byte)
                        || (symbol->mSymbolType == SymbolType_Int16)
                        || (symbol->mSymbolType == SymbolType_Int32))
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return AXP::String::Create(16, L\"%%d\", %ls);\r\n", (PCWStr)*varId);
                    else if (symbol->mSymbolType == SymbolType_Int64)
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return AXP::String::Create(32, L\"%%lld\", %ls);\r\n", (PCWStr)*varId);
                    else if ((symbol->mSymbolType == SymbolType_UInt8)
                        || (symbol->mSymbolType == SymbolType_UInt16)
                        || (symbol->mSymbolType == SymbolType_UInt32))
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return AXP::String::Create(16, L\"%%u\", %ls);\r\n", (PCWStr)*varId);
                    else if (symbol->mSymbolType == SymbolType_UInt64)
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return AXP::String::Create(32, L\"%%llu\", %ls);\r\n", (PCWStr)*varId);
                    else if ((symbol->mSymbolType == SymbolType_Float)
                        || (symbol->mSymbolType == SymbolType_Double))
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return AXP::String::Create(315, L\"%%.2f\", %ls);\r\n", (PCWStr)*varId);
                    else if (symbol->mSymbolType == SymbolType_Boolean)
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return AXP::String::Create(7, L\"%%ls\", %ls ? L\"true\" : L\"false\");\r\n", (PCWStr)*varId);
                    else {
                        DEBUG_PRINT("not support type!");
                        return;
                    }
                }
            }
        }
        else {
            Int32 index = 0;
            FWRITE("AXP::Sp<AXP::String> json = AXP::String::Create(L\"{\");\r\n");
            FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");

            Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
                if (obj->GetValue() == NULL)
                    return;

                if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                    Sp<CSymbol> symbol = GetVarSymbol(obj);
                    if (symbol == NULL)
                        return;

                    Sp<String> varId = GetVarId(obj);
                    if (varId == NULL)
                        return;

                    PCWStr comma;
                    if (++index < count)
                        comma = L",";
                    else
                        comma = L"";

                    if (symbol->mSymbolType == SymbolType_Class) {
                        Boolean isComplex;
                        if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0), isComplex)))
                            return;

                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == NULL) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":{}%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"AXP::Sp<AXP::String> str = %ls->ToString();\r\n", (PCWStr)*varId);
                        FWRITE("if (str == NULL)\r\nreturn NULL;\r\n\r\n");
                        if (isComplex)
                            WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + str->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":%%ls%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*str);\r\n", (PCWStr)*varId, comma);
                        else
                            WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + str->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%ls\\\"%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*str);\r\n", (PCWStr)*varId, comma);

                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_List) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == NULL) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":[]%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"AXP::Sp<AXP::String> jsonTmp = AXP::String::Create(L\"\\\"%ls\\\":[\");\r\n", (PCWStr)*varId);
                        FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"for(AXP::Int32 i = 0; i < %ls->GetCount(); ++i) {\r\n", (PCWStr)*varId);

                        Sp<CSymbol> elementSymbol = GetVarSymbol(obj->GetChildren()->Get(0));
                        if (elementSymbol == NULL)
                            return;

                        Sp<String> elementType = GetVarType(obj->GetChildren()->Get(0)->GetChildren()->Get(0),
                            IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_NOMODIFY);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 47, L"AXP::Sp<%ls> obj = (*%ls)[i];\r\n",
                            (PCWStr)*elementType, (PCWStr)*varId);
                        FWRITE("AXP::PCWStr comma;\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (i < %ls->GetCount() - 1)\r\n", (PCWStr)*varId);
                        FWRITE("comma = L\",\";\r\nelse\r\ncomma = L\"\";\r\n\r\n");

                        if (elementSymbol->mSymbolType == SymbolType_Class) {
                            Boolean isComplex;
                            if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0)->GetChildren()->Get(0), isComplex)))
                                return;

                            FWRITE("if (obj == NULL)\r\nreturn NULL;\r\n\r\n");
                            FWRITE("AXP::Sp<AXP::String> str = obj->ToString();\r\n");
                            FWRITE("if (str == NULL)\r\nreturn NULL;\r\n\r\n");

                            if (isComplex)
                                FWRITE("jsonTmp = AXP::String::Create(json->Length() + str->Length() + 39,"
                                " L\"%ls%ls%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*str, comma);\r\n");
                            else
                                FWRITE("jsonTmp = AXP::String::Create(json->Length() + str->Length() + 39,"
                                " L\"%ls\\\"%ls\\\"%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*str, comma);\r\n");

                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_String) {
                            FWRITE("if (obj == NULL)\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(json->Length() + 39,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*json, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + obj->Length() + 39,"
                                " L\"%ls\\\"%ls\\\"%ls\", (AXP::PCWStr)*jsonTmp, i, (AXP::PCWStr)*obj, comma);\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_ByteArray) {

                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Int16_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Int32_NULL)) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*jsonTmp, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"%d\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_UInt8_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_UInt16_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_UInt32_NULL)) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*jsonTmp, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"%u\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Int64_NULL) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*jsonTmp, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"%lld\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");

                        }
                        else if (elementSymbol->mSymbolType == SymbolType_UInt64_NULL) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*jsonTmp, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"%llu\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_Float_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Double_NULL)) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"%.2f\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);\r\n\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Boolean_NULL) {
                            FWRITE("if ((obj == NULL) || (!obj->HasValue()))\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"\\\"%ls\", (AXP::PCWStr)*jsonTmp, comma);\r\n");
                            FWRITE("else\r\n");
                            FWRITE("jsonTmp = AXP::String::Create(jsonTmp->Length() + 47,"
                                " L\"%ls\\\"%ls\\\"%ls\", (AXP::PCWStr)*jsonTmp, obj->GetValue() ? L\"true\" : L\"false\", comma);\r\n\r\n");
                            FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n");
                        }
                        else {
                            DEBUG_PRINT("not support list type!");
                            return;
                        }

                        FWRITE("}\r\n\r\n");
                        WRITE_STRING_TO_FILE(117, L"jsonTmp = AXP::String::Create(jsonTmp->Length() + 7,"
                            " L\"%%ls]%ls\", (AXP::PCWStr)*jsonTmp);\r\n", comma);
                        FWRITE("if (jsonTmp == NULL)\r\nreturn NULL;\r\n\r\n");
                        FWRITE("json = AXP::String::Create(json->Length() + jsonTmp->Length() + 2, L\"%ls%ls\","
                            " (AXP::PCWStr)*json, (AXP::PCWStr)*jsonTmp);\r\n");
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == NULL) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 3 + 157, L"json = AXP::String::Create(json->Length() + %ls->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%ls\\\"%ls\", (AXP::PCWStr)*json, (AXP::PCWStr)*%ls);\r\n",
                            (PCWStr)*varId, (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_ByteArray) {

                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8_NULL)
                        || (symbol->mSymbolType == SymbolType_Int16_NULL)
                        || (symbol->mSymbolType == SymbolType_Int32_NULL)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue()) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 64,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%d\\\"%ls\", (AXP::PCWStr)*json, %ls.GetValue());\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if ((symbol->mSymbolType == SymbolType_UInt8_NULL)
                        || (symbol->mSymbolType == SymbolType_UInt16_NULL)
                        || (symbol->mSymbolType == SymbolType_UInt32_NULL)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue()) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 64,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%u\\\"%ls\", (AXP::PCWStr)*json, %ls.GetValue());\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_Int64_NULL) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue()) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 64,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%lld\\\"%ls\", (AXP::PCWStr)*json, %ls.GetValue());\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_UInt64_NULL) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue()) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 64,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%llu\\\"%ls\", (AXP::PCWStr)*json, %ls.GetValue());\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if ((symbol->mSymbolType == SymbolType_Float_NULL)
                        || (symbol->mSymbolType == SymbolType_Double_NULL)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue()) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 67,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%.2f\\\"%ls\", (AXP::PCWStr)*json, %ls.GetValue());\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_Boolean_NULL) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (!%ls.HasValue()) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = AXP::String::Create(json->Length() + 40,"
                            " L\"%%ls\\\"%ls\\\":\\\"\\\"%ls\", (AXP::PCWStr)*json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 64,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%ls\\\"%ls\", (AXP::PCWStr)*json, %ls.GetValue() ? L\"true\" : L\"false\");\r\n",
                            (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n}\r\n\r\n");
                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8)
                        || (symbol->mSymbolType == SymbolType_Int16)
                        || (symbol->mSymbolType == SymbolType_Int32)) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 47,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%d\\\"%ls\","
                            " (AXP::PCWStr)*json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_Int64) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 47,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%lld\\\"%ls\","
                            " (AXP::PCWStr)*json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                    }
                    else if ((symbol->mSymbolType == SymbolType_UInt8)
                        || (symbol->mSymbolType == SymbolType_UInt16)
                        || (symbol->mSymbolType == SymbolType_UInt32)) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = AXP::String::Create(json->Length() + 47,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%u\\\"%ls\", (AXP::PCWStr)*json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_UInt64) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 117, L"json = AXP::String::Create(json->Length() + 47,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%llu\\\"%ls\", (AXP::PCWStr)*json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                    }
                    else if ((symbol->mSymbolType == SymbolType_Float)
                        || (symbol->mSymbolType == SymbolType_Double)) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 117, L"json = AXP::String::Create(json->Length() + 315,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%.2f\\\"%ls\", (AXP::PCWStr)*json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_Boolean) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 117, L"json = AXP::String::Create(json->Length() + 47,"
                            " L\"%%ls\\\"%ls\\\":\\\"%%ls\\\"%ls\", (AXP::PCWStr)*json, %ls ? L\"true\" : L\"false\");\r\n",
                            (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == NULL)\r\nreturn NULL;\r\n\r\n");
                    }
                    else {
                        DEBUG_PRINT("not support type!");
                        return;
                    }
                }
            } //Foreach

            FWRITE("return AXP::String::Create(json->Length() + 7, L\"%ls}\", (AXP::PCWStr)*json);\r\n");
        }

        FWRITE("}\r\n");
    }

    STATIC Void WriteClass(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CPP);
        if (scopeOfClass == NULL)
            return;

        if (gShareAreaCpp) {
            Int32 length = 0;
            Wcscpy_s(gShareAreaCpp->mDescription, 256, (PCWStr)*description, &length);
            Wcscpy_s(gShareAreaCpp->mScope, 256, (PCWStr)*scopeOfClass, &length);
            gShareAreaCpp++;
        }
        else {
            Sp<CMappingInfo> mappingInfo = new CMappingInfo();
            if (!mappingInfo)
                return;

            mappingInfo->mDescription = description;
            mappingInfo->mScope = scopeOfClass;
            if (!gMappingCpp->InsertUnique((PCWStr)description->GetPayload(), mappingInfo))
                return;
        }

        for (Int32 i = 1; i < node->GetParent()->GetChildren()->GetCount(); ++i) {
            if (node == node->GetParent()->GetChildren()->Get(i)) {
                if (node->GetParent()->GetChildren()->Get(i - 1)
                    && node->GetParent()->GetChildren()->Get(i - 1)->GetValue()
                    && ((node->GetParent()->GetChildren()->Get(i - 1)->GetValue()->mSymbolType == SymbolType_COMMENT1)
                    || (node->GetParent()->GetChildren()->Get(i - 1)->GetValue()->mSymbolType == SymbolType_COMMENT2))) {
                    Sp<String> comment = String::Create(node->GetParent()->GetChildren()->Get(i - 1)->GetValue()->mContent);
                    if (comment == NULL)
                        return;

                    WRITE_STRING_TO_FILE(comment);
                    FWRITE("\r\n");
                }

                break;
            }
        }

        Sp<String> baseClassName = GetBaseClassName(node);
        if (baseClassName == NULL)
            return;

        if ((node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        WRITE_STRING_TO_FILE(className->Length() + baseClassName->Length() + 37, L"class %ls : %ls\r\n",
            (PCWStr)*className, (PCWStr)*baseClassName);
        FWRITE("{\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                FWRITE("public:\r\n\r\n");
                WriteClass(obj);
                FWRITE("\r\n");
            }
        }

        FWRITE("public:\r\n\r\n");
        WriteMembersToParcel(node);
        FWRITE("\r\n");
        ReadMembersFromParcel(node);
        FWRITE("\r\n");
        //        WriteCopyFunction(node);
        //        FWRITE("\r\n");
        //        WriteSetNullFunction(node);
        //        FWRITE("\r\n");
        WriteToString(node);
        FWRITE("\r\n");

        FWRITE("VIRTUAL AXP::Sp<AXP::String> GetTypeName()\r\n");
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(description->Length() + 87, L"return AXP::String::Create(L\"%ls\");\r\n", (PCWStr)*description);
        FWRITE("}\r\n\r\n");

        FWRITE("public:\r\n\r\n");
        FWRITE("STATIC AXP::Sp<AXP::CObject> Create()\r\n");
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 27, L"return new %ls();\r\n", (PCWStr)*className);
        FWRITE("}\r\n\r\n");

        FWRITE("public:\r\n\r\n");
        for (Int32 i = 3; i < node->GetChildren()->GetCount(); ++i) {
            TreeNode<CSymbol> * var = node->GetChildren()->Get(i);
            if (var == NULL)
                return;

            if (var->GetValue()->mSymbolType == SymbolType_Member) {
                if (node->GetChildren()->Get(i - 1)->GetValue()
                    && ((node->GetChildren()->Get(i - 1)->GetValue()->mSymbolType == SymbolType_COMMENT1)
                    || (node->GetChildren()->Get(i - 1)->GetValue()->mSymbolType == SymbolType_COMMENT1))) {
                    Sp<String> comment = String::Create(node->GetChildren()->Get(i - 1)->GetValue()->mContent);
                    if (comment == NULL)
                        return;

                    WRITE_STRING_TO_FILE(comment);
                    FWRITE("\r\n");
                }

                Sp<String> varType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                if (varType == NULL)
                    return;

                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = var->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 12, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
            }
        }

        FWRITE("};\r\n");
    }

    STATIC Void WriteClassNameInsertMappingTable(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> varName = GetScopeChainOfClassName(node, IDL_NOP);
        if (varName == NULL)
            return;

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CPP);
        if (scopeOfClass == NULL)
            return;

        WRITE_STRING_TO_FILE(varName->Length() + description->Length() + scopeOfClass->Length() + 97,
            L"STATIC AXP::Boolean _%ls_ = AXP::Libc::Common::ClassLoader::RegisterClassCreator(L\"%ls\", %ls::Create);\r\n",
            (PCWStr)*varName, (PCWStr)*description, (PCWStr)*scopeOfClass);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class)
                WriteClassNameInsertMappingTable(obj);
        }
    }

    STATIC Void WriteModelsFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        gNamespacz = namespacz;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/%ls.h", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        Sp<String> macro = String::Create(fileNameWithoutSuffix->Length() + 17, L"__%ls_H__", (PCWStr)*fileNameWithoutSuffix);
        if (macro == NULL)
            return;

        for (PWStr pwstr = (PWStr)macro->GetPayload(); *pwstr; ++pwstr)
        if ((*pwstr >= 0x61) && (*pwstr <= 0x7a))
            *pwstr -= 0x20;

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#ifndef %ls\r\n", (PCWStr)*macro);
        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#define %ls\r\n\r\n", (PCWStr)*macro);

        Foreach(String, tmpStr, gHeadListCplusplus) {
            WRITE_STRING_TO_FILE(tmpStr);
        }
        FWRITE("#include \"AXP/cplusplus/xplatform/include/Parcel.h\"\r\n");
        FWRITE("#include \"AXP/cplusplus/xplatform/include/parcelable.h\"\r\n");
        FWRITE("#include \"AXP/cplusplus/libc/include/Common/ClassLoader.h\"\r\n");

        WriteIncludeFile(node);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                WRITE_STRING_TO_FILE(namespacz->Length() + 17, L"namespace %ls\r\n", (PCWStr)*namespacz);
                FWRITE("{\r\n");

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        WriteClass(tmp);
                        FWRITE("\r\n");
                    }
                }

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class)
                        WriteClassNameInsertMappingTable(tmp);
                }

                FWRITE("}\r\n");
            }
        }

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#endif // %ls", (PCWStr)*macro);
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    STATIC Void InsertMappingTable(IN TreeNode<CSymbol> * node)
    {
        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CPP);
        if (scopeOfClass == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        WRITE_STRING_TO_FILE(description->Length() + scopeOfClass->Length() + 77,
            L"if (!mappingTable->InsertUnique(L\"%ls\", %ls::Create))\r\nreturn NULL;\r\n\r\n",
            (PCWStr)*description, (PCWStr)*scopeOfClass);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class)
                InsertMappingTable(obj);
        }
    }

    STATIC Void WriteInitMappingTableFunction(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_FileBegin) {
                WriteInitMappingTableFunction(obj);
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        InsertMappingTable(tmp);
                    }
                }
            }
        }
    }

    STATIC Void WriteProxyFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*szNamespace);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/%ls.h", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        Sp<String> macro = String::Create(fileNameWithoutSuffix->Length() + 17, L"__%ls_H__", (PCWStr)*fileNameWithoutSuffix);;
        if (macro == NULL)
            return;

        for (PWStr pwstr = (PWStr)macro->GetPayload(); *pwstr; ++pwstr)
        if ((*pwstr >= 0x61) && (*pwstr <= 0x7a))
            *pwstr -= 0x20;

        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#ifndef %ls\r\n", (PCWStr)*macro);
        WRITE_STRING_TO_FILE(macro->Length() + 30, L"#define %ls\r\n\r\n", (PCWStr)*macro);
        FWRITE("#include \"IPC/cplusplus/lib/include/ServerConnection.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/IpcException.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/ServiceManager.h\"\r\n");
        WRITE_STRING_TO_FILE(szNamespace->Length() + fileNameWithoutSuffix->Length() + 50,
            L"#include \"./I%ls.h\"\r\n",
            (PCWStr)*szNamespace, (PCWStr)*fileNameWithoutSuffix);

        Boolean hasObjectType = HasObjectTypeInInterface(node);
        if (hasObjectType)
            FWRITE("#include \"AXP/cplusplus/libc/include/Common/ClassLoader.h\"\r\n");

        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                WRITE_STRING_TO_FILE(namespacz->Length() + 17, L"namespace %ls", (PCWStr)*namespacz);
                FWRITE("\r\n{\r\n");

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() * 2 + 87,
                            L"class %ls : public AXP::CObject, public I%ls\r\n",
                            (PCWStr)*className, (PCWStr)*className);
                        FWRITE("{\r\npublic:\r\n\r\n");

                        WriteProxyCreates(tmp);
                        WriteProxyConstructions(tmp);
                        WriteProxyDestruction(tmp);
                        WriteProxyFunctions(tmp);
                        WriteProxyGetRemoteRef(tmp);

                        FWRITE("AXP::Void AddRemoteRef(IN CONST AXP::Sp<AXP::String> & uri)\r\n"
                            "{\r\nmConn->AddRemoteRef(uri, mRef);\r\n}\r\n\r\n");
                        WRITE_STRING_TO_FILE(className->Length() + 27, L"I%ls * GetInterface()\r\n", (PCWStr)*className);
                        FWRITE("{\r\nreturn mInterface;\r\n}\r\n\r\n");

                        WriteProxyMembers(tmp);
                        FWRITE("};\r\n");
                    }
                }

                FWRITE("}\r\n");
            }
        }

        WRITE_STRING_TO_FILE(macro->Length() + 50, L"\r\n#endif // %ls\r\n", (PCWStr)*macro);
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CPP);
    }

    Void WriteCppFile(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        Sp<String> fileNameWithoutSuffix = GetOrignalFileName();
        if (fileNameWithoutSuffix == NULL)
            return;

        pthread_key_create(&sKey, NULL);
        Int32 modeFlag = GetMode(node);

        if (modeFlag & IDL_MODE_IPC) {
            WriteProxyFile(node, fileNameWithoutSuffix);
            WriteObjectHolderFile(node, fileNameWithoutSuffix);
            WriteStubFile(node, fileNameWithoutSuffix);
            WriteInterfaceFile(node, fileNameWithoutSuffix);
            WriteServiceFile(node, fileNameWithoutSuffix);
        }
        else if (modeFlag & IDL_MODE_MODELS)
            WriteModelsFile(node, fileNameWithoutSuffix);

        if (gIDLFlag & IDL_GENERATE_INCLUDE_FILE) {
            Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
                if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_FileBegin)) {
                    Sp<String> fileName = obj->GetValue()->mContent;
                    if (fileName == NULL)
                        return;

                    Int32 index1 = fileName->LastIndexOf(L'/');
                    Int32 index2 = fileName->LastIndexOfString(L".idl");
                    if (index2 == -1)
                        return;

                    Sp<String> fileNameWithoutSuffix = fileName->SubString(index1 + 1, index2 - index1 - 1);
                    if (fileNameWithoutSuffix == NULL)
                        return;

                    Int32 modeFlag = GetMode(obj);
                    if (modeFlag & IDL_MODE_IPC) {
                        WriteProxyFile(obj, fileNameWithoutSuffix);
                        WriteObjectHolderFile(obj, fileNameWithoutSuffix);
                        WriteStubFile(obj, fileNameWithoutSuffix);
                        WriteInterfaceFile(obj, fileNameWithoutSuffix);
                        WriteServiceFile(node, fileNameWithoutSuffix);
                    }
                    else if (modeFlag & IDL_MODE_MODELS)
                        WriteModelsFile(obj, fileNameWithoutSuffix);
                }
            }
        }
    }
} // namespace IDLC