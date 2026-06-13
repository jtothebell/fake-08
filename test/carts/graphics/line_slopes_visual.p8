pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
-- visual layout of line_slopes.p8 for screenshot comparison
local tests={
 {x0=2,y0=10,x1=22,y1=10},
 {x0=22,y0=10,x1=2,y1=10},
 {x0=10,y0=2,x1=10,y1=22},
 {x0=10,y0=22,x1=10,y1=2},
 {x0=2,y0=2,x1=22,y1=22},
 {x0=22,y0=2,x1=2,y1=22},
 {x0=2,y0=22,x1=22,y1=2},
 {x0=22,y0=22,x1=2,y1=2},
 {x0=2,y0=10,x1=32,y1=12},
 {x0=32,y0=10,x1=2,y1=12},
 {x0=10,y0=2,x1=12,y1=32},
 {x0=10,y0=32,x1=12,y1=2},
 {x0=2,y0=2,x1=22,y1=12},
 {x0=22,y0=12,x1=2,y1=2},
 {x0=2,y0=2,x1=12,y1=22},
 {x0=12,y0=22,x1=2,y1=2},
 {x0=2,y0=2,x1=20,y1=8},
 {x0=2,y0=2,x1=8,y1=20},
 {x0=2,y0=10,x1=32,y1=16},
 {x0=10,y0=2,x1=16,y1=32},
 {x0=2,y0=2,x1=23,y1=11},
 {x0=2,y0=2,x1=11,y1=23},
 {x0=0,y0=0,x1=63,y1=31},
 {x0=63,y0=31,x1=0,y1=0},
 {x0=7,y0=3,x1=27,y1=13},
 {x0=27,y0=13,x1=7,y1=3},
}

function _draw()
 cls(0)
 local cols,rows,cw,ch=8,4,16,32
 local i=0
 for t in all(tests) do
  local col=i%cols
  local row=i\cols
  local ox=col*cw
  local oy=row*ch
  local x0,x1,y0,y1=t.x0,t.x1,t.y0,t.y1
  local minx=min(x0,x1)
  local miny=min(y0,y1)
  local bw=abs(x1-x0)
  local bh=abs(y1-y0)
  local pad=1
  local maxw,maxh=cw-2*pad,ch-2*pad
  local s=1
  if bw>maxw or bh>maxh then
   s=min(maxw/max(bw,1),maxh/max(bh,1))
  end
  line(
   ox+pad+flr((x0-minx)*s),
   oy+pad+flr((y0-miny)*s),
   ox+pad+flr((x1-minx)*s),
   oy+pad+flr((y1-miny)*s),
   7)
  i+=1
 end
end
__gfx__
