wesnoth.dofile('./../lua/map/main.lua')
return {
	create_scenario = function(cfg)
		return wc_ii_generate_scenario(cfg.nplayers, cfg)
	end,
	user_config = function(cfg)
		wesnoth.dofile('./../lua/map/settings/settings_dialog.lua')
		return wc2_debug_settings()
	end
}
