
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
    EXTERN Sp<List<String> > gHeadListCs;
    EXTERN Sp<String> gRootOutputDir;
    EXTERN CMappingArea * gShareAreaCs;
    EXTERN Sp<HashTable<PCWStr, CMappingInfo> > gMappingCs;
    EXTERN Sp<String> gNamespacz;

    EXTERN Sp<String> GetOrignalFileName();

    STATIC pthread_key_t sKey;

    STATIC Void WriteToString(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("public override String ToString()\r\n");
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
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null)\r\n", (PCWStr)*varId);
                        FWRITE("return \"\";\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 58, L"return %ls.ToString();\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_List) {
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (%ls == null)\r\nreturn null;\r\n\r\n", (PCWStr)*varId);
                        FWRITE("String json = \"[\";\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"for(Int32 i = 0; i < %ls.Count; ++i) {\r\n", (PCWStr)*varId);

                        Sp<CSymbol> elementSymbol = GetVarSymbol(obj->GetChildren()->Get(0));
                        if (elementSymbol == NULL)
                            return;

                        Sp<String> elementType = GetVarType(
                            obj->GetChildren()->Get(0)->GetChildren()->Get(0), IDL_VAR_TYPE_CS);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 47, L"%ls obj = %ls[i];\r\n",
                            (PCWStr)*elementType, (PCWStr)*varId);
                        FWRITE("String comma = null;\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (i < %ls.Count - 1)\r\n", (PCWStr)*varId);
                        FWRITE("comma = \",\";\r\nelse\r\ncomma = \"\";\r\n\r\n");

                        if (elementSymbol->mSymbolType == SymbolType_Class) {
                            Boolean isComplex;
                            if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0)->GetChildren()->Get(0), isComplex)))
                                return;

                            FWRITE("if (obj == null)\r\nreturn null;\r\n\r\n");
                            FWRITE("String str = obj.ToString();\r\n");
                            FWRITE("if (str == null)\r\nreturn null;\r\n\r\n");

                            if (isComplex)
                                FWRITE("json = String.Format(\"{0}{1}{2}\", json, str, comma);\r\n");
                            else
                                FWRITE("json = String.Format(\"{0}\\\"{1}\\\"{2}\", json, str, comma);\r\n");

                            FWRITE("if (json == null)\r\nreturn null;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_String) {
                            FWRITE("json = String.Format(\"{0}\\\"{1}\\\"{2}\", json, obj, comma);\r\n");
                            FWRITE("if (json == null)\r\nreturn null;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_ByteArray) {

                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Byte_NULL)
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
                            FWRITE("json = String.Format(\"{0}\\\"{1}\\\"{2}\", json, obj, comma);\r\n");
                            FWRITE("if (json == null)\r\nreturn null;\r\n");
                        }
                        else {
                            DEBUG_PRINT("not support list type!");
                            return;
                        }

                        FWRITE("}\r\n\r\n");
                        FWRITE("return String.Format(\"{0}]\", json);\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null)\r\n", (PCWStr)*varId);
                        FWRITE("return String.Format(\"\");\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 58, L"return %ls;\r\n", (PCWStr)*varId);
                    }
                    else if (symbol->mSymbolType == SymbolType_ByteArray) {

                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8_NULL)
                        || (symbol->mSymbolType == SymbolType_Byte_NULL)
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
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null)\r\n", (PCWStr)*varId);
                        FWRITE("return \"\";\r\nelse\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"return String.Format(\"{0}\", %ls);\r\n", (PCWStr)*varId);
                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8)
                        || (symbol->mSymbolType == SymbolType_Byte)
                        || (symbol->mSymbolType == SymbolType_Int16)
                        || (symbol->mSymbolType == SymbolType_Int32)
                        || (symbol->mSymbolType == SymbolType_Int64)
                        || (symbol->mSymbolType == SymbolType_UInt8)
                        || (symbol->mSymbolType == SymbolType_UInt16)
                        || (symbol->mSymbolType == SymbolType_UInt32)
                        || (symbol->mSymbolType == SymbolType_UInt64)
                        || (symbol->mSymbolType == SymbolType_Float)
                        || (symbol->mSymbolType == SymbolType_Double)
                        || (symbol->mSymbolType == SymbolType_Boolean)) {
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"return String.Format(\"{0}\", %ls);\r\n", (PCWStr)*varId);
                    }
                    else {
                        DEBUG_PRINT("not support type!");
                        return;
                    }
                }
            }
        }
        else {
            Int32 index = 0;
            FWRITE("String json = \"{\";\r\n");
            FWRITE("if (json == null)\r\nreturn null;\r\n\r\n");

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

                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"\\\"%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"String str = %ls.ToString();\r\n", (PCWStr)*varId);
                        FWRITE("if (str == null)\r\nreturn null;\r\n\r\n");
                        if (isComplex)
                            WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":{1}%ls\", json, str);\r\n", (PCWStr)*varId, comma);
                        else
                            WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"{1}\\\"%ls\", json, str);\r\n\r\n", (PCWStr)*varId, comma);

                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_List) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":[]%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"String jsonTmp = String.Format(\"\\\"%ls\\\":[\");\r\n", (PCWStr)*varId);
                        FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"for(Int32 i = 0; i < %ls.Count; ++i) {\r\n", (PCWStr)*varId);

                        Sp<CSymbol> elementSymbol = GetVarSymbol(obj->GetChildren()->Get(0));
                        if (elementSymbol == NULL)
                            return;

                        Sp<String> elementType = GetVarType(
                            obj->GetChildren()->Get(0)->GetChildren()->Get(0), IDL_VAR_TYPE_CS);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 47, L"%ls obj = %ls[i];\r\n",
                            (PCWStr)*elementType, (PCWStr)*varId);
                        FWRITE("String comma = null;\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (i < %ls.Count - 1)\r\n", (PCWStr)*varId);
                        FWRITE("comma = \",\";\r\nelse\r\ncomma = \"\";\r\n\r\n");

                        if (elementSymbol->mSymbolType == SymbolType_Class) {
                            Boolean isComplex;
                            if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0)->GetChildren()->Get(0), isComplex)))
                                return;

                            FWRITE("if (obj == null)\r\nreturn null;\r\n\r\n");
                            FWRITE("String str = obj.ToString();\r\n");
                            FWRITE("if (str == null)\r\nreturn null;\r\n\r\n");
                            if (isComplex)
                                FWRITE("jsonTmp = String.Format(\"{0}{1}{2}\", jsonTmp, str, comma);\r\n");
                            else
                                FWRITE("jsonTmp = String.Format(\"{0}\\\"{1}\\\"{2}\", jsonTmp, str, comma);\r\n\r\n");

                            FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_String) {
                            FWRITE("jsonTmp = String.Format("
                                "\"{0}\\\"{1}\\\"{2}\", jsonTmp, obj, comma);\r\n");
                            FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_ByteArray) {

                        }
                        else if ((elementSymbol->mSymbolType == SymbolType_Int8_NULL)
                            || (elementSymbol->mSymbolType == SymbolType_Byte_NULL)
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
                            FWRITE("jsonTmp = String.Format("
                                "\"{0}\\\"{1}\\\"{2}\", jsonTmp, obj, comma);\r\n");
                            FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n");
                        }
                        else {
                            DEBUG_PRINT("not support list type!");
                            return;
                        }

                        FWRITE("}\r\n\r\n");
                        WRITE_STRING_TO_FILE(117, L"jsonTmp = String.Format("
                            "\"{0}]%ls\", jsonTmp);\r\n", comma);
                        FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n\r\n");
                        FWRITE("json = String.Format(\"{0}{1}\", json, jsonTmp);\r\n");
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"\\\"%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 3 + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"{1}\\\"%ls\", json, %ls);\r\n",
                            (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_ByteArray) {

                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8_NULL)
                        || (symbol->mSymbolType == SymbolType_Byte_NULL)
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
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"\\\"%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"{1}\\\"%ls\", json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n\r\n");
                    }
                    else if ((symbol->mSymbolType == SymbolType_Int8)
                        || (symbol->mSymbolType == SymbolType_Byte)
                        || (symbol->mSymbolType == SymbolType_Int16)
                        || (symbol->mSymbolType == SymbolType_Int32)
                        || (symbol->mSymbolType == SymbolType_Int64)
                        || (symbol->mSymbolType == SymbolType_UInt8)
                        || (symbol->mSymbolType == SymbolType_UInt16)
                        || (symbol->mSymbolType == SymbolType_UInt32)
                        || (symbol->mSymbolType == SymbolType_UInt64)
                        || (symbol->mSymbolType == SymbolType_Float)
                        || (symbol->mSymbolType == SymbolType_Double)
                        || (symbol->mSymbolType == SymbolType_Boolean)) {
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = String.Format("
                            "\"{0}\\\"%ls\\\":\\\"{1}\\\"%ls\", json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == null)\r\nreturn null;\r\n\r\n");
                    }
                    else {
                        DEBUG_PRINT("not support type!");
                        return;
                    }
                }
            } //Foreach

            FWRITE("return String.Format(\"{0}}}\", json);\r\n");
        }

        FWRITE("}\r\n");
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
                failedMsg = String::Create(L"return AResult.AE_FAIL;\r\n");
            else
                failedMsg = String::Create(L"throw new Exception();\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
            failedMsg = String::Create(L"return null;\r\n");
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
            WRITE_STRING_TO_FILE(varId->Length() + sizeof("if (%ls != null) {\r\n"), L"if (%ls != null) {\r\n", (PCWStr)*varId);
            FWRITE("if (AResult.AFAILED(parcel.WriteBoolean(true)))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            WRITE_STRING_TO_FILE(varId->Length() + sizeof("if (AResult.AFAILED(%ls.WriteToParcel(parcel)))\r\n"),
                L"if (AResult.AFAILED(%ls.WriteToParcel(parcel)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("}\r\nelse {\r\nif (AResult.AFAILED(parcel.WriteBoolean(false)))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("}\r\n\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            WRITE_STRING_TO_FILE(varId->Length() + 97,
                L"if (AResult.AFAILED(parcel.WriteInt64(%ls.GetRemoteRef())))\r\n", (PCWStr)*varId);
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
                L"if (AResult.AFAILED(parcel.%ls(%ls)))\r\n", (PCWStr)*name, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }

            Sp<String> elementType = GetVarType(node->GetChildren()->Get(0)->GetChildren()->Get(0),
                IDL_VAR_TYPE_CS | IDL_VAR_TYPE_NOMODIFY);
            if (elementType == NULL)
                return;

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 108,
                    L"Int32 length = 0;\r\nif (%ls == null)\r\nlength = 0;\r\nelse\r\nlength = %ls.Count;\r\n\r\n",
                    (PCWStr)*varId, (PCWStr)*varId);
                FWRITE("string typeStr = \"L\";\r\n");
                FWRITE("Byte[] type = System.Text.Encoding.ASCII.GetBytes(typeStr);\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(type)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("Byte[] lengthArray = System.BitConverter.GetBytes(length);\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(lengthArray)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls != null) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 88,
                    L"foreach(%ls obj in %ls) {\r\nif (obj == null)\r\n", (PCWStr)*elementType, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (AResult.AFAILED(parcel.WriteBoolean(true)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\nif (AResult.AFAILED(obj.WriteToParcel(parcel)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("}\r\n}\r\n}\r\n");
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
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 108,
                    L"Int32 length = 0;\r\nif (%ls == null)\r\nlength = 0;\r\nelse\r\nlength = %ls.Count;\r\n\r\n",
                    (PCWStr)*varId, (PCWStr)*varId);
                FWRITE("string typeStr = \"L\";\r\n");
                FWRITE("Byte[] type = System.Text.Encoding.ASCII.GetBytes(typeStr);\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(type)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("Byte[] lengthArray = System.BitConverter.GetBytes(length);");
                FWRITE("if (AResult.AFAILED(parcel.Write(lengthArray)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls != null) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 70,
                    L"foreach(%ls obj in %ls) {\r\nif (obj == null)\r\n", (PCWStr)*elementType, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                Sp<String> functionName = table->GetValue(elementSymbol->mSymbolType);
                if (functionName == NULL)
                    return;

                WRITE_STRING_TO_FILE(functionName->Length() + 40,
                    L"if (AResult.AFAILED(parcel.%ls(obj)))\r\n", (PCWStr)*functionName);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("}\r\n}\r\n}\r\n\r\n");
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
        Sp<String> varType;
        Sp<String> failedMsg;
        if ((node->GetValue()->mSymbolType == SymbolType_Member) || (node->GetValue()->mSymbolType == SymbolType_Parameter)) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varId = node->GetChildren()->Get(1)->GetValue()->mContent;
            if (node->GetValue()->mSymbolType == SymbolType_Member)
                failedMsg = String::Create(L"return AResult.AE_FAIL;\r\n");
            else
                failedMsg = String::Create(L"throw new Exception();\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
            failedMsg = String::Create(L"return null;\r\n");
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

        varType = GetVarType(node->GetChildren()->Get(0), IDL_VAR_TYPE_CS);
        if (varType == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);
        Sp<HashTable<Int32, String> > table = MapTable::GetReadFromParcelNameOfBasicType();
        if (table == NULL)
            return;

        if (varSymbol->mSymbolType == SymbolType_Class) {
            if (node->GetValue()->mSymbolType == SymbolType_Member) {
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + sizeof("%ls = null;\r\n"), L"%ls = null;\r\n", (PCWStr)*varId);
            }
            else if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 37,
                    L"%ls %ls = null;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                FWRITE("{\r\n");
            }
            else if (node->GetValue()->mSymbolType == SymbolType_Function) {
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 37,
                    L"%ls %ls = null;\r\n", (PCWStr)*varType, (PCWStr)*varId);
            }
            FWRITE("Boolean hasValue = false;\r\nif (AResult.AFAILED(parcel.ReadBoolean(ref hasValue)))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\nif (hasValue) {\r\n");
            FWRITE("Int64 pos = parcel.GetPosition();\r\nString className = null;\r\n"
                "if (AResult.AFAILED(parcel.ReadString(ref className)))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\nparcel.Seek(pos);\r\n");
            WRITE_STRING_TO_FILE(varId->Length() * 2 + varType->Length() + 95,
                L"%ls = (%ls)Activator.CreateInstance(Type.GetType(className), false);\r\nif (%ls == null)\r\n",
                (PCWStr)*varId, (PCWStr)*varType, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            WRITE_STRING_TO_FILE(varId->Length() + 80, L"if (AResult.AFAILED(%ls.ReadFromParcel(parcel)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("}\r\n");
            if (node->GetValue()->mSymbolType == SymbolType_Function)
                WRITE_STRING_TO_FILE(varId->Length() + 20, L"\r\nreturn %ls;\r\n", (PCWStr)*varId);
            else
                FWRITE("}\r\n\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"Int64 _%ls;\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AResult.AFAILED(parcel.ReadInt64(ref _%ls)))\r\n", (PCWStr)*varId);
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
                L"if (AResult.AFAILED(parcel.%ls(ref %ls)))\r\n", (PCWStr)*name, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }

            Sp<String> elementType = GetVarType(
                node->GetChildren()->Get(0)->GetChildren()->Get(0),
                IDL_VAR_TYPE_CS);
            if (elementType == NULL)
                return;

            if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
                WRITE_STRING_TO_FILE(varId->Length() + elementType->Length() + 75,
                    L"List<%ls> %ls = null;\r\n",
                    (PCWStr)*elementType, (PCWStr)*varId);
            }

            WRITE_STRING_TO_FILE(varId->Length() * 2 + elementType->Length() + 75,
                L"{\r\n%ls = new List<%ls>();\r\nif (%ls == null)\r\n",
                (PCWStr)*varId, (PCWStr)*elementType, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            FWRITE("Byte[] type = null;\r\nif (AResult.AFAILED(parcel.Read(ref type, 1)))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            FWRITE("String typeStr = System.Text.Encoding.ASCII.GetString(type);\r\n");
            FWRITE("if (typeStr != \"L\")\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            FWRITE("Byte[] lengthArray = null;\r\n");
            FWRITE("if (AResult.AFAILED(parcel.Read(ref lengthArray, 4)))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\nInt32 length = System.BitConverter.ToInt32(lengthArray, 0);\r\n");
            FWRITE("for (Int32 i = 0; i < length; i++) {\r\n");

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                FWRITE("Boolean hasValue = false;\r\nif (AResult.AFAILED(parcel.ReadBoolean(ref hasValue)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\nif (hasValue) {\r\n");
                FWRITE("Int64 pos = parcel.GetPosition();\r\nString className = null;\r\n"
                    "if (AResult.AFAILED(parcel.ReadString(ref className)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\nparcel.Seek(pos);\r\n");
                WRITE_STRING_TO_FILE(elementType->Length() * 2 + 95,
                    L"%ls obj = (%ls)Activator.CreateInstance(Type.GetType(className), false);\r\nif (%ls == null)\r\n",
                    (PCWStr)*elementType, (PCWStr)*elementType);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (AResult.AFAILED(obj.ReadFromParcel(parcel)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 17, L"%ls.Add(obj);\r\n", (PCWStr)*varId);
                FWRITE("\r\n}\r\n");
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

                WRITE_STRING_TO_FILE(elementType->Length() + 27, L"%ls obj = null;\r\n", (PCWStr)*elementType);
                WRITE_STRING_TO_FILE(functionName->Length() + 80,
                    L"if (AResult.AFAILED(parcel.%ls(ref obj)))\r\n", (PCWStr)*functionName);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 37, L"%ls.Add(obj);\r\n", (PCWStr)*varId);
            }
            else {
                DEBUG_PRINT(L"not support list type!");
                return;
            }

            FWRITE("}\r\n");
            if (node->GetValue()->mSymbolType == SymbolType_Function)
                WRITE_STRING_TO_FILE(varId->Length() + 17, L"return %ls;\r\n", (PCWStr)*varId);

            FWRITE("}\r\n\r\n");
        }
        else {
            DEBUG_PRINT(L"not support type!");
            return;
        }
    }

    STATIC Sp<String> GetBaseClassName(IN TreeNode<CSymbol> * node)
    {
        if ((node->GetChildren()->Get(2) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue() == NULL))
            return NULL;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        if (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue()->mContent == NULL)
            return String::Create(L"AXP.IParcelable");
        else
            return GetTypeReferenceList(node->GetChildren()->Get(2)->GetChildren()->Get(0), IDL_LANG_CS);
    }

    STATIC Void WriteMembersToParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Boolean isInheritInterface = TRUE;
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
                isInheritInterface = FALSE;
        }

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (scopeOfClass == NULL)
            return;

        if (isInheritInterface)
            FWRITE("public virtual Int32 WriteToParcel(Parcel parcel)\r\n");
        else
            FWRITE("public override Int32 WriteToParcel(Parcel parcel)\r\n");

        FWRITE("{\r\n");
        FWRITE("if (parcel == null)\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        WRITE_STRING_TO_FILE(scopeOfClass->Length() + 97, L"if (AResult.AFAILED(parcel.WriteString(\"%ls\")))\r\nreturn AResult.AE_FAIL;\r\n\r\n", (PCWStr)*scopeOfClass);
        if (!isInheritInterface)
            FWRITE("if (AResult.AFAILED(base.WriteToParcel(parcel)))\r\nreturn AResult.AE_FAIL;\r\n\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member)
                WriteVarToParcel(obj);
        }

        FWRITE("return AResult.AS_OK;\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void ReadMembersFromParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Boolean isInheritInterface = TRUE;
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
                isInheritInterface = FALSE;
        }

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (scopeOfClass == NULL)
            return;

        if (isInheritInterface)
            FWRITE("public virtual Int32 ReadFromParcel(Parcel parcel)\r\n");
        else
            FWRITE("public override Int32 ReadFromParcel(Parcel parcel)\r\n");

        FWRITE("{\r\n");
        FWRITE("if (parcel == null)\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        FWRITE("String description = null;\r\n");
        FWRITE("if (AResult.AFAILED(parcel.ReadString(ref description)))\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        if (!isInheritInterface)
            FWRITE("if (AResult.AFAILED(base.ReadFromParcel(parcel)))\r\nreturn AResult.AE_FAIL;\r\n\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member)
                ReadVarFromParcel(obj);
        }

        FWRITE("return AResult.AS_OK;\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void WriteCopyFunction(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        WRITE_STRING_TO_FILE(className->Length() + 42, L"public Boolean Copy(%ls info)\r\n", (PCWStr)*className);
        FWRITE("{\r\n");
        FWRITE("if (info == null)\r\nreturn false;\r\n\r\n");

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
                FWRITE("base.Copy(info);\r\n\r\n");
        }

        Int32 count = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                count++;
                Sp<CSymbol> varSymbol = GetVarSymbol(obj);
                if (varSymbol == NULL) {
                    DEBUG_PRINT("symbol not found!\n");

                    return;
                }

                if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = obj->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                if (varSymbol->mSymbolType == SymbolType_List) {
                    Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_CS);
                    if (varType == NULL)
                        return;

                    if (count > 1)
                        FWRITE("\r\n");

                    WRITE_STRING_TO_FILE(varId->Length() + 25, L"if (info.%ls != null) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 22, L"if (%ls != null) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"%ls.Clear();\r\n}\r\nelse {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 78,
                        L"try {\r\n%ls = new %ls();\r\n}\r\ncatch {\r\nreturn false;\r\n}\r\n}\r\n",
                        (PCWStr)*varId, (PCWStr)*varType);
                    WRITE_STRING_TO_FILE(varId->Length() + 46, L"for (Int32 i = 0; i < info.%ls.Count; ++i)\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 34, L"%ls.Add(info.%ls[i]);\r\n}\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_ByteArray) {
                    if (count > 1)
                        FWRITE("\r\n");

                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (info.%ls != null) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 83,
                        L"try {\r\n%ls = new Byte[info.%ls.Length];\r\n}\r\ncatch {\r\nreturn false;\r\n}\r\n",
                        (PCWStr)*varId, (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 37, L"info.%ls.CopyTo(%ls, 0);\r\n}\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else {
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 19, L"%ls = info.%ls;\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
            }
        }

        FWRITE("\r\nreturn true;\r\n");
        FWRITE("}\r\n");

    }

    STATIC Void WriteSetNullFunction(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Boolean isInheritInterface = TRUE;
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
                isInheritInterface = FALSE;
        }

        if (isInheritInterface)
            FWRITE("public virtual void SetNull()\r\n");
        else
            FWRITE("public override void SetNull()\r\n");

        FWRITE("{\r\n");
        if (!isInheritInterface)
            FWRITE("base.SetNull();\r\n\r\n");

        Int32 count = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            count++;
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                Sp<CSymbol> varSymbol = GetVarSymbol(obj);
                if (varSymbol == NULL) {
                    DEBUG_PRINT("symbol not found!\n");

                    return;
                }

                if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = obj->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                if (varSymbol->mSymbolType == SymbolType_List) {
                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls != null)\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"%ls.Clear();\r\n", (PCWStr)*varId);

                    if (count < node->GetChildren()->GetCount() - 1)
                        FWRITE("\r\n");
                }
                else if ((varSymbol->mSymbolType == SymbolType_Class)
                    || (varSymbol->mSymbolType == SymbolType_Int8_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int16_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int32_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Int64_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt8_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt16_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt32_NULL)
                    || (varSymbol->mSymbolType == SymbolType_UInt64_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Float_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Double_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Boolean_NULL)
                    || (varSymbol->mSymbolType == SymbolType_ByteArray)
                    || (varSymbol->mSymbolType == SymbolType_String)) {
                    WRITE_STRING_TO_FILE(varId->Length() + 18, L"%ls = null;\r\n", (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_Boolean)
                    WRITE_STRING_TO_FILE(varId->Length() + 17, L"%ls = false;\r\n", (PCWStr)*varId);
                else
                    WRITE_STRING_TO_FILE(varId->Length() + 12, L"%ls = 0;\r\n", (PCWStr)*varId);
            }
        }

        FWRITE("}\r\n");
    }

    STATIC Void WriteClass(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        if ((node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        if (gShareAreaCs) {
            Int32 length = 0;
            Wcscpy_s(gShareAreaCs->mDescription, 256, (PCWStr)*description, &length);
            Wcscpy_s(gShareAreaCs->mScope, 256, (PCWStr)*description, &length);
            gShareAreaCs++;
        }
        else {
            Sp<CMappingInfo> mappingInfo = new CMappingInfo();
            if (!mappingInfo)
                return;

            mappingInfo->mDescription = description;
            mappingInfo->mScope = description;

            if (!gMappingCs->InsertUnique((PCWStr)description->GetPayload(), mappingInfo))
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

        WRITE_STRING_TO_FILE(className->Length() + baseClassName->Length() + 37, L"public class %ls : %ls\r\n", (PCWStr)*className, (PCWStr)*baseClassName);
        FWRITE("{\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                WriteClass(obj);
                FWRITE("\r\n");
            }
        }

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

                Sp<String> varType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_CS);
                if (varType == NULL)
                    return;

                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = var->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                if ((var->GetChildren()->Get(0) == NULL) || (var->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                if (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                    WRITE_STRING_TO_FILE(varType->Length() * 2 + varId->Length() + 57,
                    L"public %ls %ls = new %ls();\r\n\r\n", (PCWStr)*varType, (PCWStr)*varId, (PCWStr)*varType);
                else
                    WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 27,
                    L"public %ls %ls;\r\n\r\n", (PCWStr)*varType, (PCWStr)*varId);
            }
        }

        WriteMembersToParcel(node);
        FWRITE("\r\n");
        ReadMembersFromParcel(node);
        FWRITE("\r\n");
        // WriteCopyFunction(node);
        // FWRITE("\r\n");
        // WriteSetNullFunction(node);
        // FWRITE("\r\n");
        WriteToString(node);
        FWRITE("\r\n");

        Boolean isInheritInterface = TRUE;
        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode && baseClassNode->GetChildren()->Get(0)
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
                isInheritInterface = FALSE;
        }

        if (isInheritInterface)
            FWRITE("public virtual String GetTypeName()\r\n");
        else
            FWRITE("public override String GetTypeName()\r\n");

        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(description->Length() + 87, L"return \"%ls\";\r\n", (PCWStr)*description);
        FWRITE("}\r\n\r\n");

        if (isInheritInterface)
            FWRITE("public static IParcelable Create()\r\n{\r\n");
        else
            FWRITE("public static new IParcelable Create()\r\n{\r\n");

        WRITE_STRING_TO_FILE(className->Length() + 37, L"return new %ls();\r\n", (PCWStr)*className);
        FWRITE("}\r\n");

        FWRITE("};\r\n");
    }

    STATIC Void WriteModelsFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 38, L"%ls/%ls/",
            (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/%ls.cs", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        Foreach(String, tmpStr, gHeadListCs) {
            WRITE_STRING_TO_FILE(tmpStr);
        }

        FWRITE("using AXP;\r\n");
        FWRITE("\r\n");

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                WRITE_STRING_TO_FILE(namespacz->Length() + 37, L"namespace %ls\r\n", (PCWStr)*namespacz);
                FWRITE("{\r\n");

                Int32 count = 0;
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        if (count++ > 0)
                            FWRITE("\r\n");

                        WriteClass(tmp);
                    }
                }

                FWRITE("}\r\n");
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_CS);
    }

    Void WriteCsFile(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        Sp<String> fileNameWithoutSuffix = GetOrignalFileName();
        if (fileNameWithoutSuffix == NULL)
            return;

        pthread_key_create(&sKey, NULL);
        Int32 modeFlag = GetMode(node);

        if (modeFlag & IDL_MODE_MODELS)
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

                    if (GetMode(obj) & IDL_MODE_MODELS)
                        WriteModelsFile(obj, fileNameWithoutSuffix);
                }
            }
        }
    }
}