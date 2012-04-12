--! #textdomain wesnoth

return {

	init = function(ai)
		
		-- Initialize the cache system for LuaAI context
		local cache = wesnoth.require("ai/lua/cache.lua")
		cache.init(ai)
		
		-- Validator section
		function ai.cache.dst_src_validator()
			if not ai.is_dst_src_valid() then
				ai.cache.data["dst_src"] = nil
				return false
			end
			
			return true
		end
		
		function ai.cache.dst_src_enemy_validator() 
			if not ai.is_dst_src_enemy_valid() then
				ai.cache.data["enemy_dst_src"] = nil				
				return false
			end
			
			return true
		end
		
		function ai.cache.src_dst_validator() 
			if not ai.is_src_dst_valid() then
				ai.cache.data["src_dst"] = nil				
				return false
			end
			
			return true
		end
		
		function ai.cache.src_dst_enemy_validator() 
			if not ai.is_src_dst_enemy_valid() then
				ai.cache.data["enemy_src_dst"] = nil				
				return false
			end
			
			return true
		end		
		
		-- End of validator section
		
		-- Hiding of get_new_* methods
		local to_hide = {
			[1] = "get_new_src_dst",
			[2] = "get_new_dst_src",
			[3] = "get_new_enemy_src_dst",
			[4] = "get_new_enemy_dst_src"
		}
		
		for i, v in ipairs(to_hide) do
			ai.cache[v] = ai[v]
			ai[v] = nil
		end
		
		-- End of hiding get_new_* methods
		
		-- Proxy function section
		
		function ai.get_dst_src()	
			return ai.cache.get_cached_item("dst_src", "get_new_dst_src", "dst_src_validator")
		end

		function ai.get_src_dst()
			return ai.cache.get_cached_item("src_dst", "get_new_src_dst", "dst_src_enemy_validator")
		end
		
		function ai.get_enemy_dst_src()
			return ai.cache.get_cached_item("enemy_dst_src", "get_new_enemy_dst_src", "src_dst_validator")
		end

		function ai.get_enemy_src_dst()
			return ai.cache.get_cached_item("enemy_src_dst", "get_new_enemy_src_dst", "src_dst_enemy_validator")
		end
		
		-- End of proxy function section
	end
}

