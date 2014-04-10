local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local ca_protect_unit_attack = {}

function ca_protect_unit_attack:evaluation(ai, cfg, self)
    -- Find possible attacks for the units
    -- This is set up very conservatively
    -- If unit can die in the worst case, it is not done, even if _really_ unlikely

    local units = {}
    for i,id in ipairs(cfg.id) do
        table.insert(units, AH.get_units_with_attacks { id = id }[1])
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
    end
    return 0
end

function ca_protect_unit_attack:execution(ai, cfg, self)
    local attacker = wesnoth.get_unit(self.data.best_attack.src.x, self.data.best_attack.src.y)
    local defender = wesnoth.get_unit(self.data.best_attack.target.x, self.data.best_attack.target.y)

    AH.movefull_stopunit(ai, attacker, self.data.best_attack.dst.x, self.data.best_attack.dst.y)
    if (not attacker) or (not attacker.valid) then return end
    if (not defender) or (not defender.valid) then return end

    AH.checked_attack(ai, attacker, defender)
    self.data.best_attack = nil
end

return ca_protect_unit_attack
