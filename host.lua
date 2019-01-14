-- host.lua --
-- Try to host a game called "Test"

local function plugin()

  local function log(text)
    std_print("host: " .. text)
  end

  local counter = 0

  local events, context, info

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

  local tries = 0
  while info.name == "titlescreen" and tries < 100 do
    context.play_multiplayer({})
    tries = tries + 1
    log("playing multiplayer...")
    events, context, info = coroutine.yield()
  end
  if info.name == "titlescreen" then
    context.exit({code = 1})
    coroutine.yield()
  end

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for lobby")
  until info.name == "Multiplayer Lobby"

  context.chat({message = "hosting"})
  log("creating a game")
  context.create({})

  repeat
    events, context, info = coroutine.yield()
    idle_text("in " .. info.name .. " waiting for create")
  until info.name == "Multiplayer Create"

  context.select_type({type = "scenario"})
  local s = info.find_level({id = "test1"})
  if s.index < 0 then
	log(" error: Could not find scenario with id=test1")
  end
  context.select_level({index = s.index})

  log("configuring a game")
  context.set_name({name = "Test"})
  context.update_settings({registered_users = false})

  events, context, info = coroutine.yield()

  context.create({})

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

  log("starting game...")
  context.chat({message = "starting"})
  context.launch({})

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
