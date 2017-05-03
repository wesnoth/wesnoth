local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local function get_patrol(cfg)
    local filter = H.get_child(cfg, "filter") or { id = cfg.id }
    local patrol = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return patrol
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
    cfg.waypoint_x = AH.split(cfg.waypoint_x, ",")
    cfg.waypoint_y = AH.split(cfg.waypoint_y, ",")
    local n_wp = #cfg.waypoint_x
    local waypoints = {}
    for i = 1,n_wp do
        if patrol_vars.patrol_reverse then
            waypoints[i] = { tonumber(cfg.waypoint_x[n_wp-i+1]), tonumber(cfg.waypoint_y[n_wp-i+1]) }
        else
            waypoints[i] = { tonumber(cfg.waypoint_x[i]), tonumber(cfg.waypoint_y[i]) }
        end
    end

    -- If not set, set next location (first move)
    -- This needs to be in WML format, so that it persists over save/load cycles
    if (not patrol_vars.patrol_x) then
        patrol_vars.patrol_x = waypoints[1][1]
        patrol_vars.patrol_y = waypoints[1][2]
        patrol_vars.patrol_reverse = false
        MAIUV.set_mai_unit_variables(patrol, cfg.ai_id, patrol_vars)
    end

    while patrol.moves > 0 do
        -- Check whether one of the enemies to be attacked is next to the patroller
        -- If so, don't move, but attack that enemy
        local adjacent_enemy = AH.get_attackable_enemies {
            id = cfg.attack,
            { "filter_adjacent", { id = patrol.id } }
        }[1]
        if adjacent_enemy then break end

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

        -- If we're on the last waypoint on one_time_only is set, stop here
        if cfg.one_time_only and
            (patrol.x == waypoints[n_wp][1]) and (patrol.y == waypoints[n_wp][2])
        then
            AH.checked_stopunit_moves(ai, patrol)
        else  -- Otherwise move toward next WP
            local x, y = wesnoth.find_vacant_tile(patrol_vars.patrol_x, patrol_vars.patrol_y, patrol)
            local nh = AH.next_hop(patrol, x, y)
            if nh and ((nh[1] ~= patrol.x) or (nh[2] ~= patrol.y)) then
                AH.checked_move(ai, patrol, nh[1], nh[2])
            else
                AH.checked_stopunit_moves(ai, patrol)
            end
        end
        if (not patrol) or (not patrol.valid) then return end
    end

    -- Attack unit on the last waypoint under all circumstances if cfg.one_time_only is set
    local adjacent_enemy
    if cfg.one_time_only then
        adjacent_enemy = AH.get_attackable_enemies {
            x = waypoints[n_wp][1],
            y = waypoints[n_wp][2],
            { "filter_adjacent", { id = patrol.id } }
        }[1]
    end

    -- Otherwise attack adjacent enemy (if specified)
    if (not adjacent_enemy) then
        adjacent_enemy = AH.get_attackable_enemies {
            id = cfg.attack,
            { "filter_adjacent", { id = patrol.id } }
        }[1]
    end

    if adjacent_enemy then AH.checked_attack(ai, patrol, adjacent_enemy) end
    if (not patrol) or (not patrol.valid) then return end

    AH.checked_stopunit_all(ai, patrol)
end

return ca_patrol
