
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
    EXTERN Sp<String> gRootOutputDir;
    STATIC pthread_key_t sKey;
    EXTERN Sp<String> GetOrignalFileName();

    STATIC Void WriteModelsMmFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix);
    STATIC Void ReadMembersFromParcel(IN TreeNode<CSymbol> * node);
    STATIC Void ReadParametersFromParcel(IN TreeNode<CSymbol> * node);
    STATIC Void ReadVarFromParcel(IN TreeNode<CSymbol> * node);
    STATIC Void WriteClassNameInsertMappingTable(IN TreeNode<CSymbol> * node);
    STATIC Void WriteCopyFunction(IN TreeNode<CSymbol> * node);
    STATIC Void WriteCreatesProtocol(IN TreeNode<CSymbol> * node);
    STATIC Void WriteDestructionProtocol(IN TreeNode<CSymbol> * node);
    STATIC Void WriteGetRemoteRefProtocol(IN TreeNode<CSymbol> * node);
    STATIC Void WriteIncludeFile(IN TreeNode<CSymbol> * node);
    STATIC Void WriteInterfaceFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix);
    STATIC Void WriteMembers(IN TreeNode<CSymbol> * node);
    STATIC Void WriteMembersToParcel(IN TreeNode<CSymbol> * node);
    STATIC Void WriteStubFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix);
    STATIC Void WriteObjectHolderFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix);
    STATIC Void WriteObjectHolderStubFunction(IN TreeNode<CSymbol> * node);
    STATIC Void WriteResetFunction(IN TreeNode<CSymbol> * node);
    STATIC Void WriteModelsClassImplement(IN TreeNode<CSymbol> * node);
    STATIC Void WriteModelsClassDeclare(IN TreeNode<CSymbol> * node);
    STATIC Void WriteProxyMmFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix);
    STATIC Void WriteProxyHeadFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix);

    STATIC Void WriteFunCodeMembers(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

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

                WRITE_STRING_TO_FILE(funCode->Length() + 77, L"static const int32_t %ls = %d;\r\n", (PCWStr)*funCode, count++);
            }
        }

        if (!hasConstruction) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 77, L"static const int32_t %ls = %d;\r\n", (PCWStr)*defConsId, count);
        }
    }

    Void WriteFunCodeMembersForCpp(IN TreeNode<CSymbol> * node)
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

    STATIC Void WriteDESCRIPTORMember(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        if ((node->GetParent()->GetChildren()->Get(0) == NULL) || (node->GetParent()->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> szNamespace = node->GetParent()->GetChildren()->Get(0)->GetValue()->mContent;
        if (szNamespace == NULL)
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        WRITE_STRING_TO_FILE(descptorName->Length() + szNamespace->Length() + className->Length() + 77,
            L"static NSString * %ls = @\"%ls.%ls\";\r\n", (PCWStr)*descptorName, (PCWStr)*szNamespace, (PCWStr)*className);
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
                failedMsg = String::Create(L"[AException Raise: AXP::AE_FAIL];\r\n");
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
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AXP::AFAILED(WriteObjectToParcel(parcel, %ls)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AXP::AFAILED([parcel WriteInt64: [%ls GetRemoteRef]]))\r\n", (PCWStr)*varId);
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
                L"if (AXP::AFAILED([parcel %ls: %ls]))\r\n", (PCWStr)*name, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AXP::AFAILED(WriteListOfObjectToParcel(parcel, %ls)))\r\n", (PCWStr)*varId);
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
                    IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (elementType == NULL)
                    return;

                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 108,
                    L"int32_t length = 0;\r\nif (%ls == nil)\r\nlength = 0;\r\nelse\r\nlength = (int32_t)%ls.count;\r\n\r\n",
                    (PCWStr)*varId, (PCWStr)*varId);
                FWRITE("AXP::Char type = 'L';\r\n");
                FWRITE("if (AXP::AFAILED([parcel Write:(Byte*)&type Length: sizeof(type)]))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (AXP::AFAILED([parcel Write:(Byte*)&length Length: sizeof(length)]))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 70,
                    L"for (%ls obj in %ls) {\r\nif (obj == nil)\r\n", (PCWStr)*elementType, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                Sp<String> functionName = table->GetValue(elementSymbol->mSymbolType);
                if (functionName == NULL)
                    return;

                WRITE_STRING_TO_FILE(functionName->Length() + 40,
                    L"if (AXP::AFAILED([parcel %ls: obj]))\r\n", (PCWStr)*functionName);
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
            if (node->GetValue()->mSymbolType == SymbolType_Member)
                failedMsg = String::Create(L"return AXP::AE_FAIL;\r\n");
            else
                failedMsg = String::Create(L"return NULL;\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
            failedMsg = String::Create(L"[AException Raise: AXP::AE_FAIL];\r\n");
        }
        else
            varId = NULL;

        if (varId == NULL)
            return;

        if (failedMsg == NULL)
            return;

        Sp<CSymbol> varSymbol = GetVarSymbol(node);
        if (varSymbol == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);
        Sp<HashTable<Int32, String> > table = MapTable::GetReadFromParcelNameOfBasicType();
        if (table == NULL)
            return;

        if (varSymbol->mSymbolType == SymbolType_Class) {
            if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter))
                FWRITE("@try {\r\n");

            WRITE_STRING_TO_FILE(varId->Length() + 97, L"%ls = ReadObjectFromParcel(parcel);\r\n", (PCWStr)*varId);
            if (node->GetValue()->mSymbolType == SymbolType_Parameter)
                FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn NULL;\r\n}\r\n\r\n");
            else if (node->GetValue()->mSymbolType == SymbolType_Member)
                FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn AXP::AE_FAIL;\r\n}\r\n\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"AXP::Int64 _%ls;\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AXP::AFAILED([parcel ReadInt64: &_%ls]))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if ((varSymbol->mSymbolType == SymbolType_Int8)
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
            || (varSymbol->mSymbolType == SymbolType_Double)) {
            Sp<String> name = table->GetValue(varSymbol->mSymbolType);
            if (name == NULL)
                return;

            WRITE_STRING_TO_FILE(name->Length() + varId->Length() + 97,
                L"if (AXP::AFAILED([parcel %ls: &%ls]))\r\n", (PCWStr)*name, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if ((varSymbol->mSymbolType == SymbolType_Int8_NULL)
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
            || (varSymbol->mSymbolType == SymbolType_Double_NULL)
            || (varSymbol->mSymbolType == SymbolType_String)
            || (varSymbol->mSymbolType == SymbolType_ByteArray)) {
            Sp<String> name = table->GetValue(varSymbol->mSymbolType);
            if (name == NULL)
                return;

            if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter))
                FWRITE("@try {\r\n");

            WRITE_STRING_TO_FILE(name->Length() + varId->Length() + 97,
                L"%ls = [parcel %ls];\r\n", (PCWStr)*varId, (PCWStr)*name);
            if (node->GetValue()->mSymbolType == SymbolType_Parameter)
                FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn NULL;\r\n}\r\n\r\n");
            else if (node->GetValue()->mSymbolType == SymbolType_Member)
                FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn AXP::AE_FAIL;\r\n}\r\n\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter))
                    FWRITE("@try {\r\n");

                WRITE_STRING_TO_FILE(varId->Length() + 97, L"%ls = ReadListOfObjectFromParcel(parcel);\r\n", (PCWStr)*varId);

                if (node->GetValue()->mSymbolType == SymbolType_Parameter)
                    FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn NULL;\r\n}\r\n\r\n");
                else if (node->GetValue()->mSymbolType == SymbolType_Member)
                    FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn AXP::AE_FAIL;\r\n}\r\n\r\n");
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
                    IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (elementType == NULL)
                    return;

                Sp<String> functionName = table->GetValue(elementSymbol->mSymbolType);
                if (functionName == NULL)
                    return;

                FWRITE("{\r\n");
                FWRITE("AXP::Char type;\r\nif (AXP::AFAILED([parcel Read: (Byte*)&type DstLength: sizeof(type) Length: sizeof(type)]))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (type != 'L')\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("int32_t length;\r\nif (AXP::AFAILED([parcel Read: (Byte*)&length DstLength: sizeof(length) Length: sizeof(length)]))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 97, L"%ls = [[NSMutableArray alloc] init];\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls == nil)\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("for (int32_t i = 0; i < length; i++) {\r\n");
                if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter))
                    FWRITE("@try {\r\n");

                WRITE_STRING_TO_FILE(elementType->Length() + functionName->Length() + 27,
                    L"%ls obj = [parcel %ls];\r\n", (PCWStr)*elementType, (PCWStr)*functionName);
                WRITE_STRING_TO_FILE(varId->Length() + 37, L"[%ls addObject: obj];\r\n", (PCWStr)*varId);
                if (node->GetValue()->mSymbolType == SymbolType_Parameter)
                    FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn NULL;\r\n}\r\n");
                else if (node->GetValue()->mSymbolType == SymbolType_Member)
                    FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn AXP::AE_FAIL;\r\n}\r\n");

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

    STATIC Sp<String> GetRealParameters(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return NULL;

        Sp<String> parameterList = String::Create(L"");
        if (parameterList == NULL)
            return NULL;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if ((obj == NULL) || (obj->GetValue() == NULL) || (obj->GetValue()->mSymbolType != SymbolType_Parameter))
                return NULL;

            Sp<String> varId = GetVarId(obj);
            if (varId == NULL)
                return NULL;

            if (parameterList->Equals(L"")) {
                parameterList = String::Create(varId->Length() + 27, L": %ls", (PCWStr)*varId);
                if (parameterList == NULL)
                    return NULL;
            }
            else {
                Sp<String> tag = String::Create(varId);
                if (tag == NULL)
                    return NULL;

                PWStr pwstr = (PWStr)tag->GetPayload();
                *pwstr -= 0x20;
                Sp<String> parameter = String::Create(tag->Length() + varId->Length() + 27,
                    L"%ls: %ls", (PCWStr)*tag, (PCWStr)*varId);
                if (parameter == NULL)
                    return NULL;

                parameterList = String::Create(parameterList->Length() + parameter->Length() + 2,
                    L"%ls %ls", (PCWStr)*parameterList, (PCWStr)*parameter);
                if (parameterList == NULL)
                    return NULL;
            }
        }

        return parameterList;
    }

    STATIC Sp<String> GetFormParameters(IN TreeNode<CSymbol> * node, IN Int32 flag)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return NULL;

        Sp<String> parameterList = String::Create(L"");
        if (parameterList == NULL)
            return NULL;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if ((obj == NULL) || (obj->GetValue() == NULL) || (obj->GetValue()->mSymbolType != SymbolType_Parameter))
                return NULL;

            Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), flag);
            if (varType == NULL)
                return NULL;

            Sp<String> varId = GetVarId(obj);
            if (varType == NULL)
                return NULL;

            if (flag & IDL_LANG_OBJC) {
                if (parameterList->Equals(L"")) {
                    parameterList = String::Create(varType->Length() + varId->Length() + 27, L": (%ls)%ls", (PCWStr)*varType, (PCWStr)*varId);
                    if (parameterList == NULL)
                        return NULL;
                }
                else {
                    Sp<String> tag = String::Create(varId);
                    if (tag == NULL)
                        return NULL;

                    PWStr pwstr = (PWStr)tag->GetPayload();
                    *pwstr -= 0x20;
                    Sp<String> parameter = String::Create(tag->Length() + varType->Length() + varId->Length() + 27,
                        L"%ls: (%ls)%ls", (PCWStr)*tag, (PCWStr)*varType, (PCWStr)*varId);
                    if (parameter == NULL)
                        return NULL;

                    parameterList = String::Create(parameterList->Length() + parameter->Length() + 2,
                        L"%ls %ls", (PCWStr)*parameterList, (PCWStr)*parameter);
                    if (parameterList == NULL)
                        return NULL;
                }
            }
            else if (flag & IDL_LANG_CPP) {

            }
        }

        return parameterList;
    }

    STATIC Void WriteMembers(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);
        FWRITE("{\r\n");
        FWRITE("@private\r\n\r\n");
        FWRITE("    AXP::Int8 mTag;\r\n");
        FWRITE("    AXP::Boolean mIsRemote;\r\n");
        FWRITE("    AXP::Int64 mRef;\r\n");
        FWRITE("    AXP::Sp<IPC::IStub> mConn;\r\n");
        FWRITE("    NSString * mToken;\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 38, L"    id<I%ls> mInterface;\r\n", (PCWStr)*className);
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteCreatesProtocol(IN TreeNode<CSymbol> * node)
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
                Sp<String> formParameterList = GetFormParameters(obj->GetChildren()->Get(1), IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (formParameterList == NULL)
                    return;

                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 37,
                    L"+ (%ls*)Create%ls;\r\n", (PCWStr)*className, (PCWStr)*formParameterList);
            }
        }

        if (!hasConstructions)
            WRITE_STRING_TO_FILE(className->Length() + 37, L"+ (%ls*)Create;\r\n", (PCWStr)*className);
    }

    STATIC Void WriteGetRemoteRefProtocol(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);
        FWRITE("- (int64_t)GetRemoteRef;\r\n");
    }

    STATIC Void WriteDestructionProtocol(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);
        FWRITE("- (void)dealloc;\r\n");
    }

    STATIC Void WriteModelsHeadFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 38,
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

        Boolean hasInclude = FALSE;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_FileBegin)) {
                hasInclude = TRUE;
                break;
            }
        }

        if (!hasInclude) {
            FWRITE("#import \"AXP/objective-c/AObject.h\"\r\n");
            FWRITE("#import \"AXP/objective-c/IParcelable.h\"\r\n");
        }

        WriteIncludeFile(node);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        WriteModelsClassDeclare(tmp);
                        if (!tmp->IsLastChild())
                            FWRITE("\r\n");
                    }
                }
            }
        }
    }

    STATIC Void WriteModelsClassImplement(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        for (Int32 i = 3; i < node->GetChildren()->GetCount(); ++i) {
            TreeNode<CSymbol> * obj = node->GetChildren()->Get(i);
            if (obj == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class)
                WriteModelsClassImplement(obj);
        }

        if ((node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        WRITE_STRING_TO_FILE(className->Length() + 37, L"@implementation %ls\r\n\r\n", (PCWStr)*className);

        for (Int32 i = 3; i < node->GetChildren()->GetCount(); ++i) {
            TreeNode<CSymbol> * var = node->GetChildren()->Get(i);
            if (var == NULL)
                return;

            if (var->GetValue()->mSymbolType == SymbolType_Member) {
                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = var->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                WRITE_STRING_TO_FILE(varId->Length() + 27, L"@synthesize %ls;\r\n", (PCWStr)*varId);
            }
        }

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        FWRITE("\r\n");
        WriteMembersToParcel(node);
        FWRITE("\r\n");
        ReadMembersFromParcel(node);
        FWRITE("\r\n");
        // WriteCopyFunction(node);
        // FWRITE("\r\n");
        WriteResetFunction(node);
        FWRITE("\r\n");
        FWRITE("- (NSString*)GetTypeName\r\n");
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(description->Length() + 37, L"return @\"%ls\";\r\n", (PCWStr)*description);
        FWRITE("}\r\n");
        FWRITE("\r\n@end\r\n");
    }

    STATIC Void WriteProxyHeadFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 38,
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

        WRITE_STRING_TO_FILE(szNamespace->Length() + fileNameWithoutSuffix->Length() + 50,
            L"#import \"./I%ls.h\"\r\n",
            (PCWStr)*szNamespace, (PCWStr)*fileNameWithoutSuffix);
        FWRITE("#include \"AXP/cplusplus/xplatform/include/astring.h\"\r\n");
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_NameSpace)) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() * 2 + 50,
                            L"@interface %ls : AObject<I%ls>\r\n\r\n", (PCWStr)*className, (PCWStr)*className);

                        WriteCreatesProtocol(tmp);
                        WRITE_STRING_TO_FILE(className->Length() + 97,
                            L"+ (%ls*)Create: (int64_t)objRef Token: (NSString*)token;\r\n", (PCWStr)*className);
                        WriteDestructionProtocol(tmp);
                        WriteGetRemoteRefProtocol(tmp);
                        FWRITE("- (void) AddRemoteRef: (IN CONST AXP::Sp<AXP::String> &)uri;\r\n");
                        WRITE_STRING_TO_FILE(className->Length() + 27, L"- (id<I%ls>)GetInterface;\r\n", (PCWStr)*className);

                        FWRITE("\r\n@end\r\n");
                    }
                    else
                        continue;

                    if (!tmp->IsLastChild())
                        FWRITE("\r\n");
                }
            }
        }

        ::fclose(file);
    }

    STATIC Void WriteParametersToParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
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

    STATIC Void ReadParametersFromParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Parameter)
                ReadVarFromParcel(obj);
        }
    }

    STATIC Void WriteProxyCreatesImplementation(IN TreeNode<CSymbol> * node)
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

        Int32 countOfConstruction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Construction) {
                Sp<String> functionNameTag;
                if (countOfConstruction++ > 0)
                    functionNameTag = String::Create(7, L"%d", countOfConstruction);
                else
                    functionNameTag = String::Create(L"");

                if (functionNameTag == NULL)
                    return;

                Sp<String> formParameterList = GetFormParameters(obj->GetChildren()->Get(1),
                    IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (formParameterList == NULL)
                    return;

                WRITE_STRING_TO_FILE(className->Length() + functionNameTag->Length() + formParameterList->Length() + 37,
                    L"+ (%ls*)Create%ls%ls\r\n", (PCWStr)*className, (PCWStr)*functionNameTag, (PCWStr)*formParameterList);
                FWRITE("{\r\n");

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                Sp<String> functionName = String::Create(functionNameTag->Length() + realParameterList->Length() + 37, L"InitWith%ls",
                    (PCWStr)*functionNameTag, (PCWStr)*realParameterList);
                if (functionName == NULL)
                    return;

                WRITE_STRING_TO_FILE(className->Length() * 2 + 140,
                    L"%ls * obj = [[%ls alloc] init];\r\nif (obj == nil)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n",
                    (PCWStr)*className, (PCWStr)*className);
                WRITE_STRING_TO_FILE(functionName->Length() + 17, L"[obj %ls];\r\n", (PCWStr)*functionName);
                FWRITE("return obj;\r\n");
                FWRITE("}\r\n\r\n");
            }
        }

        if (countOfConstruction == 0) {
            WRITE_STRING_TO_FILE(className->Length() + 37,
                L"+ (%ls*)Create\r\n", (PCWStr)*className);
            FWRITE("{\r\n");
            WRITE_STRING_TO_FILE(className->Length() * 2 + 140,
                L"%ls * obj = [[%ls alloc] init];\r\nif (obj == nil)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n",
                (PCWStr)*className, (PCWStr)*className);
            FWRITE("[obj InitWith];\r\n");
            FWRITE("return obj;\r\n");
            FWRITE("}\r\n\r\n");
        }

        WRITE_STRING_TO_FILE(className->Length() + 80, L"+ (%ls*) Create:(int64_t) objRef Token: (NSString*)token\r\n", (PCWStr)*className);
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(className->Length() * 2 + 127,
            L"%ls * obj = [[%ls alloc] init];\r\nif (obj == nil)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n",
            (PCWStr)*className, (PCWStr)*className);
        FWRITE("[obj InitWithRef: objRef Token: token];\r\n");
        FWRITE("return obj;\r\n");
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyConstructionsImplementation(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);
        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        Int32 countOfConstruction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_Construction)) {
                Sp<String> functionNameTag;
                if (countOfConstruction++ > 0)
                    functionNameTag = String::Create(7, L"%d", countOfConstruction);
                else
                    functionNameTag = String::Create(L"");

                if (functionNameTag == NULL)
                    return;

                Sp<String> consId = GetConsOrFuncId(obj);
                if (consId == NULL)
                    return;

                Sp<String> formParameterList = GetFormParameters(
                    obj->GetChildren()->Get(1), IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (formParameterList == NULL)
                    return;

                WRITE_STRING_TO_FILE(functionNameTag->Length() + formParameterList->Length() + 47,
                    L"-(void) InitWith%ls%ls\r\n", (PCWStr)*functionNameTag, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                FWRITE("mTag = (AXP::Int8)0x93;\r\nmToken = nil;\r\n");
                FWRITE("mInterface = nil;\r\n");
                FWRITE("CParcel * parcel = [[CParcel alloc] init];\r\nif (parcel == NULL)\r\n"
                    "[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("AXP::Sp<AXP::String> uri = AXP::String::Create((AXP::WChar*)[");
                WRITE_STRING_TO_FILE(descptorName);
                FWRITE(" cStringUsingEncoding:NSUTF32LittleEndianStringEncoding]);\r\n"
                    "if (uri == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("AXP::Sp<IPC::IStub> stub = IPC::ServiceManager::GetService(uri);\r\n");
                FWRITE("if (stub == NULL) {\r\n");
                FWRITE("mIsRemote = TRUE;\r\n");
                FWRITE("mConn = IPC::CServerConnection::Create(uri);\r\nif (mConn == NULL)\r\n"
                    "[AException Raise: AXP::AE_FAIL];\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteInt8: mTag]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteString: mToken]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 107,
                    L"if (AXP::AFAILED([parcel WriteString: %ls]))\r\n[AException Raise: AXP::AE_FAIL];\r\n",
                    (PCWStr)*descptorName);
                FWRITE("}\r\nelse {\r\n");
                FWRITE("mIsRemote = FALSE;\r\nmConn = stub;\r\n");
                FWRITE("}\r\n\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteInt32: IPC::CommandCode::COMMAND_CREATE]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteBoolean: mIsRemote]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\nif (mIsRemote) {\r\n"
                    "if (AXP::AFAILED([parcel WriteString: [[NSString alloc] initWithBytes: IPC::ServiceManager::sServerAddress->GetPayload()"
                    " length: IPC::ServiceManager::sServerAddress->Length() * sizeof(AXP::WChar) encoding: NSUTF32LittleEndianStringEncoding]]))\r\n"
                    "[AException Raise: AXP::AE_FAIL];\r\n}\r\n\r\n");
                WRITE_STRING_TO_FILE(consId->Length() + 70, L"if (AXP::AFAILED([parcel WriteInt32: %ls]))\r\n", (PCWStr)*consId);
                FWRITE("[AException Raise: AXP::AE_FAIL];\r\n\r\n");

                WriteParametersToParcel(obj->GetChildren()->Get(1));

                FWRITE("AXP::Sp<AXP::CParcel> _parcel = new AXP::CParcel();\r\n"
                    "if (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("_parcel->Reset([parcel GetPayload], [parcel GetLength]);\r\n");
                FWRITE("_parcel = mConn->Transact(_parcel);\r\nif (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("_parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(_parcel->ReadInt32(code)))\r\n"
                    "[AException Raise: AXP::AE_FAIL];\r\n\r\nReadException(code);\r\n");
                FWRITE("if (AXP::AFAILED(_parcel->ReadInt64(mRef)))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if (!mIsRemote) {\r\n@autoreleasepool {\r\n");
                FWRITE("mInterface = ObjectManager_GetObject(mRef);\r\n}\r\n\r\n"
                    "if (mInterface == nil)\r\n[AException Raise: AXP::AE_FAIL];\r\n}\r\n");
                FWRITE("}\r\n\r\n");
            }
        }

        if (countOfConstruction == 0) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            FWRITE("-(void) InitWith\r\n");
            FWRITE("{\r\n");
            FWRITE("mTag = (AXP::Int8)0x93;\r\nmToken = nil;\r\n");
            FWRITE("mInterface = nil;\r\n");
            FWRITE("CParcel * parcel = [[CParcel alloc] init];\r\nif (parcel == NULL)\r\n"
                "[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("AXP::Sp<AXP::String> uri = AXP::String::Create((AXP::WChar*)[");
            WRITE_STRING_TO_FILE(descptorName);
            FWRITE(" cStringUsingEncoding:NSUTF32LittleEndianStringEncoding]);\r\n"
                "if (uri == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("AXP::Sp<IPC::IStub> stub = IPC::ServiceManager::GetService(uri);\r\n");
            FWRITE("if (stub == NULL) {\r\n");
            FWRITE("mIsRemote = TRUE;\r\n");
            FWRITE("mConn = IPC::CServerConnection::Create(uri);\r\nif (mConn == NULL)\r\n"
                "[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("if (AXP::AFAILED([parcel WriteInt8: mTag]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("if (AXP::AFAILED([parcel WriteString: mToken]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 107,
                L"if (AXP::AFAILED([parcel WriteString: %ls]))\r\n[AException Raise: AXP::AE_FAIL];\r\n",
                (PCWStr)*descptorName);
            FWRITE("}\r\nelse {\r\n");
            FWRITE("mIsRemote = FALSE;\r\nmConn = stub;\r\n");
            FWRITE("}\r\n\r\n");
            FWRITE("if (AXP::AFAILED([parcel WriteInt32: IPC::CommandCode::COMMAND_CREATE]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("if (AXP::AFAILED([parcel WriteBoolean: mIsRemote]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\nif (mIsRemote) {\r\n"
                "if (AXP::AFAILED([parcel WriteString: [[NSString alloc] initWithBytes: IPC::ServiceManager::sServerAddress->GetPayload()"
                " length: IPC::ServiceManager::sServerAddress->Length() * sizeof(AXP::WChar) encoding: NSUTF32LittleEndianStringEncoding]]))\r\n"
                "[AException Raise: AXP::AE_FAIL];\r\n}\r\n\r\n");
            WRITE_STRING_TO_FILE(defConsId->Length() + 70, L"if (AXP::AFAILED([parcel WriteInt32: %ls]))\r\n", (PCWStr)*defConsId);
            FWRITE("[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("AXP::Sp<AXP::CParcel> _parcel = new AXP::CParcel();\r\n"
                "if (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("_parcel->Reset([parcel GetPayload], [parcel GetLength]);\r\n");
            FWRITE("_parcel = mConn->Transact(_parcel);\r\nif (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("_parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(_parcel->ReadInt32(code)))\r\n"
                "[AException Raise: AXP::AE_FAIL];\r\n\r\nReadException(code);\r\n");
            FWRITE("if (AXP::AFAILED(_parcel->ReadInt64(mRef)))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
            FWRITE("if (!mIsRemote) {\r\n@autoreleasepool {\r\n");
            FWRITE("mInterface = ObjectManager_GetObject(mRef);\r\n}\r\n\r\nif (mInterface == nil)\r\n[AException Raise: AXP::AE_FAIL];\r\n}\r\n");
            FWRITE("}\r\n\r\n");
        }

        FWRITE("-(void) InitWithRef: (IN AXP::Int64)objRef Token: (NSString*)token\r\n");
        FWRITE("{\r\n");
        FWRITE("mTag = (AXP::Int8)0x94;\r\nmToken = token;\r\n");
        FWRITE("mInterface = nil;\r\nmIsRemote = TRUE;\r\nmRef = objRef;\r\n");
        FWRITE("mConn = IPC::CServerConnection::Create(AXP::String::Create((AXP::WChar*)[");
        WRITE_STRING_TO_FILE(descptorName);
        FWRITE(" cStringUsingEncoding: NSUTF32LittleEndianStringEncoding]));\r\nif (mConn == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        /*
        FWRITE("CParcel * parcel = [[CParcel alloc] init];\r\nif (parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 107,
        L"if (AXP::AFAILED([parcel WriteString: %ls]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n",
        (PCWStr)*descptorName);
        FWRITE("if (AXP::AFAILED([parcel WriteInt32: IPC::CommandCode::COMMAND_CALLBACK]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteString: [[NSString alloc] initWithBytes: IPC::ServiceManager::sServerAddress->GetPayload()"
        " length: IPC::ServiceManager::sServerAddress->Length() * sizeof(AXP::WChar) encoding: NSUTF32LittleEndianStringEncoding]]))\r\n"
        "[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteInt64: mRef]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("AXP::Sp<AXP::CParcel> _parcel = new AXP::CParcel();\r\nif (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("_parcel->Reset([parcel GetPayload], [parcel GetLength]);\r\n");
        FWRITE("_parcel = mConn->Transact(_parcel);\r\nif (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("_parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(_parcel->Read(code)))\r\n"
        "[AException Raise: AXP::AE_FAIL];\r\n\r\nReadException(code);\r\n");
        FWRITE("AXP::Int64 obj;\r\nif (AXP::AFAILED(_parcel->Read(obj)))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("if (obj != mRef)\r\n[AException Raise: AXP::AE_FAIL];\r\n");*/
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyDestructionImplementation(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        FWRITE("-(void) dealloc\r\n");
        FWRITE("{\r\n");
        FWRITE("if (mToken)\r\nreturn;\r\n\r\n");
        FWRITE("@try {\r\n");
        FWRITE("CParcel* parcel = [[CParcel alloc] init];\r\n");
        FWRITE("if (parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("if (mIsRemote) {\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteInt8: mTag]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteString: mToken]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 107,
            L"if (AXP::AFAILED([parcel WriteString: %ls]))\r\n[AException Raise: AXP::AE_FAIL];\r\n",
            (PCWStr)*descptorName);
        FWRITE("}\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteInt32: IPC::CommandCode::COMMAND_RELEASE]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteBoolean: mIsRemote]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\nif (mIsRemote) {\r\n"
            "if (AXP::AFAILED([parcel WriteString: [[NSString alloc] initWithBytes: IPC::ServiceManager::sServerAddress->GetPayload()"
            " length: IPC::ServiceManager::sServerAddress->Length() * sizeof(AXP::WChar) encoding: NSUTF32LittleEndianStringEncoding]]))\r\n"
            "[AException Raise: AXP::AE_FAIL];\r\n}\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteInt64: mRef]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("AXP::Sp<AXP::CParcel> _parcel = new AXP::CParcel();\r\nif (_parcel == NULL)\r\n"
            "[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("_parcel->Reset([parcel GetPayload], [parcel GetLength]);\r\n");
        FWRITE("_parcel = mConn->Transact(_parcel);\r\n");
        FWRITE("if (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
        FWRITE("_parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(_parcel->ReadInt32(code)))\r\n"
            "[AException Raise: AXP::AE_FAIL];\r\n\r\nReadException(code);\r\n");
        FWRITE("}\r\n@catch(...) {\r\n}\r\n");
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyFunctionsImplementation(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
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
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Function) {
                Sp<String> returnTypeObjc = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (returnTypeObjc == NULL)
                    return;

                Sp<String> returnTypeCpp = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CPP | IDL_VAR_TYPE_DECLARE);
                if (returnTypeCpp == NULL)
                    return;

                if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> functionName = obj->GetChildren()->Get(1)->GetValue()->mContent;
                if (functionName == NULL)
                    return;

                Sp<String> funcId = GetConsOrFuncId(obj);
                if (funcId == NULL)
                    return;

                Sp<String> formParameterList = GetFormParameters(obj->GetChildren()->Get(2),
                    IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
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
                                L"[%ls AddRemoteRef:IPC::ServiceManager::GetProxyAddr("
                                "(AXP::PCWStr)[%ls cStringUsingEncoding:NSUTF32LittleEndianStringEncoding])];\r\n",
                                (PCWStr)*varId, (PCWStr)*descptorName);
                            if (addRemoteRef == NULL)
                                return;
                        }
                    }
                }

                WRITE_STRING_TO_FILE(returnTypeObjc->Length() + functionName->Length() + formParameterList->Length() + 35,
                    L"-(%ls) %ls%ls\r\n", (PCWStr)*returnTypeObjc, (PCWStr)*functionName, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                FWRITE("if (mIsRemote) {\r\n");
                FWRITE("CParcel* parcel = [[CParcel alloc] init];\r\nif (parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteInt8: mTag]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteString: mToken]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 107,
                    L"if (AXP::AFAILED([parcel WriteString: %ls]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n",
                    (PCWStr)*descptorName);
                FWRITE("if (AXP::AFAILED([parcel WriteInt32: IPC::CommandCode::COMMAND_CALL]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if (AXP::AFAILED([parcel WriteInt64: mRef]))\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                WRITE_STRING_TO_FILE(funcId->Length() + 70,
                    L"if (AXP::AFAILED([parcel WriteInt32: %ls]))\r\n", (PCWStr)*funcId);
                FWRITE("[AException Raise: AXP::AE_FAIL];\r\n\r\n");

                WriteParametersToParcel(obj->GetChildren()->Get(2));

                if (obj->GetChildren()->Get(0) == NULL)
                    return;

                Sp<CSymbol> varSymbol = obj->GetChildren()->Get(0)->GetValue();
                if (varSymbol == NULL)
                    return;

                Sp<String> returnType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                if (returnType == NULL)
                    return;

                FWRITE("AXP::Sp<AXP::CParcel> _parcel = new AXP::CParcel();\r\n"
                    "if (_parcel == NULL)\r\n[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("_parcel->Reset([parcel GetPayload], [parcel GetLength]);\r\n");

                FWRITE("_parcel = mConn->Transact(_parcel);\r\nif (_parcel == NULL)\r\n"
                    "[AException Raise: AXP::AE_FAIL];\r\n\r\n");
                FWRITE("if ((mTag & 0x0F) == 0x04)\r\n");
                if (varSymbol->mSymbolType == SymbolType_Void)
                    FWRITE("return;\r\n");
                else if ((varSymbol->mSymbolType == SymbolType_Int8)
                    || (varSymbol->mSymbolType == SymbolType_Byte)
                    || (varSymbol->mSymbolType == SymbolType_Int16)
                    || (varSymbol->mSymbolType == SymbolType_Int32)
                    || (varSymbol->mSymbolType == SymbolType_Int64)
                    || (varSymbol->mSymbolType == SymbolType_Float)
                    || (varSymbol->mSymbolType == SymbolType_Double))
                    FWRITE("return 0;\r\n\r\n");
                else if (varSymbol->mSymbolType == SymbolType_Boolean)
                    FWRITE("return false;\r\n\r\n");
                else
                    FWRITE("return nil;\r\n\r\n");

                FWRITE("_parcel->Reset();\r\nAXP::Int32 code;\r\nif (AXP::AFAILED(_parcel->ReadInt32(code)))\r\n"
                    "[AException Raise: AXP::AE_FAIL];\r\n\r\nReadException(code);\r\n");
                WRITE_STRING_TO_FILE(addRemoteRef);
                if (varSymbol->mSymbolType == SymbolType_Void)
                    FWRITE("return;\r\n");
                else {
                    FWRITE("[parcel Reset:_parcel->GetPayload() Length:_parcel->GetLength()];\r\n");
                    FWRITE("[parcel Seek:_parcel->GetPosition()];\r\n\r\n");
                    WRITE_STRING_TO_FILE(returnType->Length() + 7, L"%ls ret;\r\n", (PCWStr)*returnType);
                    ReadVarFromParcel(obj);
                    FWRITE("return ret;\r\n");
                }

                FWRITE("}\r\nelse {\r\n");
                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(2));
                if (realParameterList == NULL)
                    return;

                if (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_Void)
                    WRITE_STRING_TO_FILE(functionName->Length() + realParameterList->Length() + 37,
                    L"[mInterface %ls%ls];\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);
                else
                    WRITE_STRING_TO_FILE(functionName->Length() + realParameterList->Length() + 37,
                    L"return [mInterface %ls%ls];\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);

                FWRITE("}\r\n");
                FWRITE("}\r\n\r\n");
            }
        }
    }

    STATIC Void WriteProxyGetRemoteRefImplementation(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("-(int64_t) GetRemoteRef\r\n");
        FWRITE("{\r\n");
        FWRITE("return mRef;\r\n");
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyMmFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 38,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*szNamespace);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 17,
            L"%ls/%ls.mm", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        FWRITE("#import \"AXP/objective-c/Parcel.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/ServerConnection.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/ServiceManager.h\"\r\n");
        FWRITE("#import \"IPC/objective-c/lib/include/IpcException.h\"\r\n");
        WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 27, L"#import \"%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);
        if (HasObjectTypeInInterface(node) || (GetMode(node) & IDL_MODE_MODELS))
            FWRITE("#import \"AXP/objective-c/libc/include/Common/ClassLoader.h\"\r\n");

        FWRITE("\r\n");
        FWRITE("EXTERN id ObjectManager_GetObject(IN AXP::Int64 key);\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_NameSpace)) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        Sp<String> interfacz = String::Create(className->Length() + 7, L"I%ls", (PCWStr)*className);
                        if (interfacz == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 27,
                            L"@implementation %ls\r\n", (PCWStr)*className);

                        WriteMembers(tmp);
                        WriteFunCodeMembers(tmp);
                        WriteDESCRIPTORMember(tmp);
                        FWRITE("\r\n");
                        WriteProxyCreatesImplementation(tmp);
                        WriteProxyConstructionsImplementation(tmp);
                        WriteProxyDestructionImplementation(tmp);
                        WriteProxyFunctionsImplementation(tmp);
                        WriteProxyGetRemoteRefImplementation(tmp);

                        FWRITE("-(void) AddRemoteRef: (IN CONST AXP::Sp<AXP::String> &)uri\r\n{\r\nmConn->AddRemoteRef("
                            "uri, mRef);\r\n}\r\n\r\n");
                        WRITE_STRING_TO_FILE(className->Length() + 27, L"-(id<I%ls>)GetInterface\r\n", (PCWStr)*className);
                        FWRITE("{\r\nreturn mInterface;\r\n}\r\n\r\n");
                        FWRITE("@end\r\n");
                    }
                    else if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class))
                        WriteModelsClassImplement(tmp);
                    else
                        continue;

                    if (!tmp->IsLastChild())
                        FWRITE("\r\n");
                }

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        FWRITE("\r\n");
                        WriteClassNameInsertMappingTable(tmp);
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_OBJC);
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

                WRITE_STRING_TO_FILE(funCodeName->Length() + 57,
                    L"%lsif (funCode == %ls) {\r\n",
                    countofFunction++ == 0 ? L"" : L"else ", (PCWStr)*funCodeName);

                Boolean flag = FALSE;
                if (obj->GetChildren()->Get(2) == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()->Get(2)->GetChildren()) {
                    if (tmp->GetValue() && tmp->GetValue()->mSymbolType == SymbolType_Parameter) {
                        Sp<String> varType = GetVarType(tmp->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
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
                            Sp<String> className = GetTypeReferenceList(tmp->GetChildren()->Get(0), IDL_LANG_OBJC);
                            if (className == NULL)
                                return;

                            flag = TRUE;

                            ReadVarFromParcel(tmp);

                            WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 17, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                            FWRITE("@try {\r\n");
                            FWRITE("NSString * token = [parcel ReadString];\r\n");
                            WRITE_STRING_TO_FILE(className->Length() + varId->Length() + 77,
                                L"%ls = [%ls Create:_%ls Token:token];\r\n",
                                (PCWStr)*varId, (PCWStr)*className, (PCWStr)*varId);
                            FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn NULL;\r\n}\r\n\r\n");
                            break;
                        }
                        else {
                            WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 17, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                            ReadVarFromParcel(tmp);
                        }
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
                        L"[mService %ls%ls];\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);
                    FWRITE("[parcel Reset];\r\n");
                    FWRITE("if (AXP::AFAILED([parcel WriteInt32:(AXP::Int32)IPC::NoException]))\r\nreturn NULL;\r\n");
                }
                else {
                    Sp<String> returnType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                    if (returnType == NULL)
                        return;

                    WRITE_STRING_TO_FILE(77 + returnType->Length() + functionName->Length() + realParameterList->Length(),
                        L"%ls ret = [mService %ls%ls];\r\n",
                        (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*realParameterList);
                    FWRITE("[parcel Reset];\r\n");
                    FWRITE("if (AXP::AFAILED([parcel WriteInt32:(AXP::Int32)IPC::NoException]))\r\nreturn NULL;\r\n\r\n");

                    WriteVarToParcel(obj);
                }

                FWRITE("}\r\n");
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
            "IN CONST AXP::Sp<AXP::CParcel> & _parcel,\r\n"
            "IN CONST AXP::Sp<AXP::String> & uri)\r\n{\r\n");
        FWRITE("if (_parcel == NULL)\r\nreturn NULL;\r\n\r\n");
        FWRITE("AXP::Int32 funCode;\r\nif (AXP::AFAILED(_parcel->ReadInt32(funCode)))\r\nreturn NULL;\r\n\r\n");
        FWRITE("CParcel* parcel = [[CParcel alloc] init];\r\nif (parcel == NULL)\r\nreturn NULL;\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel Reset:_parcel->GetPayload() Length:_parcel->GetLength()]))\r\nreturn NULL;\r\n\r\n");
        FWRITE("[parcel Seek:_parcel->GetPosition()];\r\n\r\n");

        Int32 countOfConstruction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Construction) {
                Sp<String> consCodeId = GetConsOrFuncId(obj);
                if (consCodeId == NULL)
                    return;

                WRITE_STRING_TO_FILE(consCodeId->Length() + 40, L"%lsif (funCode == %ls) {\r\n",
                    countOfConstruction++ == 0 ? L"" : L"else ", (PCWStr)*consCodeId);
                FWRITE("if (mService == NULL) {\r\n");

                Foreach(TreeNode<CSymbol>, param, obj->GetChildren()->Get(1)->GetChildren()) {
                    if ((param == NULL) || (param->GetValue() == NULL))
                        return;

                    Sp<String> varType = GetVarType(param->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                    Sp<String> varId = GetVarId(param);
                    if ((varType == NULL) || (varId == NULL))
                        return;

                    WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 7, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                }

                ReadParametersFromParcel(obj->GetChildren()->Get(1));

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                FWRITE("Synchronized (&mLock) {\r\nif (mService == NULL) {\r\n@autoreleasepool {\r\n");
                WRITE_STRING_TO_FILE(className->Length() + realParameterList->Length() + 33,
                    L"mService = [_%ls Create%ls];\r\n", (PCWStr)*className, (PCWStr)*realParameterList);
                FWRITE("if (mService == NULL)\r\nreturn NULL;\r\n}\r\n}\r\n}\r\n}\r\n\r\n");
            }
        }

        if (countOfConstruction == 0) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 40, L"if (funCode == %ls) {\r\n", (PCWStr)*defConsId);
            FWRITE("if (mService == NULL) {\r\n");
            FWRITE("Synchronized (&mLock) {\r\nif (mService == NULL) {\r\n@autoreleasepool {\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 33, L"mService = [_%ls Create];\r\n", (PCWStr)*className);
            FWRITE("if (mService == NULL)\r\nreturn NULL;\r\n}\r\n}\r\n}\r\n}\r\n\r\n");
        }

        FWRITE("if (uri == NULL) {\r\nif (!ObjectManager_RegisterObject((AXP::Int64)this, mService))\r\n"
            "return NULL;\r\n}\r\n\r\n");
        FWRITE("[parcel Reset];\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteInt32:(AXP::Int32)IPC::NoException]))\r\nreturn NULL;\r\n\r\n");
        FWRITE("if (AXP::AFAILED([parcel WriteInt64:(AXP::Int64)this]))\r\nreturn NULL;\r\n");
        FWRITE("}\r\nelse {\r\nif (mService == NULL) {\r\n");
        FWRITE("AXP::Sp<AXP::CParcel> retParcel = new AXP::CParcel();\r\nif (retParcel == NULL)\r\nreturn NULL;\r\n\r\n");
        FWRITE("if (AXP::AFAILED(retParcel->WriteInt32((AXP::Int32)IPC::RemoteRefException)))\r\nreturn NULL;\r\n\r\n");
        FWRITE("return retParcel;\r\n}\r\n\r\n");

        WriteObjectHolderStubFunction(node);

        FWRITE("}\r\n\r\n");
        FWRITE("AXP::Sp<AXP::CParcel> retParcel = new AXP::CParcel();\r\nif (_parcel == NULL)\r\nreturn NULL;\r\n\r\n");
        FWRITE("retParcel->Reset([parcel GetPayload], [parcel GetLength]);\r\n\r\n");
        FWRITE("return retParcel;\r\n}\r\n\r\n");
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
                    Sp<String> obj = fileName->SubString(index1 + 1, index2 - index1 - 1);
                    if (obj == NULL)
                        return;

                    WRITE_STRING_TO_FILE(obj->Length() + 22, L"#import \"%ls.h\"\r\n", (PCWStr)*obj);
                    continue;
                }

                Sp<String> fileNameWithoutSuffix = fileName->SubString(index1 + 1, index2 - index1 - 1);
                if (fileNameWithoutSuffix == NULL)
                    return;

                Int32 modeFlag = GetMode(obj);
                Sp<String> tmp = GetNameSpace(obj);
                if (tmp == NULL)
                    return;

                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + sizeof("#import \"IPC/gen/%ls/%ls.h\"\r\n"),
                    L"#import \"IPC/gen/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // if (mode & IDL_MODE_IPC) {
                // if (modeFlag & IDL_MODE_IPC)
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                // L"#import \"../../proxy/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // else
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 77,
                // L"#import \"../../models/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // }
                // else if (mode & IDL_MODE_MODELS) {
                // if (namespacz->Equals(tmp)) {
                // if (modeFlag & IDL_MODE_IPC)
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                // L"#import \"../proxy/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // else
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 37,
                // L"#import \"%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);
                // }
                // else {
                // if (modeFlag & IDL_MODE_IPC)
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                // L"#import \"../proxy/%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // else
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 37,
                // L"#import \"../%ls/%ls.h\"\r\n", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // }
                // }
            }
        }
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

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 38,
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

        FWRITE("#import \"AXP/objective-c/Parcel.h\"\r\n");
        FWRITE("#include \"IPC/cplusplus/lib/include/ObjectHolder.h\"\r\n");
        FWRITE("#import \"IPC/objective-c/lib/include/IpcException.h\"\r\n");

        if (HasObjectTypeInInterface(node))
            FWRITE("#import \"AXP/objective-c/libc/include/Common/ClassLoader.h\"\r\n");

        WriteIncludeFile(node);

        FWRITE("\r\n");
        FWRITE("EXTERN AXP::Boolean ObjectManager_RegisterObject(IN AXP::Int64 key, IN id value);\r\n\r\n");

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
                        WRITE_STRING_TO_FILE(className->Length() + 50, L"_%ls * mService;\r\n", (PCWStr)*className);
                        WriteFunCodeMembersForCpp(tmp);
                        FWRITE("};\r\n");

                        dump = String::Create(dump->Length() + className->Length() * 2 + modifier->Length() * 2 + 117,
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

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_OBJC);
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

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 77,
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

    STATIC Void WriteInterfaceFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 38,
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

        FWRITE("#import \"AXP/objective-c/AObject.h\"\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class)) {
                        FWRITE("#import \"AXP/objective-c/IParcelable.h\"\r\n");
                        break;
                    }
                }
            }
        }

        WriteIncludeFile(node);

        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 17, L"@protocol I%ls\r\n\r\n", (PCWStr)*className);

                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                if (returnType == NULL)
                                    return;

                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(2),
                                    IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                if (formParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length() + formParameterList->Length() + 27,
                                    L"-(%ls) %ls%ls;\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                            }
                        }

                        FWRITE("\r\n@end\r\n");
                    }
                    else if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class)) {
                        WriteModelsClassDeclare(tmp);
                        FWRITE("\r\n");
                    }
                }
            }
        }

        ::fclose(file);
    }

    STATIC Void WriteServiceHeadFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 38,
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

        WRITE_STRING_TO_FILE(namespacz->Length() + fileNameWithoutSuffix->Length() + 50,
            L"#import \"./I%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> originClassName = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (originClassName == NULL)
                            return;

                        Sp<String> className = String::Create(originClassName->Length() + 7, L"_%ls", (PCWStr)*originClassName);
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + originClassName->Length() + 77,
                            L"@interface %ls : AObject<I%ls>\r\n\r\n",
                            (PCWStr)*className, (PCWStr)*originClassName);

                        Int32 count = 0;
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1),
                                    IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                if (formParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 37,
                                    L"+(%ls*) Create%ls;\r\n",
                                    (PCWStr)*className, (PCWStr)*formParameterList);

                                count++;
                            }
                        }

                        if (count == 0)
                            WRITE_STRING_TO_FILE(className->Length() + 37, L"+(%ls*) Create;\r\n", (PCWStr)*className);

                        FWRITE("\r\n@end\r\n");
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_OBJC);
    }

    STATIC Void WriteServiceImplementFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 38,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/_%ls.mm", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        if (!ACCESS((PStr)fullFileName->GetBytes()->GetPayload(), 0))
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 70,
            L"#import \"./_%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);

        WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 70,
            L"#import \"IPC/gen/%ls/%lsObjectHolder.h\"\r\n", (PCWStr)*namespacz, (PCWStr)*fileNameWithoutSuffix);
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> originClassName = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (originClassName == NULL)
                            return;

                        Sp<String> className = String::Create(originClassName->Length() + 7, L"_%ls", (PCWStr)*originClassName);
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 77, L"@implementation %ls\r\n\r\n", (PCWStr)*className);
                        Int32 countOfConstruction = 0;
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                Sp<String> functionNameTag;
                                if (countOfConstruction++ > 0)
                                    functionNameTag = String::Create(7, L"%d", countOfConstruction);
                                else
                                    functionNameTag = String::Create(L"");

                                if (functionNameTag == NULL)
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1),
                                    IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                if (formParameterList == NULL)
                                    return;

                                Sp<String> realParameterList = GetRealParameters(var->GetChildren()->Get(1));
                                if (realParameterList == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 137,
                                    L"+(%ls*) Create%ls\r\n{\r\n%ls obj =  [[%ls alloc] init];\r\nif (obj == nil)\r\nreturn nil;\r\n\r\n"
                                    "[obj InitWith%ls%ls];\r\nreturn obj;\r\n}\r\n\r\n",
                                    (PCWStr)*className, (PCWStr)*formParameterList, (PCWStr)*className, (PCWStr)*className,
                                    (PCWStr)*functionNameTag, (PCWStr)*realParameterList);
                            }
                        }

                        if (countOfConstruction == 0)
                            WRITE_STRING_TO_FILE(className->Length() * 2 + 77,
                            L"+(%ls*) Create\r\n{\r\nreturn [[%ls alloc] init];\r\n}\r\n\r\n", (PCWStr)*className, (PCWStr)*className);

                        if (countOfConstruction > 0) {
                            countOfConstruction = 0;
                            Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                                if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                    Sp<String> functionNameTag;
                                    if (countOfConstruction++ > 0)
                                        functionNameTag = String::Create(7, L"%d", countOfConstruction);
                                    else
                                        functionNameTag = String::Create(L"");

                                    if (functionNameTag == NULL)
                                        return;

                                    Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1),
                                        IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                    if (formParameterList == NULL)
                                        return;

                                    Sp<String> realParameterList = GetRealParameters(var->GetChildren()->Get(1));
                                    if (realParameterList == NULL)
                                        return;

                                    WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 137,
                                        L"-(void) InitWith%ls%ls\r\n{\r\nreturn;\r\n}\r\n\r\n",
                                        (PCWStr)*functionNameTag, (PCWStr)*formParameterList);
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

                                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                if (returnType == NULL)
                                    return;

                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(2),
                                    IDL_LANG_OBJC | IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_DECLARE);
                                if (formParameterList == NULL)
                                    return;

                                Sp<String> realParameterList = GetRealParameters(var->GetChildren()->Get(2));
                                if (realParameterList == NULL)
                                    return;

                                if (retSymbol->mSymbolType == SymbolType_Void)
                                    WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length()
                                    + formParameterList->Length() + 77,
                                    L"-(%ls) %ls%ls\r\n{\r\n\r\n}\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                                else if ((retSymbol->mSymbolType == SymbolType_Int8)
                                    || (retSymbol->mSymbolType == SymbolType_Byte)
                                    || (retSymbol->mSymbolType == SymbolType_Int16)
                                    || (retSymbol->mSymbolType == SymbolType_Int32)
                                    || (retSymbol->mSymbolType == SymbolType_Int64)
                                    || (retSymbol->mSymbolType == SymbolType_Float)
                                    || (retSymbol->mSymbolType == SymbolType_Double))
                                    WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length()
                                    + formParameterList->Length() + 77,
                                    L"-(%ls) %ls%ls\r\n{\r\nreturn 0;\r\n}\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                                else if (retSymbol->mSymbolType == SymbolType_Boolean)
                                    WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length()
                                    + formParameterList->Length() + 77,
                                    L"-(%ls) %ls%ls\r\n{\r\nreturn false;\r\n}\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                                else
                                    WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length()
                                    + formParameterList->Length() + 77,
                                    L"-(%ls) %ls%ls\r\n{\r\nreturn nil;\r\n}\r\n\r\n",
                                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                            }
                        }

                        FWRITE("\r\n@end\r\n");
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_OBJC);
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

        if (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue()->mContent == NULL)
            return String::Create(L"AObject<IParcelable>");
        else
            return GetTypeReferenceList(node->GetChildren()->Get(2)->GetChildren()->Get(0), IDL_LANG_OBJC);
    }

    STATIC Void WriteModelsClassDeclare(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                WriteModelsClassDeclare(obj);
                FWRITE("\r\n");
            }
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

        if ((node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> baseClassName = GetBaseClassName(node);
        if (baseClassName == NULL)
            return;

        WRITE_STRING_TO_FILE(className->Length() + baseClassName->Length() + 37,
            L"@interface %ls : %ls\r\n\r\n", (PCWStr)*className, (PCWStr)*baseClassName);

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

                Sp<String> varType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_OBJC | IDL_VAR_TYPE_PROPERTY);
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

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_OBJC);
        if (scopeOfClass == NULL)
            return;


        FWRITE("\r\n");
        //WRITE_STRING_TO_FILE(scopeOfClass->Length() + 57, L"- (void)Copy: (CONST AXP::Sp<%ls>&) obj;\r\n", (PCWStr)*scopeOfClass);
        FWRITE("- (void)Reset;\r\n");
        FWRITE("- (NSString*)GetTypeName;\r\n");
        FWRITE("\r\n@end\r\n");
    }

    STATIC Void WriteCopyFunction(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_OBJC);
        if (scopeOfClass == NULL)
            return;

        WRITE_STRING_TO_FILE(scopeOfClass->Length() + 60, L"- (void)Copy: (CONST AXP::Sp<%ls>&) obj\r\n{\r\n", (PCWStr)*scopeOfClass);
        FWRITE("if (obj == NULL)\r\nreturn;\r\n\r\n");
        FWRITE("[self Reset];\r\n");

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

            if (symbol->mSymbolType == SymbolType_Class)
                FWRITE("[super Copy: obj];\r\n\r\n");
        }

        for (Int32 i = 3; i < node->GetChildren()->GetCount(); ++i) {
            if (node->GetChildren()->Get(i) == NULL)
                return;

            if (node->GetChildren()->Get(i)->GetValue()
                && (node->GetChildren()->Get(i)->GetValue()->mSymbolType == SymbolType_Member)) {
                //Sp<CSymbol> varSymbol = GetVarSymbol(node->GetChildren()->Get(i));
                Sp<CSymbol> varSymbol = node->GetChildren()->Get(i)->GetChildren()->Get(0)->GetValue();
                if (varSymbol == NULL)
                    return;

                Sp<String> varId = GetVarId(node->GetChildren()->Get(i));
                if (varId == NULL)
                    return;

                if (varSymbol->mSymbolType == SymbolType_List) {
                    if ((node->GetChildren()->Get(i)->GetChildren() == NULL)
                        || (node->GetChildren()->Get(i)->GetChildren()->Get(0) == NULL)
                        || (node->GetChildren()->Get(i)->GetChildren()->Get(0)->GetChildren() == NULL)
                        || (node->GetChildren()->Get(i)->GetChildren()->Get(0)->GetChildren()->Get(0) == NULL))
                        return;


                    //Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(i)->GetChildren()->Get(0));
                    Sp<CSymbol> elementSymbol = node->GetChildren()->Get(i)->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue();
                    if (elementSymbol == NULL)
                        return;

                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 58, L"        %ls = [NSMutableArray array];\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 58, L"        if (!%ls)\r\n            return;\r\n\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 60, L"        for (int i = 0; i < obj->%ls->GetCount(); ++i) {\r\n", (PCWStr)*varId);

                    if (elementSymbol->mSymbolType == SymbolType_ByteArray) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 125,
                            L"            NSData * nsdata = [NSData dataWithBytes: obj->%ls->Get(i)->GetPayload() length: obj->%ls->Get(i)->GetUsed()];\r\n",
                            (PCWStr)*varId, (PCWStr)*varId);
                        FWRITE("            if (!nsdata)\r\n");
                        FWRITE("                return;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 50, L"            [%ls addObject: nsdata];\r\n", (PCWStr)*varId);
                    }
                    else if (elementSymbol->mSymbolType == SymbolType_TypeReference) {
                        Sp<String> elementType = GetTypeReferenceList(node->GetChildren()->Get(i)->GetChildren()->Get(0)->GetChildren()->Get(0), IDL_LANG_OBJC);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() * 2 + 47, L"            %ls * tmp = [[%ls alloc] init];\r\n", (PCWStr)*elementType, (PCWStr)*elementType);
                        FWRITE("            if (!tmp)\r\n");
                        FWRITE("                return;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 47, L"            [tmp Copy: obj->%ls->Get(i)];\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 47, L"            [%ls addObject: tmp];\r\n", (PCWStr)*varId);
                    }
                    else if (elementSymbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 200,
                            L"            NSString * str = [[NSString alloc] initWithBytes: obj->%ls->Get(i)->GetPayload() length: obj->%ls->Get(i)->Length() * sizeof(AXP::WChar) encoding: NSUTF32LittleEndianStringEncoding];\r\n",
                            (PCWStr)*varId, (PCWStr)*varId);
                        FWRITE("            if (!str)\r\n");
                        FWRITE("                return;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 47, L"            [%ls addObject: str];\r\n", (PCWStr)*varId);
                    }
                    else {
                        if (elementSymbol->mSymbolType == SymbolType_Int8_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithInt: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Int16_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithInt: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Int32_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithInt: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Int64_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithLongLong: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_UInt8_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithUnsignedInt: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_UInt16_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithUnsignedInt: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_UInt32_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithUnsignedInt: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_UInt64_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithUnsignedLongLong: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Float_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithFloat: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Double_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithDouble: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_Boolean_NULL) {
                            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                                L"[%ls addObject: [NSNumber numberWithBool: obj->%ls->Get(i)->GetValue()]];\r\n",
                                (PCWStr)*varId, (PCWStr)*varId);
                        }
                    }

                    FWRITE("}\r\n}\r\n\r\n");
                }
                else if (varSymbol->mSymbolType == SymbolType_TypeReference) {
                    Sp<String> elementType = GetTypeReferenceList(node->GetChildren()->Get(i)->GetChildren()->Get(0), IDL_LANG_OBJC);
                    if (elementType == NULL)
                        return;

                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    if (obj->%ls) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 28, L"        if (!%ls) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + elementType->Length() + 57,
                        L"            %ls = [[%ls alloc] init];\r\n", (PCWStr)*varId, (PCWStr)*elementType);
                    WRITE_STRING_TO_FILE(varId->Length() + 66,
                        L"            if (!%ls)\r\n                return;\r\n        }\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 37, L"        [%ls Copy:obj->%ls];\r\n", (PCWStr)*varId, (PCWStr)*varId);
                    FWRITE("    }\r\n");
                }
                else if (varSymbol->mSymbolType == SymbolType_ByteArray) {
                    WRITE_STRING_TO_FILE(varId->Length() + 29, L"    if (obj->%ls)\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 3 + 97,
                        L"        %ls = [NSData dataWithBytes: obj->%ls->GetPayload() length: obj->%ls->GetUsed()];\r\n\r\n",
                        (PCWStr)*varId, (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_String) {
                    WRITE_STRING_TO_FILE(varId->Length() + 29, L"    if (obj->%ls)\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 3 + 171,
                        L"        %ls = [[NSString alloc] initWithBytes: obj->%ls->GetPayload() length: obj->%ls->Length() * sizeof(AXP::WChar) encoding: NSUTF32LittleEndianStringEncoding];\r\n\r\n",
                        (PCWStr)*varId, (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Int8_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithInt: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Int16_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithInt: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Int32_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithInt: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Int64_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithLongLong: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_UInt8_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithUnsignedInt: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_UInt16_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithUnsignedInt: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_UInt32_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithUnsignedInt: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_UInt64_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithUnsignedLongLong: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Float_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithFloat: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Double_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithDouble: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Boolean_NULL) {
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"    if (obj->%ls.HasValue())\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                        L"%ls = [NSNumber numberWithBool: obj->%ls.GetValue()];\r\n\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else {
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 23, L"    %ls = obj->%ls;\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
            }
        }

        FWRITE("}\r\n");
    }

    STATIC Void ReadMembersFromParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("- (AXP::ARESULT)ReadFromParcel: (CParcel*)parcel\r\n");
        FWRITE("{\r\n");
        FWRITE("if (parcel == nil)\r\nreturn AXP::AE_FAIL;\r\n\r\n");
        FWRITE("@try {\r\n");
        FWRITE("[parcel ReadString];\r\n");
        FWRITE("}\r\n@catch (NSException * exception) {\r\nreturn AXP::AE_FAIL;\r\n}\r\n\r\n");

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode && baseClassNode->GetChildren()->Get(0) && baseClassNode->GetChildren()->Get(0)->GetValue() && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);

                return;
            }

            if (symbol->mSymbolType == SymbolType_Class)
                FWRITE("if (AXP::AFAILED([super ReadFromParcel:parcel]))\r\nreturn AXP::AE_FAIL;\r\n\r\n");
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

    STATIC Void WriteMembersToParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (scopeOfClass == NULL)
            return;

        FWRITE("- (AXP::ARESULT)WriteToParcel: (CParcel*)parcel\r\n");
        FWRITE("{\r\n");
        FWRITE("if (parcel == nil)\r\nreturn AXP::AE_FAIL;\r\n\r\n");
        WRITE_STRING_TO_FILE(scopeOfClass->Length() + 140,
            L"if (AXP::AFAILED([parcel WriteString:@\"%ls\"]))\r\nreturn AXP::AE_FAIL;\r\n\r\n", (PCWStr)*scopeOfClass);

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode && baseClassNode->GetChildren()->Get(0) && baseClassNode->GetChildren()->Get(0)->GetValue() && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);
                return;
            }

            if (symbol->mSymbolType == SymbolType_Class)
                FWRITE("if (AXP::AFAILED([super WriteToParcel:parcel]))\r\nreturn AXP::AE_FAIL;\r\n\r\n");
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

    STATIC Void WriteResetFunction(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(0)->GetValue() == NULL)
            || (node->GetChildren()->Get(0)->GetValue()->mContent == NULL))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("- (void)Reset\r\n");
        FWRITE("{\r\n");

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode && baseClassNode->GetChildren()->Get(0) && baseClassNode->GetChildren()->Get(0)->GetValue() && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent) {
            Sp<CSymbol> symbol = GetVarSymbol(baseClassNode);
            if (symbol == NULL) {
                Sp<String> baseClassName = GetTypeReferenceList(baseClassNode->GetChildren()->Get(0), IDL_LANG_CS);
                if (baseClassName == NULL)
                    return;

                DEBUG_PRINT(baseClassName->Length() + 67, L"symbol \"%ls\" not found!", (PCWStr)*baseClassName);
                return;
            }

            if (symbol->mSymbolType == SymbolType_Class)
                FWRITE("[super Reset];\r\n\r\n");
        }

        for (Int32 i = 3; i < node->GetChildren()->GetCount(); ++i) {
            if (node->GetChildren()->Get(i) == NULL)
                return;

            if (node->GetChildren()->Get(i)->GetValue()
                && (node->GetChildren()->Get(i)->GetValue()->mSymbolType == SymbolType_Member)) {
                Sp<CSymbol> varSymbol = GetVarSymbol(node->GetChildren()->Get(i));
                if (varSymbol == NULL) {
                    DEBUG_PRINT("symbol not found!\n");

                    return;
                }

                Sp<String> varId = GetVarId(node->GetChildren()->Get(i));
                if (varId == NULL)
                    return;

                if ((varSymbol->mSymbolType == SymbolType_List)
                    || (varSymbol->mSymbolType == SymbolType_ByteArray)
                    || (varSymbol->mSymbolType == SymbolType_String)
                    || (varSymbol->mSymbolType == SymbolType_Class)
                    || (varSymbol->mSymbolType == SymbolType_Interface)
                    || (varSymbol->mSymbolType == SymbolType_Boolean_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Float_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Double_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int8_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int16_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int32_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int64_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt8_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt16_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt32_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt64_NULL))
                    WRITE_STRING_TO_FILE(varId->Length() + 18, L"    %ls = nil;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Boolean)
                    WRITE_STRING_TO_FILE(varId->Length() + 18, L"    %ls = NO;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Float)
                    WRITE_STRING_TO_FILE(varId->Length() + 22, L"    %ls = DBL_MIN;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Int8)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = INT8_MIN;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Int16)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = INT16_MIN;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Int32)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = INT32_MIN;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Int64)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = INT64_MIN;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_UInt8)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = 0;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_UInt16)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = 0;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_UInt32)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = 0;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_UInt64)
                    WRITE_STRING_TO_FILE(varId->Length() + 24, L"    %ls = 0;\r\n", (PCWStr)*varId);
            }
        }

        FWRITE("}\r\n");
    }

    STATIC Void WriteClassNameInsertMappingTable(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);

        if ((node->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> varName = GetScopeChainOfClassName(node, IDL_NOP);
        if (varName == NULL)
            return;

        WRITE_STRING_TO_FILE(varName->Length() + 77, L"STATIC id Create_%ls()\r\n", (PCWStr)*varName);
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 77, L"return [[%ls alloc] init];\r\n", (PCWStr)*className);
        FWRITE("}\r\n\r\n");

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        WRITE_STRING_TO_FILE(varName->Length() + description->Length() + varName->Length() + 97,
            L"STATIC AXP::Boolean __%ls__ = RegisterClassCreator(L\"%ls\", Create_%ls);\r\n",
            (PCWStr)*varName, (PCWStr)*description, (PCWStr)*varName);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                FWRITE("\r\n");
                WriteClassNameInsertMappingTable(obj);
            }
        }
    }

    STATIC Void WriteModelsMmFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 77,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/%ls.mm", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + 27, L"#import \"%ls.h\"\r\n", (PCWStr)*fileNameWithoutSuffix);
        FWRITE("#import \"AXP/objective-c/libc/include/Common/ClassLoader.h\"\r\n");
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        WriteModelsClassImplement(tmp);
                        if (!tmp->IsLastChild())
                            FWRITE("\r\n");
                    }
                }

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        FWRITE("\r\n");
                        WriteClassNameInsertMappingTable(tmp);
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_OBJC);
    }

    Void WriteObjcFile(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        Sp<String> fileNameWithoutSuffix = GetOrignalFileName();
        if (fileNameWithoutSuffix == NULL)
            return;

        pthread_key_create(&sKey, NULL);
        Int32 modeFlag = GetMode(node);

        if (modeFlag & IDL_MODE_IPC) {
            WriteProxyHeadFile(node, fileNameWithoutSuffix);
            WriteProxyMmFile(node, fileNameWithoutSuffix);
            WriteObjectHolderFile(node, fileNameWithoutSuffix);
            WriteStubFile(node, fileNameWithoutSuffix);
            WriteInterfaceFile(node, fileNameWithoutSuffix);
            WriteServiceFile(node, fileNameWithoutSuffix);
        }
        else if (modeFlag & IDL_MODE_MODELS) {
            WriteModelsHeadFile(node, fileNameWithoutSuffix);
            WriteModelsMmFile(node, fileNameWithoutSuffix);
        }

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
                        WriteProxyHeadFile(obj, fileNameWithoutSuffix);
                        WriteProxyMmFile(obj, fileNameWithoutSuffix);
                        WriteObjectHolderFile(obj, fileNameWithoutSuffix);
                        WriteStubFile(obj, fileNameWithoutSuffix);
                        WriteInterfaceFile(obj, fileNameWithoutSuffix);
                        WriteServiceFile(node, fileNameWithoutSuffix);
                    }
                    else if (modeFlag & IDL_MODE_MODELS) {
                        WriteModelsHeadFile(obj, fileNameWithoutSuffix);
                        WriteModelsMmFile(obj, fileNameWithoutSuffix);
                    }
                }
            }
        }
    }
}