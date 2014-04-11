local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local messenger_next_waypoint = wesnoth.require "ai/micro_ais/cas/ca_messenger_f_next_waypoint.lua"

local function messenger_find_enemies_in_way(unit, goal_x, goal_y)
    -- Returns the first unit on or next to the path of the messenger
    -- unit: proxy table for the messenger unit
    -- goal_x, goal_y: coordinates of the goal toward which the messenger moves
    -- Returns proxy table for the first unit found, or nil if none was found

    local path, cost = wesnoth.find_path(unit, goal_x, goal_y, { ignore_units = true })

    -- If unit cannot get there:
    if cost >= 42424242 then return end

    -- The second path hex is the first that is important for the following analysis
    if (not path[2]) then return end

    -- Is there an enemy unit on the second path hex?
    -- This would be caught by the adjacent hex check later, but not in the right order
    local enemy = wesnoth.get_units { x = path[2][1], y = path[2][2],
        { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } }
    }[1]
    if enemy then
        --print('  enemy on second path hex:',enemy.id)
        return enemy
    end

    -- After that, go through adjacent hexes of all the other path hexes
    for i = 2, #path do
        local path_hex = path[i]
        local sub_path, sub_cost = wesnoth.find_path( unit, path_hex[1], path_hex[2], { ignore_units = true })
        if sub_cost <= unit.moves then
            -- Check for enemy units on one of the adjacent hexes (which includes 2 hexes on path)
            for x, y in H.adjacent_tiles(path_hex[1], path_hex[2]) do
                local enemy = wesnoth.get_units { x = x, y = y,
                    { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } }
                }[1]
                if enemy then
                    --print('  enemy next to path hex:',enemy.id)
                    return enemy
                end
            end
        else  -- If we've reached the end of the path for this turn
            return
        end
    end

    -- If no unit was found, return nil
    return
end

local function messenger_find_clearing_attack(unit, goal_x, goal_y, cfg)
    -- Check if an enemy is in the way of the messenger
    -- If so, find attack that would "clear" that enemy out of the way
    -- unit: proxy table for the messenger unit
    -- goal_x, goal_y: coordinates of the goal toward which the messenger moves
    -- Returns proxy table containing the attack, or nil if none was found

    local enemy_in_way = messenger_find_enemies_in_way(unit, goal_x, goal_y)
    -- If none found, don't attack, just move
    if not enemy_in_way then return end

    local max_rating, best_attack = -9e99, {}
    --print('Finding attacks on',enemy_in_way.name,enemy_in_way.id)

    -- Find all units that can attack this enemy
    local filter = cfg.filter or { id = cfg.id }
    local units = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        { "not", filter },
        { "and", cfg.filter_second }
    }
    if (not units[1]) then return end

    local attacks = AH.get_attacks(units, { simulate_combat = true })

    for i, att in ipairs(attacks) do
        if (att.target.x == enemy_in_way.x) and (att.target.y == enemy_in_way.y) then

            -- Rating: expected HP of attacker and defender
            local rating = att.att_stats.average_hp - 2 * att.def_stats.average_hp
            --print('    rating:', rating)

            if (rating > max_rating) then
                max_rating = rating
                best_attack = att
            end
        end
    end

    -- If attack on this enemy_in_way is possible, return it
    if (max_rating > -9e99) then return best_attack end

    -- If we got here, that means there's an enemy in the way, but none of the units can reach it
    --> try to fight our way to that enemy
    --print('Find different attack to get to enemy in way')
    for i, att in ipairs(attacks) do

        -- Rating: expected HP of attacker and defender
        local rating = att.att_stats.average_hp - 2 * att.def_stats.average_hp

        -- plus, give a huge bonus for closeness to enemy_in_way
        local tmp_defender = wesnoth.get_unit(att.target.x, att.target.y)
        local dist = H.distance_between(enemy_in_way.x, enemy_in_way.y, tmp_defender.x, tmp_defender.y)
        --print('    distance:',enemy_in_way.id, tmp_defender.id, dist)

        rating = rating + 100. / dist
        --print('    rating:', rating)

        if (rating > max_rating) then
            max_rating = rating
            best_attack = att
        end
    end

    if (max_rating > -9e99) then
        return best_attack
    else
        return
    end
end

local ca_messenger_attack = {}

function ca_messenger_attack:evaluation(ai, cfg, self)
    -- Attack units in the path of the messengers
    -- goal_x, goal_y: coordinates of the goal toward which the messenger moves

    local messenger, x, y = messenger_next_waypoint(cfg)
    if (not messenger) then return 0 end

    -- See if there's an enemy in the way that should be attacked
    local attack = messenger_find_clearing_attack(messenger, x, y, cfg)

    if attack then
        self.data.best_attack = attack
        return cfg.ca_score
    end

    return 0
end

function ca_messenger_attack:execution(ai, cfg, self)
    local attacker = wesnoth.get_unit(self.data.best_attack.src.x, self.data.best_attack.src.y)
    local defender = wesnoth.get_unit(self.data.best_attack.target.x, self.data.best_attack.target.y)

    AH.movefull_stopunit(ai, attacker, self.data.best_attack.dst.x, self.data.best_attack.dst.y)
    if (not attacker) or (not attacker.valid) then return end
    if (not defender) or (not defender.valid) then return end

    AH.checked_attack(ai, attacker, defender)
    self.data.best_attack = nil
end

return ca_messenger_attack
