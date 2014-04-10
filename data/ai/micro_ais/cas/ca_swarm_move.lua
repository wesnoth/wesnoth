local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local swarm_move = {}

function swarm_move:evaluation(ai, cfg)
    local units = wesnoth.get_units { side = wesnoth.current.side }
    for i,u in ipairs(units) do
        if (u.moves > 0) then return cfg.ca_score end
    end

    return 0
end

function swarm_move:execution(ai, cfg)
    local enemy_distance = cfg.enemy_distance or 5
    local vision_distance = cfg.vision_distance or 12

    -- If no close enemies, swarm will move semi-randomly, staying close together, but away from enemies
    local all_units = wesnoth.get_units { side = wesnoth.current.side }
    local units, units_no_moves = {}, {}
    for i,u in ipairs(all_units) do
        if (u.moves > 0) then
            table.insert(units, u)
        else
            table.insert(units_no_moves, u)
        end
    end

    local enemies = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }
    --print('#units, #units_no_moves, #enemies', #units, #units_no_moves, #enemies)

    -- pick one unit at random
    local unit = units[math.random(#units)]

    -- Find best place for that unit to move to
    local best_hex = AH.find_best_move(unit, function(x, y)
        local rating = 0

        -- Only units within 'vision_distance' count for rejoining
        local close_units_no_moves = {}
        for i,u in ipairs(units_no_moves) do
            if (H.distance_between(unit.x, unit.y, u.x, u.y) <= vision_distance) then
                table.insert(close_units_no_moves, u)
            end
        end

        -- If all units on the side have moves left, simply go to a hex far away
        if (not close_units_no_moves[1]) then
            rating = rating + H.distance_between(x, y, unit.x, unit.y)
        else  -- otherwise, minimize distance from units that have already moved
            for i,u in ipairs(close_units_no_moves) do
                rating = rating - H.distance_between(x, y, u.x, u.y)
            end
        end

        -- We also try to stay out of attack range of any enemy
        for i,e in ipairs(enemies) do
            local dist = H.distance_between(x, y, e.x, e.y)
            -- If enemy is within attack range, avoid those hexes
            if (dist < enemy_distance) then
                rating = rating - (enemy_distance - dist) * 10.
            end
        end

        return rating
    end)

    AH.movefull_stopunit(ai, unit, best_hex)
end

return swarm_move
