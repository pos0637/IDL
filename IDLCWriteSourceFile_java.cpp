
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
    EXTERN Sp<String> gRootOutputDir;
    EXTERN Sp<String> gNamespacz;
    EXTERN Sp<List<String> > gHeadListJava;
    STATIC pthread_key_t sKey;
    STATIC TreeNode<CSymbol> * sRoot;
    enum _ClassMode { E_PROXY, E_STUB, E_OBJECTHOLDER, E_INTERFACE, E_MODAL };
    enum _TypeId { BASIC, OBJECT, LIST };
    STATIC Sp<String> GetFileName(IN TreeNode<CSymbol> * node);
    STATIC Void WriteJvmImportFile(IN TreeNode<CSymbol> * node, IN enum _ClassMode classType);
    STATIC Int32 GetTypeId(IN TreeNode<CSymbol> * node);
    STATIC Void WriteModelsFile(IN TreeNode<CSymbol> * node);

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

                WRITE_STRING_TO_FILE(funCode->Length() + 77, L"private static final int %ls = %d;\r\n", (PCWStr)*funCode, count++);
            }
        }

        if (!hasConstruction) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 77, L"private static final int %ls = %d;\r\n", (PCWStr)*defConsId, count);
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
                failedMsg = String::Create(L"return AResult.AE_FAIL;\r\n");
            else
                failedMsg = String::Create(L"throw new CException();\r\n");
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
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"if (AResult.AFAILED(parcel.WriteInt64(%ls.GetRemoteRef())))\r\n", (PCWStr)*varId);
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
                L"if (AResult.AFAILED(parcel.%ls(%ls)))\r\n", (PCWStr)*name, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }
            Sp<String> elementType = GetVarType(node->GetChildren()->Get(0)->GetChildren()->Get(0),
                IDL_VAR_TYPE_JAVA | IDL_VAR_TYPE_NOMODIFY);
            if (elementType == NULL)
                return;

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 108,
                    L"int length = 0;\r\nif (%ls == null)\r\nlength = 0;\r\nelse\r\nlength = %ls.size();\r\n\r\n",
                    (PCWStr)*varId, (PCWStr)*varId);
                FWRITE("String typeStr = \"L\";\r\n");
                FWRITE("byte[] type = typeStr.getBytes();\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(type)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("byte[] lengthArray = new byte[4];\r\n");
                FWRITE("lengthArray[0] = (byte) (length >>> 0);\r\nlengthArray[1] = (byte) (length >>> 8);\r\n"
                    "lengthArray[2] = (byte) (length >>> 16);\r\nlengthArray[3] = (byte) (length >>> 24);\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(lengthArray)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls != null) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 70,
                    L"for(%ls obj : %ls) {\r\nif (obj == null)\r\n", (PCWStr)*elementType, (PCWStr)*varId);
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
                    L"int length = 0;\r\nif (%ls == null)\r\nlength = 0;\r\nelse\r\nlength = %ls.size();\r\n\r\n",
                    (PCWStr)*varId, (PCWStr)*varId);
                FWRITE("String typeStr = \"L\";\r\n");
                FWRITE("byte[] type = typeStr.getBytes();\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(type)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("byte[] lengthArray = new byte[4];\r\n");
                FWRITE("lengthArray[0] = (byte) (length >>> 0);\r\nlengthArray[1] = (byte) (length >>> 8);\r\n"
                    "lengthArray[2] = (byte) (length >>> 16);\r\nlengthArray[3] = (byte) (length >>> 24);\r\n");
                FWRITE("if (AResult.AFAILED(parcel.Write(lengthArray)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (%ls != null) {\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 70,
                    L"for(%ls obj : %ls) {\r\nif (obj == null)\r\n", (PCWStr)*elementType, (PCWStr)*varId);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                Sp<String> functionName = table->GetValue(elementSymbol->mSymbolType);
                if (functionName == NULL)
                    return;

                WRITE_STRING_TO_FILE(functionName->Length() + 40,
                    L"if (AResult.AFAILED(parcel.%ls(obj)))\r\n", (PCWStr)*functionName);
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
        Sp<String> varType;
        Sp<String> failedMsg;

        if (node->GetValue()->mSymbolType == SymbolType_Member) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varId = node->GetChildren()->Get(1)->GetValue()->mContent;
            failedMsg = String::Create(L"return AResult.AE_FAIL;\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varId = node->GetChildren()->Get(1)->GetValue()->mContent;
            failedMsg = String::Create(L"return null;\r\n");
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
            failedMsg = String::Create(L"throw new CException();\r\n");
        }
        else
            varId = NULL;

        if (varId == NULL)
            return;

        if (failedMsg == NULL)
            return;

        varType = GetVarType(node->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
        if (varType == NULL)
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
            if (node->GetValue()->mSymbolType == SymbolType_Member) {
                FWRITE("try {\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + sizeof("%ls = null;\r\n"), L"%ls = null;\r\n", (PCWStr)*varId);
            }
            else if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 37,
                    L"%ls %ls = null;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                FWRITE("try {\r\n");
            }
            else if (node->GetValue()->mSymbolType == SymbolType_Function) {
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 37,
                    L"%ls %ls = null;\r\n", (PCWStr)*varType, (PCWStr)*varId);
            }
            FWRITE("boolean hasValue = parcel.ReadBoolean();\r\nif (hasValue) {\r\n");
            FWRITE("long pos = parcel.GetPosition();\r\nString className = parcel.ReadString();\r\nparcel.Seek(pos);\r\n");
            WRITE_STRING_TO_FILE(varId->Length() * 2 + varType->Length() + 80,
                L"%ls = (%ls)Class.forName(className).newInstance();\r\nif (%ls == null)\r\n",
                (PCWStr)*varId, (PCWStr)*varType, (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            WRITE_STRING_TO_FILE(varId->Length() + 80, L"if (AResult.AFAILED(%ls.ReadFromParcel(parcel)))\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("}\r\n");
            if (node->GetValue()->mSymbolType == SymbolType_Function) {
                WRITE_STRING_TO_FILE(varId->Length() + 20, L"\r\nreturn %ls;\r\n", (PCWStr)*varId);
            }
            else {
                FWRITE("}\r\ncatch (Exception e) {\r\ne.printStackTrace();\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("}\r\n\r\n");
            }
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            if (node->GetValue()->mSymbolType == SymbolType_Function)
                FWRITE("rturn parcel.ReadInt64();\r\n");
            else if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
                Sp<String> className = varType;
                if (className == NULL)
                    return;

                WRITE_STRING_TO_FILE(varId->Length() + 97, L"long _%ls = parcel.ReadInt64();\r\n", (PCWStr)*varId);
                FWRITE("String token = parcel.ReadString();\r\n");
                WRITE_STRING_TO_FILE(varType->Length() + className->Length() + varId->Length() * 2 + 77,
                    L"%ls %ls = %ls.Create(_%ls, token);\r\n",
                    (PCWStr)*varType, (PCWStr)*varId, (PCWStr)*className, (PCWStr)*varId);
            }
            else {
                WRITE_STRING_TO_FILE(varId->Length() + 97, L"long _%ls;\r\n", (PCWStr)*varId);
                WRITE_STRING_TO_FILE(varId->Length() + 97,
                    L"_%ls = parcel.ReadInt64();\r\n", (PCWStr)*varId);
            }
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

            if (node->GetValue()->mSymbolType == SymbolType_Function)
                FWRITE("return ");
            else if (node->GetValue()->mSymbolType == SymbolType_Parameter)
                WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 27,
                L"%ls %ls = ", (PCWStr)*varType, (PCWStr)*varId);
            else
                WRITE_STRING_TO_FILE(varId->Length() + 27, L"%ls = ", (PCWStr)*varId);

            WRITE_STRING_TO_FILE(name->Length() + 17, L"parcel.%ls();\r\n", (PCWStr)*name);
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }
            Sp<String> elementType = GetVarType(
                node->GetChildren()->Get(0)->GetChildren()->Get(0),
                IDL_VAR_TYPE_JAVA | IDL_VAR_TYPE_NOMODIFY);
            if (elementType == NULL)
                return;

            if (node->GetValue()->mSymbolType == SymbolType_Member) {
                WRITE_STRING_TO_FILE(varId->Length() + elementType->Length() + 65,
                    L"%ls = new LinkedList<%ls>();\r\n", (PCWStr)*varId, (PCWStr)*elementType);
                FWRITE("try {\r\n");
            }
            else if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
                WRITE_STRING_TO_FILE(varId->Length() + elementType->Length() * 2 + 65,
                    L"List<%ls> %ls = new LinkedList<%ls>();\r\n", (PCWStr)*elementType, (PCWStr)*varId, (PCWStr)*elementType);
                FWRITE("try {\r\n");
            }
            else if (node->GetValue()->mSymbolType == SymbolType_Function) {
                WRITE_STRING_TO_FILE(varId->Length() + elementType->Length() * 2 + 65,
                    L"List<%ls> %ls = new LinkedList<%ls>();\r\n", (PCWStr)*elementType, (PCWStr)*varId, (PCWStr)*elementType);
            }
            FWRITE("byte[] type = parcel.Read(1);\r\n");
            FWRITE("String typeStr = new String(type);\r\n");
            FWRITE("if (!typeStr.equals(\"L\"))\r\n");
            WRITE_STRING_TO_FILE(failedMsg);
            FWRITE("\r\n");
            FWRITE("byte [] lengthArray = parcel.Read(4);\r\n");
            FWRITE("int length = (((lengthArray[0] << 0) & 0x000000ff) | ((lengthArray[1] << 8) & 0x0000ff00)"
                " | ((lengthArray[2] << 16) & 0x00ff0000) | ((lengthArray[3] << 24) & 0xff000000));\r\n");
            FWRITE("for (int i = 0; i < length; i++) {\r\n");

            if (elementSymbol->mSymbolType == SymbolType_Class) {
                FWRITE("Boolean hasValue = parcel.ReadBoolean();\r\nif (hasValue) {\r\n");
                FWRITE("long pos = parcel.GetPosition();\r\nString className = parcel.ReadString();\r\nparcel.Seek(pos);\r\n");
                WRITE_STRING_TO_FILE(elementType->Length() * 2 + 80,
                    L"%ls obj = (%ls)Class.forName(className).newInstance();\r\nif (obj == null)\r\n",
                    (PCWStr)*elementType, (PCWStr)*elementType);
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                FWRITE("if (AResult.AFAILED(obj.ReadFromParcel(parcel)))\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("\r\n");
                WRITE_STRING_TO_FILE(varId->Length() + 17, L"%ls.add(obj);\r\n", (PCWStr)*varId);
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

                WRITE_STRING_TO_FILE(elementType->Length() + functionName->Length() + 37,
                    L"%ls obj = parcel.%ls();\r\n", (PCWStr)*elementType, (PCWStr)*functionName);
                WRITE_STRING_TO_FILE(varId->Length() + 17, L"%ls.add(obj);\r\n", (PCWStr)*varId);
            }
            else {
                DEBUG_PRINT(L"not support list type!");
                return;
            }

            FWRITE("}\r\n");
            if (node->GetValue()->mSymbolType == SymbolType_Function)
                WRITE_STRING_TO_FILE(varId->Length() + 17, L"return %ls;\r\n", (PCWStr)*varId);
            else {
                FWRITE("}\r\ncatch (Exception e) {\r\n");
                FWRITE("e.printStackTrace();\r\n");
                WRITE_STRING_TO_FILE(failedMsg);
                FWRITE("}\r\n\r\n");
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

            Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
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

    Sp<String> GetTypeOfParameters(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return NULL;

        Sp<String> parameterList = String::Create(L"");
        if (parameterList == NULL)
            return NULL;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if ((obj == NULL) || (obj->GetValue() == NULL) || (obj->GetValue()->mSymbolType != SymbolType_Parameter))
                return NULL;

            Sp<String> varType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
            if (varType == NULL)
                return NULL;

            if ((obj->GetChildren()->Get(1) == NULL) || (obj->GetChildren()->Get(1)->GetValue() == NULL))
                return NULL;

            Sp<String> varId = obj->GetChildren()->Get(1)->GetValue()->mContent;
            if (varId == NULL)
                return NULL;

            if (parameterList->Equals(L""))
                parameterList = String::Create(varType->Length() + 7, L"%ls.class", (PCWStr)*varType);
            else
                parameterList = String::Create(parameterList->Length() + varType->Length() + 37,
                L"%ls, %ls.class", (PCWStr)*parameterList, (PCWStr)*varType);

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
                FWRITE("try {\r\n");

                ReadParametersFromParcel(obj->GetChildren()->Get(2));
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
                        L"mService.%ls(%ls);\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);

                    FWRITE("parcel.Reset();\r\n");
                    FWRITE("if (AResult.AFAILED(parcel.WriteInt32(IpcException.NoException)))\r\nreturn null;\r\n");
                }
                else {
                    Sp<String> returnType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
                    if (returnType == NULL)
                        return;

                    WRITE_STRING_TO_FILE(77 + returnType->Length() + functionName->Length() + realParameterList->Length(),
                        L"%ls ret = mService.%ls(%ls);\r\n",
                        (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*realParameterList);

                    FWRITE("parcel.Reset();\r\n");
                    FWRITE("if (AResult.AFAILED(parcel.WriteInt32(IpcException.NoException)))\r\nreturn null;\r\n\r\n");

                    WriteVarToParcel(obj);
                }

                FWRITE("}\r\ncatch (Throwable e) {\r\ne.printStackTrace();\r\nreturn null;\r\n}\r\n}\r\n");
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

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> description = String::Create(namespacz->Length() + className->Length() + 2,
            L"%ls.%ls", (PCWStr)*namespacz, (PCWStr)*className);
        if (description == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("public Parcel OnTransact(Parcel parcel, String uri)\r\n{\r\n");
        FWRITE("if (parcel == null)\r\nreturn null;\r\n\r\n");
        FWRITE("int funCode;\r\ntry {\r\nfunCode = parcel.ReadInt32();\r\n}\r\n"
            "catch (Exception e) {\r\nreturn null;\r\n}\r\n\r\n");

        Int32 countOfConstruction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_Construction)) {
                Sp<String> consCodeId = GetConsOrFuncId(obj);
                if (consCodeId == NULL)
                    return;

                WRITE_STRING_TO_FILE(consCodeId->Length() + 40, L"%lsif (funCode == %ls) {\r\n",
                    countOfConstruction++ == 0 ? L"" : L"else ", (PCWStr)*consCodeId);
                FWRITE("if (mService == null) {\r\n");

                Foreach(TreeNode<CSymbol>, param, obj->GetChildren()->Get(1)->GetChildren()) {
                    if ((param == NULL) || (param->GetValue() == NULL))
                        return;

                    Sp<String> varType = GetVarType(param->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
                    Sp<String> varId = GetVarId(param);
                    if ((varType == NULL) || (varId == NULL))
                        return;

                    WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 7, L"%ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
                }

                ReadParametersFromParcel(obj->GetChildren()->Get(1));

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                Sp<String> params = GetTypeOfParameters(obj->GetChildren()->Get(1));
                if (params == NULL)
                    return;

                FWRITE("synchronized (this) {\r\nif (mService == null) {\r\n"
                    "String packageString = ServiceManager.GetServicePackage");
                WRITE_STRING_TO_FILE(description->Length() + 17, L"(\"%ls\");\r\n", (PCWStr)*description);
                FWRITE("if (packageString == null)\r\nreturn null;\r\n\r\ntry {\r\nClass<?> cls = "
                    "Class.forName(packageString);\r\n");
                WRITE_STRING_TO_FILE(params->Length() + 51,
                    L"Method method = cls.getMethod(\"Create\", %ls);\r\n", (PCWStr)*params);
                WRITE_STRING_TO_FILE(className->Length() + realParameterList->Length() + 33,
                    L"mService  = (I%ls)method.invoke(null, %ls);\r\n",
                    (PCWStr)*className, (PCWStr)*realParameterList);
                FWRITE("if (mService == null)\r\nreturn null;\r\n}\r\ncatch (Exception e) {\r\n"
                    "e.printStackTrace();\r\nreturn null;\r\n}\r\n}\r\n}\r\n}\r\n\r\n");
                FWRITE("if (uri == null) {\r\nif (!ObjectManager.RegisterObject((long)this.hashCode()"
                    ", mService))\r\nreturn null;\r\n}\r\n\r\n");
                FWRITE("parcel.Reset();\r\nif (AResult.AFAILED(parcel.WriteInt32(IpcException.NoException)))"
                    "\r\nreturn null;\r\n\r\nif (AResult.AFAILED(parcel.WriteInt64((long)this.hashCode())))\r\n"
                    "return null;\r\n}\r\n");
            }
        }

        if (countOfConstruction == 0) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 40, L"if (funCode == %ls) {\r\n", (PCWStr)*defConsId);
            FWRITE("if (mService == null) {\r\n");
            FWRITE("synchronized (this) {\r\nif (mService == null) {\r\n"
                "String packageString = ServiceManager.GetServicePackage");
            WRITE_STRING_TO_FILE(description->Length() + 17, L"(\"%ls\");\r\n", (PCWStr)*description);
            FWRITE("if (packageString == null)\r\nreturn null;\r\n\r\ntry {\r\nClass<?> cls = "
                "Class.forName(packageString);\r\n"
                "Method method = cls.getMethod(\"Create\", (Class<?>[])null);\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 77,
                L"mService  = (I%ls)method.invoke(null, (Object[])null);\r\n", (PCWStr)*className);
            FWRITE("if (mService == null)\r\nreturn null;\r\n}\r\ncatch (Exception e) {\r\n"
                "e.printStackTrace();\r\nreturn null;\r\n}\r\n}\r\n}\r\n}\r\n\r\n");
            FWRITE("if (uri == null) {\r\nif (!ObjectManager.RegisterObject((long)this.hashCode()"
                ", mService))\r\nreturn null;\r\n}\r\n\r\n");
            FWRITE("parcel.Reset();\r\nif (AResult.AFAILED(parcel.WriteInt32(IpcException.NoException)))"
                "\r\nreturn null;\r\n\r\nif (AResult.AFAILED(parcel.WriteInt64((long)this.hashCode())))\r\n"
                "return null;\r\n}\r\n");
        }

        FWRITE("else {\r\nif (mService == null) {\r\nparcel.Reset();\r\nif (AResult.AFAILED(parcel."
            "WriteInt32(IpcException.RemoteRefException)))\r\nreturn null;\r\n\r\nreturn parcel;\r\n}\r\n\r\n");

        WriteObjectHolderStubFunction(node);

        FWRITE("}\r\n\r\nreturn parcel;\r\n}\r\n");
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
                    L"public static %ls Create(%ls) throws Throwable\r\n", (PCWStr)*className, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(className->Length() + realParameterList->Length() + 37,
                    L"return new %ls(%ls);\r\n", (PCWStr)*className, (PCWStr)*realParameterList);
                FWRITE("}\r\n\r\n");
            }
        }

        if (!hasConstructions) {
            WRITE_STRING_TO_FILE(className->Length() + 77, L"public static %ls Create() throws Throwable\r\n", (PCWStr)*className);
            FWRITE("{\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 37,
                L"return new %ls();\r\n", (PCWStr)*className);
            FWRITE("}\r\n\r\n");
        }

        WRITE_STRING_TO_FILE(className->Length() + 19,
            L"public static %ls", (PCWStr)*className);
        FWRITE(" Create(long objRef, String token) throws Throwable\r\n{\r\n");
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

                WRITE_STRING_TO_FILE(className->Length() + formParameterList->Length() + 71,
                    L"protected %ls(%ls) throws Throwable\r\n", (PCWStr)*className, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                FWRITE("mTag = (byte)0xA3;\r\nmToken = null;\r\nmInterface = null;\r\n");
                FWRITE("Parcel parcel = new Parcel();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 57,
                    L"IStub stub = ServiceManager.GetService(%ls);\r\n", (PCWStr)*descptorName);
                FWRITE("if (stub == null) {\r\n");
                FWRITE("mIsRemote = true;\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 117,
                    L"mConn = CServerConnection.Create(%ls);\r\nif (mConn == null)\r\nthrow new CException();\r\n\r\n",
                    (PCWStr)*descptorName);
                FWRITE("if (AResult.AFAILED(parcel.WriteInt8(mTag)))\r\nthrow new CException();\r\n\r\n"
                    "if (AResult.AFAILED(parcel.WriteString(mToken)))\r\nthrow new CException();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 77,
                    L"if (AResult.AFAILED(parcel.WriteString(%ls)))\r\nthrow new CException();\r\n", (PCWStr)*descptorName);
                FWRITE("}\r\nelse {\r\n");
                FWRITE("mIsRemote = false;\r\nmConn = stub;\r\n");
                FWRITE("}\r\n\r\n");
                FWRITE("if (AResult.AFAILED(parcel.WriteInt32(CommandCode.COMMAND_CREATE)))\r\nthrow new CException();\r\n\r\n");
                FWRITE("if (AResult.AFAILED(parcel.WriteBoolean(mIsRemote)))\r\nthrow new CException();\r\n\r\n");
                FWRITE("if (mIsRemote) {\r\n");
                FWRITE("if (AResult.AFAILED(parcel.WriteString(ServiceManager.sServerAddress)))\r\nthrow new CException();\r\n");
                FWRITE("}\r\n\r\n");
                WRITE_STRING_TO_FILE(consCodeId->Length() + 70, L"if (AResult.AFAILED(parcel.WriteInt32(%ls)))\r\n", (PCWStr)*consCodeId);
                FWRITE("throw new CException();\r\n\r\n");

                WriteParametersToParcel(obj->GetChildren()->Get(1));

                FWRITE("parcel.Reset();\r\n");
                FWRITE("parcel = mConn.Transact(parcel);\r\nif (parcel == null)\r\nthrow new CException();\r\n\r\n");
                FWRITE("parcel.Reset();\r\nint code = parcel.ReadInt32();\r\nIpcException.ReadException(code);\r\n");
                FWRITE("mRef = parcel.ReadInt64();\r\n");
                FWRITE("if (!mIsRemote) {\r\n");
                WRITE_STRING_TO_FILE(className->Length() + 77,
                    L"mInterface = (I%ls)ObjectManager.GetObject(mRef);\r\n", (PCWStr)*className);
                FWRITE("if (mInterface == null)\r\nthrow new CException();\r\n}\r\n");
                FWRITE("}\r\n\r\n");
            }
        }

        if (!hasConstructions) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(className->Length() + 71,
                L"protected %ls() throws Throwable\r\n", (PCWStr)*className);
            FWRITE("{\r\n");
            FWRITE("mTag = (byte)0xA3;\r\nmToken = null;\r\nmInterface = null;\r\n");
            FWRITE("Parcel parcel = new Parcel();\r\n\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 57,
                L"IStub stub = ServiceManager.GetService(%ls);\r\n", (PCWStr)*descptorName);
            FWRITE("if (stub == null) {\r\n");
            FWRITE("mIsRemote = true;\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 117,
                L"mConn = CServerConnection.Create(%ls);\r\nif (mConn == null)\r\nthrow new CException();\r\n\r\n",
                (PCWStr)*descptorName);
            FWRITE("if (AResult.AFAILED(parcel.WriteInt8(mTag)))\r\nthrow new CException();\r\n\r\n"
                "if (AResult.AFAILED(parcel.WriteString(mToken)))\r\nthrow new CException();\r\n\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + 77,
                L"if (AResult.AFAILED(parcel.WriteString(%ls)))\r\nthrow new CException();\r\n", (PCWStr)*descptorName);
            FWRITE("}\r\nelse {\r\n");
            FWRITE("mIsRemote = false;\r\nmConn = stub;\r\n");
            FWRITE("}\r\n\r\n");
            FWRITE("if (AResult.AFAILED(parcel.WriteInt32(CommandCode.COMMAND_CREATE)))\r\nthrow new CException();\r\n\r\n");
            FWRITE("if (AResult.AFAILED(parcel.WriteBoolean(mIsRemote)))\r\nthrow new CException();\r\n\r\n");
            FWRITE("if (mIsRemote) {\r\n");
            FWRITE("if (AResult.AFAILED(parcel.WriteString(ServiceManager.sServerAddress)))\r\nthrow new CException();\r\n");
            FWRITE("}\r\n\r\n");
            WRITE_STRING_TO_FILE(defConsId->Length() + 70, L"if (AResult.AFAILED(parcel.WriteInt32(%ls)))\r\n", (PCWStr)*defConsId);
            FWRITE("throw new CException();\r\n\r\n");
            FWRITE("parcel.Reset();\r\n");
            FWRITE("parcel = mConn.Transact(parcel);\r\nif (parcel == null)\r\nthrow new CException();\r\n\r\n");
            FWRITE("parcel.Reset();\r\nint code = parcel.ReadInt32();\r\nIpcException.ReadException(code);\r\n");
            FWRITE("mRef = parcel.ReadInt64();\r\n");
            FWRITE("if (!mIsRemote) {\r\n");
            WRITE_STRING_TO_FILE(className->Length() + 77,
                L"mInterface = (I%ls)ObjectManager.GetObject(mRef);\r\n", (PCWStr)*className);
            FWRITE("if (mInterface == null)\r\nthrow new CException();\r\n}\r\n");
            FWRITE("}\r\n\r\n");
        }

        WRITE_STRING_TO_FILE(className->Length() + 70, L"protected %ls(long objRef, String token) throws Throwable\r\n", (PCWStr)*className);
        FWRITE("{\r\n");
        FWRITE("mTag = (byte)0xA4;\r\nmToken = token;\r\nmInterface = null;\r\nmIsRemote = true;\r\nmRef = objRef;\r\n\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 110,
            L"mConn = CServerConnection.Create(%ls);\r\nif (mConn == null)\r\nthrow new CException();\r\n\r\n", (PCWStr)*descptorName);
        /*FWRITE("Parcel parcel = new Parcel();\r\n\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 88,
        L"if (AResult.AFAILED(parcel.WriteString(%ls)))\r\nthrow new CException();\r\n\r\n", (PCWStr)*descptorName);
        FWRITE("if (AResult.AFAILED(parcel.WriteInt32(CommandCode.COMMAND_CALLBACK)))\r\nthrow new CException();\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteString(ServiceManager.sServerAddress)))\r\nthrow new CException();\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt64(mRef)))\r\nthrow new CException();\r\n\r\n");
        FWRITE("parcel.Reset();\r\n");
        FWRITE("parcel = mConn.Transact(parcel);\r\n");
        FWRITE("if (parcel == null)\r\nthrow new CException();\r\n\r\n");
        FWRITE("parcel.Reset();\r\nint code = parcel.ReadInt32();\r\nIpcException.ReadException(code);\r\n");
        FWRITE("long obj = parcel.ReadInt64();\r\n");
        FWRITE("if (obj != mRef)\r\nthrow new CException();\r\n");*/
        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteProxyDestruction(IN TreeNode<CSymbol> * node)
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

        FWRITE("public void finalize()\r\n");
        FWRITE("{\r\n");
        FWRITE("if (mToken != null)\r\nreturn;\r\n");
        FWRITE("try {\r\n");
        FWRITE("mInterface = null;\r\n");
        FWRITE("Parcel parcel = new Parcel();\r\n\r\n");
        FWRITE("if (mIsRemote) {\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt8(mTag)))\r\nthrow new CException();\r\n\r\n"
            "if (AResult.AFAILED(parcel.WriteString(mToken)))\r\nthrow new CException();\r\n\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + 70,
            L"if (AResult.AFAILED(parcel.WriteString(%ls)))\r\nthrow new CException();\r\n", (PCWStr)*descptorName);
        FWRITE("}\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt32(CommandCode.COMMAND_RELEASE)))\r\nthrow new CException();\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteBoolean(mIsRemote)))\r\nthrow new CException();\r\n\r\n");
        FWRITE("if (mIsRemote)\r\n {\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteString(ServiceManager.sServerAddress)))\r\nthrow new CException();\r\n");
        FWRITE("}\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt64(mRef)))\r\nthrow new CException();\r\n\r\n");
        FWRITE("parcel.Reset();\r\n");
        FWRITE("parcel = mConn.Transact(parcel);\r\n");
        FWRITE("if ((mTag & 0x0F) == 0x04)\r\nreturn;\r\n\r\n");
        FWRITE("if (parcel == null)\r\nthrow new CException();\r\n\r\n");
        FWRITE("parcel.Reset();\r\nint code = parcel.ReadInt32();\r\nIpcException.ReadException(code);\r\n");
        FWRITE("}\r\ncatch(Throwable e){\r\nreturn;}\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void WriteProxyFunctions(IN TreeNode<CSymbol> * node)
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

        Sp<String> descptorName = GetDESCRIPTORId(node);
        if (descptorName == NULL)
            return;

        Boolean hasConstructions = FALSE;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_Function) {
                Sp<String> returnType = GetVarType(obj->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
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
                            addRemoteRef = String::Create(varId->Length() + descptorName->Length() + 77,
                                L"%ls.AddRemoteRef(ServiceManager.GetProxyAddr(%ls));\r\n",
                                (PCWStr)*varId, (PCWStr)*descptorName);
                            if (addRemoteRef == NULL)
                                return;
                        }
                    }
                }

                WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length() + formParameterList->Length() + 45,
                    L"public %ls %ls(%ls) throws Throwable\r\n", (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                FWRITE("{\r\n");
                FWRITE("if (mIsRemote) {\r\n");
                FWRITE("Parcel parcel = new Parcel();\r\n\r\n");
                FWRITE("if (AResult.AFAILED(parcel.WriteInt8(mTag)))\r\nthrow new CException();\r\n\r\n"
                    "if (AResult.AFAILED(parcel.WriteString(mToken)))\r\nthrow new CException();\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 90,
                    L"if (AResult.AFAILED(parcel.WriteString(%ls)))\r\nthrow new CException(); \r\n\r\n", (PCWStr)*descptorName);
                FWRITE("if (AResult.AFAILED(parcel.WriteInt32(CommandCode.COMMAND_CALL)))\r\nthrow new CException();\r\n\r\n");
                FWRITE("if (AResult.AFAILED(parcel.WriteInt64(mRef)))\r\nthrow new CException();\r\n\r\n");
                WRITE_STRING_TO_FILE(funcId->Length() + 70, L"if (AResult.AFAILED(parcel.WriteInt32(%ls)))\r\n", (PCWStr)*funcId);
                FWRITE("throw new CException();\r\n\r\n");

                WriteParametersToParcel(obj->GetChildren()->Get(2));

                FWRITE("parcel.Reset();\r\n");
                FWRITE("parcel = mConn.Transact(parcel);\r\n");


                if (obj->GetChildren()->Get(0)
                    && obj->GetChildren()->Get(0)->GetValue()
                    && (obj->GetChildren()->Get(0)->GetValue()->mSymbolType != SymbolType_Void)) {
                    FWRITE("if ((mTag & 0x0F) == 0x04)\r\n");
                    SymbolType type = obj->GetChildren()->Get(0)->GetValue()->mSymbolType;
                    if ((type == SymbolType_Int8)
                        || (type == SymbolType_Byte)
                        || (type == SymbolType_Int16)
                        || (type == SymbolType_Int32)
                        || (type == SymbolType_Int64)
                        || (type == SymbolType_Double)
                        || (type == SymbolType_Boolean))
                        FWRITE("return 0;\r\n\r\n");
                    else
                        FWRITE("return null;\r\n\r\n");

                    FWRITE("if (parcel == null)\r\nthrow new CException();\r\n\r\n");
                    FWRITE("parcel.Reset();\r\nint code = parcel.ReadInt32();\r\nIpcException.ReadException(code);\r\n");
                    WRITE_STRING_TO_FILE(addRemoteRef);
                    ReadVarFromParcel(obj);
                }
                else {
                    FWRITE("if ((mTag & 0x0F) == 0x04)\r\nreturn;\r\n\r\n");
                    FWRITE("if (parcel == null)\r\nthrow new CException();\r\n\r\n");
                    FWRITE("parcel.Reset();\r\nint code = parcel.ReadInt32();\r\nIpcException.ReadException(code);\r\n");
                    WRITE_STRING_TO_FILE(addRemoteRef);
                    FWRITE("return;\r\n");
                }

                FWRITE("}\r\nelse {\r\n");
                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(2));
                if (realParameterList == NULL)
                    return;

                if (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_Void)
                    WRITE_STRING_TO_FILE(functionName->Length() + realParameterList->Length() + 37,
                    L"mInterface.%ls(%ls);\r\nreturn;\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);
                else
                    WRITE_STRING_TO_FILE(functionName->Length() + realParameterList->Length() + 37,
                    L"return mInterface.%ls(%ls);\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);

                FWRITE("}\r\n");
                FWRITE("}\r\n\r\n");
            }
        }
    }

    STATIC Void WriteProxyGetRemoteRef(IN TreeNode<CSymbol> * node)
    {
        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("public long GetRemoteRef()\r\n{\r\nreturn mRef;\r\n}\r\n\r\n");
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

        FWRITE("private byte mTag;\r\n");
        FWRITE("private boolean mIsRemote;\r\n");
        FWRITE("private long mRef;\r\n");
        FWRITE("private IStub mConn;\r\n");
        FWRITE("private String mToken;\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 58, L"private I%ls mInterface;\r\n", (PCWStr)*className);

        WriteFunCodeMembers(node);

        WRITE_STRING_TO_FILE(descptorName->Length() + namespacz->Length() + className->Length() + 87,
            L"private static final String %ls = \"%ls.%ls\";\r\n",
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

        Sp<HashTable<PCWStr, String> > importFiles = new HashTable<PCWStr, String>(50);
        if (importFiles == NULL)
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
                    Sp<String> s = fileName->SubString(0, index2);
                    if (obj == NULL)
                        return;

                    PWStr pwstr = (PWStr)s->GetPayload();
                    while (*pwstr) {
                        if (*pwstr == L'/')
                            *pwstr = L'.';

                        pwstr++;
                    }

                    WRITE_STRING_TO_FILE(s->Length() + 22, L"import %ls;\r\n", (PCWStr)*s);
                    continue;
                }

                Int32 modeFlag = GetMode(obj);
                Sp<String> objNamespace = GetNameSpace(obj);
                if (objNamespace == NULL)
                    return;

                Sp<String> importFile = NULL;
                if (modeFlag & IDL_MODE_IPC)
                    importFile = String::Create(objNamespace->Length() + 77,
                    L"import IPC.gen.%ls.*;\r\n", (PCWStr)*objNamespace);
                else
                    importFile = String::Create(objNamespace->Length() + 77,
                    L"import %ls.*;\r\n", (PCWStr)*objNamespace);

                if (importFile == NULL)
                    return;

                if (!importFiles->Contains(*importFile)) {
                    if (!importFiles->InsertUnique(*importFile, importFile))
                        return;
                }
            }
        }

        Sp<List<String> > headerFiles = importFiles->GetValues();
        if (headerFiles) {
            Foreach(String, obj, headerFiles) {
                if (obj)
                    WRITE_STRING_TO_FILE(obj);
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

    STATIC Void WriteStubFile(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
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
        Sp<String> fileNameWithoutSuffix = className;
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + stub->Length() + 15,
            L"%ls/%ls%ls.java", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix, (PCWStr)*stub);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(szNamespace->Length() + 77, L"package IPC.gen.%ls;\r\n\r\n", (PCWStr)*szNamespace);

        FWRITE("import java.util.Collection;\r\nimport java.util.Hashtable;\r\nimport java.util.Iterator;\r\n"
            "import AXP.AResult;\r\nimport AXP.Parcel;\r\nimport IPC.java.CObjectHolder;\r\nimport IPC.java.CommandCode;\r\n"
            "import IPC.java.IStub;\r\nimport IPC.java.IpcException;\r\nimport IPC.java.ObjectManager;\r\n"
            "import IPC.java.ServiceManager;\r\nimport IPC.java.UriManager;\r\n\r\n");
        WRITE_STRING_TO_FILE(className->Length() + stub->Length() + 38,
            L"public class %ls%ls implements IStub\r\n{\r\n", (PCWStr)*className, (PCWStr)*stub);
        FWRITE("private Hashtable<Long, CObjectHolder> mServiceList;\r\n\r\n");
        WRITE_STRING_TO_FILE(className->Length() + stub->Length() + 38,
            L"public %ls%ls()\r\n", (PCWStr)*className, (PCWStr)*stub);
        FWRITE("{\r\nmServiceList = new Hashtable<Long, CObjectHolder>(50);\r\n"
            "if (mServiceList == null)\r\nthrow new NullPointerException();\r\n}\r\n\r\n");
        FWRITE("public void AddRemoteRef(String uri, long objRef)\r\n"
            "{\r\nif (mServiceList == null)\r\nreturn;\r\n\r\nCObjectHolder obj"
            " = mServiceList.get(objRef);\r\nif ((obj != null) && (uri != null)){\r\nUriManager.StartThread(uri);\r\n"
            "obj.mUriList.add(uri);\r\nobj.AddRemoteRef();\r\n}\r\n}\r\n\r\n");
        FWRITE("public Parcel Transact(Parcel bundle)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (bundle == null)\r\nreturn null;\r\n\r\n");
        FWRITE("int code;\r\nString uri = null;\r\n");
        FWRITE("try {\r\n code = bundle.ReadInt32();\r\n");
        FWRITE("switch (code) {\r\ncase CommandCode.COMMAND_CREATE:\r\n"
            "case CommandCode.COMMAND_RELEASE:\r\nboolean isRemote = bundle.ReadBoolean();\r\n"
            "if (isRemote)\r\nuri = bundle.ReadString();\r\n\r\nbreak;\r\n"
            "case CommandCode.COMMAND_CALLBACK:\r\nuri = bundle.ReadString();\r\n\r\nbreak;\r\n"
            "case CommandCode.COMMAND_CALL:\r\nbreak;\r\ndefault:\r\nreturn null;\r\n}\r\n}"
            "\r\ncatch (Exception e) {\r\ne.printStackTrace();\r\nreturn null;\r\n}\r\n\r\n");
        FWRITE("Parcel parcel = new Parcel();\r\n");
        FWRITE("CObjectHolder objectHolder = CreateOrGetObjectHolder(code, uri, bundle, parcel);\r\n");
        FWRITE("if (objectHolder == null) {\r\nif (parcel.GetPosition() < 5)\r\nreturn null;\r\n"
            "else\r\nreturn parcel;\r\n}\r\n\r\n");
        FWRITE("if (code == CommandCode.COMMAND_RELEASE) {\r\n");
        FWRITE("if (AResult.AFAILED(ReleaseObjectHolder(objectHolder, uri, false)))\r\nreturn null;\r\n\r\n");
        FWRITE("parcel = new Parcel();\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt32(IpcException.NoException)))\r\nreturn null;\r\n}\r\n");
        FWRITE("else if (code == CommandCode.COMMAND_CALLBACK) {\r\n");
        FWRITE("parcel = new Parcel();\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt32(IpcException.NoException)))\r\nreturn null;\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteInt64((long)objectHolder.hashCode())))\r\nreturn null;\r\n}\r\n");
        FWRITE("else {\r\nparcel = objectHolder.OnTransact(bundle, uri);\r\n"
            "if ((parcel == null) && (code == CommandCode.COMMAND_CREATE))\r\n"
            "ReleaseObjectHolder(objectHolder, uri, false);\r\n}\r\n\r\n");
        FWRITE("if (mServiceList.size() == 0) {\r\nobjectHolder = null;\r\nSystem.gc();\r\n}\r\n\r\n");
        FWRITE("return parcel;\r\n}\r\n\r\n");

        FWRITE("public void OnDeath(String uri)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (uri == null)\r\nreturn;\r\n\r\nsynchronized(mServiceList) {\r\n"
            "Collection<CObjectHolder> colls = mServiceList.values();\r\n"
            "if (colls == null)\r\nreturn;\r\n\r\nIterator<CObjectHolder> iterator = colls.iterator();\r\n"
            "while (iterator.hasNext()) {\r\nReleaseObjectHolder((CObjectHolder)iterator.next(), uri, true);"
            "\r\n}\r\n}\r\n}\r\n\r\n");

        FWRITE("private CObjectHolder CreateOrGetObjectHolder("
            "int code, String uri, Parcel bundle, Parcel parcel)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (bundle == null)\r\nreturn null;\r\n\r\n");
        FWRITE("synchronized (mServiceList) {\r\n");
        FWRITE("CObjectHolder obj;\r\n");
        FWRITE("switch (code) {\r\n");
        FWRITE("case CommandCode.COMMAND_CREATE:\r\n");

        if ((node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        if (node->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_Singleton) {
            FWRITE("if (!mServiceList.isEmpty()) {\r\n");
            FWRITE("Collection<CObjectHolder> valueList = mServiceList.values();\r\n");
            FWRITE("if (valueList == null)\r\nreturn null;\r\n\r\n");
            FWRITE("Iterator<CObjectHolder> iterator = valueList.iterator();\r\n"
                "obj = iterator.next();\r\n}\r\nelse {\r\n");
            WRITE_STRING_TO_FILE(className->Length() + objectHolder->Length() + 77,
                L"obj = new %ls%ls();\r\n", (PCWStr)*className, (PCWStr)*objectHolder);
            FWRITE("if (mServiceList.put((long)obj.hashCode(), obj) != null)\r\nreturn null;\r\n}\r\n\r\nbreak;\r\n");
        }
        else {
            WRITE_STRING_TO_FILE(className->Length() + objectHolder->Length() + 77,
                L"obj = new %ls%ls();\r\n", (PCWStr)*className, (PCWStr)*objectHolder);
            FWRITE("if (mServiceList.put((long)obj.hashCode(), obj) != null)\r\nreturn null;\r\n\r\nbreak;\r\n");
        }

        FWRITE("case CommandCode.COMMAND_CALLBACK:\r\ncase CommandCode.COMMAND_CALL:\r\n"
            "case CommandCode.COMMAND_RELEASE:\r\n");
        FWRITE("try {\r\nlong objRef = bundle.ReadInt64();\r\n");
        FWRITE("obj = mServiceList.get(objRef);\r\nif (obj == null) {\r\n"
            "if (AResult.AFAILED(parcel.WriteInt32(IpcException.RemoteRefException)))\r\nreturn null;\r\n\r\n"
            "return null;\r\n}\r\n}\r\ncatch (Exception e) {\r\nreturn null;\r\n}\r\n\r\n");
        FWRITE("break;\r\ndefault:\r\nreturn null;\r\n}\r\n\r\n");
        FWRITE("if ((code == CommandCode.COMMAND_CREATE) || (code == CommandCode."
            "COMMAND_CALLBACK)) {\r\nif (uri != null) {\r\nUriManager.StartThread(uri);\r\n"
            "obj.mUriList.add(uri);\r\n}\r\n\r\nobj.AddRemoteRef();\r\n}\r\n\r\n");
        FWRITE("return obj;\r\n}\r\n}\r\n\r\n");

        FWRITE("private int ReleaseObjectHolder(CObjectHolder objectHolder,"
            "String uri, boolean delAll)\r\n{\r\n");
        FWRITE("if (objectHolder == null)\r\nreturn AResult.AE_INVALIDARG;\r\n\r\n");
        FWRITE("synchronized(mServiceList) {\r\n");
        FWRITE("if (uri == null) {\r\nif (objectHolder.DecreaseRemoteRef() == 0) {\r\n");
        FWRITE("long key = (long)objectHolder.hashCode();\r\n");
        FWRITE("mServiceList.remove(key);\r\nObjectManager.UnregisterObject(key);\r\n"
            "}\r\n}\r\nelse {\r\nfor (String obj : objectHolder.mUriList) {\r\n"
            "if (uri.equals(obj)) {\r\nobjectHolder.mUriList.remove(obj);\r\n"
            "if (objectHolder.DecreaseRemoteRef() == 0) {\r\nlong key = (long)objectHolder.hashCode();\r\n"
            "mServiceList.remove(key);\r\nObjectManager.UnregisterObject(key);\r\n}\r\n\r\n"
            "if (!delAll)\r\nbreak;\r\n}\r\n}\r\n}\r\n\r\n"
            "return AResult.AS_OK;\r\n");
        FWRITE("}\r\n}\r\n\r\n");

        FWRITE("public static boolean RegisterService()\r\n");
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(szNamespace->Length() + className->Length() * 2 + stub->Length() + 93,
            L"return AResult.ASUCCEEDED(ServiceManager.RegisterService(\"%ls.%ls\", new %ls%ls()));\r\n",
            (PCWStr)*szNamespace, (PCWStr)*className, (PCWStr)*className, (PCWStr)*stub);
        FWRITE("}\r\n");
        FWRITE("}\r\n");

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVA);
    }

    STATIC Void WriteObjectHolderFile(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
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
        Sp<String> fileNameWithoutSuffix = className;
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + modifier->Length() + 15,
            L"%ls/%ls%ls.java", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix, (PCWStr)*modifier);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(szNamespace->Length() + 77, L"package IPC.gen.%ls;\r\n\r\n", (PCWStr)*szNamespace);
        WriteJvmImportFile(node, E_OBJECTHOLDER);
        FWRITE("import java.lang.reflect.Method;\r\nimport AXP.AResult;\r\nimport AXP.Parcel;\r\n"
            "import IPC.java.CObjectHolder;\r\nimport IPC.java.IpcException;\r\nimport IPC.java.ObjectManager;\r\n"
            "import IPC.java.ServiceManager;\r\n");

        Int32 objectType = GetTypeId(node);
        if (objectType == LIST)
            FWRITE("import java.util.LinkedList;\r\n");

        WRITE_STRING_TO_FILE(szNamespace->Length() + fileNameWithoutSuffix->Length() + 77,
            L"import IPC.gen.%ls.I%ls;\r\n",
            (PCWStr)*szNamespace, (PCWStr)*fileNameWithoutSuffix);
        WriteIncludeFile(sRoot);
        FWRITE("\r\n");
        WRITE_STRING_TO_FILE(className->Length() + modifier->Length() + 50,
            L"public class %ls%ls extends CObjectHolder\r\n",
            (PCWStr)*className, (PCWStr)*modifier);
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 50, L"private I%ls mService;\r\n", (PCWStr)*className);
        WriteFunCodeMembers(node);
        FWRITE("\r\n");
        WriteObjectHolderOnTransact(node);
        FWRITE("}\r\n");


        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVA);
    }

    STATIC Void WriteInterfaceFile(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fileNameWithoutSuffix = className;
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/I%ls.java", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(namespacz->Length() + 77, L"package IPC.gen.%ls;\r\n\r\n", (PCWStr)*namespacz);
        WriteJvmImportFile(node, E_INTERFACE);
        WriteIncludeFile(sRoot);
        FWRITE("\r\n");

        WRITE_STRING_TO_FILE(className->Length() + 77, L"public interface I%ls\r\n", (PCWStr)*className);
        FWRITE("{\r\n");

        Foreach(TreeNode<CSymbol>, var, node->GetChildren()) {
            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
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
                    L"%ls %ls(%ls) throws Throwable;\r\n",
                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
            }
        }

        FWRITE("}\r\n");

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVA);
    }

    STATIC Void WriteServiceFile(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fileNameWithoutSuffix = className;
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/_%ls.java", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        if (!ACCESS((PStr)fullFileName->GetBytes()->GetPayload(), 0))
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(namespacz->Length() + 77, L"package IPC.gen.%ls;\r\n\r\n", (PCWStr)*namespacz);
        WriteJvmImportFile(node, E_INTERFACE);
        WRITE_STRING_TO_FILE(namespacz->Length() + fileNameWithoutSuffix->Length() + 77,
            L"import IPC.gen.%ls.I%ls;\r\n",
            (PCWStr)*namespacz, (PCWStr)*fileNameWithoutSuffix);
        WriteIncludeFile(sRoot);
        FWRITE("\r\n");

        className = String::Create(className->Length() + sizeof("_%ls"), L"_%ls", (PCWStr)*className);
        if (className == NULL)
            return;

        WRITE_STRING_TO_FILE(className->Length() + fileNameWithoutSuffix->Length() + 77, L"public class %ls implements I%ls\r\n", (PCWStr)*className, (PCWStr)*fileNameWithoutSuffix);
        FWRITE("{\r\n");

        Int32 count = 0;
        Foreach(TreeNode<CSymbol>, var, node->GetChildren()) {
            if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                if ((var->GetChildren()->Get(0) == NULL) || (var->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> formParameterList = GetFormParameters(var->GetChildren()->Get(1));
                if (formParameterList == NULL)
                    return;

                WRITE_STRING_TO_FILE((className->Length() + formParameterList->Length()) * 2 + 97,
                    L"public static %ls Create(%ls) {\r\nreturn new %ls(%ls);\r\n}\r\n\r\n",
                    (PCWStr)*className, (PCWStr)*formParameterList, (PCWStr)*className, (PCWStr)*formParameterList);
            }
        }

        if (count == 0)
            WRITE_STRING_TO_FILE(className->Length() * 2 + 77,
            L"public static %ls Create() {\r\nreturn new %ls();\r\n}\r\n\r\n", (PCWStr)*className, (PCWStr)*className);

        Foreach(TreeNode<CSymbol>, var, node->GetChildren()) {
            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                if (var->GetChildren()->Get(0) == NULL)
                    return;

                Sp<CSymbol> returnSymbol = var->GetChildren()->Get(0)->GetValue();
                if (returnSymbol == NULL)
                    return;

                Sp<String> returnType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
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

                WRITE_STRING_TO_FILE(returnType->Length() + functionName->Length() + formParameterList->Length() + 77,
                    L"public %ls %ls(%ls) throws Throwable {\r\n",
                    (PCWStr)*returnType, (PCWStr)*functionName, (PCWStr)*formParameterList);
                if (returnSymbol->mSymbolType != SymbolType_Void) {
                    if ((returnSymbol->mSymbolType == SymbolType_Int8)
                        || (returnSymbol->mSymbolType == SymbolType_Byte)
                        || (returnSymbol->mSymbolType == SymbolType_Int16)
                        || (returnSymbol->mSymbolType == SymbolType_Int32)
                        || (returnSymbol->mSymbolType == SymbolType_Int64)
                        || (returnSymbol->mSymbolType == SymbolType_Float)
                        || (returnSymbol->mSymbolType == SymbolType_Double))
                        FWRITE("return 0;\r\n");
                    else if (returnSymbol->mSymbolType == SymbolType_Boolean)
                        FWRITE("return false;\r\n");
                    else
                        FWRITE("return null;\r\n");
                }
                FWRITE("\r\n}\r\n\r\n");
            }
        }

        FWRITE("}\r\n");

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVA);
    }

    STATIC Sp<String> GetBaseClassName(IN TreeNode<CSymbol> * node)
    {
        if ((node->GetChildren()->Get(2) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue() == NULL))
            return NULL;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        if (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue()->mContent == NULL)
            return String::Create(L"implements IParcelable");
        else {
            Sp<String> baseClassName = GetTypeReferenceList(node->GetChildren()->Get(2)->GetChildren()->Get(0), IDL_LANG_JAVA);
            if (baseClassName == NULL)
                return NULL;

            return String::Create(baseClassName->Length() + 17, L"extends %ls", (PCWStr)*baseClassName);
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

        FWRITE("public int WriteToParcel(Parcel parcel)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (parcel == null)\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        FWRITE("if (AResult.AFAILED(parcel.WriteString(this.getClass().getName())))\r\nreturn AResult.AE_FAIL;\r\n\r\n");

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
                FWRITE("if (AResult.AFAILED(super.WriteToParcel(parcel)))\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        }

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                WriteVarToParcel(obj);
                FWRITE("\r\n");
            }
        }

        FWRITE("return AResult.AS_OK;\r\n");
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

        FWRITE("public int ReadFromParcel(Parcel parcel)\r\n");
        FWRITE("{\r\n");
        FWRITE("if (parcel == null)\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        FWRITE("try {parcel.ReadString();\r\n");

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
                FWRITE("if (AResult.AFAILED(super.ReadFromParcel(parcel)))\r\nreturn AResult.AE_FAIL;\r\n\r\n");
        }

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member)
                ReadVarFromParcel(obj);
        }

        FWRITE("}\r\ncatch (Exception e) {\r\nreturn AResult.AE_FAIL;\r\n}\r\n\r\n");
        FWRITE("return AResult.AS_OK;\r\n");
        FWRITE("}\r\n");
    }

    STATIC Void WriteToString(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("public String ToString()\r\n");
        FWRITE("{\r\n");

        Int32 count = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member)
                ++count;
        }

        if (count <= 0) {
            FWRITE("return String.format(\"\");\r\n");
            FWRITE("}\r\n");
            return;
        }

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
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"for(int i = 0; i < %ls.size(); ++i) {\r\n", (PCWStr)*varId);

                        Sp<CSymbol> elementSymbol = GetVarSymbol(obj->GetChildren()->Get(0));
                        if (elementSymbol == NULL)
                            return;

                        Sp<String> elementType = GetVarType(
                            obj->GetChildren()->Get(0)->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 47, L"%ls obj = %ls.get(i);\r\n",
                            (PCWStr)*elementType, (PCWStr)*varId);
                        FWRITE("String comma = null;\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (i < %ls.size() - 1)\r\n", (PCWStr)*varId);
                        FWRITE("comma = \",\";\r\nelse\r\ncomma = \"\";\r\n\r\n");

                        if (elementSymbol->mSymbolType == SymbolType_Class) {
                            Boolean isComplex;
                            if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0)->GetChildren()->Get(0), isComplex)))
                                return;

                            FWRITE("if (obj == null)\r\nreturn null;\r\n\r\n");
                            FWRITE("String str = obj.ToString();\r\n");
                            FWRITE("if (str == null)\r\nreturn null;\r\n\r\n");

                            if (isComplex)
                                FWRITE("json = String.format(\"%s%s%s\", json, str, comma);\r\n");
                            else
                                FWRITE("json = String.format(\"%s\\\"%s\\\"%s\", json, str, comma);\r\n");

                            FWRITE("if (json == null)\r\nreturn null;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_String) {
                            FWRITE("json = String.format(\"%s\\\"%s\\\"%s\", json, obj, comma);\r\n");
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
                            FWRITE("json = String.format(\"%s\\\"%s\\\"%s\", json, obj, comma);\r\n");
                            FWRITE("if (json == null)\r\nreturn null;\r\n");
                        }
                        else {
                            DEBUG_PRINT("not support list type!");
                            return;
                        }

                        FWRITE("}\r\n\r\n");
                        FWRITE("return String.format(\"%s]\", json);\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null)\r\n", (PCWStr)*varId);
                        FWRITE("return String.format(\"\");\r\nelse\r\n");
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
                            L"return String.format(\"%%s\", %ls);\r\n", (PCWStr)*varId);
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
                        || (symbol->mSymbolType == SymbolType_Boolean))
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                        L"return String.format(\"%%s\", %ls);\r\n", (PCWStr)*varId);
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
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"\\\"%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"String str = %ls.ToString();\r\n", (PCWStr)*varId);
                        FWRITE("if (str == null)\r\nreturn null;\r\n\r\n");
                        if (isComplex)
                            WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":%%s%ls\", json, str);\r\n", (PCWStr)*varId, comma);
                        else
                            WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"%%s\\\"%ls\", json, str);\r\n\r\n", (PCWStr)*varId, comma);

                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_List) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":[]%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 87,
                            L"String jsonTmp = String.format(\"\\\"%ls\\\":[\");\r\n", (PCWStr)*varId);
                        FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 57, L"for(int i = 0; i < %ls.size(); ++i) {\r\n", (PCWStr)*varId);

                        Sp<CSymbol> elementSymbol = GetVarSymbol(obj->GetChildren()->Get(0));
                        if (elementSymbol == NULL)
                            return;

                        Sp<String> elementType = GetVarType(
                            obj->GetChildren()->Get(0)->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
                        if (elementType == NULL)
                            return;

                        WRITE_STRING_TO_FILE(elementType->Length() + varId->Length() + 47, L"%ls obj = %ls.get(i);\r\n",
                            (PCWStr)*elementType, (PCWStr)*varId);
                        FWRITE("String comma = null;\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (i < %ls.size() - 1)\r\n", (PCWStr)*varId);
                        FWRITE("comma = \",\";\r\nelse\r\ncomma = \"\";\r\n\r\n");

                        if (elementSymbol->mSymbolType == SymbolType_Class) {
                            Boolean isComplex;
                            if (AFAILED(IsComplexMember(obj->GetChildren()->Get(0)->GetChildren()->Get(0), isComplex)))
                                return;

                            FWRITE("if (obj == null)\r\nreturn null;\r\n\r\n");
                            FWRITE("String str = obj.ToString();\r\n");
                            FWRITE("if (str == null)\r\nreturn null;\r\n\r\n");
                            if (isComplex)
                                FWRITE("jsonTmp = String.format(\"%s%s%s\", jsonTmp, str, comma);\r\n");
                            else
                                FWRITE("jsonTmp = String.format(\"%s\\\"%s\\\"%s\", jsonTmp, str, comma);\r\n\r\n");

                            FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n");
                        }
                        else if (elementSymbol->mSymbolType == SymbolType_String) {
                            FWRITE("jsonTmp = String.format("
                                "\"%s\\\"%s\\\"%s\", jsonTmp, obj, comma);\r\n");
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
                            FWRITE("jsonTmp = String.format("
                                "\"%s\\\"%s\\\"%s\", jsonTmp, obj, comma);\r\n");
                            FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n");
                        }
                        else {
                            DEBUG_PRINT("not support list type!");
                            return;
                        }

                        FWRITE("}\r\n\r\n");
                        WRITE_STRING_TO_FILE(117, L"jsonTmp = String.format("
                            "\"%%s]%ls\", jsonTmp);\r\n", comma);
                        FWRITE("if (jsonTmp == null)\r\nreturn null;\r\n\r\n");
                        FWRITE("json = String.format(\"%s%s\", json, jsonTmp);\r\n");
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n\r\n");
                    }
                    else if (symbol->mSymbolType == SymbolType_String) {
                        WRITE_STRING_TO_FILE(varId->Length() + 37, L"if (%ls == null) {\r\n", (PCWStr)*varId);
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"\\\"%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 3 + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"%%s\\\"%ls\", json, %ls);\r\n",
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
                        WRITE_STRING_TO_FILE(varId->Length() + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"\\\"%ls\", json);\r\n", (PCWStr)*varId, comma);
                        FWRITE("if (json == null)\r\nreturn null;\r\n}\r\n");
                        FWRITE("else {\r\n");
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"%%s\\\"%ls\", json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
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
                        WRITE_STRING_TO_FILE(varId->Length() * 2 + 157, L"json = String.format("
                            "\"%%s\\\"%ls\\\":\\\"%%s\\\"%ls\", json, %ls);\r\n", (PCWStr)*varId, comma, (PCWStr)*varId);
                        FWRITE("if (json == null)\r\nreturn null;\r\n\r\n");
                    }
                    else {
                        DEBUG_PRINT("not support type!");
                        return;
                    }
                }
            } //Foreach

            FWRITE("return String.format(\"%s}\", json);\r\n");
        }

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
                FWRITE("super.Copy(info);\r\n\r\n");
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
                    Sp<String> varType = GetVarType(obj->GetChildren()->Get(0)->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
                    if (varType == NULL)
                        return;

                    if (count > 1)
                        FWRITE("\r\n");

                    WRITE_STRING_TO_FILE(varId->Length() + 25, L"if (info.%ls != null) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 22, L"if (%ls != null)\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + 32, L"%ls.clear();\r\nelse\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() + varType->Length() + 78,
                        L"%ls = new LinkedList<%ls>();\r\n\r\n", (PCWStr)*varId, (PCWStr)*varType);
                    WRITE_STRING_TO_FILE(varId->Length() + 46, L"for (int i = 0; i < info.%ls.size(); ++i)\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 77, L"%ls.add(info.%ls.get(i));\r\n}\r\n", (PCWStr)*varId, (PCWStr)*varId);
                }
                else if (varSymbol->mSymbolType == SymbolType_ByteArray) {
                    if (count > 1)
                        FWRITE("\r\n");

                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"if (info.%ls != null) {\r\n", (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 83,
                        L"%ls = new byte[info.%ls.length];\r\n", (PCWStr)*varId, (PCWStr)*varId);
                    WRITE_STRING_TO_FILE(varId->Length() * 2 + 77, L"%ls = info.%ls.clone();\r\n}\r\n", (PCWStr)*varId, (PCWStr)*varId);
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

        FWRITE("public void SetNull()\r\n");
        FWRITE("{\r\n");
        if (!isInheritInterface)
            FWRITE("super.SetNull();\r\n\r\n");

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
                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"%ls.clear();\r\n", (PCWStr)*varId);

                    if (count < node->GetChildren()->GetCount() - 1)
                        FWRITE("\r\n");
                }
                else if ((varSymbol->mSymbolType == SymbolType_Class)
                    || (varSymbol->mSymbolType == SymbolType_Int8_NULL)
                    || (varSymbol->mSymbolType == SymbolType_Byte_NULL)
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
                    || (varSymbol->mSymbolType == SymbolType_String))
                    WRITE_STRING_TO_FILE(varId->Length() + 18, L"%ls = null;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_UInt64)
                    WRITE_STRING_TO_FILE(varId->Length() + 37, L"%ls = new BigInteger(\"0\");\r\n", (PCWStr)*varId);
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

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_JAVA);
        if (scopeOfClass == NULL)
            return;

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

        WRITE_STRING_TO_FILE(className->Length() + baseClassName->Length() + 37, L"public class %ls %ls\r\n",
            (PCWStr)*className, (PCWStr)*baseClassName);
        FWRITE("{\r\n");

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

                Sp<String> varType = GetVarType(var->GetChildren()->Get(0), IDL_VAR_TYPE_JAVA);
                if (varType == NULL)
                    return;

                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = var->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                WRITE_STRING_TO_FILE(varType->Length() + varId->Length() + 12, L"public %ls %ls;\r\n", (PCWStr)*varType, (PCWStr)*varId);
            }
        }

        FWRITE("\r\n");
        WriteMembersToParcel(node);
        FWRITE("\r\n");
        ReadMembersFromParcel(node);
        FWRITE("\r\n");
        WriteCopyFunction(node);
        FWRITE("\r\n");
        WriteSetNullFunction(node);
        FWRITE("\r\n");
        WriteToString(node);
        FWRITE("\r\n");
        FWRITE("public String GetTypeName()\r\n");
        FWRITE("{\r\n");
        WRITE_STRING_TO_FILE(description->Length() + 87, L"return \"%ls\";\r\n", (PCWStr)*description);
        FWRITE("}\r\n");
        FWRITE("}\r\n");
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

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_JAVA);
        if (scopeOfClass == NULL)
            return;

        WRITE_STRING_TO_FILE(varName->Length() + description->Length() + scopeOfClass->Length() + 97,
            L"STATIC AXP::Boolean _%ls_ = AXP.Libc.Common.ClassLoader.RegisterClassCreator(L\"%ls\", %ls::Create);\r\n",
            (PCWStr)*varName, (PCWStr)*description, (PCWStr)*scopeOfClass);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class)
                WriteClassNameInsertMappingTable(obj);
        }
    }

    STATIC Int32 GetTypeId(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return FALSE;

        if (node->GetValue()->mSymbolType == SymbolType_Class) {
            Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
                if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                    if (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference) {
                        Sp<CSymbol> symbol = GetVarSymbol(obj);
                        if (symbol == NULL) {
                            DEBUG_PRINT("not found symbol");
                            return FALSE;
                        }

                        if (symbol->mSymbolType == SymbolType_Class)
                            return OBJECT;
                    }
                    else if (((obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                        && (obj->GetChildren()->Get(0)->GetChildren()->Get(0)
                        && obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                        && (obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                        ->mSymbolType == SymbolType_TypeReference)))) {
                        return LIST;
                    }
                }
            }
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Interface) {
            Foreach(TreeNode<CSymbol>, param, node->GetChildren()) {
                if (param->GetValue()->mSymbolType == SymbolType_Construction) {
                    if (param->GetChildren()->Get(1) == NULL)
                        return FALSE;

                    Foreach(TreeNode<CSymbol>, var, param->GetChildren()->Get(1)->GetChildren()) {
                        if (var->GetChildren()->Get(0)
                            && var->GetChildren()->Get(0)->GetValue()
                            && (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference)) {
                            Sp<CSymbol> symbol = GetVarSymbol(var);
                            if (symbol == NULL) {
                                DEBUG_PRINT("not found symbol");
                                return FALSE;
                            }

                            if (symbol->mSymbolType == SymbolType_Class)
                                return OBJECT;
                        }
                        else if (((var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                            && (var->GetChildren()->Get(0)->GetChildren()->Get(0)
                            && var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                            && (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                            ->mSymbolType == SymbolType_TypeReference)))) {
                            return LIST;
                        }
                    }
                }
                else if (param->GetValue()->mSymbolType == SymbolType_Function) {
                    if (param->GetChildren()->Get(0)
                        && param->GetChildren()->Get(0)->GetValue()
                        && (param->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference)) {
                        Sp<CSymbol> symbol = GetVarSymbol(param);
                        if (symbol == NULL) {
                            DEBUG_PRINT("not found symbol");
                            return FALSE;
                        }

                        if (symbol->mSymbolType == SymbolType_Class)
                            return OBJECT;
                    }
                    else if ((param->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                        && (param->GetChildren()->Get(0)->GetChildren()->Get(0)
                        && param->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                        && (param->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType
                        == SymbolType_TypeReference))) {
                        return LIST;
                    }

                    if (param->GetChildren()->Get(2) == NULL)
                        return FALSE;

                    Foreach(TreeNode<CSymbol>, var, param->GetChildren()->Get(2)->GetChildren()) {
                        if (var->GetChildren()->Get(0)
                            && var->GetChildren()->Get(0)->GetValue()
                            && (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference)) {
                            Sp<CSymbol> symbol = GetVarSymbol(var);
                            if (symbol == NULL) {
                                DEBUG_PRINT("not found symbol");
                                return FALSE;
                            }

                            if (symbol->mSymbolType == SymbolType_Class)
                                return OBJECT;
                        }
                        else if ((var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                            && (var->GetChildren()->Get(0)->GetChildren()->Get(0)
                            && var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                            && (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                            ->mSymbolType == SymbolType_TypeReference))) {
                            return LIST;
                        }
                    }
                }
            }
        }

        return FALSE;
    }

    STATIC Void WriteModelsFile(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Class))
            return;

        if ((node->GetChildren()->Get(0) == NULL) || (node->GetChildren()->Get(0)->GetValue() == NULL))
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + namespacz->Length() + 77,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*namespacz);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> className = node->GetChildren()->Get(0)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> fullFileName = String::Create(dirPath->Length() + className->Length() + 15,
            L"%ls/%ls.java", (PCWStr)*dirPath, (PCWStr)*className);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(namespacz->Length() + 77, L"package %ls;\r\n\r\n", (PCWStr)*namespacz);
        Int32 writen = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                if ((obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64)
                    || (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL)) {
                    if (!(writen & 0x01)) {
                        writen |= 0x01;
                        FWRITE("import java.math.BigInteger;\r\n");
                    }
                }
                else if (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List) {
                    if (!(writen & 0x02)) {
                        writen |= 0x02;
                        FWRITE("import java.util.LinkedList;\r\nimport java.util.List;\r\n");
                    }
                }

                if ((writen & 0x03) == 0x03)
                    break;
            }
        }

        TreeNode<CSymbol> * baseClassNode = node->GetChildren()->Get(2);
        if (baseClassNode
            && baseClassNode->GetChildren()->Get(0)
            && baseClassNode->GetChildren()->Get(0)->GetValue()
            && baseClassNode->GetChildren()->Get(0)->GetValue()->mContent)
            FWRITE("import AXP.Parcel;\r\nimport AXP.AResult;\r\n");
        else
            FWRITE("import AXP.IParcelable;\r\nimport AXP.Parcel;\r\nimport AXP.AResult;\r\n");

        Int32 objectType = GetTypeId(node);
        if (objectType == LIST)
            FWRITE("import java.util.LinkedList;\r\n");

        WriteIncludeFile(sRoot);
        FWRITE("\r\n");
        WriteClass(node);
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVA);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                WriteModelsFile(obj);
                FWRITE("\r\n");
            }
        }
    }

    STATIC Void InsertMappingTable(IN TreeNode<CSymbol> * node)
    {
        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_JAVA);
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

            if (obj->GetValue()->mSymbolType == SymbolType_FileBegin)
                WriteInitMappingTableFunction(obj);
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

    STATIC Void WriteJvmImportFile(IN TreeNode<CSymbol> * node, IN enum _ClassMode classType)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (!((node->GetValue()->mSymbolType == SymbolType_Class)
            || (node->GetValue()->mSymbolType == SymbolType_Interface))))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Int32 writen = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if ((obj == NULL) || (obj->GetValue() == NULL))
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Construction) {
                if (obj->GetChildren()->Get(1) == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, var, obj->GetChildren()->Get(1)->GetChildren()) {
                    if (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List) {
                        if (!(writen & 0x01)) {
                            writen |= 0x01;
                            FWRITE("import java.util.List;\r\n");
                        }

                        if ((var->GetChildren()->Get(0)->GetChildren()->Get(0) == NULL)
                            || (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue() == NULL))
                            return;

                        if (!((var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_String)
                            || (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_ByteArray)
                            || (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference))) {
                            if (classType == E_OBJECTHOLDER) {
                                if (!(writen & 0x04)) {
                                    writen |= 0x04;
                                    FWRITE("import java.util.LinkedList;\r\n");
                                }
                            }
                        }

                        if (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL) {
                            if (!(writen & 0x02)) {
                                writen |= 0x02;
                                FWRITE("import java.math.BigInteger;\r\n");
                            }
                        }
                    }
                    else if ((var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64)
                        || (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL)) {
                        if (!(writen & 0x02)) {
                            writen |= 0x02;
                            FWRITE("import java.math.BigInteger;\r\n");
                        }
                    }
                }
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_Function) {
                if ((obj->GetChildren()->Get(0) == NULL)
                    || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                if (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List) {
                    if (!(writen & 0x01)) {
                        writen |= 0x01;
                        FWRITE("import java.util.List;\r\n");
                    }

                    if ((obj->GetChildren()->Get(0)->GetChildren()->Get(0) == NULL)
                        || (obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue() == NULL))
                        return;

                    if (!((obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_String)
                        || (obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_ByteArray)
                        || (obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference))) {
                        if (classType == E_PROXY) {
                            if (!(writen & 0x04)) {
                                writen |= 0x04;
                                FWRITE("import java.util.LinkedList;\r\n");
                            }
                        }
                    }

                    if (obj->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL) {
                        if (!(writen & 0x02)) {
                            writen |= 0x02;
                            FWRITE("import java.math.BigInteger;\r\n");
                        }
                    }
                }
                else if ((obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64)
                    || (obj->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL)) {
                    if (!(writen & 0x02)) {
                        writen |= 0x02;
                        FWRITE("import java.math.BigInteger;\r\n");
                    }
                }

                if (obj->GetChildren()->Get(2) == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, var, obj->GetChildren()->Get(2)->GetChildren()) {
                    if (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List) {
                        if (!(writen & 0x01)) {
                            writen |= 0x01;
                            FWRITE("import java.util.List;\r\n");
                        }

                        if ((var->GetChildren()->Get(0)->GetChildren()->Get(0) == NULL)
                            || (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue() == NULL))
                            return;

                        if (!((var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_String)
                            || (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_ByteArray)
                            || (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_TypeReference))) {
                            if (classType == E_OBJECTHOLDER) {
                                if (!(writen & 0x04)) {
                                    writen |= 0x04;
                                    FWRITE("import java.util.LinkedList;\r\n");
                                }
                            }
                        }

                        if (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL) {
                            if (!(writen & 0x02)) {
                                writen |= 0x02;
                                FWRITE("import java.math.BigInteger;\r\n");
                            }
                        }
                    }
                    else if ((var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64)
                        || (var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_UInt64_NULL)) {
                        if (!(writen & 0x02)) {
                            writen |= 0x02;
                            FWRITE("import java.math.BigInteger;\r\n");
                        }
                    }
                }
            }

            if ((writen & 0x07) == 0x07)
                break;
        }
    }

    STATIC Void WriteProxyFile(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_Interface))
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> fileNameWithoutSuffix = className;
        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 77,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*szNamespace);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/%ls.java", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WRITE_STRING_TO_FILE(szNamespace->Length() + 77, L"package IPC.gen.%ls;\r\n\r\n", (PCWStr)*szNamespace);
        WriteJvmImportFile(node, E_PROXY);
        FWRITE("import AXP.AResult;\r\nimport AXP.Parcel;\r\n");
        FWRITE("import IPC.java.CServerConnection;\r\nimport IPC.java.CommandCode;\r\n");
        FWRITE("import IPC.java.IStub;\r\nimport IPC.java.ObjectManager;\r\n");
        FWRITE("import IPC.java.IpcException;\r\nimport IPC.java.IpcException.CException;\r\n");
        FWRITE("import IPC.java.ServiceManager;\r\n");
        Int32 objectType = GetTypeId(node);
        if (objectType == LIST)
            FWRITE("import java.util.LinkedList;\r\n");

        WRITE_STRING_TO_FILE(szNamespace->Length() + fileNameWithoutSuffix->Length() + 77,
            L"import IPC.gen.%ls.I%ls;\r\n",
            (PCWStr)*szNamespace, (PCWStr)*fileNameWithoutSuffix);
        WriteIncludeFile(sRoot);
        FWRITE("\r\n");
        WRITE_STRING_TO_FILE(className->Length() * 2 + 87,
            L"public class %ls implements I%ls\r\n{\r\n",
            (PCWStr)*className, (PCWStr)*className);

        WriteProxyMembers(node);
        FWRITE("\r\n");
        WriteProxyCreates(node);
        WriteProxyConstructions(node);
        WriteProxyDestruction(node);
        FWRITE("\r\n");
        WriteProxyFunctions(node);
        WriteProxyGetRemoteRef(node);

        FWRITE("public void AddRemoteRef(String uri)\r\n{\r\nmConn.AddRemoteRef(uri, mRef);\r\n}\r\n\r\n");
        WRITE_STRING_TO_FILE(className->Length() + 27, L"public I%ls GetInterface()\r\n", (PCWStr)*className);
        FWRITE("{\r\nreturn mInterface;\r\n}\r\n");
        FWRITE("}\r\n");
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVA);
    }

    STATIC Sp<String> GetFileName(IN TreeNode<CSymbol> * node)
    {
        Sp<String> className;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return NULL;

                        className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return NULL;

                        break;
                    }
                    else if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class)) {
                        if ((tmp->GetChildren()->Get(0) == NULL) || (tmp->GetChildren()->Get(0)->GetValue() == NULL))
                            return NULL;

                        className = tmp->GetChildren()->Get(0)->GetValue()->mContent;
                        if (className == NULL)
                            return NULL;

                        break;
                    }
                }
            }
        }

        return className;
    }

    Void WriteJavaFile(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        sRoot = node;
        Sp<String> fileNameWithoutSuffix = GetFileName(node);
        if (fileNameWithoutSuffix == NULL)
            return;

        pthread_key_create(&sKey, NULL);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        WriteProxyFile(tmp);
                        WriteObjectHolderFile(tmp);
                        WriteStubFile(tmp);
                        WriteInterfaceFile(tmp);
                        WriteServiceFile(tmp);
                    }
                    else if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class))
                        WriteModelsFile(tmp);
                    else
                        continue;
                }
            }
        }

        if (gIDLFlag & IDL_GENERATE_INCLUDE_FILE) {
            Foreach(TreeNode<CSymbol>, fileNode, node->GetChildren()) {
                if (fileNode->GetValue() && (fileNode->GetValue()->mSymbolType == SymbolType_FileBegin)) {
                    Foreach(TreeNode<CSymbol>, obj, fileNode->GetChildren()) {
                        if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                            if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                                return;

                            Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                                if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                                    WriteProxyFile(tmp);
                                    WriteObjectHolderFile(tmp);
                                    WriteStubFile(tmp);
                                    WriteInterfaceFile(tmp);
                                    WriteServiceFile(tmp);
                                }
                                else if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Class))
                                    WriteModelsFile(tmp);
                                else
                                    continue;
                            }
                        }
                    }
                }
            }
        }
    }
} // namespace IDLC