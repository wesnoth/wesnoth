local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
--local LS = wesnoth.require "location_set"
local M = wesnoth.map

local ca_ogres_flee = {}

function ca_ogres_flee:evaluation()
    local units = wesnoth.get_units { side = wesnoth.current.side,
        formula = 'movement_left > 0'
    }

    if (not units[1]) then return 0 end
    return 110000
end

function ca_ogres_flee:execution()
    local units = wesnoth.get_units { side = wesnoth.current.side,
        formula = 'movement_left > 0'
    }

    local units_noMP = wesnoth.get_units { side = wesnoth.current.side,
        formula = 'movement_left = 0'
    }

    local width, height = wesnoth.get_map_size()

    -- Need the enemy map and enemy attack map if avoid_enemies is set
    local enemies = wesnoth.get_units {  { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } } }
    local enemy_attack_map = BC.get_attack_map(enemies)

    local best_hex, best_unit, max_rating = {}, nil, -9e99
    for i,u in ipairs(units) do
        local reach = wesnoth.find_reach(u)

        --local rating_map = LS.create()

        for j,r in ipairs(reach) do
            local unit_in_way = wesnoth.get_unit(r[1], r[2])

            if (not unit_in_way) or (unit_in_way == u) then

                -- First rating is distance from a map edge
                local dist_left = r[1] - 1
                local dist_right = width - r[1]
                local dist_top_left = M.distance_between(r[1], r[2], 4, 1)
                local dist_top_right = M.distance_between(r[1], r[2], 40, 1)
                local dist_bottom = height - r[2]
                local dist = math.min(dist_left, dist_right, dist_top_left, dist_top_right, dist_bottom)

                local rating = - dist

                -- If we can reach the edge, we do so
                if (dist == 0) then rating = rating + 1000 end

                local enemy_weight = 0.5
                local enemy_rating = - (enemy_attack_map.units:get(r[1], r[2]) or 0) * enemy_weight

                local enemy_rating = 0
                for k,e in ipairs(enemies) do
                    local dist = M.distance_between(r[1], r[2], e.x, e.y)
                    enemy_rating = enemy_rating + math.sqrt(dist)
                end

                rating = rating + enemy_rating

                -- Also, maximize distance from own units that have already moved
                local own_unit_weight = 0.5
                local own_unit_rating = 0
                for k,u_noMP in ipairs(units_noMP) do
                    local dist = M.distance_between(r[1], r[2], u_noMP.x, u_noMP.y)
                    own_unit_rating = own_unit_rating + math.sqrt(dist)
                end

                rating = rating + own_unit_rating * own_unit_weight

                --rating_map:insert(r[1], r[2], rating)

                if (rating > max_rating) then
                    best_hex = { r[1], r[2] }
                    best_unit = u
                    max_rating = rating
                end
            end
        end

        --AH.put_labels(rating_map)
    end
    --print(best_unit.id, best_unit.x, best_unit.y, best_hex[1], best_hex[2], max_rating)

    if best_hex then
        AH.movefull_outofway_stopunit(ai, best_unit, best_hex[1], best_hex[2])
    else
        ai.stopunit_moves(best_unit)
    end
end

return ca_ogres_flee
