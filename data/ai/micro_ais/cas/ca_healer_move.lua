local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local M = wesnoth.map

local ca_healer_move, best_healer, best_hex = {}

function ca_healer_move:evaluation(cfg, data)
    -- Should happen with higher priority than attacks, except at beginning of turn,
    -- when we want attacks (by other units) done first
    -- This is done so that it is possible for healers to attack, if they do not
    -- find an appropriate hex to back up other units
    local score = data.HS_healer_move_score or 105000

    local all_healers = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        ability = "healing",
        { "and", wml.get_child(cfg, "filter") }
    }

    local healers, healers_noMP = {}, {}
    for _,healer in ipairs(all_healers) do
        -- For the purpose of this evaluation, guardians count as units without moves, as do passive leaders
        if (healer.moves > 0) and (not healer.status.guardian)
            and ((not healer.canrecruit) or (not ai.aspects.passive_leader))
        then
            table.insert(healers, healer)
        else
            table.insert(healers_noMP, healer)
        end
    end
    if (not healers[1]) then return 0 end

    local all_healees = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        { "and", wml.get_child(cfg, "filter_second") }
    }

    local healees, healees_MP = {}, {}
    for _,healee in ipairs(all_healees) do
        -- Potential healees are units without MP that don't already have a healer (also without MP) next to them
        -- Also, they cannot be on a healing location or regenerate
        if (healee.moves == 0) then
            if (not healee:matches { ability = "regenerates" }) then
                local healing = wesnoth.terrain_types[wesnoth.current.map[healee]].healing
                if (healing == 0) then
                    local is_healee = true
                    for _,healer in ipairs(healers_noMP) do
                        if (M.distance_between(healee.x, healee.y, healer.x, healer.y) == 1) then
                            is_healee = false
                            break
                        end
                    end
                    if is_healee then table.insert(healees, healee) end
                end
            end
        else
            table.insert(healees_MP, healee)
        end
    end

    local enemies = AH.get_attackable_enemies()
    for _,healee in ipairs(healees_MP) do healee:extract() end
    local enemy_attack_map = BC.get_attack_map(enemies)
    for _,healee in ipairs(healees_MP) do healee:to_map() end

    -- Other options of adding avoid zones may be added later
    local avoid_map = AH.get_avoid_map(ai, nil, true)

    local max_rating = - math.huge
    for _,healer in ipairs(healers) do
        local reach_map = AH.get_reachmap(healer, { avoid_map = avoid_map, exclude_occupied = true })
        reach_map:iter( function(x, y, v)
            -- Only consider hexes that are next to at least one noMP unit that
            --  - either can be attacked by an enemy (15 points per enemy)
            --  - or has non-perfect HP (1 point per missing HP)

            local rating, adjacent_healer = 0
            for _,healee in ipairs(healees) do
                if (M.distance_between(healee.x, healee.y, x, y) == 1) then
                    -- Note: These ratings have to be positive or the method doesn't work
                    rating = rating + healee.max_hitpoints - healee.hitpoints

                    -- If injured_units_only = true then don't count units with full HP
                    if (healee.max_hitpoints - healee.hitpoints > 0) or (not cfg.injured_units_only) then
                        rating = rating + 15 * (enemy_attack_map.units:get(healee.x, healee.y) or 0)
                    end
                end
            end

            -- Number of enemies that can threaten the healer at that position
            -- This has to be no larger than cfg.max_threats for hex to be considered
            local enemies_in_reach = enemy_attack_map.units:get(x, y) or 0

            -- If this hex fulfills those requirements, 'rating' is now greater than 0
            -- and we do the rest of the rating, otherwise set rating to below max_rating
            if (rating == 0) or (enemies_in_reach > (cfg.max_threats or 9999)) then
                rating = max_rating - 1
            else
                -- Strongly discourage hexes that can be reached by enemies
                rating = rating - enemies_in_reach * 1000

                -- All else being more or less equal, prefer villages and strong terrain
                local terrain = wesnoth.current.map[{x, y}]
                local is_village = wesnoth.terrain_types[terrain].village
                if is_village then rating = rating + 2 end

                local defense = healer:defense_on(terrain)
                rating = rating + defense / 10.
            end

            if (rating > max_rating) then
                max_rating, best_healer, best_hex = rating, healer, { x, y }
            end
        end)
    end

    if best_healer then
        return score
    end

    return 0
end

function ca_healer_move:execution(cfg)
    AH.robust_move_and_attack(ai, best_healer, best_hex)
    best_healer, best_hex = nil, nil
end

return ca_healer_move
