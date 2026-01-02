pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
-- test ord with nil argument
-- per PICO-8 docs: "When str is not a string, ORD returns nil"

results = {}

function _init()
  -- test ord with nil first argument
  local result = ord(nil)
  if result == nil then
    add(results, {true, "ord(nil) returns nil"})
  else
    add(results, {false, "ord(nil) returned "..tostr(result)})
  end
  
  -- test ord with nil and extra args
  local result2 = ord(nil, 1, 50)
  if result2 == nil then
    add(results, {true, "ord(nil,1,50) returns nil"})
  else
    add(results, {false, "ord(nil,1,50) returned "..tostr(result2)})
  end
  
  -- test ord with number (converts to string first)
  -- ord(123) treats it as "123", returns ord('1') = 49
  local result3 = ord(123)
  if result3 == 49 then
    add(results, {true, "ord(123) = 49"})
  else
    add(results, {false, "ord(123) returned "..tostr(result3).." (expected 49)"})
  end
  
  -- test ord with table (not a string)
  local result4 = ord({})
  if result4 == nil then
    add(results, {true, "ord({}) returns nil"})
  else
    add(results, {false, "ord({}) returned "..tostr(result4)})
  end
  
  -- verify ord still works with valid string
  local result5 = ord("hello")
  if result5 == 104 then  -- 'h' = 104
    add(results, {true, "ord('hello') = 104"})
  else
    add(results, {false, "ord('hello') returned "..tostr(result5)})
  end
end

function _draw()
  cls()
  print("ord nil arg test", 0, 0, 7)
  print("----------------", 0, 8, 6)
  
  for i, r in ipairs(results) do
    local pass, msg = r[1], r[2]
    local col = pass and 11 or 8
    local prefix = pass and "pass: " or "fail: "
    print(prefix..msg, 0, 8 + i*8, col)
  end
end
