#pragma once

#include <string>

#ifndef VER_STR
#define VER_STR "v0.0.0.0"
#endif

//from PicoLove api.lua

const char * fake08BiosP8 = R"#(
pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
--all themes

--start customizable per platform
cartpath = "sdmc:/p8carts/"
selectbtn = "a"
pausebtn = "start"
versionstr = ")#" VER_STR  R"#("
exitbtn = "l + r"
sizebtn = "select to cycle screen sizes"
--end customizable per platform


-->8
--classic

function classic_init()
	
	if __getsetting then
			pal(1, __getsetting('p8_bgcolor'))
			pal(7, __getsetting('p8_textcolor'))
	end
	carts={}
	numcarts = 0
	
	cidx=0
	carttoload = ""
	t=0
	linebuffer=""
	
	bgcolor=1
	runcmd=false

	if __listcarts then
		carts = __listcarts()
		numcarts = #carts
	end

	if __getbioserror then
		linebuffer = __getbioserror()
	end
end

function classic_update()
	t+=1
	if btnp(1) then
		cidx = min((cidx + 1), numcarts)
	end
	if btnp(0) then
		cidx = max((cidx - 1), 1)
	end
	
	if btnp(2) then
		cidx = max((cidx - 10), 1)
	end
	if btnp(3) then
		cidx = min((cidx + 10), numcarts)
	end

	if btnp(4) then
		linebuffer = ""
	end
	if btnp(5) then
		linebuffer = ""
		runcmd = true
	end
	
	if cidx > 0 and cidx <= numcarts then
		carttoload = carts[cidx]
		local lastslashidx = string.find(carttoload, "/[^/]*$")
		local dispstr = carttoload
		if lastslashidx ~= nil and lastslashidx > 0 then
			dispstr = sub(dispstr, lastslashidx + 1)
		end
		linebuffer = "load " .. dispstr
	else
		carttoload = ""
	end
	
	if runcmd then
		--make call to global
		--load cart here
		
		if load and carttoload then
			load(carttoload)
		end
	end
		
end

function classic_draw()
	cls()
	rectfill(0,0,128,128,bgcolor)
	spr(0, 1, 5, 6, 1)
	color(7)
	cursor(0, 6*3)
	print("welcome to fake-08")
	print("a homebrew pico-8 emulator")
	print("currently in alpha (" .. versionstr .. ")")
	print("")
	print("place p8 carts in " .. cartpath)
	if numcarts < 1 then
		print("--no carts found--")
	else
		print("⬅️➡️ to navigate carts 1 by 1")
		print("⬆️⬇️ to navigate carts 10 by 10")
		print("🅾️ (" .. selectbtn .. ") to load selected cart")
		print(pausebtn .. " to close current cart")
	end
	print(sizebtn)
	print(exitbtn .. " to exit")
	print("")
	-- 18 pixels from cursor() call, then 
	-- 24 more from 4 print calls
	linenum=84


	rectfill(0, linenum, 128, linenum+5, bgcolor)
	color(7)
	print("> "..linebuffer, 0, linenum)
	if t%30<15 then
		rectfill((#linebuffer+2)*4, linenum, (#linebuffer+2)*4+3, linenum+4, 8)
	end
end

-->8
--runner

inits = {classic_init}
function _init()
	theme = 0 
	
	inits[theme+1]()
end

updates = {classic_update}
function _update60()
	updates[theme+1]()
end

draws = {classic_draw}
function _draw()	
	draws[theme+1]()
end



__gfx__
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777077777077007077777000000007777007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077077077000000000077077077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777077777077770077770077777077077077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077077077000000000077077077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077007077777000000077770077770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

)#";