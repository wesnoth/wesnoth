local H = wesnoth.require "helper"
local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

wesnoth.micro_ais = {}

-- Load all default MicroAIs
wesnoth.require("ai/micro_ais/mai-defs")

function wesnoth.wml_actions.micro_ai(cfg)
    local CA_path = 'ai/micro_ais/cas/'

    cfg = cfg.__shallow_parsed

    -- Check that the required common keys are all present and set correctly
    if (not cfg.ai_type) then H.wml_error("[micro_ai] is missing required ai_type= key") end
    if (not cfg.side) then H.wml_error("[micro_ai] is missing required side= key") end
    if (not wesnoth.sides[cfg.side]) then
        wesnoth.message("Warning", "[micro_ai] uses side=" .. cfg.side .. ": side does not exist")
        return
    end
    if (not cfg.action) then H.wml_error("[micro_ai] is missing required action= key") end

    if (cfg.action ~= 'add') and (cfg.action ~= 'delete') and (cfg.action ~= 'change') then
        H.wml_error("[micro_ai] unknown value for action=. Allowed values: add, delete or change")
    end

    -- Set up the configuration tables for the different Micro AIs
    if wesnoth.micro_ais[cfg.ai_type] == nil then
        H.wml_error("unknown value for ai_type= in [micro_ai]")
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
