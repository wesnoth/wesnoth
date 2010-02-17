local ai = { side = side }

--! do a full move from FROM to TO. Unit's movement points are set to 0 after a move
function ai:move_full(FROM,TO)
	 wesnoth.ai_execute_move(self.side,FROM.x,FROM.y,TO.x,TO.y,1)
end

--! do a partial move from FROM to TO.
function ai:move(FROM,TO)
	 wesnoth.ai_execute_move(self.side,FROM.x,FROM.y,TO.x,TO.y,0)
end

--! do a simple attack using aggression 0.5 and choosing best weapon
function ai:attack_simple(ATT,DEF)
    	 wesnoth.ai_execute_attack(self.side,ATT.x,ATT.y,DEF.x,DEF.y,-1,0.5)
end

--! recruit a UNIT_NAME on LOC
function ai:recruit(UNIT_NAME,LOC)
   	 wesnoth.ai_execute_recruit(self.side,UNIT_NAME,LOC.x,LOC.y)
end

--! recall a UNIT_ID at LOC
function ai:recall(UNIT_ID,LOC)
       	 wesnoth.ai_execute_recall(self.side,UNIT_ID,LOC.x,LOC.y)
end

--! stop a unit at LOC - remove MOVES and/or remove ATTACK
function ai:stop(LOC,MOVES,ATTACKS)
	 wesnoth.ai_execute_stopunit(self.side,LOC.x,LOC.y,MOVES,ATTACKS)
end

return ai
