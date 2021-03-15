local wml_actions = wesnoth.wml_actions

local scenario_items = (wesnoth.require "location_set").create()
local next_item_name = 0

local function add_overlay(x, y, cfg)

	if not cfg.name then
		cfg.name = "item_" .. tostring(next_item_name)
		next_item_name = next_item_name + 1
	end

	wesnoth.interface.add_hex_overlay(x, y, cfg)
	local items = scenario_items:get(x, y)
	if not items then
		items = {}
		scenario_items:insert(x, y, items)
	end
	table.insert(items,
		{
			x = x, y = y,
			image = cfg.image,
			halo = cfg.halo,
			team_name = cfg.team_name,
			filter_team = cfg.filter_team,
			visible_in_fog = cfg.visible_in_fog,
			redraw = cfg.redraw,
			name = cfg.name,
			z_order = cfg.z_order,
			wml.tag.variables(wml.get_child(cfg, "variables") or {}),
		})
end

function wesnoth.interface.remove_item(x, y, name)
	local items = scenario_items:get(x, y)
	if not items then return end
	wesnoth.interface.remove_hex_overlay(x, y, name)
	if name then
		for i = #items,1,-1 do
			local item = items[i]
			if item.image == name or item.halo == name or item.name == name then
				table.remove(items, i)
			end
		end
	end
	if not name or #items == 0 then
		scenario_items:remove(x, y)
	end
end

function wesnoth.interface.get_items(x, y)
	local res = {}
	local items = scenario_items:get(x, y) or {}
	for i = 1,#items do
		local cfg = items[i]
		-- make a copy, since modifying these values (except variabels) wouldn't work anyways.
		table.insert( res, {
			x = cfg.x, y = cfg.y,
			image = cfg.image,
			halo = cfg.halo,
			team_name = cfg.team_name,
			filter_team = cfg.filter_team,
			visible_in_fog = cfg.visible_in_fog,
			redraw = cfg.redraw,
			name = cfg.name,
			z_order = cfg.z_order,
			variables = wml.get_child(cfg, "variables"),
		})
	end
	return res
end

function wesnoth.persistent_tags.item.write(add)
	for x,y,v in scenario_items:iter() do
		for i,w in ipairs(v) do
			add(w)
		end
	end
end

function wesnoth.persistent_tags.next_item_name.write(add)
	add{next_item_name = next_item_name}
end

function wesnoth.persistent_tags.item.read(cfg)
	if not cfg.name then cfg.name = "" end
	add_overlay(cfg.x, cfg.y, cfg)
end

function wesnoth.persistent_tags.next_item_name.read(cfg)
	next_item_name = cfg.next_item_name or next_item_name
end


-- returns the 'name' of an item, this can be used as an id to remove the iten later.
function wml_actions.item(cfg)
	local locs = wesnoth.map.find(cfg)
	cfg = wml.parsed(cfg)
	if not cfg.image and not cfg.halo then
		wml.error "[item] missing required image= and halo= attributes."
	end
	for i, loc in ipairs(locs) do
		add_overlay(loc[1], loc[2], cfg)
	end
	local redraw = cfg.redraw
	if redraw == nil then redraw = true end
	if redraw then wml_actions.redraw {} end
	if cfg.write_name then wml.variables[cfg.write_name] = cfg.name end
	return cfg.name
end

function wml_actions.remove_item(cfg)
	local locs = wesnoth.map.find(cfg)
	for i, loc in ipairs(locs) do
		wesnoth.interface.remove_item(loc[1], loc[2], cfg.image)
	end
end

function wml_actions.store_items(cfg)
	local variable = cfg.variable or "items"
	local item_name = cfg.item_name
	variable = tostring(variable or wml.error("invalid variable= in [store_items]"))
	wml.variables[variable] = nil
	local index = 0
	for i, loc in ipairs(wesnoth.map.find(cfg)) do
		local items = scenario_items[loc]
		if items then
			for j, item in ipairs(items) do
				-- note: item_name can not be part of standard location filter because
				--       there might be multiple items on one locations and we don't
				--       want to return them all if only one matches the name
				-- todo: consider making cfg.item_name a comma list or a regex.
				if item_name == nil or item.name == item_name then
					wml.variables[string.format("%s[%u]", variable, index)] = item
					index = index + 1
				end
			end
		end
	end
end

function wesnoth.interface.add_item_image(x, y, name)
	add_overlay(x, y, { x = x, y = y, image = name })
end

function wesnoth.interface.add_item_halo(x, y, name)
	add_overlay(x, y, { x = x, y = y, halo = name })
end

local methods = {
	remove = wesnoth.deprecate_api('items.remove', 'wesnoth.interface.remove_item', 1, nil, wesnoth.interface.remove_item),
	place_image = wesnoth.deprecate_api('items.place_image', 'wesnoth.interface.add_item_image', 1, nil, wesnoth.interface.add_item_image),
	place_halo = wesnoth.deprecate_api('items.place_halo', 'wesnoth.interface.add_item_halo', 1, nil, wesnoth.interface.add_item_halo)
}

return methods
