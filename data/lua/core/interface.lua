--[========[Game Interface Control]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
	print("Loading interface module...")

	wesnoth.interface.select_unit = wesnoth.units.select

	---Fakes the move of a unit satisfying the given filter to position x, y.
	---Usable only during WML actions.
	---@param filter WML
	---@param to_x integer
	---@param to_y integer
	function wesnoth.interface.move_unit_fake(filter, to_x, to_y)
		local moving_unit = wesnoth.units.find_on_map(filter)[1]
		local from_x, from_y = moving_unit.x, moving_unit.y

		wesnoth.interface.scroll_to_hex(from_x, from_y)
		to_x, to_y = wesnoth.paths.find_vacant_hex(to_x, to_y, moving_unit)

		if to_x < from_x then
			moving_unit.facing = "sw"
		elseif to_x > from_x then
			moving_unit.facing = "se"
		end
		moving_unit:extract()

		wesnoth.wml_actions.move_unit_fake{
			type      = moving_unit.type,
			gender    = moving_unit.gender,
			variation = moving_unit.variation,
			side      = moving_unit.side,
			x         = from_x .. ',' .. to_x,
			y         = from_y .. ',' .. to_y
		}

		moving_unit:to_map(to_x, to_y)
		wesnoth.wml_actions.redraw{}
	end

	wesnoth.delay = wesnoth.deprecate_api('wesnoth.delay', 'wesnoth.interface.delay', 1, nil, wesnoth.interface.delay)
	wesnoth.float_label = wesnoth.deprecate_api('wesnoth.float_label', 'wesnoth.interface.float_label', 1, nil, wesnoth.interface.float_label)
	wesnoth.highlight_hex = wesnoth.deprecate_api('wesnoth.highlight_hex', 'wesnoth.interface.highlight_hex', 1, nil, wesnoth.interface.highlight_hex)
	wesnoth.deselect_hex = wesnoth.deprecate_api('wesnoth.deselect_hex', 'wesnoth.interface.deselect_hex', 1, nil, wesnoth.interface.deselect_hex)
	wesnoth.get_selected_tile = wesnoth.deprecate_api('wesnoth.get_selected_tile', 'wesnoth.interface.get_selected_hex', 1, nil, wesnoth.interface.get_selected_hex)
	wesnoth.get_mouseover_tile = wesnoth.deprecate_api('wesnoth.get_mouseover_tile', 'wesnoth.interface.get_hovered_hex', 1, nil, wesnoth.interface.get_hovered_hex)
	wesnoth.scroll_to_tile = wesnoth.deprecate_api('wesnoth.scroll_to_tile', 'wesnoth.interface.scroll_to_hex', 1, nil, wesnoth.interface.scroll_to_hex)
	wesnoth.scroll = wesnoth.deprecate_api('wesnoth.scroll', 'wesnoth.interface.scroll', 1, nil, wesnoth.interface.scroll)
	wesnoth.lock_view = wesnoth.deprecate_api('wesnoth.lock_view', 'wesnoth.interface.lock', 1, nil, wesnoth.interface.lock)
	wesnoth.view_locked = wesnoth.deprecate_api('wesnoth.view_locked', 'wesnoth.interface.is_locked', 1, nil, wesnoth.interface.is_locked)
	wesnoth.is_skipping_messages = wesnoth.deprecate_api('wesnoth.is_skipping_messages', 'wesnoth.interface.is_skipping_messages', 1, nil, wesnoth.interface.is_skipping_messages)
	wesnoth.skip_messages = wesnoth.deprecate_api('wesnoth.skip_messages', 'wesnoth.interface.skip_messages', 1, nil, wesnoth.interface.skip_messages)
	wesnoth.add_tile_overlay = wesnoth.deprecate_api('wesnoth.add_tile_overlay', 'wesnoth.interface.add_hex_overlay', 1, nil, wesnoth.interface.add_hex_overlay)
	wesnoth.remove_tile_overlay = wesnoth.deprecate_api('wesnoth.remove_tile_overlay', 'wesnoth.interface.remove_hex_overlay', 1, nil, wesnoth.interface.remove_hex_overlay)
	wesnoth.theme_items = wesnoth.deprecate_api('wesnoth.theme_items', 'wesnoth.interface.game_display', 1, nil, wesnoth.interface.game_display)
	wesnoth.get_displayed_unit = wesnoth.deprecate_api('wesnoth.get_displayed_unit', 'wesnoth.interface.get_displayed_unit', 1, nil, wesnoth.interface.get_displayed_unit)
	wesnoth.zoom = wesnoth.deprecate_api('wesnoth.zoom', 'wesnoth.interface.zoom', 1, nil, wesnoth.interface.zoom)
	wesnoth.color_adjust = wesnoth.deprecate_api('wesnoth.color_adjust', 'wesnoth.interface.color_adjust', 1, nil, function(cfg) wesnoth.interface.color_adjust(cfg.red, cfg.green, cfg.blue) end)
	wesnoth.allow_end_turn = wesnoth.deprecate_api('wesnoth.allow_end_turn', 'wesnoth.interface.allow_end_turn', 1, nil, wesnoth.interface.allow_end_turn)
	wesnoth.clear_messages = wesnoth.deprecate_api('wesnoth.clear_messages', 'wesnoth.interface.clear_chat_messages', 1, nil, wesnoth.interface.clear_chat_messages)
	wesnoth.end_turn = wesnoth.deprecate_api('wesnoth.end_turn', 'wesnoth.interface.end_turn', 1, nil, wesnoth.interface.end_turn)
	wesnoth.get_viewing_side = wesnoth.deprecate_api('wesnoth.get_viewing_side', 'wesnoth.interface.get_viewing_side', 1, nil, wesnoth.interface.get_viewing_side)
	wesnoth.message = wesnoth.deprecate_api('wesnoth.message', 'wesnoth.interface.add_chat_message', 1, nil, wesnoth.interface.add_chat_message)
	-- wesnoth.wml_actions.print doesn't exist yet at this point, so create a helper function instead.
	wesnoth.print = wesnoth.deprecate_api('wesnoth.print', 'wesnoth.interface.add_overlay_text', 1, nil, function(cfg)
		wesnoth.wml_actions.print(cfg)
	end)
	wesnoth.set_menu_item = wesnoth.deprecate_api('wesnoth.set_menu_item', 'wesnoth.interface.set_menu_item', 1, nil, function(id, cfg)
		-- wesnoth.set_menu_item added both the menu item and the event that it triggers
		-- wesnoth.interface.set_menu_item only adds the menu item
		wesnoth.interface.set_menu_item(id, cfg)
		wesnoth.game_events.add_menu(cfg.id, wesnoth.wml_actions.command)
	end)
	wesnoth.clear_menu_item = wesnoth.interface.clear_menu_item
	-- Event handlers don't have a separate module Lua file so dump those here
	wesnoth.add_event_handler = wesnoth.deprecate_api('wesnoth.add_event_hander', 'wesnoth.game_events.add', 1, nil, function(cfg) wesnoth.wml_actions.event(cfg) end)
	wesnoth.remove_event_handler = wesnoth.deprecate_api('wesnoth.remove_event_handler', 'wesnoth.game_events.remove', 1, nil, wesnoth.game_events.remove)

	local function old_fire_event(fcn)
		return function(...)
			local id = select(1, ...)
			local loc1, n1 = wesnoth.map.read_location(select(2, ...))
			local loc2, n2 = wesnoth.map.read_location(select(2 + n1, ...))
			local weap1, weap2 = select(2 + n1 + n2)
			local data = {}
			if weap1 ~= nil then
				table.insert(data, wml.tag.first(weap1))
			end
			if weap2 ~= nil then
				table.insert(data, wml.tag.second(weap2))
			end
			if n1 > 0 then
				if n2 > 0 then
					fcn(id, loc1, loc2, data)
				else
					fcn(id, loc1, data)
				end
			else
				fcn(id, data)
			end
		end
	end
	wesnoth.fire_event = wesnoth.deprecate_api('wesnoth.fire_event', 'wesnoth.game_events.fire', 1, nil, old_fire_event(wesnoth.game_events.fire))
	wesnoth.fire_event_by_id = wesnoth.deprecate_api('wesnoth.fire_event_by_id', 'wesnoth.game_events.fire_by_id', 1, nil, old_fire_event(wesnoth.game_events.fire_by_id))
end
