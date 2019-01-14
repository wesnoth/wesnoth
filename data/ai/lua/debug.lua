--! #textdomain wesnoth

return {

	init = function(ai)

		if (not wesnoth.game_config.debug) then
			wesnoth.message("LuaAI Error", "The LuaAI debug library is only available in debug mode")
			return
		end

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