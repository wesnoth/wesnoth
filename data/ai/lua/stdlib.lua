
return {

init = function(ai)

--! ===========================

--! say hello to player via a message
function ai:say_hello()
       wesnoth.message(string.format("Hello from Lua AI which controls side %d! It's turn %d.", self.side, wesnoth.current.turn ))
end

--! ===========================
end
}

