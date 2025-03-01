-- start-campaign.lua --
-- Try to start a campaign, recruit a unit, move to a village, and end turn

local function plugin(events, context, info)
  local function log(text)
    std_print("start-campaign: " .. text)
  end

  local counter = 0

  local function idle_text(text)
    counter = counter + 1
    if counter >= 100 then
      counter = 0
      log("idling " .. text)
    end
  end

  log("hello world from " .. info.name)

  events, context, info = wesnoth.plugin.wait_until("titlescreen", function(name)
    idle_text("in " .. name .. " waiting for titlescreen")
  end)

  local args = info.command_line().args or {}
  local campaign_id = args[1] or "The_South_Guard"

  local tries = 0
  while info.name == "titlescreen" and tries < 100 do
    context.play_campaign({})
    tries = tries + 1
    log("playing campaign...")
    events, context, info = coroutine.yield()
  end
  if info.name == "titlescreen" then
    context.exit{code = 1}
    return
  end

  events, context, info = wesnoth.plugin.wait_until("Campaign Selection", function(name)
    idle_text("in " .. name .. " waiting for campaign_selection")
  end)

  local s = info.find_level{id = campaign_id}
  if s.index < 0 then
    log(" error: Could not find campaign with id=" .. campaign_id)
  end
  log("selected "..campaign_id)
  context.select_level({index = s.index})

  events, context, info = wesnoth.plugin.next_slice()

  log("creating game")
  context.create{}
  std_print('A')

  events, context, info = wesnoth.plugin.wait_until_any({"Game", "Campaign Configure"}, function(name)
    idle_text("in " .. name .. " waiting for game or configure")
  end)

  std_print('B')
  if info.name == "Campaign Configure" then
    log("skipping configure")
    context.launch{}
    events, context, info = wesnoth.plugin.wait_until("Game", function(name)
      idle_text("in " .. name .. " waiting for game")
    end)
  end
  std_print('C')

  log("got to a game context...")

  repeat
    idle_text("in " .. info.name .. ", waiting to gain control")
    events, context, info = wesnoth.plugin.next_slice()
    if info.name == "Dialog" then
      context.skip_dialog{}
    end
  until info.name == "Game" and info.can_move().can_move
  wesnoth.game_config.debug = true

  local my_side, start_loc, keep_loc, on_keep, castle_loc, first_recruit
  wesnoth.plugin.execute(context, function()
    log('finding a spot to recruit')
    my_side = wesnoth.current.side
    std_print('my_side='..my_side)
    start_loc = wesnoth.current.map.special_locations[my_side]
    std_print('start_loc=('..start_loc.x..','..start_loc.y..')')
    local ai = wesnoth.sides.debug_ai(my_side).ai
    std_print('ai='..tostring(ai))
    local x,y = ai.suitable_keep(wesnoth.units.get(start_loc))
    if x and y then
      keep_loc = wesnoth.named_tuple({x,y}, {'x','y'})
      std_print('keep_loc=('..keep_loc.x..','..keep_loc.y..')')
    end
    on_keep = start_loc == keep_loc
    std_print('on_keep='..tostring(on_keep))
    castle_loc = wesnoth.map.find{formula = 'castle', wml.tag.filter_adjacent{x = keep_loc.x, y = keep_loc.y}}[1]
    std_print('castle_loc=('..castle_loc.x..','..castle_loc.y..')')
    first_recruit = wesnoth.sides[my_side].recruit[1]
    std_print('first_recruit='..first_recruit)
  end)

  events, context, info = wesnoth.plugin.next_slice()
  log(wesnoth.as_text(events))
  events, context, info = wesnoth.plugin.next_slice()

  if keep_loc then
    log("found a suitable keep at (" .. keep_loc.x .. "," .. keep_loc.y .. ")")
  else
    log("didn't find a suitable keep")
  end

  while not on_keep do
    context.synced_command{wml.tag.move{x = {start_loc.x, keep_loc.x}, y = {start_loc.y, keep_loc.y}}}
    wesnoth.plugin.execute(context, function()
      local u = wesnoth.units.get(keep_loc)
      on_keep = u and u.side == my_side and u.canrecruit
    end)
    events, context, info = wesnoth.plugins.next_slice()
  end

  log("recruiting a " .. first_recruit)
  context.synced_command{
    wml.tag.recruit{type = first_recruit, x = castle_loc.x, y = castle_loc.y, wml.tag.from{x = keep_loc.x, y = keep_loc.y}},
  }
  events, context, info = wesnoth.plugin.next_slice()
  log("ending turn")
  context.end_turn{}

  repeat
    idle_text("in " .. info.name .. ", waiting to gain control")
    events, context, info = wesnoth.plugin.next_slice()
    if info.name == "Dialog" then
      context.skip_dialog{}
    end
  until info.name == "Game" and info.can_move().can_move

  context.quit{}

  events, context, info = wesnoth.plugin.wait_until_not("Game", function(name)
    idle_text("in " .. name .. " waiting for not game")
  end)

  log("left a game context...")

  while info.name ~= "titlescreen" do
    log("quitting a " .. info.name .. " context...")
    context.quit{}
    events, context, info = wesnoth.plugin.next_slice()
  end

  context.exit{code = 0}
end

return plugin
