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

local function get_prey(cfg)
    local prey = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
        { "and", cfg.filter_second }
    }
    return prey
end

local ca_wolves_move = {}

function ca_wolves_move:evaluation(ai, cfg)
    if (not get_wolves(cfg)[1]) then return 0 end
    if (not get_prey(cfg)[1]) then return 0 end
    return cfg.ca_score
end

function ca_wolves_move:execution(ai, cfg)
    local wolves = get_wolves(cfg)
    local prey = get_prey(cfg)

    -- When wandering (later) they avoid dogs, but not here
    local avoid_units = wesnoth.get_units { type = cfg.avoid_type,
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }
    --print('#avoid_units', #avoid_units)
    -- negative hit for hexes these types of units can attack
    local avoid = BC.get_attack_map(avoid_units).units  -- max_moves=true is always set for enemy units

    -- Find prey that is closest to all 3 wolves
    local target, min_dist = {}, 9999
    for i,p in ipairs(prey) do
        local dist = 0
        for j,w in ipairs(wolves) do
            dist = dist + H.distance_between(w.x, w.y, p.x, p.y)
        end
        if (dist < min_dist) then
            min_dist, target = dist, p
        end
    end
    --print('target:', target.x, target.y, target.id)

    -- Now sort wolf from furthest to closest
    table.sort(wolves, function(a, b)
        return H.distance_between(a.x, a.y, target.x, target.y) > H.distance_between(b.x, b.y, target.x, target.y)
    end)

    -- First wolf moves toward target, but tries to stay away from map edges
    local w,h,b = wesnoth.get_map_size()
    local wolf1 = AH.find_best_move(wolves[1], function(x, y)
        local d_1t = H.distance_between(x, y, target.x, target.y)
        local rating = -d_1t
        if x <= 5 then rating = rating - (6 - x) / 1.4 end
        if y <= 5 then rating = rating - (6 - y) / 1.4 end
        if (w - x) <= 5 then rating = rating - (6 - (w - x)) / 1.4 end
        if (h - y) <= 5 then rating = rating - (6 - (h - y)) / 1.4 end

       -- Hexes that avoid_type units can reach get a massive negative hit
       -- meaning that they will only ever be chosen if there's no way around them
       if avoid:get(x, y) then rating = rating - 1000 end

       return rating
    end)
    --print('wolf 1 ->', wolves[1].x, wolves[1].y, wolf1[1], wolf1[2])
    --W.message { speaker = wolves[1].id, message = "Me first"}
    AH.movefull_stopunit(ai, wolves[1], wolf1)

    for i = 2,#wolves do
        move = AH.find_best_move(wolves[i], function(x,y)
            local rating = 0

            -- We ideally want wolves to be 2-3 hexes from each other
            -- but this requirement gets weaker and weaker with increasing wolf number
            for j = 1,i-1 do
                local dst = H.distance_between(x, y, wolves[j].x, wolves[j].y)
                rating = rating - (dst - 2.7 * j)^2 / j
            end

            -- Same distance from Wolf 1 and target for all the wolves
            local dst_t = H.distance_between(x, y, target.x, target.y)
            local dst_1t = H.distance_between(wolf1[1], wolf1[2], target.x, target.y)
            rating = rating - (dst_t - dst_1t)^2

            -- Hexes that avoid_type units can reach get a massive negative hit
            -- meaning that they will only ever be chosen if there's no way around them
            if avoid:get(x, y) then rating = rating - 1000 end

            return rating
        end)

        AH.movefull_stopunit(ai, wolves[i], move)
    end
end

return ca_wolves_move
