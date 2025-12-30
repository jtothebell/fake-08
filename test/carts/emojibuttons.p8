pico-8 cartridge // http://www.pico-8.com
version 29
__lua__
-- Test that emoji button variables are available in sandbox
-- These should resolve to numbers, not nil

left_val = ⬅️
right_val = ➡️
up_val = ⬆️
down_val = ⬇️
o_val = 🅾️
x_val = ❎

-- Test btn() with emoji arguments
btn_left = false
btn_o = false
btn_x = false

function _update()
    btn_left = btn(⬅️)
    btn_o = btn(🅾️)
    btn_x = btn(❎)
end

function _draw()
    cls()
end
__gfx__
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

