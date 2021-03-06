------- Spread Poison CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"

local SP_attack

local ca_spread_poison = {}

function ca_spread_poison:evaluation(cfg, data, filter_own)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'spread_poison'
    if AH.print_eval() then AH.print_ts('     - Evaluating spread_poison CA:') end

    local attacks_aspect = ai.aspects.attacks

    local poisoners = {}
    for _,unit in ipairs(attacks_aspect.own) do
        if (unit.attacks_left > 0) and (#unit.attacks > 0) and unit:find_attack { special_id = "poison" }
            and (not unit.canrecruit) and unit:matches(filter_own)
        then
            table.insert(poisoners, unit)
        end
    end

    if (not poisoners[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local target_map = LS.create()
    for _,enemy in ipairs(attacks_aspect.enemy) do
        target_map:insert(enemy.x, enemy.y)
    end

    local attacks = AH.get_attacks(poisoners)
    if (not attacks[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local aggression = ai.aspects.aggression
    if (aggression > 1) then aggression = 1 end
    local avoid_map = LS.of_pairs(ai.aspects.avoid)

    -- Go through all possible attacks with poisoners
    local max_rating, best_attack = - math.huge
    for i,a in ipairs(attacks) do
        if target_map:get(a.target.x, a.target.y) and (not avoid_map:get(a.dst.x, a.dst.y)) then
            local attacker = wesnoth.units.get(a.src.x, a.src.y)
            local defender = wesnoth.units.get(a.target.x, a.target.y)

            -- Don't try to poison a unit that cannot be poisoned
            local cant_poison = defender.status.poisoned or defender.status.unpoisonable

            -- For now, we also simply don't poison units on healing locations (unless standard combat CA does it)
            local defender_terrain = wesnoth.current.map[defender]
            local healing = wesnoth.terrain_types[defender_terrain].healing

            -- Also, poisoning units that would level up through the attack or could level on their turn as a result is very bad
            local about_to_level = defender.max_experience - defender.experience <= (attacker.level * 2 * wesnoth.game_config.combat_experience)

            if (not cant_poison) and (healing == 0) and (not about_to_level) then
                local _, poison_weapon = attacker:find_attack { special_id = "poison" }
                local dst = { a.dst.x, a.dst.y }
                local att_stats, def_stats = BC.simulate_combat_loc(attacker, dst, defender, poison_weapon)
                local _, defender_rating, attacker_rating = BC.attack_rating(attacker, defender, dst, { att_stats = att_stats, def_stats = def_stats })

                -- As this is the spread poison CA, we want to emphasize poison damage more, but only for non-regenerating units.
                -- For regenerating units this is actually a penalty, as the poison might be more useful elsewhere.
                local additional_poison_rating = wesnoth.game_config.poison_amount * (def_stats.poisoned - def_stats.hp_chance[0])
                additional_poison_rating = additional_poison_rating / defender.max_hitpoints * defender.cost
                if defender:ability('regenerate') then
                    additional_poison_rating = - additional_poison_rating
                end

                -- More priority to enemies on strong terrain
                local defense_rating = defender:defense_on(defender_terrain) / 100

                attacker_rating = attacker_rating * (1 - aggression)
                local combat_rating = attacker_rating + defender_rating + additional_poison_rating
                local total_rating = combat_rating + defense_rating

                -- Only do the attack if combat_rating is positive. As there is a sizable
                -- bonus for poisoning, this will be the case for most attacks.
                if (combat_rating > 0) and (total_rating > max_rating) then
                    max_rating, best_attack = total_rating, a
                end
            end
        end
    end

    if best_attack then
        SP_attack = best_attack
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 190000
    end
    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 0
end

function ca_spread_poison:execution(cfg, data)
    local attacker = wesnoth.units.get(SP_attack.src.x, SP_attack.src.y)
    -- If several attacks have poison, this will always find the last one
    local is_poisoner, poison_weapon = attacker:find_attack { special_id = "poison" }

    if AH.print_exec() then AH.print_ts('   Executing spread_poison CA') end
    if AH.show_messages() then wesnoth.wml_actions.message { speaker = attacker.id, message = 'Poison attack' } end

    AH.robust_move_and_attack(ai, attacker, SP_attack.dst, SP_attack.target, { weapon = poison_weapon })

    SP_attack = nil
end

return ca_spread_poison
