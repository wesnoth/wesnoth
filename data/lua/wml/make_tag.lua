local helper = wesnoth.require("lua/helper.lua")
local T = helper.set_wml_tag_metatable {}

-- a map string -> vconfig
local wml_defined_tags = {}

local add_new_wml_tag = function(tagname, command)
	if type(command) == "table" then
		command = wesnoth.tovconfig(command)
	end
	wml_defined_tags[tagname] = command
	wesnoth.wml_actions[tagname] = function(args)
		local old_args = wesnoth.get_variable("a")
		wesnoth.set_variable("a", args)
		wesnoth.wml_actions.command(command)
		wesnoth.set_variable("a", old_args)
	end
	
end

local old_on_save = wesnoth.game_events.on_save
wesnoth.game_events.on_save = function()
	local cfg = old_on_save()
	for k,v in pairs(wml_defined_tags) do
		table.insert(cfg, T.wml_defined_tag {
			tagname = k,
			T.command(helper.literal(v)),
		})
	end
	return cfg
end

local old_on_load = wesnoth.game_events.on_load
wesnoth.game_events.on_load = function(cfg)
	for i = #cfg,1,-1 do
		local v = cfg[i]
		if v[1] == "wml_defined_tag" then
			local v2 = v[2]
			add_new_wml_tag(v2.tagname, helper.get_child(v2, "command") )
			table.remove(cfg, i)
		end
	end
	old_on_load(cfg)
end

wesnoth.wml_actions.make_tag = function(cfg)
	local tagname = cfg.tagname or helper.wml_error "[make_tag] missing required tagname= attribute."
	local command = helper.get_child(cfg, "command") or helper.wml_error "[make_tag] missing required command= child."
	add_new_wml_tag(tagname, command)
end


