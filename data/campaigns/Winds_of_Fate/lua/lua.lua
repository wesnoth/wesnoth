-- from multiplayer/eras.lua, slightly modified for use in a campaign and to return a result

local function determine_advantage()
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
	local total_score = -1
	local side_comparison = ""
	local winners_color = "#000000"
	for side, team in all_sides() do
		if not team.__cfg.hidden then
			local side_color = wesnoth.colors[team.color].pango_color
			if # wesnoth.units.find_on_map( { side = side } ) == 0 then
				-- po: In the end-of-match summary, a side which has no units left and therefore lost. In English the loss is shown by displaying it with the text struck through.
				local side_text = _ "<span strikethrough='true' foreground='$side_color'>Side $side_number</span>:  Has lost all units"
				-- The double new-line here is to balance with the other sides getting a line for "Grand total"
				side_comparison = side_comparison .. side_text:vformat{side_color = side_color, side_number = side} .. "\n\n"
			else
				local income = team.total_income * income_factor
				local units = 0
				-- Calc the total unit-score here
				for i, unit in ipairs( wesnoth.units.find_on_map { side = side } ) do
					if not unit.__cfg.canrecruit then
						wml.fire("unit_worth", { id = unit.id })
						units = units + wml.variables["unit_worth"]
					end
				end
				-- Up to here
				local total = units + team.gold + income
				-- po: In the end-of-match summary, any side that still has units left
				local side_text = _ "<span foreground='$side_color'>Side $side_number</span>:  Income score = $income  Unit score = $units  Gold = $gold\nGrand total: <b>$total</b>"
				side_comparison = side_comparison .. side_text:vformat{side_color = side_color, side_number = side, income = income, units = units, gold = team.gold, total = total} .. "\n"
				if total > total_score then
					winners_color = side_color
					winning_sides = {side}
					total_score = total
				elseif total == total_score then
					table.insert(winning_sides, side)
				end
			end
		end
	end

	local result = nil

	if #winning_sides == 1 then
		-- po: In the end-of-match summary, there's a single side that's won.
		local comparison_text = _ "<span foreground='$side_color'>Side $side_number</span> has the advantage."
		side_comparison = side_comparison .. "\n" .. comparison_text:vformat{side_number = winning_sides[1], side_color = winners_color}
		result = winning_sides[1]
	else -- #winning_sides == 2, a tie (or both sides have no units or a negative score which should be impossible here)
		-- po: In the end-of-match summary, there's a two-way tie (this is only used for exactly two winning teams)
		local comparison_text = _ "Sides $side_number and $other_side_number are tied."
		side_comparison = side_comparison .. "\n" .. comparison_text:vformat{side_number = winning_sides[1], other_side_number = winning_sides[2]}
		result = "tie"
	end
	-- po: "Turns Over", meaning "turn limit reached" is the title of the end-of-match summary dialog
	local a, b = gui.show_popup(_ "dialog^Turns Over", side_comparison)
	return result
end

return determine_advantage()
