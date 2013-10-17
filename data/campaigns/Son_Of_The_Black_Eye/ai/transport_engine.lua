return {
    init = function(ai)

        local engine = {}

        local H = wesnoth.require "lua/helper.lua"
        local LS = wesnoth.require "lua/location_set.lua"

        -- Move transport ships according to these rules:
        -- 1. If transport can get to its designated landing site this move, find
        --    close hex with the most unoccupied adjacent non-water hexes and move there
        -- 2. If landing site is out of reach, move toward destination while
        --    staying in deep water surrounded by deep water only

        function engine:transport_eval()
            local units = wesnoth.get_units {
                side = wesnoth.current.side,
                formula = '$this_unit.moves > 0',
            }

            for i,u in ipairs(units) do
                local vars = H.get_child(u.__cfg, "variables")
                if vars.destination_x and vars.destination_y then
                    return 300000
                end
            end

            return 0
        end

        function engine:transport_exec(type)
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
                            -- This is mostly to avoid it being across the bay in S6
                            local adj_tiles = {}
                            if (rating >= -0.05) then
                                for x,y in H.adjacent_tiles(r[1], r[2]) do
                                    if (not unit_map:get(x, y)) then
                                        if wesnoth.match_location(x, y, { terrain = "!, W*" }) then
                                            rating = rating + 1
                                            table.insert(adj_tiles, { x, y, 1. } )
                                        end

                                        if wesnoth.match_location(x, y,
                                            {
                                                terrain = "W*",
                                                { "filter_adjacent_location", { terrain = "!, W*" } }
                                            }
                                        )
                                        then
                                            rating = rating + 0.1
                                            table.insert(adj_tiles, { x, y, 0.1 } )
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
                -- Also find the best unloading hexes
                -- TODO: this is currently not used as the AI functionality to make
                -- it replay-safe does not exist; to be added later
                --table.sort(best_adj_tiles, function(a, b) return a[3] > b[3] end)
                --for i,tile in ipairs(best_adj_tiles) do
                --    best_unit.variables['hex' .. i .. '_x'] = tile[1]
                --    best_unit.variables['hex' .. i .. '_y'] = tile[2]
                --end
                -- For remaining hexes, simply use the center hex and let the engine decide itself
                -- This also provides a safeguard against too many hexes being occupied
                --for i = #best_adj_tiles + 1, 6 do
                --    best_unit.variables['hex' .. i .. '_x'] = best_hex[1]
                --    best_unit.variables['hex' .. i .. '_y'] = best_hex[2]
                --end

                ai.move_full(best_unit, best_hex[1], best_hex[2])
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

        return engine
    end
}
