local AH = wesnoth.require "ai/lua/ai_helper.lua"
local H = wesnoth.require "lua/helper.lua"

local ca_bottleneck_attack = {}

function ca_bottleneck_attack:evaluation(ai, cfg, self)
    local attackers = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        { "filter_adjacent", {
            { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
        } }
    }
    if (not attackers[1]) then return 0 end

    local max_rating, best_attacker, best_target, best_weapon = -9e99
    for _,attacker in ipairs(attackers) do
        local targets = wesnoth.get_units {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
            { "filter_adjacent", { id = attacker.id } }
        }

        for _,target in ipairs(targets) do
            local n_weapon = 0
            for weapon in H.child_range(attacker.__cfg, "attack") do
                n_weapon = n_weapon + 1

                local att_stats, def_stats = wesnoth.simulate_combat(attacker, n_weapon, target)

                local rating
                -- This is an acceptable attack if:
                -- 1. There is no counter attack
                -- 2. Probability of death is >=67% for enemy, 0% for attacker
                if (att_stats.hp_chance[attacker.hitpoints] == 1)
                    or ((def_stats.hp_chance[0] >= 0.67) and (att_stats.hp_chance[0] == 0))
                then
                    rating = target.max_hitpoints + def_stats.hp_chance[0] * 100
                    rating = rating + att_stats.average_hp - def_stats.average_hp

                    -- If there's a chance to make the kill, unit closest to leveling up goes first,
                    -- otherwise the other way around
                    if (def_stats.hp_chance[0] >= 0.67) then
                        rating = rating + (attacker.experience - attacker.max_experience) / 10.
                    else
                        rating = rating - (attacker.experience - attacker.max_experience) / 10.
                    end
                end

                if rating and (rating > max_rating) then
                    max_rating = rating
                    best_attacker, best_target, best_weapon = attacker, target, n_weapon
                end
            end
        end
    end

    if (not best_attacker) then
        -- In this case we take attacks away from all units
        self.data.BD_bottleneck_attacks_done = true
    else
        self.data.BD_bottleneck_attacks_done = false
        self.data.BD_attacker = best_attacker
        self.data.BD_target = best_target
        self.data.BD_weapon = best_weapon
    end

    return cfg.ca_score
end

function ca_bottleneck_attack:execution(ai, cfg, self)
    if self.data.BD_bottleneck_attacks_done then
        local units = AH.get_units_with_attacks { side = wesnoth.current.side }
        for _,unit in ipairs(units) do
            AH.checked_stopunit_attacks(ai, unit)
        end
    else
        AH.checked_attack(ai, self.data.BD_attacker, self.data.BD_target, self.data.BD_weapon)
    end

    self.data.BD_attacker, self.data.BD_target, self.data.BD_weapon = nil, nil, nil
    self.data.BD_bottleneck_attacks_done = nil
end

return ca_bottleneck_attack
