#pragma once

#include <string>


//from PicoLove api.lua

const char * p8GlobalLuaFunctions = R"#(
-- The functions below are normally attached to the program code, but are here for simplicity
---------------------------------
--Table Helpers
---------------------------------
function all(a)
	if a==nil or #a==0 then
		return function() end
	end
	local i, li=1
	return function()
		if (a[i]==li) then i=i+1 end
		while(a[i]==nil and i<=#a) do i=i+1 end
		li=a[i]
		return a[i]
	end
end

function foreach(a, f)
	for v in all(a) do
		f(v)
	end
end

function count(a)
	local count=0
	for i=1, #a do
		if a[i]~=nil then count=count+1 end
	end
	return count
end

function add(a, v)
	if a==nil then return end
	a[#a+1]=v
end

function del(a, dv)
	if a==nil then return end
	for i=1, #a do
		if a[i]==dv then
			table.remove(a, i)
			return
		end
	end
end

---------------------------------
--Coroutines
---------------------------------
yield = coroutine.yield
cocreate = coroutine.create
coresume = coroutine.resume
costatus = coroutine.status

---------------------------------
--strings
---------------------------------
sub = string.sub

function tostr(val, hex)
	local kind=type(val)
	if kind == "string" then
		return val
	elseif kind == "number" then
		if hex then
			val=val*0x10000
			local part1=bit32.rshift(bit.band(val, 0xFFFF0000), 16)
			local part2=bit32.band(val, 0xFFFF)
			return string.format("0x%04x.%04x", part1, part2)
		else
			return tostring(val)
		end
	elseif kind == "boolean" then
		return tostring(val)
	else
		return "[" .. kind .. "]"
	end
end

---------------------------------
--Debug
---------------------------------
trace = debug.traceback

---------------------------------
--Math
---------------------------------
function rnd(x)
	if type(x) == 'table' then
		local idx = math.ceil(math.random()*#x)
		return x[idx]
	end

	return math.random()*(x or 1)
end

function srand(seed)
	seed = seed or 0
	math.randomseed(flr(seed*0x10000))
end

flr=math.floor
ceil=math.ceil

function sgn(x)
	return x<0 and-1 or 1
end

abs=math.abs

function min(a, b)
	if a==nil or b==nil then
		warning('min a or b are nil returning 0')
		return 0
	end
	if a<b then return a end
	return b
end

function max(a, b)
	if a==nil or b==nil then
		warning('max a or b are nil returning 0')
		return 0
	end
	if a>b then return a end
	return b
end

function mid(x, y, z)
	return (x<=y)and((y<=z)and y or((x<z)and z or x))or((x<=z)and x or((y<z)and z or y))
end

function cos(x)
	return math.cos((x or 0)*math.pi*2)
end

function sin(x)
	return-math.sin((x or 0)*math.pi*2)
end

sqrt=math.sqrt

function atan2(x, y)
	return (0.75 + math.atan2(x,y) / (math.pi * 2)) % 1.0
end

function band(x, y)
	return bit32.band(x*0x10000, y*0x10000)/0x10000
end

function bor(x, y)
	return bit32.bor(x*0x10000, y*0x10000)/0x10000
end

function bxor(x, y)
	return bit32.bxor(x*0x10000, y*0x10000)/0x10000
end

function bnot(x)
	return bit32.bnot(x*0x10000)/0x10000
end

function shl(x, y)
	return bit32.lshift(x*0x10000, y)/0x10000
end

function shr(x, y)
	return bit32.arshift(x*0x10000, y)/0x10000
end

function lshr(x, y)
	return bit32.rshift(x*0x10000, y)/0x10000
end

function rotl(x, y)
	return bit32.rol(x*0x10000, y)/0x10000
end

function rotr(x, y)
	return bit32.ror(x*0x10000, y)/0x10000
end

--Button emoji variables
⬅️ = 0
➡️ = 1
⬆️ = 2
⬇️ = 3
🅾️ = 4
❎ = 5
)#";


