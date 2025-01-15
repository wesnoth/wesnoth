--[========[Plugin module]========]

---@alias plugin_idle_function fun(ctx_name:string, events:WMLTable):boolean|nil

if wesnoth.kernel_type() == "Application Lua Kernel" then
	print("Loading plugin module...")

	---Yields control back to the game until the next slice.
	---@return WMLTable
	---@return plugin_context
	---@return plugin_info
	function wesnoth.plugin.next_slice()
		return coroutine.yield()
	end

	---@param cond fun(info:plugin_info):boolean
	---@param idle plugin_idle_function
	---@return WMLTable
	---@return plugin_context
	---@return plugin_info
	local function wait_until(cond, idle)
		local events, context, info = {}, nil, nil
		repeat
			local new_events
			new_events, context, info = wesnoth.plugin.next_slice()
			for i = 1, #new_events do
				events[#events + 1] = new_events[i]
			end
			if idle then
				if idle(info.name, events) then
					break
				end
			end
			if info.name == 'Dialog' then
				context.skip_dialog{}
			end
		until cond(info)
		return events, context, info
	end

	---Waits until the plugin reaches a specified context.
	---Unless the idle function returns true, the context returned from this function is
	---guaranteed to have the expected value.
	---@param ctx_name string The context to wait for.
	---@param idle plugin_idle_function A function that will be called on each slice, taking as argument the events since the previous slice, and the name of the latest context. It can return true to break out of the wait.
	---@return WMLTable #All the events that occurred while waiting
	---@return plugin_context #The most recent context
	---@return plugin_info #The most recent info
	function wesnoth.plugin.wait_until(ctx_name, idle)
		return wait_until(function(info) return info.name == ctx_name end, idle)
	end

	---Waits until the plugin reaches one of several specified contexts.
	---Unless the idle function returns true, the context returned from this function is
	---guaranteed to have the expected value.
	---@param ctx string[] The contexts to wait for.
	---@param idle plugin_idle_function A function that will be called on each slice, taking as argument the events since the previous slice, and the name of the latest context. It can return true to break out of the wait.
	---@return WMLTable #All the events that occurred while waiting
	---@return plugin_context #The most recent context
	---@return plugin_info #The most recent info
	function wesnoth.plugin.wait_until_any(ctx, idle)
		return wait_until(function(info)
			for i = 1, #ctx do
				if info.name == ctx[i] then return true end
			end
			return false
		end, idle)
	end

	---Waits until the plugin reaches a specified context.
	---Unless the idle function returns true, the context returned from this function is
	---guaranteed to have the expected value.
	---@param ctx string The context to wait for.
	---@param idle plugin_idle_function A function that will be called on each slice, taking as argument the events since the previous slice, and the name of the latest context. It can return true to break out of the wait.
	---@return WMLTable #All the events that occurred while waiting
	---@return plugin_context #The most recent context
	---@return plugin_info #The most recent info
	function wesnoth.plugin.wait_until_not(ctx, idle)
		return wait_until(function(info) return info.name ~= ctx end, idle)
	end
end
