return {
    init = function(ai)

        local engine = {}

        local H = wesnoth.require "lua/helper.lua"
        local W = H.set_wml_action_metatable {}
        local LS = wesnoth.require "lua/location_set.lua"
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local BC = wesnoth.require "ai/lua/battle_calcs.lua"

        ----- The parameter selection dialog ------------
        -- We are not using this here, but are keeping the code as a demonstration
        -- how to set up an interactive parameter selection dialog

        local T = H.set_wml_tag_metatable {}
        local _ = wesnoth.textdomain "wesnoth-ai"

        local tooltip_enemy_weight = _"Enemy unit weight: The (negative) weight given to each enemy unit that can reach a potential target location. Default: 100"
        local tooltip_my_unit_weight = _"My unit weight: The (positive) weight given to each of the AI's units that can reach a potential target location. Default: 1"
        local tooltip_distance_weight = _"Goal distance weight: The (negative) weight for each step the unit is away from its goal location.\nDefault: 3 -- thus, by default, being a step closer to the goal is as important as being in reach of 3 of AI's units."
        local tooltip_terrain_weight = _"Terrain defense weight: The (positive) weight of the terrain defense rating for a potential target location.\nDefault: 0.1 -- thus, by default, a difference of 30 in terrain defense rating is as important as being a step closer to the goal."
        local tooltip_bearing = _"Bearing: Everything else being equal, move protected unit toward or away from enemy groups. Default: toward"

        engine.mai_protect_unit_dialog = {
            T.tooltip { id = "tooltip_large" },
            T.helptip { id = "tooltip_large" },
            T.grid {
                T.row {
                    T.column { horizontal_alignment = "left", border = "all", border_size = 5,
                        T.label { definition = "title", label = _"Set AI parameters" }
                    }
                },

                T.row {
                    T.column { horizontal_alignment = "left", border = "all", border_size = 5,
                        T.label { label = _"Click on 'Close' without changing anything to use defaults.\nAll weights must be 0 or larger. Invalid inputs are converted to default values.\nTooltips enabled for all parameters." }
                    }
                },

                T.row { T.column { T.grid {

                    T.row {
                        T.column { horizontal_alignment = "right", border_size = 5, border = "all",
                             T.label { label = _"Enemy unit weight  (default: 100)", tooltip = tooltip_enemy_weight } },
                        T.column {  horizontal_alignment = "left", border_size = 5, border = "all",
                            T.text_box {id = "enemy_weight", tooltip = tooltip_enemy_weight }
                        }
                    },

                    T.row {
                        T.column { horizontal_alignment = "right", border_size = 5, border = "all",
                             T.label { label = _"My unit weight  (default: 1)", tooltip = tooltip_my_unit_weight } },
                        T.column {  horizontal_alignment = "left", border_size = 5, border = "all",
                            T.text_box {id = "my_unit_weight", tooltip = tooltip_my_unit_weight }
                        }
                    },

                    T.row {
                        T.column { horizontal_alignment = "right", border_size = 5, border = "all",
                             T.label { label = _"Goal distance weight  (default: 3)", tooltip = tooltip_distance_weight } },
                        T.column {  horizontal_alignment = "left", border_size = 5, border = "all",
                            T.text_box {id = "distance_weight", tooltip = tooltip_distance_weight }
                        }
                    },

                    T.row {
                        T.column { horizontal_alignment = "right", border_size = 5, border = "all",
                             T.label { label = _"Terrain defense weight  (default: 0.1)", tooltip = tooltip_terrain_weight } },
                        T.column {  horizontal_alignment = "left", border_size = 5, border = "all",
                            T.text_box {id = "terrain_weight", tooltip = tooltip_terrain_weight }
                        }
                    },

                    T.row {
                        T.column { horizontal_alignment = "right", border_size = 5, border = "all",
                            T.label { label = "Bearing  (default: toward)", tooltip = tooltip_bearing }
                        },
                        T.column { horizontal_alignment = "left",
                            T.horizontal_listbox { id = "bearing",
                                T.list_definition { T.row { T.column { border_size = 5, border = "all",
                                   T.toggle_button { id = "direction", tooltip = tooltip_bearing }
                                } } },
                                T.list_data {
                                    T.row { horizontal_alignment = "left", T.column { label = "toward enemy" } },
                                    T.row { horizontal_alignment = "left", T.column { label = "away from enemy" } }
                                }
                            }
                        }
                    }


                } } }

            },
            click_dismiss = true
        }

        function engine.mai_protect_unit_preshow()

            wesnoth.set_dialog_value(engine.data.enemy_weight or 100., "enemy_weight")
            wesnoth.set_dialog_value(engine.data.my_unit_weight or 1., "my_unit_weight")
            wesnoth.set_dialog_value(engine.data.distance_weight or 3., "distance_weight")
            wesnoth.set_dialog_value(engine.data.my_unit_weight or 0.1, "terrain_weight")

            local tmp_bear = engine.data.bearing or 1
            if (tmp_bear ~= 1) then tmp_bear = 2 end  -- -1 in code, but Option #2 in widget
            wesnoth.set_dialog_value (tmp_bear, "bearing")
        end

        function engine.mai_protect_unit_postshow()
            local tmp = tonumber(wesnoth.get_dialog_value("enemy_weight")) or -1
            if (tmp < 0) then tmp = 100 end
            engine.data.enemy_weight = tmp

            local tmp = tonumber(wesnoth.get_dialog_value("my_unit_weight")) or -1
            if (tmp < 0) then tmp = 1 end
            engine.data.my_unit_weight = tmp

            local tmp = tonumber(wesnoth.get_dialog_value("distance_weight")) or -1
            if (tmp < 0) then tmp = 3 end
            engine.data.distance_weight = tmp

            local tmp = tonumber(wesnoth.get_dialog_value("terrain_weight")) or -1
            if (tmp < 0) then tmp = 0.1 end
            engine.data.terrain_weight = tmp

            local tmp = tonumber(wesnoth.get_dialog_value("bearing")) or 1
            if (tmp ~= 1) then tmp = -1 end  -- -1 in code, but Option #2 in widget
            engine.data.bearing = tmp
        end

        --------- The actual AI functions -----------

        function engine:mai_protect_unit_finish_eval(cfg)
            -- If a unit can make it to the goal, this is the first thing that happens
            for i,id in ipairs(cfg.id) do
                local unit = wesnoth.get_units{ id = id, formula = '$this_unit.moves > 0' }[1]
                if unit then
                    local path, cost = wesnoth.find_path(unit, cfg.goal_x[i], cfg.goal_y[i])
                    if (cost <= unit.moves) and ((unit.x ~= cfg.goal_x[i]) or (unit.y ~= cfg.goal_y[i])) then
                        self.data.unit = unit
                        self.data.goal = { cfg.goal_x[i], cfg.goal_y[i] }
                        return 300000
                    end
                end
            end
            return 0
        end

        function engine:mai_protect_unit_finish_exec(...)
            AH.movefull_stopunit(ai, self.data.unit, self.data.goal)
            self.data.unit = nil
            self.data.goal = nil
        end

        function engine:mai_protect_unit_move_eval(cfg)
            -- Always 94000 if one of the units can still move
            local units = {}
            for i,id in ipairs(cfg.id) do
                table.insert(units, wesnoth.get_units{ id = id, formula = '$this_unit.moves > 0' }[1])
            end

            ----- For the time being, we disable the dialog ----
            -- If AI parameters are not set, bring up the dialog
            -- For demo scenario only, delete for real use
            --if (not self.data.enemy_weight) then
            --    W.message { speaker = "narrator", image = "wesnoth-icon.png", message = "Before we get going, you can set some of the AI parameters. If you want to work with the default values, just click on 'Close' in the following dialog." }

            --    local r = wesnoth.show_dialog(self.dialog, self.preshow, self.postshow)

            --    local tmp = 'toward enemy'
            --    if (self.data.bearing == -1) then tmp = 'away from enemy' end
            --    W.message { speaker = "narrator", image = "wesnoth-icon.png", caption = "Parameters set to:", message = "Enemy unit weight = " .. self.data.enemy_weight .. "\nMy unit weight = " .. self.data.my_unit_weight .. "\nGoal distance weight = " .. self.data.distance_weight .. "\nTerrain defense weight = " .. self.data.terrain_weight .. "\nBearing: " .. tmp }
            --end

            if units[1] then
                return 94000
            else
                return 0
            end
        end

        function engine:mai_protect_unit_move_exec(cfg)
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
            --W.message {speaker="narrator", message="My attack map" }

            -- Enemy attack map
            local EAM = BC.get_attack_map(enemy_units).units  -- enemy attack map
            --AH.put_labels(EAM)
            --W.message {speaker="narrator", message="Enemy attack map" }

            -- Now put the units back out there
            for i,u in ipairs(units) do wesnoth.put_unit(u.x, u.y, u) end

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
            --W.message {speaker="narrator", message="Unit reach map" }

            -- Now calculate the enemy inverse distance map
            -- This is done here because we only need it for the hexes the unit can reach
            -- Enemy distance map
            local EIDM = AH.inverse_distance_map(enemy_units, reach_map)
            --AH.put_labels(EIDM)
            --W.message {speaker="narrator", message="Enemy inverse distance map" }

            -- Get a terrain defense map of reachable hexes
            local TDM = LS.create()
            reach_map:iter(function(x, y, data)
                TDM:insert(x, y, 100 - wesnoth.unit_defense(unit, wesnoth.get_terrain(x, y)))
            end)
            --AH.put_labels(TDM)
            --W.message {speaker="narrator", message="Terrain defense map" }

            -- And finally, the goal distance map
            local GDM = LS.create()
            reach_map:iter(function(x, y, data)
                GDM:insert(x, y, H.distance_between(x, y, goal[1], goal[2]))
            end)
            --AH.put_labels(GDM)
            --W.message {speaker="narrator", message="Goal distance map" }

            -- Configuration parameters -- can be set in the (currently disabled) dialog above
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
            --W.message {speaker="narrator", message="Rating" }
            --print("Best rating, hex:", max_rating, best_hex)

            AH.movefull_stopunit(ai, unit, AH.get_LS_xy(best_hex))
        end

        function engine:mai_protect_unit_attack_eval(cfg)
            -- Find possible attacks for the units
            -- This is set up very conservatively
            -- If unit can die in the worst case, it is not done, even if _really_ unlikely

            local units = {}
            for i,id in ipairs(cfg.id) do
                table.insert(units, wesnoth.get_units{ id = id, formula = '$this_unit.attacks_left > 0' }[1])
            end
            if (not units[1]) then return 0 end

            local attacks = AH.get_attacks(units, { simulate_combat = true })

            if (not attacks[1]) then return 0 end
            --print('#attacks',#attacks,ids)

            -- All enemy units
            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
            }

            -- For retaliation calculation:
            -- Find all hexes enemies can attack on their next turn
            local enemy_attacks = {}
            for i,e in ipairs(enemies) do
                local attack_map = BC.get_attack_map_unit(e).units
                table.insert(enemy_attacks, { enemy = e, attack_map = attack_map })
            end

            -- Set up a retaliation table, as many pairs of attacks will be the same
            local retal_table = {}

            local max_rating, best_attack = -9e99, {}
            for i,a in pairs(attacks) do

                --print(i,a.dst.x,a.dst.y)
                --print('  chance to die:',a.att_stats.hp_chance[0])

                -- Only consider if there is no chance to die or to be poisoned or slowed
                if ((a.att_stats.hp_chance[0] == 0) and (a.att_stats.poisoned == 0) and (a.att_stats.slowed == 0)) then

                    -- Get maximum possible retaliation possible by enemies on next turn
                    local my_unit = wesnoth.get_unit(a.src.x, a.src.y)
                    local max_retal = 0

                    for j,ea in ipairs(enemy_attacks) do
                        local can_attack = ea.attack_map:get(a.dst.x, a.dst.y)
                        if can_attack then

                            -- Check first if this attack combination has already been calculated
                            local str = (a.src.x + a.src.y * 1000) .. '-' .. (a.target.x + a.target.y * 1000)
                            --print(str)
                            if retal_table[str] then  -- If so, use saved value
                                --print('    retal already calculated: ',str,retal_table[str])
                                max_retal = max_retal + retal_table[str]
                            else  -- if not, calculate it and save value
                                -- Go thru all weapons, as "best weapon" might be different later on
                                local n_weapon = 0
                                local min_hp = my_unit.hitpoints
                                for weapon in H.child_range(ea.enemy.__cfg, "attack") do
                                    n_weapon = n_weapon + 1

                                    -- Terrain does not matter for this, we're only interested in the maximum damage
                                    local att_stats, def_stats = wesnoth.simulate_combat(ea.enemy, n_weapon, my_unit)

                                    -- Find minimum HP of our unit
                                    -- find the minimum hp outcome
                                    -- Note: cannot use ipairs() because count starts at 0
                                    local min_hp_weapon = my_unit.hitpoints
                                    for hp,chance in pairs(def_stats.hp_chance) do
                                        if ((chance > 0) and (hp < min_hp_weapon)) then
                                            min_hp_weapon = hp
                                        end
                                    end
                                    if (min_hp_weapon < min_hp) then min_hp = min_hp_weapon end
                                end
                                --print('    min_hp:',min_hp, ' max damage:',my_unit.hitpoints-min_hp)
                                max_retal = max_retal + my_unit.hitpoints - min_hp
                                retal_table[str] = my_unit.hitpoints - min_hp
                            end
                        end
                    end
                    --print('  max retaliation:',max_retal)

                    -- and add this to damage possible on this attack
                    -- Note: cannot use ipairs() because count starts at 0
                    local min_hp = 1000
                    for hp,chance in pairs(a.att_stats.hp_chance) do
                        --print(hp,chance)
                        if ((chance > 0) and (hp < min_hp)) then
                            min_hp = hp
                        end
                    end
                    local min_outcome = min_hp - max_retal
                    --print('  min hp this attack:',min_hp)
                    --print('  ave hp defender:   ',a.def_stats.average_hp)
                    --print('  min_outcome',min_outcome)

                    -- If this is >0, consider the attack
                    if (min_outcome > 0) then
                        local rating = min_outcome + a.att_stats.average_hp - a.def_stats.average_hp
                        --print('  rating:',rating,'  min_outcome',min_outcome)
                        if (rating > max_rating) then
                            max_rating, best_attack = rating, a
                        end
                    end

                end
            end
            --print('Max_rating:', max_rating)

            if (max_rating > -9e99) then
                self.data.best_attack = best_attack
                return 95000
            else
                return 0
            end
        end

        function engine:mai_protect_unit_attack_exec()
            local attacker = wesnoth.get_unit(self.data.best_attack.src.x, self.data.best_attack.src.y)
            local defender = wesnoth.get_unit(self.data.best_attack.target.x, self.data.best_attack.target.y)
            --W.message {speaker=attacker.id, message="Attacking" }

            AH.movefull_stopunit(ai, attacker, self.data.best_attack.dst.x, self.data.best_attack.dst.y)
            ai.attack(attacker, defender)
            self.data.best_attack = nil
        end

        return engine
    end
}
