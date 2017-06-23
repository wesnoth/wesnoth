
local helper = wesnoth.require "helper"
local utils = {vwriter = {}}

function utils.trim(s)
	-- use (f(a)) to get first argument
	return (tostring(s):gsub("^%s*(.-)%s*$", "%1"))
end

function utils.split(s)
	return tostring(s or ""):gmatch("[^%s,][^,]*")
end

function utils.check_key(val, key, tag, convert_spaces)
	if not val then return nil end
	if convert_spaces then
		val = tostring(val):gsub(' ', '_')
	end
	if not val:match('^[a-zA-Z0-9_]+$') then
		helper.wml_error("Invalid " .. key .. "= in [" .. tag .. "]")
	end
	return val
end

function utils.vwriter.init(cfg, default_variable)
	local variable = cfg.variable or default_variable
	local is_explicit_index = string.sub(variable, string.len(variable)) == "]"
	local mode = cfg.mode or "always_clear"
	local index = 0
	if is_explicit_index then
		-- explicit indexes behave always like "replace"
	elseif mode == "append" then
		index = wesnoth.get_variable(variable .. ".length")
	elseif mode ~= "replace" then
		wesnoth.set_variable(variable)
	end
	return {
		variable = variable,
		is_explicit_index = is_explicit_index,
		index = index,
	}
end

function utils.vwriter.write(self, container)
	if self.is_explicit_index then
		wesnoth.set_variable(self.variable, container)
	else
		wesnoth.set_variable(string.format("%s[%u]", self.variable, self.index), container)
	end
	self.index = self.index + 1
end

function utils.get_sides(cfg, key_name, filter_name)
	key_name = key_name or "side"
	filter_name = filter_name or "filter_side"
	local filter = helper.get_child(cfg, filter_name)
	if filter then
		if cfg[key_name] then
			wesnoth.log('warn', "ignoring duplicate side filter information (inline side=)")
		end
		return wesnoth.get_sides(filter)
	else
		return wesnoth.get_sides{side = cfg[key_name]}
	end
end

function utils.optional_side_filter(cfg, key_name, filter_name)
	local key_name = key_name or "side"
	local filter_name = filter_name or "filter_side"
	if cfg[key_name] == nil and helper.get_child(cfg, filter_name) == nil then
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
	local cmds = helper.shallow_literal(cfg)
	for i = 1,#cmds do
		local v = cmds[i]
		local cmd = v[1]
		local arg = v[2]
		local insert_from
		if cmd == "insert_tag" then
			cmd = arg.name
			local from = arg.variable or 
				helper.wml_error("[insert_tag] found with no variable= field")

			arg = wesnoth.get_variable(from)
			if type(arg) ~= "table" then
				-- Corner case: A missing variable is replaced
				-- by an empty container rather than being ignored.
				arg = {}
			elseif string.sub(from, -1) ~= ']' then
				insert_from = from
			end
			arg = wesnoth.tovconfig(arg)
		end
		if not string.find(cmd, "^filter") then
			cmd = wesnoth.wml_actions[cmd] or
				helper.wml_error(string.format("[%s] not supported", cmd))
			if insert_from then
				local j = 0
				repeat
					cmd(arg)
					if current_exit ~= "none" then break end
					j = j + 1
					if j >= wesnoth.get_variable(insert_from .. ".length") then break end
					arg = wesnoth.tovconfig(wesnoth.get_variable(string.format("%s[%d]", insert_from, j)))
				until false
			else
				cmd(arg)
			end
		end
		if current_exit ~= "none" then break end
	end
	scope_stack:pop()
	if #scope_stack == 0 then
		if current_exit == "continue" and scope_type ~= "loop" then
			helper.wml_error("[continue] found outside a loop scope!")
		end
		current_exit = "none"
	end
	return current_exit
end

-- Splits the string argument on commas, excepting those commas that occur
-- within paired parentheses. The result is returned as a (non-empty) table.
-- (The table might have a single entry that is an empty string, though.)
-- Spaces around splitting commas are stripped (as in the C++ version).
-- Empty strings are not removed (unlike the C++ version).
function utils.parenthetical_split(str)
	local t = {""}
	-- To simplify some logic, end the string with paired parentheses.
	local formatted = (str or "") .. ",()"

	-- Isolate paired parentheses.
	for prefix,paren in string.gmatch(formatted, "(.-)(%b())") do
		-- Separate on commas
		for comma,text in string.gmatch(prefix, "(,?)([^,]*)") do
			if comma == "" then
				-- We are continuing the last string found.
				t[#t] = t[#t] .. text
			else
				-- We are starting the next string.
				-- (Now that we know the last string is complete,
				-- strip leading and trailing spaces from it.)
				t[#t] = string.match(t[#t], "^%s*(.-)%s*$")
				table.insert(t, text)
			end
		end
		-- Add the parenthetical part to the last string found.
		t[#t] = t[#t] .. paren
	end
	-- Remove the empty parentheses we had added to the end.
	table.remove(t)
	return t
end

--note: when using these, make sure that nothing can throw over the call to end_var_scope
function utils.start_var_scope(name)
	local var = helper.get_variable_array(name) --containers and arrays
	if #var == 0 then var = wesnoth.get_variable(name) end --scalars (and nil/empty)
	wesnoth.set_variable(name)
	return var
end

function utils.end_var_scope(name, var)
	wesnoth.set_variable(name)
	if type(var) == "table" then
		helper.set_variable_array(name, var)
	else
		wesnoth.set_variable(name, var)
	end
end

return utils
