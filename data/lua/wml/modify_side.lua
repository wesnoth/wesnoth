local utils = wesnoth.require "wml-utils"
local T = wml.tag

local side_changes_needing_redraw = {
	'shroud', 'fog', 'reset_map', 'reset_view', 'shroud_data',
	'share_vision', 'share_maps', 'share_view',
	'color', 'flag',
}

function wesnoth.wml_actions.modify_side(cfg)
	local sides = utils.get_sides(cfg)
	for i,side in ipairs(sides) do
		if cfg.team_name then
			side.team_name = cfg.team_name
		end
		if cfg.user_team_name then
			side.user_team_name = cfg.user_team_name
		end
		if cfg.side_name then
			side.side_name = cfg.side_name
		end
		if cfg.controller then
			side.controller = cfg.controller
		end
		if cfg.defeat_condition then
			side.defeat_condition = cfg.defeat_condition
		end
		if cfg.recruit then
			side.recruit = cfg.recruit:split()
		end
		if cfg.village_support then
			side.village_support = cfg.village_support
		end
		if cfg.village_gold then
			side.village_gold = cfg.village_gold
		end
		if cfg.income then
			side.base_income = cfg.income + wesnoth.game_config.base_income
		end
		if cfg.gold then
			side.gold = cfg.gold
		end

		if cfg.hidden ~= nil then
			side.hidden = cfg.hidden
		end
		if cfg.color or cfg.flag then
			side:set_id(cfg.flag, cfg.color)
		end
		if cfg.flag_icon then
			side.flag_icon = cfg.flag_icon
		end
		if cfg.suppress_end_turn_confirmation ~= nil then
			side.suppress_end_turn_confirmation = cfg.suppress_end_turn_confirmation
		end
		if cfg.scroll_to_leader ~= nil then
			side.scroll_to_leader = cfg.scroll_to_leader
		end

		if cfg.shroud ~= nil then
			side.shroud = cfg.shroud
		end
		if cfg.reset_maps then
			wesnoth.map.remove_shroud(side.side, "all")
		end
		if cfg.fog ~= nil then
			side.fog = cfg.fog
		end
		if cfg.reset_view then
			wesnoth.map.place_fog(side.side, {}, true)
		end
		if cfg.shroud_data then
			wesnoth.map.remove_shroud(side.side, cfg.shroud_data)
		end

		if cfg.share_vision then
			side.share_vision = cfg.share_vision
		end
		-- Legacy support
		if cfg.share_view ~= nil or cfg.share_maps ~= nil then
			if cfg.share_view then
				side.share_vision = 'all'
			elseif cfg.share_maps then
				side.share_vision = 'shroud'
			else
				side.share_vision = 'none'
			end
		end

		local ai, replace_ai = {}, false
		for k, v in ipairs(cfg) do
			local tag, content = v[1], v[2]
			if tag == "ai" then
				table.insert(ai, T.ai(content))
				if content.ai_algorithm then
					replace_ai = true
				end
			elseif tag == "set_variable" then
				wesnoth.wml_actions.set_variable(v[2], side.variables)
			elseif tag == "clear_variable" then
				wesnoth.wml_actions.clear_variable(cfg, side.variables)
			elseif tag == "variables" then
				side.variables.__cfg = wml.merge(side.variables.__cfg, content, cfg.var_merge_mode or "replace")
			end
		end
		
		if cfg.switch_ai then
			side:switch_ai(cfg.switch_ai)
		end
		if #ai > 0 then
			if replace_ai then
				side:switch_ai(ai)
			else
				side:append_ai(ai)
			end
		end
	end
	for i,key in ipairs(side_changes_needing_redraw) do
		if cfg[key] ~= nil then
			wesnoth.wml_actions.redraw{}
			return
		end
	end
end
