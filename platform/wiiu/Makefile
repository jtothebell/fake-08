#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/wut/share/wut_rules

#-------------------------------------------------------------------------------
# pkg-config for libraries flags
#-------------------------------------------------------------------------------
PKGCONF		:=	$(PORTLIBS_PATH)/wiiu/bin/powerpc-eabi-pkg-config

#-------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing header files
#-------------------------------------------------------------------------------
TARGET		:=	FAKE-08
BUILD		:=	build
SOURCES		:=	${SOURCES} source
INCLUDES	:=	${INCLUDES}
ROMFS		:=	romfs
LIBRARIES	:=	SDL2_gfx SDL2_image SDL2_mixer SDL2_ttf sdl2 \
			freetype2 zlib libpng libjpeg

#-------------------------------------------------------------------------------
# options for code generation
#-------------------------------------------------------------------------------
CC = $(CXX)

CFLAGS		:=	-g -Wall -O2 -ffunction-sections -DVER_STR=\"$(APP_VERSION)\" \
			-ffast-math \
			$(MACHDEP)

CFLAGS		+=	$(INCLUDE) -D__WIIU__ -D__WUT__

CFLAGS		+=	`$(PKGCONF) --cflags $(LIBRARIES)`

CXXFLAGS	:=	$(CFLAGS) -std=gnu++17

ASFLAGS		:=	-g $(MACHDEP)
LDFLAGS		:=	-g $(MACHDEP) $(RPXSPECS) -Wl,-Map,$(notdir $*.map)

LIBS		:=	`$(PKGCONF) --libs $(LIBRARIES)`
LIBS		+= -lwut

#-------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#-------------------------------------------------------------------------------
LIBDIRS		:=	$(PORTLIBS) $(WUT_ROOT)


#-------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

#-------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#-------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#-------------------------------------------------------------------------------
	export LD	:=	$(CC)
#-------------------------------------------------------------------------------
else
#-------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

export SRCFILES		:=	$(CPPFILES) $(CFILES) $(SFILES)
export OFILES		:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE		:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
				$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
				-I$(CURDIR)/$(BUILD)

export LIBPATHS		:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#-------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD): $(SRCFILES)
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#-------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).rpx $(TARGET).elf

#-------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS		:=	$(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all		:	$(OUTPUT).rpx

$(OUTPUT).rpx	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)

-include $(DEPENDS)

#-------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------- 
