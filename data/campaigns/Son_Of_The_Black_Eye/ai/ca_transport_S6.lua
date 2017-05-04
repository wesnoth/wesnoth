local H = wesnoth.require "helper"
local LS = wesnoth.require "location_set"

local ca_transport = {}

-- Move transport ships according to these rules:
-- 1. If transport can get to its designated landing site this move, find
--    close hex with the most unoccupied adjacent non-water hexes and move there
-- 2. If landing site is out of reach, move toward destination while
--    staying in deep water surrounded by deep water only
-- Also unload units onto best hexes adjacent to landing site

function ca_transport:evaluation()
    local units = wesnoth.get_units { side = wesnoth.current.side, formula = 'movement_left > 0' }

    for i,u in ipairs(units) do
        local vars = H.get_child(u.__cfg, "variables")
        if vars.destination_x and vars.destination_y then
            return 300000
        end
    end

    return 0
end

function ca_transport:execution()
    local units = wesnoth.get_units {}

    -- Need all transport units plus maps of all units, all transport units and
    -- all other units (as those block hexes accessible to transport units)
    local transports = {}
    local unit_map, transport_map, blocked_hex_map = LS.create(), LS.create(), LS.create()

    for i,u in ipairs(units) do
        unit_map:insert(u.x, u.y)

        if (u.side == wesnoth.current.side) and (u.moves > 0)
            and u.variables.destination_x and u.variables.destination_x
        then
            transport_map:insert(u.x, u.y)
            table.insert(transports, u)
            --print("----> Inserting " .. u.id, u.x, u.y, u.variables.destination_x, u.variables.destination_y)
        else
            blocked_hex_map:insert(u.x, u.y)
       end
    end

    -- First see if a transport is within landing distance
    local landing_site_map = LS.of_pairs(
        wesnoth.get_locations {
            terrain = 'W*',
            { "filter_adjacent_location", { terrain = '!, W*' } }
        }
    )

    local max_rating, best_unit, best_hex, best_adj_tiles = -9e99
    for i,u in ipairs(transports) do
        local dst = { u.variables.destination_x, u.variables.destination_y }

        if (not u.variables.landed) and (H.distance_between(u.x, u.y, dst[1], dst[2]) <= u.moves) then
            local reach = wesnoth.find_reach(u)

            for i,r in ipairs(reach) do
                if landing_site_map:get(r[1], r[2]) and (not unit_map:get(r[1], r[2]))
                then
                    -- Distance from destination is minor rating
                    local rating = -H.distance_between(r[1], r[2], dst[1], dst[2]) / 100.

                    -- Main rating is number of unoccupied land hexes and
                    -- water hexes next to land hexes
                    -- But shouldn't be too far away (arb. set to 5 hexes here)
                    -- This is mostly to avoid it being across the bay in SotBE S6
                    local adj_tiles = {}
                    if (rating >= -0.05) then
                        for x,y in H.adjacent_tiles(r[1], r[2]) do
                            if (not unit_map:get(x, y)) then
                                if wesnoth.match_location(x, y, { terrain = "!, W*" }) then
                                    rating = rating + 1
                                    table.insert(adj_tiles, { x, y, 1. } )
                                elseif wesnoth.match_location(x, y,
                                    {
                                        terrain = "W*",
                                        { "filter_adjacent_location", { terrain = "!, W*" } }
                                    }
                                )
                                then
                                    rating = rating + 0.1
                                    table.insert(adj_tiles, { x, y, 0.1 } )
                                else
                                    table.insert(adj_tiles, { x, y, 0.0 } )
                                end
                            end
                        end
                    end

                    if (rating > max_rating) then
                        max_rating = rating
                        best_unit = u
                        best_hex = r
                        best_adj_tiles = adj_tiles
                    end
                end
            end
        end
    end

    if (max_rating > -9e99) then
        ai.move_full(best_unit, best_hex[1], best_hex[2])

        -- Also unload units
        table.sort(best_adj_tiles, function(a, b) return a[3] > b[3] end)

        local command = "local unit = wesnoth.get_unit(x1, y1) "
            .. "unit.variables.landed = 'yes' "
            .. "unit.variables.destination_x = 1 "
            .. "unit.variables.destination_y = 30"
        ai.synced_command(command, best_unit.x, best_unit.y)

        -- Unload 1 level 2 unit
        local l2_type = H.rand('Swordsman,Javelineer,Pikeman')
        local command = "wesnoth.put_unit({ side = " .. wesnoth.current.side
            .. ", type = '" .. l2_type
            .. "', moves = 2 }, "
            .. best_adj_tiles[1][1] .. "," .. best_adj_tiles[1][2] .. ")"
        ai.synced_command(command, best_unit.x, best_unit.y)

        -- Unload up to 2 level 1 units
        for i = 2, math.min(#best_adj_tiles, 3) do
            local l1_type = H.rand('Fencer,Mage,Cavalryman,Bowman,Spearman')
            local command = "wesnoth.put_unit({ side = " .. wesnoth.current.side
                .. ", type = '" .. l1_type
                .. "', moves = 2 }, "
                .. best_adj_tiles[i][1] .. "," .. best_adj_tiles[i][2] .. ")"
            ai.synced_command(command, best_unit.x, best_unit.y)
        end

        return
    end

    -- If we got here, no landing site was found.  Do a deep-water move instead
    local deep_water_map = LS.of_pairs(
        wesnoth.get_locations {
            terrain = 'Wo',
            { "not", { { "filter_adjacent_location", { terrain = '!, Wo' } } } }
        }
    )

    local max_rating, best_unit, best_hex = -9e99, {}, {}
    for i,u in ipairs(transports) do
        local dst = { u.variables.destination_x, u.variables.destination_y }
        local reach = wesnoth.find_reach(u)

        local max_rating_unit, best_hex_unit = -9e99, {}
        for i,r in ipairs(reach) do
            if deep_water_map:get(r[1], r[2]) and (not blocked_hex_map:get(r[1], r[2])) then
                local rating = -H.distance_between(r[1], r[2], dst[1], dst[2])
                -- If possible, also move in a straight line
                rating = rating - math.abs(r[1] - dst[1]) / 100.
                rating = rating - math.abs(r[2] - dst[2]) / 100.
                if (rating > max_rating_unit) then
                    max_rating_unit = rating
                    best_hex_unit = r
                end
            end
        end

        -- We give a penalty to hexes occupied by another transport that can still move away.
        -- All ratings need to be set to the same value for this to work.
        if (max_rating_unit > -9e99) then
            max_rating_unit = 0
            if transport_map:get(best_hex_unit[1], best_hex_unit[2]) then
                max_rating_unit = -1
            end

            if (max_rating_unit > max_rating) then
                max_rating = max_rating_unit
                best_unit = u
                best_hex = best_hex_unit
            end
        end
    end

    if best_unit.id then
        ai.move_full(best_unit, best_hex[1], best_hex[2])
    else  -- still need to make sure gamestate gets changed
        ai.stopunit_moves(transports[1])
    end
end

return ca_transport
