
--[========[GUI2 Dialog Manipulations]========]
print("Loading GUI module...")

---Show a basic alert dialog with a single button
---@param title? string Dialog title string
---@param msg string Detail message
function gui.alert(title, msg)
	if not msg then
		msg = title;
		title = "";
	end
	gui.show_prompt(title, msg, "ok", true)
end

---Show a basic prompt dialog with two buttons
---@param title? string Dialog title string
---@param msg string Detail message
---@return boolean #True if OK or Yes was clicked
function gui.confirm(title, msg)
	if not msg then
		msg = title;
		title = "";
	end
	return gui.show_prompt(title, msg, "yes_no", true)
end

---Displays a WML message box with attributes from attr and options from options.
---@param attr WML The contents of a [message] tag, without any [option]s.
---@param options string[]|gui_narration_option_info[] A list of options to show in the message box
---@return integer #the index of the selected option.
function gui.get_user_choice(attr, options)
	local result = 0
	function gui.__user_choice_helper(i)
		result = i
	end
	local msg = {}
	for k,v in pairs(attr) do
		msg[k] = attr[k]
	end
	for k,v in ipairs(options) do
		if type(v) == "table" or (type(v) == "userdata" and getmetatable(v) ~= "translatable string") then
			table.insert(msg, wml.tag.option { image = v.image,
				label = v.label,
				description = v.description or v.message,
				default = v.default,
				wml.tag.command { wml.tag.lua {
					code = string.format("gui.__user_choice_helper(%d)", k)
				}}})
		elseif type(v) == "string" or type(v) == "number" or type(v) == "boolean" or
			(type(v) == "userdata" and getmetatable(v) == "translatable string") then
			table.insert(msg, wml.tag.option { description = v,
				wml.tag.command { wml.tag.lua {
					code = string.format("gui.__user_choice_helper(%d)", k)
				}}})
		else -- only function and thread, because nil stops the cycle anyway
			error(string.format("Invalid data type in gui.get_user_choice (type: %s)", type(v)))
		end
	end
	wesnoth.wml_actions.message(msg)
	gui.__user_choice_helper = nil
	return result
end

-- These functions are initially declared local, then assigned into the wesnoth table.
-- This is a compatibility path for the old GUI2 API.
-- It can be removed if the old API is ever removed.
local open_dialogs = {}

local function reorder_dialog_args(t, n)
	local res = {}
	for i = 1, n do
		table.insert( res, t[1])
		table.remove( t, 1 )
	end
	local w = open_dialogs[1]:find(table.unpack(t))
	return w, res
end

local function set_dialog_callback(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.callback = args[1]
end

local function set_dialog_tooltip(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.tooltip = args[1]
end

local function set_dialog_markup(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.use_markup = args[1]
end

local function set_dialog_canvas(...)
	local w, args = reorder_dialog_args({...}, 2)
	w:set_canvas(table.unpack(args))
end

local function set_dialog_focus(...)
	local w, args = reorder_dialog_args({...}, 0)
	w:focus()
end

local function set_dialog_active(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.enabled = args[1]
end

local function set_dialog_visible(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.visible = args[1]
end

local function set_dialog_value(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.value_compat = args[1]
end

local function get_dialog_value(...)
	local w, args = reorder_dialog_args({...}, 0)
	return w.value_compat
end

local function add_dialog_tree_node(...)
	local w, args = reorder_dialog_args({...}, 2)
	w:add_item_of_type(table.unpack(args))
end

local function remove_dialog_item(...)
	local w, args = reorder_dialog_args({...}, 2)
	w:remove_items_at(table.unpack(args))
end

local old_show_dialog = gui.show_dialog
function gui.show_dialog(dialog_wml, preshow, postshow)

	local res = old_show_dialog(
		dialog_wml,
		function(dialog)
			table.insert(open_dialogs, 1, dialog)
			if preshow then
				preshow(dialog)
			end
		end,
		postshow
	)
	table.remove( open_dialogs, 1 )
	return res
end

wesnoth.show_menu = wesnoth.deprecate_api('wesnoth.show_menu', 'gui.show_menu', 1, nil, gui.show_menu)
wesnoth.show_message_dialog = wesnoth.deprecate_api('wesnoth.show_message_dialog', 'gui.show_narration', 1, nil, gui.show_narration)
wesnoth.show_popup_dialog = wesnoth.deprecate_api('wesnoth.show_popup_dialog', 'gui.show_popup', 1, nil, gui.show_popup)
wesnoth.show_story = wesnoth.deprecate_api('wesnoth.show_story', 'gui.show_story', 1, nil, gui.show_story)
wesnoth.show_message_box = wesnoth.deprecate_api('wesnoth.show_message_box', 'gui.show_prompt', 1, nil, gui.show_prompt)
wesnoth.alert = wesnoth.deprecate_api('wesnoth.alert', 'gui.alert', 1, nil, gui.alert)
wesnoth.confirm = wesnoth.deprecate_api('wesnoth.confirm', 'gui.confirm', 1, nil, gui.confirm)
wesnoth.show_lua_console = wesnoth.deprecate_api('wesnoth.show_lua_console', 'gui.show_lua_console', 1, nil, gui.show_lua_console)
wesnoth.add_widget_definition = wesnoth.deprecate_api('wesnoth.add_widget_definition', 'gui.add_widget_definition', 1, nil, gui.add_widget_definition)
wesnoth.set_dialog_callback = wesnoth.deprecate_api('wesnoth.set_dialog_callback', '<widget>.callback', 1, nil, set_dialog_callback)
wesnoth.set_dialog_tooltip = wesnoth.deprecate_api('wesnoth.set_dialog_tooltip', '<widget>.tooltip', 1, nil, set_dialog_tooltip)
wesnoth.set_dialog_markup = wesnoth.deprecate_api('wesnoth.set_dialog_markup', '<widget>.use_markup', 1, nil, set_dialog_markup)
wesnoth.set_dialog_canvas = wesnoth.deprecate_api('wesnoth.set_dialog_canvas', '<widget>:set_canvas', 1, nil, set_dialog_canvas)
wesnoth.set_dialog_focus = wesnoth.deprecate_api('wesnoth.set_dialog_focus', '<widget>:focus', 1, nil, set_dialog_focus)
wesnoth.set_dialog_active = wesnoth.deprecate_api('wesnoth.set_dialog_active', '<widget>.enabled', 1, nil, set_dialog_active)
wesnoth.set_dialog_visible = wesnoth.deprecate_api('wesnoth.set_dialog_visible', '<widget>.visible', 1, nil, set_dialog_visible)
local value_attributes = '<container>.selected_index or <toggle>.selected or <text_widget>.text or <slider>.value or <progress_bar>.percentage or <tree_view>.selected_item_path or <tree_node>.unfolded or <unit_preview>.unit or <widget>.label'
wesnoth.set_dialog_value = wesnoth.deprecate_api('wesnoth.set_dialog_value', value_attributes, 1, nil, set_dialog_value)
wesnoth.get_dialog_value = wesnoth.deprecate_api('wesnoth.get_dialog_value', value_attributes, 1, nil, get_dialog_value)
wesnoth.add_dialog_tree_node = wesnoth.deprecate_api('wesnoth.add_dialog_tree_node', '<widget>:add_item_of_type', 1, nil, add_dialog_tree_node)
wesnoth.remove_dialog_item = wesnoth.deprecate_api('wesnoth.remove_dialog_item', '<widget>:remove_items_at', 1, nil, remove_dialog_item)
wesnoth.show_dialog = wesnoth.deprecate_api('wesnoth.show_dialog', 'gui.show_dialog', 1, nil, gui.show_dialog)
if wesnoth.kernel_type() == "Game Lua Kernel" then
	-- The deprecated function was only available in Game Lua Kernel, so even though show_help is available in other kernels, there's no need to expose the deprecated function there.
	wesnoth.open_help = wesnoth.deprecate_api('wesnoth.open_help', 'gui.show_help', 1, nil, gui.show_help)
	wesnoth.gamestate_inspector = wesnoth.deprecate_api('wesnoth.gamestate_inspector', 'gui.show_inspector', 1, nil, gui.show_inspector)
end
