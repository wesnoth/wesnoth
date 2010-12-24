local helper = wesnoth.require "lua/helper.lua"
local wml_actions = wesnoth.wml_actions
local game_events = wesnoth.game_events

local scenario_items = {}

local function add_overlay(x, y, cfg)
	wesnoth.add_tile_overlay(x, y, cfg)
	local items = scenario_items[x * 10000 + y]
	if not items then
		items = {}
		scenario_items[x * 10000 + y] = items
	end
	table.insert(items, cfg)
end

local function remove_overlay(x, y, name)
	local items = scenario_items[x * 10000 + y]
	if not items then return end
	wesnoth.remove_tile_overlay(x, y, name)
	if name then
		for i = #items,1,-1 do
			local item = items[i]
			if item.image == name or item.halo == name then
				table.remove(items, i)
			end
		end
	end
	if not name or #items == 0 then
		scenario_items[x * 10000 + y] = nil
	end
end

local old_on_save = game_events.on_save
function game_events.on_save()
	local custom_cfg = old_on_save()
	for i,v in pairs(scenario_items) do
		for j,w in ipairs(v) do
			table.insert(custom_cfg, { "item", w })
		end
	end
	return custom_cfg
end

local old_on_load = game_events.on_load
function game_events.on_load(cfg)
	local i = 1
	while i <= #cfg do
		local v = cfg[i]
		if v[1] == "item" then
			local v2 = v[2]
			add_overlay(v2.x, v2.y, v2)
			table.remove(cfg, i)
		else
			i = i + 1
		end
	end
	old_on_load(cfg)
end

function wml_actions.item(cfg)
	cfg = helper.parsed(cfg)
	if not cfg.image and not cfg.halo then
		helper.wml_error "[item] missing required image= and halo= attributes."
	end
	local x, y = tonumber(cfg.x), tonumber(cfg.y)
	if not x or not y then
		helper.wml_error "[item] missing required x= and y= attributes."
	end
	add_overlay(x, y, cfg)
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
	remove_overlay(x, y, cfg.image)
end

-- [removeitem] is deprecated, so print a WML error and call [remove_item]
-- Remove after 1.9.3
wml_actions.removeitem = helper.deprecate("Usage of [removeitem] is deprecated; support will be removed in 1.9.3. Use [remove_item] instead.", wml_actions.remove_item)

local methods = { remove = remove_overlay }

function methods.place_image(x, y, name)
	add_overlay(x, y, { x = x, y = y, image = name })
end

function methods.place_halo(x, y, name)
	add_overlay(x, y, { x = x, y = y, halo = name })
end

return methods
