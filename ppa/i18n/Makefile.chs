TARGET = i18n.chs
OBJS = main.o i18n.o

BUILD_PRX=1
PRX_EXPORTS=exports.exp

USE_PSPSDK_LIBC=1

INCDIR = 
CFLAGS = -O2 -G0 -Wall -DCHS
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LIBS = 
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
