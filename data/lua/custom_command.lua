local T = wml.tag

wesnoth.custom_synced_commands = {}

function wesnoth.game_events.on_synced_command(cfg)
	local handler = wesnoth.custom_synced_commands[cfg.name]
	local data = wml.get_child(cfg, "data")
	if handler then
		handler(data)
	end
end

function wesnoth.register_synced_command(name, handler)
	wesnoth.custom_synced_commands[name] = handler
end

function wesnoth.invoke_synced_command(name, data)
	wesnoth.wml_actions.do_command { T.custom_command { name=name, T.data(data) } }
end
