#pragma once

#include <string>

#ifndef VER_STR
#define VER_STR "v0.0.0.0"
#endif

//from PicoLove api.lua

const char * fake08BiosP8 = R"#(
pico-8 cartridge // http://www.pico-8.com
version 27
__lua__
local carts={}
local numcarts = 0

local cidx=0
local carttoload = ""
local t=0
local linebuffer=""
local line
local bgcolor=5
local runcmd=false

--start customizable per platform
cartpath = "sdmc:/p8carts/"
selectbtn = "a"
pausebtn = "start"
versionstr = ")#" VER_STR  R"#("
exitbtn = "l + r"
sizebtn = "select to cycle screen sizes"
--end customizable per platform

function _init()
	if __listcarts then
		carts = __listcarts()
		numcarts = #carts
	end

	if __getbioserror then
		linebuffer = __getbioserror()
	end
end

function _update60()
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

function _draw()
	cls(bgcolor)
	spr(0, 1, 5, 6, 1)
	color(6)
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
	line=84


	rectfill(0, line, 128, line+5, bgcolor)
	color(7)
	print("> "..linebuffer, 0, line)
	if t%30<15 then
		rectfill((#linebuffer+2)*4, line, (#linebuffer+2)*4+3, line+4, 8)
	end
end

__gfx__
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777077777077007077777000000007777007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077077077000000000077077077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777077777077770077770077777077077077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077077077000000000077077077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077007077777000000077770077770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

)#";