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
	end
}
