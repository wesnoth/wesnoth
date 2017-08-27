local params = ...

local function pre_show(window)
    window.players.value = params.nplayers
    window.width.value = params.map_width
    window.height.value = params.map_height
    window.village_density.value = params.village_density
    window.jagged.value = wml.get_nth_child(params, "chamber", 1).jagged
    window.lake_size.value = wml.get_nth_child(params, "chamber", 2).size

    local first_player = wml.get_nth_child(params, "chamber", 3)
    local windiness = wml.get_nth_child(first_player, "passage", 1).windiness
    window.windiness.value = windiness

    local roads = not wml.get_nth_child(first_player, "passage", 2).ignore
    window.roads.selected = roads

    -- Init labels
    window.players_label.label = window.players.value
    window.width_label.label = window.width.value
    window.height_label.label = window.height.value
    window.village_density_label.label = window.village_density.value
    window.jagged_label.label = window.jagged.value
    window.lake_size_label.label = window.lake_size.value
    window.windiness_label.label = window.windiness.value

    -- Callbacks...
    function window.players.on_modified()
        params.nplayers = window.players.value
        window.players_label.label = params.nplayers
    end

    function window.width.on_modified()
        params.map_width = window.width.value
        window.width_label.label = params.map_width
    end

    function window.height.on_modified()
        params.map_height = window.height.value
        window.height_label.label = params.map_height
    end

    function window.village_density.on_modified()
        params.village_density = window.village_density.value
        -- Need wesnoth-lib for the village density label
        local _ = wesnoth.textdomain "wesnoth-lib"
        window.village_density_label.label = (_"$villages/1000 tiles"):vformat{villages = params.village_density}
    end

    function window.jagged.on_modified()
        -- We're setting the value, so wml.get_child and co won't work
        -- However they do return the all-children index of the child
        local _, i = wml.get_nth_child(params, "chamber", 1)
        local val = window.jagged.value
        params[i].contents.jagged = val
        window.jagged_label.label = val
    end

    function window.lake_size.on_modified()
        -- We're setting the value, so wml.get_child and co won't work
        -- However they do return the all-children index of the child
        local _, i = wml.get_nth_child(params, "chamber", 2)
        local val = window.lake_size.value
        params[i].contents.size = val
        window.lake_size_label.label = val
    end

    function window.windiness.on_modified()
        -- We're setting the value, so wml.get_child and co won't work
        -- However they do return the all-children index of the child
        local _, i = wml.get_nth_child(params, "chamber", 3)
        local val = window.windiness.value
        for j = i, #params do
            if params[i].tag == "chamber" then
                local _, k = wml.get_nth_child(params[i].contents, "passage", 1)
                params[i].contents[k].contents.windiness = val
            end
        end
        window.windiness_label.label = val
    end

    function window.roads.on_modified()
        -- We're setting the value, so wml.get_child and co won't work
        -- However they do return the all-children index of the child
        local _, i = wml.get_nth_child(params, "chamber", 3)
        local val = not window.roads.selected
        for j = i, #params do
            if params[i].tag == "chamber" then
                local _, k = wml.get_nth_child(params[i].contents, "passage", 2)
                params[i].contents[k].contents.ignore = val
            end
        end
    end
end

local dialog = wml.load "multiplayer/gui/cave_map_settings.cfg"
gui.show_dialog(wml.get_child(dialog, 'resolution'), pre_show)
return params
