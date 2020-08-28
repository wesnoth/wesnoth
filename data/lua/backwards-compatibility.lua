--! #textdomain wesnoth

-- This file may provide an implementation of Lua functions removed from the engine.

wesnoth.set_music = wesnoth.deprecate_api('wesnoth.set_music', 'wesnoth.music_list', 1, nil, wesnoth.wml_actions.music)

-- Calling wesnoth.fire isn't the same as calling wesnoth.wml_actions[name] due to the passed vconfig userdata
-- which also provides "constness" of the passed wml object from the point of view of the caller.
-- So please don't remove since it's not deprecated.
function wesnoth.fire(name, cfg)
	wesnoth.wml_actions[name](wml.tovconfig(cfg or {}))
end

local function reorder_dialog_args(t, n)
	local res = {}
	for i = 1, n do
		table.insert( res, t[1])
		table.remove( t, 1 )
	end
	local w = gui.widget.find(unpack(t))
	return w, res
end

function wesnoth.set_dialog_callback(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.callback = args[1]
end

function wesnoth.set_dialog_tooltip(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.tooltip = args[1]
end

function wesnoth.set_dialog_markup(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.use_markup = args[1]
end

function wesnoth.set_dialog_canvas(...)
	local w, args = reorder_dialog_args({...}, 2)
	w:set_canvas(unpack(args))
end

function wesnoth.set_dialog_focus(...)
	local w, args = reorder_dialog_args({...}, 0)
	w:focus()
end

function wesnoth.set_dialog_active(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.enabled = args[1]
end

function wesnoth.set_dialog_visible(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.visible = args[1]
end

function wesnoth.set_dialog_value(...)
	local w, args = reorder_dialog_args({...}, 1)
	w.value_compat = args[1]
end

function wesnoth.get_dialog_value(...)
	local w, args = reorder_dialog_args({...}, 0)
	return w.value_compat
end

function wesnoth.add_dialog_tree_node(...)
	local w, args = reorder_dialog_args({...}, 2)
	w:add_item_of_type(unpack(args))
end

function wesnoth.remove_dialog_item(...)
	local w, args = reorder_dialog_args({...}, 2)
	w:remove_items_at(unpack(args))
end

--function wesnoth.show_dialog(...)
--	print("widget", widget)
--	widget.show_dialog(...)
--end

std_print("compat done")
