local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local ca_big_animals = {}

function ca_big_animals:evaluation(ai, cfg)
    local units = wesnoth.get_units {
        side = wesnoth.current.side,
        { "and" , cfg.filter },
        formula = '$this_unit.moves > 0'
    }

    if units[1] then return cfg.ca_score end
    return 0
end

function ca_big_animals:execution(ai, cfg)
    -- Big animals just move toward goal that gets set occasionally
    -- Avoid the other big animals (bears, yetis, spiders) and the dogs, otherwise attack whatever is in their range
    -- The only difference in behavior is the area in which the units move

    local units = wesnoth.get_units {
        side = wesnoth.current.side,
        { "and" , cfg.filter },
        formula = '$this_unit.moves > 0'
    }
    local avoid = LS.of_pairs(wesnoth.get_locations { radius = 1,
        { "filter", { { "and", cfg.avoid_unit },
            { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
        } }
    })
    --AH.put_labels(avoid)

    for i,unit in ipairs(units) do
        -- Unit gets a new goal if none exist or on any move with 10% random chance
        local r = math.random(10)
        if (not unit.variables.goal_x) or (r == 1) then
            local locs = AH.get_passable_locations(cfg.filter_location or {})
            local rand = math.random(#locs)
            --print(type, ': #locs', #locs, rand)
            unit.variables.goal_x, unit.variables.goal_y = locs[rand][1], locs[rand][2]
        end
        --print('Big animal goto: ', type, unit.variables.goal_x, unit.variables.goal_y, r)

        -- hexes the unit can reach
        local reach_map = AH.get_reachable_unocc(unit)
        local wander_terrain = cfg.filter_location_wander or {}
        reach_map:iter( function(x, y, v)
            -- Remove tiles that do not comform to the wander terrain filter
            if (not wesnoth.match_location(x, y, wander_terrain) ) then
                reach_map:remove(x, y)
            end
        end)

        -- Now find the one of these hexes that is closest to the goal
        local max_rating, best_hex = -9e99, {}
        reach_map:iter( function(x, y, v)
            -- Distance from goal is first rating
            local rating = - H.distance_between(x, y, unit.variables.goal_x, unit.variables.goal_y)

            -- Proximity to an enemy unit is a plus
            local enemy_hp = 500
            for xa, ya in H.adjacent_tiles(x, y) do
                local enemy = wesnoth.get_unit(xa, ya)
                if enemy and (enemy.side ~= wesnoth.current.side) then
                    if (enemy.hitpoints < enemy_hp) then enemy_hp = enemy.hitpoints end
                end
            end
            rating = rating + 500 - enemy_hp  -- prefer attack on weakest enemy

            -- However, hexes that enemy bears, yetis and spiders can reach get a massive negative hit
            -- meaning that they will only ever be chosen if there's no way around them
            if avoid:get(x, y) then rating = rating - 1000 end

            reach_map:insert(x, y, rating)
            if (rating > max_rating) then
                max_rating, best_hex = rating, { x, y }
            end
        end)
        --print('  best_hex: ', best_hex[1], best_hex[2])
        --AH.put_labels(reach_map)

        if (best_hex[1] ~= unit.x) or (best_hex[2] ~= unit.y) then
            AH.checked_move(ai, unit, best_hex[1], best_hex[2])  -- partial move only
        else  -- If animal did not move, we need to stop it (also delete the goal)
            ai.stopunit_moves(unit)
            unit.variables.goal_x = nil
            unit.variables.goal_y = nil
        end

        -- Or if this gets the unit to the goal, we also delete the goal
        if (unit.x == unit.variables.goal_x) and (unit.y == unit.variables.goal_y) then
            unit.variables.goal_x = nil
            unit.variables.goal_y = nil
        end

        -- Finally, if the unit ended up next to enemies, attack the weakest of those
        local min_hp, target = 9e99, {}
        for x, y in H.adjacent_tiles(unit.x, unit.y) do
            local enemy = wesnoth.get_unit(x, y)
            if enemy and (enemy.side ~= wesnoth.current.side) then
                if (enemy.hitpoints < min_hp) then
                    min_hp, target = enemy.hitpoints, enemy
                end
            end
        end
        if target.id then
            AH.checked_attack(ai, unit, target)
        end

    end
end

return ca_big_animals
