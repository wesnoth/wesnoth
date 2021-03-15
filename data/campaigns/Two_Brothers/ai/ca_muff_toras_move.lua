local M = wesnoth.map
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local muff_toras_move = {}

function muff_toras_move:evaluation()
    local muff_toras = wesnoth.units.find_on_map { id = 'Muff Toras' }[1]

    if muff_toras and (muff_toras.moves > 0) then
        return 15000
    end

    return 0
end

function muff_toras_move:execution()
    local muff_toras = wesnoth.units.find_on_map { id = 'Muff Toras' }[1]
    local units = wesnoth.units.find_on_map { side = 3, { 'not', { id = 'Muff Toras' } } }
    local enemies = AH.get_attackable_enemies()
    local enemy_attack_map = BC.get_attack_map(enemies)

    local go_to = AH.find_best_move(muff_toras, function(x, y)
        -- Note: we use no_random = false, which adds contribution of up to 0.01.
        -- This means all differences between deterministic ratings need to be
        -- larger than that

        local rating = -10000 -- This is the base rating if no other units are left

        -- Main rating is distance from the closest own unit
        local min_dist = math.huge
        for _,unit in ipairs(units) do
            local dist = M.distance_between(x, y, unit.x, unit.y)
            if (dist < min_dist) then
                min_dist = dist
            end
        end
        if min_dist then rating = -100 * min_dist end

        -- Enemy threat is next most important. We could put a scaling factor
        -- in front of this, but using 1 seems to work pretty well
        local enemy_hp = enemy_attack_map.hitpoints:get(x,y)
        if enemy_hp then
           rating = rating - enemy_hp
        end

        -- All else being equal, go with good terrain
        local hit_chance = muff_toras:defense(wesnoth.current.map[{x, y}])
        rating = rating - hit_chance

        -- Finally, we want to run away from enemies if there are no other factors.
        -- This mostly kicks in after the other AI units have been killed.
        local closest_enemy_dist
        for _,enemy in ipairs(enemies) do
            local dist = M.distance_between(x, y, enemy.x, enemy.y)
            if (not closest_enemy_dist) or (dist < closest_enemy_dist) then
                closest_enemy_dist = dist
            end
        end
        rating = rating + closest_enemy_dist

        return rating
    end)

    if go_to and ((go_to[1] ~= muff_toras.x) or (go_to[2] ~= muff_toras.y)) then
        AH.robust_move_and_attack(ai, muff_toras, go_to)
    end

    -- Test whether an attack without retaliation or with little damage is possible
    if (muff_toras.attacks_left <= 0) or (#muff_toras.attacks == 0) then
        return
    end

    local targets = AH.get_attackable_enemies { { "filter_adjacent", { id = muff_toras.id } } }

    local max_rating, best_target, best_weapon = - math.huge
    for _,target in ipairs(targets) do
        for n_weapon,weapon in ipairs(muff_toras.attacks) do
            local att_stats, def_stats = wesnoth.simulate_combat(muff_toras, n_weapon, target)

            local rating = - math.huge
            -- This is an acceptable attack if:
            -- 1. There is no counter attack
            -- 2. Probability of death is >=67% for enemy, 0% for attacker (default values)

            local enemy_death_chance = 0.67
            local muff_toras_death_chance = 0

            if (att_stats.hp_chance[muff_toras.hitpoints] == 1)
                or (def_stats.hp_chance[0] >= enemy_death_chance) and (att_stats.hp_chance[0] <= muff_toras_death_chance)
            then
                rating = target.max_hitpoints + def_stats.hp_chance[0] * 100 + att_stats.average_hp - def_stats.average_hp
            end

            if (rating > max_rating) then
                max_rating, best_target, best_weapon = rating, target, n_weapon
            end
        end
    end

    if best_target then
        AH.checked_attack(ai, muff_toras, best_target, best_weapon)
    end
    if (not muff_toras) or (not muff_toras.valid) then
        return
    end

    AH.checked_stopunit_attacks(ai, muff_toras)
end

return muff_toras_move
