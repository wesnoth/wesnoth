--! #textdomain wesnoth

example_ca = {}

function example_ca:eval(ai)
	wesnoth.message("External eval says hi!")
	
	return 10000
end

function example_ca:exec(ai)
	wesnoth.message("External CA exec attacks!")
	if (ai.store.n == nil) then
	    ai.store.n = 0
	end
	ai.store.n = ai.store.n + 5
	wesnoth.message("xtcastore = ", tostring(ai.store.n))
	ai.attack(2, 12, 3, 12, 1, 1) -- showcasing the presence of the AI table
end

return example_ca