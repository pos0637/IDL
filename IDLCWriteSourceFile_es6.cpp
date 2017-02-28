
#include <stdlib.h>
#include "AXP/cplusplus/xplatform/include/stl/tree.h"
#include "AXP/cplusplus/xplatform/include/stl/hashtable.h"
#include "IDLCParser.h"
#include "Common.h"

using namespace AXP;
using namespace AXP::STL;

namespace IDLC
{
    EXTERN Int32 gIDLFlag;
    EXTERN Sp<List<CMappingInfo> > gImportFileEs6;
    EXTERN CMappingArea * gShareAreaEs6;
    EXTERN Sp<String> gRootOutputDir;
    EXTERN Sp<String> gNamespacz;
    STATIC pthread_key_t sKey;
    STATIC TreeNode<CSymbol> * sRoot;

    EXTERN Sp<String> GetOrignalFileName();
    enum _ClassMode { E_PROXY, E_STUB, E_OBJECTHOLDER, E_INTERFACE, E_MODAL };

    STATIC Void WriteStaticMembers(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);
        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;
        
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                       Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        Sp<String> descptorName = GetDESCRIPTORId(tmp);
                        if (descptorName == NULL)
                            return;

                        WRITE_STRING_TO_FILE(descptorName->Length() + szNamespace->Length() + className->Length() + 87,
                        L"const %ls = \"%ls.%ls\";\r\n",
                        (PCWStr)*descptorName, (PCWStr)*szNamespace, (PCWStr)*className);

                        Boolean hasConstruction = FALSE;
                        Int32 count = 0;
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if ((var == NULL) || (var->GetValue() == NULL))
                                return;
                            
                            if ((var->GetValue()->mSymbolType == SymbolType_Construction)
                                || (var->GetValue()->mSymbolType == SymbolType_Function)) {
                                Sp<String> codeId = GetConsOrFuncId(var);
                                 if (codeId == NULL)
                                    return;
                                    
                                WRITE_STRING_TO_FILE(codeId->Length() + 77,
                                L"const %ls = %d;\r\n", (PCWStr)*codeId, count++);
                                if (var->GetValue()->mSymbolType == SymbolType_Construction)
                                    hasConstruction = TRUE;
                            }
                        }
                        
                        if (!hasConstruction) {
                            Sp<String> defConsId = GetDefConsId(tmp);
                            if (defConsId == NULL)
                                return;

                            WRITE_STRING_TO_FILE(defConsId->Length() + 77, L"const %ls = %d;\r\n", (PCWStr)*defConsId, count);
                        }
                    }
                }
            }
        }
    }
    
    STATIC Void WriteCtor(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("constructor() {\r\n");
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
                FWRITE("super();\r\n");
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

                if (var->GetChildren()->Get(0) == NULL)
                    return;

                Sp<CSymbol> varSymbol = var->GetChildren()->Get(0)->GetValue();
                if (varSymbol == NULL)
                    return;

                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                    return;

                Sp<String> varId = var->GetChildren()->Get(1)->GetValue()->mContent;
                if (varId == NULL)
                    return;

                if ((varSymbol->mSymbolType == SymbolType_Int8)
                    || (varSymbol->mSymbolType == SymbolType_Byte)
                    || (varSymbol->mSymbolType == SymbolType_UInt8)
                    || (varSymbol->mSymbolType == SymbolType_Int16)
                    || (varSymbol->mSymbolType == SymbolType_UInt16)
                    || (varSymbol->mSymbolType == SymbolType_Int32)
                    || (varSymbol->mSymbolType == SymbolType_UInt32)
                    || (varSymbol->mSymbolType == SymbolType_Int64)
                    || (varSymbol->mSymbolType == SymbolType_UInt64)
                    || (varSymbol->mSymbolType == SymbolType_Float)
                    || (varSymbol->mSymbolType == SymbolType_Double)
                    || (varSymbol->mSymbolType == SymbolType_Interface))
                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"this.%ls = 0;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_Boolean)
                    WRITE_STRING_TO_FILE(varId->Length() + 37, L"this.%ls = true;\r\n", (PCWStr)*varId);
                else if ((varSymbol->mSymbolType == SymbolType_String)
                    || (varSymbol->mSymbolType == SymbolType_ByteArray)
                    || (varSymbol->mSymbolType == SymbolType_Class)
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
                    || (varSymbol->mSymbolType == SymbolType_Double_NULL))
                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"this.%ls = null;\r\n", (PCWStr)*varId);
                else if (varSymbol->mSymbolType == SymbolType_List)
                    WRITE_STRING_TO_FILE(varId->Length() + 27, L"this.%ls = [];\r\n", (PCWStr)*varId);
            }
        }

        FWRITE("}\r\n\r\n");
    }

    STATIC Void WriteVarToParcel(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return;

        Sp<String> varId;
        Sp<String> failedMsg;
        if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varId = node->GetChildren()->Get(1)->GetValue()->mContent;
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Member) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            Sp<String> content = node->GetChildren()->Get(1)->GetValue()->mContent;
            varId = String::Create(content->Length() + 17, L"this.%ls", (PCWStr)*content);
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {
            varId = String::Create(L"ret");
        }
        else
            varId = NULL;

        if (varId == NULL)
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
            WRITE_STRING_TO_FILE(varId->Length() * 2 + 97,
                L"if (%ls) {\r\nparcel.writeBoolean(true);\r\n%ls.writeToParcel(parcel);\r\n}\r\n", (PCWStr)*varId, (PCWStr)*varId);
            FWRITE("else {\r\nparcel.writeBoolean(false);\r\n}\r\n");
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface)
            WRITE_STRING_TO_FILE(varId->Length() + 88, L"parcel.writeInt64(ClassLoader.registerService(%ls));\r\n", (PCWStr)*varId);
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
            Sp<String> tmp = table->GetValue(varSymbol->mSymbolType);
            if (tmp == NULL)
                return;

            Sp<String> name = String::Create(tmp);
            if (name == NULL)
                return;

            PWStr pname = (PWStr)name->GetPayload();
            *pname += 0x20;
            WRITE_STRING_TO_FILE(name->Length() + varId->Length() + 97,
                L"parcel.%ls(%ls);\r\n", (PCWStr)*name, (PCWStr)*varId);
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }

            WRITE_STRING_TO_FILE(varId->Length() + 77, L"if (%ls) {\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(varId->Length() + 77, L"parcel.writeInt32(%ls.length);\r\n", (PCWStr)*varId);
            WRITE_STRING_TO_FILE(varId->Length() + 77, L"for (var i = 0; i < %ls.length; i++) {\r\n", (PCWStr)*varId);
            if (elementSymbol->mSymbolType == SymbolType_Class)
                WRITE_STRING_TO_FILE(varId->Length() * 2 + 256,
                L"if (%ls[i]) {\r\nparcel.writeBoolean(true);\r\n%ls[i].writeToParcel(parcel);\r\n}\r\nelse {\r\nparcel.writeBoolean(false);\r\n}\r\n", (PCWStr)*varId, (PCWStr)*varId);
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
                Sp<String> tmp = table->GetValue(elementSymbol->mSymbolType);
                if (tmp == NULL)
                    return;

                Sp<String> name = String::Create(tmp);
                if (name == NULL)
                    return;

                PWStr pname = (PWStr)name->GetPayload();
                *pname += 0x20;
                WRITE_STRING_TO_FILE(name->Length() + varId->Length() * 2 + 77,
                    L"parcel.%ls(%ls[i]);\r\n", (PCWStr)*name, (PCWStr)*varId);
            }
            else {
                DEBUG_PRINT(L"not support list type!");
                return;
            }

            FWRITE("}\r\n}\r\nelse {\r\nparcel.writeInt32(0);\r\n}\r\n");
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

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> varName;
        Sp<String> varId;
        Sp<String> failedMsg;
        if (node->GetValue()->mSymbolType == SymbolType_Parameter) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varName = node->GetChildren()->Get(1)->GetValue()->mContent;
            if (varName == NULL)
                return;

            varId = String::Create(varName->Length() + 17, L"%ls", (PCWStr)*varName);
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Member) {
            if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
                return;

            varName = node->GetChildren()->Get(1)->GetValue()->mContent;
            if (varName == NULL)
                return;

            varId = String::Create(varName->Length() + 17, L"this.%ls", (PCWStr)*varName);
        }
        else if (node->GetValue()->mSymbolType == SymbolType_Function) {            
            varId = String::Create(L"ret");
            varName = varId;
        }
        else
            varId = NULL;

        if (varId == NULL)
            return;

        Sp<CSymbol> varSymbol = GetVarSymbol(node);
        if (varSymbol == NULL) {
            DEBUG_PRINT("symbol not found!\n");
            return;
        }

        Sp<HashTable<Int32, String> > table = MapTable::GetReadFromParcelNameOfBasicType();
        if (table == NULL)
            return;
        
        if (varSymbol->mSymbolType == SymbolType_Class) {
            if (node->GetValue()->mSymbolType == SymbolType_Member)
                WRITE_STRING_TO_FILE(varId->Length() + sizeof("%ls == null;\r\n"), L"%ls == null;\r\n", (PCWStr)*varId);
            else
                WRITE_STRING_TO_FILE(varId->Length() + sizeof("let %ls = null;\r\n"), L"let %ls = null;\r\n", (PCWStr)*varId);

            WRITE_STRING_TO_FILE((varId->Length() * 2) + 256, L"let hasValue_%ls = parcel.readBoolean();\r\nif (hasValue_%ls) {\r\n"
                "let obj = ClassLoader.createObject(parcel.get(parcel.getPosition()));\r\nif (!obj)\r\nreturn;\r\n\r\n", (PCWStr)*varName, (PCWStr)*varName);
            FWRITE("obj.readFromParcel(parcel);\r\n");
            WRITE_STRING_TO_FILE(varId->Length() + 27, L"%ls = obj;\r\n}\r\n", (PCWStr)*varId);
        }
        else if (varSymbol->mSymbolType == SymbolType_Interface) {
            if (node->GetValue()->mSymbolType != SymbolType_Member)
                FWRITE("let ");

            WRITE_STRING_TO_FILE(varId->Length() + 97, L"%ls = parcel.readInt64();\r\n", (PCWStr)*varId);
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
            Sp<String> tmp = table->GetValue(varSymbol->mSymbolType);
            if (tmp == NULL)
                return;

            Sp<String> name = String::Create(tmp);
            if (name == NULL)
                return;

            PWStr pname = (PWStr)name->GetPayload();
            *pname += 0x20;
            if (node->GetValue()->mSymbolType != SymbolType_Member)
                FWRITE("let ");

            WRITE_STRING_TO_FILE(name->Length() + varId->Length() + 97,
                L"%ls = parcel.%ls();\r\n", (PCWStr)*varId, (PCWStr)*name);
        }
        else if (varSymbol->mSymbolType == SymbolType_List) {
            Sp<CSymbol> elementSymbol = GetVarSymbol(node->GetChildren()->Get(0));
            if (elementSymbol == NULL) {
                DEBUG_PRINT("symbol not found!\n");
                return;
            }
            
            WRITE_STRING_TO_FILE((varId->Length() * 2) + 97, L"let list_%ls = [];\r\nlet length_%ls = parcel.readInt32();\r\n", (PCWStr)*varName, (PCWStr)*varName);
            WRITE_STRING_TO_FILE(varId->Length() + 97, L"for (let i = 0; i < length_%ls; i++) {\r\n", (PCWStr)*varName);
            if (elementSymbol->mSymbolType == SymbolType_Class) {
                FWRITE("let hasValue = parcel.readBoolean();\r\nif (hasValue) {\r\n"
                    "let obj = ClassLoader.createObject(parcel.get(parcel.getPosition()));\r\nif (!obj)\r\nreturn;\r\n\r\n"
                   "obj.readFromParcel(parcel);\r\n");
                WRITE_STRING_TO_FILE(varName->Length() + 97, L"list_%ls.push(obj);\r\n}\r\n}\r\n", (PCWStr)*varName);
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
                Sp<String> tmp = table->GetValue(elementSymbol->mSymbolType);
                if (tmp == NULL)
                    return;

                Sp<String> name = String::Create(tmp);
                if (name == NULL)
                    return;

                PWStr pname = (PWStr)name->GetPayload();
                *pname += 0x20;
                WRITE_STRING_TO_FILE(name->Length() + 77, L"let obj = parcel.%ls();\r\n", (PCWStr)*name);
                WRITE_STRING_TO_FILE(varId->Length() + 97, L"list_%ls.push(obj);\r\n}\r\n", (PCWStr)*varName);
            }
            else {
                DEBUG_PRINT(L"not support list type!");
                return;
            }

            if (node->GetValue()->mSymbolType == SymbolType_Member)
                WRITE_STRING_TO_FILE((varId->Length() * 2) + 27, L"%ls = list_%ls;\r\n", (PCWStr)*varId, (PCWStr)*varName);
            else
                WRITE_STRING_TO_FILE((varId->Length() * 2) + 27, L"let %ls = list_%ls;\r\n", (PCWStr)*varId, (PCWStr)*varName);
        }
        else {
            DEBUG_PRINT(L"not support type!");
            return;
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

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        FWRITE("writeToParcel(parcel) {\r\n");
        FWRITE("if (parcel == null)\r\nreturn;\r\n\r\n");
        WRITE_STRING_TO_FILE(scopeOfClass->Length() + 37, L"parcel.writeString(\"%ls\");\r\n\r\n", (PCWStr)*scopeOfClass);

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
                FWRITE("super.writeToParcel(parcel);\r\n\r\n");
        }

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                WriteVarToParcel(obj);
                FWRITE("\r\n");
            }
        }

        FWRITE("}\r\n\r\n");
    }

    STATIC Void ReadMembersFromParcel(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> scopeOfClass = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (scopeOfClass == NULL)
            return;

        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_CS);
        if (description == NULL)
            return;

        FWRITE("readFromParcel(parcel) {\r\n");
        FWRITE("if (parcel == null)\r\nreturn;\r\n\r\n");
        FWRITE("parcel.readString();\r\n\r\n");

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
                FWRITE("super.readFromParcel(parcel);\r\n\r\n");
        }

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Member) {
                ReadVarFromParcel(obj);
                FWRITE("\r\n");
            }
        }

        FWRITE("}\r\n");
    }

    STATIC Sp<String> GetBaseClassName(IN TreeNode<CSymbol> * node)
    {
        if ((node->GetChildren()->Get(2) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue() == NULL))
            return NULL;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        if (node->GetChildren()->Get(2)->GetChildren()->Get(0)->GetValue()->mContent == NULL)
            return String::Create(L"Parcelable");
        else {
            Sp<String> baseClassName = GetTypeReferenceList(node->GetChildren()->Get(2)->GetChildren()->Get(0), IDL_LANG_ES6);
            if (baseClassName == NULL)
                return NULL;

            return String::Create(baseClassName->Length() + 17, L"%ls", (PCWStr)*baseClassName);
        }
    }

    STATIC Void WriteClass(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);
  
        Sp<String> description = GetScopeChainOfClassName(node, IDL_LANG_ES6);
        if (description == NULL)
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

        WRITE_STRING_TO_FILE(description->Length() + baseClassName->Length() + 37, L"export class %ls extends %ls {\r\n",
            (PCWStr)*description, (PCWStr)*baseClassName);

        WriteCtor(node);
        WriteMembersToParcel(node);
        ReadMembersFromParcel(node);
        // WriteCopyFunction(node);
        // WriteSetNullFunction(node);
        // WriteToString(node);
        FWRITE("}\r\n");
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

    STATIC Void WriteIncludeFile(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        // 写入默认头部
        FWRITE("\"use strict\";\r\n\r\n");
        FWRITE("import Parcelabel from \"axp/parcelable\";\r\n");
        FWRITE("import Parcel from \"axp/parcel\";\r\n");
        FWRITE("import ServiceConnection from \"axp/serviceConnection\";\r\n");
        FWRITE("import * as IPC from \"axp/ipc\";\r\n");
        FWRITE("import ClassLoader from \"../classloader\";\r\n");
        
        Int32 mode = GetMode(node);
        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Int32 count = node->GetChildren()->GetCount();
        Int32 i = 0;
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

                    if (++i > 1)
                        FWRITE(", ");

                    WRITE_STRING_TO_FILE(s->Length() + 22, L"\"%ls\"", (PCWStr)*s);
                    continue;
                }

                Sp<String> fileNameWithoutSuffix = fileName->SubString(index1 + 1, index2 - index1 - 1);
                if (fileNameWithoutSuffix == NULL)
                    return;

                Int32 modeFlag = GetMode(obj);
                Sp<String> tmp = GetNameSpace(obj);
                if (tmp == NULL)
                    return;

                WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() * 2 + 77,
                    L"import * as %ls from \"../%ls/%ls\";\r\n", (PCWStr)*tmp, (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // if (mode & IDL_MODE_IPC) {
                // if (modeFlag & IDL_MODE_IPC)
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 77,
                // L"\"IPC/gen/%ls/%ls\"", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // else
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 77,
                // L"\"IPC/gen/%ls/%ls\"", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // }
                // else if (mode & IDL_MODE_MODELS) {
                // if (modeFlag & IDL_MODE_IPC)
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 77,
                // L"\"IPC/gen/%ls/%ls\"", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // else
                // WRITE_STRING_TO_FILE(fileNameWithoutSuffix->Length() + tmp->Length() + 77,
                // L"\"IPC/gen/%ls/%ls\"", (PCWStr)*tmp, (PCWStr)*fileNameWithoutSuffix);
                // }
                // else {
                // DEBUG_PRINT("no support mode");
                // return;
                // }
            }
        }

        FWRITE("\r\n");
    }

    STATIC Void WriteModelsFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
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

        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 15,
            L"%ls/%ls.js", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        if (gShareAreaEs6) {
            Int32 length = 0;
            Sp<String> filePath = String::Create(namespacz->Length() + fileNameWithoutSuffix->Length()
            + 37, L"\"./%ls/%ls\"", (PCWStr)*namespacz, (PCWStr)*fileNameWithoutSuffix);
            if (filePath == NULL)
                return;
            
            Wcscpy_s(gShareAreaEs6->mDescription, 256, (PCWStr)*namespacz, &length);
            Wcscpy_s(gShareAreaEs6->mScope, 256, (PCWStr)*filePath, &length);
            gShareAreaEs6++;
        }
        else {
            Sp<CMappingInfo> mappingInfo = new CMappingInfo();
            if (!mappingInfo)
                return;

            Sp<String> filePath = String::Create(namespacz->Length() + fileNameWithoutSuffix->Length()
            + 37, L"\"./%ls/%ls\"", (PCWStr)*namespacz, (PCWStr)*fileNameWithoutSuffix);
            if (filePath == NULL)
                return;
            
            mappingInfo->mDescription = namespacz;
            mappingInfo->mScope = filePath;

            gNamespacz = namespacz;
            if (!gImportFileEs6->PushBack(mappingInfo))
                return;
        }
                        
        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WriteIncludeFile(node);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
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
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
    }

    STATIC Void WriteObjectHolderMembers(IN TreeNode<CSymbol> * node)
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
        FWRITE("static: {\r\n");
        WRITE_STRING_TO_FILE(descptorName->Length() + namespacz->Length() + className->Length() + 87,
            L"%ls: \"%ls.%ls\",\r\n",
            (PCWStr)*descptorName, (PCWStr)*namespacz, (PCWStr)*className);

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

                WRITE_STRING_TO_FILE(funCode->Length() + 77, L"%ls: %d,\r\n", (PCWStr)*funCode, count++);
            }
        }

        if (!hasConstruction) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 77, L"%ls: %d,\r\n", (PCWStr)*defConsId, count);
        }

        FWRITE("},\r\n\r\n");
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

    STATIC Void WriteParametersToParcel(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL) || (node->GetValue()->mSymbolType != SymbolType_ParameterList))
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return;

            if (obj->GetValue()->mSymbolType == SymbolType_Parameter)
                WriteVarToParcel(obj);
        }
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
                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                Sp<String> consCodeId = GetConsOrFuncId(obj);
                if (consCodeId == NULL)
                    return;

                consCodeId = String::Create(namespacz->Length() + className->Length() + consCodeId->Length() + 7,
                    L"%ls.%ls.%ls", (PCWStr)*namespacz, (PCWStr)*className, (PCWStr)*consCodeId);
                if (consCodeId == NULL)
                    return;

                WRITE_STRING_TO_FILE(realParameterList->Length() + 77, L"constructor(%ls) {\r\n", (PCWStr)*realParameterList);
                FWRITE("this.mInterface = null;\r\n");
                FWRITE("var parcel = new Parcel();\r\n");
                FWRITE("if (parcel == null)\r\nreturn;\r\n\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 88,
                    L"var stub = IPC.ServiceManager.getService(%ls);\r\n", (PCWStr)*descptorName);
                FWRITE("if (stub == null) {\r\n");
                FWRITE("this.mIsRemote = true;\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + 88,
                    L"this.mConn = IPC.CServerConnection.create(%ls);\r\n", (PCWStr)*descptorName);
                FWRITE("if (this.mConn == null)\r\nreturn null;\r\n\r\n");
                FWRITE("parcel.writeInt8(0xB3);\r\nif (typeof IPC.token == \"undefined\")\r\nreturn null;\r\n\r\n");
                FWRITE("parcel.writeString(IPC.token);\r\nparcel.writeString(%ls);\r\n");
                FWRITE("}\r\nelse {\r\n");
                FWRITE("this.mIsRemote = false;\r\nthis.mConn = stub;\r\n");
                FWRITE("}\r\n\r\n");
                FWRITE("parcel.writeInt32(IPC.CommandCode.commandCreate);\r\n");
                FWRITE("parcel.writeBoolean(this.mIsRemote);\r\n");
                FWRITE("if (this.mIsRemote)\r\n");
                FWRITE("parcel.writeString(IPC.ServiceManager.sServiceManagerAddr);\r\n\r\n");
                WRITE_STRING_TO_FILE(consCodeId->Length() + 88,
                    L"parcel.writeInt32(%ls);\r\n", (PCWStr)*consCodeId);
                WriteParametersToParcel(obj->GetChildren()->Get(1));
                FWRITE("parcel.reset();\r\n");
                FWRITE("parcel = this.mConn.transact(parcel);\r\n");
                FWRITE("parcel.reset();\r\n");
                FWRITE("var code = parcel.readInt32();\r\n");
                FWRITE("IPC.IpcException.readException(code);\r\nthis.mRef = parcel.readInt64();\r\nif (!this.mIsRemote) {\r\n"
                    "this.mInterface = window.IPC.ObjectManager.getValue(this.mRef);\r\n}\r\n},\r\n\r\n");
            }
        }

        if (!hasConstructions) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            FWRITE("constructor()\r\n{\r\n");
            FWRITE("this.mConn = new ServiceConnection();\r\n");
            WRITE_STRING_TO_FILE(descptorName->Length() + defConsId->Length() + 88,
            L"this.mRef = IPC.createObject(this.mConn, %ls, %ls);\r\n",
            (PCWStr)*descptorName, (PCWStr)*defConsId);
            FWRITE("}\r\n\r\n");
        }
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

                Sp<String> realParameters = GetRealParameters(obj->GetChildren()->Get(2));
                if (realParameters == NULL)
                    return;

                WRITE_STRING_TO_FILE(functionName->Length() + realParameters->Length() + 25,
                    L"%ls(%ls)\r\n", (PCWStr)*functionName, (PCWStr)*realParameters);
                FWRITE("{\r\n");
                WRITE_STRING_TO_FILE(descptorName->Length() + funcId->Length() + 88,
                L"var parcel = IPC.makeInvokeMethodParcel(this.mRef, %ls, %ls);\r\n",
                (PCWStr)*descptorName, (PCWStr)*funcId);
                WriteParametersToParcel(obj->GetChildren()->Get(2));
                
                FWRITE("parcel = IPC.invokeMethod(this.mConn, parcel);\r\n");
                if (obj->GetChildren()->Get(0)
                    && obj->GetChildren()->Get(0)->GetValue()
                    && (obj->GetChildren()->Get(0)->GetValue()->mSymbolType != SymbolType_Void)) {
                    ReadVarFromParcel(obj);
                    FWRITE("return ret;\r\n");
                }
                else
                    FWRITE("return;\r\n");

                FWRITE("}\r\n\r\n");
            }
        }
    }

    STATIC Void WriteInterfaceFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
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
            L"%ls/I%ls.js", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;
                        
        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);
        WriteIncludeFile(node);

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if ((tmp == NULL) || (tmp->GetValue() == NULL))
                        return;
                    
                    if (tmp->GetValue()->mSymbolType == SymbolType_Interface) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + sizeof("export class %ls\r\n{\r\n"),
                        L"export class %ls\r\n{\r\n", (PCWStr)*className);
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> realParameters = GetRealParameters(var->GetChildren()->Get(2));
                                if (realParameters == NULL)
                                    return;

                                if (var->IsLastChild())
                                    WRITE_STRING_TO_FILE(functionName->Length() + realParameters->Length() + 67,
                                    L"%ls(%ls) {}\r\n",
                                    (PCWStr)*functionName, (PCWStr)*realParameters);
                                else
                                    WRITE_STRING_TO_FILE(functionName->Length() + realParameters->Length() + 67,
                                    L"%ls(%ls) {}\r\n\r\n",
                                    (PCWStr)*functionName, (PCWStr)*realParameters);
                            }
                        }

                        FWRITE("}\r\n\r\n");
                    }
                    else if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        WriteClass(tmp);
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
    }

    STATIC Void WriteServiceFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
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
        Sp<String> fullFileName = String::Create(dirPath->Length() + szNamespace->Length() + fileNameWithoutSuffix->Length() + 77,
            L"%ls/_%ls.js", (PCWStr)*dirPath, (PCWStr)*szNamespace, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        if (!ACCESS((PStr)fullFileName->GetBytes()->GetPayload(), 0))
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);
        WriteIncludeFile(node);
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 87,
                            L"export default class %ls\r\n{\r\n",
                            (PCWStr)*className);

                        Int32 numberOfCtor = 0;
                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                if ((var->GetChildren()->Get(0) == NULL) || (var->GetChildren()->Get(0)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(0)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> realParameters = GetRealParameters(var->GetChildren()->Get(1));
                                if (realParameters == NULL)
                                    return;

                                WRITE_STRING_TO_FILE(szNamespace->Length() + className->Length() + realParameters->Length() * 2 + 88,
                                    L"static create(%ls) {\r\nreturn new %ls._%ls(%ls);\r\n}\r\n\r\n",
                                    (PCWStr)*realParameters, (PCWStr)*szNamespace, (PCWStr)*className, (PCWStr)*realParameters);

                                numberOfCtor++;
                            }
                        }

                        if (numberOfCtor == 0)
                            WRITE_STRING_TO_FILE(szNamespace->Length() + className->Length() + 88,
                            L"static create() {\r\nreturn new %ls._%ls();\r\n}\r\n\r\n",
                            (PCWStr)*szNamespace, (PCWStr)*className);

                        if (numberOfCtor == 0)
                            FWRITE("constructor() {\r\n\r\n}\r\n\r\n");
                        else {
                            Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                                if (var->GetValue()->mSymbolType == SymbolType_Construction) {
                                    if ((var->GetChildren()->Get(0) == NULL) || (var->GetChildren()->Get(0)->GetValue() == NULL))
                                        return;

                                    Sp<String> functionName = var->GetChildren()->Get(0)->GetValue()->mContent;
                                    if (functionName == NULL)
                                        return;

                                    Sp<String> realParameters = GetRealParameters(var->GetChildren()->Get(1));
                                    if (realParameters == NULL)
                                        return;

                                    WRITE_STRING_TO_FILE(realParameters->Length() + 88,
                                        L"constructor(%ls) {\r\n\r\n}\r\n\r\n", (PCWStr)*realParameters);
                                }
                            }
                        }

                        Foreach(TreeNode<CSymbol>, var, tmp->GetChildren()) {
                            if (var->GetValue()->mSymbolType == SymbolType_Function) {
                                if ((var->GetChildren()->Get(1) == NULL) || (var->GetChildren()->Get(1)->GetValue() == NULL))
                                    return;

                                Sp<String> functionName = var->GetChildren()->Get(1)->GetValue()->mContent;
                                if (functionName == NULL)
                                    return;

                                Sp<String> realParameters = GetRealParameters(var->GetChildren()->Get(2));
                                if (realParameters == NULL)
                                    return;

                                if (var->IsLastChild())
                                    WRITE_STRING_TO_FILE(functionName->Length() + realParameters->Length() + 67,
                                    L"%ls(%ls) {\r\n\r\n}\r\n",
                                    (PCWStr)*functionName, (PCWStr)*realParameters);
                                else
                                    WRITE_STRING_TO_FILE(functionName->Length() + realParameters->Length() + 67,
                                    L"%ls(%ls) {\r\n\r\n}\r\n\r\n",
                                    (PCWStr)*functionName, (PCWStr)*realParameters);
                            }
                        }
                        
                        FWRITE("}\r\n\r\n");
                    }
                }
            }
        }
        
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
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

    STATIC Void WriteObjectHolderStubFunction(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        Sp<String> objectHolder = String::Create(L"ObjectHolder");
        if (objectHolder == NULL)
            return;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        if ((node->GetChildren()->Get(1) == NULL) || (node->GetChildren()->Get(1)->GetValue() == NULL))
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> className2 = String::Create(className->Length() + objectHolder->Length() + 17,
            L"%ls%ls", (PCWStr)*className, (PCWStr)*objectHolder);
        if (className2 == NULL)
            return;

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

                funCodeName = String::Create(namespacz->Length() + className2->Length() + funCodeName->Length() + 88,
                    L"%ls.%ls.%ls", (PCWStr)*namespacz, (PCWStr)*className2, (PCWStr)*funCodeName);
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
                        Sp<String> className = GetTypeReferenceList(tmp->GetChildren()->Get(0), IDL_LANG_ES6);
                        if (className == NULL)
                            return;

                        flag = TRUE;
                        ReadVarFromParcel(tmp);

                        WRITE_STRING_TO_FILE(className->Length() + varId->Length() * 2 + 51,
                            L"var %ls = %ls.Create(%ls);\r\n",
                            (PCWStr)*varId, (PCWStr)*className, (PCWStr)*varId);
                    }
                    else
                        ReadVarFromParcel(tmp);
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
                    WRITE_STRING_TO_FILE(82 + functionName->Length() + realParameterList->Length(),
                        L"this.mService.%ls(%ls);\r\n", (PCWStr)*functionName, (PCWStr)*realParameterList);
                    FWRITE("parcel = new Parcel();\r\nif (parcel == null)\r\nreturn null;\r\n\r\n");
                    FWRITE("parcel.writeInt32(IPC.IpcException.NoException);\r\n");
                }
                else {
                    WRITE_STRING_TO_FILE(77 + functionName->Length() + realParameterList->Length(),
                        L"var ret = this.mService.%ls(%ls);\r\n",
                        (PCWStr)*functionName, (PCWStr)*realParameterList);
                    FWRITE("parcel = new Parcel();\r\nif (parcel == null)\r\nreturn null;\r\n\r\n");
                    FWRITE("parcel.writeInt32(IPC.IpcException.NoException);\r\n");

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

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return;

        Sp<String> modifier = String::Create(L"ObjectHolder");
        if (modifier == NULL)
            return;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return;

        Sp<String> className2 = String::Create(className->Length() + modifier->Length() + 7, L"%ls%ls", (PCWStr)*className, (PCWStr)*modifier);
        if (className2 == NULL)
            return;

        FILE * file = (FILE*)pthread_getspecific(sKey);

        FWRITE("onTransact(parcel, isRemote) {\r\n");
        FWRITE("if (parcel == null)\r\nreturn null;\r\n\r\n");
        FWRITE("var funCode = parcel.readInt32();\r\n");

        Int32 countOfConstruction = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && (obj->GetValue()->mSymbolType == SymbolType_Construction)) {
                Sp<String> consCodeId = GetConsOrFuncId(obj);
                if (consCodeId == NULL)
                    return;

                consCodeId = String::Create(namespacz->Length() + className2->Length() + consCodeId->Length() + 88,
                    L"%ls.%ls.%ls", (PCWStr)*namespacz, (PCWStr)*className2, (PCWStr)*consCodeId);
                if (consCodeId == NULL)
                    return;

                WRITE_STRING_TO_FILE(consCodeId->Length() + 40, L"%lsif (funCode == %ls) {\r\n",
                    countOfConstruction++ == 0 ? L"" : L"else ", (PCWStr)*consCodeId);
                FWRITE("if (this.mService == null) {\r\n");

                ReadParametersFromParcel(obj->GetChildren()->Get(1));

                Sp<String> realParameterList = GetRealParameters(obj->GetChildren()->Get(1));
                if (realParameterList == NULL)
                    return;

                FWRITE("if (mService == null) {\r\n");
                WRITE_STRING_TO_FILE(namespacz->Length() + className->Length() + realParameterList->Length() + 33,
                    L"this.mService = %ls._%ls.Create(%ls);\r\n", (PCWStr)*namespacz, (PCWStr)*className, (PCWStr)*realParameterList);
                FWRITE("if (mService == null)\r\nreturn null;\r\n}\r\n\r\n");
                FWRITE("if (!isRemote) {\r\nif (!IPC.ObjectManager.registerObject(this.getHashCode(), this.mService))\r\nreturn null;\r\n}\r\n\r\n");
                FWRITE("parcel = new Parcel();\r\nif (parcel == null)\r\nreturn null;\r\n\r\n");
                FWRITE("parcel.writeInt32(IPC.IpcException.NoException);\r\nparcel.writeInt64(this.getHashCode());\r\n}\r\n");
            }
        }

        if (countOfConstruction == 0) {
            Sp<String> defConsId = GetDefConsId(node);
            if (defConsId == NULL)
                return;

            defConsId = String::Create(namespacz->Length() + className2->Length() + defConsId->Length() + 88,
                L"%ls.%ls.%ls", (PCWStr)*namespacz, (PCWStr)*className2, (PCWStr)*defConsId);
            if (defConsId == NULL)
                return;

            WRITE_STRING_TO_FILE(defConsId->Length() + 40, L"if (funCode == %ls) {\r\n", (PCWStr)*defConsId);
            FWRITE("if (this.mService == null) {\r\n");
            WRITE_STRING_TO_FILE(namespacz->Length() + className->Length() + 88,
                L"this.mService = %ls._%ls.Create();\r\n", (PCWStr)*namespacz, (PCWStr)*className);
            FWRITE("if (this.mService == null)\r\nreturn null;\r\n}\r\n\r\n");
            FWRITE("if (!isRemote) {\r\nif (!IPC.ObjectManager.registerObject(this.getHashCode(), this.mService))\r\nreturn null;\r\n}\r\n\r\n");
            FWRITE("parcel = new Parcel();\r\nif (parcel == null)\r\nreturn null;\r\n\r\n");
            FWRITE("parcel.writeInt32(IPC.IpcException.NoException);\r\nparcel.writeInt64(this.getHashCode());\r\n}\r\n");
        }

        FWRITE("else {\r\nif (this.mService == null) {\r\n"
            "parcel = new Parcel();\r\nif (parcel == null)\r\nreturn null;\r\n\r\n"
            "parcel.writeInt32(IPC.IpcException.RemoteRefException);\r\nreturn parcel;\r\n}\r\n\r\n");

        WriteObjectHolderStubFunction(node);

        FWRITE("}\r\n\r\nreturn parcel;\r\n}\r\n");
    }

    STATIC Void WriteObjectHolderFile(IN TreeNode<CSymbol> * node, IN CONST Sp<String> & fileNameWithoutSuffix)
    {
        if (node == NULL)
            return;

        Sp<String> szNamespace = GetNameSpace(node);
        if (szNamespace == NULL)
            return;

        Sp<String> modifier = String::Create(L"ObjectHolder");
        if (modifier == NULL)
            return;

        Sp<String> dirPath = String::Create(gRootOutputDir->Length() + szNamespace->Length() + 37,
            L"%ls/%ls/", (PCWStr)*gRootOutputDir, (PCWStr)*szNamespace);
        if (dirPath == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameWithoutSuffix->Length() + 37,
            L"%ls/%ls%ls.js", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix, (PCWStr)*modifier);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);
        WriteIncludeFile(node);
        WriteStaticMembers(node);
        FWRITE("\r\n");
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        className = String::Create(className->Length() + modifier->Length() + 7, L"%ls%ls", (PCWStr)*className, (PCWStr)*modifier);
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 87,
                            L"export default class %ls extends IPC.CObjectHolder {\r\n",
                            (PCWStr)*className);

                        //WriteObjectHolderMembers(tmp);
                        WriteObjectHolderOnTransact(tmp);
                        FWRITE("}\r\n\r\n");
                    }
                }
            }
        }

        FWRITE("});\r\n");
        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
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
            L"%ls/%ls.js", (PCWStr)*dirPath, (PCWStr)*fileNameWithoutSuffix);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);
        WriteIncludeFile(node);
        WriteStaticMembers(node);
        FWRITE("\r\n");
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        WRITE_STRING_TO_FILE(className->Length() + 87,
                            L"export class %ls {\r\n",
                            (PCWStr)*className);

                        WriteProxyConstructions(tmp);
                                    
                        WriteProxyFunctions(tmp);

                        FWRITE("}\r\n\r\n");
                    }
                    else if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        WriteClass(tmp);
                        FWRITE("\r\n");
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
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

        Sp<String> fileNameStub = String::Create(fileNameWithoutSuffix->Length() + stub->Length() + 7,
            L"%ls%ls", (PCWStr)*fileNameWithoutSuffix, (PCWStr)*stub);
        if (fileNameStub == NULL)
            return;

        Sp<String> fileNameObjectHolder = String::Create(fileNameWithoutSuffix->Length() + objectHolder->Length() + 7,
            L"%ls%ls", (PCWStr)*fileNameWithoutSuffix, (PCWStr)*objectHolder);
        if (fileNameObjectHolder == NULL)
            return;

        GenerateDirPath(dirPath);
        Sp<String> fullFileName = String::Create(dirPath->Length() + fileNameStub->Length() + 15,
            L"%ls/%ls.js", (PCWStr)*dirPath, (PCWStr)*fileNameStub);
        if (fullFileName == NULL)
            return;

        FILE * file = OpenFile(fullFileName);
        if (file == NULL)
            return;

        pthread_setspecific(sKey, (void *)file);

        WriteIncludeFile(node);
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if (((obj->GetChildren()->Get(0) == NULL)) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return;

                Sp<String> namespacz = obj->GetChildren()->Get(0)->GetValue()->mContent;
                if (namespacz == NULL)
                    return;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && tmp->GetValue()->mSymbolType == SymbolType_Interface) {
                        if ((tmp->GetChildren()->Get(1) == NULL) || (tmp->GetChildren()->Get(1)->GetValue() == NULL))
                            return;

                        Sp<String> className = tmp->GetChildren()->Get(1)->GetValue()->mContent;
                        if (className == NULL)
                            return;

                        Sp<String> classNameStub = String::Create(namespacz->Length() + className->Length() + stub->Length() + 7,
                            L"%ls.%ls%ls", (PCWStr)*namespacz, (PCWStr)*className, (PCWStr)*stub);
                        if (classNameStub == NULL)
                            return;

                        WRITE_STRING_TO_FILE(classNameStub->Length() + 88,
                            L"export default class %ls extends IPC.IStub, {\r\n", (PCWStr)*classNameStub);
                        FWRITE("constructor() {\r\n");
                        FWRITE("this.mServiceList = new XspWeb.Core.HashTable();\r\nif (this.mServiceList == null)\r\nreturn null;\r\n}\r\n\r\n");
                        FWRITE("transact(bundle) {\r\n");
                        FWRITE("if (bundle == null)\r\nreturn null;\r\n\r\n");
                        FWRITE("var code = bundle.readInt32();\r\n");
                        FWRITE("if (code == IPC.CommandCode.commandCreate) {\r\nvar isRemote = bundle.readBoolean();\r\n"
                            "if (isRemote)\r\nvar uri = bundle.readString();\r\n\r\n");
                        WRITE_STRING_TO_FILE(szNamespace->Length() + className->Length() + objectHolder->Length() + 77,
                            L"var obj = new %ls.%ls%ls();\r\n", (PCWStr)*szNamespace, (PCWStr)*className, (PCWStr)*objectHolder);
                        FWRITE(" if (obj == null)\r\nreturn null;\r\n\r\n");
                        FWRITE("this.mServiceList.add(obj.getHashCode(), obj);\r\nreturn obj.onTransact(bundle, isRemote);\r\n}\r\n"
                            "else if (IPC.CommandCode.commandCall) {\r\nvar objRef = bundle.readInt64();\r\n"
                            "var obj = this.mServiceList.get(objRef);\r\nif (obj == null) {\r\nparcel.writeInt32(IPC.IpcException.RemoteRefException)\r\n"
                            "return parcel;\r\n}\r\n\r\nreturn obj.onTransact(bundle, isRemote);\r\n}\r\nelse\r\nreturn null;\r\n}\r\n}\r\n\r\n");
                    }
                }
            }
        }

        ::fclose(file);
        FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
    }

    Void WriteEs6File(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return;

        sRoot = node;
        Sp<String> fileNameWithoutSuffix = GetOrignalFileName();
        if (fileNameWithoutSuffix == NULL)
            return;

        pthread_key_create(&sKey, NULL);

        Int32 modeFlag = GetMode(node);
        if (modeFlag & IDL_MODE_IPC) {
            WriteProxyFile(node, fileNameWithoutSuffix);
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

                    if (modeFlag & IDL_MODE_IPC) {
                        WriteInterfaceFile(obj, fileNameWithoutSuffix);
                        WriteProxyFile(obj, fileNameWithoutSuffix);
                        WriteObjectHolderFile(obj, fileNameWithoutSuffix);
                        WriteStubFile(obj, fileNameWithoutSuffix);
                        WriteServiceFile(node, fileNameWithoutSuffix);
                    }
                    else if (modeFlag & IDL_MODE_MODELS)
                        WriteModelsFile(obj, fileNameWithoutSuffix);
                }
            }
        }
    }
}