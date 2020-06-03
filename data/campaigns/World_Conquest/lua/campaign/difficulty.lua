-- The difficulty dialog. unlike other files this does not 'export' functions,
-- just run this file to show the diffculty dialog.

local _ = wesnoth.textdomain 'wesnoth-World_Conquest'
local strings = {
	chose_difficulty = "<span size='large'>" .. _"Choose difficulty level:" .. "</span>",
}

local _ = wesnoth.textdomain 'wesnoth-units'
local strings_mainline = {
	Sergeant = _"Sergeant",
	Peasant = _"Peasant",
	Lieutenant = _"Lieutenant",
	General = _"General",
	Grand_Marshal = _"Grand Marshal"
}

local function icon_human_difficult(unit_image, color)
	return "misc/blank-hex.png~SCALE(140,100)" ..
		"~BLIT(units/" .. unit_image .. ".png~RC(magenta>" .. color .. "),34,7)"
end

local function str_dif_lvl(name)
	return "<span size='large'>" .. name ..  "</span>"
end

local icon_nightmare_difficulty = "units/monsters/fire-dragon.png~CROP(0,0,160,160)~RC(magenta>red)"


local t_option = wml.tag.option

local function wct_difficulty(name, power, enemy_t, heroes, gold, train, exp)
	local nplayers = wml.variables.wc2_player_count
	if nplayers == 1 then
		heroes = heroes + 1
	end
	-- adjust bonus gold for number of players
	gold = gold * math.pow(2, 3 - nplayers)
	return wml.tag.command {
		wml.tag.set_variables {
			name = "wc2_difficulty",
			wml.tag.literal {
				name = name,
				enemy_power = power,
				enemy_trained = enemy_t,
				heroes = heroes,
				extra_gold = gold,
				extra_trainig = train,
				experience_penalty = exp,
			}
		}
	}
end


function wct_scenario_chose_difficulty()
	-- fixme: should the first part argument of wct_difficulty be translatable
	wesnoth.wml_actions.message {
		speaker = "narrator",
		caption = strings.chose_difficulty,
		t_option {
			image = icon_human_difficult("human-peasants/peasant", "purple"),
			label = str_dif_lvl(strings_mainline.Peasant),
			description="(" .. _"Easy" .. ")",
			wct_difficulty("Peasant", 6, 2, 2, 10, true, 0),
		},
		t_option {
			image=icon_human_difficult("human-loyalists/sergeant", "black"),
			label=str_dif_lvl(strings_mainline.Sergeant),
			wct_difficulty("Sergeant", 7, 3, 2, 7, true, 5),
		},
		t_option {
			image=icon_human_difficult("human-loyalists/lieutenant", "brown"),
			label=str_dif_lvl(strings_mainline.Lieutenant),
			wct_difficulty("Lieutenant", 8, 4, 2, 5, true, 10),
		},
		t_option {
			image=icon_human_difficult("human-loyalists/general", "orange"),
			label=str_dif_lvl(strings_mainline.General),
			wct_difficulty("General", 8, 5, 2, 2, false, 13),
		},
		t_option {
			image=icon_human_difficult("human-loyalists/marshal", "white"),
			label=str_dif_lvl(strings_mainline.Grand_Marshal),
			wct_difficulty("Grand_marshal", 9, 6, 2, 1, false, 17),
		},
		t_option {
			image=icon_nightmare_difficulty,
			label=str_dif_lvl("Nightmare"),
			description="(" .. _"Expert" .. ")",
			wct_difficulty("Nightmare", 9, 7, 1, 0, false, 20),
		},
	}
end

function wct_scenario_start_bonus()
	for side_num = 1, wml.variables.wc2_player_count do
		wesnoth.wml_actions.wc2_start_units {
			side = side_num
		}
	end

	if wml.variables.wc2_difficulty.extra_trainig then
		for side_num = 1, wml.variables.wc2_player_count do
			wesnoth.wml_actions.wc2_give_random_training {
				among="2,3,4,5,6",
				side = side_num,
			}
		end
	end
end

function wesnoth.wml_actions.wc2_choose_difficulty(cfg)
	if wml.variables["wc2_difficulty"] then
		return
	end
	wct_scenario_chose_difficulty()
	wct_scenario_start_bonus()
	wesnoth.fire_event("wc2_start")
end
