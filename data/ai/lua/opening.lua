-- An extremely simplistic example of an AI that does certain fixed moves
-- It's meant to be used as a custom stage preceding the RCA stage
-- This is a highly specific example and probably not suitable to be directly used in other places

if wesnoth.current.turn == 1 then
    ai.recruit('Skeleton Archer', 11,21)
    ai.recruit('Dark Adept', 11,22)
    ai.recruit('Dark Adept', 10,22)
    ai.recruit('Skeleton Archer', 9,22)
    ai.recruit('Ghost', 11,24)
    ai.move(11,23, 14,22)
elseif wesnoth.current.turn == 2 then
    ai.move(11,21, 13,17)
    ai.move(11,22, 13,18)
    ai.move(10,22, 7,19)
    ai.move(9,22, 4,22)
    ai.move(11,24, 18,24)
    ai.move(14,22, 11,23)
    ai.recruit('Dark Adept', 11,21)
    ai.recruit('Dark Adept', 11,22)
elseif wesnoth.current.turn == 3 then
    ai.move(18,24, 20,22)
    ai.move(15,19, 17,17)
    ai.move(4,22, 5,18)
    ai.recruit('Skeleton Archer', 11,21)
elseif wesnoth.current.turn == 4 then
    ai.recruit('Skeleton Archer', 11,21)
else
    ai.recruit('Skeleton Archer', 11,21)
    ai.recruit('Dark Adept', 11,22)
end
