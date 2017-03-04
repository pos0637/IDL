
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include "AXP/cplusplus/xplatform/include/astring.h"
#include "AXP/cplusplus/xplatform/include/stl/hashtable.h"
#include "IDLCParser.h"
#include "Common.h"

using namespace AXP;
using namespace AXP::STL;
using namespace IDLC;

extern int yyparse(void);
extern FILE * yyin;
extern char * gCurFileName;
int newfile(char *fn);

namespace IDLC {
    Sp<HashTable<PCWStr, CMappingInfo> > gMappingCpp = new HashTable<PCWStr, CMappingInfo>(1024);
    Sp<HashTable<PCWStr, CMappingInfo> > gMappingCs = new HashTable<PCWStr, CMappingInfo>(1024);
    Sp<HashTable<PCWStr, CMappingInfo> > gMappingJavascript = new HashTable<PCWStr, CMappingInfo>(10);
    STATIC Sp<List<String> > sIncludeFileListCpp = new List<String>();
    STATIC Sp<List<String> > sImportStubFilesJavascript = new List<String>();
    Sp<List<CMappingInfo> > gImportFileEs6 = new List<CMappingInfo>();

    STATIC Sp<String> sFileName = NULL;
    Sp<String> gRootOutputDir = NULL;
    Sp<String> gNamespacz = NULL;
    char * gInputDir = NULL;
    int gIDLFlag = IDL_NOP;
    CMappingArea * gShareAreaCpp = NULL;
    CMappingArea * gShareAreaCs = NULL;
    CMappingArea * gShareAreaJavascript = NULL;
    CMappingArea * gShareAreaEs6 = NULL;

    Sp<String> GetOrignalFileName()
    {
        return String::Create(sFileName);
    }
}

static void Usage(const char *av)
{
    fprintf(stderr, "Usage: %s [options] <filename.idl>\n", av);
    fprintf(stderr, "Where options are:\n");
    fprintf(stderr,
        "  -java        Generate \"Java\" Source Code\n"
        "  -javascript  Generate \"Javascript\" Source Code\n"
        "  -es6  Generate \"es6\" Source Code\n"
        "  -csharp      Generate \"CSharp\" Source Code\n"
        "  -cpp         Generate \"Cpp\" Source Code\n"
        "  -objc        Generate \"Objective-C\" Source Code\n"
        "  -output <Dir>     Directory with Source Code files\n"
        "  -g           Generate Include File\n"
        "  -p           Print tree\n");
}

static int ProcessFile(char * file)
{
    if (!file)
        return -1;

    yyin = fopen(file, "r");
    if (!yyin)
        return -1;

    gCurFileName = (char*)calloc(1, strlen(file) + 1);
    if (gCurFileName == NULL)
        return -1;

    memcpy(gCurFileName, file, strlen(file));
    if (!newfile(gCurFileName))
        return -1;

    yyparse();
    fclose(yyin);
    free(gCurFileName);
}

static int ProcessDir(char * dir, int depth)
{
    if (!dir)
        return -1;

    if (depth < 1)
        return -1;

    depth--;

    DIR* pDir;
    struct dirent* file;
    if (!(pDir = opendir(dir)))
        return -1;

    while ((file = readdir(pDir)) != 0)
    {
        if (strncmp(file->d_name, ".", 1) == 0)
            continue;

        struct stat info;
        stat(file->d_name, &info);
        if (S_ISDIR(info.st_mode)) {
            if (depth > 0)
                ProcessDir(file->d_name, depth);
        }
        else {
            Sp<String> fileName = String::Create(file->d_name);
            if (fileName == NULL)
                return -1;

            Int32 index1 = fileName->LastIndexOf(L'/');
            Int32 index2 = fileName->LastIndexOfString(L".idl");
            if (index2 == -1)
                continue;

            sFileName = fileName->SubString(index1 + 1, index2 - index1 - 1);
            if (sFileName == NULL)
                return -1;

            int fd;
            fd = open("/dev/zero", O_RDWR);
            if (fd < 0) {
                printf("open error!\n");
                return -1;
            }

            gNamespacz = NULL;
            if (gIDLFlag & IDL_LANG_CPP) {
                gShareAreaCpp = (CMappingArea*)mmap(NULL, sizeof(CMappingArea)* 1024, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
                if (gShareAreaCpp == MAP_FAILED) {
                    printf("map error\n");
                    return -1;
                }
            }
            if (gIDLFlag & IDL_LANG_CS) {
                gShareAreaCs = (CMappingArea*)mmap(NULL, sizeof(CMappingArea)* 1024, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
                if (gShareAreaCs == MAP_FAILED) {
                    printf("map error\n");
                    return -1;
                }
            }
            if (gIDLFlag & IDL_LANG_JAVASCRIPT) {
                gShareAreaJavascript = (CMappingArea*)mmap(NULL, sizeof(CMappingArea)* 1024, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
                if (gShareAreaJavascript == MAP_FAILED) {
                    printf("map error\n");
                    return -1;
                }
            }
            if (gIDLFlag & IDL_LANG_ES6) {
                gShareAreaEs6 = (CMappingArea*)mmap(NULL, sizeof(CMappingArea)* 1024, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
                if (gShareAreaEs6 == MAP_FAILED) {
                    printf("map error\n");
                    return -1;
                }
            }
            
            pid_t fpid;
            fpid = fork();
            if (fpid < 0) {
                printf("error in fork!");
                break;
            }
            else if (fpid == 0) {
                ProcessFile(file->d_name);
                Int32 length = 0;
                if (gIDLFlag & IDL_LANG_CPP) {
                    Wcscpy_s(gShareAreaCpp->mDescription, 256, L"", &length);
                    if (gNamespacz)
                        Wcscpy_s(gShareAreaCpp->mScope, 256, (PCWStr)*gNamespacz, &length);
                    else
                        Wcscpy_s(gShareAreaCpp->mScope, 256, L"", &length);
                }
                if (gIDLFlag & IDL_LANG_CS) {
                    Wcscpy_s(gShareAreaCs->mDescription, 256, L"", &length);
                    if (gNamespacz)
                        Wcscpy_s(gShareAreaCs->mScope, 256, (PCWStr)*gNamespacz, &length);
                    else
                        Wcscpy_s(gShareAreaCs->mScope, 256, L"", &length);
                }
                if (gIDLFlag & IDL_LANG_JAVASCRIPT) {
                    Wcscpy_s(gShareAreaJavascript->mDescription, 256, L"", &length);
                    if (gNamespacz)
                        Wcscpy_s(gShareAreaJavascript->mScope, 256, (PCWStr)*gNamespacz, &length);
                    else
                        Wcscpy_s(gShareAreaJavascript->mScope, 256, L"", &length);
                }
                if (gIDLFlag & IDL_LANG_ES6) {
                    Wcscpy_s(gShareAreaEs6->mDescription, 256, L"", &length);
                    if (gNamespacz)
                        Wcscpy_s(gShareAreaEs6->mScope, 256, (PCWStr)*gNamespacz, &length);
                    else
                        Wcscpy_s(gShareAreaEs6->mScope, 256, L"", &length);
                }
                
                exit(0);
            }
            else {
                int status;
                waitpid(fpid, &status, 0);
                if (gIDLFlag & IDL_LANG_CPP) {
                    CMappingArea * pMapArea = gShareAreaCpp;
                    while (true) {
                        if (!Wcscmp(pMapArea->mDescription, L"")) {
                            if (!Wcscmp(pMapArea->mScope, L"")) {
                                pMapArea++;
                                break;
                            }

                            gNamespacz = String::Create(pMapArea->mScope);
                            if (!gNamespacz)
                                return -1;

                            Sp<String> includeFileStr = String::Create(gNamespacz->Length() + sFileName->Length() + 38,
                                L"#include \"%ls/%ls.h\"\r\n", (PCWStr)*gNamespacz, (PCWStr)*sFileName);
                            if (includeFileStr == NULL)
                                return -1;

                            if (!sIncludeFileListCpp->PushBack(includeFileStr))
                                return -1;

                            pMapArea++;
                            break;
                        }

                        Sp<String> description = String::Create(pMapArea->mDescription);
                        if (!description)
                            return -1;

                        Sp<String> scope = String::Create(pMapArea->mScope);
                        if (!scope)
                            return -1;

                        Sp<CMappingInfo> mappingInfo = new CMappingInfo();
                        if (!mappingInfo)
                            return -1;

                        mappingInfo->mDescription = description;
                        mappingInfo->mScope = scope;
                        gMappingCpp->InsertUnique((PCWStr)description->GetPayload(), mappingInfo);
                        pMapArea++;
                    }
                }

                if (gIDLFlag & IDL_LANG_CS) {
                    CMappingArea * pMapArea = gShareAreaCs;
                    while (true) {
                        if (!Wcscmp(pMapArea->mDescription, L"")) {
                            pMapArea++;
                            break;
                        }

                        Sp<String> description = String::Create(pMapArea->mDescription);
                        if (!description)
                            return -1;

                        Sp<String> scope = String::Create(pMapArea->mScope);
                        if (!scope)
                            return -1;

                        Sp<CMappingInfo> mappingInfo = new CMappingInfo();
                        if (!mappingInfo)
                            return -1;

                        mappingInfo->mDescription = description;
                        mappingInfo->mScope = scope;

                        gMappingCs->InsertUnique((PCWStr)description->GetPayload(), mappingInfo);
                        pMapArea++;
                    }
                }

                if (gIDLFlag & IDL_LANG_JAVASCRIPT) {
                    CMappingArea * pMapArea = gShareAreaJavascript;
                    while (true) {
                        if (!Wcscmp(pMapArea->mDescription, L"")) {
                            pMapArea++;
                            break;
                        }

                        Sp<String> namespacz = String::Create(pMapArea->mDescription);
                        if (!namespacz)
                            return -1;

                        Sp<String> description = String::Create(519, L"%ls.%ls", pMapArea->mDescription, pMapArea->mScope);
                        if (description == NULL)
                            return -1;

                        Sp<String> scope = String::Create(pMapArea->mScope);
                        if (!scope)
                            return -1;

                        Sp<CMappingInfo> mappingInfo = new CMappingInfo();
                        if (!mappingInfo)
                            return -1;

                        mappingInfo->mDescription = namespacz;
                        mappingInfo->mScope = scope;

                        gMappingJavascript->InsertUnique((PCWStr)description->GetPayload(), mappingInfo);
                        pMapArea++;
                        Sp<String> importFile = String::Create(namespacz->Length() + sFileName->Length() + 77,
                            L", \"IPC/gen/%ls/%lsStub\"", (PCWStr)*namespacz, (PCWStr)*sFileName);
                        if (importFile == NULL)
                            return -1;

                        if (!sImportStubFilesJavascript->PushBack(importFile))
                            return -1;
                    }
                }
                
                if (gIDLFlag & IDL_LANG_ES6) {
                    CMappingArea * pMapArea = gShareAreaEs6;
                    while (true) {
                        if (!Wcscmp(pMapArea->mDescription, L"")) {
                            pMapArea++;
                            break;
                        }

                        Sp<String> namespacz = String::Create(pMapArea->mDescription);
                        if (!namespacz)
                            return -1;

                        Sp<String> scope = String::Create(pMapArea->mScope);
                        if (!scope)
                            return -1;

                        Sp<CMappingInfo> mappingInfo = new CMappingInfo();
                        if (!mappingInfo)
                            return -1;

                        mappingInfo->mDescription = namespacz;
                        mappingInfo->mScope = scope;
                        pMapArea++;
                        if (!gImportFileEs6->PushBack(mappingInfo))
                            return -1;
                    }
                }
            }
        }
    }

    closedir(pDir);
    return 0;
}

STATIC Void WriteCppMapping()
{
    Sp<List<CMappingInfo> > mappingList = gMappingCpp->GetValues();
    if ((mappingList == NULL) || (mappingList->GetCount() < 1))
        return;

    Sp<String> dirPath = String::Create(gRootOutputDir->Length() + 37, L"%ls/cplusplus/", (PCWStr)*gRootOutputDir);
    if (dirPath == NULL)
        return;

    GenerateDirPath(dirPath);
    Sp<String> fullFileName = String::Create(dirPath->Length() + 24, L"%ls/MappingTable.cpp", (PCWStr)*dirPath);
    if (fullFileName == NULL)
        return;

    FILE * file = OpenFile(fullFileName);
    if (file == NULL)
        return;

    Foreach(String, obj, sIncludeFileListCpp) {
        WRITE_STRING_TO_FILE(obj);
    }

    FWRITE("\r\n");
    FWRITE("AXP::Boolean InitMappingTable()\r\n{\r\n");
    Foreach(CMappingInfo, obj, mappingList) {
        WRITE_STRING_TO_FILE(obj->mDescription->Length() + obj->mScope->Length() + 87,
            L"if (!AXP::Libc::Common::ClassLoader::RegisterClassCreator(L\"%ls\", %ls::Create))\r\n", (PCWStr)*obj->mDescription, (PCWStr)*obj->mScope);
        FWRITE("return FALSE;\r\n\r\n");
    }

    FWRITE("return TRUE;\r\n");
    FWRITE("}\r\n\r\n");
    FWRITE("AXP::Boolean __sClassMappingTable__ = InitMappingTable();\r\n");

    fclose(file);
    FormatFile(fullFileName, IDL_LANG_CPP);
}

STATIC Void WriteCsMapping()
{
    Sp<List<CMappingInfo> > mappingList = gMappingCs->GetValues();
    if ((mappingList == NULL) || (mappingList->GetCount() < 1))
        return;

    Sp<String> dirPath = String::Create(gRootOutputDir->Length() + 24, L"%ls/", (PCWStr)*gRootOutputDir);
    if (dirPath == NULL)
        return;

    GenerateDirPath(dirPath);
    Sp<String> fullFileName = String::Create(dirPath->Length() + 24, L"%ls/MappingTable.cs", (PCWStr)*dirPath);
    if (fullFileName == NULL)
        return;

    FILE * file = OpenFile(fullFileName);
    if (file == NULL)
        return;

    FWRITE("using System;\r\nusing System.Collections;\r\nusing System.Collections.Generic;\r\nusing AXP;\r\n\r\n");
    FWRITE("namespace Models");
    FWRITE("{\r\n");
    FWRITE("class MappingTable\r\n");
    FWRITE("{\r\n");
    FWRITE("public static Boolean InsertMappingTable()\r\n");
    FWRITE("{\r\n");

    Foreach(CMappingInfo, obj, mappingList) {
        WRITE_STRING_TO_FILE(obj->mDescription->Length() + obj->mScope->Length() + 87,
            L"if (!AXP::Libc::Common::ClassLoader.RegisterClassCreator(\"%ls\", %ls.Create))\r\n", (PCWStr)*obj->mDescription, (PCWStr)*obj->mScope);
        FWRITE("return false;\r\n\r\n");
    }

    FWRITE("return true;\r\n");
    FWRITE("}\r\n");
    FWRITE("}\r\n");
    FWRITE("}\r\n");

    fclose(file);
    FormatFile(fullFileName, IDL_LANG_CS);
}

STATIC Void WriteObjcMapping()
{

}

STATIC Void WriteJavascriptClassLoader()
{
    if (gImportFileEs6 == NULL)
        return;

    Sp<String> dirPath = String::Create(gRootOutputDir->Length() + 77, L"%ls/", (PCWStr)*gRootOutputDir);
    if (dirPath == NULL)
        return;

    GenerateDirPath(dirPath);
    Sp<String> fullFileName = String::Create(dirPath->Length() + 77, L"%ls/classloader.js", (PCWStr)*dirPath);
    if (fullFileName == NULL)
        return;

    FILE * file = OpenFile(fullFileName);
    if (file == NULL)
        return;

    FWRITE("\"use strict\";\r\n\r\n");
    Foreach(CMappingInfo, obj, gImportFileEs6) {
        if (obj == NULL)
            return;
        
        WRITE_STRING_TO_FILE(obj->mDescription->Length() + obj->mScope->Length() + 77,
        L"import * as %ls from %ls;\r\n", (PCWStr)*obj->mDescription, (PCWStr)*obj->mScope);
    }
    
    FWRITE("\r\nsServiceTable = {};\r\nsMaxIndex = 0;\r\n\r\n");
    FWRITE("export class ClassLoader {\r\nstatic createObject(className)\r\n{\r\n"
    "return eval(\"new \" + className + \"();\");\r\n}\r\n\r\n"
    "static getService(remoteRefCode) {\r\nreturn sServiceTable[remoteRefCode];\r\n"
    "}\r\n\r\nstatic registerService(serviceRef) {\r\nsServiceTable[++sMaxIndex] = serviceRef;\r\n"
    "return sMaxIndex;\r\n}\r\n\r\nstatic removeService(remoteRefCode) {\r\ndelete sServiceTable[remoteRefCode];\r\n"
    "}\r\n}\r\n");
    
    fclose(file);
    FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
}

STATIC Void WriteJavascriptStubClassRegister()
{
    Sp<List<CMappingInfo> > mappingList = gMappingJavascript->GetValues();
    if (mappingList == NULL)
        return;

    Sp<String> dirPath = String::Create(gRootOutputDir->Length() + 77, L"%ls/", (PCWStr)*gRootOutputDir);
    if (dirPath == NULL)
        return;

    GenerateDirPath(dirPath);
    Sp<String> fullFileName = String::Create(dirPath->Length() + 77, L"%ls/RegisterServiceStub.js", (PCWStr)*dirPath);
    if (fullFileName == NULL)
        return;

    FILE * file = OpenFile(fullFileName);
    if (file == NULL)
        return;

    FWRITE("define([");
    FWRITE("\"IPC/javascript/ServiceManager\"");
    Foreach(String, obj, sImportStubFilesJavascript) {
        WRITE_STRING_TO_FILE(obj);
    }

    FWRITE("], function () {\r\n");
    FWRITE("$.declareClass(\"IPC.StubTable\", {\r\nstatic: {\r\n");
    Foreach(CMappingInfo, obj, mappingList) {
        WRITE_STRING_TO_FILE((obj->mDescription->Length() + obj->mScope->Length()) * 3 + 87,
            L"%ls_%ls: IPC.ServiceManager.registerService(\"%ls.%ls\", new %ls.%lsStub()),\r\n",
            (PCWStr)*obj->mDescription, (PCWStr)*obj->mScope, (PCWStr)*obj->mDescription, (PCWStr)*obj->mScope,
            (PCWStr)*obj->mDescription, (PCWStr)*obj->mScope);
    }

    FWRITE("},\r\n});\r\n});\r\n");

    fclose(file);
    FormatFile(fullFileName, IDL_LANG_JAVASCRIPT);
}

STATIC Void WriteClassMappingTable()
{
    if (gIDLFlag & IDL_LANG_JAVASCRIPT)
        WriteJavascriptStubClassRegister();
    
    if (gIDLFlag & IDL_LANG_ES6)
        WriteJavascriptClassLoader();    
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        Usage(argv[0]);
        return -1;
    }

    if (IDLC::Initialize() < 0)
        return -1;

    int ch;
    opterr = 0;
    while ((ch = getopt(argc, argv, "j:c:m:i:o:d:e:hgp")) != -1) {
        switch (ch) {
        case 'j':
            if (strcmp(optarg, "ava") == 0) {
                gIDLFlag |= IDL_LANG_JAVA;
            }
            else if (strcmp(optarg, "avascript") == 0) {
                gIDLFlag |= IDL_LANG_JAVASCRIPT;
            }
            else {
                fprintf(stderr, "-j%s: Invalid argument\n", optarg);
                exit(-1);
            }
            break;
            
        case 'e':
            if (strcmp(optarg, "s6") == 0) {
                gIDLFlag |= IDL_LANG_ES6;
            }
            else {
                fprintf(stderr, "-e%s: Invalid argument\n", optarg);
                exit(-1);
            }
            break;

        case 'o':
            if (strcmp(optarg, "bjc") == 0) {
                gIDLFlag |= IDL_LANG_OBJC;
            }
            else if (strncmp(optarg, "utput=", 6) == 0) {
                gRootOutputDir = String::Create(optarg + 6);
                if (gRootOutputDir == NULL)
                    return -1;

                break;
            }
            else {
                fprintf(stderr, "-o%s: Invalid argument\n", optarg);
                exit(-1);
            }
            break;

        case 'i':
            if (strncmp(optarg, "input=", 6) == 0) {
                gInputDir = (char*)calloc(1, strlen(optarg + 6) + 1);
                if (gInputDir == NULL)
                    return -1;

                memcpy(gInputDir, optarg + 6, strlen(optarg + 6));
                break;
            }
            else {
                fprintf(stderr, "-o%s: Invalid argument\n", optarg);
                exit(-1);
            }
            break;

        case 'c':
            if (strcmp(optarg, "sharp") == 0) {
                gIDLFlag |= IDL_LANG_CS;
            }
            else if (strcmp(optarg, "pp") == 0) {
                gIDLFlag |= IDL_LANG_CPP;
            }
            else {
                fprintf(stderr, "-c%s: Invalid argument\n", optarg);
                exit(-1);
            }
            break;

        case 'p':
            gIDLFlag |= IDL_PRINT_TREE;
            break;

        case 'g':
            gIDLFlag |= IDL_GENERATE_INCLUDE_FILE;
            break;

        case 'h':
            Usage(argv[0]);
            return -1;

        default:
            Usage(argv[0]);
            return -1;
        }
    }

    if (argc - optind != 1)
        return -1;

    if (gRootOutputDir == NULL) {
        gRootOutputDir = String::Create(L"./gen/");
        if (gRootOutputDir == NULL)
            return -1;
    }

    char * str = (char*)malloc(strlen(argv[argc - 1]) + 1);
    if (!str)
        return -1;

    memcpy(str, argv[argc - 1], strlen(argv[argc - 1]));
    str[strlen(argv[argc - 1])] = '\0';
    if ((!strcmp(str, ".")) || (!strcmp(str, "..")))
        ProcessDir(str, 1);
    else {
        if (access(str, 0)) {
            printf("file doesn't exist!\n");
            return -1;
        }

        struct stat info;
        stat(str, &info);
        if (S_ISDIR(info.st_mode))
            ProcessDir(str, 1);
        else {
            Sp<String> fileName = String::Create(str);
            if (fileName == NULL)
                return -1;

            Int32 index1 = fileName->LastIndexOf(L'/');
            Int32 index2 = fileName->LastIndexOfString(L".idl");
            if (index2 == -1)
                return -1;

            sFileName = fileName->SubString(index1 + 1, index2 - index1 - 1);
            if (sFileName == NULL)
                return -1;

            ProcessFile(str);
            if (gNamespacz) {
                if (gIDLFlag & IDL_LANG_CPP) {
                    Sp<String> includeFileStr = String::Create(gNamespacz->Length() + sFileName->Length() + 38,
                        L"#include \"%ls/%ls.h\"\r\n", (PCWStr)*gNamespacz, (PCWStr)*sFileName);
                    if (includeFileStr == NULL)
                        return -1;

                    if (!sIncludeFileListCpp->PushBack(includeFileStr))
                        return -1;
                }
                if (gIDLFlag & IDL_LANG_JAVASCRIPT) {
                    Sp<String> importFileStr = String::Create(gNamespacz->Length() + sFileName->Length() + 78,
                        L", \"IPC/gen/javascript/stub/%ls/%lsStub\"", (PCWStr)*gNamespacz, (PCWStr)*sFileName);
                    if (importFileStr == NULL)
                        return -1;

                    if (!sImportStubFilesJavascript->PushBack(importFileStr))
                        return -1;
                }
            }
        }
    }

    WriteClassMappingTable();
    free(str);

    return 0;
}