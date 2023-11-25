
-- to make code shorter
local wml_actions = wesnoth.wml_actions

-- metatable for GUI tags
local T = wml.tag


-- from the Wesnoth Lua Pack
-- [item_dialog]
-- an alternative interface to pick items
-- could be used in place of [message] with [option] tags
function wml_actions.item_dialog( cfg )
	local image_and_description = T.grid {
		T.row {
			T.column {
				vertical_alignment = "center",
				horizontal_alignment = "center",
				border = "all",
				border_size = 5,
				T.image {
					id = "image_name"
				}
			},
			T.column {
				horizontal_alignment = "left",
				border = "all",
				border_size = 5,
				T.scroll_label {
					id = "item_description"
				}
			}
		}
	}

	local buttonbox = T.grid {
		T.row {
			T.column {
				T.button {
					id = "take_button",
					return_value = 1
				}
			},
			T.column {
				T.spacer {
					width = 10
				}
			},
			T.column {
				T.button {
					id = "leave_button",
					return_value = 2
				}
			}
		}
	}

	local item_dialog = {
		T.helptip { id="tooltip_large" }, -- mandatory field
		T.tooltip { id="tooltip_large" }, -- mandatory field
		maximum_height = 640,
		maximum_width = 480,
		T.grid { -- Title, will be the object name
			T.row {
				T.column {
					horizontal_alignment = "left",
					grow_factor = 1,
					border = "all",
					border_size = 5,
					T.label {
						definition = "title",
						id = "item_name"
					}
				}
			},
			-- Image and item description
			T.row {
				T.column {
					image_and_description
				}
			},
			-- Effect description
			T.row {
				T.column {
					horizontal_alignment = "left",
					border = "all",
					border_size = 5,
					T.label {
						wrap = true,
						id = "item_effect"
					}
				}
			},
			-- button box
			T.row {
				T.column {
					buttonbox
				}
			}
		}
	}

	local function item_preshow(dialog)
		-- here set all widget starting values
		dialog.item_description.use_markup = true
		dialog.item_effect.use_markup = true
		dialog.item_name.label = cfg.name or ""
		dialog.image_name.label = cfg.image or ""
		dialog.item_description.label = cfg.description or ""
		dialog.item_effect.label = cfg.effect or ""
		dialog.take_button.label = cfg.take_string or wml.error( "Missing take_string= key in [item_dialog]" )
		dialog.leave_button.label = cfg.leave_string or wml.error( "Missing leave_string= key in [item_dialog]" )
	end

	local function sync()
		local function item_postshow(dialog)
			-- here get all widget values
		end

		local return_value = gui.show_dialog( item_dialog, item_preshow, item_postshow )

		return { return_value = return_value }
	end

	local return_table = wesnoth.sync.evaluate_single(sync)
	if return_table.return_value == 1 or return_table.return_value == -1 then
		wml.variables[cfg.variable or "item_picked"] = "yes"
	else wml.variables[cfg.variable or "item_picked"] = "no"
	end
end














-- a variation for when you are required to take the item
function wml_actions.item_dialog_musttake( cfg )
	local image_and_description = T.grid {
		T.row {
			T.column {
				vertical_alignment = "center",
				horizontal_alignment = "center",
				border = "all",
				border_size = 5,
				T.image {
					id = "image_name"
				}
			},
			T.column {
				horizontal_alignment = "left",
				border = "all",
				border_size = 5,
				T.scroll_label {
					id = "item_description"
				}
			}
		}
	}

	local buttonbox = T.grid {
		T.row {
			T.column {
				T.button {
					id = "take_button",
					return_value = 1
				}
			},
		}
	}

	local item_dialog = {
		T.helptip { id="tooltip_large" }, -- mandatory field
		T.tooltip { id="tooltip_large" }, -- mandatory field
		maximum_height = 640,
		maximum_width = 480,
		T.grid { -- Title, will be the object name
			T.row {
				T.column {
					horizontal_alignment = "left",
					grow_factor = 1,
					border = "all",
					border_size = 5,
					T.label {
						definition = "title",
						id = "item_name"
					}
				}
			},
			-- Image and item description
			T.row {
				T.column {
					image_and_description
				}
			},
			-- Effect description
			T.row {
				T.column {
					horizontal_alignment = "left",
					border = "all",
					border_size = 5,
					T.label {
						wrap = true,
						id = "item_effect"
					}
				}
			},
			-- button box
			T.row {
				T.column {
					buttonbox
				}
			}
		}
	}

	local function item_preshow(dialog)
		-- here set all widget starting values
		dialog.item_description.use_markup = true
		dialog.item_effect.use_markup = true
		dialog.item_name.label = cfg.name or ""
		dialog.image_name.label = cfg.image or ""
		dialog.item_description.label = cfg.description or ""
		dialog.item_effect.label = cfg.effect or ""
		dialog.take_button.label = cfg.take_string or wml.error( "Missing take_string= key in [item_dialog]" )
	end

	local function sync()
		local function item_postshow(dialog)
			-- here get all widget values
		end

		local return_value = gui.show_dialog( item_dialog, item_preshow, item_postshow )

		return { return_value = return_value }
	end

	local return_table = wesnoth.sync.evaluate_single(sync)
	if return_table.return_value == 1 or return_table.return_value == -1 then
		wml.variables[cfg.variable or "item_picked"] = "yes"
	else wml.variables[cfg.variable or "item_picked"] = "no"
	end
end
