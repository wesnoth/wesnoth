local utils = wesnoth.require "wml-utils"
local location_groups = {}

--[[ usage
    [elevation_score_map]
        high_border = <terrain code> (default *^Qhh*)
        low_border = <terrain code> (default *^Qhu*)
    [/elevation_score_map]

]]

--[[ There are three cycles:
 - First Cycle that grows out from the marker terrains, placing each hex in an elevation group, stopping when it gets to a border or runs out of available hexes.
   Most likely it gets itself stuck in a corner, rather than really running out of hexes to assign, so restart from some random hex somewhere in that blob

 - Second Cycle iterates over all remaining hexes and existing elevation groups, filling in gaps.
   It works, but is not very efficient and can cause wesnoth to choke, so we need to make First Cycle do as much as possible 

 - (To Do) Third Cycle to clean up border disputes ('high' reached a high border before 'high-high').  Maybe this can be folded into first and second cycles?
 ]]

wesnoth.wml_actions.elevation_score_map = function(cfg)

        local available_locations = {}
        local high_border = cfg.high_border or "*^Qhh*"
        local low_border = cfg.low_border or "*^Qhu*"
        local max_x = 0
        local max_y = 0
        for u, v, terrain_code in wesnoth.current.map:iter() do
                if max_x < u then
                    max_x = u
                end
                if max_y < u then
                    max_y = v
                end
                if string.find(terrain_code, "_mll") then
                        table.insert(location_groups, {x=u, y=v, elevation='low-low', hexes= {{x=u, y=v}}})
                elseif string.find(terrain_code, "_ml") then
                        table.insert(location_groups, {x=u, y=v, elevation='low', hexes= {{x=u, y=v}}})
                elseif string.find(terrain_code, "_mhh") then
                        table.insert(location_groups, {x=u, y=v, elevation='high-high', hexes= {{x=u, y=v}}})
                elseif string.find(terrain_code, "_mh") then
                        table.insert(location_groups, {x=u, y=v, elevation='high', hexes= {{x=u, y=v}}})
                else
                        table.insert(available_locations, {x=u, y=v})
                end
        end
        local max_map_iter = max_x * max_y
        if #location_groups == 0 then
                wml.error "No elevation markers found on map"
                return
        end

----------------------------------------------------------------------------------------------------------------------------------------------------
-- First cycle
        -- start with the marked hex
        for j in ipairs(location_groups) do
                local anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                local change_count = 0
                local loop_i = 0
                while anchor_hex.x do
                    --if wesnoth.map.matches(anchor_hex.x, anchor_hex.y, {terrain = low_border}) or wesnoth.map.matches(anchor_hex.x, anchor_hex.y, {terrain = high_border}) then
                    --    wesnoth.message("ESM Debugging", string.format("Somehow border became anchor at (%d,%d)", anchor_hex.x, anchor_hex.y))
                    --    anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                    --end
                    loop_i = loop_i + 1
                    local al_found = 'no'
                    -- If we've been here before, reshuffle the new_hexes table so the anchor hex isn't always the NW
                    local n, ne, se, s, sw, nw = wesnoth.map.get_adjacent_hexes(anchor_hex.x,anchor_hex.y)
                    local new_hexes = {n, ne, se, s, sw, nw}
                    mathx.shuffle(new_hexes)

                    for k in ipairs(new_hexes) do -- check each of the six adjacent hexes
                        al_found = 'no'
                        local m = 0
                        for mm in ipairs(available_locations) do
                            -- check that new hex is an available location
                            -- wesnoth.message("ESM Debugging", string.format("available.x = %d and new_hexes[%d].x = %d", available_locations[m].x, k, new_hexes[k].x))
                            if (new_hexes[k][1] == available_locations[mm].x) then
                               if (new_hexes[k][2] == available_locations[mm].y) then 
                                    al_found = 'yes'
                                    m = mm
                                    break -- escape the for mm
                               end
                            end
                        end
                        if al_found == 'yes' then
                                -- if terrain is a border type, but there was no location_groups for it, it simply isn't assigned (stays available) for now
                                if wesnoth.map.matches(new_hexes[k][1], new_hexes[k][2], {terrain = high_border}) then
                                        --wesnoth.message("ESM Debugging", string.format("High border found at (%d,%d)", new_hexes[k].x, new_hexes[k].y))
                                        if location_groups[j].elevation == 'high-high' or location_groups[j].elevation == 'high' then
                                                table.insert(location_groups[j].hexes, {x= new_hexes[k].x, y= new_hexes[k].y})
                                                table.remove(available_locations, m)
                                        end
                                elseif wesnoth.map.matches(new_hexes[k][1], new_hexes[k][2], {terrain = low_border}) then
                                        if location_groups[j].elevation == 'low-low' or location_groups[j].elevation == 'low' then
                                                table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                                table.remove(available_locations, m)
                                        end
                                else 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        anchor_hex.x, anchor_hex.y= new_hexes[k][1], new_hexes[k][2] 
                                        table.remove(available_locations, m)
                                        -- break -- escape the for k (so we don't complete the ring around the anchor hex)
                                end
                        end
                    end
                    -- run out of available hexes, probably because we got stuck in a NW corner.  
                    -- Go back to some random hex in the location_group (but not a border terrain or map-border hex), try again
                    if al_found == 'no' then
                        local on_border = true
                        while on_border do
                            local random_index = mathx.random(#location_groups[j].hexes)
                            anchor_hex={x=location_groups[j].hexes[random_index].x, y=location_groups[j].hexes[random_index].y}
                            if wesnoth.map.matches(anchor_hex.x, anchor_hex.y, {terrain = high_border}) or wesnoth.map.matches(anchor_hex.x, anchor_hex.y, {terrain = low_border}) then
                            else
                                if wesnoth.current.map:on_border(anchor_hex.x, anchor_hex.y) then
                                else
                                    on_border = false
                                end
                            end
                        end
                        change_count = change_count + 1
                    end                
                        
                    -- wesnoth.message("ESM Debugging", string.format("Loop number %d, attempt number %d, anchor hex (%d,%d)", loop_i, change_count, anchor_hex.x, anchor_hex.y))
                    if change_count >= max_map_iter then anchor_hex = {} end -- how many failures to find anything should be enough?  Depends on the map size, though not sure this is the best method
                    if loop_i > 2500 then anchor_hex = {} end -- just in case
                end
        end

----------------------------------------------------------------------------------------------------------------------------------------------------
-- Second cycle
        -- this is not very efficient, but ideally it isn't doing much of the assigning...
        -- check each remaining available_location to see if (*)
        local al = #available_locations
        local al_old = #available_locations + 1
        -- wesnoth.message("ESM Debugging", string.format("Entering second loop, max_map_iter = %d", max_map_iter))
        local loop_i = 0 -- debugging aid, remove before merging
        local count = 0
        local count2 = 0
        while al < al_old and loop_i < 900 do 
                al_old = al
                loop_i = loop_i + 1
                local al_found = 'no'
                for j in ipairs(available_locations) do -- we shouldn't be modifying the table while we iterate through it, so this should be OK
                     -- check if available location is surounded by other available locations, so we don't waste time checking all the location_groups.hexes against them
                     local n, ne, se, s, sw, nw = wesnoth.map.get_adjacent_hexes(available_locations[j].x,available_locations[j].y)
                     local adj_set = {n, ne, se, s, sw, nw}
                     local skip = 0
                     for k in ipairs(adj_set) do
                         for kk in ipairs(available_locations) do
                             if adj_set[k][1] == available_locations[kk].x then
                                 if adj_set[k][2] == available_locations[kk].y then
                                     skip = skip + 1
                                     break -- escape for kk (found it, no need to keep looking)
                                 end
                             end
                         end
                     end
                     if skip < 6 then
                         skip = 0
                         for jj in ipairs(location_groups) do
                                 for jjj in ipairs(location_groups[jj].hexes) do
                                     count = count + 1
                                     -- (*) it has an adjacent hex that is not a border terrain ...
                                     if not wesnoth.map.matches(location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y, {terrain = high_border}) then
                                     if not wesnoth.map.matches(location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y, {terrain = low_border}) then
                                         -- ... and is already in a location group ...
                                         if wesnoth.map.are_hexes_adjacent({available_locations[j].x,available_locations[j].y}, {location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y}) then
                                                         -- then check if the available_location isn't the wrong type of border
                                                         if wesnoth.map.matches(available_locations[j].x,available_locations[j].y, {terrain = high_border}) and (location_groups[jj].elevation == 'low' or location_groups[jj].elevation == 'low-low') then
                                                         elseif wesnoth.map.matches(available_locations[j].x,available_locations[j].y, {terrain = low_border}) and (location_groups[jj].elevation == 'high' or location_groups[jj].elevation == 'high-high') then
                                                         else
                                                                 table.insert(location_groups[jj].hexes, {x = available_locations[j].x, y = available_locations[j].y})
                                                                 table.remove(available_locations, j)
                                                                 al_found = 'yes'
                                                         end
                                         end
                                     end
                                     end
                                     if al_found == 'yes' then break end
                                 end
                                 if al_found == 'yes' then break end
                         end
                         if al_found == 'yes' then break end
                     end
                end
                if al_found == 'no' then -- iterated through all available locations, none could be assigned to a group
                -- wesnoth.message("ESM Debugging", string.format("Went through (%d loops and %d times) the available locations, %d couldn't be assigned", loop_i, count, #available_locations))
                end
                al = #available_locations
        end
----------------------------------------------------------------------------------------------------------------------------------------------------
-- Third Cycle (To Do)
-- clean up the off-by-one borders
--[[        for i in ipairs(location_groups) do
            for j in ipairs(location_groups[i].hexes) do
                if (wesnoth.map.matches(location_groups[i].hexes[j].x,location_groups[i].hexes[j].y, {terrain = high_border}) then
                end
            end
        end
]]



------------------------------------------------------------------------------------
-- Implementation of data collected
        -- for now, write each location_groups to labels
        for i in ipairs(location_groups) do
            for j in ipairs(location_groups[i].hexes) do
                wesnoth.map.add_label{x = location_groups[i].hexes[j].x, y = location_groups[i].hexes[j].y, text = location_groups[i].elevation}
            end
        end
        for i in ipairs(available_locations) do
            wesnoth.map.add_label{x = available_locations[i].x, y = available_locations[i].y, text = "default"}
        end
end
