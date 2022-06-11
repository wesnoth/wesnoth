local _ = wesnoth.textdomain 'wesnoth-wc'
local helper = wesnoth.require("helper")
local T = wml.tag

local terrain_map = { fungus = "Tt", cave = "Ut", sand = "Dt", 
	reef = "Wrt", hills = "Ht", swamp_water = "St", shallow_water = "Wst", castle = "Ct",
	mountains = "Mt", deep_water = "Wdt", flat = "Gt", forest = "Ft", frozen = "At",
	village = "Vt", impassable = "Xt", unwalkable = "Qt", rails = "Rt"
}

-- for all attacks that match [filter_attack], it add a dublicate fo that attack and modifres is as describes in the [attack]  subtag which uses the apply_to=attack syntax
function wesnoth.effects.wc2_optional_attack(u, cfg)
	local name_suffix = cfg.name_suffix or wml.error("apply_to=wc2_optional_attack missing required name_suffix= attribute.")
	local attack_mod = wml.get_child(cfg, "attack") or wml.error("apply_to=wc2_optional_attack missing required [attack] subtag")
	local attacks_to_add = {}
	local names = {}
	for i = 1, #u.attacks do
		local attack = u.attacks[i]
		if attack:matches(wml.get_child(cfg, "filter_attack")) then
			local new_name = attack.name .. name_suffix
			local new_attack = attack.__cfg
			new_attack.name = new_name
			new_attack.apply_to = "new_attack"
			table.insert(names, new_name)
			table.insert(attacks_to_add, new_attack)
		end
	end
	for k,v in ipairs(attacks_to_add) do
		wesnoth.units.add_modification(u, "object", { T.effect ( v)}, false)
	end

	if #names > 0 then
		-- if names is empty then it would give 'name=""' which would match all attacks.
		attack_mod.apply_to = "attack"
		attack_mod.name = table.concat(names, ",")

		wesnoth.units.add_modification(u, "object", { T.effect (attack_mod) }, false)
	end
end

-- The implementation of the moves defense bonus in movement training.
function wesnoth.effects.wc2_moves_defense(u, cfg)
	wesnoth.units.add_modification(u, "object", { T.effect {
		apply_to = "defense",
		replace = false,
		T.defense {
			fungus = -u.max_moves,
			cave = -u.max_moves,
			deep_water = -u.max_moves,
			shallow_water = -u.max_moves,
			swamp_water = -u.max_moves,
			flat = -u.max_moves,
			sand = -u.max_moves,
			forest = -u.max_moves,
			hills = -u.max_moves,
			mountains = -u.max_moves,
			village = -u.max_moves,
			castle = -u.max_moves,
			frozen = -u.max_moves,
			unwalkable = -u.max_moves,
			reef = -u.max_moves,
		},
	}}, false)
end

-- Like apply_to=resistance with replace=true, but never decreases resistances.
function wesnoth.effects.wc2_min_resistance(u, cfg)
	local resistance_new = {}
	local resistance_old = wml.parsed(wml.get_child(cfg, "resistance"))
	local unit_resistance_cfg = nil
	for k,v in pairs(resistance_old) do
		if type(k) == "string" and type(v) == "number" then
			if not unit_resistance_cfg then
				unit_resistance_cfg = wml.get_child(u.__cfg, "resistance")
			end
			if unit_resistance_cfg[k] >= v then
				resistance_new[k] = v
			end
		end
	end
	wesnoth.units.add_modification(u, "object", {
		T.effect {
			apply_to = "resistance",
			replace = true,
			T.resistance (resistance_new),
		},
	}, false)
end


-- Like apply_to=defense with replace=true, but never decreases defense.
function wesnoth.effects.wc2_min_defense(u, cfg)
	local defense_new = {}
	local defense_old = wml.parsed(wml.get_child(cfg, "defense"))
	for k,v in pairs(defense_old) do
		if type(k) == "string" and type(v) == "number" and wesnoth.units.chance_to_be_hit(u, terrain_map[k] or "") >= v then
			defense_new[k] = v
		end
	end
	wesnoth.units.add_modification(u, "object", {
		T.effect {
			apply_to = "defense",
			replace = true,
			T.defense (defense_new),
		},
	}, false)
end

-- Sets the auro accordingly if unit might have multiple of illumination, darkness or forcefield abilities.
function wesnoth.effects.wc2_update_aura(u, cfg)
	local illuminates = wesnoth.units.matches(u, { ability = "illumination" } )
	local darkens = wesnoth.units.matches(u, { ability = "darkness" } )
	local forcefield = wesnoth.units.matches(u, { ability = "forcefield" } )
	local halo = ""
	if illuminates and darkens then 
		wesnoth.interface.add_chat_message("WC2", "Warning illuminates and darkens discovered on a unit")
	end
	if forcefield and illuminates then
		halo = "halo/illuminates-aura.png~R(50)"
	elseif forcefield and darkens then
		halo = "halo/darkens-aura.png~R(40)"
	elseif forcefield then
		halo = "halo/darkens-aura.png~O(65%)~R(150)"
	elseif darkens then
		halo = "halo/darkens-aura.png"
	elseif illuminates then
		halo = "halo/illuminates-aura.png"
	end
	
	wesnoth.units.add_modification(u, "object", {
		T.effect {
			apply_to = "halo",
			halo = halo,
		},
	}, false)
end

-- Similar to the usual apply_to=overlay effect but does not add overlays the the unit already has.
function wesnoth.effects.wc2_overlay(u, cfg)
	if cfg.add then
		local to_add_old = stringx.split(cfg.add or "")
		local to_add_new = {}
		local current = u.overlays
		for i1,v1 in ipairs(to_add_old) do
			local has_already = false
			for i2,v2 in ipairs(current) do
				if v2 == v1 then
					has_already = true
					break
				end
			end
			if not has_already then
				table.insert(to_add_new, v1)
			end
		end
		cfg.add = table.concat(to_add_new,",")
	end
	cfg.apply_to = "overlay"
	wesnoth.units.add_modification(u, "object", { T.effect(cfg)} , false)
end

-- Can move in same turn as when recruited/recalled
function wesnoth.effects.wc2_move_on_recruit(u, cfg)
	u.variables["mods.wc2_move_on_recruit"] = true
end

-- The implementation of this mods reduced recall costs, all units get an object with this effect.
function wesnoth.effects.wc2_recall_cost(u, cfg)
	local t = wesnoth.unit_types[u.type]
	u.recall_cost = math.min(20, t.cost + 3)
end
