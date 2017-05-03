local helper = wesnoth.require "helper"
local wml_actions = wesnoth.wml_actions
local game_events = wesnoth.game_events

local scenario_items = {}
local next_item_name = 0
local function add_overlay(x, y, cfg)
	wesnoth.add_tile_overlay(x, y, cfg)
	local items = scenario_items[x * 10000 + y]
	if not items then
		items = {}
		scenario_items[x * 10000 + y] = items
	end
	table.insert(items, { x = x, y = y, image = cfg.image, halo = cfg.halo, team_name = cfg.team_name, visible_in_fog = cfg.visible_in_fog, redraw = cfg.redraw, name = cfg.name })
end

local function remove_overlay(x, y, name)
	local items = scenario_items[x * 10000 + y]
	if not items then return end
	wesnoth.remove_tile_overlay(x, y, name)
	if name then
		for i = #items,1,-1 do
			local item = items[i]
			if item.image == name or item.halo == name or item.name == name then
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
	table.insert(custom_cfg, { "next_item_name", { next_item_name = next_item_name } })
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
		elseif v[1] == "next_item_name" then
			next_item_name = v[2].next_item_name or next_item_name
			table.remove(cfg, i)
		else
			i = i + 1
		end
	end
	old_on_load(cfg)
end

function wml_actions.item(cfg)
	local locs = wesnoth.get_locations(cfg)
	cfg = helper.parsed(cfg)
	if not cfg.name then
		cfg.name = "item_" .. tostring(next_item_name)
		next_item_name = next_item_name + 1
	end
	if not cfg.image and not cfg.halo then
		helper.wml_error "[item] missing required image= and halo= attributes."
	end
	for i, loc in ipairs(locs) do
		add_overlay(loc[1], loc[2], cfg)
	end
	local redraw = cfg.redraw
	if redraw == nil then redraw = true end
	if redraw then wml_actions.redraw {} end
	if cfg.write_name then wesnoth.set_variable(write_name, cfg.name) end
	return cfg.name
end

function wml_actions.remove_item(cfg)
	local locs = wesnoth.get_locations(cfg)
	for i, loc in ipairs(locs) do
		remove_overlay(loc[1], loc[2], cfg.image)
	end
end

function wml_actions.store_items(cfg)
	local variable = cfg.variable or "items"
	variable = tostring(variable or helper.wml_error("invalid variable= in [store_items]"))
	wesnoth.set_variable(variable)
	local index = 0
	for i, loc in ipairs(wesnoth.get_locations(cfg)) do
		local items = scenario_items[loc[1] * 10000 + loc[2]]
		if items then
			for j, item in ipairs(items) do
				wesnoth.set_variable(string.format("%s[%u]", variable, index), item)
				index = index + 1
			end
		end
	end
end

local methods = { remove = remove_overlay }

function methods.place_image(x, y, name)
	add_overlay(x, y, { x = x, y = y, image = name })
end

function methods.place_halo(x, y, name)
	add_overlay(x, y, { x = x, y = y, halo = name })
end

return methods
