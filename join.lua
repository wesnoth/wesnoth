-- join.lua --
-- Try to join a game called "Test"

local function plugin()

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
      std_print("join: idling " .. text)
    end
  end

  std_print("join: hello world")

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for titlescreen or lobby")
  until info.name == "titlescreen" or info.name == "Multiplayer Lobby"

  while info.name == "titlescreen" do
    context.play_multiplayer({})
    std_print("join: playing multiplayer...")
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

  std_print("join: found a test game, joining... id = " .. test_game)
  context.chat({message = "found test game"})
  context.select_game({id = test_game})

  events, context, info = coroutine.yield()

  context.join({})

  repeat
    events, context, info = coroutine.yield()
    if context.join then
      context.join({})
    end

    idle_text("in " .. info.name .. " waiting for leader select dialog")
  until info.name == "Dialog"

  std_print("join: got a leader select dialog...")
  context.set_result({result = 0})
  events, context, info = coroutine.yield()

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for mp wait")
  until info.name == "Multiplayer Wait"

  std_print("join: got to multiplayer wait...")
  context.chat({message = "ready"})

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for game")
  until info.name == "Game"

  std_print("join: got to a game context...")

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for not game")
  until info.name ~= "Game"

  std_print("join: left a game context...")

  repeat
    context.quit({})
    std_print("join: quitting a " .. info.name .. " context...")
    events, context, info = coroutine.yield()
  until info.name == "titlescreen"

  context.exit({code = 0})
  coroutine.yield()
end

return plugin
