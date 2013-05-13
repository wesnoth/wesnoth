--! #textdomain wesnoth

return {
	get_ai = function(ai)		
		local my_ai = { }
		local ai_stdlib = wesnoth.require('ai/lua/stdlib.lua')
		ai_stdlib.init(ai, true)

		-- compulsory for the external CA's
		function my_ai:get_ai()
			return ai
		end
		
		return my_ai
	end
}