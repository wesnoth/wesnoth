local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local function get_next_sheep_enemies(cfg)
    local sheep = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter_second") }
    }
    if (not sheep[1]) then return end

    local enemies = AH.get_attackable_enemies()
    if (not enemies[1]) then return end

    local attention_distance = cfg.attention_distance or 8

    -- Simply return the first sheep, order does not matter
    for _,single_sheep in ipairs(sheep) do
        local close_enemies = {}
        for _,enemy in ipairs(enemies) do
            if (M.distance_between(single_sheep.x, single_sheep.y, enemy.x, enemy.y) <= attention_distance) then
                table.insert(close_enemies, enemy)
            end
        end
        if close_enemies[1] then
            return single_sheep, enemies
        end
    end
end

local ca_herding_sheep_runs_enemy = {}

function ca_herding_sheep_runs_enemy:evaluation(cfg)
    -- Sheep runs from any enemy within attention_distance hexes (after the dogs have moved in)
    if get_next_sheep_enemies(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_sheep_runs_enemy:execution(cfg)
    local sheep, close_enemies = get_next_sheep_enemies(cfg)

    -- Maximize distance between sheep and enemies
    local best_hex = AH.find_best_move(sheep, function(x, y)
        local rating = 0
        for _,enemy in ipairs(close_enemies) do
            rating = rating + M.distance_between(x, y, enemy.x, enemy.y)
        end
        return rating
    end)

    AH.movefull_stopunit(ai, sheep, best_hex)
end

return ca_herding_sheep_runs_enemy
