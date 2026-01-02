pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
-- test chr with large number of arguments
-- this tests the fix for chr buffer size limit

results = {}

function _init()
  -- write test pattern to memory at 0x1000
  for i=0,999 do
    poke(0x1000+i, i%256)
  end
  
  -- read 1000 bytes and convert to string using chr
  local str = chr(peek(0x1000, 1000))
  
  -- verify the string length
  if #str == 1000 then
    add(results, {true, "chr handled 1000 args"})
    add(results, {true, "length: "..#str})
  else
    add(results, {false, "expected 1000, got "..#str})
  end
  
  -- verify first and last bytes
  local first = ord(str, 1)
  local last = ord(str, 1000)
  
  if first == 0 then
    add(results, {true, "first byte: "..first})
  else
    add(results, {false, "first byte: "..tostr(first).." (expected 0)"})
  end
  
  if last == 231 then
    add(results, {true, "last byte: "..last})
  else
    add(results, {false, "last byte: "..tostr(last).." (expected 231)"})
  end
end

function _draw()
  cls()
  print("chr large args test", 0, 0, 7)
  print("-------------------", 0, 8, 6)
  
  for i, r in ipairs(results) do
    local pass, msg = r[1], r[2]
    local col = pass and 11 or 8
    local prefix = pass and "pass: " or "fail: "
    print(prefix..msg, 0, 8 + i*8, col)
  end
end
