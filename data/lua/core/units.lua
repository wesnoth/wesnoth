--[========[Units module]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
	print("Loading units module...")

	wesnoth.units.scroll_to = wesnoth.interface.scroll_to_hex

	---Modifies all the units satisfying the given filter.
	---@param filter WML A filter to select the units to modify
	---@param vars table<string, boolean|string|number> key/value pairs that need changing.
	function wesnoth.units.modify(filter, vars)
		local units = wesnoth.units.find(filter)
		for u in pairs(units) do
			for k, v in pairs(vars) do
				-- Minor TODO: What if you want to change values of subtags?
				-- Previously would've been possible with eg {['variables.some_var'] = 'some_value'}
				-- With this implementation, it's not possible.
				u[k] = v
			end
		end
	end

	---Force a unit to be rebuilt, reverting its stats to the base unit_type plus any modifications.
	---@param unit unit The unit to rebuild
	function wesnoth.units.rebuild(unit)
		unit:transform(unit.type)
	end

	---Find an attack on the unit that matches a filter
	---@param unit unit The unit to search
	---@param filter WML The filter to match
	---@return unit_weapon|nil #The matching attack
	---@return integer? #The index of the matching attack in the unit's attack list
	function wesnoth.units.find_attack(unit, filter)
		for i, atk in ipairs(unit.attacks) do
			if atk:matches(filter) then return atk, i end
		end
	end

	---Finds both map and recall list units.
	---@param filter WML A filter to select the units
	---@return unit[]
	function wesnoth.units.find(filter)
		local res = wesnoth.units.find_on_map(filter)
		for i, u in ipairs(wesnoth.units.find_on_recall(filter)) do
			table.insert(res, u)
		end
		return res
	end

	---Get the chance for the unit to be hit
	---@param unit unit The unit to test
	---@param terrain string The terrain to test for
	---@return number
	function wesnoth.units.chance_to_be_hit(unit, terrain)
		return 100 - wesnoth.units.defense_on(unit, terrain)
	end

	---@deprecated
	function wesnoth.units.resistance(...)
		return 100 - wesnoth.units.resistance_against(...)
	end

	wesnoth.match_unit = wesnoth.deprecate_api('wesnoth.match_unit', 'wesnoth.units.matches', 1, nil, wesnoth.units.matches)
	wesnoth.put_recall_unit = wesnoth.deprecate_api('wesnoth.put_recall_unit', 'wesnoth.units.to_recall', 1, nil, wesnoth.units.to_recall)
	wesnoth.put_unit = wesnoth.deprecate_api('wesnoth.put_unit', 'wesnoth.units.to_map', 1, nil, wesnoth.units.to_map)
	wesnoth.erase_unit = wesnoth.deprecate_api('wesnoth.erase_unit', 'wesnoth.units.erase', 1, nil, wesnoth.units.erase)
	wesnoth.copy_unit = wesnoth.deprecate_api('wesnoth.copy_unit', 'wesnoth.units.clone', 1, nil, wesnoth.units.clone)
	wesnoth.extract_unit = wesnoth.deprecate_api('wesnoth.extract_unit', 'wesnoth.units.extract', 1, nil, wesnoth.units.extract)
	wesnoth.advance_unit = wesnoth.deprecate_api('wesnoth.advance_unit', 'wesnoth.units.advance', 1, nil, wesnoth.units.advance)
	wesnoth.add_modification = wesnoth.deprecate_api('wesnoth.add_modification', 'wesnoth.units.add_modification', 1, nil, wesnoth.units.add_modification)
	wesnoth.remove_modifications = wesnoth.deprecate_api('wesnoth.remove_modifications', 'wesnoth.units.remove_modifications', 1, nil, wesnoth.units.remove_modifications)
	wesnoth.unit_resistance = wesnoth.deprecate_api('wesnoth.unit_resistance', 'wesnoth.units.resistance_against', 1, nil, wesnoth.units.resistance)
	wesnoth.unit_defense = wesnoth.deprecate_api('wesnoth.unit_defense', 'wesnoth.units.defense_on or wesnoth.units.chance_to_be_hit', 1, nil, wesnoth.units.chance_to_be_hit)
	wesnoth.unit_movement_cost = wesnoth.deprecate_api('wesnoth.unit_movement_cost', 'wesnoth.units.movement_on', 1, nil, wesnoth.units.movement_on)
	wesnoth.unit_vision_cost = wesnoth.deprecate_api('wesnoth.unit_vision_cost', 'wesnoth.units.vision_on', 1, nil, wesnoth.units.vision_on)
	wesnoth.unit_jamming_cost = wesnoth.deprecate_api('wesnoth.unit_jamming_cost', 'wesnoth.units.jamming_on', 1, nil, wesnoth.units.jamming_on)
	wesnoth.units.resistance = wesnoth.deprecate_api('wesnoth.units.resistance', 'wesnoth.units.resistance_against', 1, nil, wesnoth.units.resistance)
	wesnoth.units.defense = wesnoth.deprecate_api('wesnoth.units.defense', 'wesnoth.units.defense_on', 1, nil, wesnoth.units.chance_to_be_hit)
	wesnoth.units.movement = wesnoth.deprecate_api('wesnoth.units.movement', 'wesnoth.units.movement_on', 1, nil, wesnoth.units.movement_on)
	wesnoth.units.vision = wesnoth.deprecate_api('wesnoth.units.vision', 'wesnoth.units.vision_on', 1, nil, wesnoth.units.vision_on)
	wesnoth.units.jamming = wesnoth.deprecate_api('wesnoth.units.jamming', 'wesnoth.units.jamming_on', 1, nil, wesnoth.units.jamming_on)
	wesnoth.unit_ability = wesnoth.deprecate_api('wesnoth.unit_ability', 'wesnoth.units.ability', 1, nil, wesnoth.units.ability)
	wesnoth.transform_unit = wesnoth.deprecate_api('wesnoth.transform_unit', 'wesnoth.units.transform', 1, nil, wesnoth.units.transform)
	wesnoth.select_unit = wesnoth.deprecate_api('wesnoth.select_unit', 'wesnoth.units.select', 1, nil, wesnoth.units.select)
	wesnoth.create_unit = wesnoth.deprecate_api('wesnoth.create_unit', 'wesnoth.units.create', 1, nil, wesnoth.units.create)
	wesnoth.get_unit = wesnoth.deprecate_api('wesnoth.get_unit', 'wesnoth.units.get', 1, nil, wesnoth.units.get)
	wesnoth.get_units = wesnoth.deprecate_api('wesnoth.get_units', 'wesnoth.units.find_on_map', 1, nil, wesnoth.units.find_on_map)
	wesnoth.get_recall_units = wesnoth.deprecate_api('wesnoth.get_recall_units', 'wesnoth.units.find_on_recall', 1, nil, wesnoth.units.find_on_recall)
	wesnoth.create_animator = wesnoth.deprecate_api('wesnoth.create_animator', 'wesnoth.units.create_animator', 1, nil, wesnoth.units.create_animator)
	wesnoth.create_weapon = wesnoth.deprecate_api('wesnoth.create_weapon', 'wesnoth.units.create_weapon', 1, nil, wesnoth.units.create_weapon)
	wesnoth.teleport = wesnoth.deprecate_api('wesnoth.teleport', 'wesnoth.units.teleport', 1, nil, wesnoth.units.teleport)
end
