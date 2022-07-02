
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing header files
#
#---------------------------------------------------------------------------------
TARGET		:=	FAKE08
BUILD		:=	build
SOURCES		:=	${SOURCES} source
INCLUDES	:=	${INCLUDES}

#which platformm to use its host file
IMITATE		:= miyoomini
#IMITATE		:= bittboy

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CC = $(CXX)

CFLAGS	:=	-g -Wall -ffunction-sections -DVER_STR=\"$(APP_VERSION)\" \
			$(DEFINES)

CFLAGS	+=	$(INCLUDE)

CXXFLAGS	:= $(CFLAGS) -fno-rtti -std=gnu++17

#to profile, link to gperftools profiler
#LIBS	:= -Wl,--no-as-needed -lprofiler -Wl,--as-needed -lSDL 
#alternatively, use operf
#sudo operf ./platform/SDL1_2/FAKE08 ~/p8carts/png-x-zero.p8.png 
#opreport --demangle=smart --symbols > fake08-x-zero-master-preoptim2.txt
LIBS	:= -lSDL 

LDFLAGS	:= $(LIBS)

ifeq ($(IMITATE),miyoomini)

SOURCES += ../miyoomini/source
CXXFLAGS += -D_DESKTOP=1

else

SOURCES += ../bittboy/source

endif


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

ifeq ($(IMITATE),miyoomini)
#ao.c is only for building on the actual mini
CFILES := $(filter-out ao.c,$(CFILES))
endif

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
