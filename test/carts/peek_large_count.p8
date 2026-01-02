pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
-- test peek with large count
-- tests lua_checkstack fix for returning many values

results = {}

function _init()
  -- write test pattern to memory at 0x1000
  for i=0,999 do
    poke(0x1000+i, i%256)
  end
  
  -- peek 1000 values at once
  local vals = {peek(0x1000, 1000)}
  
  -- verify count
  if #vals == 1000 then
    add(results, {true, "peek returned 1000 values"})
  else
    add(results, {false, "expected 1000, got "..#vals})
  end
  
  -- verify first value
  if vals[1] == 0 then
    add(results, {true, "first value is 0"})
  else
    add(results, {false, "first value is "..tostr(vals[1])})
  end
  
  -- verify last value (999 % 256 = 231)
  if vals[1000] == 231 then
    add(results, {true, "last value is 231"})
  else
    add(results, {false, "last value is "..tostr(vals[1000])})
  end
  
  -- verify middle value (500 % 256 = 244)
  if vals[501] == 244 then
    add(results, {true, "middle value is 244"})
  else
    add(results, {false, "middle value is "..tostr(vals[501])})
  end
end

function _draw()
  cls()
  print("peek large count test", 0, 0, 7)
  print("--------------------", 0, 8, 6)
  
  for i, r in ipairs(results) do
    local pass, msg = r[1], r[2]
    local col = pass and 11 or 8
    local prefix = pass and "pass: " or "fail: "
    print(prefix..msg, 0, 8 + i*8, col)
  end
end
