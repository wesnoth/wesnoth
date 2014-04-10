local AH = wesnoth.require "ai/lua/ai_helper.lua"
local H = wesnoth.require "lua/helper.lua"

local ca_bottleneck_attack = {}

function ca_bottleneck_attack:evaluation(ai, cfg, self)
    -- All units with attacks_left and enemies next to them
    -- This will be much easier once the 'attacks' variable is implemented
    local attackers = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        { "filter_adjacent", {
            { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
        } }
    }
    --print("\n\nAttackers:",#attackers)
    if (not attackers[1]) then return 0 end

    -- Now loop through the attackers, and all attacks for each
    local max_rating, best_att, best_tar, best_weapon = 0, {}, {}, -1
    for i,a in ipairs(attackers) do
        local targets = wesnoth.get_units {
            { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } },
            { "filter_adjacent", { id = a.id } }
        }
        --print("  ",a.id,#targets)

        for j,t in ipairs(targets) do
            local n_weapon = 0
            for weapon in H.child_range(a.__cfg, "attack") do
                n_weapon = n_weapon + 1

                local att_stats, def_stats = wesnoth.simulate_combat(a, n_weapon, t)

                local rating = 0
                -- This is an acceptable attack if:
                -- 1. There is no counter attack
                -- 2. Probability of death is >=67% for enemy, 0% for attacker
                if (att_stats.hp_chance[a.hitpoints] == 1)
                    or ((def_stats.hp_chance[0] >= 0.67) and (att_stats.hp_chance[0] == 0))
                then
                    rating = 1000 + t.max_hitpoints + def_stats.hp_chance[0]*100 + att_stats.average_hp - def_stats.average_hp
                    -- if there's a chance to make the kill, unit closest to leveling up goes first, otherwise the other way around
                    if (def_stats.hp_chance[0] >= 0.67) then
                        rating = rating + (a.experience - a.max_experience) / 10.
                    else
                        rating = rating - (a.experience - a.max_experience) / 10.
                    end
                end
                --print(a.id, t.id,weapon.name, rating)
                if rating > max_rating then
                    max_rating, best_att, best_tar, best_weapon = rating, a, t, n_weapon
                end
            end
        end
    end
    --print("Best attack:",best_att.id, best_tar.id, max_rating, best_weapon)

    if max_rating == 0 then
        -- In this case we take attacks away from all units
        -- This is done so that the RCA AI CAs can be kept in place
        self.data.bottleneck_attacks_done = true
    else
        self.data.bottleneck_attacks_done = false
        self.data.attacker = best_att
        self.data.target = best_tar
        self.data.weapon = best_weapon
    end
    return cfg.ca_score
end

function ca_bottleneck_attack:execution(ai, cfg, self)
    if self.data.bottleneck_attacks_done then
        local units = AH.get_units_with_attacks { side = wesnoth.current.side }
        for i,u in ipairs(units) do
            AH.checked_stopunit_attacks(ai, u)
        end
    else
        AH.checked_attack(ai, self.data.attacker, self.data.target, self.data.weapon)
    end

    self.data.attacker, self.data.target, self.data.weapon = nil, nil, nil
    self.data.bottleneck_attacks_done = nil
end

return ca_bottleneck_attack
