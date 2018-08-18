OPENNI_BUILD_PREFIX = /usr/local/OpenNI/Platform/Linux/Build
#CPPFLAGS=-D_BSD_SOURCE -D_XOPEN_SOURCE=600 -Wall -Wno-write-strings
CFLAGS=-D_BSD_SOURCE -D_XOPEN_SOURCE=600 -Wno-write-strings -O2

include ${OPENNI_BUILD_PREFIX}/Common/CommonDefs.mak

BIN_DIR = ${OPENNI_BUILD_PREFIX}/../Bin

INC_DIRS = ${OPENNI_BUILD_PREFIX}/../../../Include

SRC_FILES = *.cpp

USED_LIBS = OpenNI png

include ${OPENNI_BUILD_PREFIX}/Common/CommonCppMakefile

