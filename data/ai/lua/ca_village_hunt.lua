------- Village Hunt CA --------------
-- Give extra priority to seeking villages if we have less than our share
-- our share is defined as being slightly more than the total/the number of sides

local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_village_hunt = {}

function ca_village_hunt:evaluation(cfg, data)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'village_hunt'
    if AH.print_eval() then AH.print_ts('     - Evaluating village_hunt CA:') end

    local villages = wesnoth.get_villages()

    if not villages[1] then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local my_villages = wesnoth.get_villages { owner_side = wesnoth.current.side }

    if #my_villages > #villages / #wesnoth.sides then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local allied_villages = wesnoth.get_villages { {"filter_owner", { {"ally_of", { side = wesnoth.current.side }} }} }
    if #allied_villages == #villages then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local units = AH.get_units_with_moves {
        side = wesnoth.current.side,
        canrecruit = false
    }

    if not units[1] then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 30000
end

function ca_village_hunt:execution(cfg, data)
    local unit = AH.get_units_with_moves({
        side = wesnoth.current.side,
        canrecruit = false
    })[1]

    if AH.print_exec() then AH.print_ts('   Executing village_hunt CA') end

    local villages = wesnoth.get_villages()
    local best_cost, target = AH.no_path
    for i,v in ipairs(villages) do
        if not wesnoth.match_location(v[1], v[2], { {"filter_owner", { {"ally_of", { side = wesnoth.current.side }} }} }) then
            local path, cost = wesnoth.find_path(unit, v[1], v[2], { ignore_units = true, max_cost = best_cost })
            if cost < best_cost then
                target = v
                best_cost = cost
            end
        end
    end

    if target then
        local x, y = wesnoth.find_vacant_tile(target[1], target[2], unit)
        local dest = AH.next_hop(unit, x, y)
        AH.checked_move(ai, unit, dest[1], dest[2])
    end
end

return ca_village_hunt
