pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
poke4(0x4310, 0x1234)
extcmd"breadcrumb"
print("should not appear")
__gfx__
