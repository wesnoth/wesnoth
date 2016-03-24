--! #textdomain wesnoth

example_ca = {}

function example_ca:evaluation(ai)
    wesnoth.message("External CA evaluation says hi.")

    return 10000
end

function example_ca:execution(ai)
    wesnoth.message("External CA execution attacks.")

    -- Note that there is no check whether these attacks are possible.
    -- The CA will therefore be blacklisted the second time it gets called.
    ai.attack(2, 12, 3, 12, 1, 1)
    ai.attack(3, 13, 3, 12, 2, 1)
    ai.attack(3, 11, 3, 12)
end

return example_ca