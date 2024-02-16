
local _ = wesnoth.textdomain 'wesnoth-help'
local T = wml.tag

local u_pos_filter = function(u_id)

        local output = "initial"
        local hex_dirs = {"n", "ne", "se", "s", "sw", "nw"}
        local diversion_unit = wesnoth.units.get(u_id)
        if not diversion_unit then
            return nil
        end
        for i, dir in ipairs(hex_dirs) do
            if diversion_unit:matches {
              id = u_id,
              T.filter_adjacent {
                  is_enemy = "yes",
                  adjacent = dir,
                  formula = "self.hitpoints > 0",
                  T.filter_adjacent {
                      is_enemy = "yes",
                      adjacent = dir,
                      formula = "self.hitpoints > 0"
                  }
              }
            } then
                output = "diverter"
                break
            end
        end
        if output ~= "initial" then
            return output
        else
        -- either nothing passed filter, or there was an error
            return nil
        end
end

local function status_anim_update(is_undo)

        local ec = wesnoth.current.event_context
        local changed_something  = false

        if not ec.x1 or not ec.y1 then
                return
        end

        -- find all units on map with ability = diversion but not status.diversion = true
        local div_candidates = wesnoth.units.find_on_map({
                ability = "diversion",
                wml.tag["not"] { status = "diversion" }
                })
        -- for those that pass the filter now, change status and fire animation
        for index, ec_unit in ipairs(div_candidates) do
                local filter_result = u_pos_filter(ec_unit.id)
                if filter_result then
                    changed_something = true
                    ec_unit.status.diversion = true
                    ec_unit:extract()
                    ec_unit:to_map(false)
                    wesnoth.wml_actions.animate_unit {
                            flag = "launching",
                            with_bars = true,
                            T.filter { id = ec_unit.id }
                    }
                end
        end

        -- find all units on map with ability = diversion and status.diversion = true
        local stop_candidates = wesnoth.units.find_on_map({
                ability = "diversion",
                status = "diversion"
                })
        -- for those that fail the filter now, change status and fire animation
        for index, ec_unit in ipairs(stop_candidates) do
                local filter_result = u_pos_filter(ec_unit.id)
                if not filter_result then
                    changed_something = true
                    ec_unit.status.diversion = false
                    ec_unit:extract()
                    ec_unit:to_map(false)
                    wesnoth.wml_actions.animate_unit {
                            flag = "landing",
                            with_bars = true,
                            T.filter { id = ec_unit.id }
                    }
                end
        end
        if not is_undo then
                wesnoth.experimental.game_events.set_undoable(true)
                if changed_something then
                        wesnoth.experimental.game_events.add_undo_actions(function(_)
                                status_anim_update(true)
                        end)
                end
        end
end

wesnoth.game_events.add_repeating("moveto, die, recruit, recall", function(_)
        status_anim_update(false)
end)
