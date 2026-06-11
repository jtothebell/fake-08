pico-8 cartridge // http://www.pico-8.com
version 16
__lua__
function d() end
function dd() end
function C(a,b,c,d) return 1 end
function g(d,n,e)local f,a,o,e,c=e or 1,n.e-d.e,n.d-d.d,C(d.e,d.d,n.e,n.d),(d.dd or 5)+(n.dd or 5) return a end
function _init()
 printh(g({e=104,d=440,i=0,o=0,a=0},{e=596,d=413,f=true,i=0,o=0,a=0,c={t="red"}},.3))
 printh("with_dd="..g({e=104,d=440,dd=5,i=0,o=0,a=0},{e=596,d=413,dd=5,f=true,i=0,o=0,a=0,c={t="red"}},.3))
 printh("done")
end
__gfx__
