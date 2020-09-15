
ROOT_SOURCES	?= source
ROOT_INCLUDES	?= include

#paths are relative to platform folder
export SOURCES   = ../../source ../../libs/lua-5.3.2/src ../../libs/utf8-util ../../libs/lodepng
export INCLUDES  = ../../include ../../libs/lua-5.3.2/src ../../libs/utf8-util ../../libs/lodepng

.PHONY: all 3ds switch clean clean-3ds clean-switch

all: 3ds switch

clean: clean-tests clean-3ds clean-switch

clean-3ds:
	@$(MAKE) -C platform/3ds clean

clean-switch:
	@$(MAKE) -C platform/switch clean

3ds:
	@$(MAKE) -C platform/3ds

cia:
	@$(MAKE) cia -C platform/3ds

switch:
	@$(MAKE) -C platform/switch

clean-tests:
	@$(MAKE) -C test clean

tests:
	@$(MAKE) -C test