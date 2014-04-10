local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local herding_area = wesnoth.require "ai/micro_ais/cas/ca_herding_f_herding_area.lua"

local function get_next_sheep(cfg)
    local sheep = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter_second }
    }
    return sheep[1]
end

local ca_herding_sheep_move = {}

function ca_herding_sheep_move:evaluation(ai, cfg)
   -- If nothing else is to be done, the sheep do a random move
    if get_next_sheep(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_sheep_move:execution(ai, cfg)
    -- We simply move the first sheep first, the order does not matter
    local sheep = get_next_sheep(cfg)

    local reach_map = AH.get_reachable_unocc(sheep)
    -- Exclude those that are next to a dog
    reach_map:iter( function(x, y, v)
        for xa, ya in H.adjacent_tiles(x, y) do
            local dog = wesnoth.get_unit(xa, ya)
            if dog and (wesnoth.match_unit(dog, cfg.filter)) then
                reach_map:remove(x, y)
            end
        end
    end)
    --AH.put_labels(reach_map)

    -- Choose one of the possible locations  at random (or the current location, if no move possible)
    local x, y = sheep.x, sheep.y
    if (reach_map:size() > 0) then
        x, y = AH.LS_random_hex(reach_map)
        --print('Sheep -> :', x, y)
    end

    -- If this move remains within herding area or dogs have no moves left, or sheep doesn't move
    -- make it a full move, otherwise partial move
    local herding_area = herding_area(cfg)
    local dogs = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter }
    }
    if herding_area:get(x, y) or (not dogs[1]) or ((x == sheep.x) and (y == sheep.y)) then
        AH.movefull_stopunit(ai, sheep, x, y)
    else
        AH.checked_move(ai, sheep, x, y)
    end
end

return ca_herding_sheep_move
