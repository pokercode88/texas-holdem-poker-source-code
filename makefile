
#-----------------------------------------------------------------------

APP           := XGame
TARGET        := DBAgentServer
CONFIG        := 
STRIP_FLAG    := N
TARS2CPP_FLAG := 
CFLAGS        += -lm
CXXFLAGS      += -lm

INCLUDE   += -ICacheOperate
INCLUDE   += -IDBOperate

INCLUDE   += -I/usr/local/cpp_modules/wbl/include
LIB       += -L/usr/local/cpp_modules/wbl/lib -lwbl

INCLUDE   += -I/usr/local/mysql/include
LIB       += -L/usr/local/mysql/lib/mysql -lmysqlclient

INCLUDE   += -I/usr/local/cpp_modules/protobuf/include
LIB       += -L/usr/local/cpp_modules/protobuf/lib -lprotobuf

INCLUDE   += -I/usr/local/cpp_modules/hiredis/include
LIB       += -L/usr/local/cpp_modules/hiredis/lib -lhiredis

LOCAL_SRC += CacheOperate/HiredisSync.cpp CacheOperate/CacheInterface.cpp \
	CacheOperate/CacheInterfaceManager.cpp CacheOperate/HiredisAsync.cpp \
	CacheOperate/CacheOperator.cpp

#-----------------------------------------------------------------------
include /home/tarsproto/XGame/Comm/Comm.mk
include /usr/local/tars/cpp/makefile/makefile.tars

#-----------------------------------------------------------------------


