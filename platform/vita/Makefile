PROJECT_TITLE := FAKE-08
PROJECT_TITLEID := FAKE00008

PROJECT := fake_08

PHONY := all package clean

SOURCES	:=	${SOURCES} ../SDL2Common/source source
INCLUDES	:=	${INCLUDES}

LIBS = -lSDL2 -lScePower_stub  -lSceDisplay_stub -lSceCtrl_stub -lSceAudio_stub \
		-lSceSysmodule_stub -lSceGxm_stub -lSceCommonDialog_stub \
		-lSceAppMgr_stub -lSceTouch_stub -lSceHid_stub -lSceMotion_stub -lm

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
OBJS     := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

export INCLUDE		:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
				$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
				-I$(CURDIR)/$(BUILD)

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
STRIP := $(PREFIX)-strip
CFLAGS  = -g -Wl,-q -O2 -ftree-vectorize -D__VITA__ -DVER_STR=\"$(APP_VERSION)\" $(INCLUDE)
CXXFLAGS  = $(CFLAGS) -fexceptions -std=gnu++17 -fpermissive
ASFLAGS = $(CFLAGS)

CC = $(CXX)



all: package

package: $(PROJECT).vpk

$(PROJECT).vpk: eboot.bin param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin \
		--add sce_sys/icon0.png=sce_sys/icon0.png \
		--add sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		--add sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		--add sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
	$(PROJECT).vpk

eboot.bin: $(PROJECT).velf
	vita-make-fself $(PROJECT).velf eboot.bin

param.sfo:
	vita-mksfoex -s TITLE_ID="$(PROJECT_TITLEID)" "$(PROJECT_TITLE)" param.sfo

$(PROJECT).velf: $(PROJECT).elf
	$(STRIP) -g $<
	vita-elf-create $< $@

$(PROJECT).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@




clean:
	rm -f $(PROJECT).velf $(PROJECT).elf $(PROJECT).vpk param.sfo eboot.bin $(OBJS)

vpksend: $(PROJECT).vpk
	curl -T $(PROJECT).vpk ftp://$(PSVITAIP):1337/ux0:/
	@echo "Sent."

send: eboot.bin
	curl -T eboot.bin ftp://$(PSVITAIP):1337/ux0:/app/$(TITLE_ID)/
	@echo "Sent."
