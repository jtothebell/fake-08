STATIC_LINKING := 0
AR             := ar

ifneq ($(V),1)
   Q := @
endif

ifneq ($(SANITIZER),)
   CFLAGS   := -fsanitize=$(SANITIZER) $(CFLAGS)
   CXXFLAGS := -fsanitize=$(SANITIZER) $(CXXFLAGS)
   LDFLAGS  := -fsanitize=$(SANITIZER) $(LDFLAGS)
endif

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
	EXE_EXT = .exe
	system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
	system_platform = osx
	arch = intel
ifeq ($(shell uname -p),powerpc)
	arch = ppc
endif
ifeq ($(shell uname -p),arm)
	arch = arm
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
	system_platform = win
endif

CORE_DIR    := ../..
TARGET_NAME := fake08
LIBM		    = -lm

ifeq ($(ARCHFLAGS),)
ifeq ($(arch),ppc)
   ARCHFLAGS = -arch ppc -arch ppc64
else ifeq ($(arch),arm)
   ARCHFLAGS = -arch arm64
else
   ARCHFLAGS = -arch i386 -arch x86_64
endif
endif

ifeq ($(platform), osx)
ifndef ($(NOUNIVERSAL))
   CFLAGS += $(ARCHFLAGS)
   CXXFLAGS += $(ARCHFLAGS)
   LFLAGS += $(ARCHFLAGS)
endif
endif

ifeq ($(STATIC_LINKING), 1)
EXT := a
endif

ifeq ($(platform), unix)
	EXT ?= so
   TARGET := $(TARGET_NAME)_libretro.$(EXT)
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
else ifeq ($(platform), linux-portable)
   TARGET := $(TARGET_NAME)_libretro.$(EXT)
   fpic := -fPIC -nostdlib
   SHARED := -shared -Wl,--version-script=link.T
	LIBM :=
else ifneq (,$(findstring osx,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifneq (,$(findstring ios,$(platform)))
   TARGET := $(TARGET_NAME)_libretro_ios.dylib
	fpic := -fPIC
	SHARED := -dynamiclib

ifeq ($(IOSSDK),)
   IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
endif

	DEFINES := -DIOS
	CC = cc -arch armv7 -isysroot $(IOSSDK)
ifeq ($(platform),ios9)
CC     += -miphoneos-version-min=8.0
CXXFLAGS += -miphoneos-version-min=8.0
else
CC     += -miphoneos-version-min=5.0
CXXFLAGS += -miphoneos-version-min=5.0
endif
else ifneq (,$(findstring qnx,$(platform)))
	TARGET := $(TARGET_NAME)_libretro_qnx.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
else ifeq ($(platform), emscripten)
   TARGET := $(TARGET_NAME)_libretro_emscripten.bc
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
else ifeq ($(platform), vita)
   TARGET := $(TARGET_NAME)_vita.a
   CC = arm-vita-eabi-gcc
   AR = arm-vita-eabi-ar
   CXXFLAGS += -Wl,-q -Wall -O3
	STATIC_LINKING = 1
else ifeq ($(platform), miyoomini)
   TARGET := $(TARGET_NAME)_libretro_miyoomini.so
   CC = arm-linux-gnueabihf-gcc
   CXX = arm-linux-gnueabihf-g++
   AR = arm-linux-gnueabihf-ar
   STRIP = arm-linux-gnueabihf-strip
   CXXFLAGS += -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve -D_NEED_FULL_PATH_
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T

else ifeq ($(platform), gcw0)
   TARGET := $(TARGET_NAME)_libretro_gcw0.so
   CC = /opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc
   CXX = /opt/gcw0-toolchain/usr/bin/mipsel-linux-g++
   AR = /opt/gcw0-toolchain/usr/bin/mipsel-linux-ar
   CXXFLAGS += -fomit-frame-pointer -ffast-math -march=mips32 -mtune=mips32r2 -mhard-float
   CXXFLAGS += -fno-common -ftree-vectorize -funswitch-loops
   fpic := -fPIC
   SHARED := -shared -Wl,-version-script=link.T
else ifeq ($(platform), libnx)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   include $(DEVKITPRO)/libnx/switch_rules
   STATIC_LINKING = 1
   DEFINES := -D__SWITCH__ -DSWITCH=1
   CFLAGS	:=	 $(DEFINES) -g -O3 \
                 -fPIE -I$(LIBNX)/include/ -ffunction-sections -fdata-sections -ftls-model=local-exec -Wl,--allow-multiple-definition -specs=$(LIBNX)/switch.specs
   CFLAGS += $(INCDIRS)
   CFLAGS	+=	-DHAVE_LIBNX -march=armv8-a -mtune=cortex-a57 -mtp=soft
   CXXFLAGS := $(ASFLAGS) $(CFLAGS)
else
   CC = gcc
   TARGET := $(TARGET_NAME)_libretro.dll
   SHARED := -shared -static-libgcc -static-libstdc++ -Wl,--version-script=link.T -Wl,--no-undefined
ifneq ($(DEBUG), 1)
   SHARED += -s
endif
endif

LDFLAGS += $(LIBM)

ifeq ($(DEBUG), 1)
   CFLAGS += -O0 -g -DDEBUG
   CXXFLAGS += -O0 -g -DDEBUG
else
   CFLAGS += -O2
   CXXFLAGS += -O2
endif

include Makefile.common

CC = $(CXX)

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

CFLAGS   += $(fpic) -Wall -D__LIBRETRO__ -DMINIZ_NO_TIME -ffunction-sections -std=gnu++17 -fno-rtti $(INCFLAGS)
CXXFLAGS += $(fpic) -Wall -D__LIBRETRO__ -DMINIZ_NO_TIME -ffunction-sections -std=gnu++17 -fno-rtti $(INCFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	@$(if $(Q), $(shell echo echo LD $@),)
	$(Q)$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) $(LDFLAGS)
endif

%.o: %.c
	@$(if $(Q), $(shell echo echo CC $<),)
	$(Q)$(CXX) $(CFLAGS) $(fpic) -c -o $@ $<

%.o: %.cpp
	@$(if $(Q), $(shell echo echo CXX $<),)
	$(Q)$(CXX) $(CFLAGS) $(fpic) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

echo:
	@echo echo ...
#	@echo $(SOURCES_C) 
#	@echo $(SOURCES_CXX)
	@echo $(platform)
	@echo $(OBJECTS) 

.PHONY: clean echo

print-%:
	@echo '$*=$($*)'
