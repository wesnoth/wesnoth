local utils = wesnoth.require "wml-utils"

function wesnoth.wml_actions.modify_ai(cfg)
	local sides = utils.get_sides(cfg)
	local component, final
	if cfg.action == "add" or cfg.action == "change" then
		local start = string.find(cfg.path, "[a-z_]+%[[a-z0-9_*]*%]$")
		final = start and (string.find(cfg.path, '[', start, true) - 1) or -1
		start = start or string.find(cfg.path, "[^.]*$") or 1
		local comp_type = string.sub(cfg.path, start, final)
		component = wml.get_child(cfg, comp_type)
		if component == nil then
			wml.error("Missing component definition in [modify_ai]")
		end
		component = wml.parsed(component)
	end
	for _, side in ipairs(sides) do
		if cfg.action == "add" then
			side:add_ai_component(cfg.path, component)
		elseif cfg.action == "delete" or cfg.action == "try_delete" then
			side:delete_ai_component(cfg.path)
		elseif cfg.action == "change" then
			local id_start = final + 2
			local id_final = string.len(cfg.path) - 1
			local id = string.sub(cfg.path, id_start, id_final)
			if id == "*" then
				wml.error("[modify_ai] can only change one component at a time")
			elseif not component.id and not id:match("[0-9]+") then
				component.id = id
			end
			side:change_ai_component(cfg.path, component)
		end
	end
end
