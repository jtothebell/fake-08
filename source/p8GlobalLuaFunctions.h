#pragma once

#include <string>


//from zepto 8 bios.p8
//this is the bios-- needs to get loaded first, and then will run every cart

const char * p8Bios = R"#(
-- The functions below are normally attached to the program code, but are here for simplicity
---------------------------------
--Table Helpers
---------------------------------

--string indexing: https://lua-users.org/wiki/StringIndexing
getmetatable('').__index = function(str,i) return string.sub(str,i,i) end

--from zepto 8 bios.p8
-- PicoLove functions did not return values added/deleted

__z8_load_code = load
--todo: see if we need the other z8 stuff
__z8_stopped = false
__z8_persist_delay = 0
__z8_cart_running = false

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

-- Experimenting with count() on PICO-8 shows that it returns the number
-- of non-nil elements between c[1] and c[#c], which is slightly different
-- from returning #c in cases where the table is no longer an array. See
-- the tables.p8 unit test cart for more details.
--
-- count() takes an optional value as its second argument, if this is present
-- then count() will return the number of times the value is found in the table.
--
-- We also try to mimic the PICO-8 error messages:
--  count(nil) → attempt to get length of local 'c' (a nil value)
--  count("x") → attempt to index local 'c' (a string value)
function count(c,v)
    local cnt,max = 0,#c
    if v == nil then
        for i=1,max do if (c[i] != nil) cnt+=1 end
    else
        for i=1,max do if (c[i] == v) cnt+=1 end
    end
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


local error = error
local tonumber = tonumber
local __cartdata = __cartdata

---------------------------------
--Coroutines
---------------------------------
yield = coroutine.yield
cocreate = coroutine.create
coresume = coroutine.resume
costatus = coroutine.status
yield = coroutine.yield

---------------------------------
--Debug
---------------------------------
trace = debug.traceback

sub = string.sub
pack = table.pack
unpack = table.unpack

function stop()
    __z8_stopped = true
    --error()
end

function assert(cond, msg)
    if not cond then
        color(14) print("assertion failed:")
        color(6) print(msg or "assert()")
        stop()
    end
end

function cartdata(s)
    if __cartdata() then
        print('cartdata() can only be called once')
        abort()
        return false
    end
    -- PICO-8 documentation: id is a string up to 64 characters long
    if #s == 0 or #s > 64 then
        print('cart data id too long')
        abort()
        return false
    end
    -- PICO-8 documentation: legal characters are a..z, 0..9 and underscore (_)
    -- PICO-8 changelog: allow '-' in cartdat() names
    if string.match(s, '[^-abcdefghijklmnopqrstuvwxyz0123456789_]') then
        print('cart data id: bad char')
        abort()
        return false
    end
    return __cartdata(s)
end

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
        callback = nil
    end

    __f08_menu_items[index][1] = label
    __f08_menu_items[index][2] = callback
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
    {"exit to menu", __loaddefaultcart},
    {"exit to settings", __loadsettingscart}
}

__f08_menu_selected = 0

function __f08_menu_update()
    if btnp(3) and __f08_menu_selected < #__f08_menu_items then
        repeat
            __f08_menu_selected = __f08_menu_selected + 1
        until __f08_menu_items[__f08_menu_selected][1] ~= nil
    end

    if btnp(2) and __f08_menu_selected > 0 then
        repeat
            __f08_menu_selected = __f08_menu_selected - 1
        until __f08_menu_items[__f08_menu_selected][1] ~= nil
    end

    if btnp(0) or btnp(1) or btnp(4) or btnp(5) then
		local bval = 0
		local allowreturn = false
		
		if btnp(4) or btnp(5) then
			bval = 16+32+64
			allowreturn = true
		else
			if btnp(0) then 
				bval = 1 
			elseif btnp(1) then 
				bval = 2			
			end
			
		end
		
        toexec = __f08_menu_items[__f08_menu_selected]
        if toexec and toexec[2] then
            local toexec_result = toexec[2](bval)
			
            if __f08_menu_selected > 0 and __f08_menu_selected < 6 and allowreturn and not toexec_result then
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
        if item and item[1] then
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
        if item and item[1] then
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
bor, bxor, bnot, shl, shr, lshr, rotl, rotr,
rrect, rrectfill =

time, t, sub, chr, ord, tostr, tonum, 
add, del, deli, clip, color, pal, palt,
fillp, pget, pset, sget, sset, fget, 
fset, circ, circfill, rect, rectfill, oval,
ovalfill, line, spr, sspr, mget, mset, 
tline, peek, poke, peek2, poke2, peek4,
poke4, memcpy, memset, max, min, mid, flr, 
ceil, cos, sin, atan2, rnd, srand, band,
bor, bxor, bnot, shl, shr, lshr, rotl, rotr,
rrect, rrectfill

--save state code modified from ps4-p8
--https://github.com/voliva/ps4-p8/blob/ecba7f93ef9ba73ccb121b45ede6f46e651cef65/pico8_ps4/lua_lang_fns.cpp
--MIT license

eris.perm = {}
eris.unperm = {}
eris.original_G = {}

eris.init_persist_all = function()
  -- lua pairs is not sorted. The order is actually random, changes on every execution (wtf?)
  
  eris.settings("path", true)
  
  local keyset={}
  local n=0
  for k,v in pairs(_G) do
    n=n+1
    keyset[n]=k
  end
  table.sort(keyset)
  local i=0
  for i=1,n do
    local k=keyset[i]
    local v=_G[k]
    eris.perm[v] = i
    eris.unperm[i] = v
    eris.original_G[k] = v
  end
end

eris.persist_all = function()
  local new_symbols = {}
  for k,v in pairs(_G) do
    if eris.original_G[k] != v then
       new_symbols[k] = v
    end
  end

  return eris.persist(eris.perm, new_symbols)
end

eris.restore_all = function(persisted)
  local new_symbols = eris.unpersist(eris.unperm, persisted)
  for k,v in pairs(new_symbols) do
    _G[k] = v
  end
end

function __z8_strlen(s)
    return #string.gsub(s, '[\128-\255]', 'XX')
end

-- Stubs for unimplemented functions
local function stub(s)
    return function(a) __stub(s.."("..(a and '"'..tostr(a)..'"' or "")..")") end
end
save = stub("save")
info = stub("info")
abort = stub("abort")
folder = stub("folder")
resume = stub("resume")
reboot = stub("reboot")
dir = stub("dir")
ls = dir

function flip()
    _update_buttons()
    yield()
end

-- Load a cart from file or URL
function load(arg)
    local success, msg

    success, msg = __load(arg), ""

    
    --if success then
    --    print('ok')
    --else
    --    color(14)
    --    print('failed')
    --    local x,y = cursor()
    --    cursor(0, y)
    --    print(msg)
    --end
    
end

--todo: make this bettter/verify the list
function __is_api(funcname)
local picofuncnames = {
    -- standard lua functions
    assert = 1, getmetatable = 1, inext = 1, next = 1, ipairs = 1, pairs = 1, rawequal = 1,
    rawlen = 1, rawget = 1, rawset = 1, setmetatable = 1, type = 1, pack = 1, unpack = 1,
    load = 1, print = 1, select = 1, tostring = 1,
    -- pico-8 math
    max = 1, min = 1, mid = 1, ceil = 1, flr = 1, cos = 1, sin = 1, atan2 = 1, sqrt = 1,
    abs = 1, sgn = 1, band = 1, bor = 1, bxor = 1, bnot = 1, shl = 1, shr = 1, lshr = 1,
    rotl = 1, rotr = 1, tostr = 1, tonum = 1, srand = 1, rnd = 1, ord = 1, chr = 1,
    split = 1,
    -- pico-8 system
    run = 1, reload = 1, reset = 1, dget = 1, dset = 1, peek = 1, peek2 = 1, peek4 = 1,
    poke = 1, poke2 = 1, poke4 = 1, memcpy = 1, memset = 1, stat = 1, printh = 1, extcmd = 1,
    -- pico-8 input
    _update_buttons = 1, btn = 1, btnp = 1,
    -- pico-8 graphics
    cursor = 1, camera = 1, circ = 1, circfill = 1,
    clip = 1, cls = 1, color = 1, fillp = 1, fget = 1, fset = 1, line = 1, map = 1, mget = 1,
    mset = 1, oval = 1, ovalfill = 1, pal = 1, palt = 1, pget = 1, pset = 1, rect = 1, rectfill = 1, rrect = 1, rrectfill = 1,
    serial = 1, sget = 1, sset = 1, spr = 1, sspr = 1, tline = 1,
    -- pico-8 audio
    music = 1, sfx = 1,
    -- pico-8 time
    time = 1, t = 1,
    -- pico-8 coroutines
    cocreate = 1, coresume = 1, costatus = 1, yield = 1, trace = 1, stop = 1,
    -- pico-8 table helpers
    count = 1, add = 1, sub = 1, foreach = 1, all = 1, del = 1, deli = 1,
    -- pico-8 cart data
    dget = 1, dset = 1, cartdata = 1,
    -- pico-8 cart management
    load = 1, save = 1, info = 1, abort = 1, folder = 1,
    resume = 1, reboot = 1, dir = 1, ls = 1, flip = 1, mapdraw = 1, menuitem = 1,
    cstore = 1, _set_fps = 1,
    -- fake-08 internal
    __ispaused = 1, __resetcart = 1, __loaddefaultcart = 1, __loadsettingscart = 1,
    __listcarts = 1, __listdirs = 1, __cd = 1, __pwd = 1,
    __getbioserror = 1, __getsetting = 1, __setsetting = 1, __loadlabel = 1
 }
 return picofuncnames[funcname] == 1
end

-- Missing PICO-8 API functions: holdframe, exit
-- Missing editor/system functions: import, export, cd, mkdir, help,
--   splore, login, backup, install_games, install_demos, keyconfig, shutdown, scoresub
-- Internal PICO-8 functions: _pausemenu, _update_framerate,
--   __flip, __trace, __type, _map_display, _mark_cpu, _set_mainloop_exists, _startframe,
--   _menuitem, _get_menu_item_selected, set_draw_slice

function create_sandbox()
    local t = {}
    for k,v in pairs(_ENV) do
        if __is_api(k) then
            t[k] = v
        end
    end
    -- Button emoji variables (must be copied to sandbox for carts to use btn(❎) etc)
    t["⬅️"] = 0
    t["➡️"] = 1
    t["⬆️"] = 2
    t["⬇️"] = 3
    t["🅾️"] = 4
    t["❎"] = 5
    -- Fill pattern emoji variables
    t["█"] = 0
    t["▒"] = 0x5a5a.8
    t["🐱"] = 0x511f.8
    t["░"] = 0x7d7d.8
    t["✽"] = 0xb81d.8
    t["●"] = 0xf99f.8
    t["♥"] = 0x51bf.8
    t["☉"] = 0xb5bf.8
    t["웃"] = 0x999f.8
    t["⌂"] = 0xb11f.8
    t["😐"] = 0xa0e0.8
    t["♪"] = 0x9b3f.8
    t["◆"] = 0xb1bf.8
    t["…"] = 0xf5ff.8
    t["★"] = 0xb15f.8
    t["⧗"] = 0x1b1f.8
    t["ˇ"] = 0xf5bf.8
    t["∧"] = 0x7adf.8
    t["▤"] = 0x0f0f.8
    t["▥"] = 0x5555.8
    return t;
end

__cart_sandbox = nil



-- copied directly from zepto 8 for the time being

--
-- Utility functions
--
function __z8_reset_state()
    -- From the PICO-8 documentation:
    -- “The draw state is reset each time a program is run. This is equivalent to calling:
    -- clip() camera() pal() color(6)”
    -- Note from Sam: color() is actually short for color(6)
    -- Note from Sam: also add fillp() here.
    clip() camera() pal() color() fillp()
end

function __z8_reset_cartdata()
    __cartdata(nil)
end

function __z8_run_cart(cart_code)
    -- Glue code that manages the game loop after the cart's code runs.
    local glue_code = [[--
        if (_init) _init()
        if _update or _update60 or _draw then
            while true do
                if _update60 then
                    _update_buttons()
                    _update60()
                else
                    yield() -- yield each other frame for 30fps
                    _update_buttons()
                    if (_update) _update()
                end
                if _draw then
                    _draw()
                    flip()
                else
                    yield()
                end
            end
        end
    ]]

    __z8_loop = cocreate(function()
        -- Memory should be reset to default by the VM before this call
        -- First reload cart into memory
        reload()

        __z8_reset_state()
        __z8_reset_cartdata()

        -- Load cart and run the user-provided functions. Note that if the
        -- cart code returns before the end, our added code will not be
        -- executed, and nothing will work. This is also PICO-8's behaviour.
        -- The code has to be appended as a string because the functions
        -- may be stored in local variables.
        -- __z8_load_code points to lua's load function https://www.lua.org/manual/5.2/manual.html#pdf-load

        if __cart_sandbox ~= nil then
            __cart_sandbox = nil
        end

        __cart_sandbox = create_sandbox()
        local code, ex = __z8_load_code(cart_code..glue_code, nil, nil,
                                        __cart_sandbox)

        if not code then
            color(14) print('syntax error')
            color(6) print(ex)
            error()
        end

        -- Mark that a cart is now running (used for pause menu logic)
        __z8_cart_running = true

        -- Run cart code
        code()
    end)
end

-- FIXME: this function is quite a mess
function __z8_tick()
    if (costatus(__z8_loop) == "dead") return -1

    -- Check if pause menu is active
    if __ispaused() then
        _update_buttons()
        __f08_menu_update()
        __f08_menu_draw()
        return 0
    end

    ret, err = coresume(__z8_loop)

    -- XXX: test eris persistence
    __z8_persist_delay += 1
    if __z8_persist_delay > 30 and btnp(13) then
        __z8_persist_delay = 0
        if backup then
            __z8_loop = unpersist(backup)
        else
            backup = persist(__z8_loop)
        end
    end

    if __z8_stopped then
        __z8_stopped = false -- FIXME: what now?
    elseif not ret then
        printh(tostr(err))
        -- Return to menu on error
        __loaddefaultcart()
        return -1
    end
    return 0
end


--
-- Splash sequence
--

local function print_clear(s)
    -- empty next line
    local y, c = @0x5f27, color(0)
    rectfill(0, y, 127, y + 5)
    color(c)
    print(s)
end

local function do_command(cmd)
    local c = color()
    if string.match(cmd, '^ *$') then
        -- empty line
    elseif string.match(cmd, '^ *run *$') then
        run()
    elseif string.match(cmd, '^ *load[ |(]') then
        load(string.gsub(cmd, '^ *load *', ''))
    elseif cmd == 'help' then
        color(12)
        print_clear('') print_clear('commands') print_clear('')
        color(6)
        print_clear('load <filename>')
        print_clear('run')
        print_clear('')
    else
        color(14)
        print_clear('syntax error')
    end
    color(c)
end

function __z8_shell()
    -- activate project
    poke(0x5f2d, 1)
    local history = {}
    local cmd, caret = "", 0
    local start_y = @0x5f27
    while true do
        local exec = false
        -- read next characters and act on them
        local chars = stat(30) and stat(31) or ""
        for n = 1, #chars do
            local c = sub(chars, n, n)
            if c == "\8" then
                if caret > 0 then
                    caret -= 1
                    cmd = sub(cmd, 0, caret)..sub(cmd, caret + 2, #cmd)
                end
            elseif c == "\x7f" then
                if caret < #cmd then
                    cmd = sub(cmd, 0, caret)..sub(cmd, caret + 2, #cmd)
                end
            elseif c == "\r" then
                exec = true
            elseif #cmd < 255 then
                cmd = sub(cmd, 0, caret)..c..sub(cmd, caret + 1, #cmd)
                caret += 1
            end
        end
        -- left/right/up/down handled by buttons instead of keys (FIXME)
        if btnp(0) then
            caret = max(caret - 1, 0)
        elseif btnp(1) then
            caret = min(caret + 1, #cmd)
        elseif btnp(2) and #history > 0 then
            if not history_pos then
                history_pos = #history + 1
            end
            if history_pos == #history + 1 then
                cmd_bak = cmd
            end
            if history_pos > 1 then
                history_pos -= 1
                cmd = history[history_pos]
                caret = #cmd
            end
        elseif btnp(3) and #history > 0 then
            if not history_pos then
                cmd, caret = "", 0
            elseif history_pos <= #history then
                history_pos += 1
                cmd = history[history_pos] or cmd_bak
                caret = #cmd
            end
        end
        -- fixme: print() behaves slightly differently when
        -- scrolling in the command prompt
        local pen = @0x5f25
        if exec then
            -- return was pressed, so print an empty line to ensure scrolling
            cursor(0, start_y)
            print('')
            start_y = @0x5f27
            do_command(cmd)
            start_y = @0x5f27
            flip()
            if (#cmd > 0 and cmd != history[#history]) add(history, cmd)
            history_pos = nil
            cmd, caret = "", 0
        else
            rectfill(0, start_y, (__z8_strlen(cmd) + 3) * 4, start_y + 5, 0)
            color(7)
            print('> ', 0, start_y, 7)
            print(cmd, 8, start_y, 7)
            -- display cursor and immediately hide it after we flip() so that it
            -- does not remain in later frames
            local on = t() * 5 % 2 > 1
            if (on) rectfill(caret * 4 + 8, start_y, caret * 4 + 11, start_y + 4, 8)
            -- Suppress pause menu if Enter key is pending (will be processed next frame)
            if stat(30) then poke(0x5f30, 1) end
            flip()
            if (on) rectfill(caret * 4 + 8, start_y, caret * 4 + 11, start_y + 4, 0)
        end
        poke(0x5f25, pen)
    end
end


--
-- Initialise the VM
--
__z8_loop = cocreate(__z8_shell)

)#";


