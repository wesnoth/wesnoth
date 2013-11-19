local H = wesnoth.require "lua/helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local unload = {}

-- Unload units from transports preferentially onto land tiles.  Second priority are
-- water hexes next to land tiles.  Units are unloaded with 2 MP.
-- Unload location is around the hex given by WML variables x1, y1
-- Parameters:
--  side: the side of the unloaded units
--  unit_types: array of unit types.  Units are created in the order in which types
--    are listed in this table

function unload:unload(side, unit_types)
    -- Do not unload units onto hexes occupied by units of any side
    local units = wesnoth.get_units {}
    local unit_map = LS.create()
    for i,u in ipairs(units) do unit_map:insert(u.x, u.y) end

    local x1, y1 = wesnoth.get_variable('x1'), wesnoth.get_variable('y1')

    local adj_tiles = {}
    for xa,ya in H.adjacent_tiles(x1, y1) do
        if (not unit_map:get(xa, ya)) then
            if wesnoth.match_location(xa, ya, { terrain = "!, W*" }) then
                table.insert(adj_tiles, { xa, ya, 1. } )
            elseif wesnoth.match_location(xa, ya,
                {
                    terrain = "W*",
                    { "filter_adjacent_location", { terrain = "!, W*" } }
                }
            )
            then
                table.insert(adj_tiles, { xa, ya, 0.1 } )
            else
                table.insert(adj_tiles, { xa, ya, 0.0 } )
            end
        end
    end

    table.sort(adj_tiles, function(a, b) return a[3] > b[3] end)

    for i,utype in ipairs(unit_types) do
        if adj_tiles[i] then
            wesnoth.put_unit(adj_tiles[i][1], adj_tiles[i][2], { side = side, type = utype, moves = 2 })
        else  -- just in case there are more units to create than available adjacent tiles
            local xv, yv = wesnoth.find_vacant_tile(x1, y1)
            wesnoth.put_unit(xv, yv, { side = side, type = utype })
        end
    end
end

return unload
