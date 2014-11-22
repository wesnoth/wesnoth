-- ilua.lua
-- A more friendly Lua interactive prompt
-- doesn't need '='
-- will try to print out tables recursively, subject to the pretty_print_limit value.
-- Steve Donovan, 2007
-- Adapted by iceiceice for wesnoth, 2014

local pretty_print_limit = 20
local max_depth = 7
local table_clever = true
local prompt = '> '
local verbose = false
local strict = true
-- suppress strict warnings
_ = true

-- imported global functions
local sub = string.sub
local match = string.match
local find = string.find
local push = table.insert
local pop = table.remove
local append = table.insert
local concat = table.concat
local floor = math.floor

local savef
local collisions = {}
local G_LIB = {}
local declared = {}
local line_handler_fn, global_handler_fn
local print_handlers = {}

ilua = {}
local num_prec
local num_all

local jstack = {}

local function join(tbl,delim,limit,depth)
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
    -- this is a hack to work out if a table is 'list-like' or 'map-like'
    -- you can switch it off with ilua.table_options {clever = false}
    local is_list
    if table_clever then
        local index1 = n > 0 and tbl[1]
        local index2 = n > 1 and tbl[2]
        is_list = index1 and index2
    end
    if is_list then
        for i,v in ipairs(tbl) do
            res = res..delim..val2str(v)
            k = k + 1
            if k > limit then
                res = res.." ... "
                break
            end
        end
    else
        for key,v in pairs(tbl) do
            if type(key) == 'number' then
                key = '['..tostring(key)..']'
            else
                key = tostring(key)
            end
            res = res..delim..key..'='..val2str(v)
            k = k + 1
            if k > limit then
                res = res.." ... "
                break
            end            
        end
    end
    pop(jstack)
    return sub(res,2)
end


function val2str(val)
    local tp = type(val)
    if print_handlers[tp] then
        local s = print_handlers[tp](val)
        return s or '?'
    end
    if tp == 'function' then
        return tostring(val)
    elseif tp == 'table' then
        if val.__tostring  then
            return tostring(val)
        else
            return '{'..join(val,',')..'}'
        end
    elseif tp == 'string' then
        return "'"..val.."'"
    elseif tp == 'number' then
        -- we try only to apply floating-point precision for numbers deemed to be floating-point,
        -- unless the 3rd arg to precision() is true.
        if num_prec and (num_all or floor(val) ~= val) then
            return num_prec:format(val)
        else
            return tostring(val)
        end
    else
        return tostring(val)
    end
end

function _pretty_print(...)
    arg = table.pack(...)
    for i,val in ipairs(arg) do
        print(val2str(val))
    end
    _G['_'] = arg[1]
end

