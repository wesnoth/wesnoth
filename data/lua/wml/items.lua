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

local old_on_save = game_events.on_save
function game_events.on_save()
	local custom_cfg = old_on_save()
	for i,v in pairs(scenario_items) do
		for j,w in pairs(v) do
			table.insert(custom_cfg, { "item", w })
		end
	end
	return custom_cfg
end

local old_on_load = game_events.on_load
function game_events.on_load(cfg)
	for i = #cfg,1,-1 do
		local v = cfg[i]
		if v[1] == "item" then
			local v2 = v[2]
			add_overlay(v2.x, v2.y, v2)
			table.remove(cfg, i)
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

function wml_actions.removeitem(cfg)
	local x, y = tonumber(cfg.x), tonumber(cfg.y)
	if not x or not y then
		local context = wesnoth.current.event_context
		x = context.x1 or
			helper.wml_error "[remove_item] missing required x= and y= attributes."
		y = context.y1
	end
	local items = scenario_items[x * 10000 + y]
	if not items then return end
	local name = cfg.image
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
