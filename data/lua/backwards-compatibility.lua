--! #textdomain wesnoth

-- This file may provide an implementation of Lua functions removed from the engine.

wesnoth.set_music = wesnoth.deprecate_api('wesnoth.set_music', 'wesnoth.music_list', 1, nil, wesnoth.wml_actions.music)

-- Calling wesnoth.fire isn't the same as calling wesnoth.wml_actions[name] due to the passed vconfig userdata
-- which also provides "constness" of the passed wml object from the point of view of the caller.
-- So please don't remove since it's not deprecated.
function wesnoth.fire(name, cfg)
	wesnoth.wml_actions[name](wesnoth.tovconfig(cfg or {}))
end
