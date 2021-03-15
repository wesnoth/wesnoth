--! #textdomain wesnoth-low

local labels = {}
local wml_label = wesnoth.wml_actions.label

local wml_actions = wesnoth.wml_actions
local T = wml.tag
local vars = wml.variables

function wesnoth.wml_actions.shift_labels(cfg)
	for k, v in ipairs(labels) do
		wml_label { x = v.x, y = v.y }
	end
	for k, v in ipairs(labels) do
		v.x = v.x + cfg.x
		v.y = v.y + cfg.y
		wml_label(v)
	end
end

--
-- Overrides of core tags
--

function wesnoth.wml_actions.label(cfg)
	table.insert(labels, cfg.__parsed)
	wml_label(cfg)
end

function wesnoth.wml_actions.replace_map_section(cfg)
	if not cfg.x and not cfg.y then
		return wesnoth.wml_actions.replace_map(cfg)
	end
	local x1,x2 = string.match(cfg.x, "(%d+)-(%d+)")
	local y1,y2 = string.match(cfg.y, "(%d+)-(%d+)")
	local map = cfg.map_data
	x1 = tonumber(x1)
	y1 = tonumber(y1)
	x2 = x2 + 2
	y2 = y2 + 2
	local t = {}
	local y = 1
	for row in string.gmatch(map, "[^\n]+") do
		if y >= y1 and y <= y2 then
			local r = {}
			local x = 1
			for tile in string.gmatch(row, "[^,]+") do
				if x >= x1 and x <= x2 then r[x - x1 + 1] = tile end
				x = x + 1
			end
			t[y - y1 + 1] = table.concat(r, ',')
		end
		y = y + 1
	end
	local new_map = table.concat(t, '\n')
	wesnoth.wml_actions.replace_map { map = new_map, expand = true, shrink = true }
end

function wesnoth.wml_actions.unstore_left_behind_units(cfg)
	if wml.variables["l3_store_kalenz"] ~= nil then
		local l3_store_kalenz = wml.array_variables["l3_store_kalenz"]
		for i,_ in ipairs(l3_store_kalenz) do
			local var_name = "l3_store_kalenz[" .. tostring(i-1) .. "]"
			wml_actions.unstore_unit {
				variable = var_name,
				x = "recall",
				y = "recall"
			}
		end
		wml.variables["l3_store_kalenz"] = nil
	end
	if wml.variables["l3_store_landar"] ~= nil then
		local l3_store_landar = wml.array_variables["l3_store_landar"]
		for i,_ in ipairs(l3_store_landar) do
			local var_name = "l3_store_landar[" .. tostring(i-1) .. "]"
			wml_actions.unstore_unit {
				variable = var_name,
				x = "recall",
				y = "recall"
			}
		end
		wml.variables["l3_store_landar"] = nil
	end
end

function wesnoth.wml_actions.persistent_carryover_store(cfg)
	for num, side in ipairs(wesnoth.sides) do
		if not side.persistent then
			goto continue
		end
		wml_actions.store_unit {
			T.filter {
				side = num,
			},
			variable = "side_store.unit"
		}
		--TODO: apply carryover multipler and carryover bonus.
		vars["side_store.gold"] = side.gold
		for i = 1, vars["side_store.unit.length"] do
			vars[string.format("side_store.unit[%d].x", i - 1)] = nil
			vars[string.format("side_store.unit[%d].y", i - 1)] = nil
			vars[string.format("side_store.unit[%d].hitpoints", i - 1)] = nil
			vars[string.format("side_store.unit[%d].moves", i - 1)] = nil
			vars[string.format("side_store.unit[%d].side", i - 1)] = nil
			vars[string.format("side_store.unit[%d].goto_x", i - 1)] = nil
			vars[string.format("side_store.unit[%d].goto_y", i - 1)] = nil
		end
		wml_actions.set_global_variable {
			namespace = cfg.scenario_id,
			from_local = "side_store",
			to_global =  side.save_id,
			immediate = true,
			side = "global",
		}
		vars["side_store"] = nil
		::continue::
	end
end

function wesnoth.wml_actions.persistent_carryover_unstore(cfg)
	if vars.side_number then
		-- Only do this if we begin from this chapter.
		return
	end
	for num, side in ipairs(wesnoth.sides) do
		for i = 1, #wesnoth.sides do
			local num2 = (i + num - 2) % #wesnoth.sides + 1
			if wesnoth.sides[num2].persistent then
				wml_actions.get_global_variable {
					namespace = cfg.scenario_id,
					to_local = "side_store",
					from_global =  side.save_id,
					immediate = true,
				}
			end
			if vars["side_store.gold"] then
				break
			end
		end
		for i = 1, vars["side_store.unit.length"] do
			vars[string.format("side_store.unit[%d].side", i - 1)] = num
			local u = wesnoth.units.get(vars[string.format("side_store.unit[%d].id", i - 1)])

			if u then
				vars[string.format("side_store.unit[%d].x", i - 1)] = u.x
				vars[string.format("side_store.unit[%d].y", i - 1)] = u.y
				u:extract()
			end
			wml_actions.unstore_unit {
				variable = string.format("side_store.unit[%d]", i - 1),
				find_vacant = false,
				check_passability = false,
				advance = false,
				animate = false,
			}
		end
		if vars["side_store.gold"] then
			side.gold = side.gold + vars["side_store.gold"]
		end
		vars["side_store"] = nil
	end
end
