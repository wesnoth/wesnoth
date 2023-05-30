local utils = wesnoth.require "wml-utils"
local location_set = wesnoth.require "location_set"
local location_groups = {}

--[[ usage
    [elevation_score_map]
        high_border = <terrain code> (default *^Qhh*)
        low_border = <terrain code> (default *^Qhu*)
    [/elevation_score_map]

]]

--[[ There are two cycles:
 - First Cycle that grows out from the marker terrains, placing each hex in an elevation group, stopping when it gets to a border or runs out of available hexes - or more likely gets itself stuck in a corner
 - Second Cycle iterates over all remaining hexes and existing elevation groups, filling in gaps.  It works, but is not very efficient and can cause wesnoth to choke, so we need to make First Cycle do as much as possible 
 - (To Do) Third Cycle to clean up border disputes ('high' reached a high border before 'high-high').  Maybe this can be folded into first and second cycles?
 ]]

wesnoth.wml_actions.elevation_score_map = function(cfg)

        local available_locations = {}
        local high_border = cfg.high_border or "*^Qhh*"
        local low_border = cfg.low_border or "*^Qhu*"
        for u, v, terrain_code in wesnoth.current.map:iter() do
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

        if #location_groups == 0 then
                wml.error "No elevation markers found on map"
                return
        end

        -- start with the marked hex
        for j in ipairs(location_groups) do
                local anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                local loop_i = 0
                while anchor_hex.x do
                    wesnoth.message("ESM Debugging", string.format("Anchor at (%d,%d)", anchor_hex.x, anchor_hex.y))
                    loop_i = loop_i + 1
                    local new_hexes = {}
                    local al_found = 'no'
                    -- this could be more compact, but whatever for now
                    -- table.insert(new_hexes, {wesnoth.map.get_adjacent_hexes(anchor_hex.x,anchor_hex.y)})
                    local ha, hb, hc, hd, he, hf = wesnoth.map.get_adjacent_hexes(anchor_hex.x,anchor_hex.y)
                    table.insert(new_hexes, {x=ha[1], y=ha[2]})
                    table.insert(new_hexes, {x=hb[1], y=hb[2]})
                    table.insert(new_hexes, {x=hc[1], y=hc[2]})
                    table.insert(new_hexes, {x=hd[1], y=hd[2]})
                    table.insert(new_hexes, {x=he[1], y=he[2]})
                    table.insert(new_hexes, {x=hf[1], y=hf[2]})
                    for k in ipairs(new_hexes) do -- check each of the six adjacent hexes
                        al_found = 'no'
                        local m = 1
                        while available_locations[m] do
                            -- check that new hex is an available location
                            -- wesnoth.message("ESM Debugging", string.format("available.x = %d and new_hexes[%d].x = %d", available_locations[m].x, k, new_hexes[k].x))
                            if (new_hexes[k].x == available_locations[m].x) and (new_hexes[k].y == available_locations[m].y) then 
                                    al_found = 'yes'
                                    break 
                            end
                            m = m +1
                        end
                        if al_found == 'yes' then
                        -- if wesnoth.current.map:on_board(new_hexes[k]) then
                                -- if terrain is a border type, but there was no location_groups for it, it simply isn't assigned (stays available) for now
                                if wesnoth.map.matches(new_hexes[k].x, new_hexes[k].y, {terrain = high_border}) then
                                        --wesnoth.message("ESM Debugging", string.format("High border found at (%d,%d)", new_hexes[k].x, new_hexes[k].y))
                                        if location_groups[j].elevation == 'high-high' or location_groups[j].elevation == 'high' then
                                                table.insert(location_groups[j].hexes, {x= new_hexes[k].x, y= new_hexes[k].y})
                                                table.remove(available_locations, m)
                                        end
                                elseif wesnoth.map.matches(new_hexes[k].x, new_hexes[k].y, {terrain = low_border}) then
                                        if location_groups[j].elevation == 'low-low' or location_groups[j].elevation == 'low' then
                                                table.insert(location_groups[j].hexes, {x= new_hexes[k].x, y= new_hexes[k].y})
                                                table.remove(available_locations, m)
                                        end
                                else 
                                        table.insert(location_groups[j].hexes, {x= new_hexes[k].x, y= new_hexes[k].y})
                                        anchor_hex.x, anchor_hex.y= new_hexes[k].x, new_hexes[k].y
                                        table.remove(available_locations, m)
                                end
                        end
                    end
                    -- run out of available hexes, probably because we got stuck in a NW corner.  Go back to original hex (marker), offset NE (if it is already part of the group), try again
                    local give_up = nil
                    -- this isn't working right now, need to figure this out next time...
                    if al_found == 'no' and not give_up then
                        anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                        anchor_hex.x = anchor_hex.x + 1
                        anchor_hex.y = anchor_hex.y - 1
                        for jj in ipairs(location_groups[j].hexes) do
                            if (location_groups[j].hexes[jj].x == anchor_hex.x) and (location_groups[j].hexes[jj].y == anchor_hex.y) then
                            -- NE hex wasn't part of the group, try SW
                            else
                                anchor_hex={x=location_groups[j].x, y=location_groups[j].y}
                                anchor_hex.x = anchor_hex.x - 1
                                anchor_hex.y = anchor_hex.y + 1
                                if (location_groups[j].hexes[jj].x == anchor_hex.x) and (location_groups[j].hexes[jj].y == anchor_hex.y) then
                                else
                                    give_up = 'yes'
                                end
                            end
                        end
                    end                
                        
                    if al_found == 'no' then anchor_hex = {} end 
                    if loop_i > 2500 then anchor_hex = {} end -- just in case
                end
        end

        -- this is not very efficient, but ideally it isn't doing most of the assigning, the first pass above should have filled in most things
        -- check each remaining available_location to see if ... 
        local loop_i = 0 -- debugging aid
        while #available_locations >= 1 and loop_i < 700 do
                loop_i = loop_i + 1
                local al_found = 'no'
                for j in ipairs(available_locations) do
                         --wesnoth.message("ESM Debugging", string.format("Entering second loop, %d pass, available_location (%d,%d)", loop_i, available_locations[j].x, available_locations[j].y))
                         for jj in ipairs(location_groups) do
                                 for jjj in ipairs(location_groups[jj].hexes) do
                                         -- ... it has an adjacent hex that is already in a location group ...
                                         if wesnoth.map.are_hexes_adjacent({available_locations[j].x,available_locations[j].y}, {location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y}) then
                                                 -- local check_hex=wesnoth.map.get({location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y})
                                                 -- ... and not a border terrain ...
                                                 if wesnoth.map.matches(location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y, {terrain = high_border}) or wesnoth.map.matches(location_groups[jj].hexes[jjj].x,location_groups[jj].hexes[jjj].y, {terrain = low_border}) then
                                                 else
                                                 -- if check_hex.terrain ~= high_border and check_hex.terrain ~= low_border then
                                                         --wesnoth.message("ESM Debugging", string.format("(%d,%d) is terrain %s, while high_border is %s and low_border is %s", check_hex.x, check_hex.y, check_hex.terrain, high_border, low_border))
                                                         -- then check if the available_location isn't the wrong type of border
                                                         --check_hex=wesnoth.map.get({available_locations[j].x,available_locations[j].y})
                                                         --if check_hex.terrain == high_border and location_groups[jj].elevation == 'low' or location_groups[jj].elevation == 'low-low' then
                                                         if wesnoth.map.matches(available_locations[j].x,available_locations[j].y, {terrain = high_border}) and (location_groups[jj].elevation == 'low' or location_groups[jj].elevation == 'low-low') then
                                                         --elseif check_hex.terrain == low_border and location_groups[jj].elevation == 'high' or location_groups[jj].elevation == 'high-high' then
                                                         elseif wesnoth.map.matches(available_locations[j].x,available_locations[j].y, {terrain = low_border}) and (location_groups[jj].elevation == 'high' or location_groups[jj].elevation == 'high-high') then
                                                         else
                                                                 --wesnoth.message("ESM Debugging", string.format("(%d,%d) assigned to %s in second loop", available_locations[j].x, available_locations[j].y, location_groups[jj].elevation))
                                                                 table.insert(location_groups[jj].hexes, {x = available_locations[j].x, y = available_locations[j].y})
                                                                 table.remove(available_locations, j)
                                                                 al_found = 'yes'
                                                         end
                                                 end
                                         end
                                         if al_found == 'yes' then break end
                                 end
                                 if al_found == 'yes' then break end
                         end
                         if al_found == 'yes' then break end
                end
                if al_found == 'no' then -- iterated through all available locations, none could be assigned to a group
                        table.insert(location_groups, {x=0, y=0, elevation='default', hexes= {}})
                        local m = #location_groups
                        for j in ipairs(available_locations) do
                            table.insert(location_groups[m].hexes,{available_locations[j].x,available_locations[j].y}) 
                            --local check_hex=wesnoth.map.get({available_locations[j].x,available_locations[j].y})
                            --if check_hex.terrain == high_border then
                            --elseif check_hex.terrain == low_border then
                            --        
                            --else
                            --end
                            table.remove(available_locations, j)
                        end
                end
        end
        -- for now, write each location_groups to labels
        for i in ipairs(location_groups) do
            for j in ipairs(location_groups[i].hexes) do
                wesnoth.map.add_label{x = location_groups[i].hexes[j].x, y = location_groups[i].hexes[j].y, text = location_groups[i].elevation}
            end
        end
end
