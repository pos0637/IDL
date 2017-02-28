all: IDLC

CFLAGS= \
	-DPLATFORM_LINUX \
	-DCOMPILER_GCC \
	-DARCH_X86 \
	-DARCH_LITTLE_ENDIAN

SRC= \
	AXP/cplusplus/xplatform/src/astring.cpp \
	AXP/cplusplus/xplatform/src/atime.cpp \
	AXP/cplusplus/xplatform/src/atom.cpp \
	AXP/cplusplus/xplatform/src/event.cpp \
	AXP/cplusplus/xplatform/src/mutex.cpp \
	AXP/cplusplus/xplatform/src/thread.cpp \
	AXP/cplusplus/xplatform/src/Parcel.cpp \
	AXP/cplusplus/libc/src/Common/BaseWorker.cpp \
	AXP/cplusplus/libc/src/Common/Tracker.cpp \
	AXP/cplusplus/libc/src/Network/HttpServer.cpp \
	AXP/cplusplus/ThirdParty/IConv/Libiconv/iconv.c \
	AXP/cplusplus/ThirdParty/IConv/Libiconv/localcharset.c \
	Common.cpp \
	IDLCWriteSourceFile_cpp.cpp \
	IDLCWriteSourceFile_cs.cpp \
	IDLCWriteSourceFile_objc.cpp \
	IDLCWriteSourceFile_java.cpp \
	IDLCWriteSourceFile_javascript.cpp \
    IDLCWriteSourceFile_es6.cpp \
	IDLCParser.cpp \
	SymbolRef.cpp \
	IDL.tab.cpp \
	IDL.cpp \
	GenerateIDL.cpp

IDLC: IDL.l IDL.y
	bison --defines=IDL.tab.hpp --output=IDL.tab.cpp IDL.y
	flex --outfile=IDL.cpp IDL.l
	g++ -g $(CFLAGS) -o GenerateIDL $(SRC) -lfl

clean:
	rm IDL.tab.hpp IDL.tab.cpp IDL.cpp GenerateIDL.exe
