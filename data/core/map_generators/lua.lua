return {
	create_scenario = function(cfg)
		local f = load(cfg.create_scenario)
		return f(cfg)
	end,
	create_map = function(cfg)
		local f = load(cfg.create_map)
		return f(cfg)
	end,
	user_config = function(cfg)
		local f = load(cfg.user_config)
		return f(cfg)
	end,
}
