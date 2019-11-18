-- from multiplayer/eras.lua, slightly modified

local res = {}

res.turns_over_advantage = function()
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

	local winning_sides = {}
	local tie = true
	local total_score = -1
	local side_comparison = ""
	local color = "#000000"
	for side, team in all_sides() do
		if not team.__cfg.hidden then
			local r, g, b = 255, 255, 255
			if     team.__cfg.color == 1 then r, g, b = 255,   0,   0
			elseif team.__cfg.color == 2 then r, g, b =   0,   0, 255 end
			if # wesnoth.units.find_on_map( { side = side } ) == 0 then
				side_comparison = side_comparison .. string.format( tostring( _ "<span strikethrough='true' foreground='#%02x%02x%02x'>Side %d</span>") .. "\n",
				r, g, b, side)
			else
				local income = team.total_income * income_factor
				local units = 0
				-- Calc the total unit-score here
				for i, unit in ipairs( wesnoth.units.find_on_map { side = side } ) do
					if not unit.__cfg.canrecruit then
						wesnoth.fire("unit_worth", { id = unit.id })
						units = units + wml.variables["unit_worth"]
					end
				end
				-- Up to here
				local total = units + team.gold + income
				side_comparison = side_comparison .. string.format( tostring( _ "<span foreground='#%02x%02x%02x'>Side %d</span>:  Income score = %d  Unit score = %d  Gold = %d") .. "\n" .. tostring( _ "Grand total: <b>%d</b>") .. "\n",
				r, g, b, side, income, units, team.gold, total)
				if total > total_score then
					color = string.format("#%02x%02x%02x", r, g, b)
					winning_sides = {side}
					tie = false
					total_score = total
				elseif total == total_score then
					table.insert(winning_sides, side)
					tie = true
				end
			end
		end
	end

	local result = nil
	if tie then
		side_comparison = side_comparison .. string.format( "\n" .. tostring( _ "Sides %s and %d are tied."), 1 , 2)
		result = "tie"
	else
		side_comparison = side_comparison .. string.format( "\n" .. tostring( _ "<span foreground='%s'>Side %d</span> has the advantage."), color, winning_sides[1])
		result = winning_sides[1]
	end
	wesnoth.fire("message", { message = side_comparison, speaker = "narrator", image = "wesnoth-icon.png"})
	return result
end
return res
