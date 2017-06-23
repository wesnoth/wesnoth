local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"

function wesnoth.wml_actions.role(cfg)
	-- role= and type= are handled differently than in other tags,
	-- so we need to remove them from the filter
	local role = cfg.role
	local filter = helper.shallow_literal(cfg)

	if role == nil then
		helper.wml_error("missing role= in [role]")
	end

	local types = {}

	if cfg.type then
		for value in utils.split(cfg.type) do
			table.insert(types, utils.trim(value))
		end
	end

	filter.role, filter.type = nil, nil
	local search_map, search_recall, reassign = true, true, true
	if cfg.search_recall_list == "only" then
		search_map = false
	elseif cfg.search_recall_list ~= nil then
		search_recall = not not cfg.search_recall_list
	end
	if cfg.reassign ~= nil then
		reassign = not not cfg.reassign
	end

	-- pre-build a new [recall] from the [auto_recall]
	-- copy only recall-specific attributes, no SUF at all
	-- the SUF will be id= which we will add in a moment
	-- keep this in sync with the C++ recall function!!!
	local recall = nil
	local child = helper.get_child(cfg, "auto_recall")
	if child ~= nil then
		if helper.get_nth_child(cfg, "auto_recall", 2) ~= nil then
			wesnoth.log("debug", "More than one [auto_recall] found within [role]", true)
		end
		local original = helper.shallow_literal(child)
		recall = {}
		recall.x = original.x
		recall.y = original.y
		recall.show = original.show
		recall.fire_event = original.fire_event
		recall.check_passability = original.check_passability
	end

	if not reassign then
		if search_map then
			local unit = wesnoth.get_units{role=role}[1]
			if unit then
				return
			end
		end
		if recall and search_recall then
			local unit = wesnoth.get_recall_units{role=role}[1]
			if unit then
				recall.id = unit.id
				wesnoth.wml_actions.recall(recall)
				return
			end
		end
	end

	if search_map then
		-- first attempt to match units on the map
		local i = 1
		repeat
			-- give precedence based on the order specified in type=
			if #types > 0 then
				filter.type = types[i]
			end
			local unit = wesnoth.get_units(filter)[1]
			if unit then
				unit.role = role
				return
			end
			i = i + 1
		until #types == 0 or i > #types
	end

	if search_recall then
		-- then try to match units on the recall lists
		i = 1
		repeat
			if #types > 0 then
				filter.type = types[i]
			end
			local unit = wesnoth.get_recall_units(filter)[1]
			if unit then
				unit.role = role
				if recall then
					recall.id = unit.id
					wesnoth.wml_actions.recall(recall)
				end
				return
			end
			i = i + 1
		until #types == 0 or i > #types
	end

	-- no matching unit found, try the [else] tags
	for else_child in helper.child_range(cfg, "else") do
		local action = utils.handle_event_commands(else_child, "conditional")
		if action ~= "none" then return end
	end
end
