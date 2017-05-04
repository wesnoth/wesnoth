local H = wesnoth.require "helper"
local LS = wesnoth.require "location_set"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local ca_healer_move, best_healer, best_hex = {}

function ca_healer_move:evaluation(cfg, data)
    -- Should happen with higher priority than attacks, except at beginning of turn,
    -- when we want attacks (by other units) done first
    -- This is done so that it is possible for healers to attack, if they do not
    -- find an appropriate hex to back up other units
    local score = data.HS_healer_move_score or 105000

    local all_healers = wesnoth.get_units {
        side = wesnoth.current.side,
        ability = "healing",
        { "and", H.get_child(cfg, "filter") }
    }

    local healers, healers_noMP = {}, {}
    for _,healer in ipairs(all_healers) do
        if (healer.moves > 0) then
            table.insert(healers, healer)
        else
            table.insert(healers_noMP, healer)
        end
    end
    if (not healers[1]) then return 0 end

    local all_healees = wesnoth.get_units {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter_second") }
    }

    local healees, healees_MP = {}, {}
    for _,healee in ipairs(all_healees) do
        -- Potential healees are units without MP that don't already have a healer (also without MP) next to them
        -- Also, they cannot be on a village or regenerate
        if (healee.moves == 0) then
            if (not wesnoth.match_unit(healee, { ability = "regenerates" })) then
                local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(healee.x, healee.y)).village
                if (not is_village) then
                    local is_healee = true
                    for _,healer in ipairs(healers_noMP) do
                        if (H.distance_between(healee.x, healee.y, healer.x, healer.y) == 1) then
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
    for _,healee in ipairs(healees_MP) do wesnoth.extract_unit(healee) end
    local enemy_attack_map = BC.get_attack_map(enemies)
    for _,healee in ipairs(healees_MP) do wesnoth.put_unit(healee) end

    local avoid_map = LS.of_pairs(ai.aspects.avoid)

    local max_rating = -9e99
    for _,healer in ipairs(healers) do
        local reach = wesnoth.find_reach(healer)

        for _,loc in ipairs(reach) do
            -- Only consider hexes that are next to at least one noMP unit that
            --  - either can be attacked by an enemy (15 points per enemy)
            --  - or has non-perfect HP (1 point per missing HP)

            local rating, adjacent_healer = 0
            if (not avoid_map:get(loc[1], loc[2])) then
                local unit_in_way = wesnoth.get_unit(loc[1], loc[2])
                if (not unit_in_way) or (unit_in_way == healer) then
                    for _,healee in ipairs(healees) do
                        if (H.distance_between(healee.x, healee.y, loc[1], loc[2]) == 1) then
                            -- Note: These ratings have to be positive or the method doesn't work
                            rating = rating + healee.max_hitpoints - healee.hitpoints

                            -- If injured_units_only = true then don't count units with full HP
                            if (healee.max_hitpoints - healee.hitpoints > 0) or (not cfg.injured_units_only) then
                                rating = rating + 15 * (enemy_attack_map.units:get(healee.x, healee.y) or 0)
                            end
                        end
                    end
                end
            end

            -- Number of enemies that can threaten the healer at that position
            -- This has to be no larger than cfg.max_threats for hex to be considered
            local enemies_in_reach = enemy_attack_map.units:get(loc[1], loc[2]) or 0

            -- If this hex fulfills those requirements, 'rating' is now greater than 0
            -- and we do the rest of the rating, otherwise set rating to below max_rating
            if (rating == 0) or (enemies_in_reach > (cfg.max_threats or 9999)) then
                rating = max_rating - 1
            else
                -- Strongly discourage hexes that can be reached by enemies
                rating = rating - enemies_in_reach * 1000

                -- All else being more or less equal, prefer villages and strong terrain
                local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(loc[1], loc[2])).village
                if is_village then rating = rating + 2 end

                local defense = 100 - wesnoth.unit_defense(healer, wesnoth.get_terrain(loc[1], loc[2]))
                rating = rating + defense / 10.
            end

            if (rating > max_rating) then
                max_rating, best_healer, best_hex = rating, healer, { loc[1], loc[2] }
            end
        end
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
