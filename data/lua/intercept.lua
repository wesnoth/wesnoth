--attacker is e
--defender is u

local _ = wesnoth.textdomain "wesnoth-help" -- !! This takes away "_" as commonly used unused variable !!

local c = wesnoth.current.event_context
local x1 = c.x1
local y1 = c.y1
if x1 ~= c.unit_x or y1 ~= c.unit_y then
    return
end
local u = wesnoth.units.get(x1,y1)

local enemies = wesnoth.units.find_on_map({{"filter_adjacent",{id=u.id, is_enemy = true}},{"has_attack",{special_type = "intercept"}}}) --TODO just active weapons

for iter = #enemies, 1, -1 do
    local e = enemies[iter]
    local special
	for _, attack in ipairs(e.attacks) do
        special = wml.child_array(attack.specials,"intercept")[1]
        if special then
            break
        end
    end
    if not e.variables.intercept_used then
        e.variables.intercept_used = 0
    end
    if special.uses_per_turn and special.uses_per_turn ~= -1 and special.uses_per_turn <= e.variables.intercept_used then
        table.remove(enemies,iter)
    end
end

if #enemies == 0 then return end

local animator =      wesnoth.units.create_animator()
local sheath_weapon = wesnoth.units.create_animator()

for iter,e in ipairs(enemies) do
    local attacking_weapon
    for i, attack in ipairs(e.attacks) do
        if wml.child_array(attack.specials,"intercept")[1] then
            attacking_weapon = i
            break
        end
    end
    local loc = e.loc
    local loc2 = u.loc
    local __, __, e_stats, u_stats = wesnoth.simulate_combat(e,attacking_weapon, u)
    local w = e.attacks[attacking_weapon]
    local w2 = nil
    local w2_cfg = nil
    if u_stats.name ~= nil then
        w2 = u.attacks[u_stats.name]
        w2_cfg = w2.__cfg
    end
    e.facing=wesnoth.map.get_relative_dir(e.x, e.y, u.x, u.y) --doesnt work if unit has no animation for the draw...
    animator:add(e, "draw_weapon", "hit",{with_bars = true, primary = w, secondary = w2})
    sheath_weapon:add(e, "sheath_weapon", "hit",{with_bars = true, primary = w, secondary = w2})
    if iter == 1 then --when drawing weapons u is already looking towards first e
        u.facing = wesnoth.map.get_relative_dir(u.x, u.y,e.x, e.y)
        animator:add(u, "draw_weapon", "hit",{with_bars = true, primary = w, secondary = w2})
        sheath_weapon:add(u, "sheath_weapon", "hit",{with_bars = true, primary = w, secondary = w2})
    end
end
animator:run()
animator:clear()
for iter,e in ipairs(enemies) do
    e.variables.intercept_used = e.variables.intercept_used and e.variables.intercept_used + 1 or 1
    local attacking_weapon
    for i, attack in ipairs(e.attacks) do
        if wml.child_array(attack.specials,"intercept")[1] then
            attacking_weapon = i
            break
        end
    end
    local last_strike = false
    local strike_number = 0
    while not last_strike do
        strike_number = strike_number + 1
        e.facing = wesnoth.map.get_relative_dir(e.x, e.y, u.x, u.y)
        u.facing = wesnoth.map.get_relative_dir(u.x, u.y,e.x, e.y)
        local loc = e.loc
        local loc2 = u.loc
        local __, __, e_stats, u_stats = wesnoth.simulate_combat(e,attacking_weapon, u)-- Defensive specials of enemy weapon do apply, but enemy doesn't fight back - makes sense for things like shield block
        local w = e.attacks[attacking_weapon]

        local strike_limit = wml.child_array(w.specials,"intercept")[1].strike_limit
        if not strike_limit or strike_limit == -1 or strike_limit > e_stats.num_blows then
            strike_limit = e_stats.num_blows
        end
        last_strike = strike_number >= strike_limit
        local w2 = nil
        local w2_cfg = nil
        if u_stats.name ~= nil then
            w2 = u.attacks[u_stats.name]
            w2_cfg = w2.__cfg
        end
        if mathx.random() < (e_stats.chance_to_hit/100) then

            local damage_inflicted = math.floor(e_stats.damage)

            local drain = 0
            if e_stats.drains then
                drain = math.floor(math.min((math.min(damage_inflicted,u.hitpoints) * e_stats.drain_percent / 100) + e_stats.drain_constant, math.max(e.max_hitpoints - e.hitpoints,0)))
            end
            local text = ""
            if drain ~= 0 then
                text = text .. drain
            end

            local text2 =  damage_inflicted .. "\n"
            local add_tab = false
            local gender = u.gender
            local statuses = {}

            local function set_status(ability, name, male_string, female_string, sound)
                if not e_stats[ability] or u.status[name] then return end
                if gender == "female" then
                    text2 = text2 .. tostring(female_string) .. "\n"
                else
                    text2 = text2 .. tostring(male_string) .. "\n"
                end

                statuses[#statuses+1]=name
                add_tab = true

                if sound then -- for unhealable, that has no sound
                    wesnoth.audio.play(sound)
                end
            end
            --TODO: check if specials are active once a convenient API is provided
            if not u.status.unpoisonable then
                set_status("poisons","poisoned", _"poisoned", _"female^poisoned", "poison.ogg")
            end
            set_status("slows","slowed", _"slowed", _"female^slowed", "slowed.wav")
            set_status("petrifies","petrified", _"petrified", _"female^petrified", "petrified.ogg")
            if add_tab then
                text2 = "     " .. text2
            end

            animator:add(e, "attack", "hit",{with_bars = true, primary = w, secondary = w2, text = text,color={0,255,0}})
            animator:add(u, "defend", "hit",{with_bars = true, primary = w, secondary = w2, text = text2,color={255,0,0}})
            animator:run()
            animator:clear()
            for i=1, #statuses do
                u.status[statuses[i]] = true
            end
            e.hitpoints = e.hitpoints + drain
            u.hitpoints = u.hitpoints - e_stats.damage
            wesnoth.game_events.fire("attacker_hits", e.loc, u.loc, {{"first",w.__cfg},{"second",w2_cfg},damage_inflicted=damage_inflicted})
                if u.valid == nil or u.hitpoints <= 0 then
                sheath_weapon:run()
                wesnoth.game_events.fire("attack end", u.loc, loc2, {{"first",wml.get_child(wesnoth.current.event_context,"weapon")},{"second",wml.get_child(wesnoth.current.event_context,"second_weapon")}})
                if u.valid == nil then
                    --u doesn't exist PROPPER DEATH EVENTS should be fired already, by some other code (modders thing)
                    goto enemy_killed
                end
                wesnoth.game_events.fire("last breath", loc2, e.loc, {{"first",w2_cfg},{"second",w.__cfg}}) --inverted
                animator:add(u, "death", "hit",{with_bars = false, primary = w2, secondary = w}) --inverted
                animator:run()
                animator:clear()
                u_level = u.level
                u_id= u.id
                wesnoth.game_events.fire("die", u.loc, e.loc, {{"first",w2_cfg},{"second",w.__cfg}}) --inverted
                wesnoth.wml_actions.kill({ id = u_id, animate = false, fire_event = false})
                if u_level == 0 then
                    e.experience = wesnoth.game_config.kill_experience * 0.5
                    e:advance(true,true)
                else
                    e.experience = e.experience +(u_level * wesnoth.game_config.kill_experience)
                    e:advance(true,true)
                end
                -- TODO doesn't work for berserker (not intended, just a warning for future me), untested with plague
                goto enemy_killed
            end
        else
            animator:add(e, "attack", "miss",{with_bars = true, primary = w, secondary = w2})
            animator:add(u, "defend", "miss",{with_bars = true, primary = w2, secondary = w})--inverted
            animator:run()
            animator:clear()
            wesnoth.game_events.fire("attacker_misses", e.loc, u.loc, {{"first",w.__cfg},{"second",w2_cfg}})
        end
    end
end
::enemy_killed::
sheath_weapon:run()
