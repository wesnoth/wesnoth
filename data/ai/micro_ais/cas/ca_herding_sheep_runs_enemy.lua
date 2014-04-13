local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_next_sheep(cfg)
    local sheep = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter_second },
        { "filter_location",
            {
                { "filter", { { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } } }
                },
                radius = (cfg.attention_distance or 8)
            }
        }
    }
    return sheep[1]
end

local ca_herding_sheep_runs_enemy = {}

function ca_herding_sheep_runs_enemy:evaluation(ai, cfg)
    -- Sheep runs from any enemy within attention_distance hexes (after the dogs have moved in)
    if get_next_sheep(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_sheep_runs_enemy:execution(ai, cfg)
    -- Simply start with the first sheep, order does not matter
    local sheep = get_next_sheep(cfg)

    local enemies = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
        { "filter_location", { x = sheep.x, y = sheep.y , radius = (cfg.attention_distance or 8) } }
    }

    -- Maximize distance between sheep and enemies
    local best_hex = AH.find_best_move(sheep, function(x, y)
        local rating = 0
        for _,enemy in ipairs(enemies) do
            rating = rating + H.distance_between(x, y, enemy.x, enemy.y)
        end
        return rating
    end)

    AH.movefull_stopunit(ai, sheep, best_hex)
end

return ca_herding_sheep_runs_enemy
