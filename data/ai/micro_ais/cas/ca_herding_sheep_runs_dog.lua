local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_next_sheep(cfg)
    local sheep = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter_second },
        { "filter_adjacent", { side = wesnoth.current.side, { "and", cfg.filter } } }
    }
    return sheep[1]
end

local ca_herding_sheep_runs_dog = {}

function ca_herding_sheep_runs_dog:evaluation(ai, cfg)
    -- Any sheep with moves left next to a dog runs away
    if get_next_sheep(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_sheep_runs_dog:execution(ai, cfg)
    -- Simply get the first sheep, order does not matter
    local sheep = get_next_sheep(cfg)

    -- Get the first dog it is adjacent to
    local dog = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
        { "filter_adjacent", { x = sheep.x, y = sheep.y } }
    }[1]

    local c_x, c_y = cfg.herd_x, cfg.herd_y
    -- If dog is farther from center, sheep moves in, otherwise it moves out
    local sign = 1
    if (H.distance_between(dog.x, dog.y, c_x, c_y) >= H.distance_between(sheep.x, sheep.y, c_x, c_y)) then
        sign = -1
    end
    local best_hex = AH.find_best_move(sheep, function(x, y)
        return H.distance_between(x, y, c_x, c_y) * sign
    end)

    AH.movefull_stopunit(ai, sheep, best_hex)
end

return ca_herding_sheep_runs_dog
