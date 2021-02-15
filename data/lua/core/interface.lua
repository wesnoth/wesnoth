--[========[Game Interface Control]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
	print("Loading interface module...")

	wesnoth.interface.select_unit = wesnoth.units.select

	--! Fakes the move of a unit satisfying the given @a filter to position @a x, @a y.
	--! @note Usable only during WML actions.
	function wesnoth.interface.move_unit_fake(filter, to_x, to_y)
		local moving_unit = wesnoth.units.find_on_map(filter)[1]
		local from_x, from_y = moving_unit.x, moving_unit.y

		wesnoth.interface.scroll_to_hex(from_x, from_y)
		to_x, to_y = wesnoth.find_vacant_tile(to_x, to_y, moving_unit)

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
	wesnoth.gamestate_inspector = wesnoth.deprecate_api('wesnoth.gamestate_inspector', 'gui.show_inspector', 1, nil, gui.show_inspector)
	wesnoth.color_adjust = wesnoth.deprecate_api('wesnoth.color_adjust', 'wesnoth.interface.color_adjust', 1, nil, wesnoth.interface.color_adjust)
	-- No deprecation for these since since they're not actually public API yet
	wesnoth.set_menu_item = wesnoth.interface.set_menu_item
	wesnoth.clear_menu_item = wesnoth.interface.clear_menu_item
end