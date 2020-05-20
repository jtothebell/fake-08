
ROOT_SOURCES	?= source
ROOT_INCLUDES	?= include

#paths are relative to platform folder
export SOURCES   = ../../source ../../libs/lua-5.3.2/src ../../libs/utf8-util
export INCLUDES  = ../../include ../../libs/lua-5.3.2/src ../../libs/utf8-util

.PHONY: all 3ds switch clean clean-3ds clean-switch

all: 3ds switch

clean: clean-3ds clean-switch

clean-3ds:
	@$(MAKE) -C platform/3ds clean

clean-switch:
	@$(MAKE) -C platform/switch clean

3ds:
	@$(MAKE) -C platform/3ds

switch:
	@$(MAKE) -C platform/switch