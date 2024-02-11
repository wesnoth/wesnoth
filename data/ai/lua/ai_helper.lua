local LS = wesnoth.require "location_set"
local F = wesnoth.require "functional"
local M = wesnoth.map

-- This is a collection of Lua functions used for custom AI development.
-- Note that this is still work in progress with significant changes occurring
-- frequently. Backward compatibility cannot be guaranteed at this time in
-- development releases, but it is of course easily possible to copy a function
-- from a previous release directly into an add-on if it is needed there.

-- Invisible units ('viewing_side' and 'ignore_visibility' parameters):
-- With their default settings, the ai_helper functions use the vision a player of
-- the respective side would see, that is, they assume no knowledge of invisible
-- units. This can be influenced with the 'viewing_side' and 'ignore_visibility' parameters,
-- which work in the same way as they do in wesnoth.paths.find_reach() and wesnoth.paths.find_path():
--   - If 'viewing_side' is set, vision for that side is used. It must be set to a valid side number.
--   - If 'ignore_visibility' is set to true, all units on the map are seen and shroud is ignored.
--       This overrides 'viewing_side'.
--   - If neither parameter is given and a function takes a parameter linked to a specific side,
--     such as a side number or a unit, as input, vision of that side is used.
--   - For some functions that take no other side-related input, 'viewing_side' is made a required parameter.

---@class ai_helper_visibility_opts
---@field viewing_side? integer If set, vision for that side is used. It must be set to a valid side number. Typically defaults to the side of a specified unit if not set.
---@field ignore_visibility? boolean If set to true, all units on the map are seen and shroud is ignored. This overrides 'viewing_side'.

-- Path finding:
-- All ai_helper functions disregard shroud for path finding (while still ignoring
-- hidden units correctly) as of Wesnoth 1.13.7. This is consistent with default
-- Wesnoth AI behavior and ensures that Lua AIs, including the Micro AIs, can be
-- used for AI sides with shroud=yes. It is accomplished by using
-- ai_helper.find_path_with_shroud() instead of wesnoth.paths.find_path().

---@class ai_helper_lib
local ai_helper = {}

----- Debugging helper functions ------

---Prints out a representation of an HP distribution table
---@param hp_distribution table<integer, number> The HP distribution table, for example from wesnoth.simulate_combat
---@param print? function Optional print function. Pass std_print if you prefer output to the console. Any other function taking a single argument will also work.
function ai_helper.print_hp_distribution(hp_distribution, print)
    print = print or _G.print
    -- hp_distribution is sort of an array, but unlike most Lua arrays it's 0-index
    for i = 0, #hp_distribution - 1 do
        if hp_distribution[i] > 0 then
            print(('P(hp = $hp) = $prob%'):vformat{hp = i, prob = hp_distribution[i] * 100})
        end
    end
end

---Returns true or false (hard-coded). To be used to show messages if in debug mode.
---@return boolean
function ai_helper.show_messages()
    -- Just edit the following line (easier than trying to set WML variable)
    local show_messages_flag = false
    if wesnoth.game_config.debug then return show_messages_flag end
    return false
end

---Returns true or false (hard-coded). To be used to show which CA is being executed if in debug mode.
---@return boolean
function ai_helper.print_exec()
    -- Just edit the following line (easier than trying to set WML variable)
    local print_exec_flag = false
    if wesnoth.game_config.debug then return print_exec_flag end
    return false
end

---Returns true or false (hard-coded). To be used to show which CA is being evaluated if in debug mode.
---@return boolean
function ai_helper.print_eval()
    -- Just edit the following line (easier than trying to set WML variable)
    local print_eval_flag = false
    if wesnoth.game_config.debug then return print_eval_flag end
    return false
end

---@param start_time integer
---@param ca_name string
function ai_helper.done_eval_messages(start_time, ca_name)
    ca_name = ca_name or 'unknown'
    if ai_helper.print_eval() then
        ai_helper.print_ts_delta(start_time, '       - Done evaluating ' .. ca_name .. ':')
    end
end

---Clear all labels on a map
function ai_helper.clear_labels()
    for x, y in wesnoth.current.map:iter(true) do
        M.add_label { x = x, y = y, text = "" }
    end
end

---@class ai_debug_label_opts
---@field show_coords? boolean Use hex coordinates as labels instead of value
---@field factor? number If value is a number, multiply by this factor
---@field keys? any[] If the value to be displayed is a subelement of the LS data, use these keys to access it. For example, if we want to display data[3], set keys = { 3 }; if we want data.arg[3], set keys = { 'arg', 3 }
---@field clear? boolean if set to 'false', do not clear existing labels
---@field color? color the color to be used for the output

---Take map (location set) and put labels containing 'value' onto the map
---Print 'nan' if element exists but is not a number.
---@param map location_set The labels to place on the map.
---@param cfg? ai_debug_label_opts table with optional configuration parameters:
function ai_helper.put_labels(map, cfg)

    cfg = cfg or {}
    local factor = cfg.factor or 1

    local clear_labels = cfg.clear
    if (clear_labels == nil) then clear_labels = true end
    if clear_labels then
        ai_helper.clear_labels()
    end

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
        M.add_label { x = x, y = y, text = out, color = cfg.color }
    end)
end

---Print arguments preceded by a time stamp in seconds.
---Also return that time stamp
---@param ... unknown
---@return number
function ai_helper.print_ts(...)
    local ts = wesnoth.ms_since_init() / 1000.

    local arg = { ... }
    arg[#arg+1] = string.format('[ t = %.3f ]', ts)

    std_print(table.unpack(arg))

    return ts
end

---Same as ai_helper.print_ts(), but also adds time elapsed since
---the time given in the first argument (in seconds)
---Returns time stamp as well as time elapsed
---@param start_time integer Time stamp in seconds as returned by wesnoth.ms_since_init / 1000.
---@param ... unknown
---@return number
---@return number
function ai_helper.print_ts_delta(start_time, ...)
    local ts = wesnoth.ms_since_init() / 1000.
    local delta = ts - start_time

    local arg = { ... }
    arg[#arg+1] = string.format('[ t = %.3f, dt = %.3f ]', ts, delta)

    std_print(table.unpack(arg))

    return ts, delta
end

----- AI execution helper functions ------

---Checks if the result of a move indicates that it was incomplete
---@param check ai_result
---@return integer|false
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

---Check if the result of a move indicates that it was either incomplete or empty.
---An empty move means an attempt to move the hex the unit is already on.
---@param check ai_result
---@return integer|false
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

---Construct a fake AI result table
---@param gamestate_changed? boolean
---@param ok? boolean
---@param result? string
---@param status? number
---@return ai_result
function ai_helper.dummy_check_action(gamestate_changed, ok, result, status)
    return {
        gamestate_changed = gamestate_changed or false,
        ok = ok or false,
        result = result or 'ai_helper::DUMMY_FAILED_ACTION',
        status = status or 99999
    }
end

---Print an error message about a failed action if debug more is enabled
---@param action string
---@param error_code string|number
function ai_helper.checked_action_error(action, error_code)
    if wesnoth.game_config.debug then
        error(action .. ' could not be executed. Error code: ' .. error_code, 3)
    end
end

---Check if an attack is viable, and execute it if it is.
---If the attack is not viable, the unit's attacks left are cleared.
---@param ai ailib
---@param attacker unit
---@param defender unit
---@param weapon integer
---@return ai_result
function ai_helper.checked_attack(ai, attacker, defender, weapon)
    local check = ai.check_attack(attacker, defender, weapon)

    if (not check.ok) then
        ai.stopunit_attacks(attacker)
        ai_helper.checked_action_error('ai.attack from ' .. attacker.x .. ',' .. attacker.y .. ' to ' .. defender.x .. ',' .. defender.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.attack(attacker, defender, weapon)
end

---@param ai ailib
---@param unit unit
---@param x integer
---@param y integer
---@param move_type string
---@return ai_result
local function checked_move_core(ai, unit, x, y, move_type)
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

---Check if a move is viable, and execute it if it is.
---Whether the move is viable or not, the unit's moves left are cleared.
---@param ai ailib
---@param unit unit
---@param x integer
---@param y integer
---@return ai_result
function ai_helper.checked_move_full(ai, unit, x, y)
    return checked_move_core(ai, unit, x, y, 'ai.move_full')
end

---Check if a move is viable, and execute it if it is.
---If the move is not viable, the unit's moves left are cleared.
---@param ai ailib
---@param unit unit
---@param x integer
---@param y integer
---@return ai_result
function ai_helper.checked_move(ai, unit, x, y)
    return checked_move_core(ai, unit, x, y, 'ai.move')
end

---Check if a recruit is viable, and execute it if it is.
---@param ai ailib
---@param unit_type string
---@param x integer
---@param y integer
---@return ai_result
function ai_helper.checked_recruit(ai, unit_type, x, y)
    local check = ai.check_recruit(unit_type, x, y)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.recruit of ' .. unit_type .. ' at ' .. x .. ',' .. y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.recruit(unit_type, x, y)
end

---Check if it's viable to clear the unit's moves and attacks, and do so if it is.
---@param ai ailib
---@param unit unit
---@return ai_result
function ai_helper.checked_stopunit_all(ai, unit)
    local check = ai.check_stopunit(unit)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.stopunit_all of ' .. unit.x .. ',' .. unit.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.stopunit_all(unit)
end

---Check if it's viable to clear the unit's attacks, and do so if it is.
---@param ai ailib
---@param unit unit
---@return ai_result
function ai_helper.checked_stopunit_attacks(ai, unit)
    local check = ai.check_stopunit(unit)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.stopunit_attacks of ' .. unit.x .. ',' .. unit.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.stopunit_attacks(unit)
end

---Check if it's viable to clear the unit's moves, and do so if it is.
---@param ai ailib
---@param unit unit
---@return ai_result
function ai_helper.checked_stopunit_moves(ai, unit)
    local check = ai.check_stopunit(unit)

    if (not check.ok) then
        ai_helper.checked_action_error('ai.stopunit_moves of ' .. unit.x .. ',' .. unit.y, check.status .. ' (' .. check.result .. ')')
        return check
    end

    return ai.stopunit_moves(unit)
end

---@class robust_move_opts : move_out_of_way_opts
---@field partial_move boolean By default, this function performs full moves. If this parameter is true, a partial move is done instead.
---@field weapon integer The number (starting at 1) of the attack weapon to be used. If omitted, the best weapon is automatically selected.

---Perform a move and/or attack with an AI unit in a way that is robust against
---unexpected outcomes such as being ambushed or changes caused by WML events.
---As much as possible, this function also tries to ensure that the gamestate
---is changed in case an action turns out to be impossible due to such an
---unexpected outcome.
---@param ai ailib The ai module
---@param src location current coordinates of the AI unit to be used
---@param dst location coordinates to which the unit should move. This does not have to be different from src. In fact, the unit does not even need to have moves left, as long as an attack is specified in the latter case. If another AI unit is at dst, it is moved out of the way.
---@param target_loc? location Coordinates of the enemy unit to be attacked. If not given, no attack is attempted.
---@param cfg? robust_move_opts Additional configuration options
---@return table
function ai_helper.robust_move_and_attack(ai, src, dst, target_loc, cfg)
    -- Notes:
    -- - If an avoid_map is given in the options table, it is assumed that dst has been
    --   checked to lie outside the area to avoid.
    -- - src, dst and target_loc can be any table (including proxy units) that contains
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

    local unit = wesnoth.units.get(src_x, src_y)
    if (not unit) then
        return ai_helper.dummy_check_action(false, false, 'robust_move_and_attack::NO_UNIT')
    end

    -- Getting target at beginning also, in case events mess up things along the way
    local target, target_x, target_y
    if target_loc then
        target_x, target_y = target_loc.x or target_loc[1], target_loc.y or target_loc[2]
        target = wesnoth.units.get(target_x, target_y)

        if (not target) then
            return ai_helper.dummy_check_action(false, false, 'robust_move_and_attack::NO_TARGET')
        end
    end

    local gamestate_changed = false
    local move_result = ai_helper.dummy_check_action(false, false, 'robust_move_and_attack::NO_ACTION')
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

            local unit_in_way = wesnoth.units.get(dst_x, dst_y)
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
                local unit_in_way_temp = wesnoth.units.get(dst_x, dst_y)
                if unit_in_way_temp then
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

---Make a copy of a table (rather than just another pointer to the same table)
---Note: This is only a shallow copy. Any nested references to tables, including WML tags,
---will still remain as a reference to the original table.
---@param t table
---@return table
function ai_helper.table_copy(t)
    local copy = {}
    for k,v in pairs(t) do copy[k] = v end
    return copy
end

---Merge two arrays without overwriting @a1 or @a2 -> create a new table
---This only works with arrays, not general tables
---@param a1 table
---@param a2 table
---@return table
function ai_helper.array_merge(a1, a2)
    local merger = {}
    for _,a in pairs(a1) do table.insert(merger, a) end
    for _,a in pairs(a2) do table.insert(merger, a) end
    return merger
end

---Convert input to a string in a format corresponding to the type of input
---The string is all put into one line
---@param input any
---@return string
function ai_helper.serialize(input)
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

---Split string str into a table using the delimiter sep (default: ',')
---@param str string
---@param sep string
---@return table
function ai_helper.split(str, sep)
    sep = sep or ","
    local fields = {}
    local pattern = string.format("([^%s]+)", sep)
    local _ = string.gsub(str, pattern, function(c) fields[#fields+1] = c end)
    return fields
end

ai_helper.split = wesnoth.deprecate_api('ai_helper.split', 'stringx.split', 3, '1.20', ai_helper.split)

--------- Location set related helper functions ----------

---Get the x,y coordinates for the index of a location set
---For some reason, there doesn't seem to be a LS function for this
---@param index integer
---@return integer
---@return integer
function ai_helper.get_LS_xy(index)
    local tmp_set = LS.of_raw{[index] = true}
    local xy = tmp_set:to_pairs()[1]

    return xy.x, xy.y
end

--------- Location, position or hex related helper functions ----------

---Converts coordinates from hex geometry to cartesian coordinates,
---meaning that y coordinates are offset by 0.5 every other hex
---Example: (1,1) stays (1,1) and (3,1) remains (3,1), but (2,1) -> (2,1.5) etc.
---@param x integer
---@param y integer
---@return number
---@return number
function ai_helper.cartesian_coords(x, y)
    return x, y + ((x + 1) % 2) / 2.
end

---Returns the angle of the direction from @from_hex to @to_hex
---Angle is in radians and goes from -pi to pi. 0 is toward east.
---Input hex tables can be of form { x, y } or { x = x, y = y }, which
---means that it is also possible to pass a unit table
---@param from_hex location
---@param to_hex location
---@return number
function ai_helper.get_angle(from_hex, to_hex)
    local x1, y1 = from_hex.x or from_hex[1], from_hex.y or from_hex[2]
    local x2, y2 = to_hex.x or to_hex[1], to_hex.y or to_hex[2]

    local _, y1cart =  ai_helper.cartesian_coords(x1, y1)
    local _, y2cart =  ai_helper.cartesian_coords(x2, y2)

    return math.atan(y2cart - y1cart, x2 - x1)
end

---Returns an integer index for the direction from from_hex to to_hex
---with the full circle divided into n slices
---1 is always to the east, with indices increasing clockwise
---Input hex tables can be of form { x, y } or { x = x, y = y }, which
---means that it is also possible to pass a unit table
---@param from_hex location
---@param to_hex location
---@param n integer
---@param center_on_east? boolean By default, the eastern direction is the northern border of the first slice. If this parameter is set, east will instead be the center direction of the first slice
---@return integer
function ai_helper.get_direction_index(from_hex, to_hex, n, center_on_east)
    local d_east = 0
    if center_on_east then d_east = 0.5 end

    local angle = ai_helper.get_angle(from_hex, to_hex)
    local index = math.floor((angle / math.pi * n/2 + d_east) % n ) + 1

    return index
end

---@param from_hex location
---@param to_hex location
---@return string
function ai_helper.get_cardinal_directions(from_hex, to_hex)
    local dirs = { "E", "S", "W", "N" }
    return dirs[ai_helper.get_direction_index(from_hex, to_hex, 4, true)]
end

---@param from_hex location
---@param to_hex location
---@return string
function ai_helper.get_intercardinal_directions(from_hex, to_hex)
    local dirs = { "E", "SE", "S", "SW", "W", "NW", "N", "NE" }
    return dirs[ai_helper.get_direction_index(from_hex, to_hex, 8, true)]
end

---@param from_hex location
---@param to_hex location
---@return string
function ai_helper.get_hex_facing(from_hex, to_hex)
    local dirs = { "se", "s", "sw", "nw", "n", "ne" }
    return dirs[ai_helper.get_direction_index(from_hex, to_hex, 6)]
end

---Find the hex that is opposite of hex with respect to center_hex
---Returns nil if hex and centre_hex are not adjacent.
---@param hex location
---@param center_hex location
---@return location?
function ai_helper.find_opposite_hex_adjacent(hex, center_hex)
    -- If the two input hexes are not adjacent, return nil
    if (M.distance_between(hex, center_hex) ~= 1) then return nil end

    -- Finding the opposite x position is easy
    local opp_x = center_hex.x + (center_hex.x - hex.x)

    -- y is slightly more tricky, because of the hexagonal shape, but there's a trick
    -- that saves us from having to build in a lot of if statements
    -- Among the adjacent hexes, it is the one with the correct x, and y _different_ from hex[2]
    for xa,ya in wesnoth.current.map:iter_adjacent(center_hex) do
        if (xa == opp_x) and (ya ~= hex.y) then
            return wesnoth.named_tuple({ xa, ya }, { 'x', 'y' })
        end
    end

    return nil
end

---Find the hex that is opposite of @hex with respect to @center_hex
---Using "square coordinate" method by JaMiT
---Note: this also works for non-adjacent hexes, but might return hexes that are not on the map!
---@param hex location
---@param center_hex location
---@return location
function ai_helper.find_opposite_hex(hex, center_hex)
    -- Finding the opposite x position is easy
    local opp_x = center_hex.x + (center_hex.x - hex.x)

    -- Going to "square geometry" for y coordinate
    local y_sq = hex.y * 2 - (hex.x % 2)
    local yc_sq = center_hex.y * 2 - (center_hex.x % 2)

    -- Now the same equation as for x can be used for y
    local opp_y = yc_sq + (yc_sq - y_sq)
    opp_y = math.floor((opp_y + 1) / 2)

    return wesnoth.named_tuple({opp_x, opp_y}, {'x', 'y'})
end

---Returns true if hex1 and hex2 are opposite from each other with respect to @center_hex
---@param hex1 location
---@param hex2 location
---@param center_hex location
---@return boolean
function ai_helper.is_opposite_adjacent(hex1, hex2, center_hex)
    local opp_hex = ai_helper.find_opposite_hex_adjacent(hex1, center_hex)

    if opp_hex and (opp_hex.x == hex2.x) and (opp_hex.y == hex2.y) then return true end
    return false
end

---Get coordinates for either a named location or from x/y coordinates specified
---in @cfg. The location can be provided:
---  - as name: cfg[param_core .. '_loc'] (string)
---  - or as coordinates: cfg[param_core .. '_x'] and cfg[param_core .. '_y'] (integers)
---This is the syntax used by many Micro AIs.
---Exception to this variable name syntax: if param_core = '', then the location
---variables are 'location_id', 'x' and 'y'
---
---An error is raised if the named location does not exist, or if
---the coordinates are not on the map. In addition, if required_for
---is provided, an error is also raised if neither a named location
---nor both coordinates are provided. If required_for is not passed and neither
---input exists, nil is returned. Omitting required_for does not suppress
---the other two error conditions.
---@param param_core string The base name of the key to look up in the config.
---@param cfg WMLTable The AI configuration to find the location in.
---@param required_for? string A string to be included in the error message in the event of failure.
---@return location?
function ai_helper.get_named_loc_xy(param_core, cfg, required_for)
    local param_loc = 'location_id'
    if (param_core ~= '') then param_loc = param_core .. '_loc' end
    local loc_id = cfg[param_loc]
    if loc_id then
        local loc = wesnoth.current.map.special_locations[loc_id]
        if loc then
            return loc
        else
            wml.error("Named location does not exist: " .. loc_id .. " " .. (required_for or ''))
        end
    end

    local param_x, param_y = 'x', 'y'
    if (param_core ~= '') then param_x, param_y = param_core .. '_x', param_core .. '_y' end
    local x, y = cfg[param_x], cfg[param_y]
    if x and y then
        if not wesnoth.current.map:on_board(x, y) then
            wml.error("Location is not on map: " .. param_x .. ',' .. param_y .. ' = ' .. x .. ',' .. y .. " " .. (required_for or ''))
        end

        return wesnoth.named_tuple({ x, y }, { 'x', 'y' })
    end

    if required_for then
        wml.error(required_for .. " requires either " .. param_loc .. "= or " .. param_x .. "/" .. param_y .. "= keys")
    end
    return nil
end

---Same as ai_helper.get_named_loc_xy, except that it takes comma separated
---lists of locations.
---The result is returned as an array of locations.
---Empty table is returned if no locations are found.
---An error is raised if x and y are provided but the sizes don't match.
---The error conditions in get_named_loc_xy can also be raised.
---@param param_core string The base name of the key to look up in the config.
---@param cfg WMLTable The AI configuration to find the location in.
---@param required_for? string A string to be included in the error message in the event of failure.
---@return location[]
function ai_helper.get_multi_named_locs_xy(param_core, cfg, required_for)
    local locs = {}
    local param_loc = 'location_id'
    if (param_core ~= '') then param_loc = param_core .. '_loc' end
    local cfg_loc = cfg[param_loc]
    if cfg_loc then
        local loc_ids = stringx.split(cfg_loc, ",")
        for _,loc_id in ipairs(loc_ids) do
            local tmp_cfg = {}
            tmp_cfg[param_loc] = loc_id
            local loc = ai_helper.get_named_loc_xy(param_core, tmp_cfg, required_for)
            table.insert(locs, loc)
        end
        return locs
    end

    local param_x, param_y = 'x', 'y'
    if (param_core ~= '') then param_x, param_y = param_core .. '_x', param_core .. '_y' end
    local cfg_x, cfg_y = cfg[param_x], cfg[param_y]
    if cfg_x and cfg_y then
        local xs = stringx.split(cfg_x, ",")
        local ys = stringx.split(cfg_y, ",")
        if (#xs ~= #ys) then
            wml.error("Coordinate lists need to have same number of elements: " .. param_x .. ' and ' .. param_y)
        end

        for i,x in ipairs(xs) do
            local tmp_cfg = {}
            tmp_cfg[param_x] = tonumber(x)
            tmp_cfg[param_y] = tonumber(ys[i])
            local loc = ai_helper.get_named_loc_xy(param_core, tmp_cfg, required_for)
            table.insert(locs, loc)
        end
        return locs
    end

    if required_for then
        wml.error(required_for .. " requires either " .. param_loc .. "= or " .. param_x .. "/" .. param_y .. "= keys")
    end

    return locs
end

---Returns the same locations array as wesnoth.map.find(location_filter),
---but excluding hexes on the map border.
---
---Note that this might not work if location_filter is a vconfig object.
---@param location_filter WMLTable
---@return terrain_hex[]
function ai_helper.get_locations_no_borders(location_filter)
    local old_include_borders = location_filter.include_borders
    location_filter.include_borders = false
    local locs = wesnoth.map.find(location_filter)
    location_filter.include_borders = old_include_borders
    return locs
end

---Get the location closest to hex (in format { x, y })
---that matches location_filter (in WML table format)
---Returns nil if no terrain matching the filter was found
---@param hex location Anchor location
---@param location_filter WMLTable Filter to match
---@param unit? unit Can be passed as an optional third parameter, in which case the terrain needs to be passable for that unit.
---@param avoid_map? location_set If given, the hexes in avoid_map are excluded
---@return terrain_hex?
function ai_helper.get_closest_location(hex, location_filter, unit, avoid_map)
    -- Find the maximum distance from 'hex' that's possible on the map
    local max_distance = 0
    local map = wesnoth.current.map
    local to_top_left = M.distance_between(hex, 0, 0)
    if (to_top_left > max_distance) then max_distance = to_top_left end
    local to_top_right = M.distance_between(hex, map.width-1, 0)
    if (to_top_right > max_distance) then max_distance = to_top_right end
    local to_bottom_left = M.distance_between(hex, 0, map.height-1)
    if (to_bottom_left > max_distance) then max_distance = to_bottom_left end
    local to_bottom_right = M.distance_between(hex, map.width-1, map.height-1)
    if (to_bottom_right > max_distance) then max_distance = to_bottom_right end

    -- If the hex is supposed to be passable for a unit, it cannot be on the map border
    local include_borders
    if unit then include_borders = 'no' end

    local radius = 0
    while (radius <= max_distance) do
        local loc_filter = {}
        if (radius == 0) then
            loc_filter = {
                wml.tag["and"] { x = hex.x, y = hex.y, include_borders = include_borders, radius = radius },
                wml.tag["and"] ( location_filter )
            }
        else
            loc_filter = {
                wml.tag["and"] { x = hex.x, y = hex.y, include_borders = include_borders, radius = radius },
                wml.tag["not"] { x = hex.x, y = hex.y, radius = radius - 1 },
                wml.tag["and"] ( location_filter )
            }
        end

        local locs = wesnoth.map.find(loc_filter)

        if avoid_map then
            for i = #locs,1,-1 do
                if avoid_map:get(locs[i]) then
                    table.remove(locs, i)
                end
            end
        end

        if unit then
            for _,loc in ipairs(locs) do
                local movecost = unit:movement_on(wesnoth.current.map[loc])
                if (movecost <= unit.max_moves) then return loc end
            end
        else
            if locs[1] then return locs[1] end
        end

        radius = radius + 1
    end

    return nil
end

---Finds all locations matching location_filter that are passable for
---the unit. This also excludes hexes on the map border.
---@param location_filter WMLTable Filter
---@param unit unit Unit to check passability; if omitted, all hexes matching the filter, but excluding border hexes are returned
---@return terrain_hex[]
function ai_helper.get_passable_locations(location_filter, unit)
    -- All hexes that are not on the map border
    local all_locs = ai_helper.get_locations_no_borders(location_filter)

    -- If a unit is provided, exclude terrain that's impassable for the unit
    if unit then
        local locs = {}
        for _,loc in ipairs(all_locs) do
            local movecost = unit:movement_on(wesnoth.current.map[loc])
            if (movecost <= unit.max_moves) then table.insert(locs, loc) end
        end
        return locs
    end

    return all_locs
end

---Finds all locations matching @location_filter that provide healing, excluding border hexes.
---@param location_filter WMLTable
---@return terrain_hex[]
function ai_helper.get_healing_locations(location_filter)
    local all_locs = ai_helper.get_locations_no_borders(location_filter)

    local locs = {}
    for _,loc in ipairs(all_locs) do
        if wesnoth.terrain_types[wesnoth.current.map[loc]].healing > 0 then
            table.insert(locs, loc)
        end
    end

    return locs
end

---Get the distance map DM for specified units (as a location set)
---DM = sum ( distance_from_unit )
---This is done for all elements of map (a location set), or for the entire map if map is not given
---@param units unit[]
---@param map? location_set
---@return location_set
function ai_helper.distance_map(units, map)
    local DM = LS.create()

    if map then
        map:iter(function(x, y, data)
            local dist = 0
            for _,unit in ipairs(units) do
                dist = dist + M.distance_between(unit, x, y)
            end
            DM:insert(x, y, dist)
        end)
    else
        for x, y in wesnoth.current.map:iter() do
            local dist = 0
            for _,unit in ipairs(units) do
                dist = dist + M.distance_between(unit, x, y)
            end
            DM:insert(x, y, dist)
        end
    end

    return DM
end

---Get the inverse distance map IDM for specified units (as a location set)
---IDM = sum ( 1 / (distance_from_unit+1) )
---This is done for all elements of map (a location set), or for the entire map if map is not given
---@param units unit[]
---@param map? location_set
---@return location_set
function ai_helper.inverse_distance_map(units, map)
    local IDM = LS.create()
    if map then
        map:iter(function(x, y, data)
            local dist = 0
            for _,unit in ipairs(units) do
                dist = dist + 1. / (M.distance_between(unit, x, y) + 1)
            end
            IDM:insert(x, y, dist)
        end)
    else
        for x, y in wesnoth.current.map:iter() do
            local dist = 0
            for _,unit in ipairs(units) do
                dist = dist + 1. / (M.distance_between(unit, x, y) + 1)
            end
            IDM:insert(x, y, dist)
        end
    end

    return IDM
end

---Determines distance of (x1,y1) from (x2,y2) even if
---x2 and y2 are not necessarily both given (or not numbers)
---@param x1 integer
---@param y1 integer
---@param x2 integer?
---@param y2 integer?
---@return integer
function ai_helper.generalized_distance(x1, y1, x2, y2)
    -- Return 0 if neither is given
    if (not x2) and (not y2) then return 0 end

    -- If only one of the parameters is set
    if (not x2) then return math.abs(y1 - y2) end
    if (not y2) then return math.abs(x1 - x2) end

    -- Otherwise, return standard distance
    return M.distance_between(x1, y1, x2, y2)
end

---Convert a list of locations as returned by wesnoth.map.find into a pair of strings
---suitable for passing in as x,y coordinate lists to wesnoth.map.find.
---Could alternatively convert to a WML table and use the find_in argument, but this is simpler.
---@param list location[]
---@return string #All the X coordinates as a comma-separated list
---@return string #All the Y coordinates as a comma-separated list
function ai_helper.split_location_list_to_strings(list)
    local locsx, locsy = {}, {}
    for i,loc in ipairs(list) do
        locsx[i] = loc.x
        locsy[i] = loc.y
    end
    return table.concat(locsx, ","), table.concat(locsy, ",")
end

---Returns a location set of hexes to be avoided by the AI. Information about
---these hexes can be provided in different ways:
---1. If avoid_tag is passed, we always use that. An example of this is when a
---   Micro AI configuration contains an [avoid] tag
---2. If use_ai_aspect (boolean) is set, we use the avoid aspect of the default AI.
---3. default_avoid_tag is used when avoid_tag is not passed and either
---   use_ai_aspect == false or the default AI aspect is not set.
---@param ai ailib
---@param avoid_tag WMLTable A location filter loaded from the MicroAI configuration
---@param use_ai_aspect boolean Whether to check the default AI avoid aspect
---@param default_avoid_tag WMLTable A hard-coded location filter to use if the aspect is not set
---@return location_set
function ai_helper.get_avoid_map(ai, avoid_tag, use_ai_aspect, default_avoid_tag)
    if avoid_tag then
        return LS.of_pairs(wesnoth.map.find(avoid_tag))
    end

    if use_ai_aspect then
        -- We need to be careful here as ai.aspects.avoid is an empty table both
        -- when the aspect is not set and when no hexes match the [avoid] tag.
        -- If @default_avoid_tag is not set, we can simply return the content of
        -- the aspect, it does not matter why it is an empty array (if it is).
        -- However, if @default_avoid_tag is set, we need to check whether the
        -- [avoid] tag is set for the default AI or not.

        if (not default_avoid_tag) then
            return LS.of_pairs(ai.aspects.avoid)
        else
            local ai_tag = wml.get_child(wesnoth.sides[wesnoth.current.side].__cfg, 'ai')
            for aspect in wml.child_range(ai_tag, 'aspect') do
                if (aspect.id == 'avoid') then
                    local facet = wml.get_child(aspect, 'facet')
                    if facet or aspect.name ~= "composite_aspect" then
                        -- If there's a [facet] child, it's set as a composite aspect,
                        -- with at least one facet.
                        -- But it might not be a composite aspect; it could be
                        -- a Lua aspect or a standard aspect.
                        return LS.of_pairs(ai.aspects.avoid)
                    end
                end
            end
        end
    end

    -- If we got here, that means neither @avoid_tag nor the default AI [avoid] aspect were used
    if default_avoid_tag then
        return LS.of_pairs(wesnoth.map.find(default_avoid_tag))
    else
        return LS.create()
    end
end


--------- Unit related helper functions ----------

---Check that viewing_side is valid and set to an existing side
---@param viewing_side? integer
---@param function_str? string
function ai_helper.check_viewing_side(viewing_side, function_str)
    if (not viewing_side) then
        error('ai_helper: missing required parameter viewing_side', 2)
    end
    if (type(viewing_side) ~= 'number') or (not wesnoth.sides[viewing_side]) then
        error('ai_helper: parameter viewing_side must be a valid side number', 2)
    end
end

---Check if the specified leader is set to be passive.
---@param aspect_value boolean|string[] The value of ai.aspects.passive_leader
---@param id string The leader's ID
---@return boolean
function ai_helper.is_passive_leader(aspect_value, id)
    if (type(aspect_value) == 'boolean') then return aspect_value end

    for _,aspect_id in ipairs(aspect_value) do
        if (aspect_id == id) then
            return true
        end
    end

    return false
end

---Find all living units on the map matching the specified filter.
---Living units is equivalent to non-petrified units.
---@param filter WMLTable
---@return unit[]
function ai_helper.get_live_units(filter)
    -- Note: the order of the filters and the [and] tags are important for speed reasons
    return wesnoth.units.find_on_map { wml.tag["not"] { status = "petrified" }, wml.tag["and"] ( filter ) }
end

---Find units who can move that match the specified filter.
---@param filter WMLTable
---@param exclude_guardians? boolean Whether to consider guardian units as able to move.
---@return unit[]
function ai_helper.get_units_with_moves(filter, exclude_guardians)
    -- Optional input: @exclude_guardians: set to 'true' to exclude units with ai_special=guardian
    -- Note: the order of the filters and the [and] tags are important for speed reasons
    local exclude_status = 'petrified'
    if exclude_guardians then
        exclude_status = exclude_status .. ',guardian'
    end
    return wesnoth.units.find_on_map {
        -- TODO: Should this also check for a case where the unit can't move because of terrain?
        -- That is, all adjacent terrains have a movement cost higher than the unit's moves left.
        wml.tag["and"] { formula = "moves > 0" },
        wml.tag["not"] { status = exclude_status },
        wml.tag["and"] ( filter )
    }
end

---Find units who can attack that match the specified filte.
---@param filter WMLTable
---@return unit[]
function ai_helper.get_units_with_attacks(filter)
    -- Note: the order of the filters and the [and] tags are important for speed reasons
    return wesnoth.units.find_on_map {
        wml.tag["and"] { formula = "attacks_left > 0 and size(attacks) > 0" },
        wml.tag["not"] { status = "petrified" },
        wml.tag["and"] ( filter )
    }
end

---Get units that are visible to the specified side
---@param viewing_side integer Must be set to a valid side number. If visibility is to be ignored, use wesnoth.units.find_on_map() instead.
---@param filter? WMLTable Standard unit filter WML table for the units; defaults to all units.
---@return table
function ai_helper.get_visible_units(viewing_side, filter)
    ai_helper.check_viewing_side(viewing_side)

    local filter_plus_vision = {}
    if filter then filter_plus_vision = ai_helper.table_copy(filter) end
    table.insert(filter_plus_vision, wml.tag.filter_vision { side = viewing_side, visible = 'yes' })

    local units = {}
    local all_units = wesnoth.units.find_on_map{}
    for _,unit in ipairs(all_units) do
        if unit:matches(filter_plus_vision) then
            table.insert(units, unit)
        end
    end

    return units
end

---Check whether a unit exists and is visible to the specified side
---@param viewing_side integer Must be set to a valid side number
---@param unit unit
---@return boolean
function ai_helper.is_visible_unit(viewing_side, unit)
    ai_helper.check_viewing_side(viewing_side)

    if (not unit) then return false end

    if unit:matches({ wml.tag.filter_vision { side = viewing_side, visible = 'no' } }) then
        return false
    end

    return true
end

---@class get_enemies_opts : ai_helper_visibility_opts
---@field avoid_map location_set If given, an enemy is included only if it does not have at least one adjacent hex outside of avoid_map

---Attackable enemies are defined as being being
---  - enemies of the specified side,
---  - not petrified
---  - and visible to the side as defined in cfg.viewing_side and cfg.ignore_visibility.
---  - have at least one adjacent hex that is not inside an area to avoid
---For speed reasons, this is done separately, rather than calling ai_helper.get_visible_units().
---@param filter? WMLTable Standard unit filter WML table for the enemies
---@param side? integer Side number, if side other than current side is to be considered
---@param cfg? get_enemies_opts
---@return table
function ai_helper.get_attackable_enemies(filter, side, cfg)
    side = side or wesnoth.current.side
    local viewing_side = cfg and cfg.viewing_side or side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    ---@type WMLTable
    local filter_plus_vision = {}
    if filter then filter_plus_vision = ai_helper.table_copy(filter) end
    if (not ignore_visibility) then
        filter_plus_vision = {
            wml.tag["and"] ( filter_plus_vision ),
            wml.tag.filter_vision { side = viewing_side, visible = 'yes' }
        }
    end

    local enemies = {}
    local all_units = wesnoth.units.find_on_map{}
    for _,unit in ipairs(all_units) do
        if wesnoth.sides.is_enemy(side, unit.side)
           and (not unit.status.petrified)
           and unit:matches(filter_plus_vision)
        then
            local is_avoided = false
            if cfg and cfg.avoid_map then
                is_avoided = true
                for xa,ya in wesnoth.current.map:iter_adjacent(unit) do
                    if (not cfg.avoid_map:get(xa, ya)) then
                        is_avoided = false
                        break
                    end
                end
            end
            if (not is_avoided) then
                table.insert(enemies, unit)
            end
        end
    end

    return enemies
end

---Check if a unit exists, is an enemy of the specifed side, is visible to the side as defined
---by cfg.viewing_side and cfg.ignore_visibility and is not petrified.
---@param unit unit
---@param side? integer Side number, defaults to current side.
---@param cfg? ai_helper_visibility_opts
---@return boolean
function ai_helper.is_attackable_enemy(unit, side, cfg)
    side = side or wesnoth.current.side
    local viewing_side = cfg and cfg.viewing_side or side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    if (not unit)
        or (not wesnoth.sides.is_enemy(side, unit.side))
        or unit.status.petrified
        or ((not ignore_visibility) and (not ai_helper.is_visible_unit(viewing_side, unit)))
    then
        return false
    end

    return true
end

---Return the enemy closest to the specified location and its distance from the location, or to the
---leader of the specified side if a location is not specified
---@param loc? location
---@param side? integer Number of side for which to find enemy; defaults to current side
---@param cfg? get_enemies_opts
---@return nil
---@return number
function ai_helper.get_closest_enemy(loc, side, cfg)
    side = side or wesnoth.current.side

    local enemies = ai_helper.get_attackable_enemies({}, side, cfg)

    local x, y
    if not loc then
        local leader = wesnoth.units.find_on_map { side = side, canrecruit = 'yes' }[1]
        x, y = leader.x, leader.y
    else
        x, y = loc[1], loc[2]
    end

    local closest_distance, closest_enemy = math.huge, nil
    for _,enemy in ipairs(enemies) do
        local enemy_distance = M.distance_between(x, y, enemy)
        if (enemy_distance < closest_distance) then
            closest_enemy = enemy
            closest_distance = enemy_distance
        end
    end

    return closest_enemy, closest_distance
end

---Get the cost of the cheapest unit that can be recruited by the current side.
---@param leader unit if given, find the cheapest recruit cost for this leader, otherwise for the combination of all leaders of the current side
---@return number
function ai_helper.get_cheapest_recruit_cost(leader)
    local recruit_ids = {}
    for _,recruit_id in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
        table.insert(recruit_ids, recruit_id)
    end

    local leaders
    if leader then
        leaders = { leader }
    else
        leaders = wesnoth.units.find_on_map { side = wesnoth.current.side, canrecruit = 'yes' }
    end
    for _,l in ipairs(leaders) do
        for _,recruit_id in ipairs(l.extra_recruit) do
            table.insert(recruit_ids, recruit_id)
        end
    end

    local cheapest_unit_cost = math.huge
    for _,recruit_id in ipairs(recruit_ids) do
        if wesnoth.unit_types[recruit_id].cost < cheapest_unit_cost then
            cheapest_unit_cost = wesnoth.unit_types[recruit_id].cost
        end
    end

    return cheapest_unit_cost
end

--------- Move related helper functions ----------

ai_helper.no_path = 42424242  -- Value returned by C++ engine for distance when no path is found

---@class dst_src_opts : reach_options
---@field moves? "'max'"|"'current'" If set to 'max', unit MP is set to max_moves before calculation

---Get the dst_src location set for the specified units.
---@param units unit[]
---@param cfg? dst_src_opts
---@return location_set
function ai_helper.get_dst_src_units(units, cfg)
    local max_moves = false
    if cfg then
        if (cfg['moves'] == 'max') then max_moves = true end
    end

    local dstsrc = LS.create()
    for _,unit in ipairs(units) do
        local tmp_moves = unit.moves
        if max_moves then
            unit.moves = unit.max_moves
        end
        local reach = wesnoth.paths.find_reach(unit, cfg)
        if max_moves then
            unit.moves = tmp_moves
        end

        for _,loc in ipairs(reach) do
            local tmp_dst = dstsrc:get(loc) or {}
            table.insert(tmp_dst, { x = unit.x, y = unit.y })
            dstsrc:insert(loc, tmp_dst)
        end
    end

    return dstsrc
end

---Get the dst_src location set for the specified units.
---If the units table is not given, use all units on the curent side.
---@param units? unit[]
---@param cfg? dst_src_opts
---@return location_set
function ai_helper.get_dst_src(units, cfg)
    if (not units) then
        units = wesnoth.units.find_on_map { side = wesnoth.current.side }
    end

    return ai_helper.get_dst_src_units(units, cfg)
end

---Get the dst_src location set for the specified enemy units.
---If the units table is not given, use all units on any enemy side.
---@param enemies? unit[]
---@param cfg? reach_options
---@return location_set
function ai_helper.get_enemy_dst_src(enemies, cfg)
    if (not enemies) then
        enemies = wesnoth.units.find_on_map {
            wml.tag.filter_side { wml.tag.enemy_of { side = wesnoth.current.side} }
        }
    end

    ---@type dst_src_opts
    local cfg_copy = {}
    if cfg then cfg_copy = ai_helper.table_copy(cfg) end
    cfg_copy.moves = 'max'

    return ai_helper.get_dst_src_units(enemies, cfg_copy)
end

---@class ai_move
---@field src location
---@field dst location

---Find all possible moves the current side can make
---@return ai_move[]
function ai_helper.my_moves()
    -- Produces an array with each field of form:
    --   [1] = { dst = { x = 7, y = 16 },
    --           src = { x = 6, y = 16 } }

    local dstsrc = ai.get_dst_src()

    ---@type ai_move[]
    local my_moves = {}
    for key,value in pairs(dstsrc) do
        local hex_x, hex_y = ai_helper.get_LS_xy(key)
        table.insert( my_moves,
            {   src = { x = value[1].x , y = value[1].y },
                dst = { x = hex_x , y = hex_y }
            }
        )
    end

    return my_moves
end

---Find all possible moves the enemy sides can make
---@return ai_move[]
function ai_helper.enemy_moves()
    -- Produces an array with each field of form:
    --   [1] = { dst = { x = 7, y = 16 },
    --           src = { x = 6, y = 16 } }

    local dstsrc = ai.get_enemy_dst_src()

    local enemy_moves = {}
    for key,value in pairs(dstsrc) do
        local hex_x, hex_y = ai_helper.get_LS_xy(key)
        table.insert( enemy_moves,
            {   src = { x = value[1].x , y = value[1].y },
                dst = { x = hex_x , y = hex_y }
            }
        )
    end

    return enemy_moves
end

---@class ai_next_hop_opts : shrouded_path_opts, reachmap_opts
---@field ignore_own_units? boolean If set to true, then own units that can move out of the way are ignored.
---@field path? location[] If given, find the next hop along this path, rather than doing new path finding In this case, it is assumed that the path is possible, in other words, that cost has been checked.
---@field fan_out? boolean Prior to Wesnoth 1.16, the unit strictly followed the path, which can lead to a line-up of units if there are allied units in the way (e.g. when multiple units are moved by the same candidate action. Now they fan out instead, trying to get as close to the ideal next_hop goal (defined as where the unit could get if there were no allied units in the way) as possible. Setting 'fan_out=false' restores the old behavior. The main disadvantage of the new method is that it needs to do more path finding and therefore takes longer.

---Finds the next "hop" of @unit on its way to (@x,@y)
---Returns coordinates of the endpoint of the hop (or nil if no path to
---(x,y) is found for the unit), and movement cost to get there.
---Only unoccupied hexes are considered
---@param unit unit The unit who's moving
---@param x integer Destination X coordinate
---@param y integer Destination Y coordinate
---@param cfg? ai_next_hop_opts Extra options, most of which are passed through to wesnoth.paths.find_path()
---@return location?
---@return integer
function ai_helper.next_hop(unit, x, y, cfg)
    local viewing_side = cfg and cfg.viewing_side or unit.side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    local path, cost
    if cfg and cfg.path then
        path = cfg.path
    else
        path, cost = ai_helper.find_path_with_shroud(unit, x, y, cfg)
        if cost >= ai_helper.no_path then return nil, cost end
    end

    -- If none of the hexes are unoccupied, use current position as default
    local next_hop, nh_cost = unit.loc, 0
    local next_hop_ideal = unit.loc

    -- Go through loop to find reachable, unoccupied hex along the path
    -- Start at second index, as first is just the unit position itself
    for i = 2,#path do
        if (not cfg) or (not cfg.avoid_map) or (not cfg.avoid_map:get(path[i])) then
            local sub_path, sub_cost = ai_helper.find_path_with_shroud(unit, path[i].x, path[i].y, cfg)

            if sub_cost <= unit.moves then
                -- Check for unit in way only if cfg.ignore_units is not set
                local unit_in_way
                if (not cfg) or (not cfg.ignore_units) then
                    unit_in_way = wesnoth.units.get(path[i])
                    if unit_in_way and (not ignore_visibility) and (not ai_helper.is_visible_unit(viewing_side, unit_in_way)) then
                        unit_in_way = nil
                    end

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
                next_hop_ideal = path[i]
            else
                break
            end
        end
    end

    local fan_out = cfg and cfg.fan_out
    if (fan_out == nil) then fan_out = true end
    if fan_out and ((next_hop.x ~= next_hop_ideal.x) or (next_hop.y ~= next_hop_ideal.y))
    then
        -- If we cannot get to the ideal next hop, try fanning out instead
        local reach = wesnoth.paths.find_reach(unit, cfg)

        -- Need the reach map of the unit from the ideal next hop hex
        -- There will always be another unit there, otherwise we would not have gotten here
        local unit_in_way = wesnoth.units.get(next_hop_ideal)
        unit_in_way:extract()
        local old_x, old_y = unit.x, unit.y
        unit:extract()
        unit:to_map(next_hop_ideal)
        local inverse_reach = wesnoth.paths.find_reach(unit, { ignore_units = true }) -- no ZoC
        unit:extract()
        unit:to_map(old_x, old_y)
        unit_in_way:to_map()

        local terrain1 = wesnoth.current.map[next_hop_ideal]
        local move_cost_endpoint = unit:movement_on(terrain1)
        local inverse_reach_map = LS.create()
        for _,r in pairs(inverse_reach) do
            -- We want the moves left for moving into the opposite direction in which the reach map was calculated
            local terrain2 = wesnoth.current.map[r]
            local move_cost = unit:movement_on(terrain2)
            local inverse_cost = r.moves_left + move_cost - move_cost_endpoint
            inverse_reach_map:insert(r, inverse_cost)
        end

        local units
        if ignore_visibility then
            units = wesnoth.units.find_on_map({ wml.tag["not"] { id = unit.id } })
        else
            units = ai_helper.get_visible_units(viewing_side, { wml.tag["not"] { id = unit.id } })
        end
        local unit_map = LS.create()
        for _,u in ipairs(units) do unit_map:insert(u.x, u.y, u.id) end

        -- Do not move farther away, but if next_hop is out of reach from next_hop_ideal,
        -- anything in reach is better -> set to -infinity in that case.
        local max_rating = inverse_reach_map:get(next_hop) or - math.huge
        for _,loc in ipairs(reach) do
            if (not unit_map:get(loc))
                and ((not cfg) or (not cfg.avoid_map) or (not cfg.avoid_map:get(loc)))
            then
                local rating = inverse_reach_map:get(loc) or - math.huge
                if (rating > max_rating) then
                    max_rating = rating
                    next_hop.x, next_hop.y = loc.x, loc.y -- eliminating the third argument
                end
            end
        end
    end

    return next_hop, nh_cost
end

---@class can_reach_opts : reachmap_opts, shrouded_path_opts

---Returns true if a hex is unoccupied (by a visible unit), or
---at most occupied by unit on same side as the seeking unit that can move away
---(can be modified with options below)
---@param unit unit
---@param x integer
---@param y integer
---@param cfg can_reach_opts
---@return boolean
function ai_helper.can_reach(unit, x, y, cfg)
    cfg = cfg or {}
    local viewing_side = cfg.viewing_side or unit.side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    -- Is there a visible unit at the goal hex?
    ---@type unit?
    local unit_in_way = wesnoth.units.get(x, y)
    if unit_in_way and (not ignore_visibility) and (not ai_helper.is_visible_unit(viewing_side, unit_in_way)) then
        unit_in_way = nil
    end

    if (cfg.exclude_occupied) and unit_in_way then
        return false
    end

    -- Otherwise, if 'ignore_units' is not set, return false if there's a unit of other side,
    -- or a unit of own side that cannot move away (this might be slow, don't know)
    if (not cfg.ignore_units) then
        -- If there's a unit at the goal that's not on own side (even ally), return false
        if unit_in_way and (unit_in_way.side ~= unit.side) then
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

---@class reachmap_opts : dst_src_opts, ai_helper_visibility_opts
---@field exclude_occupied? boolean If true, exclude hexes that have units on them, irrespective of the value of ignore_units; defaults to false, in which case hexes with own units with moves > 0 are included
---@field avoid_map? location_set A location set with the hexes the unit is not allowed to step on.

---Get all reachable hexes for @unit that are actually available; that is,
---hexes that, at most, have own units on them which can move out of the way.
---By contrast, wesnoth.paths.find_reach also includes hexes with allied units on
---them, as well as own unit with no moves left.
---Returned array is a location set, with values set to remaining MP after the
---unit moves to the respective hexes.
---@param unit unit
---@param cfg reachmap_opts
---@return location_set
function ai_helper.get_reachmap(unit, cfg)
    --   plus all other parameters to wesnoth.paths.find_reach

    local viewing_side = cfg and cfg.viewing_side or unit.side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    local old_moves = unit.moves
    if cfg and (cfg.moves == 'max') then unit.moves = unit.max_moves end

    local reachmap = LS.create()
    local initial_reach = wesnoth.paths.find_reach(unit, cfg)
    for _,loc in ipairs(initial_reach) do
        local is_available = true
        if cfg and cfg.avoid_map and cfg.avoid_map:get(loc) then
            is_available = false
        else
            ---@type unit?
            local unit_in_way = wesnoth.units.get(loc)
            if unit_in_way and (unit_in_way.id == unit.id) then
                unit_in_way = nil
            end
            if unit_in_way and (not ignore_visibility) and (not ai_helper.is_visible_unit(viewing_side, unit_in_way)) then
                unit_in_way = nil
            end

            if unit_in_way then
                if cfg and cfg.exclude_occupied then
                    is_available = false
                elseif (unit_in_way.side ~= unit.side) or (unit_in_way.moves == 0) then
                    is_available = false
                end
            end
        end

        if is_available then
            reachmap:insert(loc, loc.moves_left)
        end
    end

    unit.moves = old_moves

    return reachmap
end

---Same as ai_helper.get_reachmap with exclude_occupied = true
---This function is now redundant, but we keep it for backward compatibility.
---@param unit unit
---@param cfg reachmap_opts
---@return location_set
function ai_helper.get_reachable_unocc(unit, cfg)

    local cfg_GRU = cfg and ai_helper.table_copy(cfg) or {}
    cfg_GRU.exclude_occupied = true

    return ai_helper.get_reachmap(unit, cfg_GRU)
end

---@class shrouded_path_opts : path_options, ai_helper_visibility_opts

---Same as wesnoth.paths.find_path, just that it works under shroud as well while still ignoring invisible units. It does this by using ignore_visibility=true and taking invisible units off the map for the path finding process.
---@param unit unit The unit who wants to move.
---@param x integer Destination X coordinate.
---@param y integer Destination Y coordinate.
---@param cfg? shrouded_path_opts
---@return location[]
---@return integer
function ai_helper.find_path_with_shroud(unit, x, y, cfg)
    -- Notes on some of the optional parameters that can be passed in @cfg:
    --  - viewing_side: If not given, use side of the unit (not the current side!)
    --    for determining which units are hidden and need to be extracted, as that
    --    is what the path_finder code uses. If set to an invalid side, we can use
    --    default path finding as shroud is ignored then anyway.
    --  - ignore_visibility: see comments at beginning of this file. Defaults to nil.
    --      This applies to the units only in this function, as it always ignores shroud.
    --  - ignore_units: if true, hidden units do not need to be extracted because
    --    all units are ignored anyway

    local viewing_side = (cfg and cfg.viewing_side) or unit.side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    local path, cost
    if wesnoth.sides[viewing_side].shroud then
        local extracted_units = {}
        if (not cfg) or (not cfg.ignore_units) then
            local all_units = wesnoth.units.find_on_map{}
            for _,u in ipairs(all_units) do
                if (u.id ~= unit.id) and (u.side ~= viewing_side)
                    and (not ignore_visibility) and (not ai_helper.is_visible_unit(viewing_side, u))
                then
                    u:extract()
                    table.insert(extracted_units, u)
                end
            end
        end

        local cfg_copy = {}
        if cfg then cfg_copy = ai_helper.table_copy(cfg) end
        cfg_copy.ignore_visibility = true
        wesnoth.interface.handle_user_interact()
        path, cost = wesnoth.paths.find_path(unit, x, y, cfg_copy)

        for _,extracted_unit in ipairs(extracted_units) do
            extracted_unit:to_map()
        end
    else
        wesnoth.interface.handle_user_interact()
        path, cost = wesnoth.paths.find_path(unit, x, y, cfg)
    end

    return path, cost
end

---Custom cost function for path finding which takes hexes to be avoided into account.
---See the notes in function ai_helper.find_path_with_avoid()
---
---For efficiency reasons, this function requires quite a few arguments to be passed to it.
---Function ai_helper.find_path_with_avoid() does most of this automatically, but the custom cost
---function can be accessed directly also for even more customized behavior.
---@param x integer
---@param y integer
---@param prev_cost integer
---@param unit unit
---@param avoid_map location_set
---@param ally_map location_set
---@param enemy_map location_set
---@param enemy_zoc_map location_set
---@param strict_avoid boolean
---@return integer
function ai_helper.custom_cost_with_avoid(x, y, prev_cost, unit, avoid_map, ally_map, enemy_map, enemy_zoc_map, strict_avoid)
    if enemy_map and enemy_map:get(x, y) then
        return ai_helper.no_path
    end
    if strict_avoid and avoid_map and avoid_map:get(x, y) then
        return ai_helper.no_path
    end

    local max_moves = unit.max_moves
    local terrain = wesnoth.current.map[{x, y}]
    local move_cost = unit:movement_on(terrain)

    if (move_cost > max_moves) then
        return ai_helper.no_path
    end

    local prev_moves = math.floor(prev_cost)  -- remove all the minor ratings
    -- Note that prev_moves_left == max_moves if the unit ended turn on previous hex, as it should
    local prev_moves_left = max_moves - (unit.max_moves - unit.moves + prev_moves) % max_moves

    if enemy_zoc_map and enemy_zoc_map:get(x,y) then
        if (move_cost < prev_moves_left) then
            move_cost = prev_moves_left
        end
    end

    local moves_left = prev_moves_left - move_cost

    -- Determine whether previous hex was marked as unusable for ending the turn on (in the ones' place
    -- after multiplying by 100000), and also how many allied units are lines up along the path (tenth' place)
    -- Working with large integers for this part, in order to prevent rounding errors
    local prev_cost_int = math.floor(prev_cost * 100000 + 0.001)
    local unit_penalty = math.floor((prev_cost * 100000 - prev_cost_int + 0.001) * 10) / 10
    local avoid_penalty = math.floor(prev_cost_int - math.floor(prev_cost_int / 10) * 10 + 0.001)
    local move_cost_int = math.floor(move_cost * 100000 + 0.001)

    -- Apply unit_penalty only for the first turn
    local is_first_turn = false
    if (prev_moves < unit.moves) then is_first_turn = true end

    if is_first_turn then
        -- If the hex is both not-avoided and does not have a unit on it, we clear unit_penalty.
        -- Otherwise we add in the move cost of the current hex.
        -- The purpose of this is to have units spread out rather than move in a line, but note
        -- that this only works between paths to the hex that use up the same movement cost.
        -- It is fundamentally impossible with the Wesnoth A* search algorithm to make the unit
        -- choose a longer path in this way.
        if (ally_map and ally_map:get(x, y)) or (avoid_map and avoid_map:get(x, y)) then
            unit_penalty = unit_penalty + move_cost / 10
            -- We restrict this two 9 MP, even for units with more moves
            if (unit_penalty > 0.9) then unit_penalty = 0.9 end
            move_cost_int = move_cost_int + unit_penalty
        else
            move_cost_int = move_cost_int - unit_penalty
            unit_penalty = 0
        end
    end

    if (moves_left < 0) then
        -- This is the situation when there were moves left on the previous hex,
        -- but not enough to enter this hex. In this case, we need to apply the appropriate penalty:
        --  - If avoided hex: disqualify it
        --  - Otherwise use up full move on previous hex
        --  - Also, apply the unit line-up penalty, but only if this is the first move
        if (avoid_penalty > 0) then -- avoided hex
            return ai_helper.no_path
        end
        move_cost_int = move_cost_int + prev_moves_left * 100000
        if is_first_turn then
            move_cost_int = move_cost_int + unit_penalty * 10 * 100000 -- unit_penalty is multiples of 0.1
        end
    elseif (moves_left == 0) then
        -- And this is the case when moving to this hex uses up all moves for the turn
        if avoid_map and avoid_map:get(x, y) then
            return ai_helper.no_path
        end
        if is_first_turn then
            move_cost_int = move_cost_int + unit_penalty * 10 * 100000 -- unit_penalty is multiples of 0.1
        end
    end

    -- Here's the part that marks the hex as (un)usable
    -- We first need to subtract out the previous penalty
    move_cost_int = move_cost_int - avoid_penalty
    -- Then we need to add in a small number (remember everything is divided by 100000 at the end)
    -- because the move cost returned by this functions needs to be >= 1.  Use defense for this,
    -- thus giving a small bonus (low resulting move cost) for good terrain.
    -- Note that the returned cost is rounded to an integer by the engine, so for very long paths this
    -- will potentially add to the cost and might make the path inaccurate. However, for an average
    -- defense of 50 along the path, this will not happen until the path is 1000 hexes long. Also,
    -- in most cases this will simply add to the cost, rather than change the path itself.
    local defense = unit:defense_on(terrain)
    -- We need this to be multiples of 10 for the penalty identification to work
    defense = mathx.round(defense / 10) * 10
    if (defense > 90) then defense = 90 end
    if (defense < 10) then defense = 10 end
    move_cost_int = move_cost_int + (100 - defense)
    -- And finally we add a (very small) penalty for this hex if it is to be avoided
    -- This is used for the next hex to determine whether the previous hex was to be
    -- avoided via avoid_penalty above.
    if avoid_map and avoid_map:get(x, y) then
        move_cost_int = move_cost_int + 1
    end

    return move_cost_int / 100000
end

---@class path_with_avoid_opts
---@field strict_avoid? boolean If 'true', trigger the "strict avoid" mode described above
---@field ignore_enemies? boolean If 'true', enemies will not be taken into account.
---@field ignore_allies? boolean If 'true', allied units will not be taken into account.

---Find path while taking hexes to be avoided into account. In its default setting,
-- it also finds the path so that the unit does not end a move on a hex with an allied
-- unit, which is one of the main shortcomings of the default path finder.
--
-- Important notes:
--  - There are two modes of avoiding hexes: the default for which the unit may move through
--    the avoided area but not end a move on it; and a "strict avoid" mode for which the
--    path may not lead through the avoided area at all.
--  - Not ending turns on hexes with allied units is meant to with units moving around each other,
--    but this can cause problems in narrow passages. It can therefore also be turned off.
--  - This cost function does not provide all the configurability of the default path finder.
--    The functionality is as follows:
--     - Hexes with visible enemy units are always excluded, and enemy ZoC is taken into account
--     - Invisible enemies are always ignored (including those under shroud)
--     - Hexes with higher terrain defense are preferred, all else being equal.
---@param unit unit
---@param x integer
---@param y integer
---@param avoid_map location_set
---@param options path_with_avoid_opts
---@return nil
---@return integer
function ai_helper.find_path_with_avoid(unit, x, y, avoid_map, options)
    options = options or {}

    -- This needs to be done separately, otherwise a path that only goes a short time into the
    -- avoided area might not be disqualified correctly. It also saves evaluation time in other cases.
    if avoid_map:get(x,y) then
        return nil, ai_helper.no_path
    end

    local all_units = wesnoth.units.find_on_map{}
    local ally_map, enemy_map = LS.create(), LS.create()
    for _,u in ipairs(all_units) do
        if (u.id ~= unit.id) and ai_helper.is_visible_unit(wesnoth.current.side, u) then
            if wesnoth.sides.is_enemy(u.side, wesnoth.current.side) then
                if (not options.ignore_enemies) then
                    enemy_map:insert(u.x, u.y, u.level)
                end
            else
                if (not options.ignore_allies) then
                    ally_map:insert(u.x, u.y, u.level)
                end
            end
        end
    end

    local enemy_zoc_map = LS.create()
    if (not options.ignore_enemies) and (not unit:ability("skirmisher")) then
        enemy_map:iter(function(xx, yy, level)
            if (level > 0) then
                for xa,ya in wesnoth.current.map:iter_adjacent(xx, yy) do
                    enemy_zoc_map:insert(xa, ya, level)
                end
            end
        end)
    end

    -- Note: even though the cost function returns a float, the engine rounds the cost to an integer
    return ai_helper.find_path_with_shroud(unit, x, y, {
        calculate = function(xc, yc, current_cost)
            return ai_helper.custom_cost_with_avoid(xc, yc, current_cost, unit, avoid_map, ally_map, enemy_map, enemy_zoc_map, options.strict_avoid)
        end
    })
end

---@class best_move_opts : reachmap_opts
---@field labels? boolean If set, put labels with ratings onto map
---@field no_random? boolean If set, do not add random value between 0.0001 and 0.0099 to each hex

---Find the best move and best unit based on a rating_function
---@param units unit|unit[] single unit or list of units
---@param rating_function fun(x:integer, y:integer):integer rating function for the hexes the unit can reach
---@param cfg any
---@return location? #best_hex - nil if no valid moves were found
---@return unit? #best_unit - unit for which this rating function produced the maximum value; nil if no valid moves were found
---@return number #max_rating - the rating found for this hex/unit combination
function ai_helper.find_best_move(units, rating_function, cfg)
    cfg = cfg or {}

    -- If this is an individual unit, turn it into an array
    if units.hitpoints then units = { units } end

    local max_rating, best_hex, best_unit = - math.huge, nil, nil
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

---@class move_out_of_way_opts : reach_options, ai_helper_visibility_opts
---@field dx? integer The direction in which moving out of the way is preferred
---@field dy? integer The direction in which moving out of the way is preferred
---@field avoid_map? location_set If given, the hexes in avoid_map are excluded
---@field labels? boolean if set, display labels of the rating for each hex the unit can reach

---Move aunit to the best close location.
---Main rating is the moves the unit still has left after that
---@param ai ailib
---@param unit unit
---@param cfg move_out_of_way_opts
function ai_helper.move_unit_out_of_way(ai, unit, cfg)
    cfg = cfg or {}
    local viewing_side = cfg.viewing_side or unit.side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

    local dx, dy
    if cfg.dx and cfg.dy then
        local r = math.sqrt(cfg.dx * cfg.dx + cfg.dy * cfg.dy)
        if (r ~= 0) then dx, dy = cfg.dx / r, cfg.dy / r end
    end

    local reach = wesnoth.paths.find_reach(unit, cfg)
    local reach_map = LS.create()

    local max_rating = - math.huge
    ---@type location?
    local best_hex = nil
    for _,loc in ipairs(reach) do
        local unit_in_way = wesnoth.units.get(loc)
        if (not unit_in_way)       -- also excludes current hex
            or ((not ignore_visibility) and (not ai_helper.is_visible_unit(viewing_side, unit_in_way)))
        then
            local avoid_this_hex = cfg and cfg.avoid_map and cfg.avoid_map:get(loc)
            if (not avoid_this_hex) then
                local rating = loc.moves_left  -- also disfavors hexes next to visible enemy units for which loc[3] = 0

                if dx then
                    rating = rating + (loc.x - unit.x) * dx * 0.01
                    rating = rating + (loc.y - unit.y) * dy * 0.01
                end

                if cfg.labels then reach_map:insert(loc, rating) end

                if (rating > max_rating) then
                    max_rating, best_hex = rating, loc
                end
            end
        end
    end
    if cfg.labels then ai_helper.put_labels(reach_map) end

    if best_hex then
        ai_helper.checked_move(ai, unit, best_hex.x, best_hex.y)
    end
end

---Does ai.move_full for the unit if not at the specified location, otherwise ai.stopunit_moves
---Uses ai_helper.next_hop(), so that it works if unit cannot get there in one move
---@param ai ailib
---@param unit unit
---@param x integer
---@param y integer
---@return ai_result
---@overload fun(ai:ailib, unit:unit, loc:location):ai_result
function ai_helper.movefull_stopunit(ai, unit, x, y)
    if (type(x) ~= 'number') then
        if x[1] then
            x, y = x[1], x[2]
        else
            x, y = x.x, x.y
        end
    end
    -- TODO: Use read_location above (and in many other places as well)

    local next_hop = ai_helper.next_hop(unit, x, y)
    if next_hop and ((next_hop[1] ~= unit.x) or (next_hop[2] ~= unit.y)) then
        return ai_helper.checked_move_full(ai, unit, next_hop[1], next_hop[2])
    else
        return ai_helper.checked_stopunit_moves(ai, unit)
    end
end

---@class full_move_opts : shrouded_path_opts, move_out_of_way_opts

---Same as ai_help.movefull_stopunit(), but also moves a unit out of the way if there is one
---@param ai ailib
---@param unit unit
---@param x integer
---@param y integer
---@param cfg? full_move_opts
---@overload fun(ai:ailib, unit:unit, loc:location, cfg?:full_move_opts)
function ai_helper.movefull_outofway_stopunit(ai, unit, x, y, cfg)
    local viewing_side = cfg and cfg.viewing_side or unit.side
    ai_helper.check_viewing_side(viewing_side)
    local ignore_visibility = cfg and cfg.ignore_visibility

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
        local unit_in_way = wesnoth.units.get(x, y)
        if unit_in_way and (unit_in_way ~= unit)
            and (ignore_visibility or ai_helper.is_visible_unit(viewing_side, unit_in_way))
        then
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

---@class get_attacks_opts : dst_src_opts, ai_helper_visibility_opts
---@field include_occupied? boolean If set, also include hexes occupied by own-side units that can move away
---@field simulate_combat? boolean If set, also simulate the combat and return result (this is slow; only set if needed)

---@class ai_attack
---@field src location The location of the unit that can attack.
---@field dst location The location of the hex the unit should attack from.
---@field target location The location of the defending unit to be attacked.
---@field att_stats stats_evaluation?
---@field def_stats stats_evaluation?
---@field attack_hex_occupied boolean Whether an own unit that can move away is on the attack hex (dst).

---Get all attacks the specified units can do. Enemies invisible to the side
-- of those units are excluded, unless the option cfg.ignore_visibility=true is used.
---@param units unit[]
---@param cfg get_attacks_opts
---@return ai_attack[]
function ai_helper.get_attacks(units, cfg)
    cfg = cfg or {}

    ---@type ai_attack[]
    local attacks = {}
    if (not units[1]) then return attacks end

    local side = units[1].side  -- all units need to be on same side
    local ignore_visibility = cfg and cfg.ignore_visibility

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
    local all_units = wesnoth.units.find_on_map{}

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
            if ignore_visibility or ai_helper.is_visible_unit(side, unit) then
                other_unit_map:insert(unit.x, unit.y, i)
            end
        end
    end

    local attack_hex_map = LS.create()
    enemy_map:iter(function(e_x, e_y, i)
        for xa,ya in wesnoth.current.map:iter_adjacent(e_x, e_y) do
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
        wesnoth.interface.handle_user_interact()
        local reach
        if reaches:get(unit) then
            reach = reaches:get(unit)
        else
            reach = wesnoth.paths.find_reach(unit, cfg)
            reaches:insert(unit, reach)
        end

        for _,loc in ipairs(reach) do
            if attack_hex_map:get(loc) then
                local add_target = true
                local attack_hex_occupied = false

                -- If another unit of same side is on this hex:
                if my_unit_map:get(loc) and ((loc.x ~= unit.x) or (loc.y ~= unit.y)) then
                    attack_hex_occupied = true
                    add_target = false

                    if cfg.include_occupied then -- Test whether it can move out of the way
                        local unit_in_way = all_units[my_unit_map:get(loc)]
                        local uiw_reach
                        if reaches:get(unit_in_way) then
                            uiw_reach = reaches:get(unit_in_way)
                        else
                            uiw_reach = wesnoth.paths.find_reach(unit_in_way, cfg)
                            reaches:insert(unit_in_way.x, unit_in_way.y, uiw_reach)
                        end

                        -- Check whether the unit to move out of the way has an unoccupied hex to move to.
                        -- We do not deal with cases where a unit can move out of the way for a
                        -- unit that is moving out of the way of the initial unit (etc.).
                        for _,uiw_loc in ipairs(uiw_reach) do
                            -- Unit in the way of the unit in the way
                            local uiw_uiw = wesnoth.units.get(uiw_loc)
                            if (not uiw_uiw)
                                or ((not ignore_visibility) and (not ai_helper.is_visible_unit(side, uiw_uiw)))
                            then
                                add_target = true
                                break
                            end
                        end
                    end
                end

                if add_target then
                    for _,target in ipairs(attack_hex_map:get(loc)) do
                        local att_stats, def_stats
                        if cfg.simulate_combat then
                            local unit_dst = unit:clone()
                            unit_dst.x, unit_dst.y = loc.x, loc.y

                            local enemy = all_units[target.i]
                            att_stats, def_stats = wesnoth.simulate_combat(unit_dst, enemy)
                        end

                        table.insert(attacks, {
                            src = { x = unit.x, y = unit.y },
                            dst = { x = loc.x, y = loc.y },
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

---This is called from ai_helper.get_attack_combos_full() and
---builds up the combos for the next recursion level.
---It also calls the next recursion level, if possible
---@param combos table?
---@param attacks ai_attack[]
---@return table<integer, integer>[]
local function add_next_attack_combo_level(combos, attacks)
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
        combos_next_level = add_next_attack_combo_level(combos_this_level, attacks)
    end

    -- Finally, combine this level and next level combos
    combos_this_level = ai_helper.array_merge(combos_this_level, combos_next_level)
    return combos_this_level
end

---Calculate attack combination result by @units on @enemy
---All combinations of all units are taken into account, as well as their order
---This can result in a _very_ large number of possible combinations
---Use ai_helper.get_attack_combos() instead if order does not matter
---@param units unit[]
---@param enemy unit
---@param cfg get_attacks_opts
---@return table<integer, integer>[]
function ai_helper.get_attack_combos_full(units, enemy, cfg)
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
    local combos = add_next_attack_combo_level(nil, attacks)

    return combos
end

---Calculate attack combination result by @units on @enemy
---All the unit/hex combinations are considered, but without specifying the order of the
---attacks. Use ai_helper.get_attack_combos_full() if order matters.
---@param units unit[]
---@param enemy unit
---@param cfg shrouded_path_opts
---@return table<integer, integer>[]
---@return table<integer, integer[]>
function ai_helper.get_attack_combos(units, enemy, cfg)
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
        local all_units = wesnoth.units.find_on_map { side = wesnoth.current.side }
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
    for xa,ya in wesnoth.current.map:iter_adjacent(enemy) do
        -- Make sure the hex is not occupied by unit that cannot move out of the way

        local dst = xa * 1000 + ya

        for _,unit in ipairs(units) do
            if ((unit.x == xa) and (unit.y == ya)) or (not blocked_hexes:get(xa, ya)) then

                -- wesnoth.map.distance_between() is much faster than wesnoth.paths.find_path()
                --> pre-filter using the former
                local cost = M.distance_between(unit.x, unit.y, xa, ya)

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

    return attack_array, attacks_dst_src
end

---Get the damage multiplier for a given alignment and turn.
---@param alignment string The alignment to check.
---@param lawful_bonus integer The lawful bonus from the schedule for the desired turn.
---@return number
function ai_helper.get_unit_time_of_day_bonus(alignment, lawful_bonus)
    local multiplier = 1
    if (lawful_bonus ~= 0) then
        if (alignment == 'lawful') then
            multiplier = (1 + lawful_bonus / 100.)
        elseif (alignment == 'chaotic') then
            multiplier = (1 - lawful_bonus / 100.)
        elseif (alignment == 'liminal') then
            -- TODO: I think this is wrong?
            multiplier = (1 - math.abs(lawful_bonus) / 100.)
        end
    end
    return multiplier
end

return ai_helper
