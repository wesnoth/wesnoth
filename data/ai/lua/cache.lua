--! #textdomain wesnoth

return {
	init = function(ai)

		ai.cache = {}
		ai.cache.data = {}

		function ai.cache.update_cache(item, getter)
			ai.cache.data[item] = ai.cache[getter]()
			return ai.cache.data[item]
		end

		function ai.cache.get_cached_item(item, getter, validator)
			if not ai.cache.data[item] or not ai.cache[validator]() then
				local result = ai.cache.update_cache(item, getter)
				return result
			end
			return ai.cache.data[item]

		end
	end

}