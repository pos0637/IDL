
#include <stdlib.h>
#include "AXP/cplusplus/xplatform/include/type.h"
#include "AXP/cplusplus/xplatform/include/aresult.h"
#include "AXP/cplusplus/xplatform/include/object.h"
#include "AXP/cplusplus/xplatform/include/list.h"
#include "AXP/cplusplus/xplatform/include/stl/tree.h"
#include "AXP/cplusplus/xplatform/include/astring.h"
#include "AXP/cplusplus/libc/include/Common/Tracker.h"
#include "AXP/cplusplus/xplatform/include/stl/hashTable.h"
#include "IDLCSymbol.h"
#include "IDLCParser.h"

using namespace AXP;
using namespace AXP::STL;
using namespace AXP::Libc::Common;
using namespace IDLC;

#define TOP_LEVEL (0)

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#define DEBUG_PRINT(X) printf("==== error: %s, %d, %S \n", __FUNCTION__, __LINE__, X)
#endif

#define GET_LAST_SYMBOL(SYMBOL) \
do { \
    (SYMBOL) = GetLastSymbol(); \
if (!(SYMBOL)) { \
    DEBUG_PRINT(L"Invalid symbolList"); \
    return AE_INVALIDARG; \
} \
} while (0)

#define POP_LAST_SYMBOL(SYMBOL) \
do { \
    (SYMBOL) = PopLastSymbol(); \
if (!(SYMBOL)) { \
    DEBUG_PRINT(L"Invalid symbolList"); \
    return AE_INVALIDARG; \
} \
} while (0)

#define POP_AND_CHECK_LAST_SYMBOL(SYMBOL, SYMBOL_TYPE) \
do { \
    (SYMBOL) = PopLastSymbol(); \
if ((!(SYMBOL)) || (sSymbolList->IsEmpty())) { \
    DEBUG_PRINT(L"Invalid symbolList"); \
    return AE_INVALIDARG; \
} \
if ((SYMBOL)->mSymbolType != (SYMBOL_TYPE)) { \
    DEBUG_PRINT(L"Invalid symbolList"); \
    return AE_INVALIDARG; \
} \
} while (0)

#define ADD_SYMBOL(SYMBOL) \
    do { \
        (SYMBOL)->mLevel = parent->GetValue()->mLevel + 1; \
    if (parent->AddChildFront((SYMBOL)) == NULL) \
    return AE_OUTOFMEMORY; \
    } while (0)

#define ADD_SYMBOL_AND_GET_NODE(SYMBOL, NODE) \
    do { \
    (SYMBOL)->mLevel = parent->GetValue()->mLevel + 1; \
    (NODE) = parent->AddChildFront((SYMBOL)); \
    if ((NODE) == NULL) \
    return AE_OUTOFMEMORY; \
    } while (0)

namespace IDLC
{
    EXTERN Int32 gIDLFlag;
    EXTERN Void WriteCppFile(IN TreeNode<CSymbol> * node);
    EXTERN Void WriteCsFile(IN TreeNode<CSymbol> * node);
    EXTERN Void WriteObjcFile(IN TreeNode<CSymbol> * node);
    EXTERN Void WriteJavaFile(IN TreeNode<CSymbol> * node);
    EXTERN Void WriteJavascriptFile(IN TreeNode<CSymbol> * node);
    EXTERN Void WriteEs6File(IN TreeNode<CSymbol> * node);

    Sp<HashTable<Int32, String> > MapTable::sWriteToParcelOfBasicType = new HashTable<Int32, String>(50);
    Sp<HashTable<Int32, String> > MapTable::sReadFromParcelOfBasicType = new HashTable<Int32, String>(50);

    Sp<List<String> > gHeadListCpp = new List<String>();
    Sp<List<String> > gHeadListCplusplus = new List<String>();
    Sp<List<String> > gHeadListCs = new List<String>();
    Sp<List<String> > gHeadListJava = new List<String>();

    STATIC Sp<HashTable<PWStr, String> > sHeadHashTable = new HashTable<PWStr, String>(50);
    STATIC Sp<List<CSymbol> > sSymbolList = new List<CSymbol>();
    STATIC Sp<TreeNode<CSymbol> > sRoot;

    Boolean MapTable::SetParcelFunctionName()
    {
        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Boolean, String::Create(L"WriteBoolean")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int8, String::Create(L"WriteInt8")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Byte, String::Create(L"WriteByte")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int16, String::Create(L"WriteInt16")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int32, String::Create(L"WriteInt32")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int64, String::Create(L"WriteInt64")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt8, String::Create(L"WriteUInt8")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt16, String::Create(L"WriteUInt16")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt32, String::Create(L"WriteUInt32")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt64, String::Create(L"WriteUInt64")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Float, String::Create(L"WriteFloat")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Double, String::Create(L"WriteDouble")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Boolean_NULL, String::Create(L"WriteNullableBoolean")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int8_NULL, String::Create(L"WriteNullableInt8")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Byte_NULL, String::Create(L"WriteNullableByte")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int16_NULL, String::Create(L"WriteNullableInt16")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int32_NULL, String::Create(L"WriteNullableInt32")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Int64_NULL, String::Create(L"WriteNullableInt64")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt8_NULL, String::Create(L"WriteNullableUInt8")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt16_NULL, String::Create(L"WriteNullableUInt16")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt32_NULL, String::Create(L"WriteNullableUInt32")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_UInt64_NULL, String::Create(L"WriteNullableUInt64")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Float_NULL, String::Create(L"WriteNullableFloat")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_Double_NULL, String::Create(L"WriteNullableDouble")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_String, String::Create(L"WriteString")))
            return FALSE;

        if (!sWriteToParcelOfBasicType->InsertUnique(SymbolType_ByteArray, String::Create(L"WriteByteArray")))
            return FALSE;

        //--------------------------------------------------

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Boolean, String::Create(L"ReadBoolean")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int8, String::Create(L"ReadInt8")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Byte, String::Create(L"ReadByte")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int16, String::Create(L"ReadInt16")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int32, String::Create(L"ReadInt32")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int64, String::Create(L"ReadInt64")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt8, String::Create(L"ReadUInt8")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt16, String::Create(L"ReadUInt16")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt32, String::Create(L"ReadUInt32")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt64, String::Create(L"ReadUInt64")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Float, String::Create(L"ReadFloat")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Double, String::Create(L"ReadDouble")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Boolean_NULL, String::Create(L"ReadNullableBoolean")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int8_NULL, String::Create(L"ReadNullableInt8")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Byte_NULL, String::Create(L"ReadNullableByte")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int16_NULL, String::Create(L"ReadNullableInt16")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int32_NULL, String::Create(L"ReadNullableInt32")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Int64_NULL, String::Create(L"ReadNullableInt64")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt8_NULL, String::Create(L"ReadNullableUInt8")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt16_NULL, String::Create(L"ReadNullableUInt16")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt32_NULL, String::Create(L"ReadNullableUInt32")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_UInt64_NULL, String::Create(L"ReadNullableUInt64")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Float_NULL, String::Create(L"ReadNullableFloat")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_Double_NULL, String::Create(L"ReadNullableDouble")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_String, String::Create(L"ReadString")))
            return FALSE;

        if (!sReadFromParcelOfBasicType->InsertUnique(SymbolType_ByteArray, String::Create(L"ReadByteArray")))
            return FALSE;

        // //-----------------------------

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Boolean, String::Create(L"WriteListOfBoolean")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Int8, String::Create(L"WriteListOfInt8")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Int16, String::Create(L"WriteListOfInt16")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Int32, String::Create(L"WriteListOfInt32")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Int64, String::Create(L"WriteListOfInt64")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_UInt8, String::Create(L"WriteListOfUInt8")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_UInt16, String::Create(L"WriteListOfUInt16")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_UInt32, String::Create(L"WriteListOfUInt32")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_UInt64, String::Create(L"WriteListOfUInt64")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Float, String::Create(L"WriteListOfFloat")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_Double, String::Create(L"WriteListOfDouble")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_String, String::Create(L"WriteListOfString")))
            // return FALSE;

        // if (!sWriteToParcelOfListType->InsertUnique(SymbolType_ByteArray, String::Create(L"WriteListOfByteArray")))
            // return FALSE;

        // //-----------------------------

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Boolean, String::Create(L"ReadListOfBoolean")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Int8, String::Create(L"ReadListOfInt8")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Int16, String::Create(L"ReadListOfInt16")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Int32, String::Create(L"ReadListOfInt32")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Int64, String::Create(L"ReadListOfInt64")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_UInt8, String::Create(L"ReadListOfUInt8")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_UInt16, String::Create(L"ReadListOfUInt16")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_UInt32, String::Create(L"ReadListOfUInt32")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_UInt64, String::Create(L"ReadListOfUInt64")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Float, String::Create(L"ReadListOfFloat")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_Double, String::Create(L"ReadListOfDouble")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_String, String::Create(L"ReadListOfString")))
            // return FALSE;

        // if (!sReadFromParcelOfListType->InsertUnique(SymbolType_ByteArray, String::Create(L"ReadListOfByteArray")))
            // return FALSE;

        return TRUE;
    }

    Sp<HashTable<Int32, String> > MapTable::GetWriteToParcelNameOfBasicType()
    {
        return sWriteToParcelOfBasicType;
    }

    Sp<HashTable<Int32, String> > MapTable::GetReadFromParcelNameOfBasicType()
    {
        return sReadFromParcelOfBasicType;
    }

    /**
     * ���շ����б�
     */
    STATIC Void ResetSymbolList();

    /**
     * ���ɳ����﷨��
     *
     * @return �Ƿ��ɹ�,�ɹ�����AS_OK
     */
    STATIC ARESULT GenerateSymbolTree();

    /**
     * ���������﷨��
     */
    STATIC Void TraverseSymbolTree();

    /**
     * ���ҵ��������Ͷ�������
     *
     * @param parent ������
     *
     * @return �Ƿ��ɹ�,�ɹ�����AS_OK
     */
    STATIC ARESULT FoundNameSpace(
        IN TreeNode<CSymbol> * parent);

    /**
     * �ӷ����б��л�ȡ����һ������
     *
     * @return ����,ʧ�ܷ��ؿ�
     */
    STATIC Sp<CSymbol> GetLastSymbol();

    /**
     * �ӷ����б��е�������һ������
     *
     * @return ����,ʧ�ܷ��ؿ�
     */
    STATIC Sp<CSymbol> PopLastSymbol();

    ARESULT FoundModule(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundFile(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundNameSpace(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundClass(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundBaseClass(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundEnum(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundList(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundMember(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundVarType(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundTypeReferenceList(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundUsingList(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundParameterList(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundParameter(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundFunction(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundConstruction(
        IN TreeNode<CSymbol> * parent);

    ARESULT FoundInterface(
        IN TreeNode<CSymbol> * parent);

    STATIC ARESULT GenerateHeadList(CSymbol* symbol);
} // namespace IDLC

int IDLC::Initialize()
{
    CTracker::Initialize(NULL);

    return 0;
}

STATIC ARESULT IDLC::GenerateHeadList(CSymbol* symbol)
{
    if ((symbol == NULL)
        || (gHeadListCpp == NULL)
        || (gHeadListCplusplus == NULL)
        || (gHeadListCs == NULL)
        || (gHeadListJava == NULL)
        || (sHeadHashTable == NULL))
        return AE_INVALIDARG;

    SymbolType symbolType = symbol->mSymbolType;
    if ((symbolType < SymbolType_Int8) || (symbolType > SymbolType_List))
        return AS_OK;

    Sp<String> strCpp, strCplusplus, strCs, strJava;

    if ((symbolType >= SymbolType_Int8) && (symbolType <= SymbolType_Boolean)) {
        strCpp = String::Create(L"#include \"AXP/xplatform/include/type.h\"\r\n");
        strCplusplus = String::Create(L"#include \"AXP/cplusplus/xplatform/include/type.h\"\r\n");
    }
    else if ((symbolType >= SymbolType_Int8_NULL) && (symbolType <= SymbolType_Boolean_NULL)) {
        strCpp = String::Create(L"#include \"AXP/xplatform/include/nullable.h\"\r\n");
        strCplusplus = String::Create(L"#include \"AXP/cplusplus/xplatform/include/nullable.h\"\r\n");
    }
    else if (symbolType == SymbolType_ByteArray) {
        strCpp = String::Create(L"#include \"AXP/xplatform/include/coretype.h\"\r\n");
        strCplusplus = String::Create(L"#include \"AXP/cplusplus/xplatform/include/coretype.h\"\r\n");
    }
    else if (symbolType == SymbolType_String) {
        strCpp = String::Create(L"#include \"AXP/xplatform/include/astring.h\"\r\n");
        strCplusplus = String::Create(L"#include \"AXP/cplusplus/xplatform/include/astring.h\"\r\n");
    }
    else if (symbolType == SymbolType_List) {
        strCpp = String::Create(L"#include \"AXP/xplatform/include/list.h\"\r\n");
        strCplusplus = String::Create(L"#include \"AXP/cplusplus/xplatform/include/list.h\"\r\n");
    }
    else
        return AS_OK;

    if (strCpp == NULL)
        return AE_INVALIDARG;

    if (sHeadHashTable->InsertUnique((PWStr)strCpp->GetPayload(), strCpp))
        gHeadListCpp->PushBack(strCpp);

    if (sHeadHashTable->InsertUnique((PWStr)strCplusplus->GetPayload(), strCplusplus))
        gHeadListCplusplus->PushBack(strCplusplus);

    if (symbolType == SymbolType_List) {
        strCs = String::Create(L"using System;\r\n");
        if (strCs == NULL)
            return AE_INVALIDARG;

        if (sHeadHashTable->InsertUnique((PWStr)strCs->GetPayload(), strCs))
            gHeadListCs->PushBack(strCs);

        strCs = String::Create(L"using System.Collections.Generic;\r\n");
        if (strCs == NULL)
            return AE_INVALIDARG;

        if (sHeadHashTable->InsertUnique((PWStr)strCs->GetPayload(), strCs))
            gHeadListCs->PushBack(strCs);
    }
    else {
        strCs = String::Create(L"using System;\r\n");
        if (strCs == NULL)
            return AE_INVALIDARG;

        if (sHeadHashTable->InsertUnique((PWStr)strCs->GetPayload(), strCs))
            gHeadListCs->PushBack(strCs);
    }

    if ((symbolType == SymbolType_UInt64)
        || (symbolType == SymbolType_UInt64_NULL)
        || (symbolType == SymbolType_List)) {
        if (symbolType == SymbolType_List)
            strJava = String::Create(L"import java.util.LinkedList;\r\nimport java.util.List;\r\n");
        else
            strJava = String::Create(L"import java.math.BigInteger;\r\n");

        if (strJava == NULL)
            return AE_INVALIDARG;

        if (sHeadHashTable->InsertUnique((PWStr)strJava->GetPayload(), strJava))
            gHeadListJava->PushBack(strJava);
    }

    return AS_OK;
}

int IDLC::AddItem(
    SymbolType symbolType,
    const char * content)
{
    Sp<CSymbol> symbol = new CSymbol();
    if (symbol == NULL)
        return AE_OUTOFMEMORY;

    symbol->mLevel = TOP_LEVEL;
    symbol->mSymbolType = symbolType;
    if (content) {
        symbol->mContent = String::Create((PCByte)content, 1, strlen(content), EncodingType_UTF8);
        if (symbol->mContent == NULL)
            return AE_OUTOFMEMORY;
    }

    if (AFAILED(GenerateHeadList(symbol)))
        return AE_INVALIDARG;

    if (!sSymbolList->PushBack(symbol))
        return AE_OUTOFMEMORY;

    if (symbolType == SymbolType_Begin)
        return GenerateSymbolTree();

    return AS_OK;
}

ARESULT IDLC::RemovePrevItem()
{
    return (sSymbolList->PopBack() ? AS_OK : AE_INVALID_OPERATION);
}

Void IDLC::ResetSymbolList()
{
    sSymbolList->Clear();
    sRoot = NULL;
}

ARESULT IDLC::GenerateSymbolTree()
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > parent;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_Begin);
    sRoot = TreeNode<CSymbol>::Create(symbol);
    if (sRoot == NULL)
        return AE_OUTOFMEMORY;

    ARESULT ar = IDLC::FoundModule(sRoot);
    if (AFAILED(ar))
        exit(ar);

    if (gIDLFlag & IDL_PRINT_TREE)
        TraverseSymbolTree();

    printf("\n");

    if (!MapTable::SetParcelFunctionName())
        return AE_FAIL;

    if (gIDLFlag & IDL_LANG_CPP)
        WriteCppFile(sRoot);

    if (gIDLFlag & IDL_LANG_CS)
        WriteCsFile(sRoot);

    if (gIDLFlag & IDL_LANG_OBJC)
        WriteObjcFile(sRoot);

    if (gIDLFlag & IDL_LANG_JAVA)
        WriteJavaFile(sRoot);

    if (gIDLFlag & IDL_LANG_JAVASCRIPT)
        WriteJavascriptFile(sRoot);

    if (gIDLFlag & IDL_LANG_ES6)
        WriteEs6File(sRoot);

    printf("\n");
    ResetSymbolList();

    return AS_OK;
}

namespace IDLC {
    class CTreeNodeListener : public TreeNode<CSymbol>::ITreeNodeListener
    {
    public:

        VIRTUAL Void OnTraverseTreeNode(
            IN CONST Sp<TreeNode<CSymbol> > & node)
        {
            Sp<CSymbol> symbol = node->GetValue();
            Wp<TreeNode<CSymbol> > parent = node;

            Sp<String> padding = String::Create(L"");
            if (padding == NULL) {
                printf("error 1\n");
                return;
            }

            for (Int32 i = 0; i < symbol->mLevel; ++i) {
                const wchar_t * temp;
                if (i == 0)
                    temp = L"|--";
                else if (!parent->IsLastChild())
                    temp = L"|  ";
                else
                    temp = L"   ";

                Sp<String> str = String::Create(temp);
                if (str == NULL) {
                    printf("error 2\n");
                    return;
                }

                padding = str->Append(padding);
                if (padding == NULL) {
                    printf("error 3\n");
                    return;
                }

                parent = parent->GetParent();
            }

            printf("%S%S",
                padding->GetPayload(),
                sSymbolNameList[symbol->mSymbolType]);

            if (symbol->mContent != NULL)
                printf(": %S", (PCStr)symbol->mContent->GetPayload());

            printf("\n");
        }
    };

    STATIC Sp<CTreeNodeListener> sTreeNodeListener = new CTreeNodeListener();
} // namespace IDLC

Void IDLC::TraverseSymbolTree()
{
    if (sRoot == NULL)
        return;

    sRoot->Traverse(sTreeNodeListener);
}

ARESULT IDLC::FoundList(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_CloseAngleBracket);
    FoundVarType(parent);
    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_OpenAngleBracket);

    return AS_OK;
}

ARESULT IDLC::FoundTypeReferenceList(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    Sp<List<CSymbol> > list = new List<CSymbol>();
    if (list == NULL)
        return AE_FAIL;

    do {
        GET_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_TypeReference) {
            POP_LAST_SYMBOL(symbol);
            if (!list->PushFront(symbol))
                return AE_FAIL;
        }
        else {
            break;
        }
    } while (true);

    node = parent;
    Foreach(CSymbol, obj, list) {
        if (obj == NULL)
            return AE_FAIL;

        obj->mLevel = node->GetValue()->mLevel + 1;
        node = node->AddChildFront(obj);
        if (node == NULL)
            return AE_OUTOFMEMORY;
    }

    return AS_OK;
}

ARESULT IDLC::FoundVarType(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    GET_LAST_SYMBOL(symbol);
    if (symbol->mSymbolType == SymbolType_List) {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL_AND_GET_NODE(symbol, node);
        FoundList(node);
    }
    else if (symbol->mSymbolType == SymbolType_TypeReference) {
        FoundTypeReferenceList(parent);
    }
    else if ((symbol->mSymbolType == SymbolType_Int8)
        || (symbol->mSymbolType == SymbolType_Int8_NULL)
        || (symbol->mSymbolType == SymbolType_Byte)
        || (symbol->mSymbolType == SymbolType_Byte_NULL)
        || (symbol->mSymbolType == SymbolType_Int16)
        || (symbol->mSymbolType == SymbolType_Int16_NULL)
        || (symbol->mSymbolType == SymbolType_Int32)
        || (symbol->mSymbolType == SymbolType_Int32_NULL)
        || (symbol->mSymbolType == SymbolType_Int64)
        || (symbol->mSymbolType == SymbolType_Int64_NULL)
        || (symbol->mSymbolType == SymbolType_UInt8)
        || (symbol->mSymbolType == SymbolType_UInt8_NULL)
        || (symbol->mSymbolType == SymbolType_UInt16)
        || (symbol->mSymbolType == SymbolType_UInt16_NULL)
        || (symbol->mSymbolType == SymbolType_UInt32)
        || (symbol->mSymbolType == SymbolType_UInt32_NULL)
        || (symbol->mSymbolType == SymbolType_UInt64)
        || (symbol->mSymbolType == SymbolType_UInt64_NULL)
        || (symbol->mSymbolType == SymbolType_Float)
        || (symbol->mSymbolType == SymbolType_Float_NULL)
        || (symbol->mSymbolType == SymbolType_Double)
        || (symbol->mSymbolType == SymbolType_Double_NULL)
        || (symbol->mSymbolType == SymbolType_Boolean)
        || (symbol->mSymbolType == SymbolType_Boolean_NULL)
        || (symbol->mSymbolType == SymbolType_ByteArray)
        || (symbol->mSymbolType == SymbolType_String)) {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL(symbol);
    }
    else
        return AE_INVALIDARG;

    GET_LAST_SYMBOL(symbol);
    if (symbol->mSymbolType == SymbolType_LeftSquareBracket) {
        POP_LAST_SYMBOL(symbol);
        POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_ArrayLength);
        ADD_SYMBOL(symbol);
        POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_RightSquareBracket);
    }

    return AS_OK;
}

ARESULT IDLC::FoundMember(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_VarId);
    ADD_SYMBOL(symbol);
    FoundVarType(parent);

    return AS_OK;
}

ARESULT IDLC::FoundEnum(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_RBrace);
    while (true) {
        POP_LAST_SYMBOL(symbol);
        if ((symbol->mSymbolType == SymbolType_VarId)
            || (symbol->mSymbolType == SymbolType_COMMENT1)
            || (symbol->mSymbolType == SymbolType_COMMENT2))
            ADD_SYMBOL(symbol);
        else if (symbol->mSymbolType == SymbolType_LBrace)
            break;
        else if (symbol->mSymbolType == SymbolType_Comma)
            continue;
        else
            return AE_INVALIDARG;
    }

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_TypeReference);
    ADD_SYMBOL(symbol);

    return AS_OK;
}

ARESULT IDLC::FoundBaseClass(
    IN TreeNode<CSymbol> * parent)
{
    FoundTypeReferenceList(parent);

    return AS_OK;
}

ARESULT IDLC::FoundClass(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_RBrace);
    while (true) {
        POP_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_LBrace) {
            break;
        }
        else if (symbol->mSymbolType == SymbolType_Class) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundClass(node);
        }
        else if (symbol->mSymbolType == SymbolType_Enum) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundEnum(node);
        }
        else if (symbol->mSymbolType == SymbolType_Member) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundMember(node);
        }
        else if ((symbol->mSymbolType == SymbolType_COMMENT1) || (symbol->mSymbolType == SymbolType_COMMENT2)) {
            ADD_SYMBOL(symbol);
        }
        else if (symbol->mSymbolType == SymbolType_Semicolon) {
            continue;
        }
        else {
            DEBUG_PRINT(L"Invalid symbolList");
            return AE_INVALIDARG;
        }
    }

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_BaseClass);
    ADD_SYMBOL_AND_GET_NODE(symbol, node);
    FoundBaseClass(node);
    POP_LAST_SYMBOL(symbol);
    if ((symbol->mSymbolType == SymbolType_Serialize) || (symbol->mSymbolType == SymbolType_Unserialize)) {
        ADD_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_Serialize)
            gIDLFlag |= IDL_SERIALIZE;
    }
    else {
        DEBUG_PRINT(L"Invalid symbolList");
        return AE_INVALIDARG;
    }

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_TypeReference);
    ADD_SYMBOL(symbol);
    return AS_OK;
}

ARESULT IDLC::FoundUsingList(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    GET_LAST_SYMBOL(symbol);
    while (symbol->mSymbolType == SymbolType_Using) {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL_AND_GET_NODE(symbol, node);
        FoundTypeReferenceList(node);
        GET_LAST_SYMBOL(symbol);
    }
}

ARESULT IDLC::FoundFile(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    while (true) {
        POP_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_Class) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundClass(node);
        }
        else if (symbol->mSymbolType == SymbolType_Enum) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundEnum(node);
        }
        else if ((symbol->mSymbolType == SymbolType_COMMENT1) || (symbol->mSymbolType == SymbolType_COMMENT2)) {
            ADD_SYMBOL(symbol);
        }
        else if (symbol->mSymbolType == SymbolType_NameSpace) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            ARESULT ar = FoundNameSpace(node);
            if (AFAILED(ar))
                exit(ar);
        }
        else if (symbol->mSymbolType == SymbolType_UsingList) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundUsingList(node);
        }
        else if (symbol->mSymbolType == SymbolType_FileBegin) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundFile(node);
        }
        else if (symbol->mSymbolType == SymbolType_FileEnd) {
            return AS_OK;
        }
        else {
            DEBUG_PRINT(L"Invalid symbolList");
            return AE_INVALIDARG;
        }
    }

    return AS_OK;
}

ARESULT IDLC::FoundModule(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    while (true) {
        POP_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_Stmt) {
            symbol->mLevel = parent->GetValue()->mLevel + 1;
            parent = parent->AddChild(symbol);
            if (parent == NULL)
                return AE_OUTOFMEMORY;

            break;
        }
        else if (symbol->mSymbolType == SymbolType_Class) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundClass(node);
        }
        else if (symbol->mSymbolType == SymbolType_Enum) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundEnum(node);
        }
        else if ((symbol->mSymbolType == SymbolType_COMMENT1) || (symbol->mSymbolType == SymbolType_COMMENT2)) {
            ADD_SYMBOL(symbol);
        }
        else if (symbol->mSymbolType == SymbolType_NameSpace) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            ARESULT ar = FoundNameSpace(node);
            if (AFAILED(ar))
                exit(ar);
        }
        else if (symbol->mSymbolType == SymbolType_UsingList) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundUsingList(node);
        }
        else if (symbol->mSymbolType == SymbolType_FileBegin) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundFile(node);
        }
        else {
            DEBUG_PRINT(L"Invalid symbolList");
            return AE_INVALIDARG;
        }
    }

    return AS_OK;
}

ARESULT IDLC::FoundParameter(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_LAST_SYMBOL(symbol);
    ADD_SYMBOL(symbol);
    GET_LAST_SYMBOL(symbol);
    if (symbol->mSymbolType == SymbolType_List) {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL_AND_GET_NODE(symbol, node);
        FoundList(node);
    }
    else if (symbol->mSymbolType == SymbolType_TypeReference) {
        FoundTypeReferenceList(parent);
    }
    else {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL(symbol);
    }

    return AS_OK;
}

ARESULT IDLC::FoundParameterList(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_RParenthese);
    while (true) {
        POP_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_Parameter) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            if (AFAILED(FoundParameter(node)))
                return AE_FAIL;
        }
        else if (symbol->mSymbolType == SymbolType_LParenthese)
            break;
        else if (symbol->mSymbolType == SymbolType_Comma)
            continue;
        else
            return AE_FAIL;
    }

    return AS_OK;
}

ARESULT IDLC::FoundConstruction(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_ParameterList);
    ADD_SYMBOL_AND_GET_NODE(symbol, node);
    if (AFAILED(FoundParameterList(node)))
        return AE_FAIL;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_TypeReference);
    ADD_SYMBOL(symbol);

    return AS_OK;
}

ARESULT IDLC::FoundFunction(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_ParameterList);
    ADD_SYMBOL_AND_GET_NODE(symbol, node);
    if (AFAILED(FoundParameterList(node)))
        return AE_FAIL;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_TypeReference);
    ADD_SYMBOL(symbol);
    GET_LAST_SYMBOL(symbol);
    if (symbol->mSymbolType == SymbolType_List) {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL_AND_GET_NODE(symbol, node);
        FoundList(node);
    }
    else if (symbol->mSymbolType == SymbolType_TypeReference) {
        FoundTypeReferenceList(parent);
    }
    else {
        POP_LAST_SYMBOL(symbol);
        ADD_SYMBOL(symbol);
    }

    return AS_OK;
}

ARESULT IDLC::FoundInterface(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;


    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_RBrace);
    while (true) {
        POP_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_LBrace)
            break;
        else if (symbol->mSymbolType == SymbolType_Function) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            if (AFAILED(FoundFunction(node)))
                return AE_FAIL;
        }
        else if (symbol->mSymbolType == SymbolType_Construction) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            if (AFAILED(FoundConstruction(node)))
                return AE_FAIL;
        }
        else if ((symbol->mSymbolType == SymbolType_COMMENT1) || (symbol->mSymbolType == SymbolType_COMMENT2)) {
            ADD_SYMBOL(symbol);
        }
        else if (symbol->mSymbolType == SymbolType_Semicolon) {
            continue;
        }
        else {
            DEBUG_PRINT(L"Invalid symbolList");
            return AE_INVALIDARG;
        }
    }

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_TypeReference);
    ADD_SYMBOL(symbol);
    POP_LAST_SYMBOL(symbol);
    ADD_SYMBOL(symbol);
    return AS_OK;
}

ARESULT IDLC::FoundNameSpace(
    IN TreeNode<CSymbol> * parent)
{
    Sp<CSymbol> symbol;
    Sp<TreeNode<CSymbol> > node;

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_RBrace);
    while (true) {
        POP_LAST_SYMBOL(symbol);
        if (symbol->mSymbolType == SymbolType_LBrace) {
            break;
        }
        else if (symbol->mSymbolType == SymbolType_Class) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundClass(node);
        }
        else if (symbol->mSymbolType == SymbolType_Interface) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundInterface(node);
        }
        else if (symbol->mSymbolType == SymbolType_Enum) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            FoundEnum(node);
        }
        else if ((symbol->mSymbolType == SymbolType_COMMENT1) || (symbol->mSymbolType == SymbolType_COMMENT2)) {
            ADD_SYMBOL(symbol);
        }
        else if (symbol->mSymbolType == SymbolType_NameSpace) {
            ADD_SYMBOL_AND_GET_NODE(symbol, node);
            ARESULT ar = FoundNameSpace(node);
            if (AFAILED(ar))
                exit(ar);
        }
        else {
            DEBUG_PRINT(L"Invalid symbolList");
            return AE_INVALIDARG;
        }
    }

    POP_AND_CHECK_LAST_SYMBOL(symbol, SymbolType_TypeReference);
    ADD_SYMBOL(symbol);

    return AS_OK;
}

Sp<CSymbol> IDLC::GetLastSymbol()
{
    if (sSymbolList->IsEmpty())
        return NULL;
    else
        return sSymbolList->End()->GetPrev()->GetValue();
}

Sp<CSymbol> IDLC::PopLastSymbol()
{
    Sp<CSymbol> symbol = GetLastSymbol();
    if (symbol == NULL)
        return NULL;

    sSymbolList->PopBack();

    return symbol;
}
