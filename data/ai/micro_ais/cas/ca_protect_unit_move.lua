local LS = wesnoth.require "location_set"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local F = wesnoth.require "functional"

local function get_protected_units(cfg)
    local units = {}
    for u in wml.child_range(cfg, "unit") do
        table.insert(units, AH.get_units_with_moves { id = u.id, side = wesnoth.current.side }[1])
    end
    return units
end

local ca_protect_unit_move = {}

function ca_protect_unit_move:evaluation(cfg)
    if get_protected_units(cfg)[1] then return 94999 end
    return 0
end

function ca_protect_unit_move:execution(cfg, data)
    -- Find and execute best (safest) move toward goal
    local protected_units = get_protected_units(cfg)

    -- Need to take the protected units off the map, as they don't count into the map scores
    -- as long as they can still move
    for _,unit in ipairs(protected_units) do unit:extract() end

    local units = wesnoth.units.find_on_map { side = wesnoth.current.side }
    local enemy_units = AH.get_attackable_enemies()

    local attack_map = BC.get_attack_map(units).units  -- enemy attack map
    local enemy_attack_map = BC.get_attack_map(enemy_units).units  -- enemy attack map

    -- Now put the protected units back out there
    for _,unit in ipairs(protected_units) do unit:to_map() end

    -- We move the weakest (fewest HP unit) first
    local unit = F.choose(protected_units, function(u) return - u.hitpoints end)
    local goal = {}
    for u in wml.child_range(cfg, "unit") do
        if (unit.id == u.id) then goal = AH.get_named_loc_xy('goal', u) end
    end

    local reach_map = AH.get_reachable_unocc(unit)
    local enemy_inverse_distance_map = AH.inverse_distance_map(enemy_units, reach_map)

    local terrain_defense_map = LS.create()
    reach_map:iter(function(x, y, data)
        terrain_defense_map:insert(x, y, unit:defense_on(wesnoth.current.map[{x, y}]))
    end)

    local goal_distance_map = LS.create()
    reach_map:iter(function(x, y, data)
        goal_distance_map:insert(x, y, wesnoth.map.distance_between(x, y, goal[1], goal[2]))
    end)

    -- Configuration parameters (no option to change these enabled at the moment)
    local enemy_weight = 100.
    local my_unit_weight = 1.
    local distance_weight = 3.
    local terrain_weight = 0.1

    -- If there are no enemies left, only distance to goal matters
    -- This is to avoid rare situations where moving toward goal rating is canceled by rating for moving away from own units
    if (not enemy_units[1]) then
        enemy_weight = 0
        my_unit_weight = 0
        distance_weight = 3
        terrain_weight = 0
    end

    local max_rating, best_hex = - math.huge
    for ind,_ in pairs(reach_map.values) do
        local rating =
            (attack_map.values[ind] or 0) * my_unit_weight
            - (enemy_attack_map.values[ind] or 0) * enemy_weight

        rating = rating - goal_distance_map.values[ind] * distance_weight

        rating = rating + terrain_defense_map.values[ind] * terrain_weight

        rating = rating + (enemy_attack_map.values[ind] or 0) / 10.

        if (rating > max_rating) then
            max_rating, best_hex = rating, ind
        end
    end

    AH.movefull_stopunit(ai, unit, AH.get_LS_xy(best_hex))
end

return ca_protect_unit_move
