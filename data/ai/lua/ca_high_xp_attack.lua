local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"
local M = wesnoth.map

-- Evaluation process:
--
-- Find all enemy units that could be caused to level up by an attack
--  - If only units that would cause them to level up can attack, CA score = 100,010.
--    This means the attack will be done before the default AI attacks, so that AI
--    units do not get used otherwise by the default AI.
--  - If units that would not cause a leveling can also attack, CA score = 99,990,
--    meaning we see whether the default AI attacks that unit with one of those first.
-- We also check whether it is possible to move an own unit out of the way
--
-- Attack rating:
-- 0. If the CTD (chance to die) of the AI unit is larger than the value of
--    aggression for the side, do not do the attack
-- 1. Otherwise, if the attack might result in a kill, do that preferentially:
--     rating = CTD of defender - CTD of attacker
-- 2. Otherwise, if the enemy is poisoned, do not attack (wait for it
--    weaken and attack on a later turn)
-- 3. Otherwise, calculate damage done to enemy (as if it were not leveling) and
--    own unit, expressed in partial loss of unit value (gold) and minimize both.
--    Damage to enemy is minimized because we want to level it with the weakest AI unit,
--    so that we can follow up with stronger units. In addition, use of poison or
--    slow attacks is strongly discouraged. See code for exact equations.

local ca_attack_highxp = {}

function ca_attack_highxp:evaluation(cfg, data)
    -- Note: (most of) the code below is set up to maximize speed. Do not
    -- "simplify" this unless you understand exactly what that means

    local attacks_aspect = ai.aspects.attacks
    local max_unit_level = 0
    local units = {}
    for _,unit in ipairs(attacks_aspect.own) do
        if (unit.attacks_left > 0) and (#unit.attacks > 0) and (not unit.canrecruit) then
            table.insert(units, unit)

            local level = unit.level
            if (level > max_unit_level) then
                max_unit_level = level
            end
        end
    end
    if (not units[1]) then return 0 end

    -- Mark enemies as potential targets if they are close enough to an AI unit
    -- that could trigger them leveling up; this is not a sufficient criterion,
    -- but it is much faster than path finding, so it is done for preselection.
    local target_infos = {}
    for i_t,enemy in ipairs(attacks_aspect.enemy) do
        if AH.is_attackable_enemy(enemy) then
            local XP_to_levelup = enemy.max_experience - enemy.experience
            if (max_unit_level >= XP_to_levelup) then
                local potential_target = false
                local ind_attackers, ind_other_units = {}, {}
                for i_u,unit in ipairs(units) do
                    if (M.distance_between(enemy.x, enemy.y, unit.x, unit.y) <= unit.moves + 1) then
                        if (unit.level >= XP_to_levelup) then
                            potential_target = true
                            table.insert(ind_attackers, i_u)
                        else
                            table.insert(ind_other_units, i_u)
                        end
                    end
                end

                if potential_target then
                    local target_info = {
                        ind_target = i_t,
                        ind_attackers = ind_attackers,
                        ind_other_units = ind_other_units
                    }
                    table.insert(target_infos, target_info)
                end
            end
        end
    end
    if (not target_infos[1]) then return 0 end

    -- The following location sets are used so that we at most need to call
    -- find_reach() and wesnoth.copy_unit() once per unit
    local reaches = LS.create()
    local attacker_copies = LS.create()

    local aggression = ai.aspects.aggression
    local avoid_map = LS.of_pairs(ai.aspects.avoid)
    local max_ca_score, max_rating, best_attack = 0, 0
    for _,target_info in ipairs(target_infos) do
        local target = attacks_aspect.enemy[target_info.ind_target]
        local can_force_level = {}
        local attack_hexes = LS.create()
        for xa,ya in H.adjacent_tiles(target.x, target.y) do
            if (not avoid_map:get(xa, ya)) then
                local unit_in_way = wesnoth.get_unit(xa, ya)

                if AH.is_visible_unit(wesnoth.current.side, unit_in_way) then
                    if (unit_in_way.side == wesnoth.current.side) then
                        local uiw_reach
                        if reaches:get(unit_in_way.x, unit_in_way.y) then
                            uiw_reach = reaches:get(unit_in_way.x, unit_in_way.y)
                        else
                            uiw_reach = wesnoth.find_reach(unit_in_way)
                            reaches:insert(unit_in_way.x, unit_in_way.y, uiw_reach)
                        end

                        -- Check whether the unit to move out of the way has an unoccupied hex to move to.
                        -- We do not deal with cases where a unit can move out of the way for a
                        -- unit that is moving out of the way of the initial unit (etc.).
                        local can_move = false
                        for _,uiw_loc in ipairs(uiw_reach) do
                            -- Unit in the way of the unit in the way
                            local uiw_uiw = wesnoth.get_unit(uiw_loc[1], uiw_loc[2])
                            if (not AH.is_visible_unit(wesnoth.current.side, uiw_uiw)) then
                                can_move = true
                                break
                            end
                        end
                        if (not can_move) then
                            -- Keep this case as the unit in the way might be a potential attacker
                            attack_hexes:insert(xa, ya, unit_in_way.id)
                        else
                            attack_hexes:insert(xa, ya, 'can_move_away')
                        end
                    end
                else
                    attack_hexes:insert(xa, ya, 'empty')
                end
            end
        end

        attack_hexes:iter(function(xa, ya, occupied)
            for _,i_a in ipairs(target_info.ind_attackers) do
                local attacker = units[i_a]
                if (occupied == 'empty') or (occupied == 'can_move_away') then
                    -- If the hex is not blocked, check all potential attackers
                    local reach
                    if reaches:get(attacker.x, attacker.y) then
                        reach = reaches:get(attacker.x, attacker.y)
                    else
                        reach = wesnoth.find_reach(attacker)
                        reaches:insert(attacker.x, attacker.y, reach)
                    end

                    for _,loc in ipairs(reach) do
                        if (loc[1] == xa) and (loc[2] == ya) then
                            local tmp = {
                                ind_attacker = i_a,
                                dst = { x = xa, y = ya },
                                src = { x = attacker.x, y = attacker.y }
                            }
                            table.insert(can_force_level, tmp)
                            break
                        end
                    end
                else
                    -- If hex is blocked by own units, check whether this unit
                    -- is one of the potential attackers
                    if (attacker.id == occupied) then
                        local tmp = {
                            ind_attacker = i_a,
                            dst = { x = xa, y = ya },
                            src = { x = attacker.x, y = attacker.y }
                        }

                        table.insert(can_force_level, tmp)
                    end
                end
            end
        end)

        -- If a leveling attack is possible, check whether any of the other
        -- units (those with too low a level to force leveling up) can get there
        local ca_score = 100010

        attack_hexes:iter(function(xa, ya, occupied)
            if (ca_score == 100010) then  -- cannot break out of the iteration with goto
                for _,i_u in ipairs(target_info.ind_other_units) do
                    local unit = units[i_u]
                    if (occupied == 'empty') or (occupied == 'can_move_away') then
                        -- If the hex is not blocked, check if unit can get there
                        local reach
                        if reaches:get(unit.x, unit.y) then
                            reach = reaches:get(unit.x, unit.y)
                        else
                            reach = wesnoth.find_reach(unit)
                            reaches:insert(unit.x, unit.y, reach)
                        end

                        for _,loc in ipairs(reach) do
                            if (loc[1] == xa) and (loc[2] == ya) then
                                ca_score = 99990
                                goto found_unit
                            end
                        end
                    else
                        -- If hex is blocked by own units, check whether this unit
                        -- is one of the potential attackers
                        if (unit.id == occupied) then
                            ca_score = 99990
                            goto found_unit
                        end
                    end
                end
                -- It is sufficient to find one unit that can get to any attack hex
                ::found_unit::
            end
        end)

        if (ca_score >= max_ca_score) then
            for _,attack_info in ipairs(can_force_level) do
                local attacker = units[attack_info.ind_attacker]
                local attacker_copy
                if attacker_copies:get(attacker.x, attacker.y) then
                    attacker_copy = attacker_copies:get(attacker.x, attacker.y)
                else
                    attacker_copy = wesnoth.copy_unit(attacker)
                    attacker_copies:insert(attacker.x, attacker.y, attacker_copy)
                end

                attacker_copy.x = attack_info.dst.x
                attacker_copy.y = attack_info.dst.y

                -- Choose the attacker that would do the *least* damage.
                -- We want the damage distribution here as if the target were not to level up
                -- the chance to die is the same in either case
                local old_experience = target.experience
                target.experience = 0
                local att_stats, def_stats, att_weapon = wesnoth.simulate_combat(attacker_copy, target)
                target.experience = old_experience

                local rating = -1000
                if (att_stats.hp_chance[0] <= aggression) then
                    if (def_stats.hp_chance[0] > 0) then
                        rating = 5000 + def_stats.hp_chance[0] - att_stats.hp_chance[0]
                    elseif target.status.poisoned then
                        rating = -1002
                    else
                        rating = 1000

                        local enemy_value_loss = (target.hitpoints - def_stats.average_hp) / target.max_hitpoints
                        enemy_value_loss = enemy_value_loss * wesnoth.unit_types[target.type].cost

                        -- We want the _least_ damage to the enemy, so the minus sign is no typo!
                        rating = rating - enemy_value_loss

                        local own_value_loss = (attacker_copy.hitpoints - att_stats.average_hp) / attacker_copy.max_hitpoints
                        own_value_loss = own_value_loss + att_stats.hp_chance[0]
                        own_value_loss = own_value_loss * wesnoth.unit_types[attacker_copy.type].cost

                        rating = rating - own_value_loss

                        -- Strongly discourage poison or slow attacks
                        if att_weapon.poisons or att_weapon.slows then
                            rating = rating - 100
                        end

                        -- Minor penalty if the attack hex is occupied
                        if (attack_hexes:get(attack_info.dst.x, attack_info.dst.y) == 'can_move_away') then
                            rating = rating - 0.001
                        end
                    end
                end

                if (rating > max_rating)
                    or ((rating > 0) and (ca_score > max_ca_score))
                then
                    max_rating = rating
                    max_ca_score = ca_score
                    best_attack = attack_info
                    best_attack.target = { x = target.x, y = target.y }
                    best_attack.ca_score = ca_score
                    -- Also need to save weapon number because attack simulation above
                    -- is for a different situation than the actual units on the map.
                    -- +1 because of difference between Lua and C++ indices
                    best_attack.attack_num = att_weapon.attack_num + 1
                end
            end
        end

    end

    if best_attack then
        data.XP_attack = best_attack
    end

    return max_ca_score
end

function ca_attack_highxp:execution(cfg, data)
    AH.robust_move_and_attack(ai, data.XP_attack.src, data.XP_attack.dst, data.XP_attack.target, { weapon = data.XP_attack.attack_num })
    data.XP_attack = nil
end

return ca_attack_highxp
