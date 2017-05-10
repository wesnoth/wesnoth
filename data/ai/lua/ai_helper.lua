local H = wesnoth.require "helper"
local W = H.set_wml_action_metatable {}
local LS = wesnoth.require "location_set"

-- This is a collection of Lua functions used for custom AI development.
-- Note that this is still work in progress with significant changes occurring
-- frequently. Backward compatibility cannot be guaranteed at this time in
-- development releases, but it is of course easily possible to copy a function
-- from a previous release directly into an add-on if it is needed there.
--
-- Invisible units ('viewing_side' parameter):
-- With their default settings, the ai_helper functions use the vision a player of
-- the respective side would see, that is, they assume no knowledge of invisible
-- units. This can be influenced with the 'viewing_side' parameter, which works
-- in the same way as it does in wesnoth.find_reach() and wesnoth.find_path():
--   - If set to a valid side number, vision for that side is used
--   - If set to an invalid side number (e.g. 0), all units on the map are seen
--   - If omitted and a function takes a a parameter linked to a specific side,
--     such as a side number or a unit, as input, vision of that side is used. In
--     this case, viewing_side is passed as part of the optional @cfg configuration
--     table and can be passed from function to function.
--   - If omitted and the function takes no such input, viewing_side is made a
--     required parameter in order to avoid mismatches between the default values
--     of different functions.
--
-- Path finding:
-- All ai_helper functions disregard shroud for path finding (while still ignoring
-- hidden units correctly) as of Wesnoth 1.13.7. This is consistent with default
-- Wesnoth AI behavior and ensures that Lua AIs, including the Micro AIs, can be
-- used for AI sides with shroud=yes. It is accomplished by using
-- ai_helper.find_path_with_shroud() instead of wesnoth.find_path().

local ai_helper = {}

----- Debugging helper functions ------

function ai_helper.show_messages()
    -- Returns true or false (hard-coded). To be used to
    -- show messages if in debug mode.
    -- Just edit the following line (easier than trying to set WML variable)
    local show_messages_flag = false
    if wesnoth.game_config.debug then return show_messages_flag end
    return false
end

function ai_helper.print_exec()
    -- Returns true or false (hard-coded). To be used to
    -- show which CA is being executed if in debug mode.
    -- Just edit the following line (easier than trying to set WML variable)
    local print_exec_flag = false
    if wesnoth.game_config.debug then return print_exec_flag end
    return false
end

function ai_helper.print_eval()
    -- Returns true or false (hard-coded). To be used to
    -- show which CA is being evaluated if in debug mode.
    -- Just edit the following line (easier than trying to set WML variable)
    local print_eval_flag = false
    if wesnoth.game_config.debug then return print_eval_flag end
    return false
end

function ai_helper.done_eval_messages(start_time, ca_name)
    ca_name = ca_name or 'unknown'
    local dt = wesnoth.get_time_stamp() / 1000. - start_time
    if ai_helper.print_eval() then
        ai_helper.print_ts_delta(start_time, '       - Done evaluating ' .. ca_name .. ':')
    end
end

function ai_helper.clear_labels()
    -- Clear all labels on a map
    local width, height = wesnoth.get_map_size()
    for x = 1,width do
        for y = 1,height do
            W.label { x = x, y = y, text = "" }
        end
    end
end

function ai_helper.put_labels(map, cfg)
    -- Take @map (location set) and put label containing 'value' onto the map.
    -- Print 'nan' if element exists but is not a number.
    -- @cfg: table with optional configuration parameters:
    --   - show_coords: (boolean) use hex coordinates as labels instead of value
    --   - factor=1: (number) if value is a number, multiply by this factor
    --   - keys: (array) if the value to be displayed is a subelement of the LS data,
    --     use these keys to access it. For example, if we want to display data[3]
    --     set keys = { 3 }, if it's data.arg[3], set keys = { 'arg', 3 }

    cfg = cfg or {}
    local factor = cfg.factor or 1

    ai_helper.clear_labels()

    map:iter(function(x, y, data)
        local out
        if cfg.show_coords then
            out = x .. ',' .. y
        else
            if cfg.keys then
                for _,key in ipairs(cfg.keys) do data = data[key] end
            end
            if (type(data) == 'string') then
                out = data
            else
                out = tonumber(data) or 'nan'
            end
        end

        if (type(out) == 'number') then out = out * factor end
        W.label { x = x, y = y, text = out }
    end)
end

function ai_helper.print_ts(...)
    -- Print arguments preceded by a time stamp in seconds
    -- Also return that time stamp

    local ts = wesnoth.get_time_stamp() / 1000.

    local arg = { ... }
    arg[#arg+1] = string.format('[ t = %.3f ]', ts)

    print(table.unpack(arg))

    return ts
end

function ai_helper.print_ts_delta(start_time, ...)
    -- @start_time: time stamp in seconds as returned by wesnoth.get_time_stamp / 1000.

    -- Same as ai_helper.print_ts(), but also adds time elapsed since
    -- the time given in the first argument (in seconds)
    -- Returns time stamp as well as time elapsed

    local ts = wesnoth.get_time_stamp() / 1000.
    local delta = ts - start_time

    local arg = { ... }
    arg[#arg+1] = string.format('[ t = %.3f, dt = %.3f ]', ts, delta)

    print(table.unpack(arg))

    return ts, delta
end

----- AI execution helper functions ------

function ai_helper.is_incomplete_move(check)
    if (not check.ok) then
        -- Legitimately interrupted moves have the following error codes:
        -- E_AMBUSHED = 2005
        -- E_FAILED_TELEPORT = 2006,
        -- E_NOT_REACHED_DESTINATION = 2007
        if (check.status == 2005) or (check.status == 2006) or (check.status == 2007) then
            return check.status
        end
    end

    return false
end

function ai_helper.is_incomplete_or_empty_move(check)
    if (not check.ok) then
        -- Empty moves have the following error code:
        -- E_EMPTY_MOVE = 2001
        if ai_helper.is_incomplete_move(check) or (check.status == 2001) then
            return check.status
        end
    end

    return false
end

function ai_helper.dummy_check_action(gamestate_changed, ok, result, status)
    return {
        gamestate_changed = gamestate_changed or false,
        ok = ok or false,
        result = result or 'ai_helper::DUMMY_FAILED_ACTION',
        status = status or 99999
    }
end

function ai_helper.checked_action_error(action, error_code)
    if wesnoth.game_config.debug then
        error(action .. ' could not be executed. Error code: ' .. error_code, 3)
    end
end

function ai_helper.checked_attack(ai, attacker, defender, weapon)
    local check = ai.check_attack(attacker, defender, weapon)

    if (not check.ok) then
        ai.stopunit_attacks(attacker)
        ai_helper.checked_action_error('ai.attack from ' .. attacker.x .. ',' .. attacker.y .. ' to ' .. defender.x .. ',' .. defender.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.attack(attacker, defender, weapon)
end

function ai_helper.checked_move_core(ai, unit, x, y, move_type)
    local check = ai.check_move(unit, x, y)

    if (not check.ok) then
        if (not ai_helper.is_incomplete_or_empty_move(check)) then
            ai.stopunit_moves(unit)
            ai_helper.checked_action_error(move_type .. ' from ' .. unit.x .. ',' .. unit.y .. ' to ' .. x .. ',' .. y, check.status .. ' (' .. check.result .. ')')
            return check
        end
    end

    if (move_type == 'ai.move_full') then
        return ai.move_full(unit, x, y)
    else
        return ai.move(unit, x, y)
    end
end

function ai_helper.checked_move_full(ai, unit, x, y)
    return ai_helper.checked_move_core(ai, unit, x, y, 'ai.move_full')
end

function ai_helper.checked_move(ai, unit, x, y)
    return ai_helper.checked_move_core(ai, unit, x, y, 'ai.move')
end

function ai_helper.checked_recruit(ai, unit_type, x, y)
    local check = ai.check_recruit(unit_type, x, y)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.recruit of ' .. unit_type .. ' at ' .. x .. ',' .. y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.recruit(unit_type, x, y)
end

function ai_helper.checked_stopunit_all(ai, unit)
    local check = ai.check_stopunit(unit)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.stopunit_all of ' .. unit.x .. ',' .. unit.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.stopunit_all(unit)
end

function ai_helper.checked_stopunit_attacks(ai, unit)
    local check = ai.check_stopunit(unit)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.stopunit_attacks of ' .. unit.x .. ',' .. unit.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.stopunit_attacks(unit)
end

function ai_helper.checked_stopunit_moves(ai, unit)
    local check = ai.check_stopunit(unit)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.stopunit_moves of ' .. unit.x .. ',' .. unit.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.stopunit_moves(unit)
end

function ai_helper.robust_move_and_attack(ai, src, dst, target_loc, cfg)
    -- Perform a move and/or attack with an AI unit in a way that is robust against
    -- unexpected outcomes such as being ambushed or changes caused by WML events.
    -- As much as possible, this function also tries to ensure that the gamestate
    -- is changed in case an action turns out to be impossible due to such an
    -- unexpected outcome.
    --
    -- Required input parameters:
    -- @ai: the Lua ai table
    -- @src: current coordinates of the AI unit to be used
    -- @dst: coordinates to which the unit should move. This does not have to be
    --   different from @src. In fact, the unit does not even need to have moves
    --   left, as long as an attack is specified in the latter case. If another
    --   AI unit is at @dst, it is moved out of the way.
    --
    -- Optional parameters:
    -- @target_loc: coordinates of the enemy unit to be attacked. If not given, no
    --   attack is attempted.
    -- @cfg: table with optional configuration parameters:
    --   partial_move: By default, this function performs full moves. If this
    --     parameter is true, a partial move is done instead.
    --   weapon: The number (starting at 1) of the attack weapon to be used.
    --     If omitted, the best weapon is automatically selected.
    --   all optional parameters for ai_helper.move_unit_out_of_way()

    -- Notes:
    -- - @src, @dst and @target_loc can be any table (including proxy units) that contains
    --   the coordinates of the respective locations using either indices .x/.y or [1]/[2].
    --   If both are given, .x/.y takes precedence over [1]/[2].
    -- - This function only safeguards AI moves against outcomes that the AI cannot know
    --   about, such as hidden units and WML events. It is assumed that the potential
    --   move was tested for general feasibility (units are on AI side and have moves
    --   left, terrain is passable, etc.) beforehand. If that is not done, it might
    --   lead to very undesirable behavior, incl. the CA being blacklisted or even the
    --   entire AI turn being ended.

    local src_x, src_y = src.x or src[1], src.y or src[2] -- this works with units or locations
    local dst_x, dst_y = dst.x or dst[1], dst.y or dst[2]

    local unit = wesnoth.get_unit(src_x, src_y)
    if (not unit) then
        return ai_helper.dummy_check_action(false, false, 'robust_move_and_attack::NO_UNIT')
    end

    -- Getting target at beginning also, in case events mess up things along the way
    local target, target_x, target_y
    if target_loc then
        target_x, target_y = target_loc.x or target_loc[1], target_loc.y or target_loc[2]
        target = wesnoth.get_unit(target_x, target_y)

        if (not target) then
            return ai_helper.dummy_check_action(false, false, 'robust_move_and_attack::NO_TARGET')
        end
    end

    local gamestate_changed = false
    local move_result
    if (unit.moves > 0) then
        if (src_x == dst_x) and (src_y == dst_y) then
            move_result = ai.stopunit_moves(unit)

            -- The only possible failure modes are non-recoverable (such as E_NOT_OWN_UNIT)
            if (not move_result.ok) then return move_result end

            if (not unit) or (not unit.valid) then
                return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_DISAPPEARED')
            end

            gamestate_changed = true
        else
            local unit_old_moves = unit.moves

            local unit_in_way = wesnoth.get_unit(dst_x, dst_y)
            if unit_in_way and (unit_in_way.side == wesnoth.current.side) and (unit_in_way.moves > 0) then
                local uiw_old_moves = unit_in_way.moves
                ai_helper.move_unit_out_of_way(ai, unit_in_way, cfg)

                if (not unit_in_way) or (not unit_in_way.valid) then
                    return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_IN_WAY_DISAPPEARED')
                end

                -- Failed move out of way: abandon remaining actions
                if (unit_in_way.x == dst_x) and (unit_in_way.y == dst_y) then
                    if (unit_in_way.moves == uiw_old_moves) then
                        -- Forcing a gamestate change, if necessary
                        ai.stopunit_moves(unit_in_way)
                    end
                    return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_IN_WAY_EMPTY_MOVE')
                end

                -- Check whether dst hex is free now (an event could have done something funny)
                local unit_in_way = wesnoth.get_unit(dst_x, dst_y)
                if unit_in_way then
                    return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::ANOTHER_UNIT_IN_WAY')
                end

                gamestate_changed = true
            end

            if (not unit) or (not unit.valid) or (unit.x ~= src_x) or (unit.y ~= src_y) then
                return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_DISAPPEARED')
            end

            local check_result = ai.check_move(unit, dst_x, dst_y)
            if (not check_result.ok) then
                if (not ai_helper.is_incomplete_or_empty_move(check_result)) then
                    if (not gamestate_changed) then
                        ai.stopunit_moves(unit)
                    end
                    return check_result
                end
            end

            if cfg and cfg.partial_move then
                move_result = ai.move(unit, dst_x, dst_y)
            else
                move_result = ai.move_full(unit, dst_x, dst_y)
            end
            if (not move_result.ok) then return move_result end

            if (not unit) or (not unit.valid) then
                return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_DISAPPEARED')
            end

            -- Failed move: abandon rest of actions
            if (unit.x == src_x) and (unit.y == src_y) then
                if (not gamestate_changed) and (unit.moves == unit_old_moves) then
                    -- Forcing a gamestate change, if necessary
                    ai.stopunit_moves(unit)
                end
                return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNPLANNED_EMPTY_MOVE')
            end

            gamestate_changed = true
        end
    end

    -- Tests after the move, before continuing to attack, to ensure WML events
    -- did not do something funny
    if (not unit) or (not unit.valid) then
        return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_DISAPPEARED')
    end
    if (unit.x ~= dst_x) or (unit.y ~= dst_y) then
        return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_NOT_AT_DESTINATION')
    end

    -- In case all went well and there's no attack to be done
    if (not target_x) then return move_result end

    if (not target) or (not target.valid) then
        return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::TARGET_DISAPPEARED')
    end
    if (target.x ~= target_x) or (target.y ~= target_y) then
        return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::TARGET_MOVED')
    end

    local weapon = cfg and cfg.weapon
    local old_attacks_left = unit.attacks_left

    local check_result = ai.check_attack(unit, target, weapon)
    if (not check_result.ok) then
        if (not gamestate_changed) then
            ai.stopunit_all(unit)
        end
        return check_result
    end

    move_result = ai.attack(unit, target, weapon)
    -- This should not happen, given that we just checked, but just in case
    if (not move_result.ok) then return move_result end

    if (not unit) or (not unit.valid) then
        return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::UNIT_DISAPPEARED')
    end

    if (unit.attacks_left == old_attacks_left) and (not gamestate_changed) then
        ai.stopunit_all(unit)
        return ai_helper.dummy_check_action(true, false, 'robust_move_and_attack::NO_ATTACK')
    end

    return move_result
end

----- General functionality and maths helper functions ------

function ai_helper.filter(input, condition)
    -- Equivalent of filter() function in Formula AI

    local filtered_table = {}

    for _,v in ipairs(input) do
        if condition(v) then
            table.insert(filtered_table, v)
        end
    end

    return filtered_table
end

function ai_helper.choose(input, value)
    -- Equivalent of choose() function in Formula AI
    -- Returns element of a table with the largest @value (a function)
    -- Also returns the max value and the index

    local max_value, best_input, best_key = -9e99
    for k,v in pairs(input) do
        if value(v) > max_value then
            max_value, best_input, best_key = value(v), v, k
        end
    end

    return best_input, max_value, best_key
end

function ai_helper.table_copy(t)
    -- Make a copy of a table (rather than just another pointer to the same table)
    local copy = {}
    for k,v in pairs(t) do copy[k] = v end
    return copy
end

function ai_helper.array_merge(a1, a2)
    -- Merge two arrays without overwriting @a1 or @a2 -> create a new table
    -- This only works with arrays, not general tables
    local merger = {}
    for _,a in pairs(a1) do table.insert(merger, a) end
    for _,a in pairs(a2) do table.insert(merger, a) end
    return merger
end

function ai_helper.serialize(input)
    -- Convert @input to a string in a format corresponding to the type of @input
    -- The string is all put into one line

    local str = ''
    if (type(input) == "number") or (type(input) == "boolean") then
        str = tostring(input)
    elseif type(input) == "string" then
        str = string.format("%q", input)
    elseif type(input) == "table" then
        str = str .. "{ "
        for k,v in pairs(input) do
            str = str .. "[" .. ai_helper.serialize(k)  .. "] = "
            str = str .. ai_helper.serialize(v)
            str = str .. ", "
        end
        str = str .. "}"
    else
        error("cannot serialize a " .. type(input), 2)
    end

    return str
end

function ai_helper.split(str, sep)
    -- Split string @str into a table using the delimiter @sep (default: ',')

    local sep, fields = sep or ",", {}
    local pattern = string.format("([^%s]+)", sep)
    string.gsub(str, pattern, function(c) fields[#fields+1] = c end)
    return fields
end

--------- Location set related helper functions ----------

function ai_helper.get_LS_xy(index)
    -- Get the x,y coordinates for the index of a location set
    -- For some reason, there doesn't seem to be a LS function for this

    local tmp_set = LS.create()
    tmp_set.values[index] = 1
    local xy = tmp_set:to_pairs()[1]

    return xy[1], xy[2]
end

function ai_helper.LS_random_hex(map)
    -- Select a random hex from the hexes in location set @map
    -- This seems "inelegant", but I can't come up with another way without creating an extra array
    -- Return -1, -1 if @map is empty

    local r = math.random(map:size())
    local i, xr, yr = 1, -1, -1
    map:iter( function(x, y, v)
        if (i == r) then xr, yr = x, y end
        i = i + 1
    end)

    return xr, yr
end

--------- Location, position or hex related helper functions ----------

function ai_helper.cartesian_coords(x, y)
    -- Converts coordinates from hex geometry to cartesian coordinates,
    -- meaning that y coordinates are offset by 0.5 every other hex
    -- Example: (1,1) stays (1,1) and (3,1) remains (3,1), but (2,1) -> (2,1.5) etc.
    return x, y + ((x + 1) % 2) / 2.
end

function ai_helper.get_angle(from_hex, to_hex)
    -- Returns the angle of the direction from @from_hex to @to_hex
    -- Angle is in radians and goes from -pi to pi. 0 is toward east.
    -- Input hex tables can be of form { x, y } or { x = x, y = y }, which
    -- means that it is also possible to pass a unit table
    local x1, y1 = from_hex.x or from_hex[1], from_hex.y or from_hex[2]
    local x2, y2 = to_hex.x or to_hex[1], to_hex.y or to_hex[2]

    local _, y1cart =  ai_helper.cartesian_coords(x1, y1)
    local _, y2cart =  ai_helper.cartesian_coords(x2, y2)

    return math.atan(y2cart - y1cart, x2 - x1)
end

function ai_helper.get_direction_index(from_hex, to_hex, n, center_on_east)
    -- Returns an integer index for the direction from @from_hex to @to_hex
    -- with the full circle divided into @n slices
    -- 1 is always to the east, with indices increasing clockwise
    -- Input hex tables can be of form { x, y } or { x = x, y = y }, which
    -- means that it is also possible to pass a unit table
    --
    -- Optional input:
    -- @center_on_east (false): boolean. By default, the eastern direction is the
    -- northern border of the first slice. If this parameter is set, east will
    -- instead be the center direction of the first slice

    local d_east = 0
    if center_on_east then d_east = 0.5 end

    local angle = ai_helper.get_angle(from_hex, to_hex)
    local index = math.floor((angle / math.pi * n/2 + d_east) % n ) + 1

    return index
end

function ai_helper.get_cardinal_directions(from_hex, to_hex)
    local dirs = { "E", "S", "W", "N" }
    return dirs[ai_helper.get_direction_index(from_hex, to_hex, 4, true)]
end

function ai_helper.get_intercardinal_directions(from_hex, to_hex)
    local dirs = { "E", "SE", "S", "SW", "W", "NW", "N", "NE" }
    return dirs[ai_helper.get_direction_index(from_hex, to_hex, 8, true)]
end

function ai_helper.get_hex_facing(from_hex, to_hex)
    local dirs = { "se", "s", "sw", "nw", "n", "ne" }
    return dirs[ai_helper.get_direction_index(from_hex, to_hex, 6)]
end

function ai_helper.find_opposite_hex_adjacent(hex, center_hex)
    -- Find the hex that is opposite of @hex with respect to @center_hex
    -- Both input hexes are of format { x, y }
    -- Output: {opp_x, opp_y} -- or nil if @hex and @center_hex are not adjacent
    --   (or no opposite hex is found, e.g. for hexes on border)

    -- If the two input hexes are not adjacent, return nil
    if (H.distance_between(hex[1], hex[2], center_hex[1], center_hex[2]) ~= 1) then return nil end

    -- Finding the opposite x position is easy
    local opp_x = center_hex[1] + (center_hex[1] - hex[1])

    -- y is slightly more tricky, because of the hexagonal shape, but there's a trick
    -- that saves us from having to build in a lot of if statements
    -- Among the adjacent hexes, it is the one with the correct x, and y _different_ from hex[2]
    for xa,ya in H.adjacent_tiles(center_hex[1], center_hex[2]) do
        if (xa == opp_x) and (ya ~= hex[2]) then return { xa, ya } end
    end

    return nil
end

function ai_helper.find_opposite_hex(hex, center_hex)
    -- Find the hex that is opposite of @hex with respect to @center_hex
    -- Using "square coordinate" method by JaMiT
    -- Note: this also works for non-adjacent hexes, but might return hexes that are not on the map!
    -- Both input hexes are of format { x, y }
    -- Output: { opp_x, opp_y }

    -- Finding the opposite x position is easy
    local opp_x = center_hex[1] + (center_hex[1] - hex[1])

    -- Going to "square geometry" for y coordinate
    local y_sq = hex[2] * 2 - (hex[1] % 2)
    local yc_sq = center_hex[2] * 2 - (center_hex[1] % 2)

    -- Now the same equation as for x can be used for y
    local opp_y = yc_sq + (yc_sq - y_sq)
    opp_y = math.floor((opp_y + 1) / 2)

    return {opp_x, opp_y}
end

function ai_helper.is_opposite_adjacent(hex1, hex2, center_hex)
    -- Returns true if @hex1 and @hex2 are opposite from each other with respect to @center_hex

    local opp_hex = ai_helper.find_opposite_hex_adjacent(hex1, center_hex)

    if opp_hex and (opp_hex[1] == hex2[1]) and (opp_hex[2] == hex2[2]) then return true end
    return false
end

function ai_helper.get_closest_location(hex, location_filter, unit)
    -- Get the location closest to @hex (in format { x, y })
    -- that matches @location_filter (in WML table format)
    -- @unit can be passed as an optional third parameter, in which case the
    -- terrain needs to be passable for that unit
    -- Returns nil if no terrain matching the filter was found

    -- Find the maximum distance from 'hex' that's possible on the map
    local max_distance = 0
    local width, height = wesnoth.get_map_size()
    local to_top_left = H.distance_between(hex[1], hex[2], 0, 0)
    if (to_top_left > max_distance) then max_distance = to_top_left end
    local to_top_right = H.distance_between(hex[1], hex[2], width+1, 0)
    if (to_top_right > max_distance) then max_distance = to_top_right end
    local to_bottom_left = H.distance_between(hex[1], hex[2], 0, height+1)
    if (to_bottom_left > max_distance) then max_distance = to_bottom_left end
    local to_bottom_right = H.distance_between(hex[1], hex[2], width+1, height+1)
    if (to_bottom_right > max_distance) then max_distance = to_bottom_right end

    local radius = 0
    while (radius <= max_distance) do
        local loc_filter = {}
        if (radius == 0) then
            loc_filter = {
                { "and", { x = hex[1], y = hex[2], radius = radius } },
                { "and", location_filter }
            }
        else
            loc_filter = {
                { "and", { x = hex[1], y = hex[2], radius = radius } },
                { "not", { x = hex[1], y = hex[2], radius = radius - 1 } },
                { "and", location_filter }
            }
        end

        local locs = wesnoth.get_locations(loc_filter)

        if unit then
            for _,loc in ipairs(locs) do
                local movecost = wesnoth.unit_movement_cost(unit, wesnoth.get_terrain(loc[1], loc[2]))
                if (movecost <= unit.max_moves) then return loc end
            end
        else
            if locs[1] then return locs[1] end
        end

        radius = radius + 1
    end

    return nil
end

function ai_helper.get_passable_locations(location_filter, unit)
    -- Finds all locations matching @location_filter that are passable for
    -- @unit. This also excludes hexes on the map border.
    -- @unit is optional: if omitted, all hexes matching the filter, but
    -- excluding border hexes are returned

    -- All hexes that are not on the map border
    local width, height = wesnoth.get_map_size()
    local all_locs = wesnoth.get_locations{
        x = '1-' .. width,
        y = '1-' .. height,
        { "and", location_filter }
    }

    -- If @unit is provided, exclude terrain that's impassable for the unit
    if unit then
        local locs = {}
        for _,loc in ipairs(all_locs) do
            local movecost = wesnoth.unit_movement_cost(unit, wesnoth.get_terrain(loc[1], loc[2]))
            if (movecost <= unit.max_moves) then table.insert(locs, loc) end
        end
        return locs
    end

    return all_locs
end

function ai_helper.distance_map(units, map)
    -- Get the distance map DM for all units in @units (as a location set)
    -- DM = sum ( distance_from_unit )
    -- This is done for all elements of @map (a locations set), or for the entire map if @map is not given

    local DM = LS.create()

    if map then
        map:iter(function(x, y, data)
            local dist = 0
            for _,unit in ipairs(units) do
                dist = dist + H.distance_between(unit.x, unit.y, x, y)
            end
            DM:insert(x, y, dist)
        end)
    else
        local width, height = wesnoth.get_map_size()
        for x = 1,width do
            for y = 1,height do
                local dist = 0
                for _,unit in ipairs(units) do
                    dist = dist + H.distance_between(unit.x, unit.y, x, y)
                end
                DM:insert(x, y, dist)
            end
        end
    end

    return DM
end

function ai_helper.inverse_distance_map(units, map)
    -- Get the inverse distance map IDM for all units in @units (as a location set)
    -- IDM = sum ( 1 / (distance_from_unit+1) )
    -- This is done for all elements of @map (a locations set), or for the entire map if @map is not given

    local IDM = LS.create()
    if map then
        map:iter(function(x, y, data)
            local dist = 0
            for _,unit in ipairs(units) do
                dist = dist + 1. / (H.distance_between(unit.x, unit.y, x, y) + 1)
            end
            IDM:insert(x, y, dist)
        end)
    else
        local width, height = wesnoth.get_map_size()
        for x = 1,width do
            for y = 1,height do
                local dist = 0
                for _,unit in ipairs(units) do
                    dist = dist + 1. / (H.distance_between(unit.x, unit.y, x, y) + 1)
                end
                IDM:insert(x, y, dist)
            end
        end
    end

    return IDM
end

function ai_helper.generalized_distance(x1, y1, x2, y2)
    -- Determines distance of (@x1,@y1) from (@x2,@y2) even if
    -- @x2 and @y2 are not necessarily both given (or not numbers)

    -- Return 0 if neither is given
    if (not x2) and (not y2) then return 0 end

    -- If only one of the parameters is set
    if (not x2) then return math.abs(y1 - y2) end
    if (not y2) then return math.abs(x1 - x2) end

    -- Otherwise, return standard distance
    return H.distance_between(x1, y1, x2, y2)
end

function ai_helper.xyoff(x, y, ori, hex)
    -- Finds hexes at a certain offset from @x,@y
    -- @ori: direction/orientation: north (0), ne (1), se (2), s (3), sw (4), nw (5)
    -- @hex: string for the hex to be queried. Possible values:
    --   's': self, 'u': up, 'lu': left up, 'ld': left down, 'ru': right up, 'rd': right down
    --   This is all relative "looking" in the direction of 'ori'
    -- returns x,y for the queried hex

    -- Unlike Lua default, we count 'ori' from 0 (north) to 5 (nw), so that modulo operator can be used
    ori = ori % 6

    if (hex == 's') then return x, y end

    -- This is all done with ifs, to keep it as fast as possible
    if (ori == 0)  then -- "north"
        if (hex == 'u') then return x, y-1 end
        if (hex == 'd') then return x, y+1 end
        local dy = 0
        if (x % 2) == 1 then dy=1 end
        if (hex == 'lu') then return x-1, y-dy end
        if (hex == 'ld') then return x-1, y+1-dy end
        if (hex == 'ru') then return x+1, y-dy end
        if (hex == 'rd') then return x+1, y+1-dy end
    end

    if (ori == 1)  then -- "north-east"
        local dy = 0
        if (x % 2) == 1 then dy=1 end
        if (hex == 'u') then return x+1, y-dy end
        if (hex == 'd') then return x-1, y+1-dy end
        if (hex == 'lu') then return x, y-1 end
        if (hex == 'ld') then return x-1, y-dy end
        if (hex == 'ru') then return x+1, y+1-dy end
        if (hex == 'rd') then return x, y+1 end
    end

    if (ori == 2)  then -- "south-east"
        local dy = 0
        if (x % 2) == 1 then dy=1 end
        if (hex == 'u') then return x+1, y+1-dy end
        if (hex == 'd') then return x-1, y-dy end
        if (hex == 'lu') then return x+1, y-dy end
        if (hex == 'ld') then return x, y-1 end
        if (hex == 'ru') then return x, y+1 end
        if (hex == 'rd') then return x-1, y+1-dy end
    end

    if (ori == 3)  then -- "south"
        if (hex == 'u') then return x, y+1 end
        if (hex == 'd') then return x, y-1 end
        local dy = 0
        if (x % 2) == 1 then dy=1 end
        if (hex == 'lu') then return x+1, y+1-dy end
        if (hex == 'ld') then return x+1, y-dy end
        if (hex == 'ru') then return x-1, y+1-dy end
        if (hex == 'rd') then return x-1, y-dy end
    end

    if (ori == 4)  then -- "south-west"
        local dy = 0
        if (x % 2) == 1 then dy=1 end
        if (hex == 'u') then return x-1, y+1-dy end
        if (hex == 'd') then return x+1, y-dy end
        if (hex == 'lu') then return x, y+1 end
        if (hex == 'ld') then return x+1, y+1-dy end
        if (hex == 'ru') then return x-1, y-dy end
        if (hex == 'rd') then return x, y-1 end
    end

    if (ori == 5)  then -- "north-west"
        local dy = 0
        if (x % 2) == 1 then dy=1 end
        if (hex == 'u') then return x-1, y-dy end
        if (hex == 'd') then return x+1, y+1-dy end
        if (hex == 'lu') then return x-1, y+1-dy end
        if (hex == 'ld') then return x, y+1 end
        if (hex == 'ru') then return x, y-1 end
        if (hex == 'rd') then return x+1, y-dy end
    end

    return
end

function ai_helper.split_location_list_to_strings(list)
    -- Convert a list of locations @list as returned by wesnoth.get_locations into a pair of strings
    -- suitable for passing in as x,y coordinate lists to wesnoth.get_locations.
    -- Could alternatively convert to a WML table and use the find_in argument, but this is simpler.
    local locsx, locsy = {}, {}
    for i,loc in ipairs(list) do
        locsx[i] = loc[1]
        locsy[i] = loc[2]
    end
    locsx = table.concat(locsx, ",")
    locsy = table.concat(locsy, ",")

    return locsx, locsy
end

--------- Unit related helper functions ----------

function ai_helper.get_live_units(filter)
    -- Same as wesnoth.get_units(), except that it only returns non-petrified units

    local all_units = wesnoth.get_units(filter)

    local units = {}
    for _,unit in ipairs(all_units) do
        if (not unit.status.petrified) then table.insert(units, unit) end
    end

    return units
end

function ai_helper.get_units_with_moves(filter)
    -- Using formula = '$this_unit.moves > 0' is slow, this method is much faster
    local all_units = wesnoth.get_units(filter)

    local units = {}
    for _,unit in ipairs(all_units) do
        if (unit.moves > 0) then table.insert(units, unit) end
    end

    return units
end

function ai_helper.get_units_with_attacks(filter)
    -- Using formula = '$this_unit.attacks_left > 0' is slow, this method is much faster
    -- Also need to check that units actually have attacks (as attacks_left > 0 with no attacks is possible)
    local all_units = wesnoth.get_units(filter)

    local units = {}
    for _,unit in ipairs(all_units) do
        if (unit.attacks_left > 0) and (#unit.attacks > 0) then
            table.insert(units, unit)
        end
    end

    return units
end

function ai_helper.get_visible_units(viewing_side, filter)
    -- Get units that are visible to side @viewing_side
    --
    -- Required parameters:
    -- @viewing_side: see comments at beginning of this file
    --
    -- Optional parameters:
    -- @filter: Standard unit filter WML table for the units
    --   Example 1: { type = 'Orcish Grunt' }
    --   Example 2: { { "filter_location", { x = 10, y = 12, radius = 5 } } }

    if (not viewing_side) then
        error('ai_helper.get_visible_units() is missing required parameter viewing_side.', 2)
    end
    if (type(viewing_side) ~= 'number') then
        error('ai_helper.get_visible_units(): parameter viewing_side must be a number., 2')
    end

    local filter_plus_vision = {}
    if filter then filter_plus_vision = ai_helper.table_copy(filter) end
    if wesnoth.sides[viewing_side] then
        table.insert(filter_plus_vision, { "filter_vision", { side = viewing_side, visible = 'yes' } })
    end

    local units = {}
    local all_units = wesnoth.get_units()
    for _,unit in ipairs(all_units) do
        if unit:matches(filter_plus_vision) then
            table.insert(units, unit)
        end
    end

    return units
end

function ai_helper.is_visible_unit(viewing_side, unit)
    -- Check whether @unit exists and is visible to side @viewing_side.
    --
    -- Required parameters:
    -- @viewing_side: see comments at beginning of this file.
    -- @unit: unit proxy table

    if (not viewing_side) then
        error('ai_helper.is_visible_unit() is missing required parameter viewing_side.', 2)
    end
    if (type(viewing_side) ~= 'number') then
        error('ai_helper.is_visible_unit(): parameter viewing_side must be a number.', 2)
    end

    if (not unit) then return false end

    if wesnoth.sides[viewing_side]
        and unit:matches({ { "filter_vision", { side = viewing_side, visible = 'no' } } })
    then
        return false
    end

    return true
end

function ai_helper.get_attackable_enemies(filter, side, cfg)
    -- Attackable enemies are defined as being being
    --   - enemies of the side defined in @side,
    --   - not petrified
    --   - and visible to the side defined in @cfg.viewing_side.
    -- For speed reasons, this is done separately, rather than calling ai_helper.get_visible_units().
    --
    -- Optional parameters:
    -- @filter: Standard unit filter WML table for the enemies
    --   Example 1: { type = 'Orcish Grunt' }
    --   Example 2: { { "filter_location", { x = 10, y = 12, radius = 5 } } }
    -- @side: side number, if side other than current side is to be considered
    -- @cfg: table with optional configuration parameters:
    --   viewing_side: see comments at beginning of this file. Defaults to @side.

    side = side or wesnoth.current.side
    local viewing_side = cfg and cfg.viewing_side or side

    local filter_plus_vision = {}
    if filter then filter_plus_vision = ai_helper.table_copy(filter) end
    if wesnoth.sides[viewing_side] then
        table.insert(filter_plus_vision, { "filter_vision", { side = viewing_side, visible = 'yes' } })
    end

    local enemies = {}
    local all_units = wesnoth.get_units()
    for _,unit in ipairs(all_units) do
        if wesnoth.is_enemy(side, unit.side)
           and (not unit.status.petrified)
           and unit:matches(filter_plus_vision)
        then
            table.insert(enemies, unit)
        end
    end

    return enemies
end

function ai_helper.is_attackable_enemy(unit, side, cfg)
    -- Check if @unit exists, is an enemy of @side, is visible to the side defined
    -- in @cfg.viewing_side and is not petrified.
    --
    -- Optional parameters:
    -- @side: side number, defaults to current side.
    -- @cfg: table with optional configuration parameters:
    --   viewing_side: see comments at beginning of this file. Defaults to @side.

    side = side or wesnoth.current.side
    local viewing_side = cfg and cfg.viewing_side or side

    if (not unit)
        or (not wesnoth.is_enemy(side, unit.side))
        or unit.status.petrified
        or (not ai_helper.is_visible_unit(viewing_side, unit))
    then
        return false
    end

    return true
end

function ai_helper.get_closest_enemy(loc, side, cfg)
    -- Return distance to and location of the enemy closest to @loc, or to the
    -- leader of side with number @side if @loc is not specified
    --
    -- Optional parameters:
    -- @loc: location in format { x , y }
    -- @side: number of side for which to find enemy; defaults to current side
    -- @cfg: table with optional configuration parameters:
    --   viewing_side: see comments at beginning of this file. Defaults to @side.

    side = side or wesnoth.current.side

    local enemies = ai_helper.get_attackable_enemies({}, side, cfg)

    local x, y
    if not loc then
        local leader = wesnoth.get_units { side = side, canrecruit = 'yes' }[1]
        x, y = leader.x, leader.y
    else
        x, y = loc[1], loc[2]
    end

    local closest_distance, location = 9e99
    for _,enemy in ipairs(enemies) do
        enemy_distance = H.distance_between(x, y, enemy.x, enemy.y)
        if (enemy_distance < closest_distance) then
            closest_distance = enemy_distance
            location = { x = enemy.x, y = enemy.y}
        end
    end

    return closest_distance, location
end

function ai_helper.has_ability(unit, ability)
    -- Returns true/false depending on whether unit has the given ability
    local has_ability = false
    local abilities = H.get_child(unit.__cfg, "abilities")
    if abilities then
        if H.get_child(abilities, ability) then has_ability = true end
    end
    return has_ability
end

function ai_helper.has_weapon_special(unit, special)
    -- Returns true/false depending on whether @unit has a weapon with special @special
    -- Also returns the number of the first weapon with this special
    for weapon_number,att in ipairs(unit.attacks) do
        for _,sp in ipairs(att.specials) do
            if (sp[1] == special) then
                return true, weapon_number
            end
        end
    end
    return false
end

function ai_helper.get_cheapest_recruit_cost()
    local cheapest_unit_cost = 9e99
    for _,recruit_id in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
        if wesnoth.unit_types[recruit_id].cost < cheapest_unit_cost then
            cheapest_unit_cost = wesnoth.unit_types[recruit_id].cost
        end
    end

    return cheapest_unit_cost
end

--------- Move related helper functions ----------

ai_helper.no_path = 42424242  -- Value returned by C++ engine for distance when no path is found

function ai_helper.get_dst_src_units(units, cfg)
    -- Get the dst_src location set for @units
    -- @cfg: table with optional configuration parameters:
    --   moves: if set to 'max' use max_moves of units, rather than current moves
    --   all parameters for wesnoth.find_reach

    local max_moves = false
    if cfg then
        if (cfg['moves'] == 'max') then max_moves = true end
    end

    local dstsrc = LS.create()
    for _,unit in ipairs(units) do
        local tmp = unit.moves
        if max_moves then
            unit.moves = unit.max_moves
        end
        local reach = wesnoth.find_reach(unit, cfg)
        if max_moves then
            unit.moves = tmp
        end

        for _,loc in ipairs(reach) do
            local tmp = dstsrc:get(loc[1], loc[2]) or {}
            table.insert(tmp, { x = unit.x, y = unit.y })
            dstsrc:insert(loc[1], loc[2], tmp)
        end
    end

    return dstsrc
end

function ai_helper.get_dst_src(units, cfg)
    -- If @units table is given use it, otherwise use all units on the current side
    -- @cfg: table with optional configuration parameters:
    --   moves: if set to 'max' use max_moves of units, rather than current moves
    --   all parameters for wesnoth.find_reach

    if (not units) then
        units = wesnoth.get_units { side = wesnoth.current.side }
    end

    return ai_helper.get_dst_src_units(units, cfg)
end

function ai_helper.get_enemy_dst_src(enemies, cfg)
    -- If @enemies table is given use it, otherwise use all enemy units
    -- @cfg: table with optional configuration parameters:
    --   all parameters for wesnoth.find_reach

    if (not enemies) then
        enemies = wesnoth.get_units {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side} } } }
        }
    end

    local cfg_copy = {}
    if cfg then cfg_copy = ai_helper.table_copy(attack_array) end
    cfg_copy.moves = 'max'

    return ai_helper.get_dst_src_units(enemies, cfg_copy)
end

function ai_helper.my_moves()
    -- Produces an array with each field of form:
    --   [1] = { dst = { x = 7, y = 16 },
    --           src = { x = 6, y = 16 } }

    local dstsrc = ai.get_dstsrc()

    local my_moves = {}
    for key,value in pairs(dstsrc) do
        table.insert( my_moves,
            {   src = { x = value[1].x , y = value[1].y },
                dst = { x = key.x , y = key.y }
            }
        )
    end

    return my_moves
end

function ai_helper.enemy_moves()
    -- Produces an array with each field of form:
    --   [1] = { dst = { x = 7, y = 16 },
    --           src = { x = 6, y = 16 } }

    local dstsrc = ai.get_enemy_dstsrc()

    local enemy_moves = {}
    for key,value in pairs(dstsrc) do
        table.insert( enemy_moves,
            {   src = { x = value[1].x , y = value[1].y },
                dst = { x = key.x , y = key.y }
            }
        )
    end

    return enemy_moves
end

function ai_helper.next_hop(unit, x, y, cfg)
    -- Finds the next "hop" of @unit on its way to (@x,@y)
    -- Returns coordinates of the endpoint of the hop (or nil if no path to
    -- (x,y) is found for the unit), and movement cost to get there.
    -- Only unoccupied hexes are considered
    -- @cfg: standard extra options for wesnoth.find_path()
    --   including:
    --     viewing_side: see comments at beginning of this file. Defaults to side of @unit
    --   plus:
    --     ignore_own_units: if set to true, then own units that can move out of the way are ignored

    local path, cost = ai_helper.find_path_with_shroud(unit, x, y, cfg)

    if cost >= ai_helper.no_path then return nil, cost end

    -- If none of the hexes are unoccupied, use current position as default
    local next_hop, nh_cost = { unit.x, unit.y }, 0

    -- Go through loop to find reachable, unoccupied hex along the path
    -- Start at second index, as first is just the unit position itself
    for i = 2,#path do
        local sub_path, sub_cost = ai_helper.find_path_with_shroud(unit, path[i][1], path[i][2], cfg)

        if sub_cost <= unit.moves then
            -- Check for unit in way only if cfg.ignore_units is not set
            local unit_in_way
            if (not cfg) or (not cfg.ignore_units) then
                unit_in_way = wesnoth.get_unit(path[i][1], path[i][2])

                -- If ignore_own_units is set, ignore own side units that can move out of the way
                if cfg and cfg.ignore_own_units then
                    if unit_in_way and (unit_in_way.side == unit.side) then
                        local reach = ai_helper.get_reachable_unocc(unit_in_way, cfg)
                        if (reach:size() > 1) then unit_in_way = nil end
                    end
                end
            end

            if not unit_in_way then
                next_hop, nh_cost = path[i], sub_cost
            end
        else
            break
        end
    end

    return next_hop, nh_cost
end

function ai_helper.can_reach(unit, x, y, cfg)
    -- Returns true if hex (@x, @y) is unoccupied (by a visible unit), or
    -- at most occupied by unit on same side as @unit that can move away
    -- (can be modified with options below)
    --
    -- @cfg: table with optional configuration parameters:
    --   moves = 'max' use max_moves instead of current moves
    --   ignore_units: if true, ignore both own and enemy units
    --   exclude_occupied: if true, exclude hex if there's a unit there, irrespective of value of 'ignore_units'
    --   viewing_side: see comments at beginning of this file. Defaults to side of @unit

    cfg = cfg or {}
    local viewing_side = cfg.viewing_side or unit.side

    -- Is there a unit at the goal hex?
    local unit_in_way = wesnoth.get_unit(x, y)
    if (cfg.exclude_occupied)
      and unit_in_way and ai_helper.is_visible_unit(viewing_side, unit_in_way)
    then
        return false
    end

    -- Otherwise, if 'ignore_units' is not set, return false if there's a unit of other side,
    -- or a unit of own side that cannot move away (this might be slow, don't know)
    if (not cfg.ignore_units) then
        -- If there's a unit at the goal that's not on own side (even ally), return false
        if unit_in_way
            and (unit_in_way.side ~= unit.side)
            and ai_helper.is_visible_unit(viewing_side, unit_in_way)
        then
            return false
        end

        -- If the unit in the way is on 'unit's' side and cannot move away, also return false
        if unit_in_way and (unit_in_way.side == unit.side) then
            -- need to pass the cfg here so that it works for enemy units (generally with no moves left) also
            local move_away = ai_helper.get_reachable_unocc(unit_in_way, cfg)
            if (move_away:size() <= 1) then return false end
        end
    end

    -- After all that, test whether our unit can actually get there
    local old_moves = unit.moves
    if (cfg.moves == 'max') then unit.moves = unit.max_moves end

    local can_reach = false
    local path, cost = ai_helper.find_path_with_shroud(unit, x, y, cfg)
    if (cost <= unit.moves) then can_reach = true end

    unit.moves = old_moves

    return can_reach
end

function ai_helper.get_reachable_unocc(unit, cfg)
    -- Get all reachable hexes for @unit that are (or appear) unoccupied, taking
    -- vision into account as specified by the @cfg.viewing_side parameter.
    -- Returned array is a location set, with value = 1 for each reachable hex
    --
    -- @cfg: table with optional configuration parameters:
    --   moves: if set to 'max', unit MP is set to max_moves before calculation
    --   viewing_side: see comments at beginning of this file. Defaults to side of @unit
    --   plus all other parameters to wesnoth.find_reach

    local viewing_side = cfg and cfg.viewing_side or unit.side

    local old_moves = unit.moves
    if cfg then
        if (cfg.moves == 'max') then unit.moves = unit.max_moves end
    end

    local reach = LS.create()
    local initial_reach = wesnoth.find_reach(unit, cfg)
    for _,loc in ipairs(initial_reach) do
        local unit_in_way = wesnoth.get_unit(loc[1], loc[2])
        if (not unit_in_way) or (not ai_helper.is_visible_unit(viewing_side, unit_in_way)) then
            reach:insert(loc[1], loc[2], 1)
        end
    end

    -- Also need to include the hex the unit is on itself
    reach:insert(unit.x, unit.y, 1)

    unit.moves = old_moves

    return reach
end

function ai_helper.find_path_with_shroud(unit, x, y, cfg)
    -- Same as wesnoth.find_path, just that it works under shroud as well while still
    -- ignoring invisible units. It does this by using viewing_side=0 and taking
    -- invisible units off the map for the path finding process.
    --
    -- Notes on some of the optional parameters that can be passed in @cfg:
    --  - viewing_side: If not given, use side of the unit (not the current side!)
    --    for determining which units are hidden and need to be extracted, as that
    --    is what the path_finder code uses. If set to an invalid side, we can use
    --    default path finding as shroud is ignored then anyway.
    --  - ignore_units: if true, hidden units do not need to be extracted because
    --    all units are ignored anyway

    local viewing_side = (cfg and cfg.viewing_side) or unit.side

    local path, cost
    if wesnoth.sides[viewing_side] and wesnoth.sides[viewing_side].shroud then
        local extracted_units = {}
        if (not cfg) or (not cfg.ignore_units) then
            local all_units = wesnoth.get_units()
            for _,u in ipairs(all_units) do
                if (u.side ~= viewing_side)
                    and (not ai_helper.is_visible_unit(viewing_side, u))
                then
                    wesnoth.extract_unit(u)
                    table.insert(extracted_units, u)
                end
            end
        end

        local cfg_copy = {}
        if cfg then cfg_copy = ai_helper.table_copy(cfg) end
        cfg_copy.viewing_side = 0
        path, cost = wesnoth.find_path(unit, x, y, cfg_copy)

        for _,extracted_unit in ipairs(extracted_units) do
            wesnoth.put_unit(extracted_unit)
        end
    else
        path, cost = wesnoth.find_path(unit, x, y, cfg)
    end

    return path, cost
end

function ai_helper.find_best_move(units, rating_function, cfg)
    -- Find the best move and best unit based on @rating_function
    -- INPUTS:
    --  @units: (required) single unit or table of units
    --  @rating_function: (required) function(x, y) with rating function for the hexes the unit can reach
    --  @cfg: table with optional configuration parameters:
    --    labels: if set, put labels with ratings onto map
    --    no_random: if set, do not add random value between 0.0001 and 0.0099 to each hex
    --    plus all the possible parameters for ai_helper.get_reachable_unocc()
    --
    -- OUTPUTS:
    --  best_hex: format { x, y }
    --  best_unit: unit for which this rating function produced the maximum value
    --  max_rating: the rating found for this hex/unit combination
    -- If no valid moves were found, best_unit and best_hex are empty arrays

    -- TODO: change return value to nil if no unit/hex is found later in 1.13., but keep as is in 1.12

    cfg = cfg or {}

    -- If this is an individual unit, turn it into an array
    if units.hitpoints then units = { units } end

    local max_rating, best_hex, best_unit = -9e99, {}, {}
    for _,unit in ipairs(units) do
        -- Hexes each unit can reach
        local reach_map = ai_helper.get_reachable_unocc(unit, cfg)
        reach_map:iter( function(x, y, v)
            -- Rate based on rating_function argument
            local rating = rating_function(x, y)

            -- If cfg.random is set, add some randomness (on 0.0001 - 0.0099 level)
            if (not cfg.no_random) then rating = rating + math.random(99) / 10000. end
            -- If cfg.labels is set: insert values for label map
            if cfg.labels then reach_map:insert(x, y, rating) end

            if rating > max_rating then
                max_rating, best_hex, best_unit = rating, { x, y }, unit
            end
        end)
        if cfg.labels then ai_helper.put_labels(reach_map) end
    end

    return best_hex, best_unit, max_rating
end

function ai_helper.move_unit_out_of_way(ai, unit, cfg)
    -- Move @unit to the best close location.
    -- Main rating is the moves the unit still has left after that
    -- Other, configurable, parameters are given to function in @cfg:
    --   dx, dy: the direction in which moving out of the way is preferred
    --   labels: if set, display labels of the rating for each hex the unit can reach
    --   viewing_side: see comments at beginning of this file. Defaults to side of @unit.
    --   all other optional parameters to wesnoth.find_reach()

    cfg = cfg or {}
    local viewing_side = cfg.viewing_side or unit.side

    local dx, dy
    if cfg.dx and cfg.dy then
        local r = math.sqrt(cfg.dx * cfg.dx + cfg.dy * cfg.dy)
        dx, dy = cfg.dx / r, cfg.dy / r
    end

    local reach = wesnoth.find_reach(unit, cfg)
    local reach_map = LS.create()

    local max_rating, best_hex = -9e99
    for _,loc in ipairs(reach) do
        local unit_in_way = wesnoth.get_unit(loc[1], loc[2])
        if (not unit_in_way)       -- also excludes current hex
            or (not ai_helper.is_visible_unit(viewing_side, unit_in_way))
        then
            local rating = loc[3]  -- also disfavors hexes next to visible enemy units for which loc[3] = 0

            if dx then
                rating = rating + (loc[1] - unit.x) * dx * 0.01
                rating = rating + (loc[2] - unit.y) * dy * 0.01
            end

            if cfg.labels then reach_map:insert(loc[1], loc[2], rating) end

            if (rating > max_rating) then
                max_rating, best_hex = rating, { loc[1], loc[2] }
            end
        end
    end
    if cfg.labels then ai_helper.put_labels(reach_map) end

    if best_hex then
        ai_helper.checked_move(ai, unit, best_hex[1], best_hex[2])
    end
end

function ai_helper.movefull_stopunit(ai, unit, x, y)
    -- Does ai.move_full for @unit if not at (@x,@y), otherwise ai.stopunit_moves
    -- Uses ai_helper.next_hop(), so that it works if unit cannot get there in one move
    -- Coordinates can be given as x and y components, or as a 2-element table { x, y } or { x = x, y = y }
    if (type(x) ~= 'number') then
        if x[1] then
            x, y = x[1], x[2]
        else
            x, y = x.x, x.y
        end
    end

    local next_hop = ai_helper.next_hop(unit, x, y)
    if next_hop and ((next_hop[1] ~= unit.x) or (next_hop[2] ~= unit.y)) then
        return ai_helper.checked_move_full(ai, unit, next_hop[1], next_hop[2])
    else
        return ai_helper.checked_stopunit_moves(ai, unit)
    end
end

function ai_helper.movefull_outofway_stopunit(ai, unit, x, y, cfg)
    -- Same as ai_help.movefull_stopunit(), but also moves a unit out of the way if there is one
    --
    -- @cfg: table with optional configuration parameters:
    --   viewing_side: see comments at beginning of this file. Defaults to side of @unit
    --   all other optional parameters to ai_helper.move_unit_out_of_way() and wesnoth.find_path()

    local viewing_side = cfg and cfg.viewing_side or unit.side

    if (type(x) ~= 'number') then
        if x[1] then
            x, y = x[1], x[2]
        else
            x, y = x.x, x.y
        end
    end

    -- Only move unit out of way if the main unit can get there
    local path, cost = ai_helper.find_path_with_shroud(unit, x, y, cfg)
    if (cost <= unit.moves) then
        local unit_in_way = wesnoth.get_unit(x, y)
        if unit_in_way and (unit_in_way ~= unit)
            and ai_helper.is_visible_unit(viewing_side, unit_in_way)
        then
            --W.message { speaker = 'narrator', message = 'Moving out of way' }
            ai_helper.move_unit_out_of_way(ai, unit_in_way, cfg)
        end
    end

    local next_hop = ai_helper.next_hop(unit, x, y)
    if next_hop and ((next_hop[1] ~= unit.x) or (next_hop[2] ~= unit.y)) then
        ai_helper.checked_move_full(ai, unit, next_hop[1], next_hop[2])
    else
        ai_helper.checked_stopunit_moves(ai, unit)
    end
end

---------- Attack related helper functions --------------

function ai_helper.get_attacks(units, cfg)
    -- Get all attacks the units stored in @units can do. Invisible enemies are
    -- excluded unless option @cfg.viewing_side=0 is used.
    --
    -- This includes a variety of configurable options, passed in the @cfg table
    -- @cfg: table with optional configuration parameters:
    --   moves: "current" (default for units on current side) or "max" (always used for units on other sides)
    --   include_occupied (false): if set, also include hexes occupied by own-side units that can move away
    --   simulate_combat (false): if set, also simulate the combat and return result (this is slow; only set if needed)
    --   viewing_side: see comments at beginning of this file. Defaults to side of @units
    --   all other optional parameters to wesnoth.find_reach()
    --
    -- Returns {} if no attacks can be done, otherwise table with fields:
    --   dst: { x = x, y = y } of attack position
    --   src: { x = x, y = y } of attacking unit (don't use id, could be ambiguous)
    --   target: { x = x, y = y } of defending unit
    --   att_stats, def_stats: as returned by wesnoth.simulate_combat (if cfg.simulate_combat is set)
    --   attack_hex_occupied: boolean storing whether an own unit that can move away is on the attack hex

    cfg = cfg or {}

    local attacks = {}
    if (not units[1]) then return attacks end

    local side = units[1].side  -- all units need to be on same side
    local viewing_side = cfg and cfg.viewing_side or side

    -- 'moves' can be either "current" or "max"
    -- For unit on current side: use "current" by default, or override by cfg.moves
    local moves = cfg.moves or "current"
    -- For unit on any other side, only moves="max" makes sense
    if (side ~= wesnoth.current.side) then moves = "max" end

    local old_moves = {}
    if (moves == "max") then
        for i,unit in ipairs(units) do
            old_moves[i] = unit.moves
            unit.moves = unit.max_moves
        end
    end

    -- Note: the remainder is optimized for speed, so we only get_units once,
    -- do not use WML filters, etc.
    local all_units = wesnoth.get_units()

    local enemy_map, my_unit_map, other_unit_map = LS.create(), LS.create(), LS.create()
    for i,unit in ipairs(all_units) do
        -- The value of all the location sets is the index of the
        -- unit in the all_units array
        if ai_helper.is_attackable_enemy(unit, side, cfg) then
            enemy_map:insert(unit.x, unit.y, i)
        end

        if (unit.side == side) then
            my_unit_map:insert(unit.x, unit.y, i)
        else
            if ai_helper.is_visible_unit(viewing_side, unit) then
                other_unit_map:insert(unit.x, unit.y, i)
            end
        end
    end

    local attack_hex_map = LS.create()
    enemy_map:iter(function(e_x, e_y, i)
        for xa,ya in H.adjacent_tiles(e_x, e_y) do
            -- If there's no unit of another side on this hex, include it
            -- as possible attack location (this includes hexes occupied
            -- by own units at this time)
            if (not other_unit_map:get(xa, ya)) then
                local target_table = attack_hex_map:get(xa, ya) or {}
                table.insert(target_table, { x = e_x, y = e_y, i = i })
                attack_hex_map:insert(xa, ya, target_table)
            end
        end
    end)

    -- The following is done so that we at most need to do find_reach() once per unit
    -- It is needed for all units in @units and for testing whether units can move out of the way
    local reaches = LS.create()

    for _,unit in ipairs(units) do
        local reach
        if reaches:get(unit.x, unit.y) then
            reach = reaches:get(unit.x, unit.y)
        else
            reach = wesnoth.find_reach(unit, cfg)
            reaches:insert(unit.x, unit.y, reach)
        end

        for _,loc in ipairs(reach) do
            if attack_hex_map:get(loc[1], loc[2]) then
                local add_target = true
                local attack_hex_occupied = false

                -- If another unit of same side is on this hex:
                if my_unit_map:get(loc[1], loc[2]) and ((loc[1] ~= unit.x) or (loc[2] ~= unit.y)) then
                    attack_hex_occupied = true
                    add_target = false

                    if cfg.include_occupied then -- Test whether it can move out of the way
                        local unit_in_way = all_units[my_unit_map:get(loc[1], loc[2])]
                        local uiw_reach
                        if reaches:get(unit_in_way.x, unit_in_way.y) then
                            uiw_reach = reaches:get(unit_in_way.x, unit_in_way.y)
                        else
                            uiw_reach = wesnoth.find_reach(unit_in_way, cfg)
                            reaches:insert(unit_in_way.x, unit_in_way.y, uiw_reach)
                        end

                        -- Check whether the unit to move out of the way has an unoccupied hex to move to.
                        -- We do not deal with cases where a unit can move out of the way for a
                        -- unit that is moving out of the way of the initial unit (etc.).
                        for _,uiw_loc in ipairs(uiw_reach) do
                            -- Unit in the way of the unit in the way
                            local uiw_uiw = wesnoth.get_unit(uiw_loc[1], uiw_loc[2])
                            if (not uiw_uiw) or (not ai_helper.is_visible_unit(viewing_side, uiw_uiw)) then
                                add_target = true
                                break
                            end
                        end
                    end
                end

                if add_target then
                    for _,target in ipairs(attack_hex_map:get(loc[1], loc[2])) do
                        local att_stats, def_stats
                        if cfg.simulate_combat then
                            local unit_dst = wesnoth.copy_unit(unit)
                            unit_dst.x, unit_dst.y = loc[1], loc[2]

                            local enemy = all_units[target.i]
                            att_stats, def_stats = wesnoth.simulate_combat(unit_dst, enemy)
                        end

                        table.insert(attacks, {
                            src = { x = unit.x, y = unit.y },
                            dst = { x = loc[1], y = loc[2] },
                            target = { x = target.x, y = target.y },
                            att_stats = att_stats,
                            def_stats = def_stats,
                            attack_hex_occupied = attack_hex_occupied
                        })
                    end
                end
            end
        end
    end

    if (moves == "max") then
        for i,unit in ipairs(units) do
            unit.moves = old_moves[i]
        end
    end

    return attacks
end

function ai_helper.add_next_attack_combo_level(combos, attacks)
    -- This is called from ai_helper.get_attack_combos_full() and
    -- builds up the combos for the next recursion level.
    -- It also calls the next recursion level, if possible
    -- Important: function needs to make a copy of the input array, otherwise original is changed

    -- Set up the array, if this is the first recursion level
    if (not combos) then combos = {} end

    -- Array to hold combinations for this recursion level only
    local combos_this_level = {}

    for _,attack in ipairs(attacks) do
        local dst = attack.dst.y + attack.dst.x * 1000.  -- attack hex (src)
        local src = attack.src.y + attack.src.x * 1000.  -- attacker hex (dst)
        if (not combos[1]) then  -- if this is the first recursion level, set up new combos for this level
            local move = {}
            move[dst] = src
            table.insert(combos_this_level, move)
        else
            -- Otherwise, we need to go through the already existing elements in 'combos'
            -- to see if either hex, or attacker is already used; and then add new attack to each
            for _,combo in ipairs(combos) do
                local this_combo = {}  -- needed because tables are pointers, need to create a separate one
                local add_combo = true
                for d,s in pairs(combo) do
                    if (d == dst) or (s == src) then
                        add_combo = false
                        break
                    end
                    this_combo[d] = s  -- insert individual moves to a combo
                end
                if add_combo then  -- and add it into the array, if it contains only unique moves
                    this_combo[dst] = src
                    table.insert(combos_this_level, this_combo)
                end
            end
        end
    end

    local combos_next_level = {}
    if combos_this_level[1] then  -- If moves were found for this level, also find those for the next level
        combos_next_level = ai_helper.add_next_attack_combo_level(combos_this_level, attacks)
    end

    -- Finally, combine this level and next level combos
    combos_this_level = ai_helper.array_merge(combos_this_level, combos_next_level)
    return combos_this_level
end

function ai_helper.get_attack_combos_full(units, enemy, cfg)
    -- Calculate attack combination result by @units on @enemy
    -- All combinations of all units are taken into account, as well as their order
    -- This can result in a _very_ large number of possible combinations
    -- Use ai_helper.get_attack_combos() instead if order does not matter
    --
    -- Optional inputs:
    --   @cfg: Configuration table to be passed on to ai_helper.get_attacks()
    --
    -- Return value:
    --   1. Attack combinations in form { dst = src }

    local all_attacks = ai_helper.get_attacks(units, cfg)

    -- Eliminate those that are not on @enemy
    local attacks = {}
    for _,attack in ipairs(all_attacks) do
        if (attack.target.x == enemy.x) and (attack.target.y == enemy.y) then
            table.insert(attacks, attack)
        end
    end
    if (not attacks[1]) then return {} end

    -- This recursive function does all the work:
    local combos = ai_helper.add_next_attack_combo_level(combos, attacks)

    return combos
end

function ai_helper.get_attack_combos(units, enemy, cfg)
    -- Calculate attack combination result by @units on @enemy
    -- All the unit/hex combinations are considered, but without specifying the order of the
    -- attacks. Use ai_helper.get_attack_combos_full() if order matters.
    --
    -- Optional inputs:
    -- @cfg: Configuration table to be passed on to wesnoth.find_path()
    --
    -- Return values:
    --   1. Attack combinations in form { dst = src }
    --   2. All the attacks indexed by [dst][src]

    -- We don't need the full attacks here, just the coordinates,
    -- so for speed reasons, we do not use ai_helper.get_attacks()

    -- For units on the current side, we need to make sure that
    -- there isn't a unit in the way that cannot move any more
    -- TODO: generalize it so that it works not only for units with moves=0, but blocked units etc.
    local blocked_hexes = LS.create()
    if units[1] and (units[1].side == wesnoth.current.side) then
        local all_units = wesnoth.get_units { side = wesnoth.current.side }
        for _,unit in ipairs(all_units) do
            if (unit.moves == 0) then
                blocked_hexes:insert(unit.x, unit.y)
            end
        end
    end

    -- For sides other than the current, we always use max_moves,
    -- for the current side we always use current moves
    local old_moves = {}
    for i,unit in ipairs(units) do
        if (unit.side ~= wesnoth.current.side) then
            old_moves[i] = unit.moves
            unit.moves = unit.max_moves
        end
    end

    -- Find which units in @units can get to hexes next to the enemy
    local attacks_dst_src = {}
    local found_attacks = false
    for xa,ya in H.adjacent_tiles(enemy.x, enemy.y) do
        -- Make sure the hex is not occupied by unit that cannot move out of the way

        local dst = xa * 1000 + ya

        for _,unit in ipairs(units) do
            if ((unit.x == xa) and (unit.y == ya)) or (not blocked_hexes:get(xa, ya)) then

                -- helper.distance_between() is much faster than wesnoth.find_path()
                --> pre-filter using the former
                local cost = H.distance_between(unit.x, unit.y, xa, ya)

                -- If the distance is <= the unit's MP, then see if it can actually get there
                -- This also means that only short paths have to be evaluated (in most situations)
                if (cost <= unit.moves) then
                    local path  -- since cost is already defined outside this block
                    path, cost = ai_helper.find_path_with_shroud(unit, xa, ya, cfg)
                end

                if (cost <= unit.moves) then
                    -- for attack by no unit on this hex
                    if (not attacks_dst_src[dst]) then
                        attacks_dst_src[dst] = { 0, unit.x * 1000 + unit.y }
                        found_attacks = true  -- since attacks_dst_src is not a simple array, this is easier
                    else
                        table.insert(attacks_dst_src[dst], unit.x * 1000 + unit.y )
                    end
                end
            end
        end
    end

    for i,unit in ipairs(units) do
        if (unit.side ~= wesnoth.current.side) then
            unit.moves = old_moves[i]
        end
    end

    if (not found_attacks) then return {}, {} end

    -- Now we set up an array of all attack combinations
    -- at this time, this includes all the 'no unit attacks this hex' elements
    -- which have a value of 0 for 'src'
    -- They need to be kept in this part, so that we get the combos that do not
    -- use the maximum amount of units possible. They will be eliminated below.
    local attack_array = {}
    -- For all values of 'dst'
    for dst,ads in pairs(attacks_dst_src) do
        local org_array = ai_helper.table_copy(attack_array)
        attack_array = {}

        -- Go through all the values of 'src'
        for _,src in ipairs(ads) do
            -- If the array does not exist, set it up
            if (not org_array[1]) then
                local tmp = {}
                tmp[dst] = src
                table.insert(attack_array, tmp)
            else  -- otherwise, add the new dst-src pair to each element of the existing array
                for _,org in ipairs(org_array) do
                    -- but only do so if that 'src' value does not exist already
                    -- except for 0's those all need to be kept
                    local add_attack = true
                    for _,s in pairs(org) do
                        if (s == src) and (src ~=0) then
                            add_attack = false
                            break
                        end
                    end
                    -- Finally, add it to the array
                    if add_attack then
                        local tmp = ai_helper.table_copy(org)
                        tmp[dst] = src
                        table.insert(attack_array, tmp)
                    end
                end
            end
        end
    end

    -- Now eliminate all the 0s
    -- Also eliminate the combo that has no attacks on any hex (all zeros)
    local i_empty = 0
    for i,att in ipairs(attack_array) do
        local count = 0
        for dst,src in pairs(att) do
            if (src == 0) then
                att[dst] = nil
            else
                count = count + 1
            end
        end
        if (count == 0) then i_empty = i end
    end

    -- This last step eliminates the "empty attack combo" (the one with all zeros)
    -- Since this is only one, it's okay to use table.remove (even though it's slow)
    table.remove(attack_array, i_empty)

    return attack_array
end

function ai_helper.get_unit_time_of_day_bonus(alignment, lawful_bonus)
    local multiplier = 1
    if (lawful_bonus ~= 0) then
        if (alignment == 'lawful') then
            multiplier = (1 + lawful_bonus / 100.)
        elseif (alignment == 'chaotic') then
            multiplier = (1 - lawful_bonus / 100.)
        elseif (alignment == 'liminal') then
            multipler = (1 - math.abs(lawful_bonus) / 100.)
        end
    end
    return multiplier
end

return ai_helper
