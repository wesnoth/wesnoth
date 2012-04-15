--! #textdomain wesnoth

return { -- TODO: Disable the ability to register this library
	 -- if wesnoth started without -d argument
	init = function(ai)
		
		ai.debug = {}
	
		function ai.debug.get_dst_src()
			ai.recalculate_move_maps()
			return ai.get_dst_src()
		end
		
		function ai.debug.get_src_dst()
			ai.recalculate_move_maps()
			return ai.get_src_dst()
		end
		
		function ai.debug.get_enemy_dst_src()
			ai.recalculate_enemy_move_maps()
			return ai.get_enemy_dst_src()
		end
		
		function ai.debug.get_enemy_src_dst()
			ai.recalculate_enemy_move_maps()
			return ai.get_enemy_src_dst()
		end
	
	end
}