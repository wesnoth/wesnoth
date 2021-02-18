--[=[
Functions to support the retreat of injured units
]=]

local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"

local retreat_functions = {}

function retreat_functions.min_hp(unit)
    -- The minimum hp to retreat is a function of level and terrain defense
    -- We want to stay longer on good terrain and leave early on very bad terrain

    -- Take caution into account here. We want the multiplier to be:
    --   1 for default caution (0.25)
    --   0 for minimal caution <= 0
    --   2 for caution = 1
    local caution_factor = ai.aspects.caution
    if (caution_factor < 0) then caution_factor = 0 end
    caution_factor = math.sqrt(caution_factor) * 2

    local hp_per_level = (100 - unit:defense_on(wesnoth.current.map[unit]))/15 * caution_factor
    local level = unit.level

    -- Leaders are considered to be higher level because of their value
    if unit.canrecruit then level = level+2 end

    local min_hp = hp_per_level*(level+2)

    -- Account for poison damage on next turn
    if unit.status.poisoned then min_hp = min_hp + wesnoth.game_config.poison_amount end

    -- Make sure that units are actually injured (only relevant for low-HP units)
    -- Want this to be roughly half the units HP at caution=0, close to full HP at caution=1
    local hp_factor = 0.5 + 0.25 * caution_factor
    if (hp_factor > 1) then hp_factor = 1 end
    local max_min_hp = (unit.max_hitpoints - 4) * hp_factor
    if (min_hp > max_min_hp) then
        min_hp = max_min_hp
    end

    return min_hp
end

-- Given a set of units, return one from the set that should retreat and the location to retreat to
-- Return nil if no unit needs to retreat
function retreat_functions.retreat_injured_units(units, avoid_map)
    -- Split units into those that regenerate and those that do not
    local regen, regen_amounts, non_regen = {}, {}, {}
    for i,u in ipairs(units) do
        if (u.hitpoints < retreat_functions.min_hp(u))
            and ((not u.canrecruit) or (not ai.aspects.passive_leader))
        then
            if u:ability('regenerate') then
                -- Find the best regeneration ability and use it to estimate hp regained by regeneration
                local abilities = wml.get_child(u.__cfg, "abilities")
                local regen_amount = 0
                if abilities then
                    for regen in wml.child_range(abilities, "regenerate") do
                        if regen.value > regen_amount then
                            regen_amount = regen.value
                        end
                    end
                end
                table.insert(regen, u)
                table.insert(regen_amounts, regen_amount)
            else
                table.insert(non_regen, u)
            end
        end
    end

    -- First we retreat non-regenerating units to healing terrain, if they can get to a safe location
    local unit_nr, loc_nr, threat_nr
    if non_regen[1] then
        unit_nr, loc_nr, threat_nr = retreat_functions.get_retreat_injured_units(non_regen, {}, avoid_map)
        if unit_nr and (threat_nr == 0) then
            return unit_nr, loc_nr, threat_nr
        end
    end

    -- Then we retreat regenerating units to terrain with high defense, if they can get to a safe location
    local unit_r, loc_r, threat_r
    if regen[1] then
        unit_r, loc_r, threat_r = retreat_functions.get_retreat_injured_units(regen, regen_amounts, avoid_map)
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

function retreat_functions.get_healing_locations()
    local possible_healers = AH.get_live_units {
        { "filter_side", {{"allied_with", {side = wesnoth.current.side} }} }
    }

    local healing_locs = LS.create()
    for i,u in ipairs(possible_healers) do
        -- Only consider healers that cannot move this turn
        if u.moves == 0 or u.side ~= wesnoth.current.side then
            local heal_amount = 0
            local cure = 0
            local abilities = wml.get_child(u.__cfg, "abilities") or {}
            for ability in wml.child_range(abilities, "heals") do
                heal_amount = ability.value or 0
                if ability.poison == "slowed" then
                    cure = 1
                elseif ability.poison == "cured" then
                    cure = 2
                end
            end
            if heal_amount + cure > 0 then
                for x, y in H.adjacent_tiles(u.x, u.y) do
                    local old_values = healing_locs:get(x, y) or {0, 0}
                    local best_heal = math.max(old_values[0] or heal_amount)
                    local best_cure = math.max(old_values[1] or cure)
                    healing_locs:insert(u.x, u.y, {best_heal, best_cure})
                end
            end
        end
    end

    return healing_locs
end

function retreat_functions.get_retreat_injured_units(healees, regen_amounts, avoid_map)
    -- Only retreat to safe locations
    local enemies = AH.get_attackable_enemies()
    local enemy_attack_map = BC.get_attack_map(enemies)

    local healing_locs = retreat_functions.get_healing_locations()

    local max_rating, best_loc, best_unit = - math.huge
    for i,u in ipairs(healees) do
        local possible_locations = wesnoth.find_reach(u)
        -- TODO: avoid ally's villages (may be preferable to lower rating so they will
        -- be used if unit is very injured)
        if (not regen_amounts[i]) then
            -- Unit cannot self heal, make the terrain do it for us if possible
            local location_subset = {}
            for j,loc in ipairs(possible_locations) do
                if (not avoid_map) or (not avoid_map:get(loc[1], loc[2])) then
                    local heal_amount = wesnoth.terrain_types[wesnoth.current.map[loc]].healing or 0
                    if heal_amount == true then
                        -- handle deprecated syntax
                        -- TODO: remove this when removed from game
                        heal_amount = 8
                    end
                    local curing = 0
                    if heal_amount > 0 then
                        curing = 2
                    end
                    local healer_values = healing_locs:get(loc[1], loc[2]) or {0, 0}
                    heal_amount = math.max(heal_amount, healer_values[1])
                    curing = math.max(curing, healer_values[2])
                    table.insert(location_subset, {loc[1], loc[2], heal_amount, curing})
                end
            end

            possible_locations = location_subset
        end

        local is_healthy = false
        for _,trait in ipairs(u.traits) do
            if (trait == 'healthy') then
                is_healthy = true
                break
            end
        end

        local base_rating = - u.hitpoints + u.max_hitpoints / 2.
        if u.status.poisoned then base_rating = base_rating + wesnoth.game_config.poison_amount end
        if u.status.slowed then base_rating = base_rating + 4 end
        base_rating = base_rating * 1000

        for j,loc in ipairs(possible_locations) do
            local unit_in_way = wesnoth.units.get(loc[1], loc[2])
            if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way))
                or ((unit_in_way.moves > 0) and (unit_in_way.side == wesnoth.current.side))
            then
                local rating = base_rating
                local heal_score = 0
                if regen_amounts[i] then
                    heal_score = math.min(regen_amounts[i], u.max_hitpoints - u.hitpoints)
                else
                    if u.status.poisoned then
                        if loc[4] > 0 then
                            heal_score = math.min(wesnoth.game_config.poison_amount, u.hitpoints - 1)
                            if loc[4] == 2 then
                                -- This value is arbitrary, it just represents the ability to heal on the turn after
                                heal_score = heal_score + 1
                            end
                        end
                    else
                        heal_score = math.min(loc[3], u.max_hitpoints - u.hitpoints)
                    end
                end

                -- Huge penalty for each enemy that can reach location,
                -- this is the single most important point (and non-linear)
                local enemy_count = enemy_attack_map.units:get(loc[1], loc[2]) or 0
                rating = rating - enemy_count * 100000

                -- Penalty based on terrain defense for unit
                rating = rating - (100 - u:defense_on(wesnoth.current.map[loc]))/10

                if (loc[1] == u.x) and (loc[2] == u.y) and (not u.status.poisoned) then
                    if is_healthy or enemy_count == 0 then
                        -- Bonus if we can rest heal
                        heal_score = heal_score + wesnoth.game_config.rest_heal_amount
                    end
                elseif unit_in_way then
                    -- Penalty if a unit has to move out of the way
                    -- (based on hp of moving unit)
                    rating = rating + unit_in_way.hitpoints - unit_in_way.max_hitpoints
                end

                rating = rating + heal_score^2

                if (rating > max_rating) then
                    max_rating, best_loc, best_unit = rating, loc, u
                end
            end
        end
    end

    return best_unit, best_loc, enemy_attack_map.units:get(best_loc[1], best_loc[2]) or 0
end

return retreat_functions
