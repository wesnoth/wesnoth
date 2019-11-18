-- ilua.lua
-- A more friendly Lua interactive prompt
-- doesn't need '='
-- will try to print out tables recursively, subject to the pretty_print_limit value.
-- Steve Donovan, 2007
-- Adapted by iceiceice for wesnoth, 2014
-- Retrieved from: http://lua-users.org/files/wiki_insecure/users/steved/ilua.lua
-- Note: This file is loaded automatically by the engine.

local pretty_print_limit = 20
local max_depth = 7

-- imported global functions
local sub = string.sub
local push = table.insert
local pop = table.remove
local pack = table.pack

local declared = {}

local jstack = {}

local ilua = { strict = true }

function ilua.join(tbl,delim,limit,depth)
    if not limit then limit = pretty_print_limit end
    if not depth then depth = max_depth end
    local n = #tbl
    local res = ''
    local k = 0
    -- very important to avoid disgracing ourselves with circular referencs...
    if #jstack > depth then
        return "..."
    end
    for i,t in ipairs(jstack) do
        if tbl == t then
            return "<self>"
        end
    end
    push(jstack,tbl)
    if n > 0 then
        for i,v in ipairs(tbl) do
            res = res..delim..ilua.val2str(v)
            k = k + 1
            if k > limit then
                res = res.." ... "
                break
            end
        end
    end
    for key,v in pairs(tbl) do
        if type(key) == 'number' then
            if key <= n then goto continue end
            key = '['..tostring(key)..']'
        else
            key = tostring(key)
        end
        res = res..delim..key..'='..ilua.val2str(v)
        k = k + 1
        if k > limit then
            res = res.." ... "
            break
        end
        ::continue::
    end
    pop(jstack)
    return sub(res,2)
end

-- Need to save this locally because the debug module will be disabled right after this file is loaded
-- The or clause is so that it doesn't totally break if someone decides to force-reload ilua during a session.
local rawgetmetatable = debug.getmetatable or getmetatable
local function getmetatable(t)
    return rawgetmetatable(t) or {}
end

function ilua.val2str(val)
    local tp = type(val)
    if tp == 'function' then
        return tostring(val)
    elseif tp == 'table' then
        if getmetatable(val).__tostring  then
            return tostring(val)
        else
            return '{'..ilua.join(val,',')..'}'
        end
    elseif tp == 'userdata' then
        local mt = getmetatable(val)
        if mt.__len and mt.__pairs then
            return '{'..ilua.join(val,',')..'}'
        else
            return tostring(val)
        end
    elseif tp == 'string' then
        return "'"..val.."'"
    elseif tp == 'number' then
	-- removed numeric precision features, but we might actually want these... might put them back
        return tostring(val)
    else
        return tostring(val)
    end
end

function ilua._pretty_print(...)
    local arg = pack(...)
    for i,val in ipairs(arg) do
        print(ilua.val2str(val))
    end
end

--
-- strict.lua
-- checks uses of undeclared global variables
-- All global variables must be 'declared' through a regular assignment
-- (even assigning nil will do) in a main chunk before being used
-- anywhere.
--
function ilua.set_strict()
    local mt = getmetatable(_G)
    if mt == nil then
        mt = {}
        setmetatable(_G, mt)
    end

    local function what ()
        local d = debug.getinfo(3, "S")
        return d and d.what or "C"
    end

    mt.__newindex = function (t, n, v)
        declared[n] = true
        rawset(t, n, v)
    end

    mt.__index = function (t, n)
        if not declared[n] and ilua.strict and what() ~= "C" then
            error("variable '"..n.."' must be assigned before being used", 2)
        end
        return rawget(t, n)
    end
end

return ilua
