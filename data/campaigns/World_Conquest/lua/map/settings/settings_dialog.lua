-- the Dialog we show in the generate map tool in wesnoths own map editor.
-- in case that someone wants to create a map based on a random wcii map in his campaign. 
local T = wml.tag
local _ = wesnoth.textdomain 'wesnoth-wc'


function get_defaults(nplayer, nscenario)
	if filesystem.have_file(string.format("./../scenarios/WC_II_%dp_scenario%d.lua", nplayer, nscenario)) then
		local old_wct_map_generator = _G.wct_map_generator
		_G.wct_map_generator = function(default_id, postgen_id, length, villages, castle, iterations, hill_size, players, island)
			return {
				default_id=default_id,
				postgen_id=postgen_id,
				length=length,
				villages=villages,
				castle=castle,
				iterations=iterations,
				hill_size=hill_size,
				ncastles=players,
				island=island
			}
		end
		local scenario_data = wesnoth.dofile(string.format("./../scenarios/WC_II_%dp_scenario%d.lua", nplayer, nscenario))
		_G.wct_map_generator = old_wct_map_generator
		return scenario_data.generators
	end
	std_print(string.format("./../scenarios/WC_II_%dp_scenario%d.lua", nplayer, nscenario), " Not found")
	return {}
end

local dialog_wml = wml.load "campaigns/World_Conquest/gui/settings_dialog.cfg"

function wc2_debug_settings()


	local function preshow(window)

		local sl_scenario = window.sl_scenario
		local sl_map = window.sl_map
		local sl_players = window.sl_players
		
		local function on_set_map()

			globals.settings.scenario_num = sl_scenario.value

			std_print(sl_players.value, globals.settings.scenario_num)
			local generators = get_defaults(sl_players.value, globals.settings.scenario_num)
			local map_data = generators[sl_map.value]
			if map_data then
	
				window.sl_length.value = map_data.length
				window.sl_villages.value = map_data.villages
				window.sl_castle.value = map_data.castle
				window.sl_iterations.value = map_data.iterations
				window.sl_island.value = map_data.island
				window.sl_hill_size.value = map_data.hill_size
				window.sl_ncastles.value = map_data.ncastles


				globals.settings.default_id = map_data.default_id
				globals.settings.postgen_id = map_data.postgen_id
			end
		end

		local function on_set_scenario()
			globals.settings.scenario_num = sl_scenario.value
			get_defaults(sl_players.value, globals.settings.scenario_num)
			on_set_map()
		end

		local function on_set_players()
			globals.settings.nplayers = sl_players.value
			get_defaults(sl_players.value, globals.settings.scenario_num)
			on_set_map()
		end


		sl_scenario.value = globals.settings.scenario_num or 1
		sl_map.value = globals.settings.map_num or 1
		sl_players.value = globals.settings.nplayers or 1
		sl_scenario.on_modified = on_set_scenario
		sl_map.on_modified = on_set_map
		sl_players.on_modified = on_set_players

		on_set_map()
	end

	local function postshow(window)
		globals.settings.scenario_num = window.sl_scenario.value
		globals.settings.map_num = window.sl_map.value
		globals.settings.nplayers = window.sl_players.value

		globals.settings.length = window.sl_length.value
		globals.settings.villages = window.sl_villages.value
		globals.settings.castle = window.sl_castle.value
		globals.settings.iterations = window.sl_iterations.value
		globals.settings.island = window.sl_island.value
		globals.settings.hill_size = window.sl_hill_size.value
		globals.settings.ncastles = window.sl_ncastles.value

	end
	gui.show_dialog(wml.get_child(dialog_wml, "resolution"), preshow, postshow)
end
