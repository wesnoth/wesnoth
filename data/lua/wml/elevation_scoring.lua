local utils = wesnoth.require "wml-utils"
local location_groups = {}

--[[ usage
    [elevation_score_map]
        high_border = <terrain code> (default *^Qhh*)
        low_border = <terrain code> (default *^Qhu*)
    [/elevation_score_map]

]]

--[[ There are two cycles:
 - First Cycle that grows out from the marker terrains, placing each hex in an elevation group, stopping when it gets to a border or runs out of available hexes.
   Most likely it gets itself stuck in a corner, rather than really running out of hexes to assign, so restart from some random candidate (non-repeating) hex 
   somewhere in that blob.

 - Second Cycle iterates over all remaining available hexes and checks if they are a border terrain type, and if next to a high-high/low-low border or not, 
   to assign to one of the four non-default elevations.  If not a border terrain type, they stay default

 ]]

wesnoth.wml_actions.elevation_score_map = function(cfg)

        local available_locations = {} -- unassigned hexes
        local border_locations = {} -- assigned hexes that will need review afterwards
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
        if #location_groups < 1 then
                wml.error "No elevation markers found on map"
                return
        end

----------------------------------------------------------------------------------------------------------------------------------------------------
-- First cycle
        -- start with the marked hex
        for j in ipairs(location_groups) do
            if (location_groups[j].elevation ~= 'dummy-high' and location_groups[j].elevation ~= 'dummy-low') then
                local anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                local change_count = 0
                local candidate_locations = {}
                while anchor_hex.x do
                    --if wesnoth.map.matches(anchor_hex.x, anchor_hex.y, {terrain = low_border}) or wesnoth.map.matches(anchor_hex.x, anchor_hex.y, {terrain = high_border}) then
                    --    wesnoth.message("ESM Debugging", string.format("Somehow border became anchor at (%d,%d)", anchor_hex.x, anchor_hex.y))
                    --    anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                    --end
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
                        if al_found == 'no' then
                            for mm in ipairs(border_locations) do
                            -- none of new hexes are available, but is one a known border of some sort?
                                if (new_hexes[k][1] == border_locations[mm].x) then
                                   if (new_hexes[k][2] == border_locations[mm].y) then 
                                        al_found = 'maybe'
                                        m = mm
                                        break -- escape the for mm
                                   end
                                end
                            end
                        end
                        if al_found == 'yes' then
                                -- Whenever a non-border-type location is transfered out of available_locations into a location_group, 
                                -- it is copied to candidate_locations as well, so we can go back to it later if needed.
                                -- If terrain is a border type, but this loop is the wrong location_groups for it, we assign it to a dummy group, and clean up later
                                if wesnoth.map.matches(new_hexes[k][1], new_hexes[k][2], {terrain = high_border}) then
                                        if location_groups[j].elevation == 'high-high' or location_groups[j].elevation == 'high' then
                                                table.insert(border_locations, {x= new_hexes[k][1], y= new_hexes[k][2], elevation= location_groups[j].elevation})
                                                table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        else
                                                table.insert(border_locations, {x= new_hexes[k][1], y= new_hexes[k][2], elevation= 'dummy-high'})
                                        end
                                elseif wesnoth.map.matches(new_hexes[k][1], new_hexes[k][2], {terrain = low_border}) then
                                        if location_groups[j].elevation == 'low-low' or location_groups[j].elevation == 'low' then
                                                table.insert(border_locations, {x= new_hexes[k][1], y= new_hexes[k][2], elevation= location_groups[j].elevation})
                                                table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        else
                                                table.insert(border_locations, {x= new_hexes[k][1], y= new_hexes[k][2], elevation= 'dummy-low'})
                                        end
                                else 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.insert(candidate_locations, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        --table.insert(border_locations, {x= new_hexes[k][1], y= new_hexes[k][2], elevation= location_groups[j].elevation})
                                        anchor_hex.x, anchor_hex.y= new_hexes[k][1], new_hexes[k][2] 
                                        -- break -- escape the for k (so we don't complete the ring around the anchor hex)
                                end
                                table.remove(available_locations, m)
                        elseif al_found == 'maybe' then
                        -- this new hex is a border between two regions, and is already part of some group, 
                        -- check that it is the correct type (not just whatever growing blob reached it first, or a dummy group)
                                if border_locations[m].elevation == 'low' then
                                   if location_groups[j].elevation == 'low-low' then 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.remove(border_locations, m)                                        
                                   end
                                elseif border_locations[m].elevation == 'high' then
                                   if location_groups[j].elevation == 'high-high' then 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.remove(border_locations, m)                                        
                                   end
                                elseif border_locations[m].elevation == 'dummy-high' then
                                   if location_groups[j].elevation == 'high-high' then 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.remove(border_locations, m)                                        
                                   elseif location_groups[j].elevation == 'high' then 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.remove(border_locations, m)                                        
                                   end
                                elseif border_locations[m].elevation == 'dummy-low' then
                                   if location_groups[j].elevation == 'low-low' then 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.remove(border_locations, m)                                        
                                   elseif location_groups[j].elevation == 'low' then 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k][1], y= new_hexes[k][2]})
                                        table.remove(border_locations, m)                                        
                                   end
                                else
                                    -- Nothing to do for high-high/low-low
                                end
--                                table.remove(border_locations, m)                                        
                                al_found = 'no'
                        else
                        end
                    end
                    if #candidate_locations < 1 then 
                        anchor_hex = {} 
                        al_found = 'blah'
                        break
                    end
                    -- run out of available hexes, probably because we got stuck in a NW corner.  
                    -- Go back to some random hex in the candidate_locations group (but not a map-border hex), try again
                    if al_found == 'no' then
                        local on_border = true
                        while on_border do
                            local random_index = mathx.random(#candidate_locations)
                            if wesnoth.current.map:on_border(anchor_hex.x, anchor_hex.y) then
                                    --table.remove(candidate_locations, random_index)
                            else
                                    on_border = false
                                    anchor_hex.x, anchor_hex.y = candidate_locations[random_index].x, candidate_locations[random_index].y
                                    table.remove(candidate_locations, random_index)
                            end
                        end
                        change_count = change_count + 1
                    end                
                        
                    -- failsafe
                    if change_count >= max_map_iter then anchor_hex = {} end -- How many failures to find anything should be enough?  Depends on the map size, though not sure this is the best method
                end
            end
        end

----------------------------------------------------------------------------------------------------------------------------------------------------
-- Second cycle
-- iterate over all the remaining available_locations
     -- This isn't very efficient, and can assign the hex to the wrong blob (but still correct elevation), 
     -- but it would be even less efficient if we looked for the closest marker
        if #available_locations > 0 then
           for i in ipairs(available_locations) do
               if wesnoth.map.matches(available_locations[i].x, available_locations[i].y, {terrain = high_border}) then
                   if #border_locations > 0 then
                       for j in ipairs(border_locations) do
                           if wesnoth.map.are_hexes_adjacent({available_locations[i].x,available_locations[i].y}, {border_locations[j].x, border_locations[j].y}) then
                               if border_locations[j].elevation == 'high-high' then
                                    for m in ipairs (location_groups) do
                                        if location_groups[m].elevation == 'high-high' then
                                            table.insert(location_groups[m].hexes, {x= available_locations[i].x, y= available_locations[i].y})
                                            available_locations[i].x, available_locations[i].y = 0, 0 -- rather than than table.remove, so we don't screw with the indexes
                                        end
                                    end
                               end
                           end
                       end
                   end
                   -- if it isn't adjacent to a high-high border, it is just high
                   if available_locations[i].x ~= 0 then
                       for m in ipairs (location_groups) do
                           if location_groups[m].elevation == 'high' then
                              table.insert(location_groups[m].hexes, {x= available_locations[i].x, y= available_locations[i].y})
                              available_locations[i].x, available_locations[i].y = 0, 0 
                           end
                       end
                   end
               elseif wesnoth.map.matches(available_locations[i].x, available_locations[i].y, {terrain = low_border}) then
                   if #border_locations > 0 then
                       for j in ipairs(border_locations) do
                           if wesnoth.map.are_hexes_adjacent({available_locations[i].x,available_locations[i].y}, {border_locations[j].x, border_locations[j].y}) then
                               if border_locations[j].elevation == 'low-low' then
                                    for m in ipairs (location_groups) do
                                        if location_groups[m].elevation == 'low-low' then
                                            table.insert(location_groups[m].hexes, {x= available_locations[i].x, y= available_locations[i].y})
                                            available_locations[i].x, available_locations[i].y = 0, 0 
                                        end
                                    end
                               end
                           end
                       end
                   end
                   -- if it isn't adjacent to a low-low border, it is just low
                   if available_locations[i].x ~= 0 then
                       for m in ipairs (location_groups) do
                           if location_groups[m].elevation == 'low' then
                              table.insert(location_groups[m].hexes, {x= available_locations[i].x, y= available_locations[i].y})
                              available_locations[i].x, available_locations[i].y = 0, 0 
                           end
                       end
                   end
               end
           end
       end
       
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
