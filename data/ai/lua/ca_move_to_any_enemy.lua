------- Move To Any Enemy CA --------------
-- Move AI units toward any enemy on the map. This has a very low CA score and
-- only kicks in when the AI would do nothing else. It prevents the AI from
-- being inactive on maps without enemy leaders and villages.
-- It has a very simple algorithm that does well enough in many cases, but should
-- be considered a fall-back option. If more complex behavior is desired, use
-- the move-to-targets CA and customize it with [goal] tags. That works even
-- on maps without enemy leaders and villages.

local AH = wesnoth.require "ai/lua/ai_helper.lua"

local MTAE_unit, MTAE_destination

local ca_move_to_any_enemy = {}

function ca_move_to_any_enemy:evaluation(cfg, data, filter_own)
    local start_time, ca_name = wesnoth.ms_since_init() / 1000., 'move_to_any_enemy'
    if AH.print_eval() then AH.print_ts('     - Evaluating move_to_any_enemy CA:') end

    local units = AH.get_units_with_moves({
        side = wesnoth.current.side,
        canrecruit = 'no',
        { "and", filter_own }
    }, true)

    if (not units[1]) then
        -- No units with moves left
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local avoid_map = AH.get_avoid_map(ai, nil, true)

    -- In principle we don't even need to pass avoid_map here, as the loop below also
    -- checks this, but we might as well eliminate unreachable enemies right away
    local enemies = AH.get_attackable_enemies({}, wesnoth.current.side, { avoid_map = avoid_map })
    if (not enemies[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    -- Presort unit/enemy pairs by distance in hexes to avoid unnecessary path finding.
    -- As AI units are dealt with one by one below, this only needs to be done individually
    -- for each AI unit, not for the whole set of pairs.
    local unit_distances = {}
    for i,u in ipairs(units) do
        local enemy_distances = {}
        for j,e in ipairs(enemies) do
            local dist = wesnoth.map.distance_between(u, e)
            table.insert(enemy_distances, { x = e.x, y = e.y, dist = dist })
        end
        -- Sort enemies by distance for this AI unit
        table.sort(enemy_distances, function(a, b) return (a.dist < b.dist) end)

        table.insert(unit_distances, { x = u.x, y = u.y, enemy_distances = enemy_distances })
    end
    -- Finally, sort AI units by distance to their closest enemies
    table.sort(unit_distances, function(a, b) return (a.enemy_distances[1].dist < b.enemy_distances[1].dist) end)

    local unit, destination
    -- Find the closest enemies to the sorted AI units.  This does not need to find the absolutely best
    -- combination, such as which hex adjacent to the unit is best, as these enemies are out
    -- of reach of the AI (otherwise other CAs would have triggered previously).
    -- This moves one AI unit at a time in order to speed up evaluation.
    for i,ud in ipairs(unit_distances) do
        local u = wesnoth.units.get(ud.x, ud.y)
        local best_cost, best_path = AH.no_path, nil
        for j,ed in ipairs(ud.enemy_distances) do
            -- Only do path finding if the distance to the enemy in less than the current best path cost,
            -- otherwise it is impossible to find a shorter path.
            if (not best_path) or (ed.dist < best_cost) then
                if (not avoid_map:get(ed.x, ed.y)) then
                    local path, cost = AH.find_path_with_avoid(u, ed.x, ed.y, avoid_map, { ignore_enemies = true })
                    if (cost < best_cost) then
                        best_cost = cost
                        best_path = path
                    end
                end
            end
        end

        if best_path then
            MTAE_destination = AH.next_hop(u, nil, nil, { path = best_path, avoid_map = avoid_map })
            if (MTAE_destination[1] ~= u.x) or (MTAE_destination[2] ~= u.y) then
                MTAE_unit = u
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 1000
            end
        end
    end

    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 0
end

function ca_move_to_any_enemy:execution(cfg, data)
    if AH.print_exec() then AH.print_ts('   Executing move_to_any_enemy CA') end
    AH.robust_move_and_attack(ai, MTAE_unit, MTAE_destination)
    MTAE_unit, MTAE_destination = nil,nil
end

return ca_move_to_any_enemy
