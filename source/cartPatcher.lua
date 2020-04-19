function patchLuaForP8(lua)
    --from LovePotion: https://github.com/gamax92/picolove/blob/master/cart.lua (zlib license)
    --it may be possible to do this kind of pattern matching with another library, or use z8Lua
    --but for now I have lua 5.3 working so I'm going with this since I know it works
    
    -- patch the lua
    lua=lua:gsub("!=", "~=").."\n"
    -- rewrite shorthand if statements eg. if (not b) i=1 j=2
    lua=lua:gsub("if%s*(%b())%s*([^\n]*)\n", function(a, b)
        local nl=a:find('\n', nil, true)
        local th=b:find('%f[%w]then%f[%W]')
        local an=b:find('%f[%w]and%f[%W]')
        local o=b:find('%f[%w]or%f[%W]')
        local ce=b:find('--', nil, true)
        if not (nl or th or an or o) then
            if ce then
                local c, t=b:match("(.-)(%s-%-%-.*)")
                return "if "..a:sub(2, -2).." then "..c.." end"..t.."\n"
            else
                return "if "..a:sub(2, -2).." then "..b.." end\n"
            end
        end
    end)
    -- rewrite assignment operators
    lua=lua:gsub("(%S+)%s*([%+-%*/%%])=", "%1 = %1 %2 ")
    -- convert binary literals to hex literals
    lua=lua:gsub("([^%w_])0[bB]([01.]+)", function(a, b)
        local p1, p2=b, ""
        if b:find('.', nil, true) then
            p1, p2=b:match("(.-)%.(.*)")
        end
        -- pad to 4 characters
        p2=p2..string.rep("0", 3-((#p2-1)%4))
        p1, p2=tonumber(p1, 2), tonumber(p2, 2)
        if p1 and p2 then
            return string.format("%s0x%x.%x", a, p1, p2)
        end
    end)

    return lua;
end
