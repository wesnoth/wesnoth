local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_forest_animals_new_rabbit = {}

function ca_forest_animals_new_rabbit:evaluation(ai, cfg)
    -- Put new rabbits out the if there are fewer than cfg.rabbit_number
    -- but only if cfg.rabbit_type is set, otherwise do nothing
    -- If this gets executed, we'll let the CA black-list itself

    if (not cfg.rabbit_type) then return 0 end
    return cfg.ca_score
end

function ca_forest_animals_new_rabbit:execution(ai, cfg)
    local number = cfg.rabbit_number or 6
    local rabbit_enemy_distance = cfg.rabbit_enemy_distance or 3

    -- Get the locations of all items on that map (which could be rabbit holes)
    W.store_items { variable = 'holes_wml' }
    local all_items = H.get_variable_array('holes_wml')
    W.clear_variable { name = 'holes_wml' }

    -- Eliminate all holes that have an enemy within 'rabbit_enemy_distance' hexes
    -- We also add a random number to the ones we keep, for selection of the holes later
    local holes = {}
    for _, item in ipairs(all_items) do
        local enemies = wesnoth.get_units {
            { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
            { "filter_location", { x = item.x, y = item.y, radius = rabbit_enemy_distance } }
        }
        if (not enemies[1]) then
            -- If cfg.rabbit_hole_img is set, only items with that image or halo count as holes
            if cfg.rabbit_hole_img then
                if (item.image == cfg.rabbit_hole_img) or (item.halo == cfg.rabbit_hole_img) then
                    item.random = math.random(100)
                    table.insert(holes, item)
                end
            else
                item.random = math.random(100)
                table.insert(holes, item)
            end
        end
    end
    table.sort(holes, function(a, b) return a.random > b.random end)

    local rabbits = wesnoth.get_units { side = wesnoth.current.side, type = cfg.rabbit_type }
    --print('total number:', number)
    number = number - #rabbits
    --print('to add number:', number)
    number = math.min(number, #holes)
    --print('to add number possible:', number)

    -- Now we just can take the first 'number' (randomized) holes
    local tmp_unit = wesnoth.get_units { side = wesnoth.current.side }[1]
    for i = 1,number do
        local x, y = -1, -1
        if tmp_unit then
            x, y = wesnoth.find_vacant_tile(holes[i].x, holes[i].y, tmp_unit)
        else
            x, y = wesnoth.find_vacant_tile(holes[i].x, holes[i].y)
        end

        local command =  "wesnoth.put_unit(x1, y1, { side = "
            .. wesnoth.current.side
            .. ", type = '"
            ..  cfg.rabbit_type
            .. "' } )"
        ai.synced_command(command, x, y)
    end
end

return ca_forest_animals_new_rabbit
