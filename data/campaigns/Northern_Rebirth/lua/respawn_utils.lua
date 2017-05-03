local helper = wesnoth.require "helper"
local T = helper.set_wml_tag_metatable {}

function wesnoth.wml_actions.find_respawn_point(cfg)
	local respawn_near = cfg.respawn_near or helper.wml_error "[find_respawn_point] missing required respawn_near= key"
	local variable = cfg.variable or "nearest_hex"
	local respawn_point
	local radius = 1

	if not wesnoth.eval_conditional { T.have_unit { id = respawn_near} } then
		respawn_near = "Tallin"
	end

	repeat
		respawn_point = wesnoth.get_locations({
			include_borders = false,
			T["and"] {
				T.filter {
					id = respawn_near
				},
				radius = radius
			},
			T["not"] {
				T.filter {
				}
			},
			T["not"] {
				terrain = "Wo,*^Xm,X*,Q*"
			}
		})

		radius = radius + 1
	until respawn_point[1]

	wesnoth.set_variable(variable .. ".x", respawn_point[1][1])
	wesnoth.set_variable(variable .. ".y", respawn_point[1][2])
end
