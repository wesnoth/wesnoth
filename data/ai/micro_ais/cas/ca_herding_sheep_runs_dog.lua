local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local function get_next_sheep(cfg)
    local sheep = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", wml.get_child(cfg, "filter_second") },
        { "filter_adjacent", { side = wesnoth.current.side, { "and", wml.get_child(cfg, "filter") } } }
    }
    return sheep[1]
end

local ca_herding_sheep_runs_dog = {}

function ca_herding_sheep_runs_dog:evaluation(cfg)
    -- Any sheep with moves left next to a dog runs away
    if get_next_sheep(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_sheep_runs_dog:execution(cfg)
    -- Simply get the first sheep, order does not matter
    local sheep = get_next_sheep(cfg)

    -- Get the first dog that the sheep is adjacent to
    local dog = wesnoth.units.find_on_map { side = wesnoth.current.side, { "and", wml.get_child(cfg, "filter") },
        { "filter_adjacent", { x = sheep.x, y = sheep.y } }
    }[1]

    local herd_loc = AH.get_named_loc_xy('herd', cfg)
    local c_x, c_y = herd_loc[1], herd_loc[2]
    -- If dog is farther from center, sheep moves in, otherwise it moves out
    local sign = 1
    if (M.distance_between(dog.x, dog.y, c_x, c_y) >= M.distance_between(sheep.x, sheep.y, c_x, c_y)) then
        sign = -1
    end
    local best_hex = AH.find_best_move(sheep, function(x, y)
        return M.distance_between(x, y, c_x, c_y) * sign
    end)

    AH.movefull_stopunit(ai, sheep, best_hex)
end

return ca_herding_sheep_runs_dog
