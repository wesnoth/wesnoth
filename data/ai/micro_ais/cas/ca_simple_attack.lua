local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"

local ca_simple_attack, best_attack = {}

function ca_simple_attack:evaluation(cfg)
    local units = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter") }
    }
    if (not units[1]) then return 0 end

    -- If cfg.filter_second is set, set up a map (location set)
    -- of enemies that it is okay to attack
    local enemy_filter = H.get_child(cfg, "filter_second")
    local enemy_map
    if enemy_filter then
        local enemies = AH.get_attackable_enemies(enemy_filter)
        if (not enemies[1]) then return 0 end

        enemy_map = LS.create()
        for _,enemy in ipairs(enemies) do enemy_map:insert(enemy.x, enemy.y) end
    end

    -- Now find the best of the possible attacks
    local attacks = AH.get_attacks(units, { include_occupied = true })
    if (not attacks[1]) then return 0 end

    local max_rating = -9e99
    for _, att in ipairs(attacks) do
        local valid_target = true
        if enemy_filter and (not enemy_map:get(att.target.x, att.target.y)) then
            valid_target = false
        end

        if valid_target then
            local attacker = wesnoth.get_unit(att.src.x, att.src.y)
            local enemy = wesnoth.get_unit(att.target.x, att.target.y)
            local dst = { att.dst.x, att.dst.y }

            local rating = BC.attack_rating(attacker, enemy, dst)

            if (rating > max_rating) then
                max_rating, best_attack = rating, att
            end
        end
    end

    if best_attack then
        return cfg.ca_score
    end

    return 0
end

function ca_simple_attack:execution(cfg)
    AH.robust_move_and_attack(ai, best_attack.src, best_attack.dst, best_attack.target, cfg)
    best_attack = nil
end

return ca_simple_attack
