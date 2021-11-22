local _ = wesnoth.textdomain 'wesnoth-wc'
local dialog = wml.load "campaigns/World_Conquest/gui/help_dialog.cfg"

local function make_caption(text)
	return ("<big><b>%s</b></big>"):format(text)
end

local function help_page_text(caption, description)
	return caption, ("%s\n\n%s"):format(make_caption(caption), description)
end

function wesnoth.wml_actions.wc2_show_wocopedia(cfg)

	local show_help_mechanics = cfg.show_mechanics ~= false
	local show_help_training = cfg.show_training ~= false
	local show_help_factions = cfg.show_factions ~= false
	local show_help_artifacts = cfg.show_artifacts ~= false
	local show_help_settings = cfg.show_settings ~= false
	-- maps the treeview rows to pagenumber in the help page.
	local index_map = {}

	local current_side = wesnoth.interface.get_viewing_side()
	local preshow = function(dialog)
		local str_cat_mechnics = _ "Game Mechanics"
		local str_des_mechnics = cfg.mechanics_text or
			make_caption( _ "Game Mechanics") .. "\n\n" ..
			_ "<b>Gold</b>\n" ..
			_ "Carryover rate is 15% and split evenly among players. Negative amounts will not carry over. Early finish bonus is superior to village control, but it is not directly related to the carryover amount.\n\n" ..
			_ "<b>Autorecall</b>\n" ..
			_ "Units with the <b>heroic</b> trait are recalled at the start of each scenario with no cost (up to castle size).\n\n" ..
			_ "<b>Recall Cost</b>\n" ..
			_ "Units costing less than 17 gold are cheaper to recall.\n\n" ..
			_ "<b>Training</b>\n" ..
			_ "Every time you recruit a new unit, your training levels will be applied. If a unit gains training benefits, you can see them with the trait \"trained\". Each unit’s chance of gaining training benefits is independent of another’s.\n\n" ..
			_ "<b>Upkeep</b>\n" ..
			_ "Units with the <b>heroic</b> trait or holding any magic <b>item</b> have free upkeep.\n\n" ..
			_ "<b>Bonus Points</b>\n" ..
			_ "In every scenario the game generates as many bonus points on the map as there are players in the game, the bonus points can be picked up by player units and either contain artifacts, loyal units, or training.\n\n" ..
			_ "<b>Army discipline</b>\n" ..
			_ "At scenarios 1 to 3, for each training level players already own, trainers found have 2% to 4% chance to become advanced trainers (provide 2 levels). Becomes irrelevant from scenario 4 onwards because all trainers will always be advanced.\n\n" ..
			""
		local str_cat_training, str_des_training = help_page_text( _ "Training", _ "Training improves newly recruited units, it has no effect on already recruited units. The following list shows all available training, the training you currently have is marked in green.")
		local str_cat_items, str_des_items = help_page_text( _ "Items", _ "Items can be given to units to make them stronger. You can get items in three ways: 1) By choosing an item as your starting bonus; 2) By finding it on a map in a bonus point; 3) By dropping from enemies in later scenarios. Note, however, that not all units can pick up all items.")
		local str_cat_era, str_des_era = help_page_text( _ "Factions" , _ "The World Conquest II era consists of factions that are built from pairs of mainline factions. One faction will have a healer available (Drakes, Rebels and Loyalists) and one will not (Orcs, Dwarves and Undead). The recruit list is also organized in pairs so that sometimes you will have to recruit a different unit before you can recruit the units that you want. The available heroes, deserters, and random leaders also depend on your factions; the items you can get do not depend on the faction you choose.")
		local str_cat_settings = _ "Settings"

		local root_node = dialog:find("treeview_topics")
		local details = dialog:find("details")

		function gui.widget.add_help_page(parent_node, args)
			local node_type = args.node_type or "category"
			local page_type = args.page_type or "simple"

			local node = parent_node:add_item_of_type(node_type)
			local details_page = details:add_item_of_type(page_type)
			if args.title then
				node.label_topic.label = args.title
				node.unfolded = true
			end
			index_map[table.concat(node.path, "_")] = details.item_count
			return node, details_page
		end

		---- add general topic ----
		if show_help_mechanics then
			local node, page = root_node:add_help_page {
				title = str_cat_mechnics
			}
			page.label_content.marked_up_text = str_des_mechnics
		end

		-- add general training topic.
		if show_help_training then
			local node, page = root_node:add_help_page {
				title = str_cat_training
			}
			page.label_content.marked_up_text = str_des_training
			-- add specific training pages
			for i = 1, #wc2_training.get_list() do
				local current_level = wc2_training.get_level(current_side, i)
				local trainer = wc2_training.get_trainer(i)


				local subnode, page = node:add_help_page {
					title = trainer.name,
					page_type = "training",
					node_type = "subcategory",
				}

				local function set_description(train_num, j)
					local desc = wc2_training.generate_message(i, train_num)
					if train_num == current_level then
						desc.caption = "<span color='#00FF00'>" .. desc.caption .. "</span>"
						desc.message = "<span color='#00FF00'>" .. desc.message .. "</span>"
					end

					local page_element = page.treeview_details:add_item_of_type("training_details")
					page_element.training_caption.marked_up_text = desc.caption
					page_element.training_description.marked_up_text = desc.message
				end

				set_description(1)
				for j = 2, #trainer.grade - 1, 1 do
					page.treeview_details:add_item_of_type("seperator")
					set_description(j)
				end
			end
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
			local era_wml = wesnoth.scenario.era

			local node, page = root_node:add_help_page {
				title = str_cat_era
			}

			page.label_content.marked_up_text = str_des_era

			for i, faction_info in ipairs(wc2_era.factions_wml) do
				local faction_wml = wml.get_child(era_wml, "multiplayer_side", faction_info.id)

				local subnode, page = node:add_help_page {
					title = faction_info.name,
					page_type = "faction",
					node_type = "subcategory",
				}

				for p_wml in wml.child_range(faction_info, "pair") do
					local p = stringx.split(p_wml.types or "")
					local ut1 = wesnoth.unit_types[p[1]] or error("invald unit type" .. tostring(p[1]))
					local ut2 = wesnoth.unit_types[p[2]] or error("invald unit type" .. tostring(p[2]))

					local page_element = page.treeview_recruits:add_item_of_type("recruit_pair")
					page_element.label1.marked_up_text = ut1.name
					page_element.image1.label = type_icon(ut1)
					page_element.label2.marked_up_text = ut2.name
					page_element.image2.label = type_icon(ut2)

				end

				page.label_deserters.marked_up_text = wesnoth.format_conjunct_list("", wc2_era.expand_hero_names(faction_info.deserters))
				page.label_commanders.marked_up_text = wesnoth.format_conjunct_list("", wc2_era.expand_hero_names(faction_info.commanders))
				page.label_heroes.marked_up_text = wesnoth.format_conjunct_list("", wc2_era.expand_hero_names(faction_info.heroes, true))

				if faction_wml then
					local random_leaders = {}
					for i,v in ipairs(stringx.split(faction_wml.random_leader or "")) do
						table.insert(random_leaders, wesnoth.unit_types[v].name)
					end
					random_leaders = wesnoth.format_conjunct_list("", random_leaders)

					page.label_random_leaders.marked_up_text = random_leaders
				else
					page.title_random_leaders.visible = false
				end
			end
		end

		if show_help_artifacts then
			local str_not_for_enemies = _ " (not available for enemies)"
			local str_not_for_players = _ " (not available for players)"

			local node, page = root_node:add_help_page {
				title = str_cat_items,
				page_type = "artifacts",
			}

			page.desc.marked_up_text = str_des_items

			for i, artifact in ipairs(wc2_artifacts.get_artifact_list()) do
				local artifact_icon = artifact.icon or ""
				local artifact_name = artifact.name or ""
				local artifact_desc = artifact.description or ""
				local not_available = stringx.map_split(artifact.not_available or "")

				if not_available.player then
					artifact_name = artifact_name .. str_not_for_players
				end
				if not_available.enemy then
					artifact_name = artifact_name .. str_not_for_enemies
				end

				local page_element = page.treeview_artifacts:add_item_of_type("artifact")
				page_element.image.label = artifact_icon
				page_element.label_name.label = artifact_name .. "\n" .. artifact_desc
			end
		end

		if show_help_settings then
			local node, page = root_node:add_help_page {
				title = str_cat_settings,
				page_type = "settings",
			}

			page.checkbox_use_pya.selected = not not wml.variables["wc2_config_enable_pya"]
			page.checkbox_use_pya.enabled = false
			page.checkbox_use_markers.selected = not not wml.variables["wc2_config_enable_unitmarker"]
			page.checkbox_use_markers.enabled = false
			page.checkbox_use_pickup.selected = not not wml.variables["wc2_config_experimental_pickup"]
			page.checkbox_use_pickup.enabled = false
			page.checkbox_show_pickup_confirmation.selected = not wc2_utils.global_vars.skip_pickup_dialog
			page.checkbox_show_pickup_confirmation.enabled = true

			page.label_version.marked_up_text = wml.variables["wc2_host_version"] or "unknown"
			page.label_difficulty.marked_up_text = wml.variables["wc2_difficulty.name"] or "unknown"

			function page.checkbox_show_pickup_confirmation.on_modified()
				wc2_utils.global_vars.skip_pickup_dialog = not page.checkbox_show_pickup_confirmation.selected
			end
		end

		root_node:focus()

		function root_node.on_modified()
			local selected_index = index_map[table.concat(root_node.selected_item_path, '_')]
			if selected_index ~= nil then
				details.selected_index = selected_index
			end
		end
	end

	local dialog_wml = wml.get_child(dialog, 'resolution')
	gui.show_dialog(dialog_wml, preshow)
end

wc2_utils.menu_item {
	id = "5_WCT_Wocopedia_Option",
	description = _ "WoCopedia",
	image= "help/closed_section.png~SCALE(18,17)",
	synced = false,
	filter = function(x, y)
		local u = wesnoth.units.get(x, y)
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
