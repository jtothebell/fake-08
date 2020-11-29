
ROOT_SOURCES	?= source
ROOT_INCLUDES	?= include

#paths are relative to platform folder
export SOURCES   = ../../source ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng
export INCLUDES  = ../../include ../../libs/z8lua ../../libs/utf8-util ../../libs/lodepng

.PHONY: all 3ds switch sdl2 clean clean-3ds clean-switch clean-sdl2

all: 3ds switch sdl2

clean: clean-tests clean-3ds clean-switch clean-sdl2

clean-3ds:
	@$(MAKE) -C platform/3ds clean

clean-switch:
	@$(MAKE) -C platform/switch clean

clean-sdl2:
	@$(MAKE) -C platform/SDL2 clean

3ds:
	@$(MAKE) -C platform/3ds

cia:
	@$(MAKE) cia -C platform/3ds

switch:
	@$(MAKE) -C platform/switch

sdl2:
	@$(MAKE) -C platform/SDL2

clean-tests:
	@$(MAKE) -C test clean

tests:
	@$(MAKE) -C test