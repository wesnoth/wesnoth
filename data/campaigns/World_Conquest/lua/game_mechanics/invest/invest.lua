local on_event = wesnoth.require("on_event")
local _ = wesnoth.textdomain 'wesnoth-wc'

local wc2_invest = {}

function wc2_invest.add_items(side_num, num_items)
	local side = wesnoth.sides[side_num]
	local items_left = stringx.split(side.variables["wc2.items_left"] or "")
	local items_available = stringx.split(side.variables["wc2.items"] or "")
	for j = 1, num_items do
		local i = mathx.random(#items_left)
		table.insert(items_available, items_left[i])
		table.remove(items_left, i)
	end

	side.variables["wc2.items_left"] = table.concat(items_left, ",")
	side.variables["wc2.items"] = table.concat(items_available, ",")
end


function wc2_invest.has_items(side_num)
	local side = wesnoth.sides[side_num]
	return side.variables["wc2.items"] ~= nil
end

function wc2_invest.initialize()
	local all_items = {}
	for i,v in ipairs(wc2_artifacts.get_artifact_list()) do
		local not_available = stringx.map_split(v.not_available or "")
		if not not_available["player"] then
			table.insert(all_items, i)
		end
	end

	for side_num, side in ipairs(wesnoth.sides) do
		if wc2_scenario.is_human_side(side_num) then
			if not wc2_invest.has_items(side_num) then
				side.variables["wc2.items_left"] = table.concat(all_items, ",")
				wc2_invest.add_items(side_num, 9)
			else
				wc2_invest.add_items(side_num, 1)				
			end
		end
	end
end

on_event("prestart", function()
	wc2_invest.initialize()
end)

local function find_index(t, v)
	for i,v2 in ipairs(t) do
		if v2 == v then return i end
	end
end

function wc2_invest.do_gold()
	local side_num = wesnoth.current.side
	local side = wesnoth.sides[side_num]
	local leaders = wesnoth.units.find_on_map { side = side_num, canrecruit = true }
	side.gold = side.gold + 70
	wesnoth.wml_actions.wc2_map_supply_village { 
		x = leaders[1].x,
		y = leaders[1].y
	}
end

function wc2_invest.do_hero(t, is_local)
	local side_num = wesnoth.current.side
	local side = wesnoth.sides[side_num]
	local leaders = wesnoth.units.find_on_map { side = side_num, canrecruit = true }
	local x,y = leaders[1].x, leaders[1].y
	if t == "wc2_commander" then
		local commanders = stringx.split(side.variables["wc2.commanders"] or "")
		local i = mathx.random(#commanders)
		t = commanders[i]
		table.remove(commanders, i)
		side.variables["wc2.commanders"] = table.concat(commanders, ",")
		if is_local then
			wc2_invest_tellunit.execute(t)
		end
		wc2_heroes.place(t, side_num, x, y, true)
	elseif t == "wc2_deserter" then

		wesnoth.sides[side_num].gold = wesnoth.sides[side_num].gold + 15

		local deserters = stringx.split(side.variables["wc2.deserters"] or "")
		local i = mathx.random(#deserters)
		t = deserters[i]
		table.remove(deserters, i)
		side.variables["wc2.deserters"] = table.concat(deserters, ",")
		if is_local then
			wc2_invest_tellunit.execute(t)
		end
		wc2_heroes.place(t, side_num, x, y, false)
	else
		local heroes_available = stringx.split(side.variables["wc2.heroes"] or "")
		local i = find_index(heroes_available, t)
		if i == nil then
			error("wc2 invest: invalid pick")
		end
		table.remove(heroes_available, i)
		side.variables["wc2.heroes"] = table.concat(heroes_available, ",")
		wc2_heroes.place(t, side_num, x, y, false)
	end
	
end

function wc2_invest.do_training(t)
	local side_num = wesnoth.current.side
	wc2_training.inc_level(side_num, t)
end

function wc2_invest.do_item(t)
	local side_num = wesnoth.current.side
	local side = wesnoth.sides[side_num]
	local leaders = wesnoth.units.find_on_map { side = side_num, canrecruit = true }
	local x,y = leaders[1].x, leaders[1].y
	
	local items_available = stringx.split(side.variables["wc2.items"] or "")
	local i = find_index(items_available, tostring(t))
	if i == nil then
		error("wc2 invest: invalid item pick '" .. t .. "' (" .. type(t) ..")")
	end
	table.remove(items_available, i)
	side.variables["wc2.items"] = table.concat(items_available, ",")

	wc2_artifacts.place_item(x, y + 1, t)
end

function wc2_invest.invest()
	local side_num = wesnoth.current.side
	local side = wesnoth.sides[side_num]
	local items_available = stringx.split(side.variables["wc2.items"] or "")
	local heroes_available = stringx.split(side.variables["wc2.heroes"] or "")
	local commanders_available = stringx.split(side.variables["wc2.commanders"] or "")
	local deserters_available = stringx.split(side.variables["wc2.deserters"] or "")
	local trainings_available = wc2_training.list_available(side_num, {2,3,4,5,6})
	local gold_available = true
	for i = 1,2 do
		local is_local = false
		local res = wesnoth.sync.evaluate_single(_"WC2 Invest", function()
			is_local = true
			return wc2_show_invest_dialog {
				items_available = items_available,
				heroes_available = heroes_available,
				trainings_available = trainings_available,
				gold_available = gold_available,
				deserters_available = deserters_available,
				commanders_available = commanders_available,
			}
		end)
		if res.pick == "gold" then
			wc2_invest.do_gold()
			gold_available = nil
		elseif res.pick == "hero" then
			wc2_invest.do_hero(res.type, is_local)
			heroes_available = nil
		elseif res.pick == "training" then
			wc2_invest.do_training(res.type)
			trainings_available = nil
		elseif res.pick == "item" then
			wc2_invest.do_item(res.type)
			items_available = nil
		else
			error("wc2 invest: invalid pick , pick='" .. tostring(res.pick) .. "'.")
		end
	end
	
end

function wesnoth.wml_actions.wc2_invest(cfg)
	--disallow undoing.
	mathx.random(100)
	wc2_invest.invest()
end

return wc2_invest
