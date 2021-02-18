------- Village Hunt CA --------------
-- Give extra priority to seeking villages if we have less than our share.
-- Our share is defined as being slightly more than the total/the number of sides,
-- but only in the area not prohibited by an [avoid] directive.

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"

local VH_unit, VH_dst = {}, {}

local ca_village_hunt = {}

function ca_village_hunt:evaluation(cfg, data, filter_own)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'village_hunt'
    if AH.print_eval() then AH.print_ts('     - Evaluating village_hunt CA:') end

    local avoid_map = LS.of_pairs(ai.aspects.avoid)

    local all_villages, villages = wesnoth.map.find{gives_income = true}, {}
    for _,village in ipairs(all_villages) do
        if (not avoid_map:get(village[1], village[2])) then
            table.insert(villages, village)
        end
    end

    if (not villages[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local n_my_villages, n_allied_villages = 0, 0
    for _,village in ipairs(villages) do
        local owner = wesnoth.map.get_owner(village) or -1
        if (owner == wesnoth.current.side) then
            n_my_villages = n_my_villages + 1
        end
        if (owner ~= -1) and (not wesnoth.sides.is_enemy(owner, wesnoth.current.side)) then
            n_allied_villages = n_allied_villages + 1
        end
    end

    if (n_my_villages > #villages / #wesnoth.sides) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    if (n_allied_villages == #villages) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local units = AH.get_units_with_moves({
        side = wesnoth.current.side,
        canrecruit = 'no',
        { "and", filter_own }
    }, true)

    if (not units[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local avoid_map = AH.get_avoid_map(ai, nil, true)

    VH_unit = nil
    for _,unit in ipairs(units) do
        local best_cost = AH.no_path
        for i,v in ipairs(villages) do
            if not wesnoth.map.matches(v, { {"filter_owner", { {"ally_of", { side = wesnoth.current.side }} }} }) then
                local path, cost = AH.find_path_with_avoid(unit, v[1], v[2], avoid_map)
                if (cost < best_cost) then
                    local dst = AH.next_hop(unit, nil, nil, { path = path, avoid_map = avoid_map })
                    if (dst[1] ~= unit.x) or (dst[2] ~= unit.y) then
                        best_cost = cost
                        VH_unit = unit
                        VH_dst = dst
                    end
                end
            end
        end
    end

    if (not VH_unit) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 30000
end

function ca_village_hunt:execution(cfg, data, filter_own)
    if AH.print_exec() then AH.print_ts('   Executing village_hunt CA') end

    AH.robust_move_and_attack(ai, VH_unit, VH_dst)
    VH_unit, VH_dst = nil, nil
end

return ca_village_hunt
