pico-8 cartridge // http://www.pico-8.com
version 39
__lua__

function _init()
 init_world() 
 init_rotate()
 instars=false
 lastscene=nil
 switch_title()
end

function init_world()
 player={
  r=4,x=0,y=0, walkspr=16,
 }
 wx=0
 wy=0
 world={}
 world[0]=planet1
 world[2]=planet2
 world[20]=planet3
 world[-2]=planet4
 world[-20]=planet5
end

sceneobj=nil
function switch_world()
 local scn = world[wy*10+wx]
 if scn==nil then
  sceneobj=starscene
 else
  sceneobj=scn
 end
 sceneobj.switch()
end

function _update60()
 lastscene=scene
	local s=scene
 if s=="title" then
  update_title()
 else
  sceneobj.update()
 end
 
 --must be last
 update_btnp1()
end

function _draw()
 local s=lastscene
 if s=="title" then
	 draw_title()
 else
  sceneobj.draw()
 end
end
-->8
--rotation
function addr(a)
 return 512*(a\16)+4*(a%16)
end

function h4(byte)
 return (byte<<4)/16%16
end

function l4(byte)
 return flr(byte>>4)
end

function init_rotate()
 for s = 16,31 do
  rotate_sprite(s,s+16)
 end
end



function rotate_sprite(a,b)
 a=addr(a)
 b=addr(b)
 for x=0,3 do
  for y=0,3 do
   local pix1=peek(a+x+y*128)
   local pix2=peek(a+x+y*128+64)
   local out1=l4(pix1)
   out1+=l4(pix2)*16
   local out2=h4(pix1)
   out2+=h4(pix2)*16
   poke(b+y+x*128,out2)
   poke(b+y+x*128+64,out1)
  end
 end
end

--btnp1
btnp1_bits=0
function update_btnp1()
 btnp1_bits=btn()
end

function btnp1(b)
 local mask=1<<b
 if 0!=band(btnp1_bits,mask) then
  return false
 end
 return 0!=band(btn(),mask) 
end
-->8
--stars
function gk(x,y)
 return flr(x)*128+flr(y)
end

wstmap={}

function getv(x,y)
 if scene=="title" then
  return { dx=cos(star_angle)-((128-x)/30), dy=sin(star_angle)}
 end
 local a=get_star_angle(x/128+wx,y/128+wy)
 local ent_x=(star_ent.x-wx)*128
 local ent_y=(star_ent.y-wy)*128
 local d=(x-ent_x)*cos(star_angle)+(y-ent_y)*sin(star_angle)
 d/=30
 return {
  dx=3*cos(a)+cos(a)*d,
  dy=3*sin(a)+sin(a)*d,
 }
end

function stars_init()
 if wy>0 then
  scx=2.5
  scy=4.5
  scr=3
 else
  scx=-2.5
  scy=-4.5
  scr=3
 end
 
 
 star_ent=get_enter_point()
 invoid=true
 if (star_ent != nil) then
  invoid=false
  star_angle=get_star_angle(star_ent.x,star_ent.y)
 end
 if scene=="title" then
  star_ent={x=1,y=0.6}
  star_angle=.45
  invoid=false
 end
 stars={}
 whirls={}
 if (invoid) return

 local starc={6,6,6,7,7,7,7,7,7,10}
 for i=1,200 do
  local star={
   c=starc[ceil(rnd(#starc))],
  }
  
  star.x=(star_ent.x-wx)*128
  star.y=(star_ent.y-wy)*128
  local shift= rnd(12)-6
  shift*=shift*sgn(shift)
  star.y-=shift*cos(star_angle)
  star.x+=shift*sin(star_angle)
  local lx,ly
  local try=1
  repeat
   try+=1
   local d=rnd(130)-4
	  lx=star.x+d*cos(star_angle)
	  ly=star.y+d*sin(star_angle)
	 
	 until try>5 or min(lx,ly) >=0 and max(lx,ly)<=128
	 star.x=lx
	 star.y=ly
  stars[i]=star
 end
 print(stars[100].c)
end

function get_star_angle(x,y)
 return atan2(scx-x,scy-y)+.25
end

function test_enter(e)
 local x = e.x
 local y = e.y
 local sa = get_star_angle(x,y)
 local sax=cos(sa)
 local say=sin(sa)
 if (x==wx and sax>0) return true
 if (x==wx+1 and sax<0) return true
 if (y==wy and say>0) return true
 if (y==wy+1 and say<0) return true
 return false
end

function get_enter_point()
 for xenter=wx,wx+1 do
	 local ybit=sqrt(scr^2-(xenter-scx)^2)
	 local ybit1=scy-ybit
	 local ybit2=scy+ybit
	 if ybit1>=wy and ybit1<=wy+1 then
	  local e={x=xenter, y=ybit1}
	  if (test_enter(e)) return e
	 elseif (ybit2>=wy and ybit2<=wy+1) then
	  local e={x=xenter, y=ybit2}
	  if (test_enter(e)) return e
	 end
	end
 
 for yenter=wy,wy+1 do
	 local xbit=sqrt(scr^2-(yenter-scy)^2)
	 local xbit1=scx-xbit
	 local xbit2=scx+xbit
	 if (xbit1>=wx and xbit1<=wx+1) then
	  local e={x=xbit1, y=yenter}
	  if (test_enter(e)) return e
	 elseif (xbit2>=wx and xbit2<=wx+1) then
	  local e={x=xbit2, y=yenter}
	  if (test_enter(e)) return e
	 end
	end
 
 return nil
end

function position_entering_star(star)
 star.x=rnd(1)
 star.y=rnd(12)-6
 star.y*=star.y*sgn(star.y)
 if invoid then
  star.y+=64
 else
  local rand_bit=star.y
  star.x=128*(star_ent.x-wx)
  star.y=128*(star_ent.y-wy)
  if star.x==0 then
   star.x+=rnd(1)
   star.y+=rand_bit
  elseif star.x==128 then
   star.x-=rnd(1)
   star.y+=rand_bit
  elseif star.y==0 then
   star.y+=rnd(1)
   star.x+=rand_bit
  elseif star.y==128 then
   star.y-=rnd(1)
   star.x+=rand_bit
  end
 end
 return star
end

function just_update_stars()
 for star in all(stars) do
  local v=getv(star.x,star.y)

  star.x+=v.dx/3
  star.y+=v.dy/3
  
  if min(star.x,star.y)<0 or max(star.x,star.y)>128 then
   position_entering_star(star)
  end
 end
 update_whirls()
end

function update_whirls()
	for w in all(whirls) do
  tlen=80
  local spawning=w.tick%4==0
  w.tick+=1
  w.fudge=sin(time())

  for i=tlen,2,-1 do
   
   local t=w.tail[i]
   if (spawning) t=w.tail[i-1]
   if (t!=nil) then
    local v=getv(t.x,t.y)
    if v!=nil then
     local dx,dy=rot(v,i,w.dir)
	    t.x+=dx/8+rnd(0.1)-.05
	    t.y+=dy/8+rnd(0.1)-.05
	    
	    w.tail[i]=t
	   end
   end
  end
  local c=w.c
  if rnd(1)>.5 then
   
   c*=2
   c+=10
  end
  
  if spawning then
	  w.tail[1]={
	   i=rnd(1000),
	   x=w.x,
	   y=w.y+w.fudge+rnd(5)-2.5,
	   c=c,
	  }
	 end
 end
end  

function rot(v,i,dir)
 local x=v.dx
 local y=v.dy
 local t=i/tlen
 t=t*t*t*2
 t*=dir
 local c=cos(t)
 local s=sin(t)
 return x*c-y*s, x*s+y*c
end

function stars_update60()
 just_update_stars()
 local sp=.02
 local dr=.99
 if btn(0) then
  p.dx-=sp
 end
 if btn(1) then
  p.dx+=sp
 end
 if btn(2) then
  p.dy-=sp
 end
 if btn(3) then
  p.dy+=sp
 end
 local vx,vy
 if not invoid then
  local v = getv(p.x,p.y)
  vx=v.dx/10
  vy=v.dy/10
 else
  vx,vy=0,0
 end
 p.dx-=vx
 p.dy-=vy
 p.dx *= dr
 p.dy *= dr
 p.dx+=vx
 p.dy+=vy
 p.x += p.dx
 p.y += p.dy
 p.s = (p.s+.05+(abs(p.dy) + abs(p.dx))*.1)%2
 
 if p.x<0 or p.x>128 or p.y<0 or p.y>128 then
  if (p.x<0) wx-=1
  if (p.x>128) wx+=1
  if (p.y<0) wy-=1
  if (p.y>128) wy+=1
  switch_world()
 end
end

invoid=true
scx=0.5
scy=4.5
scr=5
function switch_stars()
 local lastscene=scene
 scene="stars"
 init_whirls()
 --local
  
 stars_init()
 if invoid then
  music(-1,1000)
 else
  local m=stat(54)
  if (m<9 or m>39) music(39,1000) 

 end


 local sx
 local sy
 if lastscene=="stars" then
  sx=p.x
  sy=p.y
 else
	 p={dx=player.sx,dy=player.sy,s=0}
	 sx=player.x+planet.x
  sy=player.y+planet.y
 end
 if abs(sx-64) > 64 then
  p.x=sx%128
  p.y=64+(sy-64)/2
 else
  p.y=sy%128
  p.x=64+(sx-64)/2
 end
end


function init_whirls()
 whirls={}
 srand(wx*10+wy)
	for i=1,flr(rnd(3))+1 do
  local w={
   tick=0,
   x=rnd(55)+34,
   y=55+sgn(rnd(1)-.5)*sqrt(rnd(1600)),
   c=flr(rnd(2))+1,
   tail={},
   fudge=sin(time()),
  }
  w.dir=sgn(40-w.y)
  whirls[i]=w
 end
end

function draw_star(star,i)
 if (invoid) return
 i=flr(4*time()+i*.932849)%68
 if i>64 then
  i=i%5
  i/=5
  i=1-i
  i=i*i*i
  i=1-i
  i*=5
 else
  i=i%2
  i/=2
  i=1-i
  i=i*i*i
  i=1-i
  i*=2
 end
 spr(199+i,star.x-4,star.y-4)
end

function just_draw_stars()
 for i, star in pairs(stars) do
  
  draw_star(star,i)
 end

 for w in all(whirls) do
  pal(7,w.c*2+10)
  pal(5,w.c)
  for t in all(w.tail) do
   draw_star(t, t.i)
  end
  
  spr(213,w.x-4,w.fudge+w.y-2)
  pal(7,7)
  pal(5,5)
 end
end

function stars_draw()
 cls()
 just_draw_stars()
 spr(1+flr(p.s),p.x,p.y,1,1,p.dx<0)
end

starscene={
 init=stars_init,
 update=stars_update60,
 switch=switch_stars,
 draw=stars_draw,
}
-->8
--title screen
frdivider=0
function update_title()
 frdivider+=1
 --if frdivider%3==0 then
 just_update_stars()
 --end
 if btn(❎) then
  switch_world()
 end
end

function switch_title()
 scene="title"
 stars_init()

 init_whirls()
 music(1,3000)
end

function draw_title()
 cls()
 
 just_draw_stars()
 --rectfill(0,50,64,128,1)
 spr(72,0,64,8,8)
 spr(66+flr(time()*4%2)*3,0,80,3,3)
 print("❎ to start",5,54,7)
end
-->8
--planet1
function learning_planet_init()
 planet={x=64,y=64,r=13,c=3}
 if not knowshowtofly then
	 showhint=false
	 knowshowtotalk=false
	 knowshowtofly=false
	 talkingto=nil
	 holdtime=nil
	 learningtofly=false
	end
 player.y=-planet.r

 inventory={}
 grave={
  x=2, walkspr=5,
  y=planet.r,
  chatpitch=32,
  text={"   \"rip spot.\
he was the best friend\"",
   "...",
   "a sadness comes over you",
   "there is nothing for you\
  here any more",
   "what's the point\
of this life?"
  }
 }
 talkables={ grave }
end

function col(p1,p2)
 return abs(p1.x-p2.x)<4 and abs(p1.y-p2.y)<4
end

function update_dialogue_learning()
 if talkingto == nil then
  if (learningtofly) return
  showhint=false
  for t in all(talkables) do
 	 if col(t,player) then
 	  showhint=true
 	 end
 	end
 else
	 knowshowtotalk=true
 end
 local wastalking=talkingto!=nil
 update_dialogue()
 if talkingto==nil and wastalking then
  if not knowshowtofly then
   learningtofly=true
  end
 end 
end



function switch_planet1()
 scene="planet1"
 whirls={}
 learning_planet_init()
 if (lastscene!="title") then
  switch_planet()
 else
  music(-1,1000)
 end
end

function walk_player(p, dir)
 local ws=.5/2 // walk speed
 local x=planet
  
 if (dir==1) p.flipped=false
 if (dir==-1) p.flipped=true
 p.still=dir==0
 if dir==1 then
  if p.y == x.r then
   p.x-=ws
  end
  if p.x == -x.r then
   p.y-=ws
  end
  if p.y == -x.r then
   p.x+=ws
  end
  if p.x == x.r then
   p.y+=ws
   if (p.y==x.r) p.x-=ws
  end
 elseif dir==-1 then
  if (p.y== x.r) p.x+=ws
  if (p.x== x.r) p.y-=ws
  if (p.y==-x.r) p.x-=ws
  if (p.x==-x.r) then
   p.y+=ws
   if (p.y==x.r) p.x+=ws
  end
 end
end

function update_player()
 local sx=player.x+planet.x
 local sy=player.y+planet.y
 if abs(sx-64)>64 or abs(sy-64)>64 then
  if (sx<0) wx-=1
  if (sx>128) wx+=1
  if (sy<0) wy-=1
  if (sy>128) wy+=1
  switch_world()
  return
 end

 local p=player
 local x=planet
 local d=max(abs(p.y),abs(p.x))
 local g=d<2*x.r

 local dir=0
 if (btn(⬅️)) dir=-1
 if (btn(➡️)) dir=1
 walk_player(player,dir)
 
 local ws=.5 // walk speed
 local holding=btn(🅾️) and knowshowtotalk
 if holding and holdtime==nil then
  holdtime=time()
  sfx(0,0)
 end
 if holdtime!=nil and not holding then
  holdtime=nil
  if (g) sfx(27,0)
 end
 local floating=false
 if holdtime!=nil and time()-holdtime>.5 then
  floating = true  
 end
 fs=ws/2
 if g and floating then
	 if p.walking then
	  if (p.y== x.r) p.y+=p.r
	  if (p.x== x.r) p.x+=p.r
	  if (p.y==-x.r) p.y-=p.r
	  if (p.x==-x.r) p.x-=p.r
	  p.walking=false
	 end
  if (p.y>= x.r) p.y+=fs
  if (p.x>= x.r) p.x+=fs
  if (p.y<=-x.r) p.y-=fs
  if (p.x<=-x.r) p.x-=fs
 end
 
 if g and not floating then
  if (p.y> x.r) p.y-=fs
  if (p.x> x.r) p.x-=fs
  if (p.y<-x.r) p.y+=fs
  if (p.x<-x.r) p.x+=fs
  if (p.y== x.r+p.r) p.y-=p.r
  if (p.x== x.r+p.r) p.x-=p.r
  if (p.y==-x.r-p.r) p.y+=p.r
  if (p.x==-x.r-p.r) p.x+=p.r
  if max(abs(p.x),abs(p.y))==x.r then
   p.walking=true
   if abs(p.x)==x.r and abs(p.y)==x.r then
    p.x -=sgn(p.x)
   end   
  end
 end
 

 
 if not g then
  learningtofly=false
  knowshowtofly=true
  local acc = .02
  if (btn(⬅️)) p.sx-=acc
  if (btn(➡️)) p.sx+=acc
  if (btn(⬆️)) p.sy-=acc
  if (btn(⬇️)) p.sy+=acc
  p.sx*=.99
  p.sy*=.99
  p.x+=p.sx
  p.y+=p.sy
  return
 else
  local nextd=max(abs(p.y),abs(p.x))
  if nextd>=2*x.r then
   p.sx=0
   p.sy=0
	  if (p.y>= x.r) p.sy+=fs/2;p.y+=fs
	  if (p.x>= x.r) p.sx+=fs/2;p.x+=fs
	  if (p.y<=-x.r) p.sy-=fs/2;p.y-=fs
	  if (p.x<=-x.r) p.sx-=fs/2;p.x-=fs
	 else
	  p.sx=0
	  p.sy=0
	  p.x=sgn(p.x)*fs*flr(abs(p.x/fs))
	  p.y=sgn(p.y)*fs*flr(abs(p.y/fs))
	 end
 end

 walk_player(p,dir)
 if not p.walking then
  if (btn(⬅️)) p.x-=fs
  if (btn(➡️)) p.x+=fs
  if (btn(⬆️)) p.y-=fs
  if (btn(⬇️)) p.y+=fs
 end
end

function learning_planet_update()
 update_dialogue_learning()
 if (talkingto!=nil) return
 update_player()
end


function draw_planet(p)
 circfill(
  p.x,p.y,p.r*2,
  1
 )
 circfill(
  p.x,p.y,p.r*sqrt(2),
  p.c
 )
end

function draw_player(p)
 local pr = planet.r
 local x = p.x
 local y = p.y
 local r = p.r or 4 
 local s = 1
 local fx = false
 local fy = false
 
 if (abs(y) <=pr and (x==pr or x==-pr)) then
  fy=p.flipped
 else
  fx=p.flipped
 end
 if p.walking != false then
  s=p.walkspr
  if (p.chilling and p.chillspr) s=p.chillspr
  if singing and p == active_talkable then
   s=p.singspr
   s+= flr(stat(56)/(30))%2
   if stat(46)!=-1 then
    local note = get_note(stat(46),stat(50))
    if (note[1]%64>24) s+=2
   end
   
   --s+= 2*(flr(stat(56)/(30*4))%2)
  end
	 if (x== pr) then 
	  x+=r
	  s+=16
	  fx=not fx
	 elseif (x==-pr) then
	  x-=r-1
	  s+=16
	  fy=not fy
	 elseif (y== pr) then
	  y+=r
	  fy=not fy
	  fx=not fx
	 elseif (y==-pr) then
	  y-=r-1 
	 end
 end
  
 x+=planet.x
 y+=planet.y
 local fr=0
 local an=(false == p.walking) or (p.still == false) or (nil != p.anstill)
 if (talkingto!=nil and not col(active_talkable,player)) an=false
 if (an) fr=time()*4%2

 
 spr(s+fr,x-r,y-r,1,1,fx,fy)
end

function draw_learning_planet()
 draw_on_planet()
 if not knowshowtotalk and showhint then
  print("press ❎ to talk",30,10,7)
 end
 if learningtofly then
  print("hold 🅾️ to hold\
your breath",30,10,7)
 end
end

function draw_on_planet()
 cls()
 just_draw_stars()

 draw_planet(planet)
 for t in all(talkables) do
  draw_player(t)
 end
 draw_player(player)
 draw_dialogue()
end

planet1={
 init=learning_planet_init,
 switch=switch_planet1,
 draw=draw_learning_planet,
 update=learning_planet_update,
}
-->8
 --planet2
function switch_planet()
 stars_init()
 music(-1,1000)
 player.x=p.x%128-planet.x
 player.y=p.y%128-planet.y
 player.sx=p.dx
 player.sy=p.dy
 whirls={}
end

function zgn(x)
 if (x==0) return 0
 return sgn(x)
end

function normal_planet_update()
	update_dialogue()
	if (talkingto!=nil) then

	 return
	end
	for t in all(talkables) do
		if t.wandering then
		 if (t.dir==nil) t.dir=0
		 if t.lasttime < time() then
		  local wness=.2
		  t.dir=flr(rnd(1+2*wness)-wness)
		  t.lasttime=time()+rnd(3)
		  t.chilling=t.dir==0
		 end
	 	walk_player(t,t.dir)
	 end
	end
	update_player()
	--just_update_stars()
end

planet2={
	update=normal_planet_update,
	
	init=function ()
	 planet={x=32,y=48,r=10,c=2}
	 steve={
	  y=-planet.r,x=4,
	  walkspr=10,
	  text={
	   "hello, i'm steve.",
	   "i've been meditating\
for fifty years", {res={
	    ["fifty years!?"]={"yeah"},
	    ["what did you learn?"]={"that's not the point\ni'm being *present*."}
	   }}
	  }
	 }
	 talkables={ steve } 
	end,
	
	draw=function ()
	 draw_on_planet()
	end,
	
	switch=function ()
	 scene="planet2"
	 planet2.init()
	 switch_planet()
	end,
}


-->8
--planet 3
planet3={
	init=function ()
	 planet={x=32,y=48,r=14,c=3}
	 musician={
	  lasttime=time()+rnd(3),
	  chatpitch=12,
	  wandering=true,
	  playing=false,
	  y=-planet.r,x=4,
	  walkspr=28,
	  singspr=20,
	  still=false,
	  text={
	   "far out, man.", {res={
	    ["who are you"]={
	     "shane legend, man.\
maybe you heard of me?"},
	    ["far out, dude."]={
	    	"you dig? let me play\
a rad song for you."},
	    ["yeah i am very far out."]={
	     "oh cool, what brought\
you here, man?", {res={
	     ["i'm looking for meaning\
in this world"]={
	     	"my reason is music. you\
wanna learn to play the bass?"},
	     ["i'm lookin for a\
reason to live"]={
	      "man, sounds like life's got\
you down. you got the *blues*.",
	      "let me play you a\
song about just that.",
        { song = 3,
          lyrics = {
          "                                    \
          oh, oh, oh",
          "oh i got the blues\
  the blues  oh so bad",
          "oh i got the blues\
  don't know what wrong i did",
          "wish my heart would sing\
  can't remember when it did",
          "oh i got the blues\
  don't know what wrong i did",
         },
        },
        "hope you dug that"

       }
	     }}
	    }    
	   }}
	  }
	 }
	 mic = {
   walkspr=14,
	  x=2, 
	  y=planet.r,
	 }
	 talkables={ mic, musician } 
	end,
	
	
	update=function ()
  normal_planet_update()
	 if (talkingto!=nil) return
	 
	 
	end,
	
	switch=function ()
	 scene="planet3"
	 planet3.init()
	 switch_planet()
	end,
	
	draw=function ()
	 draw_on_planet()
	end,
}
-->8
--dialogue
slow_diaogue=nil
chatpitch=nil
singing=nil
singstart=nil

function nextline()
 textline+=1
 if textline > count(talkingto) then
  talkingto=nil
 elseif type(talkingto[textline])=="string" then
  chatter(textline)
 elseif talkingto[textline].song then
  singing=true
  singstart=time()
  music(talkingto[textline].song)
 else
  talkingto=talkingto[textline]
  dialogueoption=1
 end
end

function update_dialogue()
 _udialog()
 if (talkingto!=nil) then
	 local a,p=active_talkable,player
		 local dir=zgn(a.x-p.x)*zgn(p.y)+zgn(p.y-a.y)*zgn(p.x)
		 dir=sgn(dir)
	 	if col(a,p) then 
	 	 walk_player(p,dir)
	 	 p.still=false
	 	else
	 	 p.still=true
	 	 p.flipped=dir==1
	 	 if a.wandering then
 	 	 a.flipped=not p.flipped
 	 	end
	 end
	end
end

function _udialog()
 update_whirls()--preload
 
 if singing!=nil then
  local lyrics = talkingto[textline].lyrics
  if stat(55) >= #lyrics then
   singing=nil
   music(-1)
   nextline()
  else
   return
  end
 end
 
 if slow_dialogue!=nil then
  if btnp1(❎) then
   slow_dialogue=nil
   sfx(-1,1)
   return
  end
  local speed=.2
  slow_dialogue+=speed 
  if slow_dialogue > #talkingto[textline] then
   slow_dialogue=nil
   sfx(-1,1)
  end  
  --do something?
  return
 end
 if talkingto == nil then
	 for t in all(talkables) do
	  if col(t,player) then
	   if btnp1(❎) and t.text then
	    active_talkable=t
	    talkingto=t.text
	    chatpitch=t.chatpitch or 24
	    chatter(1)
	   end
	  end
	 end
 else
	 if type(talkingto[textline])=="string" then
		 if (btnp1(❎)) then
		  nextline()
  	end
  else
   if (btnp1(⬆️)) dialogueoption+=1
   if (btnp1(⬇️)) dialogueoption-=1
   local dsize=0
   for a,b in pairs(talkingto.res) do
    dsize+=1
   end
   dialogueoption=(dialogueoption-1)%dsize+1
   if btnp1(❎) then
    local i=0
    for a,b in pairs(talkingto.res) do
     i+=1
     if i==dialogueoption then
      talkingto=b
      chatter(1)
      
      break
     end
    end
   end
	 end
 end  
end

function draw_dialogue()
 if talkingto!=nil then
  if talkingto.res==nil then
   local s=talkingto[textline]
   if slow_dialogue!=nil then
    s=sub(s,1,flr(slow_dialogue))
   end
   if singing then
    local l = stat(55)
    s = s.lyrics[stat(55)+1]
    s=sub(s,1,flr(stat(56)/800*#s))
   end
   local bits=split(s,"*",false)
   
   local frst,hlt=true,false
   for b in all(bits) do
    local c=7
    if (hlt) c=11
    if frst then
     print(b.."\0",10,10,c)
    else
     print(b.."\0",c)
    end
    
    frst,hlt=false,not hlt
   end
  else
   local i=0
   local h=0
   for a,b in pairs(talkingto.res) do
    i+=1
    local lines=split(a,"\n")
    h+=#lines
    local y = 128-6*h
    local slctd=dialogueoption==i
    if (slctd) then
     color(10)
     print(">",4,y)
    else
     color(6)
    end
    

    print(a,10,y)
   end
  end
 end
end

function make_note(pitch, instr, vol, effect)
  local xinstr=flr(instr/8)
  instr = instr%8
  return {
   pitch + 64*(instr%4),
   128*xinstr + 16*effect + 2*vol + flr(instr/4)
  }
end

function get_note(sfx, time)
  local addr = 0x3200 + 68*sfx + 2*time
  return { peek(addr) , peek(addr + 1) }
end

function set_note(sfx, time, note)
  local addr = 0x3200 + 68*sfx + 2*time
  poke(addr, note[1])
  poke(addr+1, note[2])
end


function chatter(i)
 textline=i
 slow_dialogue=0
 local n=make_note(chatpitch,15,7,0)
 set_note(8,0,n)
 set_note(8,1,n)
 --sfx(8,1)
 sfx(7,1)
end
-->8
--planet4

planet4={
 update=normal_planet_update,
	
	init=function ()
	 planet={x=42,y=48,r=18,c=8}
	 sculptor={
	  lasttime=time()+rnd(3),
	  r=4,
	  dir=0,
	  wandering=true,
	  y=-planet.r,x=4,
	  walkspr=24,
	  p=planet,
	  flipped=true,
	  walking=true,
	  still=true,
	  chillspr=30,
	  chatpitch=10,
	  anstill=true,
	  chilling=true,
	  text={
	   "ooooh, yeah burns so good.",
	   "fuck, i'm gonna be so ripped.",
	   {res={
	    ["you're already pretty ripped"]={
	     "ha.\
i'm far from my goal weight",
	     "i'm gonna be the strongest\
man in the world",
	     {res={
	      ["what's the point? nothing\
is permanent."]={
	       "nah bro. glory is forever"},
	      ["i've been all over and you\
already are the strongest"]={
        "oh...",
        "i guess i should stop?"}
	     }}
	    },
	    ["who are you doing this for?"]={"for me, bro. i'm ripped\nand i feel great.",
	     {res={
	      ["it seems shallow"]={"oh, you think i need\nmore definition on the abs?"},
	      ["it's grotesque"]={"ha! \nyou think all this...", "is grotesque?"},
	     }}
	    }
	   }}
	  }
	 }
	 bench={
	  walkspr=48,
	  x=9,
	  y=planet.r,
	 }
	 talkables={ bench, sculptor } 
	end,
	
	draw=function ()
	 draw_on_planet()
	end,
	
	switch=function ()
	 scene="planet4"
	 planet4.init()
	 switch_planet()
	end,
}
-->8
--planet5

planet5={
 update=normal_planet_update,
	
	init=function()
	 planet={x=49,y=48,r=11,c=2}
	 painter={
	  lasttime=time()+rnd(3),
	  r=4,
	  dir=0,
	  wandering=true,
	  x=-planet.r,y=4,
	  walkspr=18,
	  p=planet,
	  flipped=true,
	  walking=true,
	  still=true,
	  chatpitch=28,
	  text={
	   "oh muse show me beauty.",
	   {res={
	    ["who are you?"]={
	     "i ",
	     "am an artist",
	    }
	   }}
	  }
	 }
	 easle={
	  r=4,x=-2, walkspr=11,
	  p=planet,y=-planet.r,
	  walking=true,
	  still=true,
	  text={"i liked her earlier work"}
  }
  vase={
	  r=4,x=6, walkspr=12,
	  p=planet,y=-planet.r,
	  walking=true,
	  still=true,
	  text={"i wonder where she\ngets her flowers"}
  }
	 talkables={ easle, vase, painter } 
	end,
	
	draw=function ()
	 draw_on_planet()
	end,
	
	switch=function ()
	 scene="planet5"
	 planet5.init()
	 switch_planet()
	end,
}
__gfx__
00000000000044000004440000004400000044400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000990ff000000ff00000cff00000cff000000000000000000000000000000000000000000007777000000000000080e00000000000067000000000000
00700700000999000999990000fc9c0000fc9c000000000000000000000000000000000000000000007ff70000f000000000b000000000000076000000000000
000770000fcccc000fcccc000fcccc0000fccc000000000000000000000000000000000000000000000ff000078e700000066600000000000067000000000000
00077000000ccc00000ccc00000ccc00000ccc00006660000000000000000000000000000000000000977900076b700000066600000000000005000000000000
0070070000c00c0000c00c0000c00c0000c00c0000d6d0a0000000000000000000000000000000000f9999f007677d0000006000000000000005000000000000
000000000c00c0000c00c0000c00c00000c00c00006660b0000000000000000000000000000000000555555000f0f0d000044400000000000005000000000000
0000000000000000000000000000000000000000046664b400000000000000000000000000000000005ff50000f0f00000040400000000000055500000000000
00440000044400000aae00000aae000000055000000550000005500000055000000ff000000ff00007770000077700000005500000055000000ff000000ff000
00ff000000ff00000aff00000aff0000000440000004400000044000000440000fffff000fffff0007ff000007ff00000004400000044000fffffff0fffffff0
00990000009900000af00000aef00000000eea00000eea00000eea00000eea000fffff000fffff00079700000797000000aee00000aee000fffffff0fffff5f5
09cc000009cc00000a4f0000e04f0000000ea400000ea400000ea000000ea0000ffff5000fffff500099000000990000009ee000009ee000f0fff5f5f0fff555
09cc000090cc0000004440000044400000a4a00000aea00000a4400000ae400000fdd5000f0dd55000f900000099f00000aee00000aee0000f0dd5550f0dd505
09fc00009fcc000000f400000f44000000aae00000a4e00000aae00000a4e000000ff50000ffff50006000000066000000a4e000004ee00000fff50500ffff00
00c0000000dc000000440000004f400000052000000520000005200000052000000ff00000ffff000060000000560000000200000005200000ffff0000ffff00
00c000000d0c000000f000000d0f0000005020000050200000502000005020000000f000000f0f0000600000050600000002000000502000000ff000000ff000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00800800008008000080080000800800008008000080080000800800008008000080080000800800008008000080080000800800008008000080080000800800
00088000000880000008800000088000000880000008800000088000000880000008800000088000000880000008800000088000000880000008800000088000
00088000000880000008800000088000000880000008800000088000000880000008800000088000000880000008800000088000000880000008800000088000
00800800008008000080080000800800008008000080080000800800008008000080080000800800008008000080080000800800008008000080080000800800
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
66600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
65600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
65600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
05000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
05555555000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
05000005000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000055544444444400000000000000000000000000
00000000000000000000000000000000000000990000000000000000000000000000000000000000000005555555544444444444000000000000000000000000
00000000000000000000000000000000000009990000000000000000000099990000000000000000000005555555544444444444000000000000000000000000
00000000000000000000000000000000000099990000000000000000000999990000000000000000000055444444444444444444000000000000000000000000
00000000000000000000000000000000000999990000000000000000000999990000000000000000000055444444444444444fff000000000000000000000000
000000000000000000000000000000099999999900000000000000000099999900000000000000000000554444444444444fffff000000000000000000000000
000000000000000000000000000009999999999900000000000000000099999900000000000000000000000004444444444fffff000000000000000000000000
000000000000000000000000000009999999999000000000000099900999999900000000000000000000000000044444444f444f000000000000000000000000
00000000000000000000000000009999999999000000000000099999999999990000000000000000000000000004444fff4ff7cf000000000000000000000000
00000000000000000000000000099999999000000000000000999999999999990000000000000000000000000004444f6fff677f000000000000000000000000
00000000000000000000000999999999000000000000000000999999999999000000000000000000000000000005444ffffffffff00000000000000000000000
0000000000000000000000999999999000000000000000000999999999990000000000000000000000000000000054446ffffffff00000000000000000000000
000000000000000000000099990000000000000000000000099999000000000000000000000000000000000000000004ffffffff000000000000000000000000
000000000000000000000999900000000000000000000009999990000000000000000000000000000000000000000000fffffffe000000000000000000000000
000000000000000000000999900000000000000000009999999900000000000000000000000000000000000000000000ddffffff000000000000000000000000
000000000000000000099999000000000000000000099999999900000000000000000000000000000000000000099999ffddffff000000000000000000000000
000000000000000000099999000000000000000000099999999000000000000000000000000000000000000009999999fff90000000000000000000000000000
00000000000000000009999900000000000000000009990000000000000000000000000000000000000000009999999999999000000000000000000000000000
00000000000000000000000000000000000000000009900000000000000000000000000000000000000000009999999999999000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000009999999999999000000000000000000000000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099000cccccccc000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000009000cccccccccc00000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccccc00000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccccccc0000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccccccc0000000000000000000000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccccccccc0000000000000000000000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccc1cccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cc111c110000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cc1c11c10000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cc111c110000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccc1cccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccccccccc0000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccccccccccf000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff0ccccccccccf000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff0ccccccccccf000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffcc1111111cf000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffccccccccccccc0000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff0fccccc1cccccccccc000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff0cccccc1cccccccccccccc00000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff0cccccccccccccccccccc1cc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffccccccccccccccccccc1ccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccccccccccccccc1ccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc000ccccccccc1cccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc00000cccccc1ccccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc000000000000ccccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc000000000000ccccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc000000000000ccccc000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc000000000000cccc0000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc00000000000ccccc0000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccc00000000000ccccc0000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc000000000000ccccc0000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc00000000000ccccc00000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc00000000000ccccc00000000000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc00000000dddddddcccc000000000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ccccccc0000000dddddddddccccdddd0000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000cccccccdddd0ddddddddddddddddddd0000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000dddddcccccccdddddddddddddddddddddddd0000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000ddddddcccccccdddddddddddddddddddddddd0000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000ddddddcccccccddddddddddddddddddddddd00000000
00000500000000000000050000000000000700000007057000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000050000000000000005000000000000070000005777500007000000000000000000000000000000000000000000000000000000000ccccc00000000000000
00005750000000000000575000000000077777000777770007777700000000000000000000000000000000000000000000000000000ccc0000cc000000000000
0000575000000000000057500000000000777000000777500077700000000000000000000000000000060000000700000000000000cc0000000c000000000000
5555555555500000555555555550000000707000000750700a777a000005000000060000000700000067600000767000000000000cc00000000c000000000000
057577757500000005757775750000000000000000070000070007000000000000000000000000000006000000070000000000000cc00007000c000000000000
0055777550000000005577755000000000000000000000000000000000000000000000000000000000000000000000000000000000cc0000000c000000000000
00055755000000000005575500000000000000000000000000000000000000000000000000000000000000000000000000000000000c0000000cc00000000000
00575557500000000057555750000000000000000000000000000000000000000000000000000000000000000000000000000007000c0000000c000700000000
00555055500000000055505550000000007000000770000000000000000000000000000000000000000000000000000000000000000cc000000c000000000000
055000005500000005500000550000000777000077750000000000000000000000000000000000000000000000000000000888000000c000000c000000000000
000000000000000000000000000000000070000077750000000000000000000000000000000000000000000000000000000808880000cc0700cc000000000000
0000000000000000000000000000000007070000775500000000000000000000000000000000000000000000000000000088000888000c000cc00000aaaaaa00
0000000000000000000000000000000000000000055000000000000000000000000000000000000000000000000000000080000088800ccccc0000aa00000aa0
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000008000000088000000000aa0000000a0
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000800000000800000000a000000000a0
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000080070000780007000a0070070700a0
0000000000000000000000000000000000000000007000000000000000000000000000000000000000000000000000000088000000080000000aa000000000a0
0000000000000000000000000000000000000000007707700000000000000000000000000000000000000000000000000008800008800000000aaaaa00000aa0
00000000000000000000000000000000000000000077775000000000000000000000000000000000000000000000000000000888880000bbc00000aaa0000a00
0000000000000000000000000000000000000000077777000000000000000000000000000000000000000000000000000000000000000bb0bb000000aaaaaa00
000000000000000000000000000000000000000077777770000000000000000000000000000000000000000000000000000000000000bb070bb0000000aaa000
000000000000000000000000000000000000000000777777000000000000000000000000000000000000000000000000000000000000bc0000b0000000000000
000000000000000000000000000000000000000000077000000000000000000000000000000000000000000000000000000000007000bc0000bb000700000000
000000000000000000000000000000000000000000070000000000000000000000000000000000000000000000000000000000000000bc00000b000000000000
000000000000000000000000000000000000000000005000000000000000000000000000000000000000000000000000000000000000bc0000cb000000000000
000000000000000000000000000000000000000000057000000000000000000000000000000000000000000000000000000000000000bc0700cb000000000000
000000000000000000000000000000000000000057575000000000000000000000000000000000000000000000000000000000000000bb000cbb000000000000
0000000000000000000000000000000000000000057757500000000000000000000000000000000000000000000000000000000000000bb0cbb0000000000000
00000000000000000000000000000000000000000077750000000000000000000000000000000000000000000000000000000000000000bbbb00000000000000
000000000000000000000000000000000000000005575000000000000000000000000000000000000000000000000000000000000000000c0000000000000000
00000000000000000000000000000000000000005755750000000000000000000000000000000000000000000000000000000000000000000000000000000000
__label__
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888ffffff882222228888888888888888888888888888888888888888888888888888888888888888228228888ff88ff888222822888888822888888228888
88888f8888f882888828888888888888888888888888888888888888888888888888888888888888882288822888ffffff888222822888882282888888222888
88888ffffff882888828888888888888888888888888888888888888888888888888888888888888882288822888f8ff8f888222888888228882888888288888
88888888888882888828888888888888888888888888888888888888888888888888888888888888882288822888ffffff888888222888228882888822288888
88888f8f8f88828888288888888888888888888888888888888888888888888888888888888888888822888228888ffff8888228222888882282888222288888
888888f8f8f8822222288888888888888888888888888888888888888888888888888888888888888882282288888f88f8888228222888888822888222888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555
55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000000000000000000000000000000000005555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000000011111111112222222222333333333305555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000088888888000000000000000005555550444444444455555555556666666666777777777705555555
55555550000000000000000000000000000000000000000088888888000000000000000005555557777777777775555555556666666666777777777705555555
5555555000000000000000000000000000000000000000008888888800000000000000000555555700000000007999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000000000000000000008888888800000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000000000000000000008888888800000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000000000000000000008888888800000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000000000000000000008888888800000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000000000000000000008888888800000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000088888888888888880000000000000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000088888888888888880000000000000000000000000555555708888888807999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000088888888888888880000000000000000000000000555555700000000007999999999aaaaaaaaaabbbbbbbbbb05555555
5555555000000000000000000000000088888888888888880000000000000000000000000555555777777777777dddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000000000000888888888888888800000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000000000000888888888888888800000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000000000000888888888888888800000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000000000000888888888888888800000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000088888888888888880000000000000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000088888888888888880000000000000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000088888888888888880000000000000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000088888888888888880000000000000000000000000000000005555550ccccccccccddddddddddeeeeeeeeeeffffffffff05555555
55555550000000000000000088188888888888880000000000000000000000000000000005555550000000000000000000000000000000000000000005555555
55555550000000000000000081718888888888880000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000018881888888888880000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000178887188888888880000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000010001000000000000000000000000000000000000000000005555550000000555556667655555555555555555555555555555555
55555550000000000000000001710000000000000000000000000000000000000000000005555550000000555555666555555555555555555555555555555555
5555555000000000000000000010000000000000000000000000000000000000000000000555555000000055555556dddddddddddddddddddddddd5555555555
555555500000000000000000000000000000000000000000000000000000000000000000055555500080005555555655555555555555555555555d5555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000555555576666666d6666666d666666655555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550000000555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555556665666555556667655555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555556555556555555666555555555555555555555555555555555
5555555000000000000000000000000000000000000000000000000000000000000000000555555555555555555556dddddddddddddddddddddddd5555555555
555555500000000000000000000000000000000000000000000000000000000000000000055555565555565555555655555555555555555555555d5555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555556665666555555576666666d6666666d666666655555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555550005550005550005550005550005550005550005550005555
555555500000000000000000000000000000000000000000000000000000000000000000055555011d05011d05011d05011d05011d05011d05011d05011d0555
55555550000000000000000000000000000000000000000000000000000000000000000005555501110501110501110501110501110501110501110501110555
55555550000000000000000000000000000000000000000000000000000000000000000005555501110501110501110501110501110501110501110501110555
55555550000000000000000000000000000000000000000000000000000000000000000005555550005550005550005550005550005550005550005550005555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555550000000000000000000000000000000000000000000000000000000000000000005555555555555555555555555555555555555555555555555555555
55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555
55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555
555555555555575555555ddd55555d5d5d5d55555d5d555555555d5555555ddd5555550000000055555555555555555555555555555555555555555555555555
555555555555777555555ddd55555555555555555d5d5d55555555d55555d555d555550000000056666666666666555557777755555555555555555555555555
555555555557777755555ddd55555d55555d55555d5d5d555555555d555d55555d55550000080056ddd6d6d6ddd6555577ddd775566666555666665556666655
555555555577777555555ddd55555555555555555ddddd5555ddddddd55d55555d55550008800056d6d6d6d666d6555577d7d77566dd666566ddd66566ddd665
5555555557577755555ddddddd555d55555d555d5ddddd555d5ddddd555d55555d55550088000056d6d6ddd6ddd6555577d7d775666d66656666d665666dd665
5555555557557555555d55555d55555555555555dddddd555d55ddd55555d555d555550000000056d6d666d6d666555577ddd775666d666566d666656666d665
5555555557775555555ddddddd555d5d5d5d555555ddd5555d555d5555555ddd5555550000000056ddd666d6ddd655557777777566ddd66566ddd66566ddd665
55555555555555555555555555555555555555555555555555555555555555555555550000000056666666666666555577777775666666656666666566666665
55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555566666665ddddddd5ddddddd5ddddddd5
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000044000004440000004400000044400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000990ff000000ff00000cff00000cff000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700000999000999990000fc9c0000fc9c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000770000fcccc000fcccc000fcccc0000fccc000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000000ccc00000ccc00000ccc00000ccc000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0070070000c00c0000c00c0000c00c0000c00c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000c00c0000c00c0000c00c00000c00c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000004400000444000000440000000000000000000000000000000000000000000000000000077700000777000000000000000000000000000000000000
0000000000ff000000ff000000ff000000000000000000000000000000000000000000000000000007ff000007ff000000000000000000000000000000000000
00000000009900000099000000990000000000000000000000000000000000000000000000000000079700000797000000000000000000000000000000000000
0000000009cc000009cc000009cc0000000000000000000000000000000000000000000000000000009900000099000000000000000000000000000000000000
0000000009cc000090cc000009ccc00000000000000000000000000000000000006660000000000000f900000099f00000000000000000000000000000000000
0000000009fc000090ccf00009fc0f000000000000000000000000000000000000d6d0a000000000006000000066000000000000000000000000000000000000
0000000000c0000000dc000000c0c00000000000000000000000000000000000006660b000000000000000000056000000000000000000000000000000000000
0000000000c000000d0c000000c0c00000000000000000000000000000000000046664b400000007777777777006000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000000007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000000007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000008007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000880007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007008800007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000000007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000000007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007000000007000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000007777777777000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888

__sfx__
01040000256112c611306113261133611356113661137621386213762000002000020000200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0340100010624056212b611326213262232625006000060007624056212b61137621376223762500600006002b6002b6002b60035600356003560000600006002b6002b6002b6003560035600356000060000600
484000081f02123001260212400129001280012602124021000010000100001000010000100001000010000100001000010000100001000010000100001000010000100001000010000100001000010000100001
1d4302001855518505375053750537505305053250533505355053750537505335053550536505375050050500505005050050500505005050050500505005050050500505005050050500505005050050500505
3b400020185661c5561f55624556185661c5561f556245561d5662155624556295561d566215562455629556215662455628556305561f56624556265562b556185661d5562455629556185661d5562455629556
01400000265602b56028560295602843028560295602b55029560295602923029560295602d2302f23724560345602f5602f56032560305603056030550305402f5602d5602b2302623729560292302d5502b540
011000202864500605006052860528605006050060500605266452664500605260052600500605006050060528645006050060528605246450060500605006052464524645006052460524605006050060500605
110d000000703237532675327753297532d7532b75327753007032b75328753007032b753297532d7532c753297530070328753007032d7532c7532b7530070300703307532e753007032b7532d7532875324753
01d000020bf7013f700cf000ef000cf000ef000cf000ef000cf000ef000cf000ef000cf000ef000cf000ef000000000000000000000000000000000000000000000000000000000000000000000000000000bf00
1f0500000010000100001002c1502d1502e1503015032150341503515035150351503415033150321503115031150001000010000100001000010000100001000010000100001000010000100001000010000100
011e000018b5018b501cb001bb501bb5000b001fb501fb501db501db5000b001db501db5000b001bb501bb501ab501ab5000b001ab501ab5000b001bb501bb5018b5018b501cb001bb501bb5000b001db501eb51
011e00000cb501bb501cb001fb500fb50000000fb5023b5011b5021b500000021b5011b50000000fb501fb500eb501db50000001fb501fb50000001db501db501bb501bb501cb001fb501fb500000021b5022b50
011e00001fb501fb500cb5021b5021b500000026b5026b5024b5024b5011b5024b5024b500000023b5023b5021b5021b500eb5023b5023b500000023b5023b501fb501fb500cb5021b5021b500000024b5025b50
011e00000cb50000000cb50000000fb50000000fb500000011b500000011b500000011b5011b000fb50000000eb50000000eb50000000bb50000000bb500c5720cb500c5720cb500c5720fb500c5720fb500c572
011e00000c5720e5720f5721357211572115721157211572115721157211572115720e5710e5710f5710f5720f5720f5720e5720e5720e5720b5720b5720b5720c5710c5720c5720c5720f5720f5720e5720e572
011e00000c5720e5720f572135721457214572145721457211572115721157211572145711452111571115720f5720f5720e5720e5720e5720b5720b5720b5720c5710c5720c5720c5720f5720f5720e5720e572
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
d3200000265102651026520265202653226532265422654232520325303254232552305203053030532305422d5402d55026530265402d55028520285322854229550305502d5502955028550305502b55024550
d320000028520285202853028530285422854228552285522d5202d5302d5222d522305203053030522305222d5502d5522f5602f5522b5502b5522f5602f552305502f5602b5502f560295502f5602955028550
d320000029530295302954029540295522955229562295622d5502d5502d5522d5522e5702d5502e5702d5502e5702e5722d5502d5522e5702e5722d5502d552295502e5722d5502b550285502b5502955021550
132000000277402761027520274202770027610275202742027700276102752027420277002760027520274205771057620575205742057700576105752057420777107762077520774207770077610775207742
0f1000200564505605056252b605056250560505625006050564505605056252b605056250560505625006050564505605056252b605056250560505625006050562505605056250060505625056052963511655
d31000002651026510265202652026532265322654226542265322653226522265222651226512265122651526500265002650026500265002650026500265000000000000000000000000000000000000000000
011000000564505605000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
011000003861037615000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__music__
03 01024344
03 04050644
03 07424344
00 0a0b0c0d
01 0a0b0c0e
02 0a0b0c0f
00 01424344
00 01424344
00 08424344
01 01421744
00 14011744
00 1401174a
00 15011744
00 16011744
00 15140117
00 16140117
00 14150117
00 16140117
00 15141817
00 16141817
00 14151817
00 16141817
00 15141857
00 16141857
00 14151857
00 16141857
00 1514011a
00 16140157
00 14150157
00 16140157
00 14011744
00 14011744
00 15011744
00 16011744
00 14015744
00 14015744
00 15015744
00 16015744
00 01194344
02 01424344
00 41424344
03 17424344

