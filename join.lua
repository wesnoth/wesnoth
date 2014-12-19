-- join.lua --
-- Try to join a game called "Test"

local function plugin()

  local function log(text)
    std_print("join: " .. text)
  end

  local counter = 0

  local events, context, info

  local helper = wesnoth.require("lua/helper.lua")

  local function find_test_game(info)
    local g = info.game_list()
    if g then
      local gamelist = helper.get_child(g, "gamelist")
      if gamelist then
        for i = 1, #gamelist do
          local t = gamelist[i]
          if t[1] == "game" then
            local game = t[2]
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

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for titlescreen or lobby")
  until info.name == "titlescreen" or info.name == "Multiplayer Lobby"

  while info.name == "titlescreen" do
    context.play_multiplayer({})
    log("playing multiplayer...")
    events, context, info = coroutine.yield()
  end

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for lobby")
  until info.name == "Multiplayer Lobby"

  events, context, info = coroutine.yield()

  context.chat({message = "waiting for test game to join..."})

  local test_game = nil

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for test game")

    for i,v in ipairs(events) do
      if v[1] == "chat" then
        std_print("chat:", v[2].message)
      end
    end

    test_game = find_test_game(info)
  until test_game

  log("found a test game, joining... id = " .. test_game)
  context.chat({message = "found test game"})
  context.select_game({id = test_game})

  events, context, info = coroutine.yield()

  context.join({})

  events, context, info = coroutine.yield()

  repeat
    if context.join then
      context.join({})
    else
      std_print("did not find join...")
    end

    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for leader select dialog")
  until info.name == "Dialog" or info.name == "Multiplayer Wait"

  if info.name == "Dialog" then
    log("got a leader select dialog...")
    context.set_result({result = 0})
    events, context, info = coroutine.yield()

    repeat
      events, context, info = coroutine.yield()
      idle_text("in " .. info.name .. " waiting for mp wait")
    until info.name == "Multiplayer Wait"
  end

  log("got to multiplayer wait...")
  context.chat({message = "ready"})

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for game")
  until info.name == "Game"

  log("got to a game context...")

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for not game")
  until info.name ~= "Game"

  log("left a game context...")

  repeat
    context.quit({})
    log("quitting a " .. info.name .. " context...")
    events, context, info = coroutine.yield()
  until info.name == "titlescreen"

  context.exit({code = 0})
  coroutine.yield()
end

return plugin
