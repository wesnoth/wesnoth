return {
    init = function(ai)

        local healer_support = {}

        local H = wesnoth.require "lua/helper.lua"
        local W = H.set_wml_action_metatable {}
        local LS = wesnoth.require "lua/location_set.lua"
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local BC = wesnoth.require "ai/lua/battle_calcs.lua"

        -----------------------------------------------------------------
        ----------------- The Healer Support CAs ------------------------
        -----------------------------------------------------------------

        ------ Initialize healer support at beginning of turn -----------

        -- Set variables and aspects correctly at the beginning of the turn
        -- This will be blacklisted after first execution each turn
        function healer_support:initialize_healer_support_eval()
            local score = 999990
            return score
        end

        function healer_support:initialize_healer_support_exec(cfg)
            --print(' Initializing healer_support at beginning of Turn ' .. wesnoth.current.turn)

            -- First, modify the attacks aspect to exclude healers
            -- Always delete the attacks aspect first, so that we do not end up with 100 copies of the facet
            W.modify_ai {
                side = wesnoth.current.side,
                action = "try_delete",
                path = "aspect[attacks].facet[no_healers_attack]"
            }

            -- Then set the aspect to exclude healers
            W.modify_ai {
                side = wesnoth.current.side,
                action = "add",
                path = "aspect[attacks].facet",
                { "facet", {
                    name = "ai_default_rca::aspect_attacks",
                    id = "no_healers_attack",
                    invalidate_on_gamestate_change = "yes",
                    { "filter_own", {
                       { "not", { ability = "healing", { "and", cfg.filter } } }
                    } }
                } }
            }

            -- We also need to set the return score of healer moves to happen _after_ combat at beginning of turn
            self.data.HS_return_score = 95000
        end

        ------ Let healers participate in attacks -----------

        -- After attacks by all other units are done, reset things so that healers can attack, if desired
        -- This will be blacklisted after first execution each turn
        function healer_support:healers_can_attack_eval()
            local score = 99990
            return score
        end

        function healer_support:healers_can_attack_exec()
            --print(' Letting healers participate in attacks from now on')

            --local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
            --W.message { speaker = leader.id, message = "I'm done with the RCA AI combat CA for all other units, letting healers participate now (if they cannot find a support position)." }

            -- Delete the attacks aspect
            --print("Deleting attacks aspect")
            W.modify_ai {
                side = wesnoth.current.side,
                action = "try_delete",
                path = "aspect[attacks].facet[no_healers_attack]"
            }

            -- We also reset the variable containing the return score of the healers CA
            -- This will make it use its default value
            self.data.HS_return_score = nil
        end

        ------ Place healers -----------

        function healer_support:healer_support_eval(cfg)

            -- Should happen with higher priority than attacks, except at beginning of turn,
            -- when we want attacks done first
            -- This is done so that it is possible for healers to attack, if they do not
            -- find an appropriate hex to back up other units
            local score = 105000
            if self.data.HS_return_score then score = self.data.HS_return_score end
            --print('healer_support score:', score)

            cfg = cfg or {}

            local healers = wesnoth.get_units { side = wesnoth.current.side, ability = "healing",
                formula = '$this_unit.moves > 0', { "and", cfg.filter }
            }
            if (not healers[1]) then return 0 end

            local healers_noMP = wesnoth.get_units { side = wesnoth.current.side, ability = "healing",
                formula = '$this_unit.moves = 0', { "and", cfg.filter }
            }

            local all_units = wesnoth.get_units{ side = wesnoth.current.side,
                {"and", cfg.filter_second}
            }

            local healees, units_MP = {}, {}
            for i,u in ipairs(all_units) do
                -- Potential healees are units without MP that don't already have a healer (also without MP) next to them
                -- Also, they cannot be on a village or regenerate
                if (u.moves == 0) then
                    if (not wesnoth.match_unit(u, {ability = "regenerates"})) then
                        local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(u.x, u.y)).village
                        if (not is_village) then
                            local healee = true
                            for j,h in ipairs(healers_noMP) do
                                if (H.distance_between(u.x, u.y, h.x, h.y) == 1) then
                                    --print('Already next to healer:', u.x, u.y, h.x, h.y)
                                    healee = false
                                    break
                                end
                            end
                            if healee then table.insert(healees, u) end
                        end
                    end
                else
                    table.insert(units_MP,u)
                end
            end
            --print('#healees, #units_MP', #healees, #units_MP)

            -- Take all units with moves left off the map, for enemy path finding
            for i,u in ipairs(units_MP) do wesnoth.extract_unit(u) end

            -- Enemy attack map
            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
            }
            local enemy_attack_map = BC.get_attack_map(enemies)
            --AH.put_labels(enemy_attack_map.units)

            local avoid_map = LS.of_pairs(ai.get_avoid())

            -- Put units back out there
            for i,u in ipairs(units_MP) do wesnoth.put_unit(u.x, u.y, u) end

            -- Now find the best healer move
            local max_rating, best_hex = -9e99, {}
            for i,h in ipairs(healers) do
                --local rating_map = LS.create()

                local reach = wesnoth.find_reach(h)
                for j,r in ipairs(reach) do

                    local rating, adjacent_healer = 0

                    -- Only consider hexes that are next to at least one noMP unit that
                    --  - either can be attacked by an enemy (15 points per enemy)
                    --  - or has non-perfect HP (1 point per missing HP)

                    -- Also, hex must be unoccupied by another unit, of course
                    local unit_in_way = wesnoth.get_unit(r[1], r[2])
                    if (not avoid_map:get(r[1], r[2])) then
                        if (not unit_in_way) or ((unit_in_way.x == h.x) and (unit_in_way.y == h.y)) then
                            for k,u in ipairs(healees) do
                                if (H.distance_between(u.x, u.y, r[1], r[2]) == 1) then
                                    -- !!!!!!! These ratings have to be positive or the method doesn't work !!!!!!!!!
                                    rating = rating + u.max_hitpoints - u.hitpoints

                                    -- If injured_units_only = true then don't count units with full HP
                                    if (u.max_hitpoints - u.hitpoints > 0) or (not cfg.injured_units_only) then
                                        rating = rating + 15 * (enemy_attack_map.units:get(u.x, u.y) or 0)
                                    end
                                end
                            end
                        end
                    end

                    -- Number of enemies that can threaten the healer at that position
                    -- This has to be no larger than cfg.max_threats for hex to be considered
                    local enemies_in_reach = enemy_attack_map.units:get(r[1], r[2]) or 0

                    -- If this hex fulfills those requirements, 'rating' is now greater than 0
                    -- and we do the rest of the rating, otherwise set rating to below max_rating
                    if (rating == 0) or (enemies_in_reach > (cfg.max_threats or 9999)) then
                        rating = max_rating - 1
                    else
                        -- Strongly discourage hexes that can be reached by enemies
                        rating = rating - enemies_in_reach * 1000

                        -- All else being more or less equal, prefer villages and strong terrain
                        local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(r[1], r[2])).village
                        if is_village then rating = rating + 2 end

                        local defense = 100 - wesnoth.unit_defense(h, wesnoth.get_terrain(r[1], r[2]))
                        rating = rating + defense / 10.

                        --rating_map:insert(r[1], r[2], rating)
                    end

                    if (rating > max_rating) then
                        max_rating, best_healer, best_hex = rating, h, {r[1], r[2]}
                    end
                end
                --AH.put_labels(rating_map)
                --W.message { speaker = h.id, message = 'Healer rating map for me' }
            end
            --print('best unit move', best_hex[1], best_hex[2], max_rating)

            -- Only move healer if a good move as found
            -- Be aware that this means that other CAs will move the healers if not
            if (max_rating > -9e99) then
                self.data.HS_unit, self.data.HS_hex = best_healer, best_hex
                return score
            end

            return 0
        end

        function healer_support:healer_support_exec()
            -- Only show this message in the healer_support scenario in AI-Demos
            local scenario = wesnoth.get_variable("scenario_name")
            if (scenario == 'healer_support') then
                W.message { speaker = self.data.HS_unit.id, message = 'Moving in to back injured and/or threatened units' }
            end

            AH.movefull_outofway_stopunit(ai, self.data.HS_unit, self.data.HS_hex)
            self.data.HS_unit, self.data.HS_hex =  nil, nil
        end

        return healer_support
    end
}
