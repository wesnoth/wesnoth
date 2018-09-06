------- Spread Poison CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"

local SP_attack

local ca_spread_poison = {}

function ca_spread_poison:evaluation(cfg, data)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'spread_poison'
    if AH.print_eval() then AH.print_ts('     - Evaluating spread_poison CA:') end

    -- If a unit with a poisoned weapon can make an attack, we'll do that preferentially
    -- (with some exceptions)
    local poisoners = AH.get_units_with_attacks { side = wesnoth.current.side,
        { "filter_wml", {
            { "attack", {
                { "specials", {
                    { "poison", { } }
                } }
            } }
        } },
        canrecruit = 'no'
    }
    if (not poisoners[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local attacks = AH.get_attacks(poisoners)
    if (not attacks[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    -- Go through all possible attacks with poisoners
    local max_rating, best_attack = - math.huge
    for i,a in ipairs(attacks) do
        local attacker = wesnoth.get_unit(a.src.x, a.src.y)
        local defender = wesnoth.get_unit(a.target.x, a.target.y)

        -- Don't try to poison a unit that cannot be poisoned
        local cant_poison = defender.status.poisoned or defender.status.unpoisonable

        -- For now, we also simply don't poison units on villages (unless standard combat CA does it)
        local defender_terrain = wesnoth.get_terrain(defender.x, defender.y)
        local on_village = wesnoth.get_terrain_info(defender_terrain).village

        -- Also, poisoning units that would level up through the attack or could level on their turn as a result is very bad
        local about_to_level = defender.max_experience - defender.experience <= (attacker.level * 2)

        if (not cant_poison) and (not on_village) and (not about_to_level) then
            -- Strongest enemy gets poisoned first
            local rating = defender.hitpoints

            -- Always attack enemy leader, if possible
            if defender.canrecruit then rating = rating + 1000 end

            -- Enemies that can regenerate are not good targets
            if defender:ability('regenerate') then rating = rating - 1000 end

            -- More priority to enemies on strong terrain
            local defender_defense = 100 - defender:defense(defender_terrain)
            rating = rating + defender_defense / 4.

            -- For the same attacker/defender pair, go to strongest terrain
            local attacker_terrain = wesnoth.get_terrain(a.dst.x, a.dst.y)
            local attacker_defense = 100 - attacker:defense(attacker_terrain)
            rating = rating + attacker_defense / 2.

            -- And from village everything else being equal
            local is_village = wesnoth.get_terrain_info(attacker_terrain).village
            if is_village then rating = rating + 0.5 end

            if rating > max_rating then
                max_rating, best_attack = rating, a
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
    local attacker = wesnoth.get_unit(SP_attack.src.x, SP_attack.src.y)
    -- If several attacks have poison, this will always find the last one
    local is_poisoner, poison_weapon = AH.has_weapon_special(attacker, "poison")

    if AH.print_exec() then AH.print_ts('   Executing spread_poison CA') end
    if AH.show_messages() then wesnoth.wml_actions.message { speaker = attacker.id, message = 'Poison attack' } end

    AH.robust_move_and_attack(ai, attacker, SP_attack.dst, SP_attack.target, { weapon = poison_weapon })

    SP_attack = nil
end

return ca_spread_poison
