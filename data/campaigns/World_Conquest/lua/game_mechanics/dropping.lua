-- general code related to dropping items, code taken from 'scenario with robots' add-on.
-- todo: maybe we coudl mainline this and merge it with the iterms.lua code ( this means
-- adding a mainline "pickup item" event that fires when a unit steops on an [item] and
-- would have the same features as this here)
local on_event = wesnoth.require("on_event")

local dropping = {}

dropping.field_data = {}

dropping.loc_to_index = function(x,y)
	return (y - 1) * 1000 + x
end

dropping.index_to_loc = function(index)
	local y_m1 = math.floor(index / 1000)
	return index - y_m1 * 1000, y_m1 + 1
end

dropping.decorate_imagename = function(id)
	return "wc2_item_" .. id
end

dropping.place_image = function(x, y, cfg)
	wesnoth.add_tile_overlay(x, y, {
		image = cfg.image,
		team_name = cfg.team_name,
		visible_in_fog = cfg.visible_in_fog,
		redraw = cfg.redraw,
		name = dropping.decorate_imagename(cfg.id),
		z_order = cfg.z_order
	})
end


-- this  can be used to remove item but not to add items.
dropping.get_entries_at_readonly = function(x,y)
	return dropping.field_data[dropping.loc_to_index(x,y)] or {}
end

dropping.get_entries_at_readwrite = function(x,y)
	local index = dropping.loc_to_index(x,y)
	dropping.field_data[index] = dropping.field_data[index] or {}
	return dropping.field_data[index]
end

dropping.remove_empty_lists = function()
	local to_delete = {}
	for k,v in pairs(dropping.field_data) do
		if #v == 0 then
			to_delete[k] = true
		end
	end
	for k,v in pairs(to_delete) do
		dropping.field_data[k] = nil
	end
end

dropping.add_item = function(x, y, cfg)
	table.insert(dropping.get_entries_at_readwrite(x,y), cfg)
	cfg.id = dropping.next_id
	dropping.next_id = dropping.next_id + 1
	dropping.place_image(x, y, cfg)
end

dropping.remove_item = function(x, y, id)
	local entries = dropping.get_entries_at_readwrite(x,y)
	for i,v in ipairs(entries) do
		if v.id == id then
			wesnoth.remove_tile_overlay(x, y, dropping.decorate_imagename(id))
			table.remove(entries, i)
			break
		end
	end
end

dropping.remove_all_items = function(filter)
	for k,v in pairs(dropping.field_data) do
		local start = #v
		local x, y = dropping.index_to_loc(k)
		for i = start, 1, -1 do
			if filter(v[i], x, y) then
				wesnoth.remove_tile_overlay(x, y, dropping.decorate_imagename(v[i].id))
				table.remove(v, i)
			end
		end
	end
end

dropping.remove_current_item = function()
	local v = dropping.current_item
	local ec = wesnoth.current.event_context
	wesnoth.remove_tile_overlay(ec.x1, ec.y1, dropping.decorate_imagename(v.id))
	dropping.item_taken = true
end


wesnoth.persistent_tags.wc2_dropping.write = function(add)
	local res = {
		next_id = dropping.next_id,
	}
	for i,v in pairs(dropping.field_data) do
		local x,y = dropping.index_to_loc(i)
		for i2,v2 in ipairs(v) do
			table.insert(res, wml.tag.item {
				x = x,
				y = y,
				wml.tag.data (v2)
			})
		end
	end		
	add(res)
end
-- read might not be called if there is no [dropped_items] tag found
wesnoth.persistent_tags.wc2_dropping.read = function(cfg)
	for item in wml.child_range(cfg, "item") do
		local hex_list = dropping.get_entries_at_readwrite(item.x, item.y)
		table.insert(hex_list, (wml.get_child(item, "data")))
	end
	dropping.next_id = cfg.next_id or 0
	dropping.remove_empty_lists()
end

on_event("moveto", function(event_context)
	local x = event_context.x1
	local y = event_context.y1
	local entries = dropping.get_entries_at_readonly(x,y)
	local i = 1
	while i <= #entries do
		local v = entries[i]
		dropping.current_item = v
		dropping.item_taken = nil
		wesnoth.fire_event("wc2_drop_pickup", x, y)
		if dropping.item_taken then
			table.remove(entries, i)
			wesnoth.remove_tile_overlay(x, y, dropping.decorate_imagename(v.id))
			wesnoth.allow_undo(false)
		else
			i = i + 1
		end
		dropping.current_item = nil
		dropping.item_taken = nil
	end
end)

on_event("preload", function()
	dropping.next_id = dropping.next_id or 0
	for k,v in pairs(dropping.field_data) do
		local x,y = dropping.index_to_loc(k)
		for i, cfg in ipairs(v) do
			dropping.place_image(x, y, cfg)
		end
	end
end)

return dropping
