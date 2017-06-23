local helper = wesnoth.require "helper"
local T = helper.set_wml_tag_metatable {}

local res = {}

res.quick_4mp_leaders = function(args)
	local make_4mp_leaders_quick = wesnoth.get_variable("make_4mp_leaders_quick")
	if make_4mp_leaders_quick == nil then
		make_4mp_leaders_quick = wesnoth.game_config.mp_settings and (wesnoth.game_config.mp_settings.mp_campaign == "")
	end
	if not make_4mp_leaders_quick then
		return
	end

	local trait_quick = args[1][2]
	for i, unit in ipairs(wesnoth.get_units { canrecruit = true, T.filter_wml { max_moves = 4 } }) do
		if not unit.variables.dont_make_me_quick then
			wesnoth.add_modification(unit, "trait", trait_quick )
			unit.moves = unit.max_moves
			unit.hitpoints = unit.max_hitpoints
		end
	end
end

res.turns_over_advantage = function()
	local show_turns_over_advantage = wesnoth.get_variable("show_turns_over_advantage")
	if show_turns_over_advantage == nil then
		show_turns_over_advantage = wesnoth.game_config.mp_settings and (wesnoth.game_config.mp_settings.mp_campaign == "")
	end
	if not show_turns_over_advantage then
		return
	end
	local _ = wesnoth.textdomain "wesnoth-multiplayer"
	local function all_sides()
		local function f(s, i)
			i = i + 1
			local t = wesnoth.sides[i]
			return t and i, t
		end
		return f, nil, 0
	end
	
	local income_factor = 5
	
	local side_num = -1
	local total_score = -1
	local side_comparison = ""
	local color = nil
	for side, team in all_sides() do
		if not team.__cfg.hidden then
			local r, g, b = 255, 255, 255
			if     team.__cfg.color == 1 then r, g, b = 255,   0,   0
			elseif team.__cfg.color == 2 then r, g, b =   0,   0, 255
			elseif team.__cfg.color == 3 then r, g, b =   0, 255,   0
			elseif team.__cfg.color == 4 then r, g, b = 155,  48, 255
			elseif team.__cfg.color == 5 then r, g, b =   0,   0,   0
			elseif team.__cfg.color == 6 then r, g, b = 165,  42,  42
			elseif team.__cfg.color == 7 then r, g, b = 255, 165,   0
			elseif team.__cfg.color == 8 then r, g, b = 255, 255, 255
			elseif team.__cfg.color == 9 then r, g, b =   0, 128, 128 end
			if # wesnoth.get_units( { side = side } ) == 0 then
				side_comparison = side_comparison .. string.format( tostring( _ "<span strikethrough='true' foreground='#%02x%02x%02x'>Side %d</span>") .. "\n",
				r, g, b, side)
			else
				local income = team.total_income * income_factor
				local units = 0
				-- Calc the total unit-score here
				for i, unit in ipairs( wesnoth.get_units { side = side } ) do
					if not unit.__cfg.canrecruit then
						wesnoth.fire("unit_worth", { id = unit.id })
						units = units + wesnoth.get_variable("unit_worth")
					end
				end
				-- Up to here
				local total = units + team.gold + income
				side_comparison = side_comparison .. string.format( tostring( _ "<span foreground='#%02x%02x%02x'>Side %d</span>:  Income score = %d  Unit score = %d  Gold = %d") .. "\n" .. tostring( _ "Grand total: <b>%d</b>") .. "\n",
				r, g, b, side, income, units, team.gold, total)
				if total > total_score then
					color = string.format("#%02x%02x%02x", r, g, b)
					side_num = side
					total_score = total
				end
			end
		end
	end

	side_comparison = side_comparison .. string.format( "\n" .. tostring( _ "<span foreground='%s'>Side %d</span> has the advantage."), color, side_num)
	wesnoth.fire("message", { message = side_comparison, speaker = "narrator", image = "wesnoth-icon.png"})
end
return res
