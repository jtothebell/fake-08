
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing header files
#
#---------------------------------------------------------------------------------
TARGET		:=	FAKE08
BUILD		:=	build
SOURCES		:=	${SOURCES} ../SDL2Common/source source
INCLUDES	:=	${INCLUDES}

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CC = $(CXX)

CFLAGS	:=	-g -Wall -Wno-deprecated -ffunction-sections -std=c++17 \
			-MMD -MP \
			$(DEFINES)

CFLAGS	+=	$(INCLUDE) -DVER_STR=\"$(APP_VERSION)\"

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions 
#-std=gnu++11 was used before... not sure of difference

# SDL2 flags via pkg-config or sdl2-config with sensible fallbacks (Homebrew paths)
SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL2_LIBS   := $(shell pkg-config --libs sdl2 2>/dev/null)

ifeq ($(strip $(SDL2_CFLAGS)),)
SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL2_LIBS   := $(shell sdl2-config --libs 2>/dev/null)
endif

ifeq ($(strip $(SDL2_CFLAGS)),)
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_S),Darwin)
ifeq ($(UNAME_M),arm64)
# Add parent include dir so #include <SDL2/SDL.h> resolves
SDL2_CFLAGS := -I/opt/homebrew/include
SDL2_LIBS   := -L/opt/homebrew/lib -lSDL2
else
SDL2_CFLAGS := -I/usr/local/include
SDL2_LIBS   := -L/usr/local/lib -lSDL2
endif
endif
endif

# Add include flags for SDL2 to both C and C++ compiles
CFLAGS	+= $(SDL2_CFLAGS)
CXXFLAGS += $(SDL2_CFLAGS)

# Keep legacy LIBS but prefer SDL2_LIBS for correctness
LIBS	?= -lSDL2

LDFLAGS	:= $(LIBS) $(SDL2_LIBS)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 		:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)


.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET)


#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all	:	$(OUTPUT)

$(OUTPUT)		:	$(OFILES)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OFILES_SRC)	: $(HFILES_BIN)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
