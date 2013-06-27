return {
    init = function(ai, existing_engine)

        local engine = existing_engine or {}
        -- Moves a messenger toward goal coordinates while protecting him and
        -- clearing his way with other units, if necessary

        local H = wesnoth.require "lua/helper.lua"
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local LS = wesnoth.require "lua/location_set.lua"

        function engine:mai_messenger_find_enemies_in_way(unit, goal_x, goal_y)
            -- Returns the first unit on or next to the path of the messenger
            -- unit: proxy table for the messenger unit
            -- goal_x, goal_y: coordinates of the goal toward which the messenger moves
            -- Returns proxy table for the first unit found, or nil if none was found

            local path, cost = wesnoth.find_path(unit, goal_x, goal_y, { ignore_units = true })

            -- If unit cannot get there:
            if cost >= 42424242 then return end

            -- Exclude the hex the unit is currently on
            table.remove(path, 1)

            -- Is there an enemy unit on the first path hex itself?
            -- This would be caught by the adjacent hex check later, but not in the right order
            local enemy = wesnoth.get_units { x = path[1][1], y = path[1][2],
                { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } }
            }[1]
            if enemy then
                --print('  enemy on first path hex:',enemy.id)
                return enemy
            end

            -- After that, go through adjacent hexes of all the other path hexes
            for i, p in ipairs(path) do
                local sub_path, sub_cost = wesnoth.find_path( unit, p[1], p[2], { ignore_units = true })
                if sub_cost <= unit.moves then
                    -- Check for enemy units on one of the adjacent hexes (which includes 2 hexes on path)
                    for x, y in H.adjacent_tiles(p[1], p[2]) do
                        local enemy = wesnoth.get_units { x = x, y = y,
                            { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } }
                        }[1]
                        if enemy then
                            --print('  enemy next to path hex:',enemy.id)
                            return enemy
                        end
                    end
                else  -- If we've reached the end of the path for this turn
                    return
                end
            end

            -- If no unit was found, return nil
            return
        end

        function engine:mai_messenger_find_clearing_attack(unit, goal_x, goal_y)
            -- Check if an enemy is in the way of the messenger
            -- If so, find attack that would "clear" that enemy out of the way
            -- unit: proxy table for the messenger unit
            -- goal_x, goal_y: coordinates of the goal toward which the messenger moves
            -- Returns proxy table containing the attack, or nil if none was found

            local enemy_in_way = self:mai_messenger_find_enemies_in_way(unit, goal_x, goal_y)
            -- If none found, don't attack, just move
            if not enemy_in_way then return end

            local max_rating, best_attack = -9e99, {}
            --print('Finding attacks on',enemy_in_way.name,enemy_in_way.id)

            -- Find all units that can attack this enemy
            local my_units = wesnoth.get_units{ side = wesnoth.current.side, formula = '$this_unit.attacks_left > 0',
                { "not", { id = unit.id } }
            }
            --print('#my_units', #my_units)
            if (not my_units[1]) then return end

            local my_attacks = AH.get_attacks(my_units, { simulate_combat = true })

            for i, att in ipairs(my_attacks) do
                if (att.target.x == enemy_in_way.x) and (att.target.y == enemy_in_way.y) then

                    -- Rating: expected HP of attacker and defender
                    local rating = att.att_stats.average_hp - 2 * att.def_stats.average_hp
                    --print('    rating:', rating)

                    if (rating > max_rating) then
                        max_rating = rating
                        best_attack = att
                    end
                end
            end

            -- If attack on this enemy_in_way is possible, return it
            if (max_rating > -9e99) then return best_attack end

            -- If we got here, that means there's an enemy in the way, but none of the units can reach it
            --> try to fight our way to that enemy
            --print('Find different attack to get to enemy in way')
            for i, att in ipairs(my_attacks) do

                -- Rating: expected HP of attacker and defender
                local rating = att.att_stats.average_hp - 2 * att.def_stats.average_hp

                -- plus, give a huge bonus for closeness to enemy_in_way
                local tmp_defender = wesnoth.get_unit(att.target.x, att.target.y)
                local dist = H.distance_between(enemy_in_way.x, enemy_in_way.y, tmp_defender.x, tmp_defender.y)
                --print('    distance:',enemy_in_way.id, tmp_defender.id, dist)

                rating = rating + 100. / dist
                --print('    rating:', rating)

                if (rating > max_rating) then
                    max_rating = rating
                    best_attack = att
                end
            end

            if (max_rating > -9e99) then
                return best_attack
            else
                return
            end
        end

        -----------------------

        function engine:mai_messenger_attack_eval(cfg)
            -- Attack units in the path of the messenger
            -- id: id of the messenger unit
            -- goal_x, goal_y: coordinates of the goal toward which the messenger moves

            local messenger = wesnoth.get_units{ side = wesnoth.current.side, id = cfg.id }[1]
            if (not messenger) then return 0 end

            -- Set up the waypoints
            cfg.waypoint_x = AH.split(cfg.waypoint_x, ",")
            cfg.waypoint_y = AH.split(cfg.waypoint_y, ",")
            local waypoints = {}
            for i = 1,#cfg.waypoint_x do
                waypoints[i] = { tonumber(cfg.waypoint_x[i]), tonumber(cfg.waypoint_y[i]) }
            end

            -- Variable to store which waypoint to go to next (persistent)
            if (not self.data.next_waypoint) then self.data.next_waypoint = 1 end

            -- If we're within 3 hexes of the next waypoint, we go on to the one after that
            -- except if that one's the last one already
            local dist_wp = H.distance_between(messenger.x, messenger.y,
                waypoints[self.data.next_waypoint][1], waypoints[self.data.next_waypoint][2]
            )
            if (dist_wp <= 3) and (self.data.next_waypoint < #waypoints) then
                self.data.next_waypoint = self.data.next_waypoint + 1
            end

            -- See if there's an enemy in the way that should be attacked
            local attack = self:mai_messenger_find_clearing_attack(messenger,
                waypoints[self.data.next_waypoint][1], waypoints[self.data.next_waypoint][2]
            )

            if attack then
                self.data.best_attack = attack
                return 300000
            end

            return 0
        end

        function engine:mai_messenger_attack_exec()
            local attacker = wesnoth.get_unit(self.data.best_attack.src.x, self.data.best_attack.src.y)
            local defender = wesnoth.get_unit(self.data.best_attack.target.x, self.data.best_attack.target.y)

            AH.movefull_stopunit(ai, attacker, self.data.best_attack.dst.x, self.data.best_attack.dst.y)
            ai.attack(attacker, defender)
            self.data.best_attack = nil
        end

        -----------------------

        function engine:mai_messenger_move_eval(cfg)
            -- Move the messenger (unit with passed id) toward goal, attack adjacent unit if possible
            -- without retaliation or little expected damage with high chance of killing the enemy

            local messenger = wesnoth.get_units{ id = cfg.id, formula = '$this_unit.moves > 0' }[1]

            if messenger then return 290000 end
            return 0
        end

        function engine:mai_messenger_move_exec(cfg)
            local messenger = wesnoth.get_units{ id = cfg.id, formula = '$this_unit.moves > 0' }[1]

            -- Set up the waypoints
            cfg.waypoint_x = AH.split(cfg.waypoint_x, ",")
            cfg.waypoint_y = AH.split(cfg.waypoint_y, ",")
            local waypoints = {}
            for i = 1,#cfg.waypoint_x do
                waypoints[i] = { tonumber(cfg.waypoint_x[i]), tonumber(cfg.waypoint_y[i]) }
            end

            -- Variable to store which waypoint to go to next (persistent)
            if (not self.data.next_waypoint) then self.data.next_waypoint = 1 end

            -- If we're within 3 hexes of the next waypoint, we go on to the one after that
            -- except if that one's the last one already
            local dist_wp = H.distance_between(messenger.x, messenger.y,
                waypoints[self.data.next_waypoint][1], waypoints[self.data.next_waypoint][2]
            )
            if (dist_wp <= 3) and (self.data.next_waypoint < #waypoints) then
                self.data.next_waypoint = self.data.next_waypoint + 1
            end

            -- In case an enemy is on the waypoint hex:
            local x, y = wesnoth.find_vacant_tile(
                waypoints[self.data.next_waypoint][1], waypoints[self.data.next_waypoint][2], messenger
            )
            local next_hop = AH.next_hop(messenger, x, y)

            -- Compare this to the "ideal path"
            local path, cost = wesnoth.find_path(messenger, x, y, { ignore_units = 'yes' })
            local opt_hop, opt_cost = {messenger.x, messenger.y}, 0
            for i, p in ipairs(path) do
                local sub_path, sub_cost = wesnoth.find_path(messenger, p[1], p[2])
                    if sub_cost > messenger.moves then
                    break
                else
                    local unit_in_way = wesnoth.get_unit(p[1], p[2])
                    if not unit_in_way then
                        opt_hop, nh_cost = p, sub_cost
                    end
                end
            end

            --print(next_hop[1], next_hop[2], opt_hop[1], opt_hop[2])
            -- Now compare how long it would take from the end of both of these options
            local x1, y1 = messenger.x, messenger.y
            wesnoth.put_unit(next_hop[1], next_hop[2], messenger)
            local tmp, cost1 = wesnoth.find_path(messenger, x, y, {ignore_units = 'yes'})
            wesnoth.put_unit(opt_hop[1], opt_hop[2], messenger)
            local tmp, cost2 = wesnoth.find_path(messenger, x, y, {ignore_units = 'yes'})
            wesnoth.put_unit(x1, y1, messenger)
           --print(cost1, cost2)

            -- If cost2 is significantly less, that means that the other path might overall be faster
            -- even though it is currently blocked
            if (cost2 + 4 < cost1) then next_hop = opt_hop end
            --print(next_hop[1], next_hop[2])

            if next_hop and ((next_hop[1] ~= messenger.x) or (next_hop[2] ~= messenger.y)) then
                ai.move(messenger, next_hop[1], next_hop[2])
            else
                ai.stopunit_moves(messenger)
            end

            -- We also test whether an attack without retaliation or with little damage is possible
            local targets = wesnoth.get_units {
                { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } },
                { "filter_adjacent", { id = cfg.id } }
            }

            local max_rating, best_tar, best_weapon = -9e99, {}, -1
            for i,t in ipairs(targets) do
                local n_weapon = 0
                for weapon in H.child_range(messenger.__cfg, "attack") do
                    n_weapon = n_weapon + 1

                    local att_stats, def_stats = wesnoth.simulate_combat(messenger, n_weapon, t)

                    local rating = -9e99
                    -- This is an acceptable attack if:
                    -- 1. There is no counter attack
                    -- 2. Probability of death is >=67% for enemy, 0% for attacker (default values)

                    local enemy_death_chance = cfg.enemy_death_chance or 0.67
                    local messenger_death_chance = cfg.messenger_death_chance or 0

                    if (att_stats.hp_chance[messenger.hitpoints] == 1)
                        or (def_stats.hp_chance[0] >= tonumber(enemy_death_chance)) and (att_stats.hp_chance[0] <= tonumber(messenger_death_chance))
                    then
                        rating = t.max_hitpoints + def_stats.hp_chance[0]*100 + att_stats.average_hp - def_stats.average_hp
                    end
                    --print(messenger.id, t.id,weapon.name, rating)
                    if rating > max_rating then
                        max_rating, best_tar, best_weapon = rating, t, n_weapon
                    end
                end
            end

            local target = wesnoth.get_units {
                x = cfg.waypoint_x[#cfg.waypoint_x],
                y = cfg.waypoint_y[#cfg.waypoint_y],
                { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } },
                { "filter_adjacent", { id = cfg.id } }
            }[1]

            if max_rating > -9e99 then
                ai.attack(messenger, best_tar, best_weapon)
            elseif target then
                ai.attack(messenger, target)
            end

            -- Finally, make sure unit is really done after this
            ai.stopunit_attacks(messenger)
        end

        -----------------------

        function engine:mai_messenger_other_move_eval(cfg)
            -- Move other units close to messenger, and in between messenger and enemies
            -- The messenger has moved at this time, so we don't need to exclude him
            -- But we check that he exist (not for this scenario, but for others)
            local messenger = wesnoth.get_units{ side = wesnoth.current.side, id = cfg.id }[1]
            if (not messenger) then return 0 end

            local my_units = wesnoth.get_units{ side = wesnoth.current.side, formula = '$this_unit.moves > 0' }

            if my_units[1] then return 280000 end
            return 0
        end

        function engine:mai_messenger_other_move_exec(cfg)
            local messenger = wesnoth.get_units{ id = cfg.id }[1]
            local my_units = wesnoth.get_units{ side = wesnoth.current.side, formula = '$this_unit.moves > 0' }

            -- Simply move units one at a time
            local next_unit = my_units[1]
            local reach = LS.of_pairs(wesnoth.find_reach(next_unit))

            -- Distance from messenger for each hex the unit can reach
            local dist_messenger = AH.distance_map({messenger}, reach)

            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
            }

            -- Rating (in the end, we pick the _minimum _rating):
            -- 1. Minimize distance from enemies
            local rating = AH.distance_map(enemies, reach)
            -- 2. This one favors hexes in between messenger and enemies
            rating:union_merge(dist_messenger, function(x, y, v1, v2)
                return v1 + v2*#enemies
            end)
            -- 3. Strongly prefer hexes close to the messenger
            rating:union_merge(dist_messenger, function(x, y, v1, v2)
                return v1 + v2^2
            end)
            --AH.put_labels(rating)

            -- Now find hex with minimum value that is unoccupied
            min_rating, best_hex = 9e99, {}
            rating:iter(function(x, y, r)
                local unit_in_way = wesnoth.get_units{ x = x, y = y, { "not", { id = next_unit.id } } }[1]
                if (not unit_in_way) and (r < min_rating) then
                   min_rating, best_hex = r, { x, y }
                end
            end)
            -- and move the unit there
            AH.movefull_stopunit(ai, next_unit, best_hex)
        end

        return engine
    end
}
