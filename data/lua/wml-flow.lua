local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions

function wml_actions.command(cfg)
	utils.handle_event_commands(cfg, "plain")
end

-- we can't create functions with names that are Lua keywords (eg if, while)
-- instead, we store the following anonymous functions directly into
-- the table, using the [] operator, rather than by using the point syntax

wml_actions["if"] = function(cfg)
	if not (helper.get_child(cfg, 'then') or helper.get_child(cfg, 'elseif') or helper.get_child(cfg, 'else')) then
		helper.wml_error("[if] didn't find any [then], [elseif], or [else] children.")
	end

	if wesnoth.eval_conditional(cfg) then -- evaluate [if] tag
		for then_child in helper.child_range(cfg, "then") do
			local action = utils.handle_event_commands(then_child, "conditional")
			if action ~= "none" then break end
		end
		return -- stop after executing [then] tags
	end

	for elseif_child in helper.child_range(cfg, "elseif") do
		if wesnoth.eval_conditional(elseif_child) then -- we'll evaluate the [elseif] tags one by one
			for then_tag in helper.child_range(elseif_child, "then") do
				local action = utils.handle_event_commands(then_tag, "conditional")
				if action ~= "none" then break end
			end
			return -- stop on first matched condition
		end
	end

	-- no matched condition, try the [else] tags
	for else_child in helper.child_range(cfg, "else") do
		local action = utils.handle_event_commands(else_child, "conditional")
		if action ~= "none" then break end
	end
end

wml_actions["while"] = function( cfg )
	if helper.child_count(cfg, "do") == 0 then
		helper.wml_error "[while] does not contain any [do] tags"
	end
	
	-- execute [do] up to 65536 times
	for i = 1, 65536 do
		if wesnoth.eval_conditional( cfg ) then
			for do_child in helper.child_range( cfg, "do" ) do
				local action = utils.handle_event_commands(do_child, "loop")
				if action == "break" then
					utils.set_exiting("none")
					return
				elseif action == "continue" then
					utils.set_exiting("none")
					break
				elseif action ~= "none" then
					return
				end
			end
		else return end
	end
end

wml_actions["break"] = function(cfg)
	utils.set_exiting("break")
end

wml_actions["return"] = function(cfg)
	utils.set_exiting("return")
end

function wml_actions.continue(cfg)
	utils.set_exiting("continue")
end

wesnoth.wml_actions["for"] = function(cfg)
	if helper.child_count(cfg, "do") == 0 then
		helper.wml_error "[for] does not contain any [do] tags"
	end
	
	local loop_lim = {}
	local first
	if cfg.array ~= nil then
		if cfg.reverse then
			first = wesnoth.get_variable(cfg.array .. ".length") - 1
			loop_lim.last = 0
			loop_lim.step = -1
		else
			first = 0
			loop_lim.last = '$($' .. cfg.array .. ".length - 1)"
			loop_lim.step = 1
		end
	else
		-- Get a literal config to fetch end and step;
		-- this done is to delay expansion of variables
		local cfg_lit = helper.literal(cfg)
		first = cfg.start or 0
		loop_lim.last = cfg_lit["end"] or first
		loop_lim.step = cfg_lit.step or 1
	end
	loop_lim = wesnoth.tovconfig(loop_lim)
	if loop_lim.step == 0 then -- Sanity check
		helper.wml_error("[for] has a step of 0!")
	end
	if (first < loop_lim.last and loop_lim.step <= 0)
			or (first > loop_lim.last and loop_lim.step >= 0) then
		-- Sanity check: If they specify something like start,end,step=1,4,-1
		-- then we do nothing
		return
	end
	local i_var = cfg.variable or "i"
	local save_i = utils.start_var_scope(i_var)
	wesnoth.set_variable(i_var, first)
	local function loop_condition()
		local sentinel = loop_lim.last
		if loop_lim.step then
			sentinel = sentinel + loop_lim.step
			if loop_lim.step > 0 then
				return wesnoth.get_variable(i_var) < sentinel
			else
				return wesnoth.get_variable(i_var) > sentinel
			end
		elseif loop_lim.last < first then
			sentinel = sentinel - 1
			return wesnoth.get_variable(i_var) > sentinel
		else
			sentinel = sentinel + 1
			return wesnoth.get_variable(i_var) < sentinel
		end
	end
	while loop_condition() do
		for do_child in helper.child_range( cfg, "do" ) do
			local action = utils.handle_event_commands(do_child, "loop")
			if action == "break" then
				utils.set_exiting("none")
				goto exit
			elseif action == "continue" then
				utils.set_exiting("none")
				break
			elseif action ~= "none" then
				goto exit
			end
		end
		wesnoth.set_variable(i_var, wesnoth.get_variable(i_var) + loop_lim.step)
	end
	::exit::
	utils.end_var_scope(i_var, save_i)
end

wml_actions["repeat"] = function(cfg)
	if helper.child_count(cfg, "do") == 0 then
		helper.wml_error "[repeat] does not contain any [do] tags"
	end
	
	local times = cfg.times or 1
	for i = 1, times do
		for do_child in helper.child_range( cfg, "do" ) do
			local action = utils.handle_event_commands(do_child, "loop")
			if action == "break" then
				utils.set_exiting("none")
				return
			elseif action == "continue" then
				utils.set_exiting("none")
				break
			elseif action ~= "none" then
				return
			end
		end
	end
end

function wml_actions.foreach(cfg)
	if helper.child_count(cfg, "do") == 0 then
		helper.wml_error "[foreach] does not contain any [do] tags"
	end
	
	local array_name = cfg.array or helper.wml_error "[foreach] missing required array= attribute"
	local array = helper.get_variable_array(array_name)
	if #array == 0 then return end -- empty and scalars unwanted
	local item_name = cfg.variable or "this_item"
	local this_item = utils.start_var_scope(item_name) -- if this_item is already set
	local i_name = cfg.index_var or "i"
	local i = utils.start_var_scope(i_name) -- if i is already set
	local array_length = wesnoth.get_variable(array_name .. ".length")
	
	for index, value in ipairs(array) do
		-- Some protection against external modification
		-- It's not perfect, though - it'd be nice if *any* change could be detected
		if array_length ~= wesnoth.get_variable(array_name .. ".length") then
			helper.wml_error("WML array length changed during [foreach] iteration")
		end
		wesnoth.set_variable(item_name, value)
		-- set index variable
		wesnoth.set_variable(i_name, index-1) -- here -1, because of WML array
		-- perform actions
		for do_child in helper.child_range(cfg, "do") do
			local action = utils.handle_event_commands(do_child, "loop")
			if action == "break" then
				utils.set_exiting("none")
				goto exit
			elseif action == "continue" then
				utils.set_exiting("none")
				break
			elseif action ~= "none" then
				goto exit
			end
		end
		-- set back the content, in case the author made some modifications
		if not cfg.readonly then
			array[index] = wesnoth.get_variable(item_name)
		end
	end
	::exit::
	
	-- house cleaning
	utils.end_var_scope(item_name, this_item)
	utils.end_var_scope(i_name, i)
	
	--[[
		This forces the readonly key to be taken literally.
		
		If readonly=yes, then this line guarantees that the array
		is unchanged after the [foreach] loop ends.
		
		If readonly=no, then this line updates the array with any
		changes the user has applied through the $this_item
		variable (or whatever variable was given in item_var).
		
		Note that altering the array via indexing (with the index_var)
		is not supported; any such changes will be reverted by this line.
	]]
	helper.set_variable_array(array_name, array)
end

function wml_actions.switch(cfg)
	local var_value = wesnoth.get_variable(cfg.variable)
	local found = false

	-- Execute all the [case]s where the value matches.
	for v in helper.child_range(cfg, "case") do
		for w in utils.split(v.value) do
			if w == tostring(var_value) then 
				local action = utils.handle_event_commands(v, "switch")
				found = true
				if action ~= "none" then goto exit end
				break
			end
		end
	end
	::exit::

	-- Otherwise execute [else] statements.
	if not found then
		for v in helper.child_range(cfg, "else") do
			local action = utils.handle_event_commands(v, "switch")
			if action ~= "none" then break end
		end
	end
end
