
local utils = {vwriter = {}}

function utils.split(s)
	return coroutine.wrap(function()
		local split = s:split()
		for _,sp in ipairs(split) do
			coroutine.yield(sp)
		end
	end)
end

function utils.check_key(val, key, tag, convert_spaces)
	if not val then return nil end
	if convert_spaces then
		val = tostring(val):gsub(' ', '_')
	end
	if not val:match('^[a-zA-Z0-9_]+$') then
		wml.error("Invalid " .. key .. "= in [" .. tag .. "]")
	end
	return val
end

function utils.vwriter.init(cfg, default_variable)
	local variable = cfg.variable or default_variable
	local is_explicit_index = variable[-1] == "]"
	local mode = cfg.mode or "always_clear"
	local index = 0
	-- explicit indexes behave always like "replace"
	if not is_explicit_index then
		if mode == "append" then
			index = wml.variables[variable .. ".length"]
		elseif mode ~= "replace" then
			wml.variables[variable] = nil
		end
	end
	return {
		variable = variable,
		is_explicit_index = is_explicit_index,
		index = index,
	}
end

function utils.vwriter.write(self, container)
	if self.is_explicit_index then
		wml.variables[self.variable] = container
	else
		wml.variables[string.format("%s[%u]", self.variable, self.index)] = container
	end
	self.index = self.index + 1
end

function utils.get_sides(cfg, key_name, filter_name)
	key_name = key_name or "side"
	filter_name = filter_name or "filter_side"
	local filter = wml.get_child(cfg, filter_name)
	if filter then
		if cfg[key_name] then
			wesnoth.log('warn', "ignoring duplicate side filter information (inline side=)")
		end
		return wesnoth.sides.find(filter)
	else
		return wesnoth.sides.find{side = cfg[key_name]}
	end
end

function utils.optional_side_filter(cfg, key_name, filter_name)
	key_name = key_name or "side"
	filter_name = filter_name or "filter_side"
	if cfg[key_name] == nil and wml.get_child(cfg, filter_name) == nil then
		return true
	end
	local sides = utils.get_sides(cfg, key_name, filter_name)
	for index,side in ipairs(sides) do
		if side.controller == "human" and side.is_local then
			return true
		end
	end
	return false
end

local current_exit = "none"
local scope_stack = {
	push = table.insert,
	pop = table.remove,
}

--[[ Possible exit types:
	- none - ordinary execution
	- break - exiting a loop scope
	- return - immediate termination (exit all scopes)
	- continue - jumping to the end of a loop scope
]]
function utils.set_exiting(exit_type)
	current_exit = exit_type
end

--[[ Possible scope types:
	- plain - ordinary scope, no special features; eg [command] or [event]
	- conditional - scope that's executing because of a condition, eg [then] or [else]
	- switch - scope that's part of a switch statement, eg [case] or [else]
	- loop - scope that's part of a loop, eg [do]
Currently, only "loop" has any special effects. ]]
function utils.handle_event_commands(cfg, scope_type)
	-- The WML might be modifying the currently executed WML by mixing
	-- [insert_tag] with [set_variables] and [clear_variable], so we
	-- have to be careful not to get confused by tags vanishing during
	-- the execution, hence the manual handling of [insert_tag].
	scope_type = scope_type or "plain"
	scope_stack:push(scope_type)
	local cmds = wml.shallow_literal(cfg)
	for i = 1,#cmds do
		local v = cmds[i]
		local cmd = v[1]
		local arg = v[2]
		local insert_from
		if cmd == "insert_tag" then
			cmd = arg.name
			local from = arg.variable or
				wml.error("[insert_tag] found with no variable= field")

			arg = wml.variables[from]
			if type(arg) ~= "table" then
				-- Corner case: A missing variable is replaced
				-- by an empty container rather than being ignored.
				arg = {}
			elseif string.sub(from, -1) ~= ']' then
				insert_from = from
			end
			arg = wml.tovconfig(arg)
		end
		if not string.find(cmd, "^filter") then
			local cmd_f = wesnoth.wml_actions[cmd] or
				wml.error(string.format("[%s] not supported", cmd))
			local success, error = pcall(function()
				if insert_from then
					local j = 0
					repeat
						cmd_f(arg)
						if current_exit ~= "none" then break end
						j = j + 1
						if j >= wml.variables[insert_from .. ".length"] then break end
						arg = wml.tovconfig(wml.variables[string.format("%s[%d]", insert_from, j)])
					until false
				else
					cmd_f(arg)
				end
			end)
			if not success then
				wml.error(string.format("Error occured inside [%s]: %s", cmd, error))
			end
		end
		if current_exit ~= "none" then break end
	end
	scope_stack:pop()
	if #scope_stack == 0 then
		if current_exit == "continue" and scope_type ~= "loop" then
			wml.error("[continue] found outside a loop scope!")
		end
		current_exit = "none"
	end
	return current_exit
end

--[[ Set options to preserve legacy behavior:
- Only real parentheses protect commas, not square brackets or anything else
- String spaces surrounding the commas
- Don't remove empty elements
]]
function utils.parenthetical_split(str)
	return stringx.split(str, ',', {quote_left = '(', quote_right = ')', strip_spaces = true, remove_empty = false})
end

--note: when using these, make sure that nothing can throw over the call to end_var_scope
local function start_var_scope(name)
	local var = wml.array_variables[name] --containers and arrays
	if #var == 0 then var = wml.variables[name] end --scalars (and nil/empty)
	wml.variables[name] = nil
	return var
end

local function end_var_scope(name, var)
	wml.variables[name] = nil
	if type(var) == "table" then
		wml.array_variables[name] = var
	else
		wml.variables[name] = var
	end
end

function utils.scoped_var(name)
	local orig = start_var_scope(name)
	return setmetatable({
		set = function(self, new)
			if type(new) == "table" then
				wml.array_variables[name] = new
			else
				wml.variables[name] = new
			end
		end,
		get = function(self)
			local val = wml.array_variables[name]
			if #val == 0 then val = wml.variables[name] end
			return val
		end
	}, {
		__metatable = "scoped WML variable",
		__close = function(self)
			end_var_scope(name, orig)
		end,
		__index = function(self, key)
			if key == '__original' then
				return orig
			end
		end,
		__newindex = function(self, key, val)
			if key == '__original' then
				error("scoped variable '__original' value is read-only", 1)
			end
		end
	})
end

utils.trim = wesnoth.deprecate_api('wml_utils.trim', 'stringx.trim', 1, nil, stringx.trim)
utils.parenthetical_split = wesnoth.deprecate_api('wml_utils.parenthetical_split', 'stringx.quoted_split or stringx.split', 1, nil, utils.parenthetical_split)
utils.split = wesnoth.deprecate_api('wml_utils.split', 'stringx.split', 1, nil, utils.split)
utils.start_var_scope = wesnoth.deprecate_api('wml_utils.start_var_scope', 'wml_utils.scoped_var', 1, nil, start_var_scope, 'Assign the scoped_var to a to-be-closed local variable.')
utils.end_var_scope = wesnoth.deprecate_api('wml_utils.end_var_scope', 'wml_utils.scoped_var', 1, nil, end_var_scope, 'Assign the scoped_var to a to-be-closed local variable.')

return utils
