function wesnoth.wml_actions.store_time_of_day(cfg)
	local x = tonumber(cfg.x)
	local y = tonumber(cfg.y)

	local turn = tonumber(cfg.turn)

	if not turn or turn == 0 then
		turn = wesnoth.current.turn
	end

	local variable = cfg.variable or "time_of_day"

	local out
	if x and y then
		out = wesnoth.schedule.get_time_of_day(x, y, turn)
	else
		out = wesnoth.schedule.get_time_of_day(nil, turn)
	end

	for key, value in pairs(out) do
		wml.variables[variable .. "[0]." .. key] = value
	end
end

