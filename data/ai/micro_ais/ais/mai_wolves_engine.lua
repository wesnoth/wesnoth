return {
    init = function(ai, existing_engine)
        local engine = existing_engine or {}

        local H = wesnoth.require "lua/helper.lua"
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local BC = wesnoth.require "ai/lua/battle_calcs.lua"
        local LS = wesnoth.require "lua/location_set.lua"

        function engine:mai_wolves_move_eval(cfg)
            local wolves = wesnoth.get_units { side = wesnoth.current.side,
                formula = '$this_unit.moves > 0', { "and", cfg.filter }
            }
            local prey = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
                { "and", cfg.filter_second }
            }

            if wolves[1] and prey[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_wolves_move_exec(cfg)
            local wolves = wesnoth.get_units { side = wesnoth.current.side,
                formula = '$this_unit.moves > 0', { "and", cfg.filter }
            }
            local prey = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
                { "and", cfg.filter_second }
            }
            --print('#wolves, prey', #wolves, #prey)

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

        function engine:mai_wolves_wander_eval(cfg)
            -- When there's no prey left, the wolves wander and regroup
            local wolves = wesnoth.get_units { side = wesnoth.current.side,
                formula = '$this_unit.moves > 0', { "and", cfg.filter }
            }

            if wolves[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_wolves_wander_exec(cfg)
            local wolves = wesnoth.get_units { side = wesnoth.current.side,
                formula = '$this_unit.moves > 0', { "and", cfg.filter }
            }

            -- Number of wolves that can reach each hex
            local reach_map = LS.create()
            for i,w in ipairs(wolves) do
                local r = AH.get_reachable_unocc(w)
                reach_map:union_merge(r, function(x, y, v1, v2) return (v1 or 0) + (v2 or 0) end)
            end

            -- Add a random rating; avoid avoid_type units
            local avoid_units = wesnoth.get_units { type = cfg.avoid_type,
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
            }
            --print('#avoid_units', #avoid_units)
            -- negative hit for hexes these units can attack
            local avoid = BC.get_attack_map(avoid_units).units

            local max_rating, goal_hex = -9e99, {}
            reach_map:iter( function (x, y, v)
                local rating = v + AH.random(99)/100.
                if avoid:get(x, y) then rating = rating - 1000 end

                if (rating > max_rating) then
                    max_rating, goal_hex = rating, { x, y }
                end

                reach_map:insert(x, y, rating)
            end)
            --AH.put_labels(reach_map)
            --W.message { speaker = 'narrator', message = "Wolves random wander"}

            for i,w in ipairs(wolves) do
                -- For each wolf, we need to check that goal hex is reachable, and out of harm's way
                local best_hex = AH.find_best_move(w, function(x, y)
                    local rating = - H.distance_between(x, y, goal_hex[1], goal_hex[2])
                    if avoid:get(x, y) then rating = rating - 1000 end
                    return rating
                end)
                AH.movefull_stopunit(ai, w, best_hex)
            end
        end

        return engine
    end
}
