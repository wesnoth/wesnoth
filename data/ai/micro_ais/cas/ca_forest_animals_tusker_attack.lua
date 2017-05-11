local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local function get_tuskers(cfg)
    local tuskers = AH.get_units_with_moves {
        side = wesnoth.current.side,
        type = cfg.tusker_type
    }
    return tuskers
end

local function get_adjacent_enemies(cfg)
    local adjacent_enemies = AH.get_attackable_enemies {
        { "filter_adjacent", { side = wesnoth.current.side, type = cfg.tusklet_type } }
    }
    return adjacent_enemies
end

local ca_forest_animals_tusker_attack = {}

function ca_forest_animals_tusker_attack:evaluation(cfg)
    -- Check whether there is an enemy next to a tusklet and attack it ("protective parents" AI)

    if (not cfg.tusker_type) or (not cfg.tusklet_type) then return 0 end
    if (not get_tuskers(cfg)[1]) then return 0 end
    if (not get_adjacent_enemies(cfg)[1]) then return 0 end
    return cfg.ca_score
end

function ca_forest_animals_tusker_attack:execution(cfg)
    local tuskers = get_tuskers(cfg)
    local adjacent_enemies = get_adjacent_enemies(cfg)

    -- Find the closest enemy to any tusker
    local min_dist, attacker, target = 9e99
    for _,tusker in ipairs(tuskers) do
        for _,enemy in ipairs(adjacent_enemies) do
            local dist = M.distance_between(tusker.x, tusker.y, enemy.x, enemy.y)
            if (dist < min_dist) then
                min_dist, attacker, target = dist, tusker, enemy
            end
        end
    end

    -- The tusker moves as close to enemy as possible
    -- Closeness to tusklets is secondary criterion
    local adj_tusklets = wesnoth.get_units {
        side = wesnoth.current.side,
        type = cfg.tusklet_type,
        { "filter_adjacent", { id = target.id } }
    }

    local best_hex = AH.find_best_move(attacker, function(x, y)
        local rating = -M.distance_between(x, y, target.x, target.y)
        for _,tusklet in ipairs(adj_tusklets) do
            if (M.distance_between(x, y, tusklet.x, tusklet.y) == 1) then rating = rating + 0.1 end
        end

        return rating
    end)

    AH.robust_move_and_attack(ai, attacker, best_hex, target)
end

return ca_forest_animals_tusker_attack
