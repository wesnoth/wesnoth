local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local ca_swarm_move = {}

function ca_swarm_move:evaluation(cfg)
    local units = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        wml.tag["and"] ( wml.get_child(cfg, "filter") )
    }
    for _,unit in ipairs(units) do
        if (unit.moves > 0) then return cfg.ca_score end
    end

    return 0
end

function ca_swarm_move:execution(cfg)
    local enemy_distance = cfg.enemy_distance or 5
    local vision_distance = cfg.vision_distance or 12

    -- If no close enemies, swarm will move semi-randomly, staying close together, but away from enemies
    local all_units = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        wml.tag["and"] ( wml.get_child(cfg, "filter") )
    }

    local units, units_no_moves = {}, {}
    for _,unit in ipairs(all_units) do
        if (unit.moves > 0) then
            table.insert(units, unit)
        else
            table.insert(units_no_moves, unit)
        end
    end

    local enemies = AH.get_attackable_enemies()

    local avoid_map = AH.get_avoid_map(ai, wml.get_child(cfg, "avoid"), true)

    -- Pick one unit at random, swarm does not move systematically
    local unit = units[math.random(#units)]

    -- Find best place for that unit to move to
    local best_hex = AH.find_best_move(unit, function(x, y)
        local rating = 0

        -- Only units within 'vision_distance' count for rejoining
        local close_units_no_moves = {}
        for _,unit_noMP in ipairs(units_no_moves) do
            if (M.distance_between(unit.x, unit.y, unit_noMP.x, unit_noMP.y) <= vision_distance) then
                table.insert(close_units_no_moves, unit_noMP)
            end
        end

        -- If all units on the side have moves left, simply go to a hex far away
        if (not close_units_no_moves[1]) then
            rating = rating + M.distance_between(x, y, unit.x, unit.y)
        else  -- Otherwise, minimize distance from units that have already moved
            for _,close_unit in ipairs(close_units_no_moves) do
                rating = rating - M.distance_between(x, y, close_unit.x, close_unit.y)
            end
        end

        -- We also try to stay out of attack range of any enemy
        for _,enemy in ipairs(enemies) do
            local dist = M.distance_between(x, y, enemy.x, enemy.y)
            if (dist < enemy_distance) then
                rating = rating - (enemy_distance - dist) * 10.
            end
        end

        return rating
    end, { avoid_map = avoid_map })

    AH.movefull_stopunit(ai, unit, best_hex or { unit.x, unit.y })
end

return ca_swarm_move
