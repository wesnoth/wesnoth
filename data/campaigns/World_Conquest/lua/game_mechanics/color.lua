local color = {}

function color.to_pango_string(c)
	return ("#%02x%02x%02x"):format(c.r, c.g, c.b)
end

function color.color_text(color_str, text)
	return "<span color='" .. color_str .. "'>" .. text .. "</span>"
end


function color.bonus_text(str)
	return color.color_text("#ff75ff", str)
end

function color.help_text(str)
	return color.color_text("#ff95ff", str)
end

-- note: the default argument for the first parameter is the 
--       currently active side, not the currently viewing side
function color.tc_text(team_num, text)
	if text == nil then
		text = team_num
		team_num = wesnoth.current.side
	end
	local c = wesnoth.colors[wesnoth.sides[team_num].color].mid

	local color_str = color.to_pango_string(c)
	return color.color_text(color_str, text)
end

function color.tc_image(team_num, img)
	if img == nil then
		img = team_num
		team_num = wesnoth.current.side
	end
	return img .. "~TC(" .. team_num .. ",magenta)"
end

-- Fixes the colors in mp campaigns: in case that the players changed the
-- colors in the mp setup screen, we have to remember those settings and
-- set the teams color in later scenarios acccordingly.
function wesnoth.wml_actions.wc2_fix_colors(cfg)
	local player_sides = wesnoth.sides.find(wml.get_child(cfg, "player_sides"))
	local other_sides = wesnoth.sides.find { { "not", wml.get_child(cfg, "player_sides") } }
	local available_colors = { "red", "blue", "green", "purple", "black", "brown", "orange", "white", "teal" }
	local taken_colors = {}

	for i, side in ipairs(player_sides) do
		if side.variables.wc2_color then
			side.color = side.variables.wc2_color
		else
			side.variables.wc2_color = side.color
		end
		taken_colors[side.color] = true
	end

	local color_num = 1
	for i, side in ipairs(other_sides) do
		while taken_colors[available_colors[color_num]] == true do
			color_num = color_num + 1
		end
		side.color = available_colors[color_num]
		taken_colors[side.color] = true
	end
end

return color
