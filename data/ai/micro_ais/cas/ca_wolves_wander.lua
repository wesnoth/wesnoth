local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"

local function get_wolves(cfg)
    local wolves = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter") }
    }
    return wolves
end

local ca_wolves_wander = {}

function ca_wolves_wander:evaluation(cfg)
    -- When there's no prey left, the wolves wander and regroup
    if get_wolves(cfg)[1] then return cfg.ca_score end
    return 0
end

function ca_wolves_wander:execution(cfg)
    local wolves = get_wolves(cfg)

    -- Number of wolves that can reach each hex
    local reach_map = LS.create()
    for _,wolf in ipairs(wolves) do
        local r = AH.get_reachable_unocc(wolf)
        reach_map:union_merge(r, function(x, y, v1, v2) return (v1 or 0) + (v2 or 0) end)
    end

    local avoid_units = AH.get_attackable_enemies({ type = cfg.avoid_type })
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
            local rating = -wesnoth.map.distance_between(x, y, goal_hex[1], goal_hex[2])
            if avoid_map:get(x, y) then rating = rating - 1000 end
            return rating
        end)

        local move_result = AH.movefull_stopunit(ai, wolf, best_hex)
        -- If the wolf was ambushed, return and reconsider; also if an event removed a wolf
        if (AH.is_incomplete_move(move_result)) then return end
        for _,check_wolf in ipairs(wolves) do
            if (not check_wolf) or (not check_wolf.valid) then return end
        end
    end
end

return ca_wolves_wander
