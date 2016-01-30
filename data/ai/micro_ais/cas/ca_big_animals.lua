local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local function get_big_animals(cfg)
    local big_animals = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and" , cfg.filter }
    }
    return big_animals
end

local ca_big_animals = {}

function ca_big_animals:evaluation(ai, cfg)
    if get_big_animals(cfg)[1] then return cfg.ca_score end
    return 0
end

function ca_big_animals:execution(ai, cfg)
    -- Big animals just move toward a goal that gets (re)set occasionally
    -- and attack whatever is in their range (except for some units that they avoid)

    local big_animals = get_big_animals(cfg)
    local avoid_map = LS.create()
    if cfg.avoid_unit then
        avoid_map = LS.of_pairs(wesnoth.get_locations { radius = 1,
            { "filter", { { "and", cfg.avoid_unit },
                { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } }
            } }
        })
    end

    for _,unit in ipairs(big_animals) do
        local goal = MAIUV.get_mai_unit_variables(unit, cfg.ai_id)

        -- Unit gets a new goal if none is set or on any move with a 10% random chance
        local r = math.random(10)
        if (not goal.goal_x) or (r == 1) then
            local locs = AH.get_passable_locations(cfg.filter_location or {})
            local rand = math.random(#locs)

            goal.goal_x, goal.goal_y = locs[rand][1], locs[rand][2]
            MAIUV.set_mai_unit_variables(unit, cfg.ai_id, goal)
        end

        local reach_map = AH.get_reachable_unocc(unit)
        local wander_terrain = cfg.filter_location_wander or {}
        reach_map:iter( function(x, y, v)
            -- Remove tiles that do not comform to the wander terrain filter
            if (not wesnoth.match_location(x, y, wander_terrain)) then
                reach_map:remove(x, y)
            end
        end)

        -- Now find the one of these hexes that is closest to the goal
        local max_rating, best_hex = -9e99
        reach_map:iter( function(x, y, v)
            local rating = - H.distance_between(x, y, goal.goal_x, goal.goal_y)

            -- Proximity to an enemy unit is a plus
            local enemy_hp = 500
            for xa,ya in H.adjacent_tiles(x, y) do
                local enemy = wesnoth.get_unit(xa, ya)
                if enemy and wesnoth.is_enemy(enemy.side, wesnoth.current.side) then
                    if (enemy.hitpoints < enemy_hp) then enemy_hp = enemy.hitpoints end
                end
            end
            rating = rating + 500 - enemy_hp  -- Prefer attack on weakest enemy

            -- Hexes reachable by units to be be avoided get a massive negative hit
            if avoid_map:get(x, y) then rating = rating - 1000 end

            if (rating > max_rating) then
                max_rating, best_hex = rating, { x, y }
            end
        end)

        if (best_hex[1] ~= unit.x) or (best_hex[2] ~= unit.y) then
            AH.checked_move(ai, unit, best_hex[1], best_hex[2])  -- Partial move only
            if (not unit) or (not unit.valid) then return end
        else  -- If unit did not move, we need to stop it (also delete the goal)
            AH.checked_stopunit_moves(ai, unit)
            if (not unit) or (not unit.valid) then return end
            MAIUV.delete_mai_unit_variables(unit, cfg.ai_id)
        end

        -- If this gets the unit to the goal, we also delete the goal
        if (unit.x == goal.goal_x) and (unit.y == goal.goal_y) then
            MAIUV.delete_mai_unit_variables(unit, cfg.ai_id)
        end

        -- Finally, if the unit ended up next to enemies, attack the weakest of those
        local min_hp, target = 9e99
        for xa,ya in H.adjacent_tiles(unit.x, unit.y) do
            local enemy = wesnoth.get_unit(xa, ya)
            if enemy and wesnoth.is_enemy(enemy.side, wesnoth.current.side) then
                if (enemy.hitpoints < min_hp) then
                    min_hp, target = enemy.hitpoints, enemy
                end
            end
        end

        if target then
            AH.checked_attack(ai, unit, target)
        end
    end
end

return ca_big_animals
