local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_aggressive_attack_no_suicide = {}

function ca_aggressive_attack_no_suicide:evaluation(cfg, data)

    local units = wesnoth.get_units {
        side = wesnoth.current.side,
        formula = 'attacks_left > 0'
    }
    if (not units[1]) then return 0 end

    -- Get all possible attacks
    local attacks = AH.get_attacks(units, { include_occupied = true })
    if (not attacks[1]) then return 0 end

    -- Now find the best of the possible attacks
    local max_rating, best_attack = - math.huge, {}
    for i, att in ipairs(attacks) do
        local attacker = wesnoth.get_unit(att.src.x, att.src.y)
        local defender = wesnoth.get_unit(att.target.x, att.target.y)

        local attacker_dst = attacker:clone()
        attacker_dst.x, attacker_dst.y = att.dst.x, att.dst.y

        local att_stats, def_stats = wesnoth.simulate_combat(attacker_dst, defender)

        if (att_stats.hp_chance[0] == 0) then
            local rating = def_stats.hp_chance[0] * 100

            local attacker_damage = attacker.hitpoints - att_stats.average_hp
            local defender_damage = defender.hitpoints - def_stats.average_hp

            rating = rating + defender_damage
            rating = rating - attacker_damage / 10.

            -- Also, take strongest unit first
            rating = rating + attacker.hitpoints / 10.

            if (rating > max_rating) then
                max_rating = rating
                best_attack = att
            end
        end
    end

    if (max_rating > - math.huge) then
        data.attack = best_attack
        return 100000
    end

    return 0
end

function ca_aggressive_attack_no_suicide:execution(cfg, data)
    AH.robust_move_and_attack(ai, data.attack.src, data.attack.dst, data.attack.target)
    data.attack = nil
end

return ca_aggressive_attack_no_suicide
