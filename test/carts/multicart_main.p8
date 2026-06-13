pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
if peek4(0x4310) ~= 0x1234 then
 load("multicart_extra.p8", "back")
end
function _init()
 printh("checksum:"..peek4(0x4310))
end
__gfx__
