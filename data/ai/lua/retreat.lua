--[=[
Functions to support the retreat of injured units
]=]

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local retreat_functions = {}

-- Given a set of units, return one from the set that should retreat and the location to retreat to
-- Return nil if no unit needs to retreat
function retreat_functions.retreat_injured_units(units)
    local min_hp = function(unit)
        -- The minimum hp to retreat is a function of level and terrain defense
        -- We want to stay longer on good terrain and leave early on very bad terrain
        local hp_per_level = wesnoth.unit_defense(unit, wesnoth.get_terrain(unit.x, unit.y))/15
        local level = wesnoth.unit_types[unit.type].level

        -- Leaders are considered to be higher level because of their value
        if unit.canrecruit then level = level+2 end

        local min_hp = hp_per_level*(level+2)

        -- Account for poison damage on next turn
        if unit.status.poisoned then min_hp = min_hp + 8 end

        -- Make sure that units are actually injured
        if min_hp > unit.max_hitpoints - 4 then
            min_hp = unit.max_hitpoints - 4
        end

        return min_hp
    end

    -- Split units into those that regenerate and those that do not
    local regen, non_regen = {}, {}
    for i,u in ipairs(units) do
        if u.hitpoints < min_hp(u) then
            if wesnoth.unit_ability(u, 'regenerate') then
                table.insert(regen, u)
            else
                table.insert(non_regen, u)
            end
        end
    end

    -- First we retreat non-regenerating units to healing terrain, if they can get to a safe location
    local unit_nr, loc_nr, threat_nr
    if non_regen[1] then
        unit_nr, loc_nr, threat_nr = retreat_functions.get_retreat_injured_units(non_regen, false)
        if unit_nr and (threat_nr == 0) then
            return unit_nr, loc_nr, threat_nr
        end
    end

    -- Then we retreat regenerating units to terrain with high defense, if they can get to a safe location
    local unit_r, loc_r, threat_r
    if regen[1] then
        unit_r, loc_r, threat_r = retreat_functions.get_retreat_injured_units(regen, true)
        if unit_r and (threat_r == 0) then
            return unit_r, loc_r, threat_r
        end
    end

    -- The we retreat those that cannot get to a safe location (non-regenerating units first again)
    if unit_nr then
        return unit_nr, loc_nr, threat_nr
    end
    if unit_r then
        return unit_r, loc_r, threat_r
    end
end

function retreat_functions.get_retreat_injured_units(healees, healing_terrain_only)
    -- Only retreat to safe locations
    local enemies = AH.get_live_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }
    local enemy_attack_map = BC.get_attack_map(enemies)

    local max_rating, best_loc, best_unit = -9e99, nil, nil
    for i,u in ipairs(healees) do
        local possible_locations = wesnoth.find_reach(u)
        -- TODO: avoid ally's villages (may be preferable to lower rating so they will
        -- be used if unit is very injured)
        if healing_terrain_only then
            -- Unit cannot self heal, make the terrain do it for us if possible
            -- TODO: add hexes adjacent to healers (only those that will not move)
            local location_subset = {}
            for j,loc in ipairs(possible_locations) do
                if wesnoth.get_terrain_info(wesnoth.get_terrain(loc[1], loc[2])).healing > 0 then
                    table.insert(location_subset, loc)
                end
            end
            if location_subset[1] then
                -- If healing terrain is available, restrict retreat locations to it
                possible_locations = location_subset
            end
        end

        local base_rating = - u.hitpoints + u.max_hitpoints / 2.
        if u.status.poisoned then base_rating = base_rating + 8 end
        if u.status.slowed then base_rating = base_rating + 4 end
        base_rating = base_rating * 1000

        for j,loc in ipairs(possible_locations) do
            local unit_in_way = wesnoth.get_unit(loc[1], loc[2])
            if (not unit_in_way) or ((unit_in_way.moves > 0) and (unit_in_way.side == wesnoth.current.side)) then
                local rating = base_rating

                -- Penalty for each enemy that can reach location
                rating = rating - (enemy_attack_map.units:get(loc[1], loc[2]) or 0) * 10

                -- Penalty based on terrain defense for unit
                rating = rating - wesnoth.unit_defense(u, wesnoth.get_terrain(loc[1], loc[2]))/10

                if (loc[1] == u.x) and (loc[2] == u.y) then
                    -- Bonus if we don't have to move (might get to rest heal)
                    rating = rating + 2
                elseif unit_in_way then
                    -- Penalty if a unit has to move out of the way
                    -- (based on hp of moving unit)
                    rating = rating + unit_in_way.hitpoints - unit_in_way.max_hitpoints
                end

                if (rating > max_rating) then
                    max_rating, best_loc, best_unit = rating, loc, u
                end
            end
        end
    end

    return best_unit, best_loc
end

return retreat_functions
