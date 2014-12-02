-- host.lua --
-- Try to host a game called "Test"

local function plugin()

  local counter = 0

  local events, context, info

  local helper = wesnoth.require("lua/helper.lua")

  local function idle_text(text)
    counter = counter + 1
    if counter >= 100 then
      counter = 0
      std_print("host: idling " .. text)
    end
  end

  std_print("host: hello world")

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for titlescreen")
  until info.name == "titlescreen"

  context.play_multiplayer({})
  std_print("host: playing multiplayer...")

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for lobby")
  until info.name == "Multiplayer Lobby"

  context.chat({message = "hosting"})
  std_print("host: creating a game")
  context.create({})

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for create")
  until info.name == "Multiplayer Create"

  std_print("host: configuring a game")
  context.create({})

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for configure")
  until info.name == "Multiplayer Configure"

  context.set_name({name = "Test"})
  std_print("host: hosting a game")
  context.launch({})

  ready = nil
  repeat
    events, context, info = coroutine.yield()
    for i,v in ipairs(events) do
      if v[1] == "chat" then
        std_print(events[i][2])
        if v[2].message == "ready" then
          ready = true
        end
      end
    end
    idle_text("in " .. info.name .. " waiting for ready in chat")
  until ready

  std_print("host: starting game...")
  context.chat({message = "starting"})
  context.launch({})

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for game")
  until info.name == "Game"

  std_print("host: got to a game context...")

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for not game")
  until info.name ~= "Game"

  std_print("host: left a game context...")

  repeat
    context.quit({})
    std_print("join: quitting a " .. info.name .. " context...")
    events, context, info = coroutine.yield()
  until info.name == "titlescreen"

  context.exit({code = 0})
  coroutine.yield()
end

return plugin
