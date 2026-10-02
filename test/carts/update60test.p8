pico-8 cartridge // http://www.pico-8.com
version 29
__lua__
-- a 60fps cart; a cart restored after it must not run this
function _update60() poke(0x4301,1) end
