
return {

init = function(ai,data)

--! ===========================
--! all functions here should start with an underscore to indicate that they come from wesnoth core

--! say hello to player via a message
function ai:_say_hello()
       wesnoth.message(string.format("Hello from Lua AI which controls side %d! It's turn %d.", self.side, wesnoth.current.turn ))
end

--! ===========================
end
}

