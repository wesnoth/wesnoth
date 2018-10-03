local AH = wesnoth.require "ai/lua/ai_helper.lua"

local messenger_next_waypoint = wesnoth.require "ai/micro_ais/cas/ca_messenger_f_next_waypoint.lua"

local ca_messenger_move = {}

function ca_messenger_move:evaluation(cfg)
    -- Move the messenger toward goal, potentially attack adjacent unit

    local messenger = messenger_next_waypoint(cfg)

    if messenger then return cfg.ca_score end
    return 0
end

function ca_messenger_move:execution(cfg)
    local messenger, x, y = messenger_next_waypoint(cfg)

    if (messenger.x ~= x) or (messenger.y ~= y) then
        local wp = AH.get_closest_location(
            { x, y },
            { { "not", { { "filter", { { "not", { side = wesnoth.current.side } } } } } } },
            messenger
        )
        x, y = wp[1], wp[2]
    end

    local next_hop = AH.next_hop(messenger, x, y, { ignore_own_units = true } )
    if (not next_hop) then next_hop = { messenger.x, messenger.y } end

    -- Compare this to the "ideal path"
    local path = AH.find_path_with_shroud(messenger, x, y, { ignore_units = 'yes' })
    local optimum_hop, optimum_cost = { messenger.x, messenger.y }, 0
    for _,step in ipairs(path) do
        local sub_path, sub_cost = AH.find_path_with_shroud(messenger, step[1], step[2])
        if sub_cost > messenger.moves then
            break
        else
            local unit_in_way = wesnoth.get_unit(step[1], step[2])
            if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
                unit_in_way = nil
            end

            if unit_in_way and (unit_in_way.side == messenger.side) then
                local reach = AH.get_reachable_unocc(unit_in_way)
                if (reach:size() > 1) then unit_in_way = nil end
            end

            if not unit_in_way then
                optimum_hop, nh_cost = step, sub_cost
            end
        end
    end

    -- Now compare how long it would take from the end of both of these options
    local x_current, y_current = messenger.x, messenger.y

    local unit_in_way = wesnoth.get_unit(next_hop[1], next_hop[2])
    if (unit_in_way == messenger) then unit_in_way = nil end
    if unit_in_way then unit_in_way:extract() end

    messenger.loc = { next_hop[1], next_hop[2] }
    local _, cost1 = AH.find_path_with_shroud(messenger, x, y, { ignore_units = 'yes' })

    local unit_in_way2 = wesnoth.get_unit(optimum_hop[1], optimum_hop[2])
    if (unit_in_way2 == messenger) then unit_in_way2 = nil end
    if unit_in_way2 then unit_in_way2:extract() end

    messenger.loc = { optimum_hop[1], optimum_hop[2] }
    local _, cost2 = AH.find_path_with_shroud(messenger, x, y, { ignore_units = 'yes' })

    messenger.loc = { x_current, y_current }
    if unit_in_way then unit_in_way:to_map() end
    if unit_in_way2 then unit_in_way2:to_map() end

    -- If cost2 is significantly less, that means that the optimum path might
    -- overall be faster even though it is currently blocked
    if (cost2 + messenger.max_moves/2 < cost1) then next_hop = optimum_hop end

    if next_hop and ((next_hop[1] ~= messenger.x) or (next_hop[2] ~= messenger.y)) then
        AH.robust_move_and_attack(ai, messenger, next_hop)
    else
        AH.checked_stopunit_moves(ai, messenger)
    end
    if (not messenger) or (not messenger.valid) then return end

    -- Test whether an attack without retaliation or with little damage is possible
    if (messenger.attacks_left <= 0) then return end
    if (#messenger.attacks == 0) then return end

    local targets = AH.get_attackable_enemies { { "filter_adjacent", { id = messenger.id } } }

    local max_rating, best_target, best_weapon = - math.huge
    for _,target in ipairs(targets) do
        for n_weapon,weapon in ipairs(messenger.attacks) do
            local att_stats, def_stats = wesnoth.simulate_combat(messenger, n_weapon, target)

            local rating = - math.huge
            -- This is an acceptable attack if:
            -- 1. There is no counter attack
            -- 2. Probability of death is >=67% for enemy, 0% for attacker (default values)

            local enemy_death_chance = cfg.enemy_death_chance or 0.67
            local messenger_death_chance = cfg.messenger_death_chance or 0

            if (att_stats.hp_chance[messenger.hitpoints] == 1)
                or (def_stats.hp_chance[0] >= tonumber(enemy_death_chance)) and (att_stats.hp_chance[0] <= tonumber(messenger_death_chance))
            then
                rating = target.max_hitpoints + def_stats.hp_chance[0]*100 + att_stats.average_hp - def_stats.average_hp
            end

            if (rating > max_rating) then
                max_rating, best_target, best_weapon = rating, target, n_weapon
            end
        end
    end

    if best_target then
        AH.checked_attack(ai, messenger, best_target, best_weapon)
    else
        -- Always attack enemy on last waypoint
        local waypoint_x = AH.split(cfg.waypoint_x, ",")
        local waypoint_y = AH.split(cfg.waypoint_y, ",")
        local target = AH.get_attackable_enemies {
            x = tonumber(waypoint_x[#waypoint_x]),
            y = tonumber(waypoint_y[#waypoint_y]),
            { "filter_adjacent", { id = messenger.id } }
        }[1]

        if target then
            AH.checked_attack(ai, messenger, target)
        end
    end
    if (not messenger) or (not messenger.valid) then return end

    AH.checked_stopunit_attacks(ai, messenger)
end

return ca_messenger_move
