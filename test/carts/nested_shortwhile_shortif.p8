pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
-- Regression: short-while body must not absorb the next line when it
-- ends with a short-if (pole station eo() map decoder).
function n6(n)if(ord(n)>=92)return ord(n)-41
return ord(n)-40end
function _init()
  local e="))(0Q((6c(d((,b(9)d((+Y/87"
  local t,o,l,n,d,f,u,i=3,0,0,1,1,n6(e[1])*8,n6(e[2])*8,{}
  for row=1,f do i[row]={} for col=1,u do i[row][col]=0 end end
  while t<#e do l=n6(e[t])o=n6(e[t+1])i[n][d]=l n+=1if(n>f)n=1d+=1
  while(o>0)i[n][d]=l o-=1n+=1if(n>f)n=1d+=1
  t+=2 end
  printh("ok")
end
__gfx__
