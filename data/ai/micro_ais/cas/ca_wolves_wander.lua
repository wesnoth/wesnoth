local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "lua/location_set.lua"

local function get_wolves(cfg)
    local wolves = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter }
    }
    return wolves
end

local ca_wolves_wander = {}

function ca_wolves_wander:evaluation(ai, cfg)
    -- When there's no prey left, the wolves wander and regroup
    if get_wolves(cfg)[1] then return cfg.ca_score end
    return 0
end

function ca_wolves_wander:execution(ai, cfg)
    local wolves = get_wolves(cfg)

    -- Number of wolves that can reach each hex
    local reach_map = LS.create()
    for _,wolf in ipairs(wolves) do
        local r = AH.get_reachable_unocc(wolf)
        reach_map:union_merge(r, function(x, y, v1, v2) return (v1 or 0) + (v2 or 0) end)
    end

    local avoid_units = wesnoth.get_units { type = cfg.avoid_type,
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } }
    }
    local avoid_map = BC.get_attack_map(avoid_units).units

    local max_rating, goal_hex = -9e99
    reach_map:iter( function (x, y, v)
        local rating = v + math.random(99)/100.
        if avoid_map:get(x, y) then rating = rating - 1000 end

        if (rating > max_rating) then
            max_rating, goal_hex = rating, { x, y }
        end

        reach_map:insert(x, y, rating)
    end)

    for _,wolf in ipairs(wolves) do
        -- For each wolf, we need to check that goal hex is reachable, and out of harm's way
        local best_hex = AH.find_best_move(wolf, function(x, y)
            local rating = - H.distance_between(x, y, goal_hex[1], goal_hex[2])
            if avoid_map:get(x, y) then rating = rating - 1000 end
            return rating
        end)

        AH.movefull_stopunit(ai, wolf, best_hex)
    end
end

return ca_wolves_wander
