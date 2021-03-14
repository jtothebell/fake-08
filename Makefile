
ROOT_SOURCES	?= source
ROOT_INCLUDES	?= include

export APP_TITLE	= FAKE-08
export APP_AUTHOR	= jtothebell
export APP_VERSION	= v0.0.2.5

#paths are relative to platform folder
export SOURCES   = ../../source ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng ../../libs/simpleini
export INCLUDES  = ../../include ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng ../../libs/simpleini

.PHONY: all 3ds switch wiiu vita sdl2 clean clean-3ds clean-switch clean-wiiu clean-vita clean-sdl2

all: 3ds switch wiiu sdl2

clean: clean-tests clean-3ds clean-switch clean-wiiu clean-vita clean-sdl2

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

clean-tests:
	@$(MAKE) -C test clean

tests:
	@$(MAKE) -C test