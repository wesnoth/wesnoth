local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"

local function path_locs(path)
	if path.location_id then
		-- Index is 1 for x, 2 for y
		local function special_locations(index)
			return function()
				for loc in utils.split(path.location_id) do
					loc = wesnoth.special_locations[loc]
					if loc then coroutine.yield(loc[index]) end
				end
			end
		end
		return coroutine.wrap(special_locations(1)), coroutine.wrap(special_locations(2))
	elseif path.dir then
		local coord = {'x', 'y'}
		-- Index is 1 for x, 2 for y
		local function relative_locations(index)
			return function(u)
				local last = {x = u.x, y = u.y}
				for dir in utils.split(cfg.dir) do
					local count = 1
					if dir:find(":") then
						local error_dir = dir
						dir, count = dir:match("([a-z]+):(%d+)")
						if not dir or not count then
							wml.error("Invalid direction:count in move_unit: " .. error_dir)
						end
					end
					next_loc = wesnoth.map.get_direction(last.x, last.y, dir, count)
					coroutine.yield(next_loc[index])
					last.x, last.y = next_loc[1], next_loc[2]
				end
			end
		end
	else
		return utils.split(path.to_x), utils.split(path.to_y)
	end
end

function wesnoth.wml_actions.move_unit(cfg)
	local coordinate_error = "invalid location in [move_unit]"
	local path
	if cfg.to_location then
		path = {location_id = cfg.to_location}
	elseif cfg.dir then
		path = {dir = cfg.dir}
	else
		path = {to_x = cfg.to_x, to_y = cfg.to_y}
	end
	if not path then
		helper.wml_error(coordinate_error)
	end
	local fire_event = cfg.fire_event
	local muf_force_scroll = cfg.force_scroll
	local check_passability = cfg.check_passability
	if check_passability == nil then check_passability = true end
	cfg = wml.literal(cfg)
	cfg.to_location, cfg.to_x, cfg.to_y, cfg.fire_event = nil
	local units = wesnoth.get_units(cfg)

	for current_unit_index, current_unit in ipairs(units) do
		if not fire_event or current_unit.valid then
			local xs, ys = path_locs(path)
			local move_string_x = current_unit.x
			local move_string_y = current_unit.y
			local pass_check = nil
			if check_passability then pass_check = current_unit end

			local x, y = xs(current_unit), ys(current_unit)
			local prevX, prevY = tonumber(current_unit.x), tonumber(current_unit.y)
			while true do
				x = tonumber(x) or helper.wml_error(coordinate_error)
				y = tonumber(y) or helper.wml_error(coordinate_error)
				if not (x == prevX and y == prevY) then x, y = wesnoth.find_vacant_tile(x, y, pass_check) end
				if not x or not y then helper.wml_error("Could not find a suitable hex near to one of the target hexes in [move_unit].") end
				move_string_x = string.format("%s,%u", move_string_x, x)
				move_string_y = string.format("%s,%u", move_string_y, y)
				local next_x, next_y = xs(current_unit), ys(current_unit)
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
