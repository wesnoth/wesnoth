--[=[
Functions to support the retreat of injured units
]=]

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"

local function print_dbg(...)
    local show_debug_info = false -- manually set to true/false depending on whether output is desired
    if wesnoth.game_config.debug and show_debug_info then
        std_print('Retreat debug: ', ...)
    end
end

local retreat_functions = {}

function retreat_functions.min_hp(unit)
    -- The minimum hp to retreat is a function of hitpoints and terrain defense
    -- We want to stay longer on good terrain and leave early on bad terrain
    -- It can be influenced by the 'retreat_factor' AI aspect

    local retreat_factor = ai.aspects.retreat_factor

    -- Leaders are more valuable and should retreat earlier
    if unit.canrecruit then
        retreat_factor = retreat_factor * 1.5
    end

    -- Higher retreat willingness on bad terrain
    retreat_factor = retreat_factor * (100 - unit:defense_on(wesnoth.current.map[unit])) / 50

    local min_hp = retreat_factor * unit.max_hitpoints

    -- Account for poison damage on next turn
    if unit.status.poisoned then min_hp = min_hp + wesnoth.game_config.poison_amount end

    -- Large values of retreat_factor could cause fully healthy units to retreat.
    -- We require a unit to be down more than 10 HP, or half its HP for units with less than 20 max_HP.
    local max_hp = unit.max_hitpoints
    local max_min_hp = math.max(max_hp - 10, max_hp / 2)
    if (min_hp > max_min_hp) then
        min_hp = max_min_hp
    end

    local retreat_str = ''
    if (unit.hitpoints < min_hp) then retreat_str = '  --> retreat' end
    print_dbg(string.format('%-20s %3d/%-3d HP  threshold: %5.1f HP%s', unit.id, unit.hitpoints, unit.max_hitpoints, min_hp, retreat_str))

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
                    for regenerates in wml.child_range(abilities, "regenerate") do
                        if (tonumber(regenerates.value) or 0) > regen_amount then
                            regen_amount = tonumber(regenerates.value)
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

function retreat_functions.get_healing_locations(possible_healers)
    local healing_locs = LS.create()
    for i,u in ipairs(possible_healers) do
        -- Only consider healers that cannot move this turn
        if u.moves == 0 or u.side ~= wesnoth.current.side then
            local heal_amount = 0
            local cure = 0
            local abilities = wml.get_child(u.__cfg, "abilities") or {}
            for ability in wml.child_range(abilities, "heals") do
                heal_amount = (tonumber(ability.value) or 0)
                if ability.poison == "slowed" then
                    cure = 1
                elseif ability.poison == "cured" then
                    cure = 2
                end
            end
            if heal_amount + cure > 0 then
                for x, y in wesnoth.current.map:iter_adjacent(u) do
                    local old_values = healing_locs:get(x, y) or {0, 0}
                    local best_heal = math.max(old_values[0] or heal_amount)
                    local best_cure = math.max(old_values[1] or cure)
                    healing_locs:insert(x, y, {best_heal, best_cure})
                end
            end
        end
    end

    return healing_locs
end

function retreat_functions.get_retreat_injured_units(healees, regen_amounts, avoid_map)
    local retreat_enemy_weight = ai.aspects.retreat_enemy_weight

    local allies = AH.get_live_units {
        { "filter_side", { {"allied_with", { side = wesnoth.current.side } } } }
    }
    local healing_locs = retreat_functions.get_healing_locations(allies)

    -- These operations are somewhat expensive, don't do them if not necessary
    local enemy_attack_map, ally_attack_map
    if (retreat_enemy_weight ~= 0) then
        local enemies = AH.get_attackable_enemies()
        enemy_attack_map = BC.get_attack_map(enemies)
        ally_attack_map = BC.get_attack_map(allies)
    end

    local max_rating, best_loc, best_unit = - math.huge, nil, nil
    for i,u in ipairs(healees) do
        wesnoth.interface.handle_user_interact()
        local possible_locations = wesnoth.paths.find_reach(u)
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
                        -- Do not take villages from an allied side
                        local owner = wesnoth.map.get_owner(loc)
                        if owner
                            and (owner ~= wesnoth.current.side)
                            and (not wesnoth.sides.is_enemy(wesnoth.current.side, owner))
                        then
                            -- If allow_ally_villages is true, injured units are allowed to take ally villages.
                            -- However, they should do so with lower priority, which we do by halving the heal and cure amounts.
                            if ai.aspects.allow_ally_villages then
                                heal_amount = heal_amount / 2
                                curing = 1
                            else
                                heal_amount = 0
                            end
                        else
                            curing = 2
                        end
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

        print_dbg(string.format('check retreat hexes for: %-20s  base_rating = %f8.1', u.id, base_rating))

        for j,loc in ipairs(possible_locations) do
            local unit_in_way = wesnoth.units.get(loc[1], loc[2])
            if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way))
                or ((unit_in_way.moves > 0) and (unit_in_way.side == wesnoth.current.side))
            then
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

                -- Figure out the enemy threat - this is also needed to assess whether rest healing is likely
                local enemy_rating, enemy_count = 0, 0
                if (retreat_enemy_weight ~= 0) then
                    enemy_count = enemy_attack_map.units:get(loc[1], loc[2]) or 0
                    local enemy_hp = enemy_attack_map.hitpoints:get(loc[1], loc[2]) or 0
                    local ally_hp = ally_attack_map.hitpoints:get(loc[1], loc[2]) or 0
                    local hp_diff = ally_hp - enemy_hp * math.abs(retreat_enemy_weight)
                    if (hp_diff > 0) then hp_diff = 0 end

                    -- The rating is mostly the HP difference, but we still want to
                    -- avoid threatened hexes even if we have the advantage
                    enemy_rating = hp_diff - enemy_count * math.abs(retreat_enemy_weight)
                end

                if (loc[1] == u.x) and (loc[2] == u.y) and (not u.status.poisoned) then
                    if is_healthy or enemy_count == 0 then
                        -- Bonus if we can rest heal
                        heal_score = heal_score + wesnoth.game_config.rest_heal_amount
                    end
                end

                -- Only consider healing locations, except when retreat_enemy_weight is negative
                if (heal_score > 0) or (retreat_enemy_weight < 0) then
                    local rating = base_rating + heal_score^2
                    rating = rating + enemy_rating

                    -- Penalty based on terrain defense for unit
                    rating = rating - (100 - u:defense_on(wesnoth.current.map[loc]))/10

                    -- Penalty if a unit has to move out of the way
                    -- (based on hp of moving unit)
                    if unit_in_way and ((loc[1] ~= u.x) or (loc[2] ~= u.y)) then
                        rating = rating + unit_in_way.hitpoints - unit_in_way.max_hitpoints
                    end

                    print_dbg(string.format('  possible retreat hex: %3d,%-3d  rating = %9.1f  (heal_score = %5.1f,  enemy_rating = %9.1f)', loc[1], loc[2], rating, heal_score, enemy_rating))

                    if (rating > max_rating) then
                        max_rating, best_loc, best_unit = rating, loc, u
                    end
                end
            end
        end
    end

    local threat = 0
    if best_unit then
        threat = enemy_attack_map and enemy_attack_map.units:get(best_loc[1], best_loc[2]) or 0
        print_dbg(string.format('found unit to retreat: %s --> %d,%d', best_unit.id, best_loc[1], best_loc[2]))
    end

    return best_unit, best_loc, threat
end

return retreat_functions
