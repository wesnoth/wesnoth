-- Note: This file is loaded automatically by the engine.

function wesnoth.alert(title, msg)
	if not msg then
		msg = title;
		title = "";
	end
	wesnoth.show_message_box(title, msg, "ok", true)
end

function wesnoth.confirm(title, msg)
	if not msg then
		msg = title;
		title = "";
	end
	return wesnoth.show_message_box(title, msg, "yes_no", true)
end
