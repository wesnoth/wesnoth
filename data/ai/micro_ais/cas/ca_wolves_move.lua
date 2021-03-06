local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local M = wesnoth.map

local function get_wolves(cfg)
    local wolves = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", wml.get_child(cfg, "filter") }
    }
    return wolves
end

local function get_prey(cfg)
    -- Note: we cannot pass wml.get_child() directly to AH.get_attackable_enemies()
    -- as the former returns two values and the latter takes optional arguments
    local filter_second = wml.get_child(cfg, "filter_second")
    local prey = AH.get_attackable_enemies(filter_second)
    return prey
end

local ca_wolves_move = {}

function ca_wolves_move:evaluation(cfg)
    if (not get_wolves(cfg)[1]) then return 0 end

    local avoid_map = AH.get_avoid_map(ai, nil, true)
    local prey = get_prey(cfg)
    local prey_found = false
    for _,prey_unit in ipairs(prey) do
        if (not avoid_map:get(prey_unit.x, prey_unit.y)) then
            prey_found = true
            break
        end
    end
    if (not prey_found) then return 0 end

    return cfg.ca_score
end

function ca_wolves_move:execution(cfg)
    local wolves = get_wolves(cfg)
    local prey = get_prey(cfg)

    -- Only default AI [avoid] tag makes sense for the wolves since attacks are done by RCA AI
    local avoid_map = AH.get_avoid_map(ai, nil, true)

    local avoid_units = AH.get_attackable_enemies({ type = cfg.avoid_type })
    local avoid_enemies_map = BC.get_attack_map(avoid_units).units

    -- Find prey that is closest to the wolves
    local min_dist, target = math.huge
    for _,prey_unit in ipairs(prey) do
        if (not avoid_map:get(prey_unit.x, prey_unit.y)) then
            local dist = 0
            for _,wolf in ipairs(wolves) do
                dist = dist + M.distance_between(wolf.x, wolf.y, prey_unit.x, prey_unit.y)
            end
            if (dist < min_dist) then
                min_dist, target = dist, prey_unit
            end
        end
    end

    -- Now sort wolf from farthest to closest
    table.sort(wolves, function(a, b)
        return M.distance_between(a.x, a.y, target.x, target.y) > M.distance_between(b.x, b.y, target.x, target.y)
    end)
    
    -- First wolf moves toward target, but tries to stay away from map edges
    local wolf1 = AH.find_best_move(wolves[1], function(x, y)
        local dist_1t = M.distance_between(x, y, target.x, target.y)
        local rating = - dist_1t
        local map = wesnoth.current.map
        if (x <= 5) then rating = rating - (6 - x) / 1.4 end
        if (y <= 5) then rating = rating - (6 - y) / 1.4 end
        if (map.playable_width - x <= 5) then rating = rating - (6 - (map.playable_width - x)) / 1.4 end
        if (map.playable_height - y <= 5) then rating = rating - (6 - (map.playable_height - y)) / 1.4 end

       -- Hexes that avoid_type units can reach get a massive penalty
       if avoid_enemies_map:get(x, y) then rating = rating - 1000 end

       return rating
    end, { avoid_map = avoid_map })

    local move_result = AH.movefull_stopunit(ai, wolves[1], wolf1 or { wolf1.x, wolf1.y })
    -- If the wolf was ambushed, return and reconsider; also if an event removed a wolf
    if (AH.is_incomplete_move(move_result)) then return end
    for _,check_wolf in ipairs(wolves) do
        if (not check_wolf) or (not check_wolf.valid) then return end
    end

    for i = 2,#wolves do
        move = AH.find_best_move(wolves[i], function(x,y)
            local rating = 0

            -- We ideally want wolves to be 2-3 hexes from each other
            -- but this requirement gets weaker and weaker with increasing wolf number
            for j = 1,i-1 do
                local dst = M.distance_between(x, y, wolves[j].x, wolves[j].y)
                rating = rating - (dst - 2.7 * j)^2 / j
            end

            -- Same distance from Wolf 1 and target for all the wolves
            local dist_t = M.distance_between(x, y, target.x, target.y)
            local dist_1t = M.distance_between(wolf1[1], wolf1[2], target.x, target.y)
            rating = rating - (dist_t - dist_1t)^2

            -- Hexes that avoid_type units can reach get a massive penalty
            if avoid_enemies_map:get(x, y) then rating = rating - 1000 end

            return rating
        end, { avoid_map = avoid_map })

        local move_result = AH.movefull_stopunit(ai, wolves[i], move or { wolves[i].x, wolves[i].y })
        -- If the wolf was ambushed, return and reconsider; also if an event removed a wolf
        if (AH.is_incomplete_move(move_result)) then return end
        for _,check_wolf in ipairs(wolves) do
            if (not check_wolf) or (not check_wolf.valid) then return end
        end
    end
end

return ca_wolves_move
