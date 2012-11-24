local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local LS = wesnoth.require "lua/location_set.lua"

local ai_helper = {}

----- Debugging helper functions ------

function ai_helper.show_messages()
    -- Returns true or false (hard-coded).  To be used to
    -- show messages if in debug mode
    -- Just edit the following line (easier than trying to set WML variable)
    local show_messages_flag = false
    if wesnoth.game_config.debug then return show_messages_flag end
    return false
end

function ai_helper.print_exec()
    -- Returns true or false (hard-coded).  To be used to
    -- show which CA is being executed if in debug mode
    -- Just edit the following line (easier than trying to set WML variable)
    local print_exec_flag = false
    if wesnoth.game_config.debug then return print_exec_flag end
    return false
end

function ai_helper.print_eval()
    -- Returns true or false (hard-coded).  To be used to
    -- show which CA is being evaluated if in debug mode
    -- Just edit the following line (easier than trying to set WML variable)
    local print_eval_flag = false
    if wesnoth.game_config.debug then return print_eval_flag end
    return false
end

function ai_helper.done_eval_messages(start_time, ca_name)
    ca_name = ca_name or 'unknown'
    local dt = os.clock() - start_time
    if ai_helper.print_eval() then print('       - Done evaluating ' .. ca_name .. ':', os.clock(), ' ---------------> ', dt) end
    if (dt >= 10) then
        W.message{
            speaker = 'narrator',
            caption = 'Evaluation of candidate action ' .. ca_name .. ' took ' .. dt .. ' seconds',
            message = 'This took a really long time (which it should not).  If you can, would you mind sending us a screen grab of this situation?  Thanks!'
        }
    end
end

function ai_helper.clear_labels()
    -- Clear all labels on a map
    local w,h,b = wesnoth.get_map_size()
    for x = 1,w do
        for y = 1,h do
          W.label { x = x, y = y, text = "" }
        end
    end
end

function ai_helper.put_labels(map, factor)
    -- Take map (location set) and put label containing 'value' onto the map
    -- factor: multiply by 'factor' if set
    -- print 'nan' if element exists but is not a number

    factor = factor or 1

    ai_helper.clear_labels()
    map:iter(function(x, y, data)
          local out = tonumber(data) or 'nan'
          if (out ~= 'nan') then out = out * factor end
          W.label { x = x, y = y, text = out }
    end)
end

----- General functionality and maths helper functions ------

function ai_helper.got_1_11()
   if not wesnoth.compare_versions then return false end
   return wesnoth.compare_versions(wesnoth.game_config.version, ">=", "1.11.0")
end

function ai_helper.filter(input, condition)
    -- equivalent of filter() function in Formula AI

    local filtered_table = {}

    for i,v in ipairs(input) do
        if condition(v) then
            --print(i, "true")
            table.insert(filtered_table, v)
        end
    end

    return filtered_table
end

function ai_helper.choose(input, value)
    -- equivalent of choose() function in Formula AI
    -- Returns element of a table with the largest 'value' (a function)
    -- Also returns the max value and the index

    local max_value = -9e99
    local best_input = nil
    local best_key = nil

    for k,v in pairs(input) do
        if value(v) > max_value then
            max_value = value(v)
            best_input = v
            best_key = k
        end
        --print(k, value(v), max_value)
    end

    return best_input, max_value, best_key
end

function ai_helper.random(min, max)
    -- Use this function as Lua's 'math.random' is not replay or MP safe

    if not max then min, max = 1, min end
    wesnoth.fire("set_variable", { name = "LUA_random", rand = string.format("%d..%d", min, max) })
    local res = wesnoth.get_variable "LUA_random"
    wesnoth.set_variable "LUA_random"
    return res
end

function ai_helper.table_copy(t)
    -- Make a copy of a table (rather than just another pointer to the same table)
    local copy = {}
    for k,v in pairs(t) do copy[k] = v end
    return copy
end

function ai_helper.array_merge(a1, a2)
    -- Merge two arrays
    -- I want to do this without overwriting t1 or t2 -> create a new table
    -- This only works with arrays, not general tables
    local merger = {}
    for i,a in pairs(a1) do table.insert(merger, a) end
    for i,a in pairs(a2) do table.insert(merger, a) end
    return merger
end

--------- Location set related helper functions ----------

function ai_helper.get_LS_xy(index)
    -- Get the x,y coordinates from a location set index
    -- For some reason, there doesn't seem to be a LS function for this

    local tmp_set = LS.create()
    tmp_set.values[index] = 1
    local xy = tmp_set:to_pairs()[1]

    return xy[1], xy[2]
end

function ai_helper.LS_of_triples(table)
    -- Create a location set from a table of 3-element tables
    -- Elements 1 and 2 are x,y coordinates, #3 is value to be inserted

    local set = LS.create()
    for k,t in pairs(table) do
        set:insert(t[1], t[2], t[3])
    end
    return set
end

function ai_helper.to_triples(set)
    local res = {}
    set:iter(function(x, y, v) table.insert(res, { x, y, v }) end)
    return res
end

function ai_helper.LS_random_hex(set)
    -- Select a random hex from the hexes in location set 'set'
    -- This seems "inelegant", but I can't come up with another way without creating an extra array
    -- Return -1, -1 if set is empty

    local r = ai_helper.random(set:size())
    local i, xr, yr = 1, -1, -1
    set:iter( function(x, y, v)
        if (i == r) then xr, yr = x, y end
        i = i + 1
    end)

    return xr, yr
end

--------- Location, position or hex related helper functions ----------

function ai_helper.find_opposite_hex_adjacent(hex, center_hex)
    -- Find the hex that is opposite of 'hex' w.r.t. 'center_hex'
    -- Both input hexes are of format { x, y }
    -- Output: {opp_x, opp_y} -- or nil if 'hex' and 'center_hex' are not adjacent (or no opposite hex is found, e.g. for hexes on border)

    -- If the two input hexes are not adjacent, return nil
    if (H.distance_between(hex[1], hex[2], center_hex[1], center_hex[2]) ~= 1) then return nil end

    -- Finding the opposite x position is easy
    local opp_x = center_hex[1] + (center_hex[1] - hex[1])

    -- y is slightly more tricky, because of the hexagonal shape, but there's a neat trick
    -- that saves us from having to build in a lot of if statements
    -- Among the adjacent hexes, it is the one with the correct x, and y _different_ from hex[2]
    for x, y in H.adjacent_tiles(center_hex[1], center_hex[2]) do
        if (x == opp_x) and (y ~= hex[2]) then return { x, y } end
    end

    return nil
end

function ai_helper.find_opposite_hex(hex, center_hex)
    -- Find the hex that is opposite of 'hex' w.r.t. 'center_hex'
    -- Using "square coordinate" method by JaMiT
    -- Note: this also works for non-adjacent hexes, but might return hexes that are not on the map!
    -- Both input hexes are of format { x, y }
    -- Output: {opp_x, opp_y}

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
    -- Returns true if 'hex1' and 'hex2' are opposite from each other w.r.t center_hex

    local opp_hex = ai_helper.find_opposite_hex_adjacent(hex1, center_hex)

    if opp_hex and (opp_hex[1] == hex2[1]) and (opp_hex[2] == hex2[2]) then return true end
    return false
end

function ai_helper.get_closest_location(hex, location_filter)
    -- Get the location closest to 'hex' (in format { x, y })
    -- that matches 'location_filter' (in WML table format)
    -- Returns nil if no terrain matching the filter was found

    local locs = wesnoth.get_locations(location_filter)

    local max_rating, best_hex = -9e99, {}
    for i,l in ipairs(locs) do
        local rating = -H.distance_between(hex[1], hex[2], l[1], l[2])
        if (rating > max_rating) then
            max_rating, best_hex = rating, l
        end
    end

    if (max_rating > -9e99) then
        return best_hex, -max_rating
    else
        return nil, 9e99
    end
end

function ai_helper.distance_map(units, map)
    -- Get the distance map for all units in 'units' (as a location set)
    -- DM = sum ( distance_from_unit )
    -- This is done for all elements of 'map' (a locations set), or for the entire map if 'map' is not given

    local DM = LS.create()

    if map then
        map:iter(function(x, y, data)
            local dist = 0
            for i,u in ipairs(units) do
                dist = dist + H.distance_between(u.x, u.y, x, y)
            end
            DM:insert(x, y, dist)
        end)
    else
        local w,h,b = wesnoth.get_map_size()
        for x = 1,w do
            for y = 1,h do
                local dist = 0
                for i,u in ipairs(units) do
                    dist = dist + H.distance_between(u.x, u.y, x, y)
                end
                DM:insert(x, y, dist)
            end
        end
    end
    --ai_helper.put_labels(DM)
    --W.message {speaker="narrator", message="Distance map" }

    return DM
end

function ai_helper.inverse_distance_map(units, map)
    -- Get the inverse distance map for all units in 'units' (as a location set)
    -- IDM = sum ( 1 / (distance_from_unit+1) )
    -- This is done for all elements of 'map' (a locations set), or for the entire map if 'map' is not given

    local IDM = LS.create()
    if map then
        map:iter(function(x, y, data)
            local dist = 0
            for i,u in ipairs(units) do
                dist = dist + 1. / (H.distance_between(u.x, u.y, x, y) + 1)
            end
            IDM:insert(x, y, dist)
        end)
    else
        local w,h,b = wesnoth.get_map_size()
        for x = 1,w do
            for y = 1,h do
                local dist = 0
                for i,u in ipairs(units) do
                    dist = dist + 1. / (H.distance_between(u.x, u.y, x, y) + 1)
                end
                IDM:insert(x, y, dist)
            end
        end
    end
    --ai_helper.put_labels(IDM)
    --W.message {speaker="narrator", message="Inverse distance map" }

    return IDM
end

function ai_helper.generalized_distance(x1, y1, x2, y2)
    -- determines "distance of (x1,y1) from (x2,y2) even if
    -- x2 and y2 are not necessarily both given (or not numbers)

    -- Return 0 if neither is given
    if (not x2) and (not y2) then return 0 end

    -- If only one of the parameters is set
    if (not x2) then return math.abs(y1 - y2) end
    if (not y2) then return math.abs(x1 - x2) end

    -- Otherwise, return standard distance
    return H.distance_between(x1, y1, x2, y2)
end

function ai_helper.xyoff(x, y, ori, hex)
    -- Finds hexes at a certain offset from x,y
    -- ori: direction/orientation: north (0), ne (1), se (2), s (3), sw (4), nw (5)
    -- hex: string for the hex to be queried.  Possible values:
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
    -- Convert a list of locations as returned by wesnoth.get_locations into a pair of strings
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

    filter = filter or {}

    -- So that 'filter' in calling function is not modified (if it's a variable):
    local live_filter = ai_helper.table_copy(filter)

    local filter_not_petrified = { "not", {
        { "filter_wml", {
            { "status", { petrified = "yes" } }
        } }
    } }

    -- Combine the two filters.  Doing it this way around is much easier (always works, no ifs required),
    -- but it means we need to make a copy of the filter above, so that the original does not get changed
    table.insert(live_filter, filter_not_petrified)

    return wesnoth.get_units(live_filter)
end

function ai_helper.get_closest_enemy(loc)
    -- Get the closest enemy to loc, or the leader if loc not specified
    local x, y

    local enemies = ai_helper.get_live_units {
        { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
    }

    if not loc then
        local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
        x = leader.x
        y = leader.y
    else
        x = loc[1]
        y = loc[2]
    end

    local closest_distance, location = 9e99, {}
    for i,u in ipairs(enemies) do
        enemy_distance = H.distance_between(x, y, u.x, u.y)
        if enemy_distance < closest_distance then
            closest_distance = enemy_distance
            location = { x = u.x, y = u.y}
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
    -- Returns true/false depending on whether unit has a weapon with the given special
    -- Also returns the number of the first poisoned weapon
    local weapon_number = 0
    for att in H.child_range(unit.__cfg, 'attack') do
        weapon_number = weapon_number + 1
        for sp in H.child_range(att, 'specials') do
            if H.get_child(sp, special) then
                return true, weapon_number
            end
        end
    end
    return false
end

function ai_helper.get_cheapest_recruit_cost()
    local cheapest_unit_cost = 9e99
    for i, recruit_id in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
        if wesnoth.unit_types[recruit_id].cost < cheapest_unit_cost then
            cheapest_unit_cost = wesnoth.unit_types[recruit_id].cost
        end
    end
    return cheapest_unit_cost
end

--------- Move related helper functions ----------

ai_helper.no_path = 42424242  -- Value returned by engine for distance when no path is found

function ai_helper.get_dst_src_units(units, cfg)
    -- Get the dst_src LS for 'units'
    -- cfg: configuration table
    --   - moves: if set to 'max' use max_moves of units, rather than current moves

    local max_moves = false
    if cfg then
        if (cfg['moves'] == 'max') then max_moves = true end
    end

    local dstsrc = LS.create()
    for i,u in ipairs(units) do
        -- If {moves = 'max} is set
        local tmp = u.moves
        if max_moves then
            u.moves = u.max_moves
        end
        local reach = wesnoth.find_reach(u)
        if max_moves then
            u.moves = tmp
        end
        for j,r in ipairs(reach) do
            local tmp = dstsrc:get(r[1], r[2]) or {}
            table.insert(tmp, {x = u.x, y = u.y})
            dstsrc:insert(r[1], r[2], tmp)
        end
    end
    return dstsrc
end

function ai_helper.get_dst_src(units)
    -- Produces the same output as ai.get_dst_src()   (available in 1.11.0)
    -- If units is given, use them, otherwise do it for all units on the current side

    local my_units = {}
    if units then
        my_units = units
    else
        my_units = wesnoth.get_units { side = wesnoth.current.side }
    end

    return ai_helper.get_dst_src_units(my_units)
end

function ai_helper.get_enemy_dst_src()
    -- Produces the same output as ai.get_enemy_dst_src()   (available in 1.11.0)

    local enemies = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }

    return ai_helper.get_dst_src_units(enemies, {moves = 'max'})
end

function ai_helper.my_moves()
    -- Produces a table with each (numerical) field of form:
    --   [1] = { dst = { x = 7, y = 16 },
    --           src = { x = 6, y = 16 } }

    local dstsrc = ai.get_dstsrc()

    local my_moves = {}
    for key,value in pairs(dstsrc) do
        --print("src: ",value[1].x,value[1].y,"    -- dst: ",key.x,key.y)
        table.insert( my_moves,
            {   src = { x = value[1].x , y = value[1].y },
                dst = { x = key.x , y = key.y }
            }
        )
    end

    return my_moves
end

function ai_helper.enemy_moves()
    -- Produces a table with each (numerical) field of form:
    --   [1] = { dst = { x = 7, y = 16 },
    --           src = { x = 6, y = 16 } }

    local dstsrc = ai.get_enemy_dstsrc()

    local enemy_moves = {}
    for key,value in pairs(dstsrc) do
        --print("src: ",value[1].x,value[1].y,"    -- dst: ",key.x,key.y)
        table.insert( enemy_moves,
            {   src = { x = value[1].x , y = value[1].y },
                dst = { x = key.x , y = key.y }
            }
        )
    end

    return enemy_moves
end

function ai_helper.next_hop(unit, x, y, cfg)
    -- Finds the next "hop" of 'unit' on its way to (x,y)
    -- Returns coordinates of the endpoint of the hop, and movement cost to get there
    -- only unoccupied hexes are considered
    -- cfg: standard extra options for wesnoth.find_path()
    --   plus:
    --     ignore_own_units: if set to true, then own units that can move out of the way are ignored

    local path, cost = wesnoth.find_path(unit, x, y, cfg)

    -- If unit cannot get there:
    if cost >= ai_helper.no_path then return nil, cost end

    -- If none of the hexes are unoccupied, use current position as default
    local next_hop, nh_cost = {unit.x, unit.y}, 0

    -- Go through loop to find reachable, unoccupied hex along the path
    -- Start at second index, as first is just the unit position itself
    for i = 2,#path do
        local sub_path, sub_cost = wesnoth.find_path( unit, path[i][1], path[i][2], cfg)

        if sub_cost <= unit.moves then
            local unit_in_way = wesnoth.get_unit(path[i][1], path[i][2])

            -- If ignore_own_units is set, ignore own side units that can move out of the way
            if cfg and cfg.ignore_own_units then
                if unit_in_way and (unit_in_way.side == unit.side) then
                    local reach = ai_helper.get_reachable_unocc(unit_in_way)
                    if (reach:size() > 1) then unit_in_way = nil end
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
    -- Returns true if unit can reach (x,y), else false
    -- This only returns true if the hex is unoccupied, or at most occupied by unit on same side as 'unit'
    -- that can move away (can be modified with options below)
    -- cfg:
    --   moves = 'max' use max_moves instead of current moves
    --   ignore_units: if true, ignore both own and enemy units
    --   exclude_occupied: if true, exclude hex if there's a unit there, irrespective of value of 'ignore_units'

    -- If 'cfg' is not set, we need it as an empty array
    cfg = cfg or {}

    -- Is there a unit at the goal hex?
    local unit_in_way = wesnoth.get_unit(x, y)

    -- If there is, and 'exclude_occupied' is set, always return false
    if (cfg.exclude_occupied) and unit_in_way then return false end

    -- Otherwise, if 'ignore_units' is not set, return false if there's a unit of other side,
    -- or a unit of own side that cannot move away (this might be slow, don't know)
    if (not cfg.ignore_units) then
        -- If there's a unit at the goal that's not on own side (even ally), return false
        if unit_in_way and (unit_in_way.side ~= unit.side) then return false end

        -- If the unit in the way is on 'unit's' side and cannot move away, also return false
        if unit_in_way and (unit_in_way.side == unit.side) then
            -- need to pass the cfg here so that it works for enemy units (generally with no moves left) also
            local move_away = ai_helper.get_reachable_unocc(unit_in_way, cfg)
            if (move_away:size() <= 1) then return false end
        end
    end

    -- After all that, test whether our unit can actually get there
    -- Set moves to max_moves, if { moves = 'max' } is set
    local old_moves = unit.moves
    if (cfg.moves == 'max') then unit.moves = unit.max_moves end

    local can_reach = false
    local path, cost = wesnoth.find_path(unit, x, y, cfg)
    if (cost <= unit.moves) then can_reach = true end

    -- Reset moves
    unit.moves = old_moves

    return can_reach
end

function ai_helper.get_reachable_unocc(unit, cfg)
    -- Get all reachable hexes for unit that are unoccupied (incl. by allied units)
    -- Returned array is a location set, with value = 1 for each reachable hex
    -- cfg: parameters to wesnoth.find_reach, such as {additional_turns = 1}
    -- additional, {moves = 'max'} can be set inside cfg, which sets unit MP to max_moves before calculation

    local old_moves = unit.moves
    if cfg then
        if (cfg.moves == 'max') then unit.moves = unit.max_moves end
    end

    local reach = LS.create()
    local initial_reach = wesnoth.find_reach(unit, cfg)

    for i,loc in ipairs(initial_reach) do
        local unit_in_way = wesnoth.get_unit(loc[1], loc[2])
        if not unit_in_way then
            reach:insert(loc[1], loc[2], 1)
        end
    end

    -- Also need to include the hex the unit is on itself
    reach:insert(unit.x, unit.y, 1)

    -- Reset unit moves (can be done whether it was changed or not)
    unit.moves = old_moves

    return reach
end

function ai_helper.find_best_move(units, rating_function, cfg)
    -- Find the best move and best unit based on 'rating_function'
    -- INPUTS:
    --  units: single unit or table of units
    --  rating_function: function(x, y) with rating function for the hexes the unit can reach
    --  cfg: table with elements
    --    labels: if set, put labels with ratings onto map
    --    no_random: if set, do not add random value between 0.0001 and 0.0099 to each hex
    --               (otherwise that's the default)
    -- OUTPUTS:
    --  best_hex: format { x, y }
    --  best_unit: unit for which this rating function produced the maximum value
    -- If no valid moves were found, best_unit and best_hex are empty arrays

    -- If 'cfg' is not set, we need it as an empty array
    cfg = cfg or {}

    -- If this is an individual unit, turn it into an array
    if units.hitpoints then units = { units } end

    local max_rating, best_hex, best_unit = -9e99, {}, {}
    for i,u in ipairs(units) do
        -- Hexes each unit can reach
        local reach_map = ai_helper.get_reachable_unocc(u)
        reach_map:iter( function(x, y, v)
            -- Rate based on rating_function argument
            local rating = rating_function(x, y)

            -- If cfg.random is set, add some randomness (on 0.0001 - 0.0099 level)
            if (not cfg.no_random) then rating = rating + ai_helper.random(99) / 10000. end
            -- If cfg.labels is set: insert values for label map
            if cfg.labels then reach_map:insert(x, y, rating) end

            if rating > max_rating then
                max_rating, best_hex, best_unit = rating, { x, y }, u
            end
        end)
        if cfg.labels then ai_helper.put_labels(reach_map) end
    end

    return best_hex, best_unit
end

function ai_helper.move_unit_out_of_way(ai, unit, cfg)
    -- Find best close location to move unit to
    -- Main rating is the moves the unit still has left after that
    -- Other, configurable, parameters are given to function in 'cfg':
    --   - dx, dy: the direction in which moving out of the way is preferred
    --   - labels: if set, display labels of the rating for each hex the unit can reach

    cfg = cfg or {}

    local reach = wesnoth.find_reach(unit)
    local reach_map = LS.create()

    local max_rating, best_hex = -9e99, {}
    for i,r in ipairs(reach) do
        local unit_in_way = wesnoth.get_unit(r[1], r[2])
        if (not unit_in_way) then  -- also excludes current hex
            local rating = r[3]  -- also disfavors hexes next to enemy units for which r[3] = 0

            if cfg.dx then rating = rating + (r[1] - unit.x) * cfg.dx end
            if cfg.dy then rating = rating + (r[2] - unit.y) * cfg.dy end

            if cfg.labels then reach_map:insert(r[1], r[2], rating) end

            if (rating > max_rating) then
                max_rating, best_hex = rating, { r[1], r[2] }
            end
        end
    end
    if cfg.labels then ai_helper.put_labels(reach_map) end

    if (max_rating > -9e99) then
        --W.message { speaker = unit.id, message = 'Moving out of way' }
        ai.move(unit, best_hex[1], best_hex[2])
    end
end

function ai_helper.movefull_stopunit(ai, unit, x, y)
    -- Does ai.move_full for a unit if not at (x,y), otherwise ai.stopunit_moves
    -- Uses ai_helper.next_hop(), so that it works if unit cannot get there in one move
    -- Coordinates can be given as x and y components, or as a 2-element table { x, y }
    if (type(x) ~= 'number') then
        if x[1] then
            x, y = x[1], x[2]
        else
            x, y = x.x, x.y
        end
    end

    local next_hop = ai_helper.next_hop(unit, x, y)
    if next_hop and ((next_hop[1] ~= unit.x) or (next_hop[2] ~= unit.y)) then
        ai.move_full(unit, next_hop[1], next_hop[2])
    else
        ai.stopunit_moves(unit)
    end
end

function ai_helper.movefull_outofway_stopunit(ai, unit, x, y, cfg)
    -- Same as ai_help.movefull_stopunit(), but also moves unit out of way if there is one
    -- Additional input: cfg for ai_helper.move_unit_out_of_way()
    if (type(x) ~= 'number') then
        if x[1] then
            x, y = x[1], x[2]
        else
            x, y = x.x, x.y
        end
    end

    -- Only move unit out of way if the main unit can get there
    local path, cost = wesnoth.find_path(unit, x, y)
    if (cost <= unit.moves) then
        local unit_in_way = wesnoth.get_unit(x, y)
        if unit_in_way and ((unit_in_way.x ~= unit.x) or (unit_in_way.y ~= unit.y)) then
            --W.message { speaker = 'narrator', message = 'Moving out of way' }
            ai_helper.move_unit_out_of_way(ai, unit_in_way, cfg)
        end
    end

    local next_hop = ai_helper.next_hop(unit, x, y)
    if next_hop and ((next_hop[1] ~= unit.x) or (next_hop[2] ~= unit.y)) then
        ai.move_full(unit, next_hop[1], next_hop[2])
    else
        ai.stopunit_moves(unit)
    end
end

---------- Attack related helper functions --------------

function ai_helper.get_attacks_unit(unit, cfg)
    -- Get all attacks a unit can do
    -- This includes a variety of configurable options, passed in the 'cfg' table
    -- cfg: table with config parameters
    --  moves: "current" (default for units on current side) or "max" (always used for units on other sides)
    --  include_occupied (false): if set, also include hexes occupied by own-side units that can move away
    --  simulate_combat (false): if set, also simulate the combat and return result (this is slow; only set if needed)

    -- Returns {} if no attacks can be done, otherwise table with fields
    --   dst: { x = x, y = y } of attack position
    --   src: { x = x, y = y } of attacking unit (don't use id, could be ambiguous)
    --   target: { x = x, y = y } of defending unit
    --   att_stats, def_stats: as returned by wesnoth.simulate_combat (if cfg.simulate_combat is set)
    --   attack_hex_occupied: boolean storing whether an own unit that can move away is on the attack hex

    cfg = cfg or {}

    -- 'moves' can be either "current" or "max"
    -- For unit on current side: use "current" by default, or override by cfg.moves
    local moves = cfg.moves or "current"
    -- For unit on any other side, only moves="max" makes sense
    if (unit.side ~= wesnoth.current.side) then moves = "max" end

    -- Need to find reachable hexes that are
    -- 1. next to a (non-petrified) enemy unit
    -- 2. not occupied by a unit of a different side (incl. allies)
    W.store_reachable_locations {
        { "filter", { x = unit.x, y = unit.y } },
        { "filter_location", {
            { "filter_adjacent_location", {
                { "filter", {
                    { "filter_side",
                        { { "enemy_of", { side = unit.side } } }
                    },
                    { "not", {
                        { "filter_wml", {
                            { "status", { petrified = "yes" } }  -- This is important!
                        } }
                    } }
                } }
            } },
            { "not", {
                { "filter", { { "not", { side = unit.side } } } }
            } }
        } },
        moves = moves,
        variable = "tmp_locs"
    }

    local attack_loc = H.get_variable_array("tmp_locs")
    W.clear_variable { name = "tmp_locs" }
    --print("reachable attack locs:", unit.id, #attack_loc)

    -- Variable to store attacks
    local attacks = {}
    -- Current position of unit
    local x1, y1 = unit.x, unit.y

    -- Go through all attack locations
    for i,p in pairs(attack_loc) do

        -- At this point, units on the side of 'unit' can still be at the attack hex.
        -- By default, exclude those hexes, but if 'include_occupied' is set
        -- units that can move away are fine

        -- Flag whether a potential unit_in_way can move away
        -- We also set this to true if there is no unit in the way
        local can_move_away = true

        local unit_in_way = wesnoth.get_unit(p.x, p.y)
        -- If unit_in_way is the unit itself, that doesn't count
        if unit_in_way and (unit_in_way.x == unit.x) and (unit_in_way.y == unit.y) then unit_in_way = nil end

        -- If there's a unit_in_way, and it is not the unit itself, check whether it can move away
        if unit_in_way then
            if (not cfg.include_occupied) then
                can_move_away = false
            else
                local move_away = ai_helper.get_reachable_unocc(unit_in_way, { moves = moves })
                if (move_away:size() <= 1) then can_move_away = false end
                --print('Can move away:', unit_in_way.id, can_move_away)
            end
        end
        -- Now can_move_away = true if there's no unit, or if it can move away

        if can_move_away then
            -- Put 'unit' at this position
            -- Remove any unit that might be there first, except if this is the unit itself
            if unit_in_way then wesnoth.extract_unit(unit_in_way) end

            wesnoth.put_unit(p.x, p.y, unit)
            --print(i,' attack pos:',p.x,p.y)

            -- As there might be several attackable units from a position, need to find all those
            local targets = wesnoth.get_units {
                { "filter_side",
                    { { "enemy_of", { side = unit.side } } }
                },
                { "not", {
                    { "filter_wml", {
                        { "status", { petrified = "yes" } }  -- This is important!
                    } }
                } },
                { "filter_location",
                    { { "filter_adjacent_location", { x = p.x, y = p.y } } }
                }
            }
            --print('  number targets: ',#targets)

            local attack_hex_occupied = false
            if unit_in_way then attack_hex_occupied = true end

            for j,t in pairs(targets) do
                local att_stats, def_stats = nil, nil
                if cfg.simulate_combat then
                    att_stats, def_stats = wesnoth.simulate_combat(unit, t)
                end

                table.insert(attacks, {
                    dst = { x = p.x, y = p.y },
                    src = { x = x1, y = y1 },
                    target = { x = t.x, y = t.y },
                    att_stats = att_stats,
                    def_stats = def_stats,
                    attack_hex_occupied = attack_hex_occupied
                } )
            end

            -- Put unit(s) back
            wesnoth.put_unit(x1, y1, unit)
            if unit_in_way then wesnoth.put_unit(p.x, p.y, unit_in_way) end
        end
    end

    return attacks
end

function ai_helper.get_attacks(units, cfg)
    -- Wrapper function for ai_helper.get_attacks_unit
    -- Returns the same sort of table (and cfg has the same structure), but for the attacks of several units

    local attacks = {}
    for k,u in pairs(units) do
        local attacks_unit = ai_helper.get_attacks_unit(u, cfg)

        if attacks_unit[1] then
            for i,a in ipairs(attacks_unit) do
                table.insert(attacks, a)
            end
        end
    end

    return attacks
end

function ai_helper.get_attack_map_unit(unit, cfg)
    -- Get all hexes that a unit can attack
    -- Return value is a location set, where the values are tables, containing
    --   - units: the number of units (always 1 for this function)
    --   - hitpoints: the combined hitpoints of the units
    --   - srcs: an array containing the positions of the units
    -- cfg: table with config parameters
    --  max_moves: if set use max_moves for units (this setting is always used for units on other sides)

    cfg = cfg or {}

    -- 'moves' can be either "current" or "max"
    -- For unit on current side: use "current" by default, or override by cfg.moves
    local max_moves = cfg.max_moves
    -- For unit on any other side, only max_moves=true makes sense
    if (unit.side ~= wesnoth.current.side) then max_moves = true end

    local old_moves = unit.moves
    if max_moves then unit.moves = unit.max_moves end

    local reach = {}
    reach.units = LS.create()
    reach.hitpoints = LS.create()

    local initial_reach = wesnoth.find_reach(unit, cfg)

    for i,loc in ipairs(initial_reach) do
        reach.units:insert(loc[1], loc[2], 1)
        reach.hitpoints:insert(loc[1], loc[2], unit.hitpoints)
        for x, y in H.adjacent_tiles(loc[1], loc[2]) do
            reach.units:insert(x, y, 1)
            reach.hitpoints:insert(x, y, unit.hitpoints)
        end
    end

    -- Reset unit moves
    if max_moves then unit.moves = old_moves end

    return reach
end

function ai_helper.get_attack_map(units, cfg)
    -- Get all hexes that units can attack (this is really just a wrapper function for ai_helper.get_attack_map_unit()
    -- Return value is a location set, where the values are tables, containing
    --   - units: the number of units (always 1 for this function)
    --   - hitpoints: the combined hitpoints of the units
    --   - srcs: an array containing the positions of the units
    -- cfg: table with config parameters
    --  max_moves: if set use max_moves for units (this setting is always used for units on other sides)

    local attack_map1 = {}
    attack_map1.units = LS.create()
    attack_map1.hitpoints = LS.create()

    for i,u in ipairs(units) do
        local attack_map2 = ai_helper.get_attack_map_unit(u, cfg)
        attack_map1.units:union_merge(attack_map2.units, function(x, y, v1, v2)
            return (v1 or 0) + v2
        end)
        attack_map1.hitpoints:union_merge(attack_map2.hitpoints, function(x, y, v1, v2)
            return (v1 or 0) + v2
        end)
    end

    return attack_map1
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

    for i,a in ipairs(attacks) do
        local dst = a.dst.y + a.dst.x * 1000.  -- attack hex (src)
        local src = a.src.y + a.src.x * 1000.  -- attacker hex (dst)
        if (not combos[1]) then  -- if this is the first recursion level, set up new combos for this level
            --print('New array')
            local move = {}
            move[dst] = src
            table.insert(combos_this_level, move)
        else
            -- Otherwise, we need to go through the already existing elements in 'combos'
            -- to see if either hex, or attacker is already used; and then add new attack to each
            for j,combo in ipairs(combos) do
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

function ai_helper.get_attack_combos_full(units, enemy)
    -- Calculate attack combination result by 'units' on 'enemy'
    -- All combinations of all units are taken into account, as well as their order
    -- This can result in a _very_ large number of possible combinations
    -- Use ai_helper.get_attack_combos() instead if order does not matter
    -- Return value:
    --   1. Attack combinations in form { dst = src }

    -- The combos are obtained by recursive call of ai_helper.add_next_attack_combo_level()

    local attacks = ai_helper.get_attacks(units)
    --print('# all attacks', #attacks)
    -- Eliminate those that are not on 'enemy'
    for i = #attacks,1,-1 do
        if (attacks[i].target.x ~= enemy.x) or (attacks[i].target.y ~= enemy.y) then
            table.remove(attacks, i)
        end
    end
    --print('# enemy attacks', #attacks)
    if (not attacks[1]) then return {} end

    -- This recursive function does all the work:
    local combos = ai_helper.add_next_attack_combo_level(combos, attacks)

    return combos
end

function ai_helper.get_attack_combos(units, enemy, cfg)
    -- Calculate attack combination result by 'units' on 'enemy'
    -- All the unit/hex combinations are considered, but without specifying the order of the
    -- attacks.  Use ai_helper.get_attack_combos_full() if order matters.
    -- cfg: A config table to be passed on to ai_helper.get_attacks
    -- Return values:
    --   1. Attack combinations in form { dst = src }
    --   2. All the attacks indexed by [dst][src]

    local attacks = ai_helper.get_attacks(units, cfg)
    --print('# all attacks', #attacks, os.clock())

    --Eliminate those that are not on 'enemy'
    for i = #attacks,1,-1 do
        if (attacks[i].target.x ~= enemy.x) or (attacks[i].target.y ~= enemy.y) then
            table.remove(attacks, i)
        end
    end
    --print('# enemy attacks', #attacks)

    if (not attacks[1]) then return {}, {} end

    -- Find all hexes adjacent to enemy that can be reached by any attacker
    -- Put this into an array that has dst as key,
    -- and array of all units (src) that can get there as value (in from x*1000+y)
    local attacks_dst_src = {}
    for i,a in ipairs(attacks) do
        local xy = a.dst.x * 1000 + a.dst.y
        if (not attacks_dst_src[xy]) then attacks_dst_src[xy] = { 0 } end  -- for attack by no unit on this hex
        table.insert(attacks_dst_src[xy], a.src.x * 1000 + a.src.y )
    end

    -- Now we set up an array of all attack combinations
    -- at this time, this includes all the 'no unit attacks this hex' elements
    -- which have a value of 0 for 'src'
    -- They need to be kept in this part, so that we get the combos that do not
    -- use the maximum amount of units possible.  They will be eliminated below.
    local attack_array = {}
    -- For all values of 'dst'
    for dst,ads in pairs(attacks_dst_src) do
        local org_array = ai_helper.table_copy(attack_array)
        attack_array = {}

        -- Go through all the values of 'src'
        for i,src in ipairs(ads) do
            -- If the array does not exist, set it up
            if (not org_array[1]) then
                local tmp = {}
                tmp[dst] = src
                table.insert(attack_array, tmp)
            else  -- otherwise, add the new dst-src pair to each element of the existing array
                for j,o in ipairs(org_array) do
                    -- but only do so if that 'src' value does not exist already
                    -- except for 0's those all need to be kept
                    local add_attack = true
                    for d,s in pairs(o) do
                        if (s == src) and (src ~=0) then
                            add_attack = false
                            break
                        end
                    end
                    -- Finally, add it to the array
                    if add_attack then
                        local tmp = ai_helper.table_copy(o)
                        tmp[dst] = src
                        table.insert(attack_array, tmp)
                    end
                end
            end
        end
    end
    --print('#attack_array before:', #attack_array)

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
    table.remove(attack_array, i_empty)
    --print('#attack_array after:', #attack_array)

    return attack_array
end

return ai_helper
