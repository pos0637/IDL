%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "IDLCParser.h"

extern char * yytext;
extern int yylineno;
extern int newfile(char *fn);
void yyerror(const char *);
int yylex();
static void Dump(const char *fmt, ...);
char * pInterfaceName = NULL;
char * pConstructionName = NULL;
char * pNamespace = NULL;
char * gCurFileName = NULL;
int line;
%}

%union {
char * pstr;
}

%type <pstr> TypeReference TypeReferenceList OptTypeReference ReturnType
%type <pstr> Int8 Byte Int16 Int32 Int64
%type <pstr> Int8_NULL Byte_NULL Int16_NULL Int32_NULL Int64_NULL
%type <pstr> Double Double_NULL ByteArray String List Boolean Boolean_NULL Void

%token TOK_namespace
%token TOK_class
%token TOK_enum
%token TOK_ClassName
%token TOK_Serialize
%token TOK_Unserialize
%token TOK_filename
%token <pstr> TOK_Void
%token <pstr> TOK_Int8
%token <pstr> TOK_Int8_NULL
%token <pstr> TOK_Byte
%token <pstr> TOK_Byte_NULL
%token <pstr> TOK_Int16
%token <pstr> TOK_Int16_NULL
%token <pstr> TOK_Int32
%token <pstr> TOK_Int32_NULL
%token <pstr> TOK_Int64
%token <pstr> TOK_Int64_NULL
%token <pstr> TOK_UInt8
%token <pstr> TOK_UInt8_NULL
%token <pstr> TOK_UInt16
%token <pstr> TOK_UInt16_NULL
%token <pstr> TOK_UInt32
%token <pstr> TOK_UInt32_NULL
%token <pstr> TOK_UInt64
%token <pstr> TOK_UInt64_NULL
%token <pstr> TOK_Float
%token <pstr> TOK_Float_NULL
%token <pstr> TOK_Double
%token <pstr> TOK_Double_NULL
%token <pstr> TOK_Boolean
%token <pstr> TOK_Boolean_NULL
%token <pstr> TOK_ByteArray
%token <pstr> TOK_String
%token <pstr> TOK_List
%token <pstr> TOK_TypeReference
%token TOK_LBRACE
%token TOK_RBRACE
%token TOK_SEMICOLON
%token TOK_COMMA
%token TOK_COLON
%token TOK_DASH
%token TOK_DIGITS
%token TOK_EQUALITY
%token TOK_DOT
%token TOK_SLASH
%token TOK_OpenAngleBracket
%token TOK_CloseAngleBracket
%token TOK_LeftSquareBracket
%token TOK_RightSquareBracket
%token TOK_public
%token TOK_protected
%token TOK_private
%token TOK_byte
%token TOK_char
%token TOK_Char
%token TOK_Char_NULL
%token TOK_short
%token TOK_ushort
%token TOK_int
%token TOK_uint
%token TOK_long
%token TOK_ulong
%token TOK_bool
%token TOK_true
%token TOK_false
%token TOK_TRUE
%token TOK_FALSE
%token TOK_COMMENT1
%token TOK_COMMENT2
%token TOK_new
%token TOK_abstract
%token TOK_virtual
%token TOK_ArrayLength
%token TOK_Question
%token TOK_Dollar
%token TOK_using
%token TOK_LPARENTHESE
%token TOK_RPARENTHESE
%token TOK_Interface
%token TOK_static
%token TOK_singleton
%token TOK_asterisk
%%

Module : { Dump("Stmt"); IDLC::AddItem(IDLC::SymbolType_Stmt, NULL); }  File { Dump("Begin"); IDLC::AddItem(IDLC::SymbolType_Begin, NULL); }
    ;

File : OptIncludeFile OptUsing Stmt
    ;

OptIncludeFile :
    | IncludeFiles
    ;

IncludeFiles : IncludeFiles IncludeFile
    | IncludeFile
    ;

IncludeFile : FileName File
    ;

FileName : TOK_filename {
    if (!newfile(yytext))
        return 0;
    }
    ;

OptUsing :
    | UsingList { Dump("SymbolType_UsingList"); IDLC::AddItem(IDLC::SymbolType_UsingList, NULL); }
    ;

UsingList : UsingList Using
    | Using
    ;

Using : TOK_using  TypeReferenceList  { Dump("SymbolType_Using"); IDLC::AddItem(IDLC::SymbolType_Using, NULL); }
    ;

Stmt : Namespace
    ;

Namespace : OptComment TOK_namespace TypeReference {
        pNamespace = (char *)malloc(strlen($3) + 1);
        if (!pNamespace)
            return 0;

        strncpy(pNamespace, $3, strlen($3));
        pNamespace[strlen($3)] = '\0';
    } LBrace OptNamespaceMember RBrace {
        if (pNamespace) {
            free(pNamespace);
            pNamespace = NULL;
        }

        Dump("SymbolType_NameSpace");
        IDLC::AddItem(IDLC::SymbolType_NameSpace, NULL);
    }
    ;

OptNamespaceMember : OptNamespaceMember NamespaceMember
    | NamespaceMember
    ;

NamespaceMember : Class
    | Enum
    | Interface
    ;

Class : OptComment TOK_class TypeReference OptSerialize BaseClass LBrace OptClassMemberList RBrace { Dump("SymbolType_Class"); IDLC::AddItem(IDLC::SymbolType_Class, NULL); }
    ;

Enum : OptComment TOK_enum TypeReference LBrace OptEnumMemberList RBrace { Dump("SymbolType_Enum"); IDLC::AddItem(IDLC::SymbolType_Enum, NULL); }
    ;

OptClassMemberList :
    | ClassMemberList
    ;

ClassMemberList : ClassMember ClassMemberList
    | ClassMember
    ;

ClassMember : OptComment TypeName VarId { Dump("ClassMember"); IDLC::AddItem(IDLC::SymbolType_Member, NULL); } Semicolon
    | Class
    | Enum
    ;

OptEnumMemberList :
    | EnumMemberList
    ;

EnumMemberList : EnumMemberList EnumMember
    | EnumMember
    ;

EnumMember : OptComment VarId
    | OptComment VarId Comma
    ;

Interface : OptComment TokInterface TypeReference {
        pInterfaceName = (char *)malloc(strlen(yytext) + 1);
        if (!pInterfaceName)
            return 0;

        strncpy(pInterfaceName, yytext, strlen(yytext));
        pInterfaceName[strlen(yytext)] = '\0';
    } LBrace InterfaceMemberList RBrace {
        if (pInterfaceName) {
            free(pInterfaceName);
            pInterfaceName = NULL;
        }
        Dump("SymbolType_Interface");
        IDLC::AddItem(IDLC::SymbolType_Interface, NULL);
    }
    ;

OptSingleton : { Dump("nonsingleton"); IDLC::AddItem(IDLC::SymbolType_Multiple, NULL); }
    | TOK_singleton { Dump("singleton"); IDLC::AddItem(IDLC::SymbolType_Singleton, NULL); }
    ;

InterfaceMemberList : InterfaceMemberList InterfaceMember
    | InterfaceMember
    ;

InterfaceMember : OptComment Function Semicolon
    ;

Function : TypeReference {
        if (strcmp(pInterfaceName, $1)) {
            printf("The constructor should be the same as the name of the interface!\n");
            return 0;
        }
    } FunctionStatement Construction
    | ReturnType MemberFunctionName FunctionStatement MemberFunction
    ;

MemberFunctionName : TypeReference {
        if (pInterfaceName && $1) {
            if (!strcmp(pInterfaceName, $1)) {
                printf("Constructors there can be no return value!\n");
                return 0;
            }
        }
    }
    ;

FunctionStatement : LParenthese OptParameterList RParenthese { Dump("SymbolType_ParameterList"); IDLC::AddItem(IDLC::SymbolType_ParameterList, NULL); }
    ;

MemberFunction : { Dump("SymbolType_Function"); IDLC::AddItem(IDLC::SymbolType_Function, NULL); }
    ;

Construction : { Dump("SymbolType_Construction"); IDLC::AddItem(IDLC::SymbolType_Construction, NULL); }
    ;

ReturnType : Void
    | Int8
    | Int8_NULL
    | Byte
    | Byte_NULL
    | Int16
    | Int16_NULL
    | Int32
    | Int32_NULL
    | Int64
    | Int64_NULL
    | Double
    | Double_NULL
    | Boolean
    | Boolean_NULL
    | ByteArray
    | String
    | List
    | TypeReferenceList
    ;

OptParameterList :
    | ParameterList
    ;

ParameterList : ParameterList Parameter
    | ParameterType VarId { Dump("Parameter"); IDLC::AddItem(IDLC::SymbolType_Parameter, NULL); }
    ;

Parameter : Comma ParameterType VarId { Dump("Parameter"); IDLC::AddItem(IDLC::SymbolType_Parameter, NULL); }
    ;

ParameterType : Int8
    | Byte
    | Int16
    | Int32
    | Int64
    | Double
    | Boolean
    | ByteArray
    | String
    | List
    | TypeReferenceList
    ;

OptSerialize : { Dump("SymbolType_Serialize");  IDLC::AddItem(IDLC::SymbolType_Serialize, NULL); }
    | TOK_Serialize { Dump("SymbolType_Serialize");  IDLC::AddItem(IDLC::SymbolType_Serialize, NULL); }
    | TOK_Unserialize { Dump("SymbolType_Unserialize");  IDLC::AddItem(IDLC::SymbolType_Unserialize, NULL); }
    ;

OptComment :
    | CommentList
    ;

CommentList : CommentList Comment
    | Comment
    ;

Comment : Comment1
    | Comment2
    ;

BaseClass :
    {
        Dump("SymbolType_TypeReference");
        IDLC::AddItem(IDLC::SymbolType_TypeReference, NULL);
        Dump("SymbolType_BaseClass");
        IDLC::AddItem(IDLC::SymbolType_BaseClass, NULL);
    }
    | TOK_COLON TypeReferenceList
    {
        Dump("SymbolType_BaseClass");
        IDLC::AddItem(IDLC::SymbolType_BaseClass, NULL);
    }
    ;

TypeName : Int8
    | Int8_NULL
    | Byte
    | Byte_NULL
    | Int16
    | Int16_NULL
    | Int32
    | Int32_NULL
    | Int64
    | Int64_NULL
    | Double
    | Double_NULL
    | Boolean
    | Boolean_NULL
    | ByteArray
    | String
    | List
    | TypeReferenceList
    ;

List : TOK_List OpenAngleBracket ListTypeName CloseAngleBracket  { Dump("SymbolType_List"); IDLC::AddItem(IDLC::SymbolType_List, NULL); }
    ;

ListTypeName : Int8_NULL
    | Byte_NULL
    | Int16_NULL
    | Int32_NULL
    | Int64_NULL
    | Double_NULL
    | Boolean_NULL
    | String
    | ByteArray
    | TypeReferenceList
    ;

TypeReferenceList : OptTypeReference TypeReferenceList
    | TypeReference
    ;

OptTypeReference : TypeReference Dot
    ;

TokInterface : OptSingleton TOK_Interface
    ;

TypeReference : TOK_TypeReference {
        $$ = (char*)malloc(strlen(yytext) + 1);
        if (!$$)
            return 0;

        strncpy($$, yytext, strlen(yytext));
        $$[strlen(yytext)] = '\0';
        Dump("SymbolType_TypeReferenc:%s", yytext);
        IDLC::AddItem(IDLC::SymbolType_TypeReference, yytext);
    }
    ;

VarId : TOK_TypeReference { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_VarId, yytext); }
    ;

Void : TOK_Void { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Void, NULL); }
    ;

Int8 : TOK_Int8 { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int8, NULL); }
    ;

Int8_NULL : TOK_Int8_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int8_NULL, NULL); }
    ;

Byte : TOK_Byte { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Byte, NULL); }
    ;

Byte_NULL : TOK_Byte_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Byte_NULL, NULL); }
    ;

Int16 : TOK_Int16 { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int16, NULL); }
    ;

Int16_NULL : TOK_Int16_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int16_NULL, NULL); }
    ;

Int32 : TOK_Int32 { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int32, NULL); }
    ;

Int32_NULL : TOK_Int32_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int32_NULL, NULL); }
    ;

Int64 : TOK_Int64 { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int64, NULL); }
    ;

Int64_NULL : TOK_Int64_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Int64_NULL, NULL); }
    ;

Double : TOK_Double { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Double, NULL); }
    ;

Double_NULL : TOK_Double_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Double_NULL, NULL); }
    ;

Boolean : TOK_Boolean { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Boolean, NULL); }
    ;

Boolean_NULL : TOK_Boolean_NULL { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Boolean_NULL, NULL); }
    ;

ByteArray : TOK_ByteArray { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_ByteArray, NULL); }
    ;

String : TOK_String { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_String, NULL); }
    ;

Semicolon : TOK_SEMICOLON { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Semicolon, NULL); }
    ;

OpenAngleBracket : TOK_OpenAngleBracket { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_OpenAngleBracket, NULL); }
    ;

CloseAngleBracket : TOK_CloseAngleBracket { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_CloseAngleBracket, NULL); }
    ;

LBrace : TOK_LBRACE { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_LBrace, NULL); }
    ;

RBrace : TOK_RBRACE { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_RBrace, NULL); }
    ;

LParenthese : TOK_LPARENTHESE { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_LParenthese, NULL); }
    ;

RParenthese : TOK_RPARENTHESE { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_RParenthese, NULL); }
    ;

Comma : TOK_COMMA { Dump("%s", yytext); IDLC::AddItem(IDLC::SymbolType_Comma, NULL); }
    ;

Dot : TOK_DOT
    ;

Comment1 : TOK_COMMENT1 { Dump("SymbolType_COMMENT1 %s", yytext); IDLC::AddItem(IDLC::SymbolType_COMMENT1, yytext); }
    ;

Comment2 : TOK_COMMENT2 { Dump("SymbolType_COMMENT2 %s", yytext); IDLC::AddItem(IDLC::SymbolType_COMMENT2, yytext); }
    ;
%%

void yyerror(const char * s)
{
    printf("bison: yyerror:%d %s %s %s\n", yylineno, s, yytext, gCurFileName);
    exit(1);
}

static void Dump(const char *fmt, ...)
{
#ifdef DEBUG_MODE
    va_list ap;
    va_start(ap, fmt);
    printf("bison found:%d: ", yylineno);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    va_end(ap);
#endif // DEBUG_MODE
}