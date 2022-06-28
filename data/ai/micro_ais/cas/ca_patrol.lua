local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.dofile "ai/lua/battle_calcs.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local function get_patrol(cfg)
    local filter = wml.get_child(cfg, "filter") or { id = cfg.id }
    local patrol = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return patrol
end

local function get_best_attack(unit, loc, last_waypoint, cfg)
    local attack_range = cfg.attack_range or 1

    -- The attack calculation can be somewhat expensive, check first if there are enemies within the specified range
    local enemies = AH.get_attackable_enemies(
        { id = cfg.attack, { "filter_location", { x = loc[1], y = loc[2], radius = attack_range } } },
        wesnoth.current.side,
        { ignore_visibility = cfg.attack_invisible_enemies }
    )

    -- An enemy on the last waypoint gets attacked preferentially and independent of
    -- whether its id is given in cfg.attack; but it still needs to be within attack range
    local enemy_last_waypoint
    if last_waypoint then
        enemy_last_waypoint = AH.get_attackable_enemies(
            { x = last_waypoint[1], y = last_waypoint[2] },
            wesnoth.current.side,
            { ignore_visibility = cfg.attack_invisible_enemies }
        )[1]
        if enemy_last_waypoint and (wesnoth.map.distance_between(enemy_last_waypoint, loc) <= attack_range) then
            local already_included = false
            for _,enemy in ipairs(enemies) do
                if (enemy.id == enemy_last_waypoint.id) then
                    already_included = true
                    break
                end
            end
            if (not already_included) then
                table.insert(enemies, enemy_last_waypoint)
            end
        end
    end
    if (#enemies == 0) then return end

    local old_moves, old_loc
    if ((loc[1] ~= unit.x) or (loc[2] ~= unit.y)) then
        old_moves, old_loc = unit.moves, unit.loc
        local _,sub_cost = AH.find_path_with_shroud(unit, loc)
        unit.moves = unit.moves - sub_cost
        unit.loc = loc
    end
    local attacks = AH.get_attacks({ unit }, { ignore_visibility = cfg.attack_invisible_enemies })
    if old_moves then
        unit.moves, unit.loc = old_moves, old_loc
    end

    local max_rating, best_enemy, best_dst = -math.huge, nil, nil
    for _,attack in ipairs(attacks) do
        for _,enemy in ipairs(enemies) do
            if (attack.target.x == enemy.x) and (attack.target.y == enemy.y) then
                local dst = { attack.dst.x, attack.dst.y }
                local rating = BC.attack_rating(unit, enemy, dst)

                -- Prioritize any enemy on the last waypoint
                if enemy_last_waypoint and (enemy_last_waypoint.id == enemy.id) then
                    rating = rating + 1000
                end

                if (rating > max_rating) then
                    max_rating = rating
                    best_enemy = enemy
                    best_dst = dst
                end

                break
            end
        end
    end

    return best_enemy, best_dst
end

local ca_patrol = {}

function ca_patrol:evaluation(cfg)
    if get_patrol(cfg) then return cfg.ca_score end
    return 0
end

function ca_patrol:execution(cfg)
    local patrol = get_patrol(cfg)
    local patrol_vars = MAIUV.get_mai_unit_variables(patrol, cfg.ai_id)

    -- Set up waypoints, taking into account whether 'reverse' is set
    -- This works even the first time, when patrol_vars.patrol_reverse is not set yet
    local waypoints = AH.get_multi_named_locs_xy('waypoint', cfg)
    local n_wp = #waypoints
    if patrol_vars.patrol_reverse then
        local tmp = {}
        for i = 1,n_wp do
            tmp[i] = { waypoints[n_wp-i+1][1], waypoints[n_wp-i+1][2] }
        end
        waypoints = tmp
    end

    -- If not set, set next location (first move)
    -- This needs to be in WML format, so that it persists over save/load cycles
    if (not patrol_vars.patrol_x) then
        patrol_vars.patrol_x = waypoints[1][1]
        patrol_vars.patrol_y = waypoints[1][2]
        patrol_vars.patrol_reverse = false
        MAIUV.set_mai_unit_variables(patrol, cfg.ai_id, patrol_vars)
    end

    -- Check for a possible attack from the patrol's current position first, that
    -- way we can skip the other evaluation if one is found
    local last_waypoint
    if cfg.one_time_only then last_waypoint = waypoints[n_wp] end
    local enemy, dst
    if (patrol.attacks_left > 0) and (#patrol.attacks > 0) then
        enemy, dst = get_best_attack(patrol, patrol.loc, last_waypoint, cfg)
    end

    while (not enemy) and (patrol.moves > 0) do
        -- Also check whether we're next to any unit (enemy or ally) which is on the next waypoint
        local unit_on_wp = AH.get_visible_units(wesnoth.current.side, {
            x = patrol_vars.patrol_x,
            y = patrol_vars.patrol_y,
            { "filter_adjacent", { id = patrol.id } }
        })[1]

        for i,wp in ipairs(waypoints) do
            -- If the patrol is on a waypoint or adjacent to one that is occupied by any unit
            if ((patrol.x == wp[1]) and (patrol.y == wp[2]))
                or (unit_on_wp and ((unit_on_wp.x == wp[1]) and (unit_on_wp.y == wp[2])))
            then
                if (i == n_wp) then
                    -- Move him to the first one (or reverse route), if he's on the last waypoint
                    -- Unless cfg.one_time_only is set
                    if cfg.one_time_only then
                        patrol_vars.patrol_x = waypoints[n_wp][1]
                        patrol_vars.patrol_y = waypoints[n_wp][2]
                        MAIUV.set_mai_unit_variables(patrol, cfg.ai_id, patrol_vars)
                    else
                        -- Go back to first WP or reverse direction
                        if cfg.out_and_back then
                            patrol_vars.patrol_x = waypoints[n_wp-1][1]
                            patrol_vars.patrol_y = waypoints[n_wp-1][2]
                            -- We also need to reverse the waypoints right here, as this might not be the end of the move
                            patrol_vars.patrol_reverse = not patrol_vars.patrol_reverse
                            MAIUV.set_mai_unit_variables(patrol, cfg.ai_id, patrol_vars)

                            local tmp_wp = {}
                            for j,wp2 in ipairs(waypoints) do tmp_wp[n_wp-j+1] = wp2 end
                            waypoints = tmp_wp
                        else
                            patrol_vars.patrol_x = waypoints[1][1]
                            patrol_vars.patrol_y = waypoints[1][2]
                            MAIUV.set_mai_unit_variables(patrol, cfg.ai_id, patrol_vars)
                        end
                    end
                else
                    -- ... else move him on toward the next waypoint
                    patrol_vars.patrol_x = waypoints[i+1][1]
                    patrol_vars.patrol_y = waypoints[i+1][2]
                    MAIUV.set_mai_unit_variables(patrol, cfg.ai_id, patrol_vars)
                end
            end
        end

        -- If we're on the last waypoint and one_time_only is set, stop here
        if cfg.one_time_only and
            (patrol.x == waypoints[n_wp][1]) and (patrol.y == waypoints[n_wp][2])
        then
            AH.checked_stopunit_moves(ai, patrol)
        else  -- Otherwise move toward next WP
            local x, y = wesnoth.paths.find_vacant_hex(patrol_vars.patrol_x, patrol_vars.patrol_y, patrol)
            local nh = AH.next_hop(patrol, x, y)
            if nh and ((nh[1] ~= patrol.x) or (nh[2] ~= patrol.y)) then
                -- Check whether an attackable enemy comes into attack range at any hex along the way
                local path = AH.find_path_with_shroud(patrol, nh[1], nh[2])
                for i = 2,#path do -- The patrol's current position is already checked above
                    local loc = path[i]
                    enemy, dst = get_best_attack(patrol, loc, last_waypoint, cfg)
                    if enemy then
                        nh = loc
                        break
                    end
                end
                AH.checked_move(ai, patrol, nh[1], nh[2])
            else
                AH.checked_stopunit_moves(ai, patrol)
            end
        end
        if (not patrol) or (not patrol.valid) then return end
    end

    -- It is possible that the patrol unexpectedly ends up next to an enemy, e.g. because of an ambush
    if not (enemy) then
        enemy, dst = get_best_attack(patrol, patrol.loc, last_waypoint, cfg)
    end

    -- It is also possible that the patrol cannot make it to 'dst' because of an ambush,
    -- in which case we can check whether the ambusher can/should be attacked.
    -- So we need to execute the move and the attack separately.
    if enemy then
        AH.robust_move_and_attack(ai, patrol, dst)
        if (not patrol) or (not patrol.valid) then return end

        if (patrol.x ~= dst[1]) or (patrol.y ~= dst[2]) then
            enemy, dst = get_best_attack(patrol, patrol.loc, last_waypoint, cfg)
        end
    end

    if enemy then
        AH.robust_move_and_attack(ai, patrol, dst, enemy)
    end
end

return ca_patrol
