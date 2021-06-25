local on_event = wesnoth.require("on_event")
local _ = wesnoth.textdomain 'wesnoth-wc'

local wc2_era = {}
wc2_era.factions_wml = {}
wc2_era.standard_factions = {}
wc2_era.hero_types = {}
wc2_era.hero_traits = {}
wc2_era.spawn_filters = {}

local strings = {
	info_menu = _"Tell me how my recruit works",
	info_recruit = _"Every time you recruit a unit, it is replaced in your recruit list by its pair. The following pairs can also be found here:",
}

local images = {
	menu_recruit_info = "icons/action/editor-tool-unit_25.png~CROP(3,4,18,18)~GS()"
}

-- the wc2 recruit pair logic.
on_event("recruit", function(ctx)
	local unit = wesnoth.units.get(ctx.x1, ctx.y1)

	local side_num = unit.side
	local side = wesnoth.sides[side_num]
	local unittype = unit.type

	for i,v in ipairs(wml.array_access.get("wc2.pair", side)) do
		local p = stringx.split(v.types or "")
		if p[1] == unittype and p[2] ~= nil then
			wesnoth.wml_actions.disallow_recruit {
				side = side_num,
				type = p[1],
			}
			wesnoth.wml_actions.allow_recruit {
				side = side_num,
				type = p[2],
			}
			p[1],p[2] = p[2],p[1]
			side.variables["wc2.pair[" .. (i - 1) .. "].types"] = table.concat(p, ",")
			wesnoth.allow_undo(false)
			return
		end
	end
end)

function wc2_era.get_faction(id)
	if type(id) == "number" then
		local side = wesnoth.sides[id]
		id = side.variables["wc2.faction_id"] or side.faction
	end
	for i, faction in ipairs(wc2_era.factions_wml) do
		if faction.id == id then
			return faction
		end
	end
end


local function init_side(side_num)

	if wesnoth.sides[side_num].variables["wc2.faction_id"] ~= nil then
		-- don't do this twice.
		return
	end
	local side = wesnoth.sides[side_num]
	local faction = wc2_era.get_faction(side_num)

	if faction and side.variables["wc2.pair.length"] == 0 and wml.get_child(faction, "pair") then
		side.variables["wc2.faction_id"] = faction.id
		wesnoth.wml_actions.disallow_recruit { side = side_num, recruit="" }
		local i = 0
		for v in wml.child_range(faction, "pair") do
			i = i + 1
			local p = stringx.split(v.types or "")
			if mathx.random(1,2) == 2 then
				p[1],p[2] = p[2],p[1]
			end
			wesnoth.wml_actions.allow_recruit {
				side = side_num,
				type = p[1],
			}
			side.variables["wc2.pair[" .. (i - 1) .. "].types"] = table.concat(p, ",")
		end
	end

	if not faction and #wc2_era.factions_wml > 0 then
		faction = wc2_era.factions_wml[mathx.random(#wc2_era.factions_wml)]
	end

	if not faction then
		faction = wc2_era.create_random_faction()
	end

	local heroes = wc2_era.expand_hero_types(faction.heroes)
	local deserters = wc2_era.expand_hero_types(faction.deserters)
	local commanders = wc2_era.expand_hero_types(faction.commanders)

	mathx.shuffle(heroes)
	mathx.shuffle(deserters)
	mathx.shuffle(commanders)

	side.variables["wc2.heroes"] = table.concat(heroes, ",")
	side.variables["wc2.deserters"] = table.concat(deserters, ",")
	side.variables["wc2.commanders"] = table.concat(commanders, ",")
end

local function add_known_faction(cfg)
	table.insert(wc2_era.factions_wml, cfg)
end

local function add_known_hero_group(id, cfg)
	wc2_era.hero_types[id] = cfg
end

local function add_known_spawn_filter(spawn_filter)
	local types = stringx.map_split(spawn_filter.types or "")
	local filter_location = wml.get_child(spawn_filter, "filter_location") or wml.error("missing [filter_location] in [hero_spawn_filter]")
	table.insert(wc2_era.spawn_filters, { types = types, filter_location = filter_location} )
end

local function add_known_trait_extra(trait_extra)
	local types = stringx.map_split(trait_extra.types or "")
	local trait = wml.get_child(trait_extra, "trait") or wml.error("missing [trait] in [trait_extra]")
	table.insert(wc2_era.hero_traits, { types = types, trait = trait} )
end
-- in case that a [multiplayer_side] has not [world_conquest_data] we generate it randomly.
function wc2_era.create_random_faction(id)

	local deserters_set = {}
	local heroes_set = {}
	local commanders_set = {}

	local i_deserters1 = mathx.random(#wc2_era.standard_factions)
	local i_deserters2 = mathx.random(#wc2_era.standard_factions)
	local i_heroes1 = mathx.random(#wc2_era.standard_factions)
	local i_heroes2 = mathx.random(#wc2_era.standard_factions)
	local i_commanders = mathx.random(#wc2_era.standard_factions)

	deserters_set = stringx.map_split(wc2_era.standard_factions[i_deserters1].recruits .. ',' .. wc2_era.standard_factions[i_deserters2].recruits)
	heroes_set = stringx.map_split(wc2_era.standard_factions[i_heroes1].recruits .. ',' .. wc2_era.standard_factions[i_heroes2].recruits)
	commanders_set = stringx.map_split(wc2_era.standard_factions[i_commanders].recruits or "")

	local faction = {
		id = "custom_random",
		deserters = table.concat( wc2_utils.set_to_array(deserters_set), ","),
		heroes = table.concat( wc2_utils.set_to_array(heroes_set), ","),
		commanders = table.concat( wc2_utils.set_to_array(commanders_set), ","),
	}
	return faction
end

function wc2_era.read_era_tag(era_wml)
	era_wml = wml.literal(era_wml)

	for  multiplayer_side in wml.child_range(era_wml, "multiplayer_side") do
		if not multiplayer_side.random_faction then
			--used for random faction generation.
			table.insert(wc2_era.standard_factions, {
				id = multiplayer_side.id,
				recruits = multiplayer_side.recruit
			})
		end
		local faction = wml.get_child(multiplayer_side, "world_conquest_data")
		if faction then
			faction.id = multiplayer_side.id
			faction.name = multiplayer_side.name
			faction.image = multiplayer_side.image
			add_known_faction(faction)
		end
	end
	-- No need to read [hero_types] since wc2_utils.get_wc2_data also reads from [era] 
end

function wc2_era.init_era_default()
	wc2_era.read_era_tag(wesnoth.scenario.era)
	wc2_era.init_data()
end

function wc2_era.init_data()
	
	for i,v in ipairs(wml.get_child(wc2_utils.get_wc2_data("hero_types"), "hero_types")) do
		add_known_hero_group(v[1], v[2])
	end

	for trait_extra in wml.child_range(wc2_utils.get_wc2_data("trait_extra"), "trait_extra") do
		add_known_trait_extra(trait_extra)
	end

	for spawn_filter in wml.child_range(wc2_utils.get_wc2_data("hero_spawn_filter"), "hero_spawn_filter") do
		add_known_spawn_filter(spawn_filter)
	end
end


function wesnoth.wml_actions.wc2_init_era(cfg)
	cfg = wml.literal(cfg)

	if cfg.clear then
		wc2_era.factions_wml = {}
		wc2_era.hero_types = {}
		wc2_era.hero_traits = {}
		wc2_era.spawn_filters = {}
	end

	wc2_era.wc2_era_id = cfg.wc2_era_id -- TODO removed for testing or error("missing wc2_era_id")
	for faction in wml.child_range(cfg, "faction") do
		add_known_faction(faction)
	end

	for i,v in ipairs(wml.get_child(cfg, "hero_types")) do
		add_known_hero_group(v[1], v[2])
	end

	for trait_extra in wml.child_range(cfg, "trait_extra") do
		add_known_trait_extra(trait_extra)
	end

	for spawn_filter in wml.child_range(cfg, "hero_spawn_filter") do
		add_known_spawn_filter(spawn_filter)
	end

end

-- picks a deserter for the side @a side_num using the list of posibel deserters for that sides faction.
function wc2_era.pick_deserter(side_num)
	local side_variables = wesnoth.sides[side_num].variables
	local deserters = stringx.split(side_variables["wc2.deserters"] or "")
	if #deserters == 0 then
		return nil
	end
	-- fixme: shouldn't we randomoize this?
	local index = #deserters
	local res = deserters[index]
	table.remove(deserters, index)
	side_variables["wc2.deserters"] = table.concat(deserters, ",")
	return res
end

-- replaces group ids with the corresponding unit lists.
-- @a types_str a comma seperated list of unti types and group ids.
-- @returns an array of unit types.
function wc2_era.expand_hero_types(types_str)
	local types = stringx.split(types_str or "")
	local types_new = {}
	local types_res = {}
	while #types > 0 do
		for i,v in ipairs(types) do
			if wesnoth.unit_types[v] then
				table.insert(types_res, v)
			elseif wc2_era.hero_types[v] then
				local group =  wc2_era.hero_types[v]
				local these_types = stringx.split(group.types or "")
				for j,type in ipairs(these_types) do
					table.insert(types_new, type)
				end
			else
				wesnoth.interface.add_chat_message("WCII ERROR", "unknown deserter group: '" .. v .. "'")
			end
		end
		types = types_new
		types_new = {}
	end
	wc2_utils.remove_duplicates(types_res)
	return types_res
end

-- replaces group ids with the corresponding unit lists.and replaces unit type ids with their names. (used by wocopedia)
-- @a types_str a comma seperated list of unti types and group ids.
-- @returns an array of unit type names.
function wc2_era.expand_hero_names(types_str, only_unitnames)
	local types = stringx.split(types_str or "")
	local types_new = {}
	local names_res = {}
	while #types > 0 do
		for i,v in ipairs(types) do
			local ut = wesnoth.unit_types[v]
			if ut then
				table.insert(names_res, ut.name)
			else
				local group = wc2_era.hero_types[v]
				if group.name and not only_unitnames then
					table.insert(names_res, group.name)
				else
					local these_types = stringx.split(group.types or "")
					for j,type in ipairs(these_types) do
						table.insert(types_new, type)
					end
				end
			end
		end
		types = types_new
		types_new = {}
	end
	return names_res
end

on_event("prestart", function()
	for i, s in ipairs(wesnoth.sides) do
		init_side(i)
	end
end)

-- Generates the list of possible unit types that can be found in bonus points.
function wc2_era.generate_bonus_heroes()
	return wc2_era.expand_hero_types("Bonus_All")
end

-- shows the recruit info dialog for the faction of the currently viewing side.
function wesnoth.wml_actions.wc2_recruit_info(cfg)

	local side_num = wesnoth.interface.get_viewing_side()
	local faction = wc2_era.get_faction(side_num)
	if not faction then
		wesnoth.wml_actions.message {
			scroll = false,
			canrecruit = true,
			side = side_num,
			message = _ "You are not using a WC2 faction.",
		}
		return
	end
	local message = {
		scroll = false,
		canrecruit = true,
		side = side_num,
		caption = faction.name,
		message = cfg.message,
	}

	for i,v in ipairs(wml.array_access.get("wc2.pair", side_num)) do
		local p = stringx.split(v.types or "")
		local ut1 = wesnoth.unit_types[p[1]]
		local ut2 = wesnoth.unit_types[p[2]]
		local img = "misc/blank.png~SCALE(144,72)" ..
			"~BLIT(" .. wc2_color.tc_image(side_num, ut1.image) .. ")" ..
			"~BLIT(" .. wc2_color.tc_image(side_num, ut2.image) .. ",72,0)"
		table.insert(message, {"option", {
			image = img,
			label= ut1.name,
			description = ut2.name,
		}})
	end
	wesnoth.wml_actions.message(message)
end

wc2_utils.menu_item {
	id="1_WC_II_Era_Info_Recruit_Option",
	description=strings.info_menu,
	image=images.menu_recruit_info,
	synced=false,
	filter = function (x, y)
		local u wesnoth.units.get(x, y)
		if u and u.side == wesnoth.current.side then
			return false
		end
		if not wc2_era.get_faction(wesnoth.interface.get_viewing_side()) then
			return false
		end
		if wc2_artifacts.is_item_at(x, y) then
			return false
		end
		return true
	end,
	handler = function(cx)
		wesnoth.wml_actions.wc2_recruit_info {
			message = strings.info_recruit
		}
	end
}

wc2_era.init_era_default()

return wc2_era
