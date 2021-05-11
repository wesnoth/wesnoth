-- simulate-lobby-activity.lua --
-- Goes to the MP lobby and chats and creates and leaves games forever. --

local function create_game(context)
  local events, info

  context.create({})

  repeat
    events, context, info = coroutine.yield()
  until info.name == "Multiplayer Create"

  context.select_type({type = "scenario"})
  local s = info.find_level({id = "test1"})
  context.select_level({index = s.index})
  context.set_name({name = tostring(mathx.random(999999))})
  context.update_settings({registered_users = false})

  events, context, info = coroutine.yield()

  context.create({})
end

local function exit_game(context)
  local events, info

  repeat
    context.quit({})
    events, context, info = coroutine.yield()
  until info.name == "titlescreen"

  context.exit({code = 0})
  coroutine.yield()
end

return function()
  local events, context, info

  repeat
    events, context, info = coroutine.yield()
  until info.name == "titlescreen" or info.name == "Multiplayer Lobby"

  while info.name == "titlescreen" do
    context.play_multiplayer({})
    events, context, info = coroutine.yield()
  end

  repeat
    events, context, info = coroutine.yield()
  until info.name == "Multiplayer Lobby"

  -- Reached the lobby. Random delay before we start actually simulating activity.
  -- This is here to avoid a situation where activity arrives in bursts after a script
  -- has launched, say, 100 copies of Wesnoth at the same time.
  wesnoth.interface.delay(mathx.random(15000))

  events, context, info = coroutine.yield()

  local in_staging = false

  while true do
    if info.name ~= "Multiplayer Lobby" and info.name ~= "Multiplayer Staging" then
      -- most often, this means that the server was terminated -> stop generating traffic
      exit_game(context)
    end

    if mathx.random() > 0.1 then
      -- chat message
      local messages = {"asdf", "qwerty", "zxc"}
      context.chat({message = messages[mathx.random(#messages)]})
    else
      -- toggle between creating a game and leaving it
      if not in_staging then
        create_game(context)
        in_staging = true
      else
        repeat
          context.quit({})
          events, context, info = coroutine.yield()
        until info.name == "Multiplayer Lobby"
        in_staging = false
      end
    end

    wesnoth.interface.delay(15000)

    events, context, info = coroutine.yield()
  end
end
