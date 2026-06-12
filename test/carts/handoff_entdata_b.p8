pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
function usplit(...)return unpack(split(...))end
function at(e,n,t,d)n=n or{}for t,e in inext,split(e,t)do local t,e=usplit(e,d or":")n[t]=e=="{}"and{}or e end return n end
function _init()
 entdata=at(chr(peek(32770,%32768)),nil,"\n","=")
 printh("316:"..tostring(entdata[316]))
 foreach(split("316"),function(e)
  entdata[172]..=entdata[e]
 end)
 printh("ok")
end
__gfx__
