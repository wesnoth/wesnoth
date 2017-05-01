--! #textdomain wesnoth

-- This file may provide an implementation of Lua functions removed from the engine.

local helper = wesnoth.require "lua/helper.lua"

function wesnoth.set_music(cfg)
	wesnoth.wml_actions.music(cfg)
end

-- Calling wesnoth.fire isn't the same as calling wesnoth.wml_actions[name] due to the passed vconfig userdata
-- which also provides "constness" of the passed wml object from the point of view of the caller.
-- So please don't remove since it's not deprecated.
function wesnoth.fire(name, cfg)
	wesnoth.wml_actions[name](wesnoth.tovconfig(cfg or {}))
end
