
#include <stdlib.h>
#include "Common.h"

using namespace AXP;
using namespace AXP::STL;

namespace IDLC
{
    Sp<String> GetConsOrFuncId(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL))
            return NULL;

        Int32 index;
        if (node->GetValue()->mSymbolType == SymbolType_Construction)
            index = 0;
        else if (node->GetValue()->mSymbolType == SymbolType_Function)
            index = 1;
        else
            return NULL;

        if ((node->GetParent()->GetChildren()->Get(1) == NULL)
            || (node->GetParent()->GetChildren()->Get(1)->GetValue() == NULL))
            return NULL;

        Sp<String> className = node->GetParent()->GetChildren()->Get(1)->GetValue()->mContent;
        if (className == NULL)
            return NULL;

        if ((node->GetChildren()->Get(index) == NULL)
            || (node->GetChildren()->Get(index)->GetValue() == NULL)
            || (node->GetChildren()->Get(index)->GetValue()->mContent == NULL))
            return NULL;

        Sp<String> funcName = node->GetChildren()->Get(index)->GetValue()->mContent;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return NULL;
        
        return String::Create(namespacz->Length() + className->Length() + funcName->Length() + 17,
        L"%ls_%ls_%ls", (PCWStr)*namespacz, (PCWStr)*className, (PCWStr)*funcName);
    }

    Sp<String> GetDefConsId(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL))
            return NULL;

        if (node->GetValue()->mSymbolType != SymbolType_Interface)
            return NULL;

        if ((node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL)
            || (node->GetChildren()->Get(1)->GetValue()->mContent == NULL))
            return NULL;

        Sp<String> funcName = node->GetChildren()->Get(1)->GetValue()->mContent;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return NULL;
        
        return String::Create(namespacz->Length() + funcName->Length() * 2 + 17,
        L"%ls_%ls_%ls", (PCWStr)*namespacz, (PCWStr)*funcName, (PCWStr)*funcName);
    }

    Sp<String> GetDESCRIPTORId(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL))
            return NULL;

        if (node->GetValue()->mSymbolType != SymbolType_Interface)
            return NULL;

        if ((node->GetChildren()->Get(1) == NULL)
            || (node->GetChildren()->Get(1)->GetValue() == NULL)
            || (node->GetChildren()->Get(1)->GetValue()->mContent == NULL))
            return NULL;

        Sp<String> className = node->GetChildren()->Get(1)->GetValue()->mContent;

        Sp<String> namespacz = GetNameSpace(node);
        if (namespacz == NULL)
            return NULL;
        
        return String::Create(namespacz->Length() + className->Length() + 37, L"%ls_%ls_DESCRIPTOR", (PCWStr)*namespacz, (PCWStr)*className);
    }

    FILE* OpenFile(IN CONST Sp<String> & fullFileName)
    {
        if (fullFileName == NULL)
            return NULL;

        Sp<ByteArray> byteArray = fullFileName->GetBytes();
        FILE * file = ::fopen((PCStr)byteArray->GetPayload(), "w");
        if (file == NULL)
            return NULL;

        Char head[] = { (Char)0xEF, (Char)0xBB, (Char)0xBF, (Char)0x0D, (Char)0x0A, (Char)0x00 };
        if (::fwrite(head, Strlen(head), 1, file) != 1)
            return NULL;

        return file;
    }

    Void GenerateDirPath(IN CONST Sp<String> & dirPath)
    {
        if (dirPath == NULL)
            return;

        Sp<ByteArray> dirPathBytes = dirPath->GetBytes();
        PStr dirPathStr = (PStr)dirPathBytes->GetPayload();
        for (Size i = 0; i < dirPathBytes->GetUsed(); i++) {
            if ((i == 0) && ((dirPathStr[i] == '\\') || (dirPathStr[i] == '/')))
                continue;

            if ((dirPathStr[i] == '\\') || (dirPathStr[i] == '/')) {
                dirPathStr[i] = '\0';

                // ACCESS 参数2
                // 6 检查读写权限
                // 4 检查读权限
                // 2 检查写权限
                // 1 检查执行权限
                // 0 检查文件的存在性
                if (ACCESS(dirPathStr, 0) != 0)
                if (MKDIR(dirPathStr) != 0)
                    return;

                dirPathStr[i] = '/';
            }
            else if (dirPathStr[i] == '\0') {
                // ACCESS 参数2
                // 6 检查读写权限
                // 4 检查读权限
                // 2 检查写权限
                // 1 检查执行权限
                // 0 检查文件的存在性
                if (ACCESS(dirPathStr, 0) != 0)
                if (MKDIR(dirPathStr) != 0)
                    return;
            }
        }
    }

    Void FormatFile(IN CONST Sp<String> & fileName, IN enum _IDLFlag language)
    {
        char command[1024];

        if (language == IDL_LANG_CPP)
            sprintf(command, "astyle -n --style=kr --mode=c -y -Y -N %ls", (PCWStr)*fileName);
        else if (language == IDL_LANG_CS)
            sprintf(command, "astyle -n --style=kr --mode=cs -y -Y -N %ls", (PCWStr)*fileName);
        else if (language == IDL_LANG_JAVA)
            sprintf(command, "astyle -n --style=kr --mode=java -y -Y -N %ls", (PCWStr)*fileName);
        else if (language == IDL_LANG_JAVASCRIPT)
            sprintf(command, "astyle -n --style=kr --mode=java -y -Y -N %ls", (PCWStr)*fileName);
        else if (language == IDL_LANG_OBJC)
            sprintf(command, "astyle -n --style=kr --mode=c -y -Y -N %ls", (PCWStr)*fileName);
        else
            return;

        system(command);
    }

    Int32 GetMode(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return 0;

        Int32 modeFlag = 0;
        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() == NULL)
                return 0;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() == NULL)
                        return 0;

                    if (tmp->GetValue()->mSymbolType == SymbolType_Class) {
                        modeFlag |= IDL_MODE_MODELS;
                    }
                    else if (tmp->GetValue()->mSymbolType == SymbolType_Interface) {
                        modeFlag |= IDL_MODE_IPC;
                    }
                }
            }
        }

        return modeFlag;
    }

    Sp<String> GetTypeReferenceList(IN TreeNode<CSymbol> * node, IN enum _IDLFlag language)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetValue()->mContent == NULL))
            return NULL;

        Sp<String> typeReferenceList = node->GetValue()->mContent;
        if (typeReferenceList == NULL)
            return NULL;

        if (node->GetChildren()->Get(0) == NULL) {
            if (language == IDL_LANG_JAVASCRIPT) {
                Sp<String> namespacz = GetNameSpace(node);
                if (namespacz == NULL)
                    return NULL;

                return String::Create(namespacz->Length() + typeReferenceList->Length() + 7, L"%ls.%ls", (PCWStr)*namespacz, (PCWStr)*typeReferenceList);
            }
        }

        while (node->GetChildren()->Get(0)) {
            node = node->GetChildren()->Get(0);
            if (node->GetValue() == NULL)
                return NULL;

            Sp<String> obj = node->GetValue()->mContent;
            if (obj == NULL)
                return NULL;

            if (language == IDL_LANG_CPP)
                typeReferenceList = String::Create(typeReferenceList->Length() + obj->Length() + 7, L"%ls::%ls",
                (PCWStr)*typeReferenceList, (PCWStr)*obj);
            else if ((language == IDL_LANG_CS) || (language == IDL_LANG_JAVA) || (language == IDL_LANG_JAVASCRIPT))
                typeReferenceList = String::Create(typeReferenceList->Length() + obj->Length() + 7, L"%ls.%ls", (PCWStr)*typeReferenceList, (PCWStr)*obj);
            else if (language == IDL_LANG_ES6)
                typeReferenceList = String::Create(typeReferenceList->Length() + obj->Length() + 7, L"%ls.%ls", (PCWStr)*typeReferenceList, (PCWStr)*obj);
            else if (language == IDL_LANG_OBJC)
                typeReferenceList = obj;
            else
                return NULL;

            if (typeReferenceList == NULL)
                return NULL;
        }

        return typeReferenceList;
    }

    Sp<String> GetVarType(IN TreeNode<CSymbol> * node, IN Int32 flag)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return NULL;

        enum SymbolType symbolType = node->GetValue()->mSymbolType;
        if (flag & IDL_VAR_TYPE_OBJC) {
            if (flag & IDL_VAR_TYPE_PROPERTY) {
                if (symbolType == SymbolType_TypeReference) {
                    TreeNode<CSymbol> * obj = node;
                    while (obj->GetChildren()->Get(0))
                        obj = obj->GetChildren()->Get(0);

                    if (obj->GetValue() == NULL)
                        return NULL;

                    return String::Create(obj->GetValue()->mContent->Length() + 27, L"@property (retain) %ls *",
                        (PCWStr)*(obj->GetValue()->mContent));
                }
                else if (symbolType == SymbolType_List) {
                    return String::Create(L"@property (retain) NSMutableArray *");
                }
                else if (symbolType == SymbolType_String) {
                    return String::Create(L"@property (retain) NSString *");
                }
                else if (symbolType == SymbolType_ByteArray) {
                    return String::Create(L"@property (retain) NSData *");
                }
                else {
                    Sp<String> obj = String::Create(sDataTypeMappingToObjC[symbolType - SymbolType_Int8]);
                    if (obj == NULL)
                        return NULL;

                    if ((symbolType == SymbolType_Int8_NULL)
                        || (symbolType == SymbolType_Int16_NULL)
                        || (symbolType == SymbolType_Int32_NULL)
                        || (symbolType == SymbolType_Int64_NULL)
                        || (symbolType == SymbolType_UInt8_NULL)
                        || (symbolType == SymbolType_UInt16_NULL)
                        || (symbolType == SymbolType_UInt32_NULL)
                        || (symbolType == SymbolType_UInt64_NULL)
                        || (symbolType == SymbolType_Float_NULL)
                        || (symbolType == SymbolType_Double_NULL)
                        || (symbolType == SymbolType_Boolean_NULL)) {
                        return String::Create(obj->Length() + 22, L"@property (retain) %ls", (PCWStr)*obj);
                    }

                    return String::Create(obj->Length() + 22, L"@property (assign) %ls", (PCWStr)*obj);
                }
            }
            else if (flag & IDL_VAR_TYPE_DECLARE) {
                if (symbolType == SymbolType_TypeReference) {
                    TreeNode<CSymbol> * obj = node;
                    while (obj->GetChildren()->Get(0))
                        obj = obj->GetChildren()->Get(0);

                    if (obj->GetValue() == NULL)
                        return NULL;

                    return String::Create(obj->GetValue()->mContent->Length() + 7, L"%ls *", (PCWStr)*(obj->GetValue()->mContent));
                }
                else if (symbolType == SymbolType_List) {
                    return String::Create(L"NSMutableArray *");
                }
                else if (symbolType == SymbolType_String) {
                    return String::Create(L"NSString *");
                }
                else if (symbolType == SymbolType_ByteArray) {
                    return String::Create(L"NSData *");
                }
                else {
                    return String::Create(sDataTypeMappingToObjC[symbolType - SymbolType_Int8]);
                }
            }
        }
        else if (flag & IDL_VAR_TYPE_CPP) {
            if (flag & IDL_VAR_TYPE_NOMODIFY) {
                if (symbolType == SymbolType_TypeReference) {
                    return GetTypeReferenceList(node, IDL_LANG_CPP);
                }
                else if (symbolType == SymbolType_List) {
                    Sp<String> type = GetVarType(node->GetChildren()->Get(0), flag);
                    if (type == NULL)
                        return NULL;

                    return String::Create(type->Length() + 37, L"AXP::List<%ls>", (PCWStr)*type);
                }
                else if (symbolType == SymbolType_String) {
                    return String::Create(L"AXP::String");
                }
                else if (symbolType == SymbolType_ByteArray) {
                    return String::Create(L"AXP::ByteArray");
                }
                else {
                    Sp<String> type = String::Create(sDataTypeMappingToCpp[symbolType - SymbolType_Int8]);
                    if (type == NULL)
                        return NULL;

                    return String::Create(type->Length() + 7, L"%ls", (PCWStr)*type);
                }
            }
            else if (flag & IDL_VAR_TYPE_DECLARE) {
                if (symbolType == SymbolType_TypeReference) {
                    Sp<String> obj = GetTypeReferenceList(node, IDL_LANG_CPP);
                    if (obj == NULL)
                        return NULL;

                    return String::Create(obj->Length() + 17, L"AXP::Sp<%ls>", (PCWStr)*obj);
                }
                else if (symbolType == SymbolType_List) {
                    Sp<String> type = GetVarType(node->GetChildren()->Get(0),
                        flag & ~IDL_VAR_TYPE_DECLARE & ~IDL_VAR_TYPE_FORM_PARAMETER | IDL_VAR_TYPE_NOMODIFY);
                    if (type == NULL)
                        return NULL;

                    return String::Create(type->Length() + 57, L"AXP::Sp<AXP::List<%ls > >", (PCWStr)*type);
                }
                else if (symbolType == SymbolType_String) {
                    return String::Create(L"AXP::Sp<AXP::String>");
                }
                else if (symbolType == SymbolType_ByteArray) {
                    return String::Create(L"AXP::Sp<AXP::ByteArray>");
                }
                else {
                    Sp<String> type = String::Create(sDataTypeMappingToCpp[symbolType - SymbolType_Int8]);
                    if (type == NULL)
                        return NULL;

                    return String::Create(type->Length() + 7, L"%ls", (PCWStr)*type);
                }
            }
            else if (flag & IDL_VAR_TYPE_FORM_PARAMETER) {
                if (symbolType == SymbolType_TypeReference) {
                    Sp<String> obj = GetTypeReferenceList(node, IDL_LANG_CPP);
                    if (obj == NULL)
                        return NULL;

                    return String::Create(obj->Length() + 27, L"IN CONST AXP::Sp<%ls> &", (PCWStr)*obj);
                }
                else if (symbolType == SymbolType_List) {
                    Sp<String> type = GetVarType(node->GetChildren()->Get(0),
                        flag & ~IDL_VAR_TYPE_DECLARE & ~IDL_VAR_TYPE_FORM_PARAMETER | IDL_VAR_TYPE_NOMODIFY);
                    if (type == NULL)
                        return NULL;

                    return String::Create(type->Length() + 57, L"IN CONST AXP::Sp<AXP::List<%ls > > &", (PCWStr)*type);
                }
                else if (symbolType == SymbolType_String) {
                    return String::Create(L"IN CONST AXP::Sp<AXP::String> &");
                }
                else if (symbolType == SymbolType_ByteArray) {
                    return String::Create(L"IN CONST AXP::Sp<AXP::ByteArray> &");
                }
                else {
                    Sp<String> type = String::Create(sDataTypeMappingToCpp[symbolType - SymbolType_Int8]);
                    if (type == NULL)
                        return NULL;

                    return String::Create(type->Length() + 7, L"IN %ls", (PCWStr)*type);
                }
            }
        }
        else if (flag & IDL_VAR_TYPE_CS) {
            if (symbolType == SymbolType_TypeReference) {
                return GetTypeReferenceList(node, IDL_LANG_CS);
            }
            else if (symbolType == SymbolType_List) {
                Sp<String> type = GetVarType(node->GetChildren()->Get(0), flag);
                if (type == NULL)
                    return NULL;

                return String::Create(type->Length() + 37, L"List<%ls>", (PCWStr)*type);
            }
            else {
                return String::Create(sDataTypeMappingToCs[symbolType - SymbolType_Int8]);
            }
        }
        else if (flag & IDL_VAR_TYPE_JAVA) {
            if (symbolType == SymbolType_TypeReference) {
                TreeNode<CSymbol> * obj = node;
                while (obj->GetChildren()->Get(0))
                    obj = obj->GetChildren()->Get(0);

                if (obj->GetValue() == NULL)
                    return NULL;

                return obj->GetValue()->mContent;
            }
            else if (symbolType == SymbolType_List) {
                Sp<String> type = GetVarType(node->GetChildren()->Get(0), flag);
                if (type == NULL)
                    return NULL;

                return String::Create(type->Length() + 37, L"List<%ls>", (PCWStr)*type);
            }
            else {
                return String::Create(sDataTypeMappingToJava[symbolType - SymbolType_Int8]);
            }
        }
    }

    STATIC Boolean FoundSymbolInEnum(IN TreeNode<CSymbol> * node1, IN TreeNode<CSymbol> * node2)
    {
        if ((node1 == NULL) || (node1->GetValue() == NULL) || (node2 == NULL) || (node2->GetValue() == NULL))
            return FALSE;

        if (node1->GetChildren()->GetCount() > 0)
            return FALSE;

        if (node1->GetValue()->mContent
            && node2->GetChildren()->Get(0)
            && node2->GetChildren()->Get(0)->GetValue()
            && node2->GetChildren()->Get(0)->GetValue()->mContent
            && node1->GetValue()->mContent->Equals(node2->GetChildren()->Get(0)->GetValue()->mContent)) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    STATIC Boolean FoundSymbolInClass(
        IN TreeNode<CSymbol> * node1,
        IN TreeNode<CSymbol> * node2,
        OUT Sp<CSymbol> & symbol,
        OUT TreeNode<CSymbol> ** outNode)
    {
        if ((node1 == NULL) || (node1->GetValue() == NULL) || (node2 == NULL) || (node2->GetValue() == NULL))
            return FALSE;

        if ((node1->GetValue()->mContent == NULL)
            || (node2->GetChildren()->Get(0) == NULL)
            || (node2->GetChildren()->Get(0)->GetValue() == NULL)
            || (node2->GetChildren()->Get(0)->GetValue()->mContent == NULL))
            return FALSE;

        if (node1->GetValue()->mContent->Equals(node2->GetChildren()->Get(0)->GetValue()->mContent)) {
            if (node1->GetChildren()->GetCount() > 0) {
                Foreach(TreeNode<CSymbol>, obj, node2->GetChildren()) {
                    if (obj->GetValue() == NULL)
                        return FALSE;

                    if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                        if (FoundSymbolInClass(node1->GetChildren()->Get(0), obj, symbol, outNode)) {
                            return TRUE;
                        }
                    }
                    else if (obj->GetValue()->mSymbolType == SymbolType_Enum) {
                        if (FoundSymbolInEnum(node1->GetChildren()->Get(0), obj)) {
                            symbol = new CSymbol();
                            if (symbol == NULL)
                                return FALSE;

                            symbol->mSymbolType = SymbolType_Enum;
                            return TRUE;
                        }
                    }
                }

                return FALSE;
            }
            else {
                symbol = new CSymbol();
                if (symbol == NULL)
                    return FALSE;

                symbol->mSymbolType = SymbolType_Class;
                if (outNode)
                    *outNode = node2;

                return TRUE;
            }
        }
        else {
            Foreach(TreeNode<CSymbol>, obj, node2->GetChildren()) {
                if (obj->GetValue() == NULL)
                    return FALSE;

                if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                    if (FoundSymbolInClass(node1, obj, symbol, outNode))
                        return TRUE;
                }
                else if (obj->GetValue()->mSymbolType == SymbolType_Enum) {
                    if (FoundSymbolInEnum(node1, obj)) {
                        symbol = new CSymbol();
                        if (symbol == NULL)
                            return FALSE;

                        symbol->mSymbolType = SymbolType_Enum;
                        return TRUE;
                    }
                }
            }
        }

        return FALSE;
    }

    Sp<String> GetNameSpace(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return NULL;

        if (node->GetValue()->mSymbolType == SymbolType_NameSpace) {
            if (node->GetChildren()->Get(0)
                && node->GetChildren()->Get(0)->GetValue()
                && node->GetChildren()->Get(0)->GetValue()->mContent) {
                return String::Create(node->GetChildren()->Get(0)->GetValue()->mContent);
            }
            else
                return NULL;
        }
        else if ((node->GetValue()->mSymbolType == SymbolType_Begin)
            || (node->GetValue()->mSymbolType == SymbolType_FileBegin)) {
            Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
                if (obj->GetValue() == NULL)
                    return NULL;

                if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                    if (obj->GetChildren()->Get(0)
                        && obj->GetChildren()->Get(0)->GetValue()
                        && obj->GetChildren()->Get(0)->GetValue()->mContent) {
                        return String::Create(obj->GetChildren()->Get(0)->GetValue()->mContent);
                    }
                    else
                        return NULL;
                }
            }
        }
        else {
            return GetNameSpace(node->GetParent());
        }
    }

    STATIC Boolean FoundSymbolInInterface(IN TreeNode<CSymbol> * node1, IN TreeNode<CSymbol> * node2)
    {
        if ((node1 == NULL) || (node1->GetValue() == NULL) || (node2 == NULL) || (node2->GetValue() == NULL))
            return FALSE;

        if (node1->GetChildren()->GetCount() > 0)
            return FALSE;

        if (node1->GetValue()->mContent
            && node2->GetChildren()->Get(1)
            && node2->GetChildren()->Get(1)->GetValue()
            && node2->GetChildren()->Get(1)->GetValue()->mContent
            && node1->GetValue()->mContent->Equals(node2->GetChildren()->Get(1)->GetValue()->mContent)) {
            Sp<TreeNode<CSymbol> > node = node2;
            while (true) {
                node = node->GetParent();
                if ((node == NULL) || (node->GetValue() == NULL))
                    break;

                if (node->GetValue()->mSymbolType == SymbolType_FileBegin) {
                    Sp<String> fileName = node->GetValue()->mContent;
                    if (fileName == NULL)
                        return FALSE;

                    break;
                }
            }

            return TRUE;
        }
        else
            return FALSE;
    }

    STATIC Boolean FoundSymbolInNameSpace(
        IN TreeNode<CSymbol> * node1,
        IN TreeNode<CSymbol> * node2,
        OUT Sp<CSymbol> & symbol,
        OUT TreeNode<CSymbol> ** outNode)
    {
        if ((node1 == NULL) || (node1->GetValue() == NULL) || (node2 == NULL) || (node2->GetValue() == NULL))
            return FALSE;

        Sp<String> namespacz = GetNameSpace(node1);
        if (namespacz == NULL)
            return FALSE;


        if (node1->GetValue()->mContent == NULL
            || (node2->GetChildren()->Get(0) == NULL)
            || (node2->GetChildren()->Get(0)->GetValue() == NULL)
            || (node2->GetChildren()->Get(0)->GetValue()->mContent == NULL))
            return FALSE;

        Sp<TreeNode<CSymbol> > node;
        if (namespacz->Equals(node2->GetChildren()->Get(0)->GetValue()->mContent)) {
            if (node1->GetValue()->mContent->Equals(namespacz))
                node = node1->GetChildren()->Get(0);
            else
                node = node1;
        }
        else {
            if (node1->GetValue()->mContent->Equals(node2->GetChildren()->Get(0)->GetValue()->mContent))
                node = node1->GetChildren()->Get(0);
            else
                return FALSE;
        }

        Foreach(TreeNode<CSymbol>, obj, node2->GetChildren()) {
            if (obj->GetValue() == NULL)
                return FALSE;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                if (FoundSymbolInClass(node, obj, symbol, outNode))
                    return TRUE;
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_Interface) {
                if (FoundSymbolInInterface(node, obj)) {
                    symbol = new CSymbol();
                    if (symbol == NULL)
                        return FALSE;

                    symbol->mSymbolType = SymbolType_Interface;

                    return TRUE;
                }
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_Enum) {
                if (FoundSymbolInEnum(node, obj)) {
                    symbol = new CSymbol();
                    if (symbol == NULL)
                        return FALSE;

                    symbol->mSymbolType = SymbolType_Enum;

                    return TRUE;
                }
            }
        }

        return FALSE;
    }

    STATIC Sp<CSymbol> GetRefSymbol(IN TreeNode<CSymbol> * node, IN TreeNode<CSymbol> * node2, OUT TreeNode<CSymbol> ** outNode)
    {
        if ((node == NULL) || (node->GetValue() == NULL))
            return NULL;

        if ((node2 == NULL) || (node2->GetValue() == NULL))
            return NULL;

        Foreach(TreeNode<CSymbol>, obj, node2->GetChildren()) {
            if (obj->GetValue() == NULL)
                return NULL;

            if (obj->GetValue()->mSymbolType == SymbolType_Class) {
                Sp<CSymbol> symbol;
                if (FoundSymbolInClass(node, obj, symbol, outNode))
                    return symbol;
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                Sp<CSymbol> symbol;
                if (FoundSymbolInNameSpace(node, obj, symbol, outNode))
                    return symbol;
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_Enum) {
                if (FoundSymbolInEnum(node, obj)) {
                    Sp<CSymbol> symbol = new CSymbol();
                    if (symbol == NULL)
                        return NULL;

                    symbol->mSymbolType = SymbolType_Enum;
                    return symbol;
                }
            }
            else if (obj->GetValue()->mSymbolType == SymbolType_FileBegin) {
                Sp<CSymbol> symbol = GetRefSymbol(node, obj, outNode);
                if (symbol != NULL)
                    return symbol;
            }
        }

        return NULL;
    }

    TreeNode<CSymbol> * GetTypeReferenceTreeNode(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetValue()->mSymbolType != SymbolType_TypeReference))
            return NULL;

        TreeNode<CSymbol> * tmp = node;
        while (true) {
            if ((tmp == NULL) || (tmp->GetValue() == NULL))
                return NULL;

            if (tmp->GetValue()->mSymbolType == SymbolType_Begin)
                break;
            else
                tmp = tmp->GetParent();
        }

        TreeNode<CSymbol> * ret;
        GetRefSymbol(node, tmp, &ret);

        return ret;
    }

    ARESULT IsComplexMember(IN TreeNode<CSymbol> * node, OUT Boolean & isComplex)
    {
        TreeNode<CSymbol> * tmpNode = GetTypeReferenceTreeNode(node);
        if (tmpNode == NULL)
            return AE_FAIL;

        if (tmpNode->GetValue() && (tmpNode->GetValue()->mSymbolType != SymbolType_Class))
            return AE_FAIL;

        if ((tmpNode->GetChildren()->Get(3) == NULL)
            || (tmpNode->GetChildren()->Get(3)->GetChildren()->Get(0) == NULL)
            || (tmpNode->GetChildren()->Get(3)->GetChildren()->Get(0)->GetValue() == NULL))
            return AE_FAIL;

        enum SymbolType symbolType = tmpNode->GetChildren()->Get(3)->GetChildren()->Get(0)->GetValue()->mSymbolType;

        if ((tmpNode->GetChildren()->GetCount() > 4) || (symbolType == SymbolType_List)) {
            isComplex = TRUE;
            return AS_OK;
        }
        else if (symbolType == SymbolType_TypeReference)
            return IsComplexMember(tmpNode->GetChildren()->Get(3)->GetChildren()->Get(0), isComplex);
        else {
            isComplex = FALSE;
            return AS_OK;
        }
    }

    Sp<CSymbol> GetVarSymbol(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetChildren()->Get(0) == NULL))
            return NULL;

        Sp<CSymbol> symbol = node->GetChildren()->Get(0)->GetValue();
        if (symbol->mSymbolType == SymbolType_TypeReference) {
            TreeNode<CSymbol> * tmp = node;
            while (true) {
                if ((tmp == NULL) || (tmp->GetValue() == NULL))
                    return NULL;

                if (tmp->GetValue()->mSymbolType == SymbolType_Begin)
                    break;
                else
                    tmp = tmp->GetParent();
            }

            return GetRefSymbol(node->GetChildren()->Get(0), tmp, NULL);
        }
        else {
            return symbol;
        }
    }

    Sp<String> GetVarId(IN TreeNode<CSymbol> * node)
    {
        if ((node == NULL) || (node->GetChildren()->GetCount() < 1))
            return NULL;

        Sp<TreeNode<CSymbol> > tmpNode = node->GetChildren()->Get(node->GetChildren()->GetCount() - 1);
        if ((tmpNode == NULL) || (tmpNode->GetValue() == NULL))
            return NULL;

        return String::Create(tmpNode->GetValue()->mContent);
    }

    Sp<String> GetScopeChainOfClassName(IN TreeNode<CSymbol> * node, IN enum _IDLFlag language)
    {
        if ((node == NULL)
            || (node->GetValue() == NULL)
            || (node->GetValue()->mSymbolType != SymbolType_Class)
            || (node->GetChildren()->Get(0) == NULL)
            || (node->GetChildren()->Get(0)->GetValue() == NULL)
            || (node->GetChildren()->Get(0)->GetValue()->mContent == NULL))
            return NULL;

        Sp<String> scopeChainName = node->GetChildren()->Get(0)->GetValue()->mContent;

        TreeNode<CSymbol> * obj = node;
        while (true) {
            obj = obj->GetParent();
            if ((obj == NULL)
                || (obj->GetChildren()->Get(0) == NULL)
                || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                return NULL;

            Sp<String> tmp = obj->GetChildren()->Get(0)->GetValue()->mContent;
            if (tmp == NULL)
                return NULL;

            if ((language == IDL_LANG_CPP) || (language == IDL_LANG_OBJC))
                scopeChainName = String::Create(tmp->Length() + scopeChainName->Length() + 17, L"%ls::%ls", (PCWStr)*tmp, (PCWStr)*scopeChainName);
            else if ((language == IDL_LANG_CS) || (language == IDL_LANG_JAVA))
                scopeChainName = String::Create(tmp->Length() + scopeChainName->Length() + 17, L"%ls.%ls", (PCWStr)*tmp, (PCWStr)*scopeChainName);
            else if ((language == IDL_LANG_ES6))
                scopeChainName = String::Create(scopeChainName->Length() + 17, L"%ls", (PCWStr)*scopeChainName);
            else if (language == IDL_NOP)
                scopeChainName = String::Create(tmp->Length() + scopeChainName->Length() + 17, L"%ls_%ls", (PCWStr)*tmp, (PCWStr)*scopeChainName);
            else
                return NULL;

            if (scopeChainName == NULL)
                return NULL;

            if (obj->GetValue()->mSymbolType == SymbolType_NameSpace)
                break;
            else if (obj->GetValue()->mSymbolType == SymbolType_Class)
                continue;
            else
                return NULL;
        }

        return scopeChainName;
    }

    Boolean HasObjectTypeInInterface(IN TreeNode<CSymbol> * node)
    {
        if (node == NULL)
            return FALSE;

        Foreach(TreeNode<CSymbol>, obj, node->GetChildren()) {
            if (obj->GetValue() && obj->GetValue()->mSymbolType == SymbolType_NameSpace) {
                if ((obj->GetChildren()->Get(0) == NULL) || (obj->GetChildren()->Get(0)->GetValue() == NULL))
                    return FALSE;

                Foreach(TreeNode<CSymbol>, tmp, obj->GetChildren()) {
                    if (tmp->GetValue() && (tmp->GetValue()->mSymbolType == SymbolType_Interface)) {
                        Foreach(TreeNode<CSymbol>, param, tmp->GetChildren()) {
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
                                            return TRUE;
                                    }
                                    else if (((var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                                        && (var->GetChildren()->Get(0)->GetChildren()->Get(0)
                                        && var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                                        && (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                                        ->mSymbolType == SymbolType_TypeReference)))) {
                                        return TRUE;
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
                                        return TRUE;
                                }
                                else if ((param->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                                    && (param->GetChildren()->Get(0)->GetChildren()->Get(0)
                                    && param->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                                    && (param->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()->mSymbolType
                                    == SymbolType_TypeReference))) {
                                    return TRUE;
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
                                            return TRUE;
                                    }
                                    else if ((var->GetChildren()->Get(0)->GetValue()->mSymbolType == SymbolType_List)
                                        && (var->GetChildren()->Get(0)->GetChildren()->Get(0)
                                        && var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                                        && (var->GetChildren()->Get(0)->GetChildren()->Get(0)->GetValue()
                                        ->mSymbolType == SymbolType_TypeReference))) {
                                        return TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return FALSE;
    }
}