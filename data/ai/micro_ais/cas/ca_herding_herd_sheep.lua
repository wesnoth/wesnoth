local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local herding_area = wesnoth.require "ai/micro_ais/cas/ca_herding_f_herding_area.lua"

local function get_dogs(cfg)
    local dogs = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", wml.get_child(cfg, "filter") }
    }
    return dogs
end

local function get_sheep_to_herd(cfg)
    local all_sheep = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        { "and", wml.get_child(cfg, "filter_second") },
        { "not", { { "filter_adjacent", { side = wesnoth.current.side, { "and", wml.get_child(cfg, "filter") } } } } }
    }

    local sheep_to_herd = {}
    local herding_area = herding_area(cfg)
    for _,single_sheep in ipairs(all_sheep) do
        if (not herding_area:get(single_sheep.x, single_sheep.y)) then
            table.insert(sheep_to_herd, single_sheep)
        end
    end
    return sheep_to_herd
end

local ca_herding_herd_sheep = {}

function ca_herding_herd_sheep:evaluation(cfg)
    -- If dogs have moves left, and there is a sheep with moves left outside the
    -- herding area, chase it back
    if (not get_dogs(cfg)[1]) then return 0 end
    if (not get_sheep_to_herd(cfg)[1]) then return 0 end
    return cfg.ca_score
end

function ca_herding_herd_sheep:execution(cfg)
    local dogs = get_dogs(cfg)
    local sheep_to_herd = get_sheep_to_herd(cfg)

    local max_rating, best_dog, best_hex = - math.huge
    local herd_loc = AH.get_named_loc_xy('herd', cfg)
    local c_x, c_y = herd_loc[1], herd_loc[2]
    for _,single_sheep in ipairs(sheep_to_herd) do
        -- Farthest sheep goes first
        local sheep_rating = M.distance_between(c_x, c_y, single_sheep.x, single_sheep.y) / 10.
        -- Sheep with no movement left gets big hit
        if (single_sheep.moves == 0) then sheep_rating = sheep_rating - 100. end

        for _,dog in ipairs(dogs) do
            local reach_map = AH.get_reachable_unocc(dog)
            reach_map:iter( function(x, y, v)
                local dist = M.distance_between(x, y, single_sheep.x, single_sheep.y)
                local rating = sheep_rating - dist
                -- Needs to be on "far side" of sheep, wrt center for adjacent hexes
                if (M.distance_between(x, y, c_x, c_y) <= M.distance_between(single_sheep.x, single_sheep.y, c_x, c_y))
                    and (dist == 1)
                then rating = rating - 1000 end
                -- And the closer dog goes first (so that it might be able to chase another sheep afterward)
                rating = rating - M.distance_between(x, y, dog.x, dog.y) / 100.
                -- Finally, prefer to stay on path, if possible
                if (wesnoth.map.matches(x, y, wml.get_child(cfg, "filter_location")) ) then rating = rating + 0.001 end

                reach_map:insert(x, y, rating)

                if (rating > max_rating) then
                    max_rating, best_dog, best_hex = rating, dog, { x, y }
                end
            end)
         end
    end

    AH.robust_move_and_attack(ai, best_dog, best_hex, nil, { partial_move = true })
end

return ca_herding_herd_sheep
