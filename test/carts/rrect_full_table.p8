pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
function _init()
 for r=1,64 do
  local w=max(80,2*r+2)
  cls()
  rrectfill(0,0,w,w,r,7)
  local cuts=""
  for row=0,r-1 do
   local cut=-1
   for x=0,w-1 do
    if pget(x,row)==7 then cut=x break end
   end
   cuts=cuts..cut..","
  end
  printh("r="..r.." "..cuts)
 end
end
__gfx__
