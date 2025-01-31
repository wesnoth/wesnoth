-- join.lua --
-- Try to join a game called "Test"

local function plugin()

  local function log(text)
    std_print("join: " .. text)
  end

  local counter = 0

  local function find_test_game(game_info)
    local g = game_info.game_list()
    if g then
      local gamelist = wml.get_child(g, "gamelist")
      if gamelist then
        for i = 1, #gamelist do
          local t = gamelist[i]
          if t.tag == "game" then
            local game = t.contents
            if game.scenario == "Test" then
              return game.id
            end
          end
        end
      end
    end
    return nil
  end

  local function idle_text(text)
    counter = counter + 1
    if counter >= 100 then
      counter = 0
      log("idling " .. text)
    end
  end

  log("hello world")

  local events, context, info = wesnoth.plugin.wait_until_any({"titlescreen", "Multiplayer Lobby"}, function(name)
    idle_text("in " .. name .. " waiting for titlescreen or lobby")
  end)

  local tries = 0
  while info.name == "titlescreen" and tries < 100 do
    context.play_multiplayer{}
    tries = tries + 1
    log("playing multiplayer...")
    events, context, info = wesnoth.plugin.next_slice()
  end
  if info.name == "titlescreen" then
    context.exit{code = 1}
    return
  end

  events, context, info = wesnoth.plugin.wait_until("Multiplayer Lobby", function(name)
    idle_text("in " .. name .. " waiting for lobby")
  end)

  events, context, info = wesnoth.plugin.next_slice()

  context.chat{message = "waiting for test game to join..."}

  local test_game = nil

  repeat
    events, context, info = wesnoth.plugin.next_slice()
    idle_text("in " .. info.name .. " waiting for test game")

    for i,v in ipairs(events) do
      if v.tag == "chat" then
        std_print("chat:", v.contents.message)
      end
    end

    test_game = find_test_game(info)
  until test_game

  log("found a test game, joining... id = " .. test_game)
  context.chat{message = "found test game"}
  context.select_game{id = test_game}

  events, context, info = wesnoth.plugin.next_slice()

  context.chat{message = "going to join"}

  context.join{}

  events, context, info = wesnoth.plugin.next_slice()

  -- Don't know why THIS context has no chat member but it doesn't
  -- Adding the guard if to bypass a script crash and get mp_tests running.
  -- GAL 28NOV2017
  if context.chat then
    context.chat{message = "done first join"}
  end

  while not (info.name == "Dialog" or info.name == "Multiplayer Join") do
    if context.join then
      context.join{}
    else
      std_print("did not find join...")
    end

    events, context, info = wesnoth.plugin.next_slice()
    idle_text("in " .. info.name .. " waiting for leader select dialog")
  end

  if info.name == "Dialog" then
    log("got a leader select dialog... id=" .. info.id().id)
    context.skip_dialog{}
    events, context, info = wesnoth.plugin.wait_until("Multiplayer Join", function(name)
      idle_text("in " .. name .. " waiting for mp join")
    end)
  end

  log("got to multiplayer join...")
  context.chat{message = "ready"}

  events, context, info = wesnoth.plugin.wait_until("Game", function(name)
    idle_text("in " .. name .. " waiting for game")
  end)

  log("got to a game context...")

  repeat
    events, context, info = wesnoth.plugin.next_slice()
    idle_text("in " .. info.name .. " waiting for the last scenario")
  until info.scenario_name ~= nil and info.scenario_name().scenario_name == "Multiplayer Unit Test test2"

  events, context, info = wesnoth.plugin.wait_until_not("Game", function(name)
    idle_text("in " .. name .. " waiting for not game")
  end)

  log("left a game context...")

  repeat
    context.quit{}
    log("quitting a " .. info.name .. " context...")
    events, context, info = wesnoth.plugin.next_slice()
  until info.name == "titlescreen"

  context.exit{code = 0}
end

return plugin
