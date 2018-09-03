------- Place Healers CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local HS = wesnoth.require "ai/micro_ais/cas/ca_healer_move.lua"

local ca_place_healers = {}

function ca_place_healers:evaluation(cfg, data)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'place_healers'
    if AH.print_eval() then AH.print_ts('     - Evaluating place_healers CA:') end

    if HS:evaluation(cfg, data) > 0 then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 96000
    end
    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 0
end

function ca_place_healers:execution(cfg, data)
    if AH.print_exec(cfg, data) then AH.print_ts('   Executing place_healers CA') end
    HS:execution()
end

return ca_place_healers
