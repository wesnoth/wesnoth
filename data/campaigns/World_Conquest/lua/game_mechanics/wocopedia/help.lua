local _ = wesnoth.textdomain 'wesnoth-wc'
local dialog = wc2_wiki_dialog

function wesnoth.set_dialog_text(text, ...)
	wesnoth.set_dialog_markup(true, ...)
	wesnoth.set_dialog_value(text, ...)
end

function wesnoth.wml_actions.wc2_show_wocopedia(cfg)

	local show_help_mechanics = cfg.show_mechanics ~= false
	local show_help_training = cfg.show_training ~= false
	local show_help_factions = cfg.show_factions ~= false
	local show_help_artifacts = cfg.show_artifacts ~= false
	local show_help_settings = cfg.show_settings ~= false
	local show_help_feedback = cfg.show_feedback ~= false
	-- maps the treeview rows to pagenumber in the help page.
	local index_map = {}
	local desc_index = 1
	local ti = {1}
	local function add_index()
		index_map[table.concat(ti, "_")] = desc_index
		desc_index = desc_index + 1
		ti[#ti] = ti[#ti] + 1
	end
	local function tree_enter_mode()
		ti[#ti] = ti[#ti] - 1
		table.insert(ti, 1)
	end
	local function leave_enter_mode()
		table.remove(ti)
		ti[#ti] = ti[#ti] + 1
	end
	local current_side = wesnoth.get_viewing_side()
	local preshow = function()
		local str_cat_mechnics = _ "Game Mechanics"
		local str_des_mechnics = cfg.mechanics_text or
			_ "<b>Gold</b>:\n" ..
			_ "Carryover is 15%, comunitary and avoid negative amounts. Early finish bonus is superior to village control, but it is not directly related to their amount.\n\n" ..
			_ "<b>Autorecall</b>:\n" ..
			_ "Units with trait HEROIC are recalled at start of each scenario with no cost (up to castle size).\n\n" ..
			_ "<b>Recall Cost</b>:\n" ..
			_ "Units costing less than 17 gold are cheaper to recall.\n\n" ..
			_ "<b>Trainings</b>:\n" ..
			_ "Every time you recruit a new unit, your trainings levels will be applied. Every chance will do different dice rolls, if a unit gains training beneficts, you can see them in a trait \"trained\".\n\n" ..
			_ "<b>Upkeep</b>:\n" ..
			_ "Units with trait HEROIC or holding any magic ITEM have FREE upkeep.\n\n" ..
			_ "<b>Bonus Points</b>:\n" ..
			_ "In every scenario the game generates as many bonus points on the map as there are players in the game, the bonus points can be picked up by player units and either contain artifacts, loyal units or training.\n\n" ..
			_ "<b>Army discipline</b>:\n" ..
			_ "At scenarios 1 to 3, for each training level player already own, trainers found have 2% to 4% chance to become advanced trainers (provide 2 levels). Becomes irrelevant from scenario 4 because all trainers always will be advanced.\n\n" ..
			""
		local str_cat_feedback = _ "Feedback"
		local str_des_feedback =
			_ "<b>Feedback</b>:\n" ..
			_ "For feedback plase either post in the Word conquest II thread in the official wesnoth forum https://r.wesnoth.org/t39651 or file an issue at github https://github.com/gfgtdf/World_Conquest_II/issues .\n\n" ..
			""
		local str_cat_abilities = _ "Abilities"
		local str_des_abilities =
			_ "Ability <b>Autorecall</b>:\n" ..
			_ "Units with trait HEROIC are recalled at start of each scenario with no cost (up to castle size).\n\n" ..
			""
		local str_cat_training = _ "Training"
		local str_des_training = _ "<b>Training</b>\nTraining improves newly recruited units, it has no effect on already recruited units. The follwing list shows all available trainings, the training you currently have is marked in green."
		local str_cat_items = _ "Artifacts"
		local str_des_items = _ "<b>Items</b>\nItems can be given to units to make them stronger. You can get artifcats in three ways: 1) By choosing an item as your starting bonus, 2) By finding it on a map in a bonus point, 3) By dropping from enemies in later scenarios. Note however that not all units can pickup all items."
		local str_cat_era = _ "Factions"
		local str_des_era = _ "<b>Factions</b>\n The Word Conquest 2 era consists of faction that are build of pairs of mainline faction of which at one has a healer available (Drakes, Rebels and Loyalists), and one does not (Orcs, Dwarves and Undead) the recruilist is also organized in pairs so that you sometimes have to recruit a different units before you can recruit the units that you want. The available heroes, desertes and random leaders also depend on your factions, the items you can get do not depend on the faction you choose."
		local str_cat_settings = _ "Settings"

		---- add general topic ----
		if show_help_mechanics then
			wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
			wesnoth.set_dialog_value(str_cat_mechnics, "left_tree", ti[1], "training_name")
			wesnoth.set_dialog_value(true, "left_tree", ti[1])
			wesnoth.add_dialog_tree_node("simpletext", -1, "details")
			wesnoth.set_dialog_text(str_des_mechnics, "details", desc_index, "label")
			add_index()
		end
		if show_help_training then
			---- add general training topic ----
			wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
			wesnoth.set_dialog_value(str_cat_training, "left_tree", ti[1], "training_name")
			wesnoth.set_dialog_value(true, "left_tree", ti[1])
			wesnoth.add_dialog_tree_node("simpletext", -1, "details")
			wesnoth.set_dialog_text(str_des_training, "details", desc_index, "label")
			add_index()
			tree_enter_mode()
			-- add specific training pages
			for i = 1, #wc2_training.get_list() do
				local current_level = wc2_training.get_level(current_side, i)
				local function set_description(train_num, j)
					local desc = wc2_training.generate_message(i, train_num)
					if train_num == current_level then
						desc.caption = "<span color='#00FF00'>" .. desc.caption .. "</span>"
						desc.message = "<span color='#00FF00'>" .. desc.message .. "</span>"
					end
					wesnoth.add_dialog_tree_node("training_details", j, "details", desc_index, "tree_details")
					wesnoth.set_dialog_text(desc.caption, "details", desc_index, "tree_details", j, "training_caption")
					wesnoth.set_dialog_text(desc.message, "details", desc_index, "tree_details", j, "training_description")
				end
				local trainer = wc2_training.get_trainer(i)
				wesnoth.add_dialog_tree_node("training_category", i, "left_tree", ti[1])
				wesnoth.set_dialog_value(trainer.name, "left_tree", ti[1], ti[2], "training_name")
				set_description(1, 1)
				for j = 2, #trainer.grade - 1, 1 do
					wesnoth.add_dialog_tree_node("seperator", 2*j - 2, "details", desc_index, "tree_details")
					set_description(j, 2*j - 1)
				end
				add_index()
			end
			leave_enter_mode()
		end
		if show_help_factions then
			local function type_icon(ut)
				local icon = ut.icon
				if icon and icon ~= "" then
					return icon
				else
					return ut.image
				end
			end
			---- add general factions topic ----
			local era_wml = wesnoth.game_config.era
			wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
			wesnoth.set_dialog_value(str_cat_era, "left_tree", ti[1], "training_name")
			wesnoth.set_dialog_value(true, "left_tree", ti[1])
			wesnoth.add_dialog_tree_node("simpletext", -1, "details")
			wesnoth.set_dialog_text(str_des_era, "details", desc_index, "label")
			add_index()
			tree_enter_mode()
			for i, faction_info in ipairs(wc2_era.factions_wml) do
				local faction_wml = wml.get_child(era_wml, "multiplayer_side", faction_info.id)
				wesnoth.add_dialog_tree_node("training_category", ti[2], "left_tree", ti[1])
				wesnoth.set_dialog_value(faction_info.name, "left_tree", ti[1], ti[2], "training_name")
				wesnoth.add_dialog_tree_node("faction_info", -1, "details")
				j = 0
				for p_wml in wml.child_range(faction_info, "pair") do
					j = j + 1
					local p = wc2_utils.split_to_array(p_wml.types)
					local ut1 = wesnoth.unit_types[p[1]] or error("invald unit type" .. tostring(p[1]))
					local ut2 = wesnoth.unit_types[p[2]] or error("invald unit type" .. tostring(p[2]))

					wesnoth.add_dialog_tree_node("recruit_pair", j, "details", desc_index, "recruit_pairs")
					wesnoth.set_dialog_text(ut1.name, "details", desc_index, "recruit_pairs", j, "label1")
					print(tostring(ut1.icon or ut1.image))
					wesnoth.set_dialog_value(type_icon(ut1), "details", desc_index, "recruit_pairs", j, "image1")
					wesnoth.set_dialog_text(ut2.name, "details", desc_index, "recruit_pairs", j, "label2")
					wesnoth.set_dialog_value(type_icon(ut2), "details", desc_index, "recruit_pairs", j, "image2")
				end
				local deserters_names = wesnoth.format_conjunct_list("", wc2_era.expand_hero_names(faction_info.deserters))
				wesnoth.set_dialog_text(deserters_names, "details", desc_index, "deserters")
				local deserters_names = wesnoth.format_conjunct_list("", wc2_era.expand_hero_names(faction_info.commanders))
				wesnoth.set_dialog_text(deserters_names, "details", desc_index, "commanders")
				local deserters_names = wesnoth.format_conjunct_list("", wc2_era.expand_hero_names(faction_info.heroes, true))
				wesnoth.set_dialog_text(deserters_names, "details", desc_index, "heroes")

				if faction_wml then
					local random_leaders = {}
					for i,v in ipairs(wc2_utils.split_to_array(faction_wml.random_leader)) do
						table.insert(random_leaders, wesnoth.unit_types[v].name)
					end
					random_leaders = wesnoth.format_conjunct_list("", random_leaders)
					wesnoth.set_dialog_text(random_leaders, "details", desc_index, "random_leaders")
				else
					wesnoth.set_dialog_visible(false, "details", desc_index, "tit_random_leaders")
				end

				add_index()
			end
			leave_enter_mode()
		end
		---- add general bonus point topic ----
		--wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
		--wesnoth.set_dialog_value(str_cat_bonus, "left_tree", ti[1], "training_name")
		--wesnoth.add_dialog_tree_node("simpletext", -1, "details")
		--wesnoth.set_dialog_text(str_des_bonus, "details", desc_index, "label")
		--add_index()

		if show_help_artifacts then
			local str_not_for_enemies = _ " (not available for enemies)"
			local str_not_for_players = _ " (not available for players)"

			wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
			wesnoth.set_dialog_value(str_cat_items, "left_tree", ti[1], "training_name")
			wesnoth.add_dialog_tree_node("artifact_list", -1, "details")
			wesnoth.set_dialog_text(str_des_items, "details", desc_index, "desc")

			for i, artifact in ipairs(wc2_artifacts.get_artifact_list()) do
				local artifact_icon = artifact.icon or ""
				local artifact_name = artifact.name or ""
				local artifact_desc = artifact.description or ""
				local not_available = wc2_utils.split_to_set(artifact.not_available or "")

				if not_available.player then
					artifact_name = artifact_name .. str_not_for_players
				end
				if not_available.enemy then
					artifact_name = artifact_name .. str_not_for_enemies
				end
				wesnoth.add_dialog_tree_node("artifact", i, "details", desc_index, "artifact_list_tv")
				wesnoth.set_dialog_value(artifact_icon, "details", desc_index, "artifact_list_tv", i, "image")
				wesnoth.set_dialog_value(artifact_name .. "\n" .. artifact_desc, "details", desc_index, "artifact_list_tv", i, "label")
			end
			add_index()
		end

		if show_help_settings then
			wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
			wesnoth.set_dialog_value(str_cat_settings, "left_tree", ti[1], "training_name")
			wesnoth.set_dialog_value(true, "left_tree", ti[1])
			wesnoth.add_dialog_tree_node("settings", -1, "details")
			wesnoth.set_dialog_value(not not wml.variables["wc2_config_enable_pya"], "details", desc_index, "checkbox_use_pya")
			wesnoth.set_dialog_active(false, "details", desc_index, "checkbox_use_pya")
			wesnoth.set_dialog_value(not not wml.variables["wc2_config_enable_unitmarker"], "details", desc_index, "checkbox_use_markers")
			wesnoth.set_dialog_active(false, "details", desc_index, "checkbox_use_markers")
			wesnoth.set_dialog_value(not not wml.variables["wc2_config_experimental_pickup"], "details", desc_index, "checkbox_use_pickup")
			wesnoth.set_dialog_active(false, "details", desc_index, "checkbox_use_pickup")
			wesnoth.set_dialog_text(wml.variables["wc2_host_version"] or "unknown", "details", desc_index, "label_version")
			wesnoth.set_dialog_text(wml.variables["wc2_difficulty.name"] or "unknown", "details", desc_index, "label_difficulty")

			wesnoth.set_dialog_value(not wc2_utils.global_vars.skip_pickup_dialog, "details", desc_index, "checkbox_show_pickup_confirmation")
			wesnoth.set_dialog_active(true, "details", desc_index, "checkbox_show_pickup_confirmation")
			local desc_index_copy = desc_index
			wesnoth.set_dialog_callback(function()
				wc2_utils.global_vars.skip_pickup_dialog = not wesnoth.get_dialog_value("details", desc_index_copy, "checkbox_show_pickup_confirmation")
			end, "details", desc_index_copy, "checkbox_show_pickup_confirmation")

			local mp_settings = wesnoth.game_config.mp_settings
			if mp_settings and mp_settings.active_mods then
				--FIXME: mp_settings.active_mods was removed during an internal refactor of the c++ mp_settings object.
				--       but maybe we shouldn't need this in the mainline version anyways.
				wesnoth.set_dialog_text(mp_settings.active_mods, "details", desc_index, "label_activemods")
			end
			add_index()
		end
		if show_help_feedback then
			wesnoth.add_dialog_tree_node("category", ti[1], "left_tree")
			wesnoth.set_dialog_value(str_cat_feedback, "left_tree", ti[1], "training_name")
			wesnoth.set_dialog_value(true, "left_tree", ti[1])
			wesnoth.add_dialog_tree_node("simpletext", -1, "details")
			wesnoth.set_dialog_text(str_des_feedback, "details", desc_index, "label")
			add_index()
		end
		wesnoth.set_dialog_focus("left_tree")

		wesnoth.set_dialog_callback(function()
			local selected = wesnoth.get_dialog_value("left_tree")
			local selected_page_index = index_map[table.concat(selected, '_')]
			if selected_page_index ~= nil then
				wesnoth.set_dialog_value(selected_page_index, "details")
			end
		end, "left_tree")
	end

	wesnoth.show_dialog(dialog, preshow)
end

wc2_utils.menu_item {
	id = "5_WCT_Wocopedia_Option",
	description = _ "WoCopedia",
	image= "help/closed_section.png~SCALE(18,17)",
	synced = false,
	filter = function(x, y)
		local u = wesnoth.get_unit(x, y)
		if wc2_artifacts.is_item_at(x, y) then
			return false
		end
		return not (u and u.side == wesnoth.current.side)
	end,
	handler = function(cx)
		wesnoth.wml_actions.wc2_show_wocopedia {
			x = cx.x1,
			y = cx.y1,
		}
	end
}
