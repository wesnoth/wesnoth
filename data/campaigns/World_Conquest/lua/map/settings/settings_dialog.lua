-- the Dialog we show in the generate map tool in wesnoths own map editor.
-- in case that someone wants to create a map based on a random wcii map in his campaign. 
local T = wml.tag
local _ = wesnoth.textdomain 'wesnoth-World_Conquest'


function get_defaults(nplayer, nscenario)
	if wesnoth.have_file(string.format("./../scenarios/WC_II_%dp_scenario%d.lua", nplayer, nscenario)) then
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

local dialog_wml = {
	maximum_width = 900,
	T.helptip { id = "tooltip_large" }, -- mandatory field
	T.tooltip { id = "tooltip_large" }, -- mandatory field

	T.linked_group { id = "linked_group1", fixed_width = true },

	T.grid {
		T.row {
			grow_factor = 1,
			T.column {
				border = "all",
				border_size = 5,
				horizontal_alignment = "left",
				T.label {
					definition = "title",
					label = _"Map Settings",
					id = "title"
				}
			}
		},
		T.row {
			grow_factor = 1,
			T.column {
				horizontal_grow = true,
				vertical_grow = true,
				T.grid {
					T.row {
						T.column {
							border = "all",
							border_size = 5,
							horizontal_grow = true,
							vertical_grow = true,
							T.grid {
								T.row {
									T.column {
										T.label {
											label = _" scenario: ",
										}
									},
									T.column {
										T.slider {
											definition="short",
											id = "sl_scenario",
											minimum_value=1,
											maximum_value=6,
										}
									},
									T.column {
										T.label {
											label = _" map: ",
											width=20,
										}
									},
									T.column {
										T.slider {
											definition="default",
											id = "sl_map",
											minimum_value=1,
											maximum_value=10,
										}
									},
								}
							},
						},
					}
				}
			}
		},
		T.row {
			T.column {
				horizontal_grow = true,
				T.grid {
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "length"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_length",
								minimum_value=1,
								maximum_value=100,
							}
						}
					},
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "villages"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_villages",
								minimum_value=1,
								maximum_value=100,
							}
						}
					},
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "castle"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_castle",
								minimum_value=1,
								maximum_value=20,
							}
						}
					},
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "iterations"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_iterations",
								minimum_value=10,
								maximum_value=20000,
							}
						}
					},
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "hill_size"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_hill_size",
								minimum_value=1,
								maximum_value=10,
							}
						}
					},
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "island"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_island",
								minimum_value=1,
								maximum_value=10,
							}
						}
					},
					T.row {
						T.column {
							grow_factor = 1,
							T.label {
								label = "castles"
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.slider {
								id = "sl_ncastles",
								minimum_value=1,
								maximum_value=10,
							}
						}
					},
				}
			},
			T.row {
				T.column {
					horizontal_grow = true,
					T.grid {
						T.row {
							T.column {
								grow_factor = 1,
								T.spacer {
								}
							},
							T.column {
								horizontal_alignment = "right",
								T.button {
									label = "Ok",
									id = "ok",
								}
							}
						}
					}
				}
			}
		}
	}
}

function wc2_debug_settings(nplayers)
	local function on_set_map()
		globals.settings.scenario_num = wesnoth.get_dialog_value("sl_scenario")
			std_print(nplayers, globals.settings.scenario_num)
		local generators = get_defaults(nplayers, globals.settings.scenario_num)
		local map_data = generators[wesnoth.get_dialog_value("sl_map")]
		if map_data then
			wesnoth.set_dialog_value(map_data.length, "sl_length")
			wesnoth.set_dialog_value(map_data.villages, "sl_villages")
			wesnoth.set_dialog_value(map_data.castle, "sl_castle")
			wesnoth.set_dialog_value(map_data.iterations, "sl_iterations")
			wesnoth.set_dialog_value(map_data.island, "sl_island")
			wesnoth.set_dialog_value(map_data.hill_size, "sl_hill_size")
			wesnoth.set_dialog_value(map_data.ncastles, "sl_ncastles")

			globals.settings.default_id = map_data.default_id
			globals.settings.postgen_id = map_data.postgen_id
		end
	end
	local function on_set_scenario()
		globals.settings.scenario_num = wesnoth.get_dialog_value("sl_scenario")
		get_defaults(nplayers, globals.settings.scenario_num)
		on_set_map()
	end
	local function preshow()
		wesnoth.set_dialog_value(globals.settings.scenario_num or 1, "sl_scenario")
		wesnoth.set_dialog_value(globals.settings.map_num or 1, "sl_map")
		wesnoth.set_dialog_callback(on_set_scenario, "sl_scenario")
		wesnoth.set_dialog_callback(on_set_map, "sl_map")
		on_set_map()
	end
	local function postshow()
		globals.settings.scenario_num = wesnoth.get_dialog_value("sl_scenario")
		globals.settings.map_num = wesnoth.get_dialog_value("sl_map")

		globals.settings.length = wesnoth.get_dialog_value("sl_length")
		globals.settings.villages = wesnoth.get_dialog_value("sl_villages")
		globals.settings.castle = wesnoth.get_dialog_value("sl_castle")
		globals.settings.iterations = wesnoth.get_dialog_value("sl_iterations")
		globals.settings.island = wesnoth.get_dialog_value("sl_island")
		globals.settings.hill_size = wesnoth.get_dialog_value("sl_hill_size")
		globals.settings.ncastles = wesnoth.get_dialog_value("sl_ncastles")
	end
	wesnoth.show_dialog(dialog_wml, preshow, postshow)
end
