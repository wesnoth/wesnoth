local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"
local M = wesnoth.map

local function get_dog(cfg)
    local dogs = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter") },
        { "not", { { "filter_adjacent", { side = wesnoth.current.side, { "and", H.get_child(cfg, "filter_second") } } } } }
    }
    return dogs[1]
end

local ca_herding_dog_move = {}

function ca_herding_dog_move:evaluation(cfg)
    -- As a final step, any dog not adjacent to a sheep moves within herding_perimeter
    if get_dog(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_dog_move:execution(cfg)
    -- We simply move the first dog first, order does not matter
    local dog = get_dog(cfg)
    local herding_perimeter = LS.of_pairs(wesnoth.get_locations(H.get_child(cfg, "filter_location")))

    -- Find average distance of herding_perimeter from center
    local av_dist = 0
    herding_perimeter:iter( function(x, y, v)
        av_dist = av_dist + M.distance_between(x, y, cfg.herd_x, cfg.herd_y)
    end)
    av_dist = av_dist / herding_perimeter:size()

    local best_hex = AH.find_best_move(dog, function(x, y)
        -- Prefer hexes on herding_perimeter, or close to it
        -- Or, if dog cannot get there, prefer to be av_dist from the center
        local rating = 0
        if herding_perimeter:get(x, y) then
            rating = rating + 1000 + math.random(99) / 100.
        else
            rating = rating
                - math.abs(M.distance_between(x, y, cfg.herd_x, cfg.herd_y) - av_dist)
                + math.random(99) / 100.
        end

        return rating
    end)

    AH.movefull_stopunit(ai, dog, best_hex)
end

return ca_herding_dog_move
