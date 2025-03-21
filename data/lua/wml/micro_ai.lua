local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

---@class ca_info
---@field ca_id string The base ID used for the candidate action
---@field location string Path to the Lua file that defines the candidate action
---@field score number The maximum score the candidate action can return

---@type table<string, fun(cfg:WML):string[],string[],ca_info>
wesnoth.micro_ais = {}

-- Load all default MicroAIs
wesnoth.require("ai/micro_ais/mai-defs")

function wesnoth.wml_actions.micro_ai(cfg)
    local CA_path = 'ai/micro_ais/cas/'

    cfg = wml.shallow_parsed(cfg)

    -- Check that the required common keys are all present and set correctly
    if (not cfg.ai_type) then wml.error("[micro_ai] is missing required ai_type= key") end
    if (not cfg.side) then wml.error("[micro_ai] is missing required side= key") end
    if string.find(cfg.side, ',') or string.find(cfg.side, '-') then
        for side in stringx.iter_ranges(cfg.side) do
            cfg.side = tonumber(side)
            wesnoth.wml_actions.micro_ai(cfg)
        end
        return
    end
    if (not wesnoth.sides[cfg.side]) then
        wesnoth.interface.add_chat_message("Warning", "[micro_ai] uses side=" .. cfg.side .. ": side does not exist")
        return
    end
    if (not cfg.action) then wml.error("[micro_ai] is missing required action= key") end

    if (cfg.action ~= 'add') and (cfg.action ~= 'delete') and (cfg.action ~= 'change') then
        wml.error("[micro_ai] unknown value for action=. Allowed values: add, delete or change")
    end

    -- Set up the configuration tables for the different Micro AIs
    if wesnoth.micro_ais[cfg.ai_type] == nil then
        wml.error("unknown value for ai_type= in [micro_ai]")
    end

    local required_keys, optional_keys, CA_parms = wesnoth.micro_ais[cfg.ai_type](cfg)

    -- Fixup any relative CA paths
    for i,v in ipairs(CA_parms) do
        if v.location and v.location:find('~') ~= 1  then
            v.location = CA_path .. v.location
        end
    end

    MAIH.micro_ai_setup(cfg, CA_parms, required_keys, optional_keys)
end
