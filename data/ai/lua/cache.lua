--! #textdomain wesnoth

return {
	init = function(ai)
		
		ai.cache = {}

		function ai.update_cache(item, getter)
			ai.cache[item] = ai[getter]()
			return ai.cache[item]
		end

		function ai.get_cached_item(item, getter, validator)
			if not ai.cache[item] or not ai[validator]() then
				local result = ai.update_cache(item, getter)
				return result
			end
			return ai.cache[item]

		end
	end

}