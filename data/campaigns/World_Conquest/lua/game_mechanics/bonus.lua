local _ = wesnoth.textdomain 'wesnoth-wc'
local on_event = wesnoth.require("on_event")
local helper = wesnoth.require("helper")

local bonus = {}
bonus.sceneries = {}

-- places a bonus point on the map.
function wesnoth.wml_actions.wc2_place_bonus(cfg)
	local x = cfg.x or wml.error("[wc2_place_bonus] missing required 'x' attribute")
	local y = cfg.y or wml.error("[wc2_place_bonus] missing required 'y' attribute")
	local scenery = cfg.scenery or wml.error("[wc2_place_bonus] missing required 'scenery' attribute")
	local c_scenery = bonus.sceneries[scenery]
	if not c_scenery then
		wml.error("[wc2_place_bonus] invalid 'scenery' attribute: ".. tostring(scenery))
		end
	local image = c_scenery.image or scenery
	bonus.place_item(x, y, image)

	-- Note: although the numbers of options passed to mathx.random_choice might depend on the langauge
	--       the number of times random is called does not (random is called even if there is
	--       only one option), so this doesn't cause OOS.
	local name1 = wc2_random_names()
	local name_options = c_scenery.names or { _"place" }
	local name2 = tostring(name_options[mathx.random(#name_options)])

	local function span_font_family(str, fam)
		return string.format("<span font-family='%s'>%s</span>", fam, str)
	end
	print("placed label", x, y)
	wesnoth.wml_actions.label {
		x = x,
		y = y,
		text = span_font_family(stringx.vformat(_ "$name's $type", {name = name1, type = name2}), "Lucida Sans Unicode")
	}
end

function bonus.place_item(x, y, image)
	if image == "campfire" then
		wesnoth.current.map[{x, y}] = "^Ecf"
		image = nil
	else
		image = image or "scenery/lighthouse.png"
	end

	wesnoth.wml_actions.item {
		x = x,
		y = y,
		image = image,
		z_order = 10,
		wml.tag.variables { wc2_is_bonus = true },
	}
end

function bonus.remove_current_item(ec)
	wc2_dropping.remove_current_item()
	--TODO: i don't think its worth to keep this code, to alos allow bonus points to use terrains instead of overlays.
    wesnoth.wml_actions.terrain {
        x = ec.x1,
		y = ec.y1,
        wml.tag["and"] {
            terrain = "*^Ecf",
        },
        terrain = "Gs",
        layer = "overlay",
    }
    wesnoth.wml_actions.item {
        x = ec.x1,
		y = ec.y1,
        image = "scenery/rubble.png",
		z_order = -10,
    }
end

-- check to be overwritten by other mods.
function bonus.can_pickup_bonus(side_num, x, y)
	return wc2_scenario.is_human_side(side_num)
end

-- callback to be overwritten by other mods.
function bonus.post_pickup(side_num, x, y)
end


-- event fired by dropping.lua
on_event("wc2_drop_pickup", function(ec)
	local item = wc2_dropping.current_item
	local side_num = wesnoth.current.side

	if not item.variables.wc2_is_bonus then
		return
	end

	if not bonus.can_pickup_bonus(side_num, ec.x1, ec.y1) then
		return
	end

	local bonus_type = item.wc2_type
	if bonus_type == nil then
		local training_chance = wml.variables.wc2_config_training_chance or 1
		local hero_chance = wml.variables.wc2_config_hero_chance or 1
		local item_chance = wml.variables.wc2_config_item_chance or 1
		local r = mathx.random(training_chance + hero_chance + item_chance)
		if r <= training_chance then
			bonus_type = 1
		elseif r <= training_chance + item_chance then
			bonus_type = 2
		else
			bonus_type = 3
		end
	end
	local bonus_subtype = item.wc2_subtype
	if bonus_type == 1 then
		if not bonus.found_training(wesnoth.current.side, bonus_subtype, ec) then
			bonus_type = mathx.random(2,3)
			bonus_subtype = nil
		end
	end
	if bonus_type == 2 then
		bonus_subtype = bonus_subtype or bonus.get_random_item()
		bonus.found_artifact(ec, tonumber(bonus_subtype))
	elseif bonus_type == 3 then
		bonus_subtype = bonus_subtype or bonus.get_random_hero(ec.x1, ec.y1)
		bonus.found_hero(ec, bonus_subtype)
	end
	bonus.post_pickup(side_num, ec.x1, ec.y1)
	assert(wc2_dropping.item_taken, "item still there")
end)

function bonus.get_random_item()
	return tonumber(wc2_utils.pick_random("wc2.random_items", wc2_artifacts.fresh_artifacts_list))
end

function bonus.get_random_hero(x, y)
	return wc2_utils.pick_random_filtered("wc2.random_heroes", wc2_era.generate_bonus_heroes, function(unittypeid)
		for _, sf in ipairs(wc2_era.spawn_filters) do
			if sf.types[unittypeid] and not wesnoth.map.matches(x, y, sf.filter_location) then
				return false
			end
		end
		return true
	end)
end

function bonus.found_artifact(ec, index)
	wesnoth.wml_actions.message {
		x = ec.x1,
		y = ec.y1,
		message = _ "Hey, I found some treasure!",
	}
	bonus.remove_current_item(ec)
	wc2_artifacts.drop_message(index)
	--local x2, y2 = wesnoth.paths.find_vacant_hex(ec.x1, ec.y1)
	local x2, y2 = ec.x1, ec.y1 + 1
	wc2_artifacts.place_item(x2, y2, index)
	return true
end

function bonus.found_hero(ec, herotype)
	local finder = wesnoth.units.get(ec.x1, ec.y1)
	wesnoth.wml_actions.message {
		x = ec.x1,
		y = ec.y1,
		message = _"Someone is here!",
	}

	bonus.remove_current_item(ec)

	local newunit = wc2_heroes.place(herotype, finder.side, ec.x1, ec.y1)

	-- hero found and unit in bonus point face each other
	wc2_utils.facing_each_other(finder, newunit)
	wc2_heroes.founddialouge(finder, newunit)
	wesnoth.wml_actions.redraw {
		clear_shroud = true,
		side = newunit.side,
	}
	return true
end

function bonus.found_training(side_num, suggested_subtype, ec)
	local traintype, amount
	if suggested_subtype then
		amount = 1
		traintype = wc2_training.trainings_left(side_num, suggested_subtype) >= amount and suggested_subtype or nil
	else
		traintype, amount = wc2_training.pick_bonus(side_num)
	end

	if traintype == nil then
		return false
	end
	wesnoth.wml_actions.message {
		x = ec.x1,
		y = ec.y1,
		message = _"Someone is here!",
	}
	bonus.remove_current_item(ec)
	wc2_training.give_bonus(side_num, ec, amount, traintype)
	return true
end

function bonus.init_data(cfg)
	local sceneries = bonus.sceneries
	local lit = wml.literal(cfg)
	for k,v in pairs(wml.get_child(lit, "str") or {}) do
		local scenery = sceneries[k] or {}
		scenery.names = v
		sceneries[k] = scenery
	end
	for k,v in pairs(wml.get_child(lit, "img") or {}) do
		local scenery = sceneries[k] or {}
		sceneries[k].image = v
		sceneries[k] = scenery
	end
end

if true then
	local sceneries = bonus.sceneries
	local strings, images = wesnoth.dofile("./bonus_point_definitions.lua")
	for k,v in pairs(strings) do
		local scenery = sceneries[k] or {}
		scenery.names = v
		sceneries[k] = scenery
	end
	for k,v in pairs(images) do
		local scenery = sceneries[k] or {}
		scenery.image = v
		sceneries[k] = scenery
	end
end

return bonus
