------- Move To Any Enemy CA --------------
-- Move AI units toward any enemy on the map. This has a very low CA score and
-- only kicks in when the AI would do nothing else. It prevents the AI from
-- being inactive on maps without enemy leaders and villages.

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

    local unit, destination
    -- Find first unit that can reach a hex adjacent to an enemy, and find closest enemy of those reachable.
    -- This does not need to find the absolutely best combination, close to that is good enough.
    for i,u in ipairs(units) do
        local best_cost, best_path, best_enemy = AH.no_path, nil, nil
        for j,e in ipairs(enemies) do
            -- We only need to look at adjacent hexes. And we don't worry whether they
            -- are occupied by other enemies. If that is the case, no path will be found,
            -- but one of those enemies will later be found as potential target.
            for xa,ya in wesnoth.current.map:iter_adjacent(e) do
                if (not avoid_map:get(xa, ya)) then
                    local path, cost = AH.find_path_with_avoid(u, xa, ya, avoid_map)
                    if (cost < best_cost) then
                        best_cost = cost
                        best_path = path
                        best_enemy = e
                        -- We also don't care if this is the closest adjacent hex, just pick the first found
                        break
                    end
                end
            end
        end

        if best_enemy then
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
