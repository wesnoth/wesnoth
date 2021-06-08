------- Retreat CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"
local R = wesnoth.require "ai/lua/retreat.lua"

local retreat_unit, retreat_loc

local ca_retreat_injured = {}

function ca_retreat_injured:evaluation(cfg, data, filter_own)
    local start_time, ca_name = wesnoth.ms_since_init() / 1000., 'retreat_injured'
    if AH.print_eval() then AH.print_ts('     - Evaluating retreat_injured CA:') end

    if (ai.aspects.retreat_factor <= 0) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local units = AH.get_units_with_moves({
        side = wesnoth.current.side,
        { "and", filter_own }
    }, true)
    local avoid_map = LS.of_pairs(ai.aspects.avoid)
    local unit, loc = R.retreat_injured_units(units, avoid_map)
    if unit then
        retreat_unit = unit
        retreat_loc = loc

        -- First check if attacks are possible for any unit
        -- If one with > 50% chance of kill is possible, set return_value to lower than combat CA
        local attacks = ai.get_attacks()
        for i,a in ipairs(attacks) do
            if (#a.movements == 1) and (a.chance_to_kill > 0.5) then
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 95000
            end
        end
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 192000
    end
    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 0
end

function ca_retreat_injured:execution(cfg, data)
    if AH.print_exec() then AH.print_ts('   Executing retreat_injured CA') end
    AH.robust_move_and_attack(ai, retreat_unit, retreat_loc)
    retreat_unit = nil
    retreat_loc = nil
end

return ca_retreat_injured
