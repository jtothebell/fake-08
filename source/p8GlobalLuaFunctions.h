#pragma once

#include <string>


//from zepto 8 bios.p8

const char * p8GlobalLuaFunctions = R"#(
-- The functions below are normally attached to the program code, but are here for simplicity
---------------------------------
--Table Helpers
---------------------------------

--from zepto 8 bios.p8
-- PicoLove functions did not return values added/deleted

function all(c)
    if (c==nil or #c==0) return function() end
    local i,prev = 1,nil
    return function()
        -- increment unless the current value changed
        if (c[i]==prev) i+=1
        -- skip until non-nil or end of table
        while (i<=#c and c[i]==nil) i+=1
        prev=c[i]
        return prev
    end
end

function foreach(c, f)
     for v in all(c) do f(v) end
end

sub = string.sub
pack = table.pack
unpack = table.unpack

-- Experimenting with count() on PICO-8 shows that it returns the number
-- of non-nil elements between c[1] and c[#c], which is slightly different
-- from returning #c in cases where the table is no longer an array. See
-- the tables.p8 unit test cart for more details.
--
-- We also try to mimic the PICO-8 error messages:
--  count(nil) → attempt to get length of local 'c' (a nil value)
--  count("x") → attempt to index local 'c' (a string value)
function count(c)
    local cnt,max = 0,#c
    for i=1,max do if (c[i] != nil) cnt+=1 end
    return cnt
end

-- It looks like table.insert() would work here but we also try to mimic
-- the PICO-8 error messages:
--  add("") → attempt to index local 'c' (a string value)
function add(c, x, i)
    if c != nil then
        -- insert at i if specified, otherwise append
        i=i and mid(1,i\1,#c+1) or #c+1
        for j=#c,i,-1 do c[j+1]=c[j] end
        c[i]=x
        return x
    end
end

function del(c,v)
    if c != nil then
        local max = #c
        for i=1,max do
            if c[i]==v then
                for j=i,max do c[j]=c[j+1] end
                return v
            end
        end
    end
end

function deli(c,i)
    if c != nil then
        -- delete at i if specified, otherwise at the end
        i=i and mid(1,i\1,#c) or #c
        local v=c[i]
        for j=i,#c-1 do c[j]=c[j+1] end
        return v
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
--Debug
---------------------------------
trace = debug.traceback

sub = string.sub
pack = table.pack
unpack = table.unpack

--Button emoji variables
⬅️ = 0
➡️ = 1
⬆️ = 2
⬇️ = 3
🅾️ = 4
❎ = 5

function menuitem()
--noop placeholder for now
end

--https://twitter.com/lexaloffle/status/1314463271324315649
--save host cpu by making most of the api local
local 
time, t, sub, chr, ord, tostr, tonum, 
add, del, deli, clip, color, pal, palt,
fillp, pget, pset, sget, sset, fget, 
fset, circ, circfill, rect, rectfill, oval,
ovalfill, line, spr, sspr, mget, mset, 
tline, peek, poke, peek2, poke2, peek4,
poke4, memcpy, memset, max, min, mid, flr, 
ceil, cos, sin, atan2, rnd, srand, band,
bor, bxor, bnot, shl, shr, lshr, rotl, rotr =

time, t, sub, chr, ord, tostr, tonum, 
add, del, deli, clip, color, pal, palt,
fillp, pget, pset, sget, sset, fget, 
fset, circ, circfill, rect, rectfill, oval,
ovalfill, line, spr, sspr, mget, mset, 
tline, peek, poke, peek2, poke2, peek4,
poke4, memcpy, memset, max, min, mid, flr, 
ceil, cos, sin, atan2, rnd, srand, band,
bor, bxor, bnot, shl, shr, lshr, rotl, rotr

)#";


