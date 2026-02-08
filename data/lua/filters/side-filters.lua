
function wesnoth.wml_filters.side.has_unit(cfg, side)
	local search_recall = cfg.search_recall_list
	cfg = wml.literal(cfg)
	cfg.search_recall_list = nil
	cfg = wml.tovconfig(cfg)
	local filter = {side = side.side, wml.tag["and"](cfg)}
	local res = wesnoth.units.find_on_map(filter)
	if #res > 0 then return true end
	if not search_recall then return false end
	res = wesnoth.units.find_on_recall(filter)
	return #res > 0
end

function wesnoth.wml_filters.side.has_enemy(cfg, side)
	local matches = wesnoth.sides.find(cfg)
	for i = 1, #matches do
		if wesnoth.sides.is_enemy(matches[i], side) then
			return true;
		end
	end
	return false;
end

function wesnoth.wml_filters.side.has_ally(cfg, side)
	local matches = wesnoth.sides.find(cfg)
	for i = 1, #matches do
		if not wesnoth.sides.is_enemy(matches[i], side) then
			return true;
		end
	end
	return false;
end

function wesnoth.wml_filters.side.enemy_of(cfg, side)
	local matches = wesnoth.sides.find(cfg)
	if #matches == 0 then return false end
	for i = 1, #matches do
		if not wesnoth.sides.is_enemy(matches[i], side) then
			return false;
		end
	end
	return true
end

function wesnoth.wml_filters.side.allied_with(cfg, side)
	local matches = wesnoth.sides.find(cfg)
	if #matches == 0 then return false end
	for i = 1, #matches do
		if wesnoth.sides.is_enemy(matches[i], side) then
			return false;
		end
	end
	return true
end
