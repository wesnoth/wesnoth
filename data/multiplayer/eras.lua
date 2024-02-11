local T = wml.tag

local res = {}

res.quick_4mp_leaders = function(args)
	local make_4mp_leaders_quick = wml.variables["make_4mp_leaders_quick"]
	if make_4mp_leaders_quick == nil then
		make_4mp_leaders_quick = wesnoth.scenario.mp_settings and (wesnoth.scenario.campaign == "")
	end
	if not make_4mp_leaders_quick then
		return
	end

	local trait_quick = args[1][2]
	for i, unit in ipairs(wesnoth.units.find_on_map { canrecruit = true, T.filter_wml { max_moves = 4 } }) do
		if not unit.variables.dont_make_me_quick then
			unit:add_modification("trait", trait_quick )
			unit.moves = unit.max_moves
			unit.hitpoints = unit.max_hitpoints
		end
	end
end

function res.turns_over_advantage()
	local show_turns_over_advantage = wml.variables["show_turns_over_advantage"]
	if show_turns_over_advantage == nil then
		show_turns_over_advantage = wesnoth.scenario.campaign == ""
	end
	if not show_turns_over_advantage then
		return
	end
	local winning_sides, side_results = res.calc_turns_over_advantage()
	res.show_turns_over_advantage(winning_sides, side_results)
end

---@class side_result
---@field income integer
---@field num_units integer
---@field gold integer
---@field total integer

---@alias sides_score_table table<integer, side_result|false>

---Calculate the turns over advantage.
---@param income_factor? integer Indicates how important income is in the calculation.
---@return integer[]
---@return sides_score_table
function res.calc_turns_over_advantage(income_factor)
	local function all_sides()
		local function f(s, i)
			i = i + 1
			local t = wesnoth.sides[i]
			return t and i, t
		end
		return f, nil, 0
	end

	income_factor = income_factor or 5

	local winning_sides = {}
	local total_score = -1

	---@type sides_score_table
	local side_outcomes = {}
	for side, team in all_sides() do
		if not team.__cfg.hidden then
			if # wesnoth.units.find_on_map( { side = side } ) == 0 then
				side_outcomes[side] = false
			else
				local income = team.total_income * income_factor
				local units = 0
				-- Calc the total unit-score here
				for i, unit in ipairs( wesnoth.units.find_on_map { side = side } ) do
					if not unit.__cfg.canrecruit then
						wml.fire.unit_worth{ id = unit.id }
						units = units + wml.variables.unit_worth
					end
				end
				-- Up to here
				local total = units + team.gold + income
				side_outcomes[side] = {
					income = income,
					num_units = units,
					gold = team.gold,
					total = total
				}
				if total > total_score then
					winning_sides = {side}
					total_score = total
				elseif total == total_score then
					table.insert(winning_sides, side)
				end
			end
		end
	end

	return winning_sides, side_outcomes
end

---Show the turns over advantage popup.
---@param winning_sides integer[] The list of sides who tied for first place
---@param side_results sides_score_table The table of each side's score calculations
---@param title? tstring The title to display in the popup
function res.show_turns_over_advantage(winning_sides, side_results, title)
	local _ = wesnoth.textdomain "wesnoth-multiplayer"
	---@type tstring
	local side_comparison = ""
	for side = 1, #wesnoth.sides do
		local outcome = side_results[side]
		local side_color = wesnoth.colors[wesnoth.sides[side].color].pango_color
		if outcome == false then
			-- po: In the end-of-match summary, a side which has no units left and therefore lost. In English the loss is shown by displaying it with the text struck through.
			local side_text = _ "<span strikethrough='true' foreground='$side_color'>Side $side_number</span>:  Has lost all units"
			-- The double new-line here is to balance with the other sides getting a line for "Grand total"
			side_comparison = side_comparison .. side_text:vformat{side_color = side_color, side_number = side} .. "\n\n"
		elseif outcome ~= nil then
			-- po: In the end-of-match summary, any side that still has units left
			local side_text = _ "<span foreground='$side_color'>Side $side_number</span>:  Income score = $income  Unit score = $units  Gold = $gold\nGrand total: <b>$total</b>"
			side_comparison = side_comparison .. side_text:vformat{side_color = side_color, side_number = side, income = outcome.income, units = outcome.num_units, gold = outcome.gold, total = outcome.total} .. "\n"
		end
	end

	if #winning_sides == 1 then
		local side = winning_sides[1]
		local side_color = wesnoth.colors[wesnoth.sides[side].color].pango_color
		-- po: In the end-of-match summary, there's a single side that's won.
		local comparison_text = _ "<span foreground='$side_color'>Side $side_number</span> has the advantage."
		side_comparison = side_comparison .. "\n" .. comparison_text:vformat{side_number = winning_sides[1], side_color = side_color}
	elseif #winning_sides == 2 then
		-- po: In the end-of-match summary, there's a two-way tie (this is only used for exactly two winning teams)
		-- Separated from the three-or-more text in case a language differentiates "two sides" vs "three sides".
		local comparison_text = _ "Sides $side_number and $other_side_number are tied."
		side_comparison = side_comparison .. "\n" .. comparison_text:vformat{side_number = winning_sides[1], other_side_number = winning_sides[2]}
	elseif #winning_sides ~= 0 then
		local winners = stringx.format_conjunct_list("", winning_sides)
		-- po: In the end-of-match summary, three or more teams have all tied for the best score. $winners contains the result of formatting the conjunct list.
		local comparison_text = _ "Sides $winners are tied."
		side_comparison = side_comparison .. "\n" .. comparison_text:vformat{winners = winners}
	end
	-- if #winning_sides==0, then every side either has no units or has a negative score
	-- po: "Turns Over", meaning "turn limit reached" is the title of the end-of-match summary dialog
	title = title or _ "dialog^Turns Over"
	gui.show_popup(title, side_comparison)
end
return res
