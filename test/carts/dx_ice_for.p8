pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
function _init()
 k=false
 for n=1,2 do
  if(k=="ice")if(d>=16and d<20)d=59
  mset(n-1,0,18)
 end
 printh("mget00="..mget(0,0).." mget10="..mget(1,0))
end
__gfx__
