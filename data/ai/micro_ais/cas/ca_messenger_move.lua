local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_messenger_move = {}

local messenger_next_waypoint = wesnoth.require "ai/micro_ais/cas/ca_messenger_f_next_waypoint.lua"

function ca_messenger_move:evaluation(ai, cfg)
    -- Move the messenger (unit with passed id) toward goal, attack adjacent unit if possible
    -- without retaliation or little expected damage with high chance of killing the enemy

    local messenger = wesnoth.get_units{ id = cfg.id, formula = '$this_unit.moves > 0' }[1]

    if messenger then
        return cfg.ca_score
    end
    return 0
end

function ca_messenger_move:execution(ai, cfg, self)
    local messenger = wesnoth.get_units{ id = cfg.id, formula = '$this_unit.moves > 0' }[1]

    local x, y = messenger_next_waypoint(messenger, cfg, self)
    if (messenger.x ~= x) or (messenger.y ~= y) then
        x, y = wesnoth.find_vacant_tile( x, y, messenger)
    end
    local next_hop = AH.next_hop(messenger, x, y)
    if (not next_hop) then next_hop = { messenger.x, messenger.y } end

    -- Compare this to the "ideal path"
    local path, cost = wesnoth.find_path(messenger, x, y, { ignore_units = 'yes' })
    local opt_hop, opt_cost = {messenger.x, messenger.y}, 0
    for i, p in ipairs(path) do
        local sub_path, sub_cost = wesnoth.find_path(messenger, p[1], p[2])
            if sub_cost > messenger.moves then
            break
        else
            local unit_in_way = wesnoth.get_unit(p[1], p[2])
            if not unit_in_way then
                opt_hop, nh_cost = p, sub_cost
            end
        end
    end

    --print(next_hop[1], next_hop[2], opt_hop[1], opt_hop[2])
    -- Now compare how long it would take from the end of both of these options
    local x1, y1 = messenger.x, messenger.y
    wesnoth.put_unit(next_hop[1], next_hop[2], messenger)
    local tmp, cost1 = wesnoth.find_path(messenger, x, y, {ignore_units = 'yes'})
    wesnoth.put_unit(opt_hop[1], opt_hop[2], messenger)
    local tmp, cost2 = wesnoth.find_path(messenger, x, y, {ignore_units = 'yes'})
    wesnoth.put_unit(x1, y1, messenger)
    --print(cost1, cost2)

    -- If cost2 is significantly less, that means that the other path might overall be faster
    -- even though it is currently blocked
    if (cost2 + messenger.max_moves/2 < cost1) then next_hop = opt_hop end
    --print(next_hop[1], next_hop[2])

    if next_hop and ((next_hop[1] ~= messenger.x) or (next_hop[2] ~= messenger.y)) then
        AH.checked_move(ai, messenger, next_hop[1], next_hop[2])
    else
        AH.checked_stopunit_moves(ai, messenger)
    end
    if (not messenger) or (not messenger.valid) then return end

    -- We also test whether an attack without retaliation or with little damage is possible
    if (messenger.attacks_left <= 0) then return end
    if (not H.get_child(messenger.__cfg, 'attack')) then return end

    local targets = wesnoth.get_units {
        { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } },
        { "filter_adjacent", { id = cfg.id } }
    }

    local max_rating, best_tar, best_weapon = -9e99, {}, -1
    for i,t in ipairs(targets) do
        local n_weapon = 0
        for weapon in H.child_range(messenger.__cfg, "attack") do
            n_weapon = n_weapon + 1

            local att_stats, def_stats = wesnoth.simulate_combat(messenger, n_weapon, t)

            local rating = -9e99
            -- This is an acceptable attack if:
            -- 1. There is no counter attack
            -- 2. Probability of death is >=67% for enemy, 0% for attacker (default values)

            local enemy_death_chance = cfg.enemy_death_chance or 0.67
            local messenger_death_chance = cfg.messenger_death_chance or 0

            if (att_stats.hp_chance[messenger.hitpoints] == 1)
                or (def_stats.hp_chance[0] >= tonumber(enemy_death_chance)) and (att_stats.hp_chance[0] <= tonumber(messenger_death_chance))
            then
                rating = t.max_hitpoints + def_stats.hp_chance[0]*100 + att_stats.average_hp - def_stats.average_hp
            end
            --print(messenger.id, t.id,weapon.name, rating)
            if rating > max_rating then
                max_rating, best_tar, best_weapon = rating, t, n_weapon
            end
        end
    end

    if max_rating > -9e99 then
        AH.checked_attack(ai, messenger, best_tar, best_weapon)
    else
        -- Otherwise, always attack enemy on last waypoint
        local waypoint_x = AH.split(cfg.waypoint_x, ",")
        local waypoint_y = AH.split(cfg.waypoint_y, ",")
        local target = wesnoth.get_units {
            x = tonumber(waypoint_x[#waypoint_x]),
            y = tonumber(waypoint_y[#waypoint_y]),
            { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } },
            { "filter_adjacent", { id = cfg.id } }
        }[1]

        if target then
            AH.checked_attack(ai, messenger, target)
        end
    end
    if (not messenger) or (not messenger.valid) then return end

    -- Finally, make sure unit is really done after this
    AH.checked_stopunit_attacks(ai, messenger)
end

return ca_messenger_move
