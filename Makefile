
ROOT_SOURCES	?= source
ROOT_INCLUDES	?= include

export APP_TITLE	= FAKE-08
export APP_AUTHOR	= jtothebell
export V_MAJOR	= 0
export V_MINOR	= 0
export V_PATCH	= 2
export V_BUILD	= 13
export APP_VERSION	= v$(V_MAJOR).$(V_MINOR).$(V_PATCH).$(V_BUILD)


#paths are relative to platform folder
export SOURCES   = ../../source ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng ../../libs/simpleini
export INCLUDES  = ../../include ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng ../../libs/simpleini

.PHONY: all 3ds switch wiiu vita sdl2 sdl clean clean-3ds clean-switch clean-wiiu clean-vita clean-sdl2 clean-sdl

all: 3ds switch wiiu vita bittboy

clean: clean-tests clean-3ds clean-switch clean-wiiu clean-vita clean-sdl2 clean-sdl clean-bittboy

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

bittboy:
	@$(MAKE) -C platform/bittboy

clean-tests:
	@$(MAKE) -C test clean

tests:
	@$(MAKE) -C test