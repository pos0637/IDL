
%option noyywrap yylineno
%x IFILE

%{
#include <stdarg.h>
#include "IDL.tab.hpp"
#include "IDLCParser.h"
static void Dump(const char *fmt, ...);
extern char * gCurFileName;

struct bufstack {
    struct bufstack *prev;
    YY_BUFFER_STATE bs;
    int lineno;
    char *filename;
    FILE *f;
} *curbs = 0;
int newfile(char *fn);
int popfile(void);

%}

DIGITS [0-9]+
WHITE_SPACE    [\t\r\n ]

%%
^import[ \t]*[\"<]   { BEGIN IFILE; }
<IFILE>[^ \t\n\">]+ {

    {
        int c;
        while ((c = yyinput()) && c != '\n');
    }

    return TOK_filename;
   // if (!newfile(yytext))
  //      yyterminate();

  //  BEGIN INITIAL;
}


<IFILE>.|\n {
    printf("%4d bad import line\n", yylineno);
    yyterminate();
}

<<EOF>> {
    char * filename = strdup(gCurFileName);
    if (!popfile()) {
        free(filename);
        yyterminate();
    }

    Dump("SymbolType_FileBegin: %s", filename);
    IDLC::AddItem(IDLC::SymbolType_FileBegin, filename);
    free(filename);
}

"namespace"                    { Dump("namespace"); return TOK_namespace; }
"class"                        { Dump("class"); return TOK_class; }
"enum"                         { Dump("enum"); return TOK_enum; }
"interface"                    { Dump("TOK_Interface"); return TOK_Interface; }
"singleton"                    { Dump("TOK_singleton"); return TOK_singleton; }
"Void"                         { Dump("Void"); return TOK_Void; }
"Int8"                         { Dump("Int8"); return TOK_Int8; }
"Byte"                         { Dump("Byte"); return TOK_Byte; }
"Int8?"                        { Dump("Int8?"); return TOK_Int8_NULL; }
"Byte?"                        { Dump("Byte?"); return TOK_Byte_NULL; }
"Int16"                        { Dump("Int16"); return TOK_Int16; }
"Int16?"                       { Dump("Int16?"); return TOK_Int16_NULL; }
"Int32"                        { Dump("Int32"); return TOK_Int32; }
"Int32?"                       { Dump("Int32?"); return TOK_Int32_NULL; }
"Int64"                        { Dump("Int64"); return TOK_Int64; }
"Int64?"                       { Dump("Int64?"); return TOK_Int64_NULL; }
"UInt8"                        { Dump("UInt8"); return TOK_UInt8; }
"UInt8?"                       { Dump("UInt8?"); return TOK_UInt8_NULL; }
"UInt16"                       { Dump("UInt16"); return TOK_UInt16; }
"UInt16?"                      { Dump("UInt16?"); return TOK_UInt16_NULL; }
"UInt32"                       { Dump("UInt32"); return TOK_UInt32; }
"UInt32?"                      { Dump("UInt32?"); return TOK_UInt32_NULL; }
"UInt64"                       { Dump("UInt64"); return TOK_UInt64; }
"UInt64?"                      { Dump("UInt64?"); return TOK_UInt64_NULL; }
"Float"                        { Dump("Float"); return TOK_Float; }
"Float?"                       { Dump("Float?"); return TOK_Float_NULL; }
"Double"                       { Dump("Double"); return TOK_Double; }
"Double?"                      { Dump("Double?"); return TOK_Double_NULL; }
"Boolean"                      { Dump("Boolean"); return TOK_Boolean; }
"Boolean?"                     { Dump("Boolean?"); return TOK_Boolean_NULL; }
"ByteArray"                    { Dump("ByteArray"); return TOK_ByteArray; }
"String"                       { Dump("String"); return TOK_String; }
"List"                         { Dump("List"); return TOK_List; }
"public"                       { Dump("public"); return TOK_public; }
"protected"                    { Dump("protected"); return TOK_protected; }
"private"                      { Dump("private"); return TOK_private; }
"char"                         { Dump("char"); return TOK_char; }
"Char"                         { Dump("Char"); return TOK_Char; }
"Char?"                        { Dump("Char?"); return TOK_Char_NULL; }
"short"                        { Dump("short"); return TOK_short; }
"ushort"                       { Dump("ushort"); return TOK_ushort; }
"int"                          { Dump("int"); return TOK_int; }
"uint"                         { Dump("uint"); return TOK_uint; }
"long"                         { Dump("long"); return TOK_long; }
"ulong"                        { Dump("ulong"); return TOK_ulong; }
"bool"                         { Dump("bool"); return TOK_bool; }
"true"                         { Dump("true"); return TOK_true; }
"false"                        { Dump("false"); return TOK_false; }
"TRUE"                         { Dump("TRUE"); return TOK_TRUE; }
"FALSE"                        { Dump("FALSE"); return TOK_FALSE; }
"new"                          { Dump("new"); return TOK_new; }
"abstract"                     { Dump("abstract"); return TOK_abstract; }
"virtual"                      { Dump("virtual"); return TOK_virtual; }
"using"                        { Dump("using"); return TOK_using; }
"static"                       { Dump("static"); return TOK_static; }

{WHITE_SPACE}+
"--"[^\n\r]*

"{"                            { Dump("TOK_LBRACE"); return TOK_LBRACE; }

"}"                            { Dump("TOK_RBRACE"); return TOK_RBRACE; }

"("                            { Dump("TOK_LPARENTHESE"); return TOK_LPARENTHESE; }

")"                            { Dump("TOK_RPARENTHESE"); return TOK_RPARENTHESE; }

";"                            { Dump("TOK_SEMICOLON"); return TOK_SEMICOLON; }

","                            { Dump("TOK_COMMA"); return TOK_COMMA; }

":"                            { Dump("TOK_COLON"); return TOK_COLON; }

"."                            { Dump("TOK_DOT"); return TOK_DOT; }

"<"                            { Dump("TOK_OpenAngleBracket"); return TOK_OpenAngleBracket; }

">"                            { Dump("TOK_CloseAngleBracket"); return TOK_CloseAngleBracket; }

"?"                            { Dump("TOK_Question"); return TOK_Question; }

"$"                            { Dump("TOK_Dollar"); return TOK_Dollar; }

"*"                            { Dump("TOK_asterisk"); return TOK_asterisk; }

"//"[^\n\r]*                   { Dump("COMMENT1"); return TOK_COMMENT1; }

"/*"([^*]|\*+[^/*])*"*/"       { Dump("COMMENT2"); return TOK_COMMENT2; }

=                              { Dump("TOK_EQUALITY"); return TOK_EQUALITY; }

"[N]"                          { Dump("TOK_Unserialize"); return TOK_Unserialize; }

"[Y]"                          { Dump("TOK_Serialize"); return TOK_Serialize; }

"["                            { Dump("TOK_LeftSquareBracket"); return TOK_LeftSquareBracket; }

"]"                            { Dump("TOK_RightSquareBracket"); return TOK_RightSquareBracket; }

"/"                            { Dump("TOK_SLASH"); return TOK_SLASH; }

[a-zA-Z_][A-Za-z0-9_]*         { Dump("TOK_TypeReference"); return TOK_TypeReference; }

([2-9]|[1-9][0-9]+)            { Dump("TOK_ArrayLength"); return TOK_ArrayLength; }

{DIGITS}                       { Dump("TOK_DIGITS"); return TOK_DIGITS; }
%%

int newfile(char *fn)
{
    FILE *f = fopen(fn, "r");
    struct bufstack *bs = (struct bufstack*)malloc(sizeof(struct bufstack));
    if (!f) {perror(fn); return 0;}
    if (!bs) {perror("malloc"); exit(1);}

    if (curbs) curbs->lineno = yylineno;
    bs->prev = curbs;
    bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
    bs->f = f;
    bs->filename = fn;
    yy_switch_to_buffer(bs->bs);
    curbs = bs;
    yylineno = 1;
    gCurFileName = fn;
    Dump("SymbolType_FileEnd: %s", gCurFileName);
    IDLC::AddItem(IDLC::SymbolType_FileEnd, gCurFileName);
    BEGIN INITIAL;
    return 1;
}

int popfile(void)
{
    struct bufstack *bs = curbs;
    struct bufstack *prevbs;

    if (!bs) return 0;

    fclose(bs->f);
    yy_delete_buffer(bs->bs);

    prevbs = bs->prev;
    free(bs);

    if (!prevbs) return 0;

    yy_switch_to_buffer(prevbs->bs);
    curbs = prevbs;
    yylineno = curbs->lineno;
    gCurFileName = curbs->filename;
    return 1;
}

static void Dump(const char *fmt, ...)
{
#ifdef DEBUG_MODE
    va_list ap;
    va_start(ap, fmt);
    printf("flex found:%d: ", yylineno);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    va_end(ap);
#endif // DEBUG_MODE
}