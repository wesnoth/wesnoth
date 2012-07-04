local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local LS = wesnoth.require "lua/location_set.lua"

local ai_helper = {}

function ai_helper.get_attacks_unit(unit, moves)
    -- Get all attacks a unit can do
    -- moves: if set, use this for 'moves' key, otherwise use "current"
    -- Returns {} if no attacks can be done, otherwise table with fields
    --   x, y: attack position
    --   attacker_id: id of attacking unit
    --   defender_id: id of defending unit
    --   att_stats, def_stats: as returned by wesnoth.simulate_combat
    -- This is somewhat slow, but will hopefully replaced soon by built-in AI function

    -- Need to find reachable hexes that are
    -- 1. next to an enemy unit
    -- 2. not occupied by an allied unit (except for unit itself)
    W.store_reachable_locations {
        { "filter", { id = unit.id, side = wesnoth.current.side } },
        { "filter_location", {
            { "filter_adjacent_location", {
                { "filter",
                    { { "filter_side",
                        { { "enemy_of", { side = unit.side } } }
                    } }
                }
            } },
            { "not", {
                { "filter", { { "not", { id = unit.id, side = wesnoth.current.side } } } }
            } }
        } },
        moves = moves or "current",
        variable = "tmp_locs"
    }
    local attack_loc = H.get_variable_array("tmp_locs")
    W.clear_variable { name = "tmp_locs" }

    -- Variable to store attacks
    local attacks = {}
    -- Current position of unit
    local x1, y1 = unit.x, unit.y

    -- Go through all attack locations
    for i,p in pairs(attack_loc) do

        -- Put unit at this position
        wesnoth.put_unit(p.x, p.y, unit)

        -- As there might be several attackable units from a position, need to find all those
        local targets = wesnoth.get_units {
            { "filter_side",
                { { "enemy_of", { side = unit.side } } }
            },
            { "filter_location",
                { { "filter_adjacent_location", { x = p.x, y = p.y } } }
            }
        }

        for j,t in pairs(targets) do
            local att_stats, def_stats = wesnoth.simulate_combat(unit, t)

            table.insert(attacks, {
                x = p.x, y = p.y,
                attacker_id = unit.id,
                defender_id = t.id,
                att_stats = att_stats,
                def_stats = def_stats
            } )
        end
    end

    -- Put unit back to its location
    wesnoth.put_unit(x1, y1, unit)

    return attacks
end

function ai_helper.get_attacks(units, moves)
    -- Wrapper function for ai_helper.get_attacks_unit
    -- Returns the same sort of table, but for the attacks of several units
    -- This is somewhat slow, but will hopefully replaced soon by built-in AI function

    local attacks = {}
    for k,u in pairs(units) do
        local attacks_unit = ai_helper.get_attacks_unit(u, moves)

        if attacks_unit[1] then
            for i,a in ipairs(attacks_unit) do
                table.insert(attacks, a)
            end
        end
    end

    return attacks
end

function ai_helper.next_hop(unit, x, y, cfg)
    -- Finds the next "hop" of 'unit' on its way to (x,y)
    -- Returns coordinates of the endpoint of the hop, and movement cost to get there
    -- only unoccupied hexes are considered
    -- cfg: extra options for wesnoth.find_path()
    local path, cost = wesnoth.find_path(unit, x, y, cfg)

    -- If unit cannot get there:
    if cost >= 42424242 then return nil, cost end

    -- If none of the hexes is unoccupied, use current position as default
    local next_hop, nh_cost = {unit.x, unit.y}, 0

    -- Go through loop to find reachable, unoccupied hex along the path
    for index, path_loc in ipairs(path) do
        local sub_path, sub_cost = wesnoth.find_path( unit, path_loc[1], path_loc[2], cfg)

        if sub_cost <= unit.moves then
            local unit_in_way = wesnoth.get_units{ x = path_loc[1], y = path_loc[2] }[1]
            if not unit_in_way then
                next_hop, nh_cost = path_loc, sub_cost
            end
        else
            break
        end
    end

    return next_hop, nh_cost
end

function ai_helper.movefull_stopunit(ai, unit, x, y)
    -- Does ai.move_full for a unit if not at (x,y), otherwise ai.stopunit_moves
    -- Coordinates can be given as x and y components, or as a 2-element table { x, y }
    if (type(x) ~= 'number') then x, y = x[1], x[2] end

    if (x ~= unit.x) or (y ~= unit.y) then
        ai.move_full(unit, x, y)
    else
        ai.stopunit_moves(unit)
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

    return DM
end

return ai_helper
