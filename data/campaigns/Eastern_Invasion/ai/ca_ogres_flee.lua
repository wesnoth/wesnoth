local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local M = wesnoth.map

local ca_ogres_flee = {}

function ca_ogres_flee:evaluation()
    local units = AH.get_units_with_moves { side = wesnoth.current.side }

    if (not units[1]) then return 0 end
    return 110000
end

function ca_ogres_flee:execution()
    local units = AH.get_units_with_moves { side = wesnoth.current.side }

    local units_noMP = wesnoth.units.find_on_map { side = wesnoth.current.side,
        formula = 'movement_left = 0'
    }

    -- Need the enemy map and enemy attack map if avoid_enemies is set
    local enemies = wesnoth.units.find_on_map {  { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } } }
    local enemy_attack_map = BC.get_attack_map(enemies)

    local max_rating, best_hex, best_unit = - math.huge
    for i,u in ipairs(units) do
        local reach = wesnoth.find_reach(u)
        for j,r in ipairs(reach) do
            local unit_in_way = wesnoth.units.get(r[1], r[2])

            if (not unit_in_way) or (unit_in_way == u) then

                -- First rating is distance from a map edge
                local map = wesnoth.current.map
                local dist_left = r[1] - 1
                local dist_right = map.playable_width - r[1]
                local dist_top_left = M.distance_between(r[1], r[2], 4, 1)
                local dist_top_right = M.distance_between(r[1], r[2], 40, 1)
                local dist_bottom = map.playable_height - r[2]
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

                if (rating > max_rating) then
                    best_hex = { r[1], r[2] }
                    best_unit = u
                    max_rating = rating
                end
            end
        end
    end

    if best_hex then
        AH.movefull_outofway_stopunit(ai, best_unit, best_hex[1], best_hex[2])
    else
        ai.stopunit_moves(best_unit)
    end
end

return ca_ogres_flee
