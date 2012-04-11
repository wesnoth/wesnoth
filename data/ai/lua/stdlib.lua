--! #textdomain wesnoth

return {

	init = function(ai)
		
		-- Initialize the cache system for LuaAI context
		local cache = wesnoth.require("ai/lua/cache.lua")
		cache.init(ai)
		
		-- Validator section
		function ai.dst_src_validator()
			if not ai.is_dst_src_valid() then
				ai.cache["dstsrc"] = nil
				return false
			end
			
			return true
		end
		
		function ai.dst_src_enemy_validator() 
			if not ai.is_dst_src_enemy_valid() then
				ai.cache["enemy_dstsrc"] = nil				
				return false
			end
			
			return true
		end
		
		function ai.src_dst_validator() 
			if not ai.is_src_dst_valid() then
				ai.cache["srcdst"] = nil				
				return false
			end
			
			return true
		end
		
		function ai.src_dst_enemy_validator() 
			if not ai.is_src_dst_enemy_valid() then
				ai.cache["enemy_srcdst"] = nil				
				return false
			end
			
			return true
		end		
		
		-- End of validator section
		
		-- Proxy function section
		
		function ai.get_dstsrc()	
			return ai.get_cached_item("dstsrc", "get_new_dstsrc", "dst_src_validator")
		end

		function ai.get_srcdst()
			return ai.get_cached_item("srcdst", "get_new_srcdst", "dst_src_enemy_validator")
		end
		
		function ai.get_enemy_dstsrc()
			return ai.get_cached_item("enemy_dstsrc", "get_new_enemy_dstsrc", "src_dst_validator")
		end

		function ai.get_enemy_srcdst()
			return ai.get_cached_item("enemy_srcdst", "get_new_enemy_srcdst", "src_dst_enemy_validator")
		end
		
		-- End of proxy function section
	end
}

