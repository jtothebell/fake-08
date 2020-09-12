#pragma once

#include <string>


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


function _init()
	if __listcarts then
		carts = __listcarts()
		numcarts = #carts
	end

	if __getbioserror then
		linebuffer = __getbioserror()
	end

	cls(bgcolor)
	spr(0, 1, 5, 6, 1)
	color(6)
	cursor(0, 6*3)
	print("welcome to fake-08")
	print("a homebrew pico-8 emulator")
	print("currently in pre-alpha")
	print("")
	print("place p8 carts in sdmc:/p8carts/")
	if numcarts < 1 then
		print("--no carts found--")
	else
		print("⬅️➡️ to navigate carts")
		print("🅾️ (a) to load selected cart")
		print("start to close current cart")
	end
	print("r to cycle screen sizes")
	print("l + r to exit")
	print("")
	-- 18 pixels from cursor() call, then 
	-- 24 more from 4 print calls
	line=84
end

function _update60()
	t+=1
	
	if btnp(1) then
		cidx = min((cidx + 1), numcarts)
	end
	if btnp(0) then
		cidx = max((cidx - 1), 0)
	end
	
	if btnp(2) then
		--ls()
		linebuffer = "ls"
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
		linebuffer = "load " .. carttoload
	else
		carttoload = ""
	end
	
	if runcmd then
		--make call to global
		--load cart here
		
		if __loadcart and carttoload then
			__loadcart(carttoload)
		end
	end
		
end

function _draw()
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