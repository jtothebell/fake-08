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
        for j=i,#c do c[j]=c[j+1] end
        return v
    end
end

function serial(chan, address, len)
    --stubbed out
    --return len
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
debug = nil

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

--fiilp emoji variables
█ = 0
▒ = 0x5a5a.8
🐱 = 0x511f.8
░ = 0x7d7d.8
✽ = 0xb81d.8
● = 0xf99f.8
♥ = 0x51bf.8
☉ = 0xb5bf.8
웃 = 0x999f.8
⌂ = 0xb11f.8
😐 = 0xa0e0.8
♪ = 0x9b3f.8
◆ = 0xb1bf.8
… = 0xf5ff.8
★ = 0xb15f.8
⧗ = 0x1b1f.8
ˇ = 0xf5bf.8
∧ = 0x7adf.8
▤ = 0x0f0f.8
▥ = 0x5555.8

function menuitem(index, label, callback)
    --only 5 open slots
    if index < 1 or index > 5 then return end

    if not label or not callback then
        label = nil
        callback = nil
    end

    __f08_menu_items[index + 1][1] = label
    __f08_menu_items[index + 1][2] = callback
end

function __addbreadcrumb(label, carttoload)
    __f08_menu_items[8] = {nil, nil}
    __f08_menu_items[8][1] = label
    __f08_menu_items[8][2] = function() load(carttoload) end
end


__f08_menu_items = {
    [0] = {"continue", __togglepausemenu},
    {nil, nil},
    {nil, nil},
    {nil, nil},
    {nil, nil},
    {nil, nil},
    {"reset cart", __resetcart},
    {"exit to menu", __loadbioscart}
}

__f08_menu_selected = 0

function __f08_menu_update()
    if btnp(3) and __f08_menu_selected < #__f08_menu_items then
        repeat
            __f08_menu_selected = __f08_menu_selected + 1
        until __f08_menu_items[__f08_menu_selected][1] ~= nil
    end

    if btnp(2) and __f08_menu_selected > 1 then
        repeat
            __f08_menu_selected = __f08_menu_selected - 1
        until __f08_menu_items[__f08_menu_selected][1] ~= nil
    end

    if btnp(4) or btnp(5) then
        toexec = __f08_menu_items[__f08_menu_selected]
        if toexec and toexec[2] then
            toexec[2]()
            if __f08_menu_selected > 0 and __f08_menu_selected < 6 then
                __f08_menu_selected = 0
                __togglepausemenu()
            end
        end
    end

end

function __f08_menu_draw()
    local menuwidth = 82
    local itemcount = 0
    for i=0, 8, 1 do
        item = __f08_menu_items[i]
        if item and item[1] and item[2] then
            itemcount = itemcount + 1
        end
    end
    local menuheight = 8 + itemcount*10
    local menux = (128 - menuwidth) / 2
    local menuy = (128 - menuheight) / 2

    rectfill(menux, menuy, menux + menuwidth, menuy + menuheight, 0)
    rect(menux+1, menuy+1, menux + menuwidth-1, menuy + menuheight-1, 7)
    local itemx = menux + 8
    local itemy = menuy + 6
    local itemidx = 0

    for i=0, 8, 1 do
        item = __f08_menu_items[i]
        if item and item[1] and item[2] then
            print(item[1], itemx, itemy, 7)
            
            --draw selection indicator
            if itemidx == __f08_menu_selected then
                line(itemx-5, itemy, itemx-5, itemy+4, 7)
                line(itemx-4, itemy+1, itemx-4, itemy+3, 7)
                pset(itemx-3, itemy+2, 7)
            end

            itemy = itemy + 10
        end

        itemidx = itemidx + 1
    end
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


