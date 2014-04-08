local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local ca_herding_dog_move = {}

function ca_herding_dog_move:evaluation(ai, cfg)
    -- As a final step, any dog not adjacent to a sheep moves within herding_perimeter
    local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
        formula = '$this_unit.moves > 0',
        { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter_second} } } } }
    }
    if dogs[1] then return cfg.ca_score end
    return 0
end

function ca_herding_dog_move:execution(ai, cfg)
    -- We simply move the first dog first
    local dog = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
        formula = '$this_unit.moves > 0',
        { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter_second} } } } }
    }[1]

    local herding_perimeter = LS.of_pairs(wesnoth.get_locations(cfg.filter_location))
    --AH.put_labels(herding_perimeter)

    -- Find average distance of herding_perimeter from center
    local av_dist = 0
    herding_perimeter:iter( function(x, y, v)
        av_dist = av_dist + H.distance_between(x, y, cfg.herd_x, cfg.herd_y)
    end)
    av_dist = av_dist / herding_perimeter:size()
    --print('Average distance:', av_dist)

    local best_hex = AH.find_best_move(dog, function(x, y)
        -- Prefer hexes on herding_perimeter, or close to it
        -- Or, if dog cannot get there, prefer to be av_dist from the center
        local rating = 0
        if herding_perimeter:get(x, y) then
            rating = rating + 1000 + math.random(99) / 100.
        else
            rating = rating - math.abs(H.distance_between(x, y, cfg.herd_x, cfg.herd_y) - av_dist) + math.random(99) / 100.
        end

        return rating
    end)

    --print('Dog wandering')
    AH.movefull_stopunit(ai, dog, best_hex)
end

return ca_herding_dog_move
