local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local ca_protect_unit_attack, best_attack = {}

function ca_protect_unit_attack:evaluation(cfg)
    -- Find possible attacks for the units
    -- This is set up very conservatively: if a unit can die in the attack
    -- or the counter attack on the enemy turn, it does not attack, even if that's really unlikely

    local units = {}
    for u in H.child_range(cfg, "unit") do
        table.insert(units, AH.get_units_with_attacks { id = u.id }[1])
    end
    if (not units[1]) then return 0 end

    local attacks = AH.get_attacks(units, { simulate_combat = true })
    if (not attacks[1]) then return 0 end

    local enemies = AH.get_attackable_enemies()

    -- Counter attack calculation
    local enemy_attacks = {}
    for _,enemy in ipairs(enemies) do
        local attack_map = BC.get_attack_map_unit(enemy).units
        table.insert(enemy_attacks, { enemy = enemy, attack_map = attack_map })
    end

    -- Set up a counter attack damage table, as many pairs of attacks will be the same
    local counter_damage_table = {}

    local max_rating = -9e99
    for _,attack in pairs(attacks) do
        -- Only consider attack if there is no chance to die or to be poisoned or slowed
        if (attack.att_stats.hp_chance[0] == 0)
            and (attack.att_stats.poisoned == 0)
            and (attack.att_stats.slowed == 0)
        then
            -- Get maximum possible counter attack damage by enemies on their turn
            local unit = wesnoth.get_unit(attack.src.x, attack.src.y)

            local max_counter_damage = 0
            for _,enemy_attack in ipairs(enemy_attacks) do
                if enemy_attack.attack_map:get(attack.dst.x, attack.dst.y) then
                    -- Check first if this attack combination has already been calculated
                    local str = (attack.src.x + attack.src.y * 1000) .. '-' .. (attack.target.x + attack.target.y * 1000)
                    if counter_damage_table[str] then  -- If so, use saved value
                        max_counter_damage = max_counter_damage + counter_damage_table[str]
                    else  -- if not, calculate it and save value
                        -- Go through all weapons, as "best weapon" might be different later on
                        local min_hp = unit.hitpoints
                        for n_weapon,weapon in ipairs(enemy_attack.enemy.attacks) do
                            -- Terrain does not matter for this, we're only interested in the maximum damage
                            local att_stats, def_stats = wesnoth.simulate_combat(enemy_attack.enemy, n_weapon, unit)

                            -- Find minimum HP of our unit
                            local min_hp_weapon = unit.hitpoints
                            for hp,chance in pairs(def_stats.hp_chance) do
                                if (chance > 0) and (hp < min_hp_weapon) then
                                    min_hp_weapon = hp
                                end
                            end
                            if (min_hp_weapon < min_hp) then min_hp = min_hp_weapon end
                        end

                        max_counter_damage = max_counter_damage + unit.hitpoints - min_hp
                        counter_damage_table[str] = unit.hitpoints - min_hp
                    end
                end
            end

            -- Add this to damage possible on this attack
            local min_hp = 1000
            for hp,chance in pairs(attack.att_stats.hp_chance) do
                if (chance > 0) and (hp < min_hp) then
                    min_hp = hp
                end
            end
            local min_outcome = min_hp - max_counter_damage

            -- If this is > 0, consider the attack
            if (min_outcome > 0) then
                local rating = min_outcome + attack.att_stats.average_hp - attack.def_stats.average_hp
                if (rating > max_rating) then
                    max_rating, best_attack = rating, attack
                end
            end

        end
    end

    if best_attack then
        return 95000
    end
    return 0
end

function ca_protect_unit_attack:execution(cfg)
    AH.robust_move_and_attack(ai, best_attack.src, best_attack.dst, best_attack.target)
    best_attack = nil
end

return ca_protect_unit_attack
