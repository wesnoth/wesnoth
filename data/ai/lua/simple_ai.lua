local ai = wesnoth.dofile('ai/lua/ai.lua')

function ai:play_turn()
        --! only a few thousands lines have to be added here to complete the ai
	my_leader = wesnoth.get_units( {canrecruit="yes", side=self.side})[1]
   	self:move_full(my_leader,{x=11, y=4})
end

return ai
