pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
-- shrinko8 pattern: pass _ENV as a param so x/y come from a vec2
function localfillp(p, _ENV)
 local p16, x = flr(p), x&3
 local f, p32 = 15\2^x*0x1111,p16+(p16>>>16)>><(y&3)*4+x
 return p - p16 + flr((p32&f) + band(p32<<>4, 0xffff - f))
end

function _init()
 local pos = vec2(10, 20)
 printh("fillp:"..tostring(localfillp(49110.25, pos)))
end

function vec2(x,y)
 return {x=x,y=y}
end
__gfx__
