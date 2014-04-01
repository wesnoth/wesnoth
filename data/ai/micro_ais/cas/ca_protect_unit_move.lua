local H = wesnoth.require "lua/helper.lua"
local LS = wesnoth.require "lua/location_set.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local ca_protect_unit_move = {}

function ca_protect_unit_move:evaluation(ai, cfg, self)
    -- Always 94999 if one of the units can still move
    local units = {}
    for i,id in ipairs(cfg.id) do
        table.insert(units, wesnoth.get_units{ id = id, formula = '$this_unit.moves > 0' }[1])
    end

    if units[1] then return 94999 end
    return 0
end

function ca_protect_unit_move:execution(ai, cfg, self)
    -- Find and execute best (safest) move toward goal
    local units = {}
    for i,id in ipairs(cfg.id) do
        table.insert(units, wesnoth.get_units{ id = id, formula = '$this_unit.moves > 0' }[1])
    end

    -- Need to take the units off the map, as they don't count into the map scores
    -- (as long as they can still move)
    for i,u in ipairs(units) do wesnoth.extract_unit(u) end

    -- All the units on the map
    -- Counts all enemies, but only own units (not allies)
    local my_units = wesnoth.get_units {side = wesnoth.current.side}
    local enemy_units = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }

    -- My attack map
    local MAM = BC.get_attack_map(my_units).units  -- enemy attack map
    --AH.put_labels(MAM)

    -- Enemy attack map
    local EAM = BC.get_attack_map(enemy_units).units  -- enemy attack map
    --AH.put_labels(EAM)

    -- Now put the units back out there
    for i,u in ipairs(units) do wesnoth.put_unit(u) end

    -- We move the weakest (fewest HP unit) first
    local unit = AH.choose(units, function(tmp) return -tmp.hitpoints end)
    --print("Moving: ",unit.id)

    -- Also need the goal for this unit
    local goal = {}
    for i,id in ipairs(cfg.id) do
        if (unit.id == id) then goal = { cfg.goal_x[1], cfg.goal_y[i] } end
    end
    --print("Goal:",goal[1],goal[2])

    -- Reachable hexes
    local reach_map = AH.get_reachable_unocc(unit)
    --AH.put_labels(reach_map)

    -- Now calculate the enemy inverse distance map
    -- This is done here because we only need it for the hexes the unit can reach
    -- Enemy distance map
    local EIDM = AH.inverse_distance_map(enemy_units, reach_map)
    --AH.put_labels(EIDM)

    -- Get a terrain defense map of reachable hexes
    local TDM = LS.create()
    reach_map:iter(function(x, y, data)
        TDM:insert(x, y, 100 - wesnoth.unit_defense(unit, wesnoth.get_terrain(x, y)))
    end)
    --AH.put_labels(TDM)

    -- And finally, the goal distance map
    local GDM = LS.create()
    reach_map:iter(function(x, y, data)
        GDM:insert(x, y, H.distance_between(x, y, goal[1], goal[2]))
    end)
    --AH.put_labels(GDM)

    -- Configuration parameters (no option to change these enabled at the moment)
    local enemy_weight = self.data.enemy_weight or 100.
    local my_unit_weight = self.data.my_unit_weight or 1.
    local distance_weight = self.data.distance_weight or 3.
    local terrain_weight = self.data.terrain_weight or 0.1
    local bearing = self.data.bearing or 1

    -- If there are no enemies left, only distance to goal matters
    -- This is to avoid rare situations where moving toward goal is canceled by moving away from own units
    if (not enemy_units[1]) then
        enemy_weight = 0
        my_unit_weight = 0
        distance_weight = 3
        terrain_weight = 0
    end

    local max_rating, best_hex = -9e99, -1
    local rating_map = LS.create()  -- Also set up rating map, so that it can be displayed

    for ind,r in pairs(reach_map.values) do
        -- Most important: stay away from enemy: default weight=100 per enemy unit
        -- Staying close to own troops: default weight=1 per own unit (allies don't count)
        local rating =
            (MAM.values[ind] or 0) * my_unit_weight
            - (EAM.values[ind] or 0) * enemy_weight

        -- Distance to goal is second most important thing: weight=3 per hex
        rating = rating - GDM.values[ind] * distance_weight
        -- Note: rating will usually be negative, but that's ok (the least negative hex wins)

        -- Terrain rating. Difference of 30 in defense should be worth ~1 step toward goal
        rating = rating + TDM.values[ind] * terrain_weight

        -- Tie breaker: closer to or farther from enemy
        rating = rating + (EIDM.values[ind] or 0) / 10. * bearing

        if (rating > max_rating) then
            max_rating, best_hex = rating, ind
        end

        rating_map.values[ind] = rating
    end
    --AH.put_labels(rating_map)
    --print("Best rating, hex:", max_rating, best_hex)

    AH.movefull_stopunit(ai, unit, AH.get_LS_xy(best_hex))
end

return ca_protect_unit_move
