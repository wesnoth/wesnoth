local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local messenger_next_waypoint = wesnoth.require "ai/micro_ais/cas/ca_messenger_f_next_waypoint.lua"
local best_attack

local function messenger_find_enemies_in_way(messenger, goal_x, goal_y)
    -- Returns the first unit on or next to the path of the messenger
    -- @messenger: proxy table for the messenger unit
    -- @goal_x,@goal_y: coordinates of the goal toward which the messenger moves
    -- Returns proxy table for the first unit found, or nil if none was found

    local path, cost = AH.find_path_with_shroud(messenger, goal_x, goal_y, { ignore_units = true })
    if cost >= 42424242 then return end

    -- The second path hex is the first that is important for the following analysis
    if (not path[2]) then return end

    -- Is there an enemy unit on the second path hex?
    -- This would be caught by the adjacent hex check later, but not in the right order
    local enemy = wesnoth.get_unit(path[2][1], path[2][2])
    if AH.is_attackable_enemy(enemy) then return enemy end

    -- After that, go through adjacent hexes of all the other path hexes
    for i = 2,#path do
        local sub_path, sub_cost = AH.find_path_with_shroud(messenger, path[i][1], path[i][2], { ignore_units = true })
        if (sub_cost <= messenger.moves) then
            for xa,ya in H.adjacent_tiles(path[i][1], path[i][2]) do
                local enemy = wesnoth.get_unit(xa, ya)
                if AH.is_attackable_enemy(enemy) then return enemy end
            end
        else  -- If we've reached the end of the path for this turn
            return
        end
    end
end

local function messenger_find_clearing_attack(messenger, goal_x, goal_y, cfg)
    -- Check if an enemy is in the way of the messenger
    -- If so, find attack that would clear that enemy out of the way
    -- @messenger: proxy table for the messenger unit
    -- @goal_x,@goal_y: coordinates of the goal toward which the messenger moves
    -- Returns proxy table containing the attack, or nil if none was found

    local enemy_in_way = messenger_find_enemies_in_way(messenger, goal_x, goal_y)
    if (not enemy_in_way) then return end

    local filter = H.get_child(cfg, "filter") or { id = cfg.id }
    local units = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        { "not", filter },
        { "and", H.get_child(cfg, "filter_second") }
    }
    if (not units[1]) then return end

    local attacks = AH.get_attacks(units, { simulate_combat = true })

    local max_rating = -9e99
    for _,attack in ipairs(attacks) do
        if (attack.target.x == enemy_in_way.x) and (attack.target.y == enemy_in_way.y) then

            -- Rating: expected HP of attacker and defender
            local rating = attack.att_stats.average_hp - 2 * attack.def_stats.average_hp

            if (rating > max_rating) then
                max_rating, best_attack = rating, attack
            end
        end
    end

    if best_attack then return best_attack end

    -- If we got here, that means there's an enemy in the way, but none of the units can reach it
    --> try to fight our way to that enemy
    for _,attack in ipairs(attacks) do
        -- Rating: expected HP of attacker and defender
        local rating = attack.att_stats.average_hp - 2 * attack.def_stats.average_hp

        -- Give a huge bonus for closeness to enemy_in_way
        local tmp_defender = wesnoth.get_unit(attack.target.x, attack.target.y)
        local dist = wesnoth.map.distance_between(enemy_in_way.x, enemy_in_way.y, tmp_defender.x, tmp_defender.y)

        rating = rating + 100. / dist

        if (rating > max_rating) then
            max_rating, best_attack = rating, attack
        end
    end

    if best_attack then return best_attack end
end

local ca_messenger_attack = {}

function ca_messenger_attack:evaluation(cfg)
    -- Attack units in the path of the messengers

    local messenger, x, y = messenger_next_waypoint(cfg)
    if (not messenger) then return 0 end

    local attack = messenger_find_clearing_attack(messenger, x, y, cfg)

    if attack then
        return cfg.ca_score
    end

    return 0
end

function ca_messenger_attack:execution(cfg)
    AH.robust_move_and_attack(ai, best_attack.src, best_attack.dst, best_attack.target)
    best_attack = nil
end

return ca_messenger_attack
