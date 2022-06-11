local helper = wesnoth.require("helper")
local T = wml.tag
local _ = wesnoth.textdomain 'wesnoth-wc'

local wc2_heroes = {}
-- an array of wml tables, usually containing type,
wc2_heroes.commander_overlay = "misc/wct-commander.png"
wc2_heroes.hero_overlay = "misc/hero-icon.png"
wc2_heroes.dialogues = {}
wc2_heroes.trait_heroic = nil
wc2_heroes.trait_expert = nil

if filesystem.have_file("./unittypedata.lua") then
	local data = wesnoth.require("./unittypedata.lua")
	for v,k in pairs(data) do
		wc2_heroes.dialogues[v] = k
	end
end

function wc2_heroes.find_dialogue(t)
	return wc2_heroes.dialogues[t] or wc2_heroes.dialogues.default
end

function wc2_heroes.init_data()
	local cfg_heroic = wc2_utils.get_wc2_data("trait_heroic")
	local cfg_expert = wc2_utils.get_wc2_data("trait_expert")
	wc2_heroes.trait_heroic = wml.get_child(wml.get_child(cfg_heroic, "trait_heroic"), "trait")
	wc2_heroes.trait_expert = wml.get_child(wml.get_child(cfg_expert, "trait_expert"), "trait")
end

function wc2_heroes.commander_overlay_object()
	return wml.tag.object {
		id = "wc2_commander_overlay",
		wml.tag.effect {
			apply_to="overlay",
			add = wc2_heroes.commander_overlay
		}
	}
end

function wc2_heroes.hero_overlay_object()
	return wml.tag.object {
		id = "wc2_hero_overlay",
		wml.tag.effect {
			apply_to="overlay",
			add = wc2_heroes.hero_overlay
		}
	}
end
-- @a t the unit type id
-- @returns the content of [modifications] for a unit.
function wc2_heroes.generate_traits(t)
	local res = {}

	if wc2_heroes.trait_heroic then
		table.insert(res, wml.tag.trait (wc2_heroes.trait_heroic))
	end
	for k,v in ipairs(wc2_era.hero_traits) do
		if v.types[t] then
			table.insert(res, wml.tag.trait (v.trait))
		end
	end
	return res
end

-- @a t the unit type
function wc2_heroes.place(t, side, x, y, is_commander)
	--print("wc2_heroes.place type=" .. t .. " side=" .. side)

	local modifications = wc2_heroes.generate_traits(t)
	table.insert(modifications, 1, wml.tag.advancement { wc2_scenario.experience_penalty() })

	table.insert(
		modifications,
		is_commander and wc2_heroes.commander_overlay_object() or wc2_heroes.hero_overlay_object()
	)
	local u = wesnoth.units.create {
		type = t,
		side = side,
		random_traits = false,
		role = is_commander and "commander" or nil,
		wml.tag.modifications (modifications),
	}
	if is_commander then
		u.variables["wc2.is_commander"] = true
	end
	local x2,y2 = wesnoth.paths.find_vacant_hex(x, y, u)
	u:to_map(x2,y2)
	return u
end

function wesnoth.wml_actions.wc2_random_hero(cfg)
	local side_num = cfg.side or wml.error("missing side= attribute in [wc2_initial_hero]")
	local x = cfg.x or wml.error("missing x= attribute in [wc2_initial_hero]")
	local y = cfg.y or wml.error("missing y= attribute in [wc2_initial_hero]")
	local t = wc2_era.pick_deserter(side_num)

	if t == nil then
		print("No serserter available for side", side_num)
		return
	end
	wc2_heroes.place(t, side_num, x, y)
end

-- prints the dialoge when @finder finds @found from a unit type, both parameters are lua unit objects.
function wc2_heroes.founddialouge(finder, found)
	local type_dialogue = wc2_heroes.find_dialogue(found.type)
	wesnoth.wml_actions.message {
		id = found.id,
		message = type_dialogue.founddialogue,
	}
	local reply = type_dialogue.reply or wc2_heroes.dialogues.default.reply

	for i, alt_replay in ipairs(type_dialogue.alt_reply or {}) do
		local function matches(attr)
			return string.match(alt_replay[attr] or "", finder[attr])
		end
		if matches("race") or matches("gender") or matches("type") then
			reply = alt_replay.reply
		end
	end
	wesnoth.wml_actions.message {
		id = finder.id,
		message = reply,
	}
end

wc2_heroes.init_data()

return wc2_heroes
