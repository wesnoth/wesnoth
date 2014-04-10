local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local herding_area = wesnoth.require "ai/micro_ais/cas/ca_herding_f_herding_area.lua"

local function get_dogs(cfg)
    local dogs = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter }
    }
    return dogs
end

local function get_sheep_to_herd(cfg)
    local all_sheep = wesnoth.get_units {
        side = wesnoth.current.side,
        { "and", cfg.filter_second },
        { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter} } } } }
    }
    local sheep_to_herd = {}
    local herding_area = herding_area(cfg)
    for i,s in ipairs(all_sheep) do
        if (not herding_area:get(s.x, s.y)) then table.insert(sheep_to_herd, s) end
    end
    return sheep_to_herd
end

local ca_herding_herd_sheep = {}

function ca_herding_herd_sheep:evaluation(ai, cfg)
    -- If dogs have moves left, and there is a sheep with moves left outside the
    -- herding area, chase it back
    if get_dogs(cfg)[1] then
        if get_sheep_to_herd(cfg)[1] then return cfg.ca_score end
    end
    return 0
end

function ca_herding_herd_sheep:execution(ai, cfg)
    local dogs = get_dogs(cfg)
    local sheep_to_herd = get_sheep_to_herd(cfg)

    local max_rating, best_dog, best_hex = -9e99, {}, {}
    local c_x, c_y = cfg.herd_x, cfg.herd_y
    for i,s in ipairs(sheep_to_herd) do
        -- This is the rating that depends only on the sheep's position
        -- Farthest sheep goes first
        local sheep_rating = H.distance_between(c_x, c_y, s.x, s.y) / 10.
        -- Sheep with no movement left gets big hit
        if (s.moves == 0) then sheep_rating = sheep_rating - 100. end

        for i,d in ipairs(dogs) do
            local reach_map = AH.get_reachable_unocc(d)
            reach_map:iter( function(x, y, v)
                local dist = H.distance_between(x, y, s.x, s.y)
                local rating = sheep_rating - dist
                -- Needs to be on "far side" of sheep, wrt center for adjacent hexes
                if (H.distance_between(x, y, c_x, c_y) <= H.distance_between(s.x, s.y, c_x, c_y))
                    and (dist == 1)
                then rating = rating - 1000 end
                -- And the closer dog goes first (so that it might be able to chase another sheep afterward)
                rating = rating - H.distance_between(x, y, d.x, d.y) / 100.
                -- Finally, prefer to stay on path, if possible
                if (wesnoth.match_location(x, y, cfg.filter_location) ) then rating = rating + 0.001 end

                reach_map:insert(x, y, rating)

                if (rating > max_rating) then
                    max_rating = rating
                    best_dog = d
                    best_hex = { x, y }
                end
            end)
            --AH.put_labels(reach_map)
            --W.message{ speaker = d.id, message = 'My turn' }
         end
    end

    -- Now we move the best dog
    -- If it's already in the best position, we just take moves away from it
    -- (to avoid black-listing of CA, in the worst case)
    if (best_hex[1] == best_dog.x) and (best_hex[2] == best_dog.y) then
        AH.checked_stopunit_moves(ai, best_dog)
    else
        --print('Dog moving to herd sheep')
        AH.checked_move(ai, best_dog, best_hex[1], best_hex[2])  -- partial move only
    end
end

return ca_herding_herd_sheep
