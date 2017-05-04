return {
    init = function(ai)

        -- Grab a useful separate CA as a starting point
        local generic_rush = wesnoth.require("ai/lua/move_to_any_target.lua").init(ai)

        -- More generic grunt rush (and can, in fact, be used with other unit types as well)

        local H = wesnoth.require "helper"
        local W = H.set_wml_action_metatable {}
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local BC = wesnoth.require "ai/lua/battle_calcs.lua"
        local LS = wesnoth.require "location_set"
        local HS = wesnoth.require "ai/micro_ais/cas/ca_healer_move.lua"
        local R = wesnoth.require "ai/lua/retreat.lua"

        local function print_time(...)
            if generic_rush.data.turn_start_time then
                AH.print_ts_delta(generic_rush.data.turn_start_time, ...)
            else
                AH.print_ts(...)
            end
        end

        ------ Stats at beginning of turn -----------

        -- This will be blacklisted after first execution each turn
        function generic_rush:stats_eval()
            local score = 999999
            return score
        end

        function generic_rush:stats_exec()
            local tod = wesnoth.get_time_of_day()
            AH.print_ts(' Beginning of Turn ' .. wesnoth.current.turn .. ' (' .. tod.name ..') stats')
            generic_rush.data.turn_start_time = wesnoth.get_time_stamp() / 1000.

            for i,s in ipairs(wesnoth.sides) do
                local total_hp = 0
                local units = AH.get_live_units { side = s.side }
                for i,u in ipairs(units) do total_hp = total_hp + u.hitpoints end
                local leader = wesnoth.get_units { side = s.side, canrecruit = 'yes' }[1]
                if leader then
                    print('   Player ' .. s.side .. ' (' .. leader.type .. '): ' .. #units .. ' Units with total HP: ' .. total_hp)
                end
            end
        end

        ------- Recruit CA --------------

        local params = {
            score_function = (function() return 300000 end),
            min_turn_1_recruit = (function() return generic_rush:castle_switch_eval() > 0 end),
            leader_takes_village = (function()
                    if generic_rush:castle_switch_eval() > 0 then
                        local take_village = #(wesnoth.get_villages {
                            x = generic_rush.data.leader_target[1],
                            y = generic_rush.data.leader_target[2]
                        }) > 0
                        return take_village
                    end
                    return true
                end
            )
        }
        wesnoth.require("ai/lua/generic_recruit_engine.lua").init(generic_rush, params)

        -------- Castle Switch CA --------------
        local function get_reachable_enemy_leaders(unit)
            -- We're cheating a little here and also find hidden enemy leaders. That's
            -- because a human player could make a pretty good educated guess as to where
            -- the enemy leaders are likely to be while the AI does not know how to do that.
            local potential_enemy_leaders = AH.get_live_units { canrecruit = 'yes',
                { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
            }
            local enemy_leaders = {}
            for j,e in ipairs(potential_enemy_leaders) do
                local path, cost = wesnoth.find_path(unit, e.x, e.y, { ignore_units = true, viewing_side = 0 })
                if cost < AH.no_path then
                    table.insert(enemy_leaders, e)
                end
            end

            return enemy_leaders
        end

        function generic_rush:castle_switch_eval()
            local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'castle_switch'
            if AH.print_eval() then print_time('     - Evaluating castle_switch CA:') end

            if ai.aspects.passive_leader then
                -- Turn off this CA if the leader is passive
                return 0
            end

            local leader = wesnoth.get_units {
                    side = wesnoth.current.side,
                    canrecruit = 'yes',
                    formula = '(movement_left = total_movement) and (hitpoints = max_hitpoints)'
                }[1]
            if not leader then
                -- CA is irrelevant if no leader or the leader may have moved from another CA
                self.data.leader_target = nil
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 0
            end

            local cheapest_unit_cost = AH.get_cheapest_recruit_cost()

            if self.data.leader_target and wesnoth.sides[wesnoth.current.side].gold >= cheapest_unit_cost then
                -- make sure move is still valid
                local next_hop = AH.next_hop(leader, self.data.leader_target[1], self.data.leader_target[2])
                if next_hop and next_hop[1] == self.data.leader_target[1]
                and next_hop[2] == self.data.leader_target[2] then
                    return self.data.leader_score
                end
            end

            local width,height,border = wesnoth.get_map_size()
            local keeps = wesnoth.get_locations {
                terrain = 'K*,K*^*,*^K*', -- Keeps
                x = '1-'..width,
                y = '1-'..height,
                { "not", { {"filter", {}} }}, -- That have no unit
                { "not", { radius = 6, {"filter", { canrecruit = 'yes',
                    { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
                }} }}, -- That are not too close to an enemy leader
                { "not", {
                    x = leader.x, y = leader.y, terrain = 'K*,K*^*,*^K*',
                    radius = 3,
                    { "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
                }}, -- That are not close and connected to a keep the leader is on
                { "filter_adjacent_location", {
                    terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*'
                }} -- That are not one-hex keeps
            }
            if #keeps < 1 then
                -- Skip if there aren't extra keeps to evaluate
                -- In this situation we'd only switch keeps if we were running away
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 0
            end

            local enemy_leaders = get_reachable_enemy_leaders(leader)

            -- Look for the best keep
            local best_score, best_loc, best_turns = 0, {}, 3
            for i,loc in ipairs(keeps) do
                -- Only consider keeps within 2 turns movement
                local path, cost = wesnoth.find_path(leader, loc[1], loc[2])
                local score = 0
                -- Prefer closer keeps to enemy
                local turns = math.ceil(cost/leader.max_moves)
                if turns <= 2 then
                    score = 1/turns
                    for j,e in ipairs(enemy_leaders) do
                        score = score + 1 / H.distance_between(loc[1], loc[2], e.x, e.y)
                    end

                    if score > best_score then
                        best_score = score
                        best_loc = loc
                        best_turns = turns
                    end
                end
            end

            -- If we're on a keep,
            -- don't move to another keep unless it's much better when uncaptured villages are present
            if best_score > 0 and wesnoth.get_terrain_info(wesnoth.get_terrain(leader.x, leader.y)).keep then
                local close_unowned_village = (wesnoth.get_villages {
                    { "and", {
                    x = leader.x,
                    y = leader.y,
                    radius = leader.max_moves
                    }},
                    owner_side = 0
                })[1]
                if close_unowned_village then
                    local score = 1/best_turns
                    for j,e in ipairs(enemy_leaders) do
                        -- count all distances as three less than they actually are
                        score = score + 1 / (H.distance_between(leader.x, leader.y, e.x, e.y) - 3)
                    end

                    if score > best_score then
                        best_score = 0
                    end
                end
            end

            if best_score > 0 then
                local next_hop = AH.next_hop(leader, best_loc[1], best_loc[2])

                if next_hop and ((next_hop[1] ~= leader.x) or (next_hop[2] ~= leader.y)) then
                    -- See if there is a nearby village that can be captured without delaying progress
                    local close_villages = wesnoth.get_villages( {
                        { "and", { x = next_hop[1], y = next_hop[2], radius = leader.max_moves }},
                        owner_side = 0 })
                    for i,loc in ipairs(close_villages) do
                        local path_village, cost_village = wesnoth.find_path(leader, loc[1], loc[2])
                        if cost_village <= leader.moves then
                            local dummy_leader = wesnoth.copy_unit(leader)
                            dummy_leader.x = loc[1]
                            dummy_leader.y = loc[2]
                            local path_keep, cost_keep = wesnoth.find_path(dummy_leader, best_loc[1], best_loc[2])
                            local turns_from_keep = math.ceil(cost_keep/leader.max_moves)
                            if turns_from_keep < best_turns
                            or (turns_from_keep == 1 and wesnoth.sides[wesnoth.current.side].gold < cheapest_unit_cost)
                            then
                                -- There is, go there instead
                                next_hop = loc
                                break
                            end
                        end
                    end
                end

                self.data.leader_target = next_hop

                -- if we're on a keep, wait until there are no movable units on the castle before moving off
                self.data.leader_score = 290000
                if wesnoth.get_terrain_info(wesnoth.get_terrain(leader.x, leader.y)).keep then
                    local castle = wesnoth.get_locations {
                        x = "1-"..width, y = "1-"..height,
                        { "and", {
                            x = leader.x, y = leader.y, radius = 200,
                            { "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
                        }}
                    }
                    local should_wait = false
                    for i,loc in ipairs(castle) do
                        local unit = wesnoth.get_unit(loc[1], loc[2])
                        if (not AH.is_visible_unit(wesnoth.current.side, unit)) then
                            should_wait = false
                            break
                        elseif unit.moves > 0 then
                            should_wait = true
                        end
                    end
                    if should_wait then
                        self.data.leader_score = 15000
                    end
                end

                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return self.data.leader_score
            end

            if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
            return 0
        end

        function generic_rush:castle_switch_exec()
            local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]

            if AH.print_exec() then print_time('   Executing castle_switch CA') end
            if AH.show_messages() then W.message { speaker = leader.id, message = 'Switching castles' } end

            AH.checked_move(ai, leader, self.data.leader_target[1], self.data.leader_target[2])
            self.data.leader_target = nil
        end

        ------- Grab Villages CA --------------

        function generic_rush:grab_villages_eval()
            local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'grab_villages'
            if AH.print_eval() then print_time('     - Evaluating grab_villages CA:') end

            -- Check if there are units with moves left
            local units = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'no',
                formula = 'movement_left > 0'
            }
            if (not units[1]) then
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 0
            end

            local enemies = AH.get_attackable_enemies()

            local villages = wesnoth.get_villages()
            -- Just in case:
            if (not villages[1]) then
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 0
            end
            --print('#units, #enemies', #units, #enemies)

            -- First check if attacks are possible for any unit
            local return_value = 200000
            -- If one with > 50% chance of kill is possible, set return_value to lower than combat CA
            local attacks = ai.get_attacks()
            --print(#attacks)
            for i,a in ipairs(attacks) do
                if (#a.movements == 1) and (a.chance_to_kill > 0.5) then
                    return_value = 90000
                    break
                end
            end

            -- Also find which locations can be attacked by enemies
            local enemy_attack_map = BC.get_attack_map(enemies).units

            -- Now we go through the villages and units
            local max_rating, best_village, best_unit = -9e99, {}, {}
            local village_ratings = {}
            for j,v in ipairs(villages) do
                -- First collect all information that only depends on the village
                local village_rating = 0  -- This is the unit independent rating

                local unit_in_way = wesnoth.get_unit(v[1], v[2])

                -- If an enemy can get within one move of the village, we want to hold it
                if enemy_attack_map:get(v[1], v[2]) then
                        --print('  within enemy reach', v[1], v[2])
                        village_rating = village_rating + 100
                end

                -- Unowned and enemy-owned villages get a large bonus
                local owner = wesnoth.get_village_owner(v[1], v[2])
                if (not owner) then
                    village_rating = village_rating + 10000
                else
                    if wesnoth.is_enemy(owner, wesnoth.current.side) then village_rating = village_rating + 20000 end
                end

                local enemy_distance_from_village = AH.get_closest_enemy(v)

                -- Now we go on to the unit-dependent rating
                local best_unit_rating = -9e99
                local reachable = false
                for i,u in ipairs(units) do
                    -- Skip villages that have units other than 'u' itself on them
                    local village_occupied = false
                    if AH.is_visible_unit(wesnoth.current.side, unit_in_way) and ((unit_in_way ~= u)) then
                        village_occupied = true
                    end

                    -- Rate all villages that can be reached and are unoccupied by other units
                    if (not village_occupied) then
                        -- Path finding is expensive, so we do a first cut simply by distance
                        -- There is no way a unit can get to the village if the distance is greater than its moves
                        local dist = H.distance_between(u.x, u.y, v[1], v[2])
                        if (dist <= u.moves) then
                            local path, cost = wesnoth.find_path(u, v[1], v[2])
                            if (cost <= u.moves) then
                                village_rating = village_rating - 1
                                reachable = true
                                --print('Can reach:', u.id, v[1], v[2], cost)
                                local rating = 0

                                -- Prefer strong units if enemies can reach the village, injured units otherwise
                                if enemy_attack_map:get(v[1], v[2]) then
                                    rating = rating + u.hitpoints
                                else
                                    rating = rating + u.max_hitpoints - u.hitpoints
                                end

                                -- Prefer not backtracking and moving more distant units to capture villages
                                local enemy_distance_from_unit = AH.get_closest_enemy({u.x, u.y})
                                rating = rating - (enemy_distance_from_village + enemy_distance_from_unit)/5

                                if (rating > best_unit_rating) then
                                    best_unit_rating, best_unit = rating, u
                                end
                                --print('  rating:', rating)
                            end
                        end
                    end
                end
                village_ratings[v] = {village_rating, best_unit, reachable}
            end
            for j,v in ipairs(villages) do
                local rating = village_ratings[v][1]
                if village_ratings[v][3] and rating > max_rating then
                    max_rating, best_village, best_unit = rating, v, village_ratings[v][2]
                end
            end
            --print('max_rating', max_rating)

            if (max_rating > -9e99) then
                self.data.unit, self.data.village = best_unit, best_village
                if (max_rating >= 1000) then
                    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                    return return_value
                else
                    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                    return 0
                end
            end
            if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
            return 0
        end

        function generic_rush:grab_villages_exec()
            if AH.print_exec() then print_time('   Executing grab_villages CA') end
            if AH.show_messages() then W.message { speaker = self.data.unit.id, message = 'Grab villages' } end

            AH.movefull_stopunit(ai, self.data.unit, self.data.village)
            self.data.unit, self.data.village = nil, nil
        end

        ------- Spread Poison CA --------------

        function generic_rush:spread_poison_eval()
            local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'spread_poison'
            if AH.print_eval() then print_time('     - Evaluating spread_poison CA:') end

            -- If a unit with a poisoned weapon can make an attack, we'll do that preferentially
            -- (with some exceptions)
            local poisoners = AH.get_live_units { side = wesnoth.current.side,
                formula = 'attacks_left > 0',
                { "filter_wml", {
                    { "attack", {
                        { "specials", {
                            { "poison", { } }
                        } }
                    } }
                } },
                canrecruit = 'no'
            }
            --print('#poisoners', #poisoners)
            if (not poisoners[1]) then
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 0
            end

            local attacks = AH.get_attacks(poisoners)
            --print('#attacks', #attacks)
            if (not attacks[1]) then
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 0
            end

            -- Go through all possible attacks with poisoners
            local max_rating, best_attack = -9e99, {}
            for i,a in ipairs(attacks) do
                local attacker = wesnoth.get_unit(a.src.x, a.src.y)
                local defender = wesnoth.get_unit(a.target.x, a.target.y)

                -- Don't try to poison a unit that cannot be poisoned
                local cant_poison = defender.status.poisoned or defender.status.unpoisonable

                -- For now, we also simply don't poison units on villages (unless standard combat CA does it)
                local on_village = wesnoth.get_terrain_info(wesnoth.get_terrain(defender.x, defender.y)).village

                -- Also, poisoning units that would level up through the attack or could level on their turn as a result is very bad
                local about_to_level = defender.max_experience - defender.experience <= (wesnoth.unit_types[attacker.type].level * 2)

                if (not cant_poison) and (not on_village) and (not about_to_level) then
                    -- Strongest enemy gets poisoned first
                    local rating = defender.hitpoints

                    -- Always attack enemy leader, if possible
                    if defender.canrecruit then rating = rating + 1000 end

                    -- Enemies that can regenerate are not good targets
                    if wesnoth.unit_ability(defender, 'regenerate') then rating = rating - 1000 end

                    -- More priority to enemies on strong terrain
                    local defender_defense = 100 - wesnoth.unit_defense(defender, wesnoth.get_terrain(defender.x, defender.y))
                    rating = rating + defender_defense / 4.

                    -- For the same attacker/defender pair, go to strongest terrain
                    local attack_defense = 100 - wesnoth.unit_defense(attacker, wesnoth.get_terrain(a.dst.x, a.dst.y))
                    rating = rating + attack_defense / 2.
                    --print('rating', rating)

                    -- And from village everything else being equal
                    local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(a.dst.x, a.dst.y)).village
                    if is_village then rating = rating + 0.5 end

                    if rating > max_rating then
                        max_rating, best_attack = rating, a
                    end
                end
            end

            if (max_rating > -9e99) then
                self.data.attack = best_attack
                if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
                return 190000
            end
            if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
            return 0
        end

        function generic_rush:spread_poison_exec()
            local attacker = wesnoth.get_unit(self.data.attack.src.x, self.data.attack.src.y)
            -- If several attacks have poison, this will always find the last one
            local is_poisoner, poison_weapon = AH.has_weapon_special(attacker, "poison")

            if AH.print_exec() then print_time('   Executing spread_poison CA') end
            if AH.show_messages() then W.message { speaker = attacker.id, message = 'Poison attack' } end

            AH.robust_move_and_attack(ai, attacker, self.data.attack.dst, self.data.attack.target, { weapon = poison_weapon })

            self.data.attack = nil
        end

        ------- Place Healers CA --------------

        function generic_rush:place_healers_eval()
            if HS:evaluation(ai, {}, self) > 0 then
                return 95000
            end
            return 0
        end

        function generic_rush:place_healers_exec()
            HS:execution(ai, nil, self)
        end

        ------- Retreat CA --------------

        function generic_rush:retreat_injured_units_eval()
            local units = wesnoth.get_units {
                side = wesnoth.current.side,
                formula = 'movement_left > 0'
            }
            local unit, loc = R.retreat_injured_units(units)
            if unit then
                self.data.retreat_unit = unit
                self.data.retreat_loc = loc

                -- First check if attacks are possible for any unit
                -- If one with > 50% chance of kill is possible, set return_value to lower than combat CA
                local attacks = ai.get_attacks()
                for i,a in ipairs(attacks) do
                    if (#a.movements == 1) and (a.chance_to_kill > 0.5) then
                        return 95000
                    end
                end
                return 205000
            end
            return 0
        end

        function generic_rush:retreat_injured_units_exec()
            AH.robust_move_and_attack(ai, self.data.retreat_unit, self.data.retreat_loc)
            self.data.retreat_unit = nil
            self.data.retreat_loc = nil
        end

        ------- Village Hunt CA --------------
        -- Give extra priority to seeking villages if we have less than our share
        -- our share is defined as being slightly more than the total/the number of sides

        function generic_rush:village_hunt_eval()
            local villages = wesnoth.get_villages()

            if not villages[1] then
                return 0
            end

            local my_villages = wesnoth.get_villages { owner_side = wesnoth.current.side }

            if #my_villages > #villages / #wesnoth.sides then
                return 0
            end

            local allied_villages = wesnoth.get_villages { {"filter_owner", { {"ally_of", { side = wesnoth.current.side }} }} }
            if #allied_villages == #villages then
                return 0
            end

            local units = wesnoth.get_units {
                side = wesnoth.current.side,
                canrecruit = false,
                formula = 'movement_left > 0'
            }

            if not units[1] then
                return 0
            end

            return 30000
        end

        function generic_rush:village_hunt_exec()
            local unit = wesnoth.get_units({
                side = wesnoth.current.side,
                canrecruit = false,
                formula = 'movement_left > 0'
            })[1]

            local villages = wesnoth.get_villages()
            local target, best_cost = nil, AH.no_path
            for i,v in ipairs(villages) do
                if not wesnoth.match_location(v[1], v[2], { {"filter_owner", { {"ally_of", { side = wesnoth.current.side }} }} }) then
                    local path, cost = wesnoth.find_path(unit, v[1], v[2], { ignore_units = true, max_cost = best_cost })
                    if cost < best_cost then
                        target = v
                        best_cost = cost
                    end
                end
            end

            if target then
                local x, y = wesnoth.find_vacant_tile(target[1], target[2], unit)
                local dest = AH.next_hop(unit, x, y)
                AH.checked_move(ai, unit, dest[1], dest[2])
            end
        end

        return generic_rush
    end
}
