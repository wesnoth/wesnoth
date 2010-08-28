local helper = wesnoth.require "lua/helper.lua"
local wml_actions = wesnoth.wml_actions

function wml_actions.item(cfg)
	if not cfg.image and not cfg.halo then
		helper.wml_error "[item] missing required image= and halo= attributes."
	end
	local x, y = tonumber(cfg.x), tonumber(cfg.y)
	if not x or not y then
		helper.wml_error "[item] missing required x= and y= attributes."
	end
	wesnoth.add_tile_overlay(x, y, cfg)
	wml_actions.redraw {}
end

function wml_actions.remove_item(cfg)
	local x, y = tonumber(cfg.x), tonumber(cfg.y)
	if not x or not y then
		local context = wesnoth.current.event_context
		x = context.x1 or
			helper.wml_error "[remove_item] missing required x= and y= attributes."
		y = context.y1
	end
	wesnoth.remove_tile_overlay(x, y, cfg.image)
end
