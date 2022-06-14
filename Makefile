
ROOT_SOURCES	?= source
ROOT_INCLUDES	?= include

export APP_TITLE	= FAKE-08
export APP_AUTHOR	= jtothebell
export V_MAJOR	= 0
export V_MINOR	= 0
export V_PATCH	= 2
export V_BUILD	= 17
export APP_VERSION	= v$(V_MAJOR).$(V_MINOR).$(V_PATCH).$(V_BUILD)


#paths are relative to platform folder
export SOURCES   = ../../source ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng ../../libs/simpleini ../../libs/miniz 
export INCLUDES  = ../../include ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng ../../libs/simpleini ../../libs/miniz

.PHONY: all 3ds switch wiiu vita sdl2 sdl windows clean clean-3ds clean-switch clean-wiiu clean-vita clean-sdl2 clean-sdl clean-windows

all: 3ds switch wiiu vita bittboy windows

clean: clean-tests clean-3ds clean-switch clean-wiiu clean-vita clean-sdl2 clean-sdl clean-bittboy clean-windows

clean-3ds:
	@$(MAKE) -C platform/3ds clean

clean-switch:
	@$(MAKE) -C platform/switch clean

clean-wiiu:
	@$(MAKE) -C platform/wiiu clean

clean-vita:
	@$(MAKE) -C platform/vita clean

clean-sdl2:
	@$(MAKE) -C platform/SDL2Desktop clean

clean-sdl:
	@$(MAKE) -C platform/SDL1_2 clean

clean-bittboy:
	@$(MAKE) -C platform/bittboy clean
	
clean-funkey:
	@$(MAKE) -C platform/funkey-s clean
	
clean-gcw0:
	@$(MAKE) -C platform/gcw0 clean
	
clean-gkd:
	@$(MAKE) -C platform/gcw0 GKD=1 clean

clean-miyoomini:
	@$(MAKE) -C platform/miyoomini clean

clean-windows:
	@$(MAKE) -C platform/windows clean

3ds:
	@$(MAKE) -C platform/3ds

cia:
	@$(MAKE) cia -C platform/3ds

switch:
	@$(MAKE) -C platform/switch

wiiu:
	@$(MAKE) -C platform/wiiu

vita:
	@$(MAKE) -C platform/vita

sdl2:
	@$(MAKE) -C platform/SDL2Desktop

sdl:
	@$(MAKE) -C platform/SDL1_2

funkey:
	@$(MAKE) -C platform/funkey-s

bittboy:
	@$(MAKE) -C platform/bittboy
	
gcw0:
	@$(MAKE) -C platform/gcw0
	
gkd:
	@$(MAKE) -C platform/gcw0 GKD=1

miyoomini:
	@$(MAKE) -C platform/miyoomini

windows:
	@$(MAKE) -C platform/windows

clean-tests:
	@$(MAKE) -C test clean

tests:
	@$(MAKE) -C test
	cd test && ./testrunner.a
