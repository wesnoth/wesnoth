-- host.lua --
-- Try to host a game called "Test"

local function plugin()

  local function log(text)
    std_print("host: " .. text)
  end

  local counter = 0

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

  context.chat{message = "hosting"}
  log("creating a game")
  context.create{}

  events, context, info = wesnoth.plugin.wait_until("Multiplayer Create", function(name)
    idle_text("in " .. name .. " waiting for create")
  end)

  context.select_type{type = "scenario"}
  local s = info.find_level{id = "test1"}
  if s.index < 0 then
    log(" error: Could not find scenario with id=test1")
  end
  context.select_level{index = s.index}

  log("configuring a game")
  context.set_name{name = "Test"}
  context.update_settings{registered_users = false}

  events, context, info = wesnoth.plugin.next_slice()

  context.create{}

  local ready = nil
  repeat
    events, context, info = wesnoth.plugin.next_slice()
    for i,v in ipairs(events) do
      if v.tag == "chat" then
        std_print(events[i].contents.message)
        if v.contents.message == "ready" then
          ready = true
        end
      end
    end
    idle_text("in " .. info.name .. " waiting for ready in chat")
  until ready

  log("starting game...")
  context.chat{message = "starting"}
  context.launch{}

  events, context, info = wesnoth.plugin.wait_until("Game", function(name)
    idle_text("in " .. name .. " waiting for game")
  end)

  log("got to a game context...")

  events, context, info = wesnoth.plugin.wait_until_not("Game", function(name)
    idle_text("in " .. name .. " waiting for not game")
  end)

  log("left a game context...")

  repeat
    context.quit({})
    log("quitting a " .. info.name .. " context...")
    events, context, info = wesnoth.plugin.next_slice()
  until info.name == "titlescreen"

  context.exit({code = 0})
end

return plugin
