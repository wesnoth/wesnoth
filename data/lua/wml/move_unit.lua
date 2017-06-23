local helper = wesnoth.require "helper"

function wesnoth.wml_actions.move_unit(cfg)
	local coordinate_error = "invalid coordinate in [move_unit]"
	local to_x = tostring(cfg.to_x or helper.wml_error(coordinate_error))
	local to_y = tostring(cfg.to_y or helper.wml_error(coordinate_error))
	local fire_event = cfg.fire_event
	local muf_force_scroll = cfg.force_scroll
	local check_passability = cfg.check_passability
	if check_passability == nil then check_passability = true end
	cfg = helper.literal(cfg)
	cfg.to_x, cfg.to_y, cfg.fire_event = nil, nil, nil
	local units = wesnoth.get_units(cfg)

	local pattern = "[^%s,]+"
	for current_unit_index, current_unit in ipairs(units) do
		if not fire_event or current_unit.valid then
			local xs, ys = string.gmatch(to_x, pattern), string.gmatch(to_y, pattern)
			local move_string_x = current_unit.x
			local move_string_y = current_unit.y
			local pass_check = nil
			if check_passability then pass_check = current_unit end

			local x, y = xs(), ys()
			local prevX, prevY = tonumber(current_unit.x), tonumber(current_unit.y)
			while true do
				x = tonumber(x) or helper.wml_error(coordinate_error)
				y = tonumber(y) or helper.wml_error(coordinate_error)
				if not (x == prevX and y == prevY) then x, y = wesnoth.find_vacant_tile(x, y, pass_check) end
				if not x or not y then helper.wml_error("Could not find a suitable hex near to one of the target hexes in [move_unit].") end
				move_string_x = string.format("%s,%u", move_string_x, x)
				move_string_y = string.format("%s,%u", move_string_y, y)
				local next_x, next_y = xs(), ys()
				if not next_x and not next_y then break end
				prevX, prevY = x, y
				x, y = next_x, next_y
			end

			if current_unit.x < x then current_unit.facing = "se"
			elseif current_unit.x > x then current_unit.facing = "sw"
			end

			wesnoth.extract_unit(current_unit)
			local current_unit_cfg = current_unit.__cfg
			wesnoth.wml_actions.move_unit_fake {
				type = current_unit_cfg.type,
				gender = current_unit_cfg.gender,
				variation = current_unit_cfg.variation,
				image_mods = current_unit.image_mods,
				side = current_unit_cfg.side,
				x = move_string_x,
				y = move_string_y,
				force_scroll = muf_force_scroll
			}
			local x2, y2 = current_unit.x, current_unit.y
			current_unit.x, current_unit.y = x, y
			wesnoth.put_unit(current_unit)

			if fire_event then
				wesnoth.fire_event("moveto", x, y, x2, y2)
			end
		end
	end
end
