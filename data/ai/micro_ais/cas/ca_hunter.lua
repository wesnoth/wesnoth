local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.dofile "ai/micro_ais/micro_ai_unit_variables.lua"

local ca_hunter = {}

local function hunter_attack_weakest_adj_enemy(ai, unit)
    -- Attack the enemy with the fewest hitpoints adjacent to 'unit', if there is one
    -- Returns status of the attack:
    --   'attacked': if a unit was attacked
    --   'killed': if a unit was killed
    --   'no_attack': if no unit was attacked

    -- First check that the unit exists and has attacks left
    if (not unit.valid) then return 'no_attack' end
    if (unit.attacks_left == 0) then return 'no_attack' end

    local min_hp, target = 9e99, {}
    for x, y in H.adjacent_tiles(unit.x, unit.y) do
        local enemy = wesnoth.get_unit(x, y)
        if enemy and wesnoth.is_enemy(enemy.side, wesnoth.current.side) then
            if (enemy.hitpoints < min_hp) then
                min_hp, target = enemy.hitpoints, enemy
            end
        end
    end

    if target.id then
        --W.message { speaker = unit.id, message = 'Attacking weakest adjacent enemy' }
        AH.checked_attack(ai, unit, target)
        if target.valid then
            return 'attacked'
        else
            return 'killed'
        end
    end

    return 'no_attack'
end

function ca_hunter:evaluation(ai, cfg)
    local filter = cfg.filter or { id = cfg.id }
    local unit = wesnoth.get_units({
        side = wesnoth.current.side,
        { "and", filter },
        formula = '$this_unit.moves > 0' }
    )[1]

    if unit then return cfg.ca_score end
    return 0
end

-- cfg parameters: id, hunting_ground, home_x, home_y, rest_turns, show_messages
function ca_hunter:execution(ai, cfg)
    -- Unit with the given ID goes on a hunt, doing a random wander in area given by
    -- hunting_ground, then retreats to
    -- position given by 'home_x,home_y' for 'rest_turns' turns, or until fully healed

    local filter = cfg.filter or { id = cfg.id }
    local unit = wesnoth.get_units({
        side = wesnoth.current.side,
        { "and", filter },
        formula = '$this_unit.moves > 0' }
    )[1]

    -- If hunting_status is not set for the unit -> default behavior -> hunting
    local hunter_vars = MAIUV.get_mai_unit_variables(unit, cfg.ai_id)
    if (not hunter_vars.hunting_status) then
        -- Unit gets a new goal if none exist or on any move with 10% random chance
        local r = math.random(10)
        if (not hunter_vars.goal_x) or (r <= 1) then
            -- 'locs' includes border hexes, but that does not matter here
            locs = AH.get_passable_locations((cfg.filter_location or {}), unit)
            local rand = math.random(#locs)
            --print('#locs', #locs, rand)
            hunter_vars.goal_x, hunter_vars.goal_y = locs[rand][1], locs[rand][2]
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
        end
        --print('Hunter goto: ', hunter_vars.goal_x, hunter_vars.goal_y, r)

        -- Hexes the unit can reach
        local reach_map = AH.get_reachable_unocc(unit)

        -- Now find the one of these hexes that is closest to the goal
        local max_rating, best_hex = -9e99, {}
        reach_map:iter( function(x, y, v)
            -- Distance from goal is first rating
            local rating = - H.distance_between(x, y, hunter_vars.goal_x, hunter_vars.goal_y)

            -- Proximity to an enemy unit is a plus
            local enemy_hp = 500
            for xa, ya in H.adjacent_tiles(x, y) do
                local enemy = wesnoth.get_unit(xa, ya)
                if enemy and wesnoth.is_enemy(enemy.side, wesnoth.current.side) then
                    if (enemy.hitpoints < enemy_hp) then enemy_hp = enemy.hitpoints end
                end
            end
            rating = rating + 500 - enemy_hp  -- prefer attack on weakest enemy

            reach_map:insert(x, y, rating)
            if (rating > max_rating) then
                max_rating, best_hex = rating, { x, y }
            end
        end)
        --print('  best_hex: ', best_hex[1], best_hex[2])
        --AH.put_labels(reach_map)

        if (best_hex[1] ~= unit.x) or (best_hex[2] ~= unit.y) then
            AH.checked_move(ai, unit, best_hex[1], best_hex[2])  -- partial move only
            if (not unit) or (not unit.valid) then return end
        else  -- If hunter did not move, we need to stop it (also delete the goal)
            AH.checked_stopunit_moves(ai, unit)
            if (not unit) or (not unit.valid) then return end
            hunter_vars.goal_x, hunter_vars.goal_y = nil, nil
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
        end

        -- Or if this gets the unit to the goal, we also delete the goal
        if (unit.x == hunter_vars.goal_x) and (unit.y == hunter_vars.goal_y) then
            hunter_vars.goal_x, hunter_vars.goal_y = nil, nil
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
        end

        -- Finally, if the unit ended up next to enemies, attack the weakest of those
        local attack_status = hunter_attack_weakest_adj_enemy(ai, unit)

        -- If the enemy was killed, hunter returns home
        if unit.valid and (attack_status == 'killed') then
            hunter_vars.goal_x, hunter_vars.goal_y = nil, nil
            hunter_vars.hunting_status = 'return'
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
            if cfg.show_messages then
                W.message { speaker = unit.id, message = 'Now that I have eaten, I will go back home.' }
            end
        end

        -- At this point, issue a 'return', so that no other action takes place this turn
        return
    end

    -- If we got here, this means the unit is either returning, or resting
    if (hunter_vars.hunting_status == 'return') then
        goto_x, goto_y = wesnoth.find_vacant_tile(cfg.home_x, cfg.home_y)
        --print('Go home:', home_x, home_y, goto_x, goto_y)

        local next_hop = AH.next_hop(unit, goto_x, goto_y)
        if next_hop then
            --print(next_hop[1], next_hop[2])
            AH.movefull_stopunit(ai, unit, next_hop)
            if (not unit) or (not unit.valid) then return end

            -- If there's an enemy on the 'home' hex and we got right next to it, attack that enemy
            if (H.distance_between(cfg.home_x, cfg.home_y, next_hop[1], next_hop[2]) == 1) then
                local enemy = wesnoth.get_unit(cfg.home_x, cfg.home_y)
                if enemy and wesnoth.is_enemy(enemy.side, unit.side) then
                    if cfg.show_messages then
                        W.message { speaker = unit.id, message = 'Get out of my home!' }
                    end
                    AH.checked_attack(ai, unit, enemy)
                    if (not unit) or (not unit.valid) then return end
                end
            end
        end

        -- We also attack the weakest adjacent enemy, if still possible
        hunter_attack_weakest_adj_enemy(ai, unit)
        if (not unit) or (not unit.valid) then return end

        -- If the unit got home, start the resting counter
        if (unit.x == cfg.home_x) and (unit.y == cfg.home_y) then
            hunter_vars.hunting_status = 'resting'
            hunter_vars.resting_until = wesnoth.current.turn + (cfg.rest_turns or 3)
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
            if cfg.show_messages then
                W.message { speaker = unit.id, message = 'I made it home - resting now until the end of Turn ' .. hunter_vars.resting_until .. ' or until fully healed.' }
            end
        end

        -- At this point, issue a 'return', so that no other action takes place this turn
        return
    end

    -- If we got here, the only remaining action is resting
    if (hunter_vars.hunting_status == 'resting') then
        -- So all we need to do is take moves away from the unit
        AH.checked_stopunit_moves(ai, unit)
        if (not unit) or (not unit.valid) then return end

        -- However, we do also attack the weakest adjacent enemy, if still possible
        hunter_attack_weakest_adj_enemy(ai, unit)
        if (not unit) or (not unit.valid) then return end

        -- If this is the last turn of resting, we also remove the status and turn variable
        if (unit.hitpoints >= unit.max_hitpoints) and (hunter_vars.resting_until <= wesnoth.current.turn) then
            hunter_vars.hunting_status = nil
            hunter_vars.resting_until = nil
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
            if cfg.show_messages then
                W.message { speaker = unit.id, message = 'I am done resting. It is time to go hunting again next turn.' }
            end
        end
        return
    end

    -- In principle we should never get here, but just in case: reset variable, so that unit goes hunting on next turn
    hunter_vars.hunting_status = nil
    MAIUV.set_mai_unit_variables(unit, cfg.ai_id, hunter_vars)
end

return ca_hunter
