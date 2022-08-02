--! #textdomain wesnoth

return {

	init = function(ai, dbg)

		-- Initialize the cache system for LuaAI context
		local cache = wesnoth.require("ai/lua/cache.lua")
		cache.init(ai)

		-- Hiding of get_new_* and is_*_valid methods
		local to_hide = {
			[1] = "get_new_src_dst",
			[2] = "get_new_dst_src",
			[3] = "get_new_enemy_src_dst",
			[4] = "get_new_enemy_dst_src",
			[5] = "is_enemy_dst_src_valid",
			[6] = "is_dst_src_valid",
			[7] = "is_src_dst_valid",
			[8] = "is_enemy_src_dst_valid"
		}

		for i, v in ipairs(to_hide) do
			ai.cache[v] = ai[v]
			ai[v] = nil
		end

		-- End of hiding get_new_* methods

		-- Validator section
		function ai.cache.dst_src_validator()
			if not ai.cache.is_dst_src_valid() then
				ai.cache.data["dst_src"] = nil
				return false
			end

			return true
		end

		function ai.cache.enemy_dst_src_validator()
			if not ai.cache.is_enemy_dst_src_valid() then
				ai.cache.data["enemy_dst_src"] = nil
				return false
			end

			return true
		end

		function ai.cache.src_dst_validator()
			if not ai.cache.is_src_dst_valid() then
				ai.cache.data["src_dst"] = nil
				return false
			end

			return true
		end

		function ai.cache.enemy_src_dst_validator()
			if not ai.cache.is_enemy_src_dst_valid() then
				ai.cache.data["enemy_src_dst"] = nil
				return false
			end

			return true
		end

		-- End of validator section

		-- Proxy function section

		function ai.get_dst_src()
			return ai.cache.get_cached_item("dst_src", "get_new_dst_src", "dst_src_validator")
		end

		function ai.get_src_dst()
			return ai.cache.get_cached_item("src_dst", "get_new_src_dst", "enemy_dst_src_validator")
		end

		function ai.get_enemy_dst_src()
			return ai.cache.get_cached_item("enemy_dst_src", "get_new_enemy_dst_src", "src_dst_validator")
		end

		function ai.get_enemy_src_dst()
			return ai.cache.get_cached_item("enemy_src_dst", "get_new_enemy_src_dst", "enemy_src_dst_validator")
		end

		-- End of proxy function section

		if dbg then
			local debug = wesnoth.require("ai/lua/debug.lua")
			debug.init(ai)
		end

		-- Deprecated functions
		ai.get_aggression = wesnoth.deprecate_api('ai.get_aggression', 'ai.aspects.aggression', 2, '1.15', ai.get_aggression)
		ai.get_avoid = wesnoth.deprecate_api('ai.get_avoid', 'ai.aspects.avoid', 2, '1.15', ai.get_avoid)
		ai.get_caution = wesnoth.deprecate_api('ai.get_caution', 'ai.aspects.caution', 2, '1.15', ai.get_caution)
		ai.get_grouping = wesnoth.deprecate_api('ai.get_grouping', 'ai.aspects.grouping', 2, '1.15', ai.get_grouping)
		ai.get_leader_aggression = wesnoth.deprecate_api('ai.get_leader_aggression', 'ai.aspects.leader_aggression', 2, '1.15', ai.get_leader_aggression)
		ai.get_leader_goal = wesnoth.deprecate_api('ai.get_leader_goal', 'ai.aspects.leader_goal', 2, '1.15', ai.get_leader_goal)
		ai.get_leader_ignores_keep = wesnoth.deprecate_api('ai.get_leader_ignores_keep', 'ai.aspects.leader_ignores_keep', 2, '1.15', ai.get_leader_ignores_keep)
		ai.get_leader_value = wesnoth.deprecate_api('ai.get_leader_value', 'ai.aspects.leader_value', 2, '1.15', ai.get_leader_value)
		ai.get_passive_leader = wesnoth.deprecate_api('ai.get_passive_leader', 'ai.aspects.passive_leader', 2, '1.15', ai.get_passive_leader)
		ai.get_passive_leader_shares_keep = wesnoth.deprecate_api('ai.get_passive_leader_shares_keep', 'ai.aspects.passive_leader_shares_keep', 2, '1.15', ai.get_passive_leader_shares_keep)
		ai.get_recruitment_pattern = wesnoth.deprecate_api('ai.get_recruitment_pattern', 'ai.aspects.recruitment_pattern', 2, '1.15', ai.get_recruitment_pattern)
		ai.get_scout_village_targeting = wesnoth.deprecate_api('ai.get_scout_village_targeting', 'ai.aspects.scout_village_targeting', 2, '1.15', ai.get_scout_village_targeting)
		ai.get_simple_targeting = wesnoth.deprecate_api('ai.get_simple_targeting', 'ai.aspects.simple_targeting', 2, '1.15', ai.get_simple_targeting)
		ai.get_support_villages = wesnoth.deprecate_api('ai.get_support_villages', 'ai.aspects.support_villages', 2, '1.15', ai.get_support_villages)
		ai.get_village_value = wesnoth.deprecate_api('ai.get_village_value', 'ai.aspects.village_value', 2, '1.15', ai.get_village_value)
		ai.get_villages_per_scout = wesnoth.deprecate_api('ai.get_villages_per_scout', 'ai.aspects.villages_per_scout', 2, '1.15', ai.get_villages_per_scout)
	end
}
