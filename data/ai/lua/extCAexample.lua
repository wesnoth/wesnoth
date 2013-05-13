--! #textdomain wesnoth

example_ca = {}

function example_ca:eval(ai)
	wesnoth.message("External eval says hi!")
	
	return 10000
end

function example_ca:exec(ai)
	wesnoth.message("External CA exec attacks!")
	ai.attack(2, 12, 3, 12, 1, 1) -- showcasing the presence of the AI table
end

return example_ca