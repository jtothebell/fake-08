pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
function d() end
function g(d,n,e)local f,a,o,e,c=e or 1,n.e-d.e,n.d-d.d
 printh("g ok dist="..tostr(e))
end
function _init()
 i={{e=64,d=64,f=true,c={t="red"}}}
 d9(5,5)
 for d in all(b) do d:w() end
 printh("done")
end
b={}
function d9(d,n)add(b,{e=d*8,d=n*8,i=0,o=0,a=0,k=function(d)spr(6,d.e-4,d.d-4)end,w=function(d)for n in all(i)do if(g(d,n,.3)and n.f)sfx(15,0)
end D(d)d.o*=.98d.a*=.98end})
end
function D(d)return{}end
__gfx__
