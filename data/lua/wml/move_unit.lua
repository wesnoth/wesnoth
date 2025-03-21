
local function path_locs(path)
	if path.location_id then
		local function special_locations()
			return function()
				for _,loc_id in ipairs(tostring(path.location_id):split()) do
					local loc = wesnoth.current.map.special_locations[loc_id]
					if loc then coroutine.yield(loc.x, loc.y) end
				end
			end
		end
		return coroutine.wrap(special_locations())
	elseif path.dir then
		local function relative_locations()
			return function(u)
				local last = {x = u.x, y = u.y}
				for _,dir in ipairs(path.dir:split()) do
					local count = 1
					if dir:find(":") then
						local error_dir = dir
						dir, count = dir:match("([a-z]+):(%d+)")
						if not dir or not count then
							wml.error("Invalid direction:count in move_unit: " .. error_dir)
						end
					end
					local next_loc = wesnoth.map.get_direction(last.x, last.y, dir, count)
					coroutine.yield(next_loc[1], next_loc[2])
					last.x, last.y = next_loc[1], next_loc[2]
				end
			end
		end
		return coroutine.wrap(relative_locations())
	else
		local function abs_locations(coord)
			return function()
				for _,s in ipairs(tostring(path[coord]):split()) do
					coroutine.yield(tonumber(s))
				end
			end
		end
		-- Double-coroutining seems a bit excessive but I can't think of a better way to do this?
		return coroutine.wrap(function()
			local xs, ys = coroutine.wrap(abs_locations('to_x')), coroutine.wrap(abs_locations('to_y'))
			repeat
				local x, y = xs(), ys()
				coroutine.yield(x, y)
			until x == nil or y == nil
		end)
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
		wml.error(coordinate_error)
	end
	local fire_event = cfg.fire_event
	local unshroud = cfg.clear_shroud
	local muf_force_scroll = cfg.force_scroll
	local check_passability = cfg.check_passability
	if check_passability == nil then check_passability = true end
	cfg = wml.literal(cfg)
	cfg.to_location, cfg.to_x, cfg.to_y, cfg.fire_event, cfg.clear_shroud = nil, nil, nil, nil, nil
	local units = wesnoth.units.find_on_map(cfg)

	for current_unit_index, current_unit in ipairs(units) do
		if not fire_event or current_unit.valid then
			local locs = path_locs(path)
			local x_list = {current_unit.x}
			local y_list = {current_unit.y}
			local pass_check = nil
			if check_passability then pass_check = current_unit end

			current_unit:extract()
			local x, y = locs(current_unit)
			local prevX, prevY = tonumber(current_unit.x), tonumber(current_unit.y)
			while true do
				x = tonumber(x) or current_unit:to_map(false) or wml.error(coordinate_error)
				y = tonumber(y) or current_unit:to_map(false) or wml.error(coordinate_error)
				if not (x == prevX and y == prevY) then x, y = wesnoth.paths.find_vacant_hex(x, y, pass_check) end
				if not x or not y then wml.error("Could not find a suitable hex near to one of the target hexes in [move_unit].") end
				table.insert(x_list, x)
				table.insert(y_list, y)
				local next_x, next_y = locs(current_unit)
				if not next_x and not next_y then break end
				prevX, prevY = x, y
				x, y = next_x, next_y
			end

			if current_unit.x < x then current_unit.facing = "se"
			elseif current_unit.x > x then current_unit.facing = "sw"
			end

			local current_unit_cfg = current_unit.__cfg
			wesnoth.wml_actions.move_unit_fake {
				type = current_unit_cfg.type,
				gender = current_unit_cfg.gender,
				variation = current_unit_cfg.variation,
				image_mods = current_unit.image_mods,
				side = current_unit_cfg.side,
				x = x_list,
				y = y_list,
				force_scroll = muf_force_scroll
			}
			local x2, y2 = current_unit.x, current_unit.y
			current_unit.x, current_unit.y = x, y
			current_unit:to_map(false)
			if unshroud then
				wesnoth.wml_actions.redraw {clear_shroud=true}
			end

			if fire_event then
				wesnoth.game_events.fire("moveto", x, y, x2, y2)
			end
		end
	end
end
