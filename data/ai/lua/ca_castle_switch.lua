-------- Castle Switch CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local function get_reachable_enemy_leaders(unit)
	-- We're cheating a little here and also find hidden enemy leaders. That's
	-- because a human player could make a pretty good educated guess as to where
	-- the enemy leaders are likely to be while the AI does not know how to do that.
	local potential_enemy_leaders = AH.get_live_units { canrecruit = 'yes',
		{ "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
	}
	local enemy_leaders = {}
	for j,e in ipairs(potential_enemy_leaders) do
		local path, cost = wesnoth.find_path(unit, e.x, e.y, { ignore_units = true, viewing_side = 0 })
		if cost < AH.no_path then
			table.insert(enemy_leaders, e)
		end
	end

	return enemy_leaders
end

local ca_castle_switch = {}

function ca_castle_switch:evaluation(cfg, data)
	local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'castle_switch'
	if AH.print_eval() then AH.print_ts('     - Evaluating castle_switch CA:') end

	if ai.aspects.passive_leader then
		-- Turn off this CA if the leader is passive
		return 0
	end

	local leader = wesnoth.get_units {
			side = wesnoth.current.side,
			canrecruit = 'yes',
			formula = '(movement_left = total_movement) and (hitpoints = max_hitpoints)'
		}[1]
	if not leader then
		-- CA is irrelevant if no leader or the leader may have moved from another CA
		data.leader_target = nil
		if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
		return 0
	end

	local cheapest_unit_cost = AH.get_cheapest_recruit_cost()

	if data.leader_target and wesnoth.sides[wesnoth.current.side].gold >= cheapest_unit_cost then
		-- make sure move is still valid
		local next_hop = AH.next_hop(leader, data.leader_target[1], data.leader_target[2])
		if next_hop and next_hop[1] == data.leader_target[1]
		and next_hop[2] == data.leader_target[2] then
			return data.leader_score
		end
	end

	local width,height,border = wesnoth.get_map_size()
	local keeps = wesnoth.get_locations {
		terrain = 'K*,K*^*,*^K*', -- Keeps
		x = '1-'..width,
		y = '1-'..height,
		{ "not", { {"filter", {}} }}, -- That have no unit
		{ "not", { radius = 6, {"filter", { canrecruit = 'yes',
			{ "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
		}} }}, -- That are not too close to an enemy leader
		{ "not", {
			x = leader.x, y = leader.y, terrain = 'K*,K*^*,*^K*',
			radius = 3,
			{ "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
		}}, -- That are not close and connected to a keep the leader is on
		{ "filter_adjacent_location", {
			terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*'
		}} -- That are not one-hex keeps
	}
	if #keeps < 1 then
		-- Skip if there aren't extra keeps to evaluate
		-- In this situation we'd only switch keeps if we were running away
		if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
		return 0
	end

	local enemy_leaders = get_reachable_enemy_leaders(leader)

	-- Look for the best keep
	local best_score, best_loc, best_turns = 0, {}, 3
	for i,loc in ipairs(keeps) do
		-- Only consider keeps within 2 turns movement
		local path, cost = wesnoth.find_path(leader, loc[1], loc[2])
		local score = 0
		-- Prefer closer keeps to enemy
		local turns = math.ceil(cost/leader.max_moves)
		if turns <= 2 then
			score = 1/turns
			for j,e in ipairs(enemy_leaders) do
				score = score + 1 / M.distance_between(loc[1], loc[2], e.x, e.y)
			end

			if score > best_score then
				best_score = score
				best_loc = loc
				best_turns = turns
			end
		end
	end

	-- If we're on a keep,
	-- don't move to another keep unless it's much better when uncaptured villages are present
	if best_score > 0 and wesnoth.get_terrain_info(wesnoth.get_terrain(leader.x, leader.y)).keep then
		local close_unowned_village = (wesnoth.get_villages {
			{ "and", {
			x = leader.x,
			y = leader.y,
			radius = leader.max_moves
			}},
			owner_side = 0
		})[1]
		if close_unowned_village then
			local score = 1/best_turns
			for j,e in ipairs(enemy_leaders) do
				-- count all distances as three less than they actually are
				score = score + 1 / (M.distance_between(leader.x, leader.y, e.x, e.y) - 3)
			end

			if score > best_score then
				best_score = 0
			end
		end
	end

	if best_score > 0 then
		local next_hop = AH.next_hop(leader, best_loc[1], best_loc[2])

		if next_hop and ((next_hop[1] ~= leader.x) or (next_hop[2] ~= leader.y)) then
			-- See if there is a nearby village that can be captured without delaying progress
			local close_villages = wesnoth.get_villages( {
				{ "and", { x = next_hop[1], y = next_hop[2], radius = leader.max_moves }},
				owner_side = 0 })
			for i,loc in ipairs(close_villages) do
				local path_village, cost_village = wesnoth.find_path(leader, loc[1], loc[2])
				if cost_village <= leader.moves then
					local dummy_leader = leader:clone()
					dummy_leader.x = loc[1]
					dummy_leader.y = loc[2]
					local path_keep, cost_keep = wesnoth.find_path(dummy_leader, best_loc[1], best_loc[2])
					local turns_from_keep = math.ceil(cost_keep/leader.max_moves)
					if turns_from_keep < best_turns
					or (turns_from_keep == 1 and wesnoth.sides[wesnoth.current.side].gold < cheapest_unit_cost)
					then
						-- There is, go there instead
						next_hop = loc
						break
					end
				end
			end
		end

		data.leader_target = next_hop

		-- if we're on a keep, wait until there are no movable units on the castle before moving off
		data.leader_score = 290000
		if wesnoth.get_terrain_info(wesnoth.get_terrain(leader.x, leader.y)).keep then
			local castle = wesnoth.get_locations {
				x = "1-"..width, y = "1-"..height,
				{ "and", {
					x = leader.x, y = leader.y, radius = 200,
					{ "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
				}}
			}
			local should_wait = false
			for i,loc in ipairs(castle) do
				local unit = wesnoth.get_unit(loc[1], loc[2])
				if (not AH.is_visible_unit(wesnoth.current.side, unit)) then
					should_wait = false
					break
				elseif unit.moves > 0 then
					should_wait = true
				end
			end
			if should_wait then
				data.leader_score = 15000
			end
		end

		if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
		return data.leader_score
	end

	if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
	return 0
end

function ca_castle_switch:execution(cfg, data)
	local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]

	if AH.print_exec() then AH.print_ts('   Executing castle_switch CA') end
	if AH.show_messages() then wesnoth.wml_actions.message { speaker = leader.id, message = 'Switching castles' } end

	AH.checked_move(ai, leader, data.leader_target[1], data.leader_target[2])
	data.leader_target = nil
end

return ca_castle_switch
