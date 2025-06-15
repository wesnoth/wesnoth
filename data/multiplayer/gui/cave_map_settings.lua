local params = ...
local MG = wesnoth.require "mapgen_helper"

local function pre_show(window)
    window.players.value = params.nplayers
    window.width.value = params.map_width
    window.height.value = params.map_height
    window.village_density.value = params.village_density

    local central_chamber = MG.get_chamber(params, 'central_chamber')
    if central_chamber then
        window.jagged.value = central_chamber.jagged
    else
        error('cave_map_settings requires a [chamber] with id=central_chamber')
    end

    local lake = MG.get_chamber(params, 'lake')
    if lake then
        window.lake_size.value = lake.size
    else
        error('cave_map_settings requires a [chamber] with id=lake')
    end

    local first_player = MG.get_chamber(params, 'player_1')
    if first_player then
        local tunnel = MG.get_passage(first_player, 1)
        if tunnel then
            window.windiness.value = tunnel.windiness
        else
            error('cave_map_settings requires that each player [chamber] contains at least two [passage] tags')
        end

        local road = MG.get_passage(first_player, 2)
        if road then
            window.roads.selected = not road.ignore
        else
            error('cave_map_settings requires that each player [chamber] contains at least two [passage] tags')
        end
    else
        error('cave_map_settings requires a [chamber] for each player with id=player_n where n is the player number')
    end

    local all_players = {first_player}
    for i = 2, #params do
        local next_player = MG.get_chamber(params, 'player_' .. i)
        if next_player then
            table.insert(all_players, next_player)
        else
            break
        end
    end

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
        central_chamber.size = mathx.round((params.map_height + params.map_width) / 4)
    end

    function window.height.on_modified()
        params.map_height = window.height.value
        window.height_label.label = params.map_height
        central_chamber.size = mathx.round((params.map_height + params.map_width) / 4)
    end

    function window.village_density.on_modified()
        params.village_density = window.village_density.value
        -- Need wesnoth-lib for the village density label
        local _ = wesnoth.textdomain "wesnoth-lib"
        window.village_density_label.label = (_"$villages/1000 tiles"):vformat{villages = params.village_density}
    end

    function window.jagged.on_modified()
        local val = window.jagged.value
        central_chamber.jagged = val
        window.jagged_label.label = val
    end

    function window.lake_size.on_modified()
        local val = window.lake_size.value
        lake.size = val
        window.lake_size_label.label = val
    end

    function window.windiness.on_modified()
        local val = window.windiness.value
        for i = 1, #all_players do
            local tunnel = MG.get_passage(all_players[i], 1)
            tunnel.windiness = val
        end
        window.windiness_label.label = val
    end

    function window.roads.on_modified()
        local val = not window.roads.selected
        for i = 1, #all_players do
            local road = MG.get_passage(all_players[i], 2)
            road.ignore = val
        end
    end
end

local dialog = wml.load "multiplayer/gui/cave_map_settings.cfg"
gui.show_dialog(wml.get_child(dialog, 'resolution'), pre_show)
return params
