
-- Loads the WC2 'engine' that is making bonus points, items, training etc work.

wc2_convert = wesnoth.require("./../shared_utils/wml_converter.lua")

wc2_ability_events = wesnoth.require("./ability_events.lua")
wc2_artifacts = wesnoth.require("./artifacts.lua")
wc2_bonus = wesnoth.require("./bonus.lua")
wc2_color = wesnoth.require("./color.lua")
wc2_dropping = wesnoth.require("./dropping.lua")
wc2_effects = wesnoth.require("./effects.lua")
wc2_heroes = wesnoth.require("./heroes.lua")
wc2_pickup_confirmation_dialog = wesnoth.require("./pickup_confirmation_dialog.lua")
wc2_random_names = wesnoth.require("./random_names.lua")
wc2_recall = wesnoth.require("./recall.lua")
wc2_supply = wesnoth.require("./supply.lua")
wc2_training = wesnoth.require("./training.lua")
wc2_unittypedata = wesnoth.require("./unittypedata.lua")
wc2_utils = wesnoth.require("./utils.lua")

wc2_wiki = wesnoth.require("./wocopedia/help.lua")

wc2_invest = wesnoth.require("./invest/invest.lua")
wc2_invest_show_dialog = wesnoth.require("./invest/invest_show_dialog.lua")
wc2_invest_tellunit = wesnoth.require("./invest/invest_tellunit.lua")

wesnoth.require("./promote_commander.lua")
