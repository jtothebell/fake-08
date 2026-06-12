pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
function _init()
 local entstr = ""
 for i=1,750 do
  entstr = entstr.."172=n:base\n"
 end
 entstr = entstr.."316=n:variant\n"
 poke2(0x8000,#entstr)
 ?"\^!8002"..entstr
end
__gfx__
