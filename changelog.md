## Version 1.14.4+dev
 ### AI
  * Fix AI ignoring teleport locations when moving leader to leader goals.
 ### Campaigns
  * Heir to the Throne:
    * S6: Allow canceling an attack when a move+attack spawns enemy units (issue #3459)
    * S6: Change the trigger for spawning undead reinforcements (issue #3459)
    * S17: Add dialog explaining the lava game mechanic and an easter egg (issue #3473)
  * Liberty:
    * S3: Add story and war drums music
  * Northern Rebirth:
    * S12: empty sides are hidden
    * S13: allied leaders whose death causes defeat won't leave their keep and
      the lich brothers and Krash got more gold
  * Sceptre of Fire:
    * S4: a cave entrance has been added.
    * S5: Gryphon Riders are now available starting form S5 (formerly S3).
    * S7: it's now impossible to kill all pursuers.
  * Secrets of the Ancients:
    * Fix dialog said by wrong unit & revert previous fix (issue #3294)
  * The Hammer of Thursagan:
    * S10: Fix objectives requiring too much of the player
  * The South Guard:
    * S2: the case where Deoran is not sent to the Citadel is handled better.
    * S2: enemy units will no longer neutralize villages instead of capturing or burning them.
      This also means certain units will no longer be hidden in the Game Stats list.
    * S4: bodyguards are never spawned next to other units, and always in forest.
    * S5: zombies have now one castle with two leaders.
    * S6a: mermen leave after this scenario and will no longer be available.
    * S7a & S8a: a certain hero may now die.
   * Tutorial:
     * Added more information to the Status Table prompt about how to access it (issue #2883).
   * Under the Burning Suns:
     * New graphics for Eloh.
 ### Multiplayer
   * A New Land: Fixed village gold being 1 instead of 2.
 ### Language and i18n
   * Updated translations: British English, Chinese (Traditional), Czech, French,
     German, Hungarian, Italian, Japanese, Scottish Gaelic, Ukrainian
 ### User Interface
   * On laptop computers we now show how much battery you have left
   * MacOS: Fixed moving the mouse with a touchpad also scrolling GUI dialogs
   * MacOS: Add build number to OS version report
   * Fix layout of Preferences window with some localizations (such as Czech)
   * Load Game now shows the chosen difficulty with the same name that you originally selected
   * Load Game now shows the modifications enabled in the selected savegame (issue #3495)
   * Force leader sprites larger than 72x72 to be downscaled in Load Game (issue #3474)
   * Add left padding to dialog messages shown with a portrait on the right (issue #1938)
   * Addon Manager uses clearer terms for sort order
   * Fix reversed sort order by unit level in Unit List and Recall List (#3370)
 ### WML engine
   * Fixed [animate_unit] and [heal_unit] preventing unit halo from disappearing if the unit
     dies later (issue #3509)
 ### Miscellaneous and bug fixes
   * Fix some minor problems with the macOS package
   * Fixed crash when trying to attack with a unit without usable weapons (issue #3424)
   * Fast Micro AI: Fix bug crashing the AI when units with chance-to-hit
     specials without id are on map
   * Fixed the debug mode Create Unit dialog crashing when changing the gender
     of the previous selection after causing the list to come up empty using
     the filter box.
   * Allow searching by race and internal unit type id with the Create Unit
     filter box as well.
   * wmlxgettext is now included in release packages, so a separate download is no longer required to use it with GUI.pyw.
   * Fixed possible crash when leaving a game as an observer #3017
   * Fixed require_scenario and require_era attributes.
   * Fixed skip replay when joining mp campaigns.
   * Fixed oos errors when undoing after ally chat.
   * Fixed crash whe pressing Reset replay. #3439
   * Fixed 'start game' locked when other players abort flg dialog.  #3452
   * Fixed game freezes on droiding when using dsu. #3453
   * Fixed editor cannot add starting location for > 9 players.
   * Fixed crash in ai code when a side has multiple leaders.
   * Fixed random start time not working. #3515
   * Fixed crash in lua tstring comparision. #3541
   * Fixed objectives at scenario start use old/cached values of conditions/variables (issue #3544)
   * Show an error message when trying to open the lua console not in debug mode
   * On enemy side's turn, don't scroll to that side's leader if it's invisible (#3492)
   * Fixed crash in recall list when no units matched the filter (#3475)
   * Fixed :droid ignore its second argument
   * Fixed :droid giving no feedback when successful

## Version 1.14.4
 ### Security Fixes
   * Fixed Lua being able to escape sandboxing via load/loadstring (CVE-2018-1999023).
 ### Add-ons server
   * Made it so plain-text .po catalogues in add-ons are detected and added to
     the list of translations for them.
 ### Campaigns
   * Dead Water:
     * In 'Tirigaz', take the situation into account of orcs being killed either
       first or by undead.
   * Delfador's Memoirs:
     * Fix hero units costing upkeep (issue #3277)
   * Eastern Invasion:
     * Fixed missing prisoners and loss of recallable units in 'Captured'.
   * Northern Rebirth:
     * Level 0 units are not available anymore after scenario 5
   * Secrets of the Ancients:
     * Adjust gender of enemies to better match story in S11 & 21 (issue #3294)
     * Simplify dialog to fix possible confusion in S16 (issue #3291)
     * Have nagas be able to recruit in S21 (issue #3293)
   * The South Guard:
     * S4: undead leader won't leave the castle anymore
     * S5: the untypical situation that one can defeat the lich before finding
       Urza Afalas is now handled
   * Under the Burning Suns:
     * S11: added custom graphics for the citadel.
     * S12: clarified the alien bodies' weaknesses.
     * Various visual improvements.
 ### Editor
   * Fixed saving a map as a scenario not enabling scenario editor tools.
 ### Graphics
   * New attack animation for the Peasant.
   * Tweaked the Ruffian's attack animation timing.
 ### Language and i18n
   * Updated translations: British English, Bulgarian, Chinese (Simplified),
     Chinese (Traditional), Czech, French, German, Hungarian, Japanese,
     Scottish Gaelic, Slovak, Spanish
 ### Lua API
   * Upgrade to Lua 5.3.5.
 ### Multiplayer
   * Non hosts can now change their faction in the mp wait dialog.
 ### Multiplayer server
   * Fixed lobby and whisper messages not having a maximum length.
   * Partly fixed the mp server breaking translatable strings.
 ### User interface
   * Improved the layout of the Statistics dialog.
   * Allow changing dropdown menu selections with the scrollwheel (FR #3251).
   * Fixed lobby chat box scrolling to top on a new message if it isn't at the
     bottom (issue #2789).
   * Fixed the unit preview pane not showing the default race icon when detailing
     a single unit's stats.
   * Sort units secondarily by XP in the unit list dialog.
   * Whiteboard related bugfixes
 ### WML engine
   * Fixed errors about WESNOTH_VERSION not being defined when trying to load
     add-ons that have preprocessor errors (issues #1924, #1634).
 ### Miscellaneous and bug fixes
   * Added an advanced preference to enable experimental PRNG combat.
   * Campfires use illumination instead of a different ToD.
   * Linux builds now enable security hardening by default.
   * Fixed MP admins being unable to observe private games.
   * Fixed MP faction, leader, and leader gender changes persisting even if the
     selection dialog is dismissed.
   * Fixed an issue with positioned sound sources ignoring the volume set in
     Preferences after going off the audible radius and back (issue #3280).
   * Fixed wmllint choking on gzipped binary files (e.g. gzipped tarballs).
   * Fixed wmllint crashing on nonexistent paths provided in the command line
     (issue #3286).
   * Slight changes to the objectives dialogue (pr #3309)
   * Greatly improved touch control support.
   * Fixed wmlindent crashing on nonexistent paths provided in the command line
     (issue #3346).
   * [do_command][attack] can no longer invoke disabled attacks.
   * [delay] is now skipped during preload events.
   * Fixed wrong simulated movement points when planning to capture a village.
   * Fixed attacks wrongly being disabled in the UI.

## Version 1.14.3
 ### AI
   * Fixed crash in presence of units with negative hitpoints (issue #3042).
   * Efficiency improvements to filter evaluations in the Goto Micro AI and some
     AI helper functions
 ### Campaigns
   * Dead Water:
     * Fix possibility of villagers blocking pickup of sword in S10 The Flaming
       Sword (issue #3207).
   * Descent Into Darkness:
     * New Parthyn map in S1 and S6.
   * Northern Rebirth:
     * Reduced starting gold and income in scenarios 10 to 13
   * Secrets of the Ancients:
     * Fix S09 Training Session not ending when all dark adepts die (issue #3192)
   * Tutorial:
     * S2: made enemy starting gold equal to that of the player.
   * Under the Burning Suns:
     * Added portrait and updated sprite for Giant Ant.
 ### Language and i18n
   * Updated translations: British English, Chinese (Traditional), Czech, French,
     Italian, Japanese, Scottish Gaelic, Spanish
 ### Multiplayer server
   * Fixed lan_server option not causing the server to exit once vacated, e.g.
     when using the Host Networked Game option from the Multiplayer menu in the
     game (issue #3206).
   * It is now possible to query the client version of other players.
 ### Music and sound effects
   * The music now changes immediately when you load a save file (issue #2602).
   * Fixed Lua errors when setting a music track that cannot be found when the
     playlist is already empty, e.g. if there's no music installed for the
     game (issue #3194).
 ### WML engine
   * Removed validation to ensure units cannot have negative hitpoints. We
     learned that the ability to create such units is documented, and thus
     disallowing it was an API change. Since API changes aren't allowed in
     the stable branch, we have reverted it.
 ### Miscellaneous and bug fixes
   * Fixed an occasional crash at the loading screen related to multi-thread
     access of the image cache.
   * [kill] animate=yes no longer scrolls to units through fog or shroud, thus
     matching 1.12's behavior again.
   * [message] displays the unit type name when a nameless unit speaks and no
     custom caption= is specified (issue #3211).
   * do_not_list=yes units are no longer excluded from the debug mode-only
     Create Unit dialog.
   * Fixed a rare issue where disabled attacks could cause the wrong attack to
     be initially selected in the Unit Attack dialog. This bug also had the
     potential to cause units to the wrong attack when  engaging or viewing
     damage calculations.
   * Fixed [scenario] map_file= being unusable in most circumstances.

## Version 1.14.2
 ### Campaigns
   * Northern Rebirth:
     * S02.1 Infested Caves: keep side 8 AI leader from wandering off too far
       and ending up on a keep with only one hex for recruiting.
     * S02.1 Infested Caves: AIs are less likely to kill each other in early
       game, which would make it harder for the player otherwise.
     * S02.1 Infested Caves: Dwarvish allies are also less likely to die.
     * S02.1 Infested Caves: minor map tweaks and improvements.
     * S05 The Pursuit: removed a bottleneck and tweaked Rod of Justice.
   * Sceptre of Fire:
     * In 'Caverns of Flame', fixed various issues with the volcano eruption.
   * Under the Burning Suns:
     * Various Quenoth unit graphics updates.
   * Descent into Darkness:
     * In 'A small Favor', disabled a not intended way to win the scenario.
   * Secrets of the Ancients:
     * Bats are transformed to normal ones already after S5
 ### Graphics
   * Fixed a minor team coloring mistake in the north-facing Revenant standing
     animation.
 ### Language and i18n
   * Updated translations: British English, Chinese (Simplified), Czech, French,
     Galician, German, Italian, Japanese, Scottish Gaelic, Spanish, Ukrainian
 ### User interface
   * List boxes (MP lobby game list in particular) now keep the scroll position
     when they change, instead of keeping the selected item visible (issue #3016).
   * Fixed MP lobby player list scrolling to top when it changes
   * Fixed the first unit sometimes not being selected when opening the Recall
     dialog.
   * Fixed a crash when using very large portraits in [message] (issue #2912)
   * Added a button to access the version info dialog to Preferences.
   * Removed player list sorting options in the MP lobby. The list is now
     automatically sorted alphabetically, friends first.
   * Rearranged campaign difficulty menu layout
 ### Terrains
   * Removed hidden terrains that were not supposed to make it to 1.14: ^Prg,
     ^Prgo, ^Pwd, ^Pwdo
 ### Miscellaneous and bug fixes
   * Fixed memory leak in terrain filter code. In a huge map with Silver Mages,
     it could leak several gigabytes of memory in a long session.
   * Fixed: unit halo remained after undoing a recall (issue #3065)
   * Fixed: unit halo intensity doubled during AMLA animation
   * [change_theme] no longer causes a Lua error when theme= is not specified
     instead of explicitly set to an empty string.
   * [change_theme] no longer requires running a separate action to update the
     UI afterwards (e.g. [redraw]) and the status panels are updated immediately.
     as well.
   * Lua random map generator: fixed error when flipping map; specifically this
     also fixes an assert at the start of HttT S7 (Sceptre of Fire) that had a
     50% chance of being triggered
   * Experimental AI: fixed recruiting not working on maps without enemies, such
     as the first two turns of Dark Forecast
   * Experimental AI: fixed recruiting evaluations sometimes not being updated
   * Replaced deprecated Lua code and all remaining uses of FOREACH and MESSAGE
     macros
   * Fixed [color_adjust] interacting poorly with time of day color shifts and
     values outside the [-255, 255] range (issue #3144).
   * Fixed a regression from 1.13.10 where modification option values couldn't
     be properly saved in arrays.
   * Added stricter validation to ensure units cannot have negative hitpoints,
     except during specific attack-related events.
   * Added deprecation notices for several macros that had them missing before.
   * [message] no longer scrolls to units through fog or shroud so it matches
     1.12's behavior.
   * Fixed animation-wide text_color and blend_color keys being overwritten. This
     fixes level-in and level-out animations sometimes fading to black instead of
     white.
   * Fixed [animate_unit] freezing the game when observing MP games (#2970).
   * Fixed carryover behaving differently when loading a start-of-scenario save
     (issue #3152).
   * Fixed turn replay function in MP.
   * Fixed savegames being created even when not needed (issue #3150).
   * Fixed handling of extra_recruit in planning mode (issue #3100).
   * Fixed handling of skirmisher in planning mode.
   * Fixed handling of filter_recall in planning mode.
   * Fixed possible segfault at game end.
   * Fixed require_resource in [resource].
   * Fixed require_scenario=yes not working with map_generation (issue #3105).

## Version 1.14.1
 ### Campaigns
   * Eastern Invasion:
     * In 'Captured', fixed units incorrectly costing upkeep after leveling up.
   * Secrets of the Ancients
     * Fixed minor unit naming inconsistencies (issues #2844 and #2846).
   * Under the Burning Suns
     * Added custom graphics for the broken tree in S1.
     * Various sprite and image updates.
     * Fixed a bug in the formation ability causing defense bonuses not being
       received in some cases.
 ### Language and i18n
   * Updated translations: British English, Chinese (Simplified), Czech, French,
     Galician, German, Italian, Japanese, Scottish Gaelic, Slovak, Spanish
   * Fixed Logging Options tooltips not being translatable (issue #2837).
   * Add command-line argument to disable the filtering of incomplete translations
     in the language selection list.
 ### Multiplayer server
   * Added support for matching user, IP, and email bans from a forums board
     when the forum_user_handler is enabled and active. (IP and email bans with
     wildcards are not supported yet.)
   * Fixed various instances of the server crashing under certain conditions.
 ### Performance
   * Added an option to disable the FPS limiter for a slight performance boost.
 ### Units
   * Changed the plural name for the merfolk race from Mermen to Merfolk (issue #2940)
     and replaced a few instances in core unit or terrain descriptions accordingly.
 ### User interface
   * Removed individual Join/Observe buttons for each game in the MP Lobby.
   * Highlight the titles of MP games with vacant slots.
   * Improved MP Lobby layout on low resolutions.
   * Improved reporting of network errors in the MP lobby (issue #3005).
   * Ensure the chat widget remains the correct size even after a window resize.
   * Custom MP game names are now capped at 50 characters.
   * Restored Era info to main MP game display.
   * Improved the resolution selection criteria for the MP Lobby.
   * Fixed inactive weapon specials being displayed in the Unit Attack dialog
     unlike in 1.12 (issue #3033).
 ### Miscellaneous and bug fixes
   * Removed misleading tooltip text stating registered nicknames are optional for
     the official MP server.
   * Attempting to save a screenshot with an unsupported format now shows an error
     message, instead of saving the screenshot as BMP with the requested extension.
   * It is now possible to disable logdomains in the Logging Options dialog.
   * Fixed the wesnoth(6) manpage claiming the default log level is 'error' when it
     has been 'warning' since version 1.9.0.
   * Document --log-none in the wesnoth(6) manpage.
   * Avoid trying to load invalid base64-encoded data URIs.
   * wesnoth_addon_manager and the addons.wesnoth.org web index can now use data URIs.
   * Fixed a crash when using certain invalid color= values.
   * Implemented a workaround for an unhandled std::bad_cast from string comparison
     functions that caused a crash-to-desktop when opening Preferences among others
     (issue #3050).
   * Fixed many crashes and out-of-sync errors when using the planning mode.

## Version 1.14.0
 ### Campaigns
   * Under the Burning Suns
     * New set of Quenoth faction and character portraits by LordBob.
     * Updated sprites for several Quenoth units.
     * Fixed "Invalid WML found" error that can be caused by the Quenoth Youth
       support ability.
 ### Help browser
   * Temporarily hidden Editor section as it is mostly incomplete and of little
     use right now (issue #2963).
 ### Language and i18n
   * Fixed Version label on the title screen not being translatable (issue #2914).
   * Updated translations: Czech, French, Galician, German, Japanese, Polish,
     Scottish Gaelic, Slovak, Spanish
 ### Multiplayer
   * Fixed regression causing a crash-to-desktop when trying to log into the
     server using a registered and active account without specifying a
     password.
   * Fixed an infinite loading screen if the server shut down or restarted
     mid-login.
   * Fixed an infinite loading screen when attempting to login with an
     unregistered nickname followed by a registered one.
   * Dark Forecast: Fixed possible Lua error when the weather changes.
 ### User Interface
   * Implemented MP chat message history saving (issue #1194, issue #2802).
   * Fixed context menus not dismissing on right click.
 ### Miscellaneous and bug fixes
   * Fixed an AI assertion when a unit with one disabled attack attacked a unit
     with no attacks or a single disabled attack.

## Version 1.13.14
 ### Security fixes
   * Fixed an issue allowing MP lobby and whisper message origins to be spoofed
     by clients.
 ### Campaigns
   * The Hammer of Thursagan
     * S12 Fixed enemies from ai6 (south-east lich) going to the book (spider)
       room
     * S12 Fixed north treasure chest disappearing
   * The Rise of Wesnoth
     * New set of story art.
 ### Help browser
   * Unit descriptions use the new multiplication sign format for attack lists
     now (issue #2873).
 ### Language and i18n
   * Updated translations: Chinese (Simplified), Czech, French, Galician,
     Italian, Polish
 ### Lua API
   * Fix wesnoth.show_popup_dialog and wesnoth.show_message_box not accepting
     translatable strings
 ### Multiplayer
   * Added team color to a few background units missing in Aethermaw.
 ### User Interface
   * Swapped the position and formatting of game names and titles in the MP
     lobby.
   * Made Faction Select button's purpose more clear in MP Staging.
   * Added a convenient button in Load Game to open your saves folder.
 ### Miscellaneous and bug fixes
   * Removed the Font Scaling preference. It was too buggy (issues #2792 and
     #1624).
   * Fixed some hotkeys triggering multiple commands on GNU/Linux (bug #1736).
   * Fixed [modify_side] share_vision=yes not doing anything (bug #2850).
   * Fixed regression where unit filters in [disable] weapon specials would not
     match the attacking unit.
   * Fixed images with no alpha channel rendering incorrectly.
   * Fixed unit selection not persisting between uses of Create Unit.
   * Fixed assertion when undoing actions in a synced context.
   * [filter_wml] no longer accepts [and] and [or] in addition to [not] since
     the implementation was non-functional.
   * Fixed some MP passwords being saved incorrectly (issue #2745)
   * Fixed AI not recruiting in some circumstances when there are only cheap
     units on the recall list.
   * Fixed sometimes being unable to join MP games with non-required eras.
   * Fixed locations not being added to the palette when loading a map (#1023)

## Version 1.13.13
 ### Campaigns
   * Eastern Invasion
     * Fixed some Lua errors in S05.
   * The Hammer of Thursagan
     * Fixed a misplaced door image in S12.
   * Under the Burning Suns
     * Changed Sun Singer movetype from float to foot.
     * Added some Quenoth elf unit animations.
     * Hide technical terrains in the Help browser (Human Ship, Lava overlay).
 ### Language and i18n
   * Fixed many cases of interpolated strings in the engine possibly having
     their translations retrieved from the wrong textdomain and falling back
     to the English original if that failed (PR #2711, bug #2709, bug #2732).
   * Fixed parts of the MP game setup UI having their translations ignored by
     the game (bug #2709).
   * Fixed certain parts of the UI displaying unit stats and trait effects
     having incomplete translations (bug #2732).
   * Fixed "Level" label in unit descriptions in the help being untranslatable
     (bug #2732)
   * Fixed "Search" placeholder text in dialog item filters not being
     translatable (bug #2709, bug #2732).
   * Fixed "Time of Day Schedule" heading for the index for the ToD Schedule
     top-level help section, "Lawful Bonus", "Schedule" (back link to index)
     and an error message not being translatable.
   * Updated translations: British English, Chinese (Simplified), Czech, French,
     Scottish Gaelic, Spanish, Ukrainian
 ### Multiplayer
   * A New Land: Fixed the scenario being broken.
   * Dark Forecast: Fixed possible Lua error when attempting to spawn units.
 ### Miscellaneous and bug fixes
   * Fixed minimap buttons not doing anything (bug #2681)
   * Fixed events with an id but no name being rejected
   * Fixed assertion when using [inspect]
   * Fixed inability to deselect modifications in single-player
   * Fixed infinite loading screen when logging in with an invalid name
   * Improved UX in multiplayer when synced debug commands are used during a
     game.
   * Updated bundled Oldania font to version 1.007 (from 1.006).
   * Added bold and italic Oldania font variants.
   * Updated bundled DejaVu Sans fonts to version 2.37 (from 2.35).
   * Fixed [chat] not working during [delay] or animations.
   * Removed the Password Reminder option from the Login screen.
   * Removed (optional) requirement of libpng from SCons and CMake and the
     associated options as Wesnoth now uses SDL_image to write PNG files.
   * Fixed assertion when saving game events mid-event.

## Version 1.13.12
 ### Security fixes
   * Disallowed access to blacklisted file paths such as hidden files and
     directories.
 ### Add-ons client
   * Addon upload progress bar now also works on Windows (bug #1439).
   * Fixed inability to cancel addon upload (bug #2591).
 ### Add-ons server
   * Added support for adding free-form comma-separated tags to add-ons in
     their publishing info (bug #2565).
   * Added support for overriding existing add-on attributes using the control
     FIFO.
 ### Campaigns
   * Northern Rebirth:
     * Fixed S06a Rakshas displaying incorrect portrait (issue #2569)
     * Fixed S12a invalid side error (issue #2569)
     * Fixed S13a incorrect leader when Eryssa is alive (issue #2569)
     * Fix Krash being brought back to life if he is dead
     * Appearance improvements for S04a 06a, 07a, 09a, 10a, 11a, 12a, 13a
     * Fix gold storing in S07a for S13a if retrieved
     * S04a, 05a, & S13a Make enemy units loyal so gold & events work properly
 ### Graphics
   * Updated default Time of Day schedule images.
   * New Heavy Infantryman attack animation by Zoomo.
   * New Elvish Sylph baseframe by Jetrel.
 ### Language and i18n
   * Updated translations: British English, Czech, Spanish
 ### Lua API
   * Add side_name to side proxy
   * Added wesnoth.custom_synced_commands table where you can register
     custom synced commands.
   * Add wesnoth.invoke_synced_command
   * Removed ai.synced_command() - replaced with the above two
   * wesnoth.end_turn() now allows to specify the next side
   * The new wml module is now considered mostly final.
     It has new remove_child and remove_children functions.
   * New wesnoth.persistent_tags table for more convenient custom
     save data (intended to replace game_events.on_load/save)
 ### User Interface
   * Improved outro screen.
   * Fixed a few cases of data not displaying in the MP Join Game screen.
   * Fixed required addon names not displaying properly when joining an
     MP game.
   * The Toggle Fullscreen hotkey now works everywhere.
 ### WML engine
   * Fixed units shown with [move_units_fake] disappearing between steps
     (bug #1516).
   * [modify_side] now supports side_name
   * [set_menu_item] no longer fires repeatedly if the player holds the
     hotkey (bug #1711). If you were relying on repeated firing, add
     repeat_on_hold=yes to [default_hotkey].
   * [set_variable] now supports prefix and suffix operations for
     string concatenation.
   * [effect] apply_to=attack now supports set_range
   * [cancel_action] now works in attack-related events.
   * [unit_type] upkeep= now works again. It was inadvertently broken during
     the 1.13 development cycle.
   * [filter_wml] now accepts [and] and [or] in addition to [not].
   * Added bar_offset_x/y in [unit_type].
   * The MP setup code no longer generates [side]s if the scenario has at
     least one side defined.
 ### Unit changes and balancing
   * Decreased cold resistance of Dune Burner line from 0% to -20%
   * Decreased cold resistance of Dune Soldier line from 0% to -10%
   * Add possibility for 'fearless' trait in Dune Soldier line
   * Increased pierce resistance of Dune Rover line from 0% to 10%
   * Increased impact resistance of Dune Piercer from 0% to 10%
   * Decreased Dune Piercer's XP to next level from 48 to 42
   * Increased Dune Riders's XP to next level from 39 to 42
   * Change mountain stats of Dune Marauder from impassable to 4 MP, 60% def
   * Increased cold resistance of Dune Marauder from -20% to 0%
   * Increased blade resistance of Dune Raider from 10% to 20%
   * Decreased hills defense of Dune Raider from 60% to 50%
 ### Miscellaneous and bug fixes
   * Fixed standing animation toggle not taking immediate effect (bug
     #1653).
   * Fixed error when de-assigning village with [capture_village].
   * Enabled the use of Open Type Font (.otf) fonts.
   * Fresh installs of the game will now open in fullscreen by default.
   * New game theme music by Mattias Westlund.
   * The unit advancement prompt is no longer shown for droided sides.
   * Fixed custom game titles being lost when reloading MP games.
   * The copy-to-clipboard function on the Lua console now produces plain text
     without Pango markup or entities for special characters (<, > and &)
     (bug #2434).
   * 'Turn Changed' desktop notifications in MP will now only display when
     human-controlled sides take control.
   * Fixed regression in 1.13.11 where completed events could fire again when
     reloading a save.
   * Fixed PNG images with an embedded palette displaying incorrectly.
   * It's now possible to save screenshots as JPEG (by changing the file
     extension before saving).
   * Fixed no_leader having no effect
   * Fixed units randomly being unable to move over merged terrains.
 ### Music and sound effects
   * Updated a few UI sounds.

## Version 1.13.11
 ### Add-ons client
   * Added an order dropdown that allows you to sort add-ons by the time of
     latest update or original upload (issue #1747)
   * Players will now be prompted to update outdated dependencies alongside
     downloading missing ones when installing an add-on.
 ### Add-ons server
   * Empty passphrases from malfunctioning clients that do not provide or
     generate a passphrase otherwise are now rejected instead of treated as
     valid.
 ### Campaigns
   * The Cutscene_Minimal theme is now used in all dialog-only scenarios that
     have `linger=no` in [end_level].
   * An Orcish Incursion:
     * New story art.
   * A Tale of Two Brothers:
     * Added a full set of hints on easy difficulty.
   * Delfador's Memoirs:
     * Iliah-Malal can now be killed by either undead or Delfador in S19
     * S19 has been rebalanced to increase difficulty and allow player & enemy
       to field more units.
   * Descent into Darkness:
     * 'Endless Night' now has accurate & improved scenario objectives. It also
       has received improvements in dialog, map appearance, and difficulty.
     * S08/S09 A Small Favor 2/3 - Fixed bug preventing door operation/usage.
     * Fixed and improved appearance and handling of the wose.
     * Other minor fixes and improvements.
 ### Packaging
   * Renamed the target and binary for the Boost unit tests from `test` to
     `boost_unit_tests`. This quells warnings from CMake about reserved target
     names, and reduces confusion about which `test` is intended: the wesnoth
     executable or the standard shell command.
   * The CMake build system now uses standard Kitware-supported variables
     from the `GNUInstallDirs` module. Distributions might have to change
     the defaults to suit their FHS needs.
   * The previous addition of `GNUInstallDirs` necessitates a bumping of the
     CMake minimum requirements, which is now at 2.8.5. Most distributions
     already use a CMake version above 3.0, so this would only affect users
     of vintage CMake versions.
   * higher resolutions of the application icon have been added and are now
     stored in the hicolor icon theme's directory
 ### Language and i18n
   * Updated translations: British English, Chinese (Simplified), Czech,
     Italian, Spanish
 ### Lua API
   * New wesnoth.unit_types[] getters:
     * `advances_to`
     * `advances_from`
     * `profile`
     * `small_profile`
   * The wml module now offers another new way of accessing WML variables:
     wml.variables is a simple wrapper around wml.get|set_variable.
   * unit.id is now a modifiable field for off-map (Lua-only) units.
   * Allow moving on-map units by setting `unit.x` and `unit.y` fields, or with
     `unit.loc = {x, y}` or `unit.loc = {x = x, y = y}`
   * Added `side.chose_random` getter.
   * Lua GUI2 API: added support for slider callbacks via
     `wesnoth.set_dialog_callback`.
 ### Multiplayer
   * Dark Forecast: Fixed broken faction and leader selection.
   * Rename the Khalifate to Dunefolk. This includes renaming all the faction's
     units.
 ### Performance
   * GUI2 windows no longer redraw everything 50 times per second. This reduces
     CPU usage in fullscreen windows such as MP lobby by about 85 %.
   * Miscellaneous low-level optimizations in game rendering code, improving
     performance ingame by up to 50 %.
 ### Units
   * Changed resistances and hitpoints for Tentacle of the Deep.
 ### User Interface
   * Removed broken Unit Box and Widescreen themes.
   * Fixed a bug that partially prevented movement feedback announce messages
     to be displayed (UI regression bug #2130, affecting 1.13.8 and 1.13.10).
   * New, greatly simplified display of games in the MP Lobby.
   * Fixed individual Join/Observe buttons acting on the wrong game in the MP
     Lobby.
   * Greatly improved behavior of sliders.
   * Fixed crash when modifying an existing friend entry in Preferences.
   * Fixed players being unable to start campaigns in MP mode.
   * Added confirmation prompt when clearing map labels.
   * Added show_border= key to the [main_map_border] to control whether map
     borders draw. Right now this is utilized in the cutscene themes.
   * If [main_map_border] background_image= is empty, the game map background
     will be plain black.
   * New really nice HD main menu/storyscreen background.
 ### WFL Engine
   * A new string `insert()` function has been added, similar to `replace()`.
 ### WML Engine
   * Added a `major_amla` option in AMLAs to keep the XP bar teal-white rather
     than purple for AMLAs that behave similar to regular advancements.
     (for example, Quenoth Youth in UtBS or similar unit lines)
   * File paths with backslashes are no longer allowed. This ensures that a UMC
     author can't accidentally use them and make an add-on that breaks on
     GNU/Linux and macOS.
   * File paths are now case sensitive even on Windows.
   * [show_if] is now usable in the [objectives] subtag [gold_carryover].
   * Macro SCEPTRE_OF_FIRE_EFFECT damage increased to 15x4 so Sceptre is an
     improvement over the uncut ruby of fire (14x4) in TRoW.
   * Added [lua] as a conditional tag, with identical syntax.
     The code in such a tag must return a true or false value.
   * Lua errors now cause as a condition to fail instead of pass.
   * New `formula=` key in [set_variable] allows evaluating a WFL formula with
     the variable, which may be either a scalar or a container.
   * A missing [event] `name=` key will now raise a WML error instead of being
     silently ignored.
   * Units hidden with [hide_unit] will remain hidden after reloading saves.
   * Fixed regression where most SUF keys would match all units if given "" as
     a value.
 ### Miscellaneous and bug fixes
   * Suggested save file names now use spaces instead of underscores.
   * Fixed crash after canceling add-on download (bug #2203)
   * Fixed ingame help showing units you haven't encountered (bug #2135)
   * Fixed the opacity IPF resetting to 0 if the value given was 100% or
     greater (bug #2185).
   * Fix recalls updating shroud immediately when "Delay Shroud Updates" is set
     (bug #2196)
   * Fixed not being able to undo previous moves after entering planning mode
     (bug #2303)
   * Fixed image cache being shared between campaigns.

## Version 1.13.10
 ### Add-ons client
   * A list of illegal filenames is displayed if any are found by the server
     when uploading.
 ### Add-ons server
   * A list of illegal filenames is sent to the client if any are found during
     the upload validation process. Only clients supporting this new
     functionality can display the list.
   * Fix an issue where the implementation of the hashing function used for
     add-on passphrases was changed since 1.13.8, breaking existing hashes
     (bug #2068).
 ### Campaigns
   * Delfador's Memoirs:
     * Clarified and fixed objectives in many places.
     * Improved appearance and flow of dialog in several places.
     * Some minor gameplay changes to improve the story.
     * Delfador now progresses properly from Mage Journeyman to Elder Mage.
     * Fixed some other bugs.
     * S07, A Night in the Swamp has been rebalanced on easy & hard.
     * S09, Houses of the Dead, has been rebalanced.
     * S18, The Portal of Doom, has been rebalanced.
     * Iliah-Malal is now an Ancient Lich in S18 & 19.
 ### Language and i18n
   * Updated translations: British English
 ### Lua API
   * Upgrade to Lua 5.3.4.
 ### Multiplayer
   * Fixed a bug where the client would return to titlescreen on receiving a
     redirect message from the server, which made connecting to the official
     server via the "connect to official server" button impossible.
 ### Miscellaneous and Bug Fixes
   * Updated our included Spirit Po version from 1.0.1 to 1.1.2.
   * Fix idle AI being replaced by default AI under certain circumstances on
     loading of mid-scenario saves (bug #1955)
   * Fix rare AI crashes in the move-to-any-target candidate action and the
     Fast Micro AI
   * Fix poisoner FAI to prevent flood of error messages in Legend of Wesmere
     (bug #1999)

## Version 1.13.9
 ### Add-ons client
   * Fix HTML injection exploit in the wesnoth_addon_manager web index
     generation functionality.
 ### Campaigns
   * Reordered beginner campaigns to be friendlier to new players.
   * Delfador's Memoirs:
     * Replaced incorrect 'Defeat all enemies' objectives with more specific
     'Defeat all enemy leaders'.
     * Increased difficulty, clarified objectives, & improved feel of S21 & S22.
     * Riders are now guaranteed to go for a signpost in S21 on all levels.
   * Heir to the Throne:
     * Fixed thieves in 'Siege of Elensefar' getting duplicated.
     * Fixed Void Armor not doing anything.
     * Fixed translatibility of countdown in 'Test of the Clans'.
   * Northern Rebirth:
     * Fixed backdoor lever opening the wrong location in 'The Pursuit'.
   * Secrets of the Ancients:
     * Fix delay when additional enemies appear in S21 (issue #1706)
     * Fix bug when finding bird bones reported here:
       https://forums.wesnoth.org/viewtopic.php?f=4&t=46480#p614781
   * Under the Burning Suns:
     * Fixed crash in 'Out of the Frying Pan'.
     * New unit sprites: Quenoth Mystic line, Quenoth Flanker line, Flesh Golem.
     * Fixed Ethereal Shadow and Ethereal Nightgaunt having nightstalk even when
       they shouldn't.
     * Balance changes to the Quenoth elves:
       * Formation ability now has looser adjacency requirements.
       * Quenoth Scouts have a non-slowing sling at lvl 1 instead of bolas.
       * Adjusted stats of some units.
 ### Graphics
   * Added portrait for Giant Rat.
   * Fixed transparency artifacts in scaled portraits and other message images
     (issue #1570).
   * Fixed issue with jumpy animations on faster speeds (issue #1565).
   * Added some new item and scenery images
 ### Language and i18n
   * Updated translations: British English, Chinese (Simplified), Italian,
     Polish, Scottish Gaelic, Slovak, Spanish
 ### Lua API
   * Add wesnoth.format function to substitute variables into a string.
   * Add wesnoth.format_conjunct_list and wesnoth.format_disjunct_list.
   * New global "wml" table groups together all the functions for working
     with WML tables, and its subtable "wml.variable" groups functions
	 for working with WML variables. Most of these are functions previously
	 found only in helper.lua; they no longer need a require to use.
   * Warnings for using deprecated Lua functions now only appear in debug mode.
   * wesnoth.game_config is now accessible in application and mapgen kernels,
     though some of its contents are missing.
   * New extra argument to wesnoth.match_location and wesnoth.get_locations
     allows specifying the teleport_unit.
   * Support the extra wesnoth.match_unit arguments also in wesnoth.get_units
   * Fix wesnoth.put_unit not correctly deciding whether to fire the unit placed
     event in some situations
   * wesnoth.debug() can now be called from map generators or plugins. It is
     also deprecated, however; you should use wml.tostring() instead.
 ### Multiplayer
   * When set to remember your password, Wesnoth now encrypts it. It is still
     possible to obtain the password from preferences, but it's no longer as
     trivial as before.
   * Fixed crash when loading a replay stored on the server (Bug #1762)
   * Fixed delay or clients getting stuck when starting a mp game (Bug #1674)
   * Fix bug where leader can become unable to be killed (issue #1666)
 ### Performance
   * Rewrote the FPS cap implementation. This greatly improves
     smoothness ingame.
   * Each invalidated hex is now only redrawn once, not twice. This almost
     halves CPU usage ingame.
   * Implemented GUI canvas caching. It speeds up multiple areas, but especially
     the story screen.
 ### Terrains
     * Added ruined version of encampment terrain.
     * New stone floor terrain.
     * Gates now have new terrain codes to grant more control for map makers.
       The old codes are now deprecated (issue #1839)
 ### Units
   * Added new lvl 0 Giant Scorpling, leveling into the Giant Scorpion.
   * Tweak some costs for lvl 3 units.
 ### User Interface
   * Fixed loyal and aged traits missing from help (issue #1935).
   * Unit recall dialog now sorts the units by both level and required XP for
     their next level-up (issue #1738).
   * Enable the use of tab to switch between text fields in most dialogs.
   * Color-code modifiers in trait tooltips
   * Translation teams in credits are now listed in alphabetical order of the
     localized language name (rather than the English language name).
   * Allow deleting saves with the Delete key in the Load dialog.
   * Fixed team selection screen often ignoring attempts to scroll (bug #1632).
   * Input Method Editor support in most textboxes (except in-game chat)
   * Campaign dialog now uses different-coloured victory wreaths depending on
     the difficulty you beat the campaign on
   * Campaign dialog can now be sorted in alphabetical or chronological order
   * Fixed a number of issues with hotkeys
   * Fixed the viewport being moved when changing the zoom level
   * Fixed an occasional interface hang where only the menus work
   * Trait description tooltips now color-code modifiers
   * Fix about window not showing Mac notifications as available
   * Add descriptive tooltips for all text elements in the top status bar
   * The current screen position is now saved in savefiles
   * Removed emacs keybindings in text fields (Ctrl+A, Ctrl+U, Ctrl+E)
     Ctrl+A now selects all text in the field.
   * Fix addon manager closing when canceling an operation
   * Addons can now be filtered to show only publishable addons
 ### WFL Engine
   * Add owner key to terrain space callable, for villages
   * Location formulas in [tunnel] now have a teleport_unit variable
   * Fix a crash when attempting to call a non-existent function
   * The following previously FormulaAI-exclusive functions are now also
     available in filter formulas (SUF, SLF, SSF, SWF):
	 adjacent_locs, location_in_radius, get_unit_type, unit_at, defense_on,
	 chance_to_hit, movement_cost
   * New builtin functions for manipulating locations
     (available to all formulas):
     adjacent_locs, are_adjacent, relative_dir, direction_from,
     rotate_loc_around
   * New enemy_of function checks if its second argument is
     an enemy of the first
     Arguments can be side or unit objects, or integer side indices (1..n)
 ### WML Engine
   * If ai_algorithm is used in [modify_side][ai], it now replaces the whole AI
     with the contents of [modify_side][ai], instead of appending these
     parameters.
   * New [credits_group] tag can be used by non-campaign addons to group several
     [about] tags under a single header. This is a toplevel tag, not a subtag
	 of [era] or [modification].
   * An empty id key in SUF no longer matches all units;
     instead, it matches none.
   * Fix [primary_attack] and [secondary_attack] in [kill]
   * Fix [kill] not affecting recall list units
   * Fix [scroll] with omitted x or y
   * Fix [story] not showing if all parts are conditional
   * Fix some hotkeys not working (issues #1737 and #1769)
   * New vision_cost and jamming_cost keys in SUF
   * Integer SUF keys (eg level) now accept a list of ranges
   * Fix $other_unit SUF variable being unavailable in nested [and] [or] [not]
   * Unit ability values can now be specified with WFL
   * The {ABILITY_TELEPORT} macro no longer uses internal variable substitution,
     meaning that it can be used in an event without specifying
     delayed_variable_substitution=yes.
     (This also applies to {ABILITY_BACKSTAB} and {ABILITY_LEADERSHIP}.)
   * Add fire_event key to [unit] which determines whether to fire a unit
     placed event.
   * [resolution] window_width/height now specifies the minimum window size
     for that resolution to be chosen
   * formula code is now supported in abilities and weapon specials (fr #1436)
   * The allow_new_game= in [scenario] now defaults to false (it still
     defaults to true in [multiplayer])
   * Add [cancel_action] (bug #1427)
   * Allowed TerrainGraphicsWML random_start= to optionally take a positive
     integer to limit the range of the random shift of the animation start time
   * new attributes mp_village_gold, mp_village_support, mp_shroud, mp_fog in
     [multiplayer] that are used as default values for the corresponding
     attributes in [side]
 ### Miscellaneous and Bug Fixes
   * Add --report/-R command line switch for printing the same report from the
     Game Version dialog's clipboard function to stdout.
   * Fixed programmatically killed unit flashing after its death animation
   * On GNU/Linux the game no longer minimizes when it loses focus in
     fullscreen mode (bug #1606)
   * Fixed whiteboard crash on delete action with multiple moves (bug #1842)
   * Fix units being unable to step on hexes with too high movecosts
     (bug #1473)
   * Improved and more detailed FPS display
   * Fix minimap being sometimes black in mp create (bug #1484)
   * Optimized unit filters
   * Fixed bug #1837 empty tags missing in [store_unit]
   * Fixed preferences not being saved if they weren't in the currently
     selected tab when the settings dialog was closed.

## Version 1.13.8
  ### Campaigns
   * Son of the Black-Eye:
     * Balancing changes for 'Silent Forest'.
   * Under the Burning Suns:
     * New sprites for Quenoth Youth (Kaleh and Nym) and Human Commander.
     * Applied a color shift to the human units in S8 and S9 to give them a more
       unique look.
 ### Editor
   * Fixed a crash when placing units.
   * Restored the ability to preview different ToDs. Note this still does not
     work when invoked form the Custom ToD window.
   * Fixed editor sides not having the correct side number.
   * Redesigned Windows, Areas, and Sides menus.
   * The Windows menu will now display maps starting from 1 rather than 0.
     This is likewise reflected in the default map filename.
   * Scenario names are now displayed in more places if available.
 ### Formula Engine
   * Fixed garbage data showing up in stack traces in the event of an error.
   * Object types can now be used in contexts where previously only a list or
     map was accepted - for example in the filter() function. They will be
     treated as a map in such cases.
 ### Graphics
   * Fixed items not receiving ToD lighting.
   * Portraits for many Walking Corpse and Soulless race variants.
   * New animations: Dwarvish Fighter idle.
 ### Language and i18n
   * Updated translations: British English, Chinese (Simplified), Lithuanian,
     Slovak, Spanish
 ### Lua Engine
   * Add wesnoth.log_replay which exposes a little-used functionality of
     injecting arbitrary logging data into a saved game.
   * Small change to animator API - facing parameter replaced with target and
     required to be a space adjacent to the unit.
   * New modifiable theme attribute in wesnoth.game_config
   * New wesnoth.zoom() function allows changing the zoom level
   * The wesnoth.scroll function scrolls the screen by an offset, similar to
     [scroll]. For example, wesnoth.scroll(5, -2)
   * New is_local attribute in side proxy table allows you to detect whether
     or not the side is controlled by a network player. Note that use of this
     has the potential for OOS errors.
   * New wesnoth.music_list table which allows controlling the music playlist:
     * wesnoth.music_list[1] etc returns mutable information about a specific
       track on the playlist
     * #wesnoth.music_list counts the number of tracks on the playlist
     * wesnoth.music_list.current returns mutable information about the
       currently-playing track
     * wesnoth.music_list.current_i returns the index of the current track on
       the list. It is writable, allowing you to switch to any track on the
       list. This respects fade values.
     * wesnoth.music_list.all returns a copy of the playlist that can be
       stored in a variable.
     * wesnoth.music_list.play plays a specific track (as [music]play_once=yes)
     * wesnoth.music_list.add appends a track to the playlist (as
       [music]append=yes)
     * wesnoth.music_list.clear clears the current playlist
     * wesnoth.music_list.next fades out the current track and moves to a new
       track on the playlist
     * wesnoth.music_list.force_refresh forces any pending playlist changes
       to be immediately applied
     * wesnoth.music_list.volume attribute gets/sets the current music
       volume, as [volume]music=
     * Each track has modifiable shuffle, once, ms_before, ms_after
       attributes and read-only append, immediate, name, title attributes.
       They are also comparable with == or ~=
   * wesnoth.set_music is now deprecated, in favour of the above new API
   * New wesnoth.sound_volume function gets/sets the current sound volume, as
     [volume]sound=
   * New wesnoth.show_story function launches the storyscreen viewer
   * wesnoth.dofile now forwards any excess arguments to the file in the
     "..." argument
   * wesnoth.require can now load all modules in a directory and accepts some
     shortened paths, for example omitting the ".lua" file extension and
     searching in the current directory.
   * When you require a package that has no return value, you now get a table
     that errors on any access to it, rather than nil.
   * New wesnoth.create_weapon function to make a weapon that's not attached
     to a unit (which could be useful for animations)
   * New wesnoth.show_message function shows a simple alert dialog, possibly
     with a choice
   * New wesnoth.alert and wesnoth.confirm functions - simple shortcuts for
     the above.
   * Functions for working with hex locations have been added under wesnoth.map
     In particular, helper.distance_between is deprecated in favour of
     wesnoth.map.distance_between
   * The following existing functions now also work in plugins and map
     generators: wesnoth.log, wesnoth.get_time_stamp, wesnoth.get_image_size
   * Several error messages now point more accurately to the actual location
     of the error.
   * wesnoth.simulate_combat output has additional keys:
     * untouched (in first two return values) indicates whether the unit took
       any damage
     * number (in second two return values) contains the Lua index of the
       attack, as opposed to attack_num which contains the WML index.
     * weapon (in second two return values) contains the actual weapon userdata
   * New methods in location_set: of_triples, to_triples, random
   * New animations key in Lua unit proxy returns a list of defined flags for
     animation
   * New functional.lua file implements a number of higher-order functions
     from WFL
   * The length operator now works on translatable strings, returning the
     length of the translation.
   * wesnoth.set_side_variable and unit.variables can now clear variables
   * helper.get_variable_array and helper.set_variable_array now work on
     units and sides (pass a unit, side, or side number as an extra parameter)
     Note: helper.get_variable_proxy_array does *not* work on units and sides
   * helper.rand now has a second parameter that defaults to wesnoth.random
   * Add wesnoth.game_config.victory_music and wesnoth.game_config.defeat_music
     setters
   * Add wesnoth.game_config.scenario_id getter
   * wesnoth.set_next_scenario() was replaced with
     wesnoth.game_config.next_scenario setter/getter
   * wesnoth.set_dialog_value can now (un-) fold GUI2 tree view nodes
 ### Multiplayer
   * Fixed statistics being lost when reloading an MP game.
 ### Performance
   * Greatly speeded up switching between add-ons in the add-on manager
     (bug #25523)
 ### User Interface
   * Updated Attack Predictions dialog to GUI2.
   * Updated Story screen to GUI2.
   * Double-clicking an add-on now installs, updates, uninstalls or publishes
     it depending on the situation.
   * Fixed file path being truncated on the wrong side in the File Browser.
   * Improved Hotkey category sorting interface in Preferences.
   * Improved Addon Manager and MP Staging interfaces at low resolutions.
   * Fixed bug that allows you to use the minimap to bypass view locking
   * You can now change the theme (in preferences) while a game is in progress.
   * Fixed units moving after in game help exit. (#24644)
   * Fixed a bug that caused rapid announce messages to overlap each other.
     (bug #21634)
   * Fix buttons disappearing while menus are open.
   * Fix map labels disappearing when a dialog is open.
   * Improve tooltip placement.
   * Move recruit/recall to top of context menu
   * Add a hotkey to launch test scenarios from the titlescreen
   * Added a Cutscene and Minimal Cutscene theme for UMC authors to use.
   * Improve layout of MP Create at low resolutions.
   * Game Load screen now lists the gold and total number of units for each
     side.
   * Textboxes now have a blinking cursor when focused.
   * multi pages can now contain different types of pages
 ### WML Engine
   * Add base_income key to [store_side]
   * Fix issues with alpha animations on hidden units (#14503)
   * Extensions to GUI2 Canvas ([drawing] tag in dialogs):
     * New [round_rectangle] shape - a rectangle plus corner_radius
     * [circle] shape can now be drawn filled; color key is now border_color
     * [image] has new resize_mode=tile_center
     * Canvas colors can now be expressed as formulas. The formula must return
       the color as a list of its components, eg "([r, g, b, a])"
   * Empty tags are no longer written to the configs of [unit]s and [side]s.
   * New [change_theme] tag to change the theme mid-scenario
   * New [zoom] tag allows changing the zoom level from an event
   * [kill]animate=yes now plays victory animations if applicable
   * [kill] now supports [primary_attack] and [secondary_attack] for the
     animation
   * Fix [volume] not accepting 100% as the new volume.
   * New concat_to_* keys in unit_type inheritance allow amending keys
   * Accept [story] as ActionWML in events
   * Added a ~NO_TOD_SHIFT() ImagePathFunction which can be used on terrain and
     item images to prevent them from being affected by ToD lighting.
   * [unit]placement=name is now [unit]location_id=name and also honours the
     new overwrite and passable keys, deprecating placement=map_overwrite etc.
   * New zoom_levels key in [game_config] defines the allowed zoom levels.
   * The default color list is now defined via a default key in [color_range],
     rather than the default_color_list key in [game_config][colors].
   * Standard Unit Filter has new ability_type_active and trait keys
   * Standard Weapon Filter has new special_active key
   * [animate_unit] now raises an error if the flag key is missing
   * Fix [set_variable][join] not working with translatable strings
   * [sound_source] now starts playing the sound immediately
   * New voice key in [message] and [story][part]
   * [objectives] now supports delayed_variable_substitution=yes and also
     delayed substitution with $|variable syntax.
   * Items in [options] now support container variables again.
 ### Miscellaneous and bug fixes
   * Fixed base animation showing on walking corpse & soulless bats
     (bug #25673)

## Version 1.13.7
 ### AI
   * Fix a performance regression in complex combat situations such as the
     "Oath of Allegiance" UMC campaign.
   * Fix some Micro AIs and AI helper functions not working correctly for AI
     sides under shroud
   * New function ai_helper.find_path_with_shroud()
 ### Campaigns
   * Eastern Invasion:
     * In 'Weldyn Besieged', redesigned the way in which revealing the
       identities of the liches works.
  * Descent Into Darkness:
     * Fixed certain units not having the 'guardian' special in 'Descent into
       Darkness'.
   * Heir to the Throne:
     * Fixed bug allowing the wrong person to receive the Sceptre of Fire.
     * Fixed inconsistencies in Li'sar's sprites and animations when she has the
       scepter.
     * New death animation for Asheviere.
   * Secrets of the Ancients:
     * New campaign added to mainline (Intermediate level, 21 scenarios).
   * The Hammer of Thursagan:
     * New sprites for Dwarvish Witness line.
   * Under the Burning Suns:
     * Fixed difficulty levels with the original units not working.
     * New sprites and animations for Spider Lich.
     * Various balancing changes, bug fixes and improvements to the new units.
       * Major changes:
         * Kaleh and Nym movement points increased from 5 to 6.
         * Moon Singer branch removed.
         * Added winged lvl4 advancement for Quenoth Druid.
         * Support ability now only lowers adjacent upkeep costs by 1.
 ### Graphics
   * New or improved sprites: Royal Warrior, Walking Corpse line wolf variation.
   * Improved terrain graphics: lava, volcano, lightbeam.
   * Improved the fake map border terrain so that it now connects with the real
     map borders and can better be used to change the apparent shape of the map.
   * Smoother fade transitions between differently colored time areas.
   * Adjacent castles of different types now have fewer glitches between them.
   * Fixed glitches between various mountain tiles (introduced in 1.13.3).
   * Fixed units getting submerged on some bridge types when placed over water.
   * Updated Mermaid Initiate portrait.
 ### Language and i18n
   * Updated translations: British English, Czech, German, Portuguese (Brazil),
     Scottish Gaelic
 ### Lua API
   * New wesnoth.set_side_id function can change flag, color, or both; it automatically
     updates cached flag info for you (but you may still need to redraw to see it).
   * The wesnoth.place_shroud and wesnoth.clear_shroud functions can alter shroud data
     for a single side. They accept a list of locations, a shroud data string, or the
     special value "all".
   * New wesnoth.is_fogged and wesnoth.is_shrouded calls test the visibility level
     of a hex for a particular side.
   * New wesnoth.create_animator call produces an object that can be used to run
     animations for a group of units
   * New Lua API functions for altering AI:
     * wesnoth.switch_ai replaces the entire AI with a definition from a file
     * wesnoth.append_ai appends AI parameters to the configuration; supports goals,
       stages, and simple aspects.
       (Aspect tags are not fully parsed; only the id and facet subtags are used.)
     * wesnoth.add_ai_component, delete_ai_component, change_ai_component
       These do the work of the [modify_ai] tag for a single side.
   * Side proxy changes:
     * flag and flag_icon are never an empty string
     * New mutable keys: suppress_end_turn_confirmation, share_vision
     * New read-only keys: share_maps, share_view
       num_units, num_villages, total_upkeep, expenses, net_income
     * Existing keys made mutable: shroud, fog, flag, flag_icon
   * wesnoth.scroll_to_tile now accepts a third boolean argument - if true, the scroll
     is skipped when the tile is already visible onscreen.
   * The config accessors in the helper module now give a sensible error message if
     something other than a config is passed as the first argument.
   * wesnoth.deselect_hex no longer deselects units; it only unhighlights the hex.
   * wesnoth.select_unit with no argument (or nil argument) now deselects any unit.
   * Fix lua side:matches always iterating over all units on the map.
 ### Multiplayer
   * New maps: 2p Clearing Gushes, 2p Hellhole, 2p Ruined Passage,
     2p Ruphus Isle, 2p Swamp of Dread, 2p The Walls of Pyrennis,
     2p Tombs of Kesorak, 4p Bath of Glory, 4p Geothermal.
   * Updated maps: Fallenstar Lake.
 ### Units
   * Several changes to names of attacks:
     * Shock Trooper, Iron Mauler, Mage of Light: maces and morning stars
       changed to flails.
     * Arif, Ghazi, Khalid, Shuja: long sword changed to sword.
     * Mighwar, Monawish: long sword changed to longsword.
   * Updated unit descriptions: Orcish Grunt line, Merman Fighter line,
     Sergeant, Lieutenant, General, Peasant, Royal Warrior, Ancient Lich,
     Dwarvish Steelclad.
   * Changed (mainly reduced) recruit costs of many high-level units:
     * Dwarvish Lord, Direwolf Rider, Troll Warior, Banebow, Draug.
     * Elvish Avenger, Champion, Hero, High Lord, Marksman, Marshall,
       Ranger, Sharpshooter.
     * Mage of Light, Fugitive, Huntsman, Ranger.
     * Batal, Hadaf, Mighwar, Mufariq, Rasikh, Shuja, Tineen.
 ### User Interface
   * List boxes now keep the selected item visible when you change the sorting
     option.
   * The Addon Manager has a brand new interface.
   * Modification selection is now always available when selecting a SP campaign.
   * Scrolling the Editor palette no longer also scrolls the map.
   * Removed the old GUI1 MP screens.
   * Ensured dialogs with unit preview panes start with some extra space.
   * Fixed an issue where certain long labels would cause scrollbars to appear.
   * Greatly reduced the number of zoom levels available.
   * Experimental change: removed scrollbar up/down buttons.
   * Converted Hotkey Bind popup to GUI2.
   * Fixed TC color in Faction Select sometimes not matching the side's selection.
   * Ensured sides are always ordered by index in MP Staging.
 ### Wesnoth Formula Engine
   * New str_upper and str_lower functions for case transformations
 ### WML Engine
   * Update [store_side] to store everything in corresponding wesnoth.sides[*].__cfg
     and additional keys: num_units, num_villages, total_upkeep, expenses, net_income
   * Removed LOW_MEM option when building.
   * Add color= attribute to [floating_text]
   * Add ~CHAN() IPF that allows altering images according to formulas
     It takes up to 4 WFL formulas, one for each channel (red, green,
     blue, alpha); each works the same as the ~ADJUST_ALPHA() formula.
   * New ability_type key in standard unit filters matches if the unit has any
     ability of the specified type (tag name).
   * Terrain flags "_border" and "_board" are now automatically set for every
     tile, indicating whether it is on the map border or not.
   * SUF type_tree renamed to type_adv_tree for consistency with [hide_help]
   * [message]s no longer trigger a scroll if the unit is onscreen.
   * Removed the ~DARKEN() and ~BRIGHTEN() IPFs.
   * {SOUND_POISON} and {SOUND_SLOW} have been deprecated and replaced with empty
     macros.
   * [tunnel] now accepts delayed_variable_substitution=yes/no.
   * [on_redo] is now deprecated and does nothing if used
 ### Miscellaneous and bug fixes
   * Fixed severe lag/freeze on slow PCs (bug #25356)
   * Updated wmlscope to handle the square braces syntax in file paths
   * Fixed an issue preventing quick replay when joining multiplayer matches
   * Resolved crash on Help when font scaling set to 115% or greater (bug #25292)
   * Resolved crash when viewing Help in CJK languages (bug #253334)
   * Fixed [campaign][option] tags not being properly considered.
   * Fixed an issue where team names could get corrupted in SP.
   * Zoom levels no longer get reset between loading games.
   * Do not load the Markov name generator if a CFG generator could be loaded.
   * Exit "quick replay" mode in MP games once the game is caught up.

## Version 1.13.6
 ### AI
   * Added new high_xp_attack candidate action to default AI. This CA performs
     attacks on enemy units so close to leveling that the default AI's combat CA
     would not attack them.
   * New Micro AI: Assassin Squad AI
   * Fix bug #23720, AI units with max_moves=0 do not attack.
   * Fix bug #22179: [disable] weapon special is ignored by AI. A second
     instance of the AI also ignoring this special under different circumstances
     has also been fixed.
   * Fix bug of Experimental AI recruiting sometimes failing under shroud
   * Fix some mainline campaigns custom AIs not working due to syntax changes
     after the AI refactoring for Wesnoth 1.13.5 (e.g. bug #25123)
   * Significantly improve move times for AI sides with many guardians
   * Micro AIs, other Lua AIs and ai_helper.lua utility functions:
     * Correctly and consistently deal with invisible units
     * New ai_helper functions get_attackable_enemies, get_visible_units,
       is_attackable_enemy, is_incomplete_move, is_incomplete_or_empty_move,
       is_visible_unit and robust_move_and_attack
     * Renamed function ai_helper.to_triple to LS_to_triples for consistency
       with other functions
     * Some internal changes to fix rarely occurring bugs and to improve
       robustness and speed
 ### Campaigns
   * Eastern Invasion:
     * Fixed broken village encounters.
     * Tweaked the balance of Scenario 2.
   * Heir to the Throne:
     * Reworked the map and added new AI behavior in 'The Princess of Wesnoth'.
   * Delfador's Memoirs:
     * S9: Resolved inability to end level even when Delfador has the Staff
       (bug #24951)
     * S17: Resolved Wesnoth units returning to recall list not being healed
       properly (bug #24952)
     * S19: Resolved undead veterans victory condition not working properly.
   * Under the Burning Suns:
     * Redesign of all desert elf units (currently optional, selectable through
       the difficulty menu).
 ### Graphics
   * Improved or new terrain graphics:
     * New wooden floor variation and transitions.
     * New aquatic encampment.
     * New aquatic castles.
     * Reworked stone walls so they take up less space and improved transitions.
     * Added Wooden and Rusty Gates.
   * Improved connections between castle and wall terrains, allowing castles and
     keeps to be placed adjacent to walls without glitches in most cases.
   * New sprite for Tentacle of the Deep.
   * Tweaked colors for all time schedules.
 ### Language and i18n
   * Updated translations: Finnish, Polish, Russian
 ### Lua API
   * Upgrade to Lua 5.3.3+
     Consult http://www.lua.org/ for a full change list and updated documenation.
     * bit32 functions removed
     * utf8 support added
   * Added new function wesnoth.fire_event_by_id to fire an event with a given ID.
   * Added new function wesnoth.remove_modifications, which removes applied
     modifications of the chosen type from a unit. The most efficient use is to
     remove all modifications with a specific duration value.
     Also callable as u:remove_modifications.
   * The built-in conditionals have_unit, have_location, and variable are now
     present in the wesnoth.wml_conditionals table. This means they can be
     directly called, extended with new features, or even overridden with custom
     implementations.
   * New recall_filter field in unit proxy returns the [filter_recall] config
   * New variations field in unit_type proxy returns a list of unit variations
     Each member is a full unit_type describing that variation.
     The table is iterable with pairs().
   * Lua side, unit_type, and unit attack proxies can now be compared with ==
     with identity semantics. (Previously, each time such a proxy was obtained,
     it would produce a new object that did not compare equal to any others.)
   * Lua unit_type lists (wesnoth.unit_types and unit_type.variations) are now
     countable with the Lua length operator.
   * Lua dialog functions now support the stacked widget and the unit preview pane
   * New wesnoth.show_menu function shows a dropdown menu at the mouse location
   * Lua attack proxy has new read_only field which is true for unit_type attacks
     If true, attempts to change the attack will result in an error.
   * The name field in Lua attack proxy is now writable
   * The attacks field in the Lua unit proxy is now writable
     Specifically, attacks may be replaced, appended (by assigning a new ID or
     the next valid index), or removed (by setting a field to nil).
   * wesnoth.show_message_dialog supports second_portrait and second_mirror keys
     in its first argument, which produces a dialog with two portraits.
   * The callable userdata returned by wesnoth.textdomain can now be called with
     an additional two parameters (a plural string and a count) in order to support
     gettext plurals.
   * New matches function in team and unit attack metatables, which test if the
     side or weapon matches a filter.
   * helper.lua metatables are now protected from external access; getmetatable()
     will return a descriptive string instead of the metatable.
   * ai.aspects.attacks no longer returns a full attacks analysis. Instead it
     returns a table with "own" and "enemy" keys containing the valid units
     for attackers and targets. The function ai.get_attacks() still returns the
     full attack analysis.
   * Aspect fetcher functions (eg ai.get_aggression()) are now deprecated in favour
     of the ai.aspects table.
   * The location_set iter and stable_iter functions can now be called with no argument.
     In this case, they return an iterator suitable for use in a for loop.
   * New wesnoth.wml_matches_filter function takes a WML table and a WML filter and tests
     if the table matches the filter. The filter syntax is as with [filter_wml].
 ### Music and sound effects
   * Added a preference to pause the music when the game loses focus.
   * Now the music fades out between scenarios.
 ### Networking
   * Ported campaignd to use boost.asio instead of SDL_net.
   * Removed unit tests for old networking stack. This was the last part that
     depended on SDL_net
 ### Performance
   * When a heuristic determines that it's probably faster, the game predicts
     battle
     outcome by simulating a few thousand fights instead of calculating exact
     probabilities. This method is inexact, but in very complex battles
     (extremely high HP, drain, slow, berserk, etc.) it's significantly faster than the
     default damage calculation method.
 ### Units
   * Changed the sound for the melee attack of the
     Loyalist Bowman, Orcish Crossbowman and Orcish Slurbow.
 ### User Interface
   * Trait descriptions in the help are now generated. (This makes user-defined
     traits show up in the help as well.)
   * Fix game map sometimes showing and buttons sometimes not rendered properly
     in story screen (bug #24553)
   * Improved font rendering on Windows.
   * Redesigned gamestate inspector window.
   * Recall dialog no longer shows units that no leader on the map can recall
     (due to the [filter_recall] not matching)
   * Weapon specials only gained through AMLAs now get a help topic
   * The "Cores" button on the title screen is now hidden if no cores other
     than the default are installed
   * Redesigned game dropdown/context menu appearance
   * New categories bar in hotkey preferences allows you to filter hotkeys
   * Fix issue with the title screen not redrawing when the window size or
     fullscreen setting changes with a dialog open over it.
   * Restored descriptions for choices in combobox-based Advanced Preferences
     entries (lost in 1.13.3).
   * When using the --wconsole option, the game now prints a prompt in the event
     of a fatal error to avoid closing the console before the error can be seen.
   * Restored GUI2 textbox selection highlight lost in version 1.13.3.
   * Added a "Draw Number of Bitmaps" option to the map editor, for terrain
     graphics diagnostics.
   * Ported file chooser dialog to GUI2, and redesigned it to include a
     bookmarks bar with predefined and user-defined shortcuts.
   * Tweaked the border/groove color scheme on textboxes and sliders to better
     reflect their state and mimic window and button borders.
   * The following dialogs have been converted to GUI2: Unit Recall, Unit List,
     Game Stats, MP Create, MP Game Lobby, Faction Select, Unit Advance.
   * Improved UI responsiveness.
   * Tab completion in the Lua console is improved. It can now handle paths
     containing square brackets (though it won't complete them) and will not
     offer keys that are not valid identifiers.
 ### WML Engine
   * Added ignore_special_locations=yes|no, default no, to [terrain_mask] to
     ignore the special locations given in the mask, leaving all those on the
     underlying map unchanged.
   * [terrain_mask] starting locations and special locations are relative to
     the mask. Existing names replace any in the underlying map regardless of
     their location. Each name as a unique location; but a location may have
     any number of names. While a map/mask can give only one name per location;
     stacking masks allows multiple names. Names cannot be removed.
   * Added {HAS_NO_TURN_LIMIT} macro for objectives
   * New attributes for [message] with [option]
     * Added variable= to [message]: if specified, gives variable name to
       receive the [option] index (1..n) selected
       only used if any [option] appear
     * Added value= to [option]: if specified, gives value to store in variable
       instead of index number, only used if variable= appears in [message]
     * New second_unit, second_image, second_mirror attributes for showing two
       portraits on a single message.
   * New attributes for [role]:
     * search_recall_list=yes|no|only(default yes) controls where to look
     * reassign=yes|no(default yes) if no, check for a unit and do not assign to
       another
     * [auto_recall] sub-tag, if assigned to recall list, gives [recall]
       attributes (no SUF)
     * [else] sub-tag, WML to execute if no unit found for the role
   * New help_text= key for [trait] to set the description displayed in the
     help.
   * Added tag id= [fire_event], which allows raising events by id
   * [modify_unit] now understands [effect] tags, which it applies directly.
     This replaces the use of [object] with no_write=yes (which will be removed
     in the next release).
   * Add [object]take_only_once=yes|no (default yes) - if set to no, automatic
     tracking is disabled for this object (allowing it to be taken multiple
     times even if it has an id).
   * New [remove_object] tag which removes applied [object]s from matched units;
     it can filter on the entire [object] WML. The most efficient use is to
     remove all objects with a specific duration value.
   * Renamed [foreach] variable= to array=
   * Renamed [foreach] item_var= to variable=
   * Fixed several bugs in the name generation of the map generator
   * Fixed issues with using [endlevel] in victory/defeat events
   * The {MAGENTA_IS_THE_TEAM_COLOR} macro is no longer needed in [unit_type]
     It is now the default behaviour unless overridden with the flag_rgb key.
   * New key type_tree in unit filters - similar to type, but also matches any
     possible advancements of the specified unit types.
   * [options][combo] in [scenario], [modifications], etc has been renamed
     to [choice] (which is more accurate). The old name still works for now.
   * ~ADJUST_ALPHA() image path function now takes a WFL formula. It can access
     the following variables: x, y, width, height, red, green, blue, alpha.
     It no longer accepts a percentage (use ~O() for that).
   * Fixed [rule] in [terrain_mask] ignoring use_old=yes
   * Fixed filter returning invalid locations if invalid locations were given
     in a variable that was used with find_in=
   * Moves cave map generator to lua. scenario_generation=cave is now deprecated and
     will be removed soon. The Lua version has feature parity with the old one, but
     the syntax is a little different. It supports both map_generation and
     scenario_generation.
   * Tunnel functionality was expanded and the default behavior was altered in
     order to make moves through tunnels consistent with all other moves:
     * Vison through tunnels is now possible and enabled by default. It can be
       turned off by setting allow_vision=no in the [tunnel] tag.
     * Own and allied units on tunnel exits do not block the tunnel any more.
       The blocking behavior can be reenabled by setting pass_allied_units=no
       in the [tunnel] tag.
   * Added text_alignment= key to [story][part] to specify horizonal alignment of
     text.
   * [set_variable] now supports abs= (absolute value), power= (raise to
     power) and root= (extracts root, 'square' is the only value currently
     supported) keys
   * Individual [terrain_graphics] rotations can now be skipped entirely by
     using "skip" in the rotations= list.
   * WML macros can now include optional arguments with default values that the
     caller can override.
 ### Miscellaneous and bug fixes
   * A new way to make units invulnerable for debugging: select the unit and type
     ";unit invulnerable=yes". This method operates by decreasing the opponent's hit
     chance to zero: as a result, it doesn't slow down damage prediction unlike the
     "increase HP to ridiculous levels" method.
   * The ;choose_level command now works in the tutorial and in [test] scenarios
   * Fixed a stray ; character appearing pre-entered in the command console.
   * Fixed bug in wesnothd that was causing server crashes if game with
     multiple network and local players was ran.
   * Added a tab to run the wmlxgettext tool to GUI.pyw
   * Fixed problem with Spectre's hitpoint bar positioning.
   * Fixed crash when unit with planned actions is killed before those actions are
     completed (bug #20071)
   * Show correct number of attacks in case of swarm weapon special (bug #24978)
   * Fixed bug that icons of buttons under the minimap disappeared when the
     player opened and closed a menu.
   * Correct unit recall count in statistics when undoing a unit recall (bug #25060)
   * Add tip to recall units instead of recruiting them if costs exceed 20 gold (recruitment costs)
   * Resolve sides in map editor not having a proper side number and subsequently
     causing a crash upon editing (bug #25093)
   * Avoid rare cases of mini-map producing a divide-by-zero error (bug #25155)
   * [filter_vision]: fix bug of filter not matching own/allied hidden units
   * Prevent crash on quitting scenario with planned recruits present (bugs #24022/25193)

## Version 1.13.5
 ### Campaigns
   * An Orcish Incursion:
     * Linaera can recruit Mage, and cannot recruit Elves
   * Heir to the Throne:
     * Add journey tracks for 19c/20b path.
     * New sprites for Li'sar.
     * S10: Clarify objectives and change egg image on capture.
     * S19c: Removed the undead and the swamps.
   * Tutorial
     * Improve translatability for languages with gender-dependent pronouns
     * S1: Fix unit being deselected after the select message
     * S2: Highlight (outline) talked-about locations
   * Legend of Wesmere
     * S3: fix bug which silently disabled Urudin retreat AI
 ### Graphics
   * Updated generic portrait of Mermaid Initiate.
   * Added generic portrait for Giant Spider.
 ### Language and i18n
   * Updated translations: British English, Galician, German, Italian, Japanese,
     Polish, Portuguese, RACV, Russian, Scottish Gaelic, Spanish
 ### Networking
   * Reworked the multiplayer server to use asio functions for networking
     operations instead of SDL_net, thus it no longer depends on SDL_net and SDL.
   * The client now uses boost::asio for communication with wesnothd too.
 ### Removed support for SDL 1.2. SDL 2 is now the only supported version.
 ### Terrains
   * Changed terrain code of Desert Mountains from Mdy to Mdd.
 ### Editor
   * Allow to set special locations in the editor which can then be read by wml.
 ### User Interface
   * Fix flickering caused by tooltips, closing windows and tabbing into the game (bug #24532)
   * Various design improvements to GUI2 widgets
   * New simpler GUI2 loading screen
   * New colored cursor graphics
   * Fixed Mage of Light halo appearing in the top-left corner of the screen
     while the mage is moving (bug #23712).
   * Fixed Observers icon appearing behind other top bar items in MP games on
     horizontal UI resolutions < 1024 (bug #24455).
   * Fixed ToD schedule progress indicator appearing behind other top bar items
     on vertical UI resolutions < 600.
   * Improved the dialog for choosing what to do when a player leaves in
     multiplayer.
   * The side overview now also shows allied human sides in sp even if
     they aren't discovered yet
   * Added an option to disable the loadingscreen animation since it caused
     bugs in some configurations.
   * Added a gui method to activate loggers (Preferences -> Advanced -> Logging)
     loggers activated in the gui print just like loggers activated in the
     command line (i.e. messages appear in the console)
   * The Lua console screen now has a clear button
   * Fix bug #24762: Editor actions are out of sync after resizing.
   * Increased the font size for text in buttons.
   * Changed unit help topics to use smaller images on smaller monitors.
 ### WML engine
   * Add color= attribute to [message].
   * Add [else], search_recall_list=, auto_recall= to [role]
   * Fix some issues with [foreach]
   * Fix some issues with backstab-like weapon specials
   * Support [effect]times=<integer>
   * Add highlight=yes|no to [scroll_to], [scroll_to_unit], [message]
     Defaults to no in the first two cases, yes in the third
     If yes, the target hex is outlined.
   * New ~SCALE_INTO(w,h) IPF which preserves aspect ratio, using bilinear
     interpolation scaling.
   * New ~SCALE_INTO_SHARP(w,h) IPF which preserves aspect ratio, using
     nearest neighbor scaling.
   * Support delayed_variable_substitution= in [on_undo], [on_redo]
     Note that this means $unit.x and $unit.y may not reflect the unit's
     true location, so using [unstore_unit] on $unit may have unexpected effects.
     This applies to $second_unit too. The $x1, $y1, $x2, $y2 variables work fine
     though, so in most cases they can be used instead. Anything else in $unit
     or $second_unit is also fine.
   * formula= in SUF can now reference $other_unit via the formula variable "other"
   * formula= now supported in location, side, and weapon filters
   * Weapon filters now support number, parry, accuracy, and movement_used
   * New [has_attack] in standard unit filters; supercedes has_weapon= and
     uses full weapon filter.
   * lua_function=var.member now works in SUF; however, 'var' still needs to
     be a global variable.
   * Added new keys name_generator, male_name_generator and female_name_generator
     for the [race] tag to declare a context-free grammar to describe how names
     are derived
   * Modification tags in [modify_unit] now support delayed_variable_substitution
     (This means [advancement], [object], and [trait] tags.)
   * All looping tags now give an error if they contain no [do] tag.
     (They may contain multiple [do] tags, however.)
   * Add [message]image_pos=left|right, which mostly supercedes ~RIGHT()
   * For [core] authors: New keys for game logo (game_logo, game_logo_background)
   * AiWML:
     * Filters within [micro_ai] can now use $this_unit, which was previously
       impossible due to the config being prematurely parsed.
     * Simplified aspect syntax which works for all aspects, present and future:
       * All aspects with simple values can be specified as key=value
       * Except attacks, all aspects with complex values have a simple tag form
         containing only the aspect value (e.g. [avoid])
       * All aspects, simple and complex, can be specified using a tag named by
         the aspect, whose contents is the same as a corresponding [facet]
       * The full [aspect] and [facet] syntax also still works
     * [ai] configs no longer recognize the version= key
     * ai_algorithm key now selects a preset AI; possible values include
       "ai_default_rca", "experimental_ai", and "idle_ai", but custom AIs
       defined by eras or modifications with an [ai] tag can also be used
     * [leader_goal] now automatically sets facet ID for auto_remove
       (Only if using simplified syntax; in full syntax, the ID must still be
       specified in two places.)
     * The AI config in the gamestate inspector is now split into multiple
       sections according to the type of component.
     * The following deprecated components have been removed:
       * recruitment stage (name=ai_default::recruitment)
       * old recruitment candidate action
         (name=ai_default_rca::aspect_recruitment_phase)
       * old simple move-to-targets candidate action
         (name=ai_default_rca::simple_move_and_targeting_phase)
       * number_of_possible_recruits_to_force_recruit aspect
       * recruitment_ignore_bad_combat aspect
       * recruitment_ignore_bad_movement aspect
       * The recruitment aspect has been removed from the engine; however,
         "recruitment" is now accepted as a synonym for
         "recruitment_instructions". Old code should work without changes.
       * [goal]name=protect_my_unit
     * The following experimental components have been removed:
       (Most of these were broken or non-functional)
       * Experimental recruitment candidate action
         This worked, but was worse than the default recruitment.
       * Global fallback candidate action
       * Akihara recruitment candidate action
       * Fallback stage (functional but useless)
       * Strategy Formulation with RCA stage
         This worked, but was slow and unmaintained. If a maintainer
         steps up, it may be restored later.
       * Several of the development AIs available in debug mode in the
         multiplayer setup menu
     * The "Strong AI" has been renamed to "Default AI (RCA) with Alternate
       Recruiting" and is now only available in debug mode.
     * Lua components (goals, aspects, stages, and candidate actions)
       now support an [args] subtag, which passes any sort of WML data
       to the Lua code - this works similarly to [args] in a [lua] tag,
       though not quite identically.
     * Aspect syntax with [aspect] tag has been fully generalized so that
       [aspect], [facet], and [default] are all exactly the same in terms of
       what contents they expect. (Except that [facet] additionally takes
       turns= and time_of_day=.) The most useful consequences of this:
       * You can nest a [facet] inside another [facet] if the outer one has
         name=composite_aspect
       * You can define a [facet] with the Lua engine
     * invalidate_on_tod_change= has been implemented for aspects. It applies
       when the bonus resulting from time of day modifiers (excluding any
       illumination abilities) changes at any location on the map. Thus, it
       occurs on average less often than invalidate_on_turn_start and may
       be best combined with an explicit invalidate_on_turn_start=no.
     * Minor shorthands have been introduced in the recruitment_instructions
       aspect:
       * [pattern] tag is like [recruit] with pattern=yes
       * [total] tag is like [recruit] with total=yes
       * If no [recruit] tag is present, a default is added with importance=0.
         This means recruitment will happen even with only [limit] tags.
     * Extensions to [modify_ai]:
       * [modify_ai]action=change works on aspects, using path=aspect[id].
         Useful if you need to change all the facets at once, but note that
         it also wipes the [default] facet.
       * The [default] facet can be referenced as facet[default_facet].
         This should rarely be needed, but is there in case it is.
         It also makes them easily identifiable in the inspector.
       * Automatically copy over the id= with action=change, if the
         new component did not specify one.
       * It can add, remove, and change jobs and limits in the
         recruitment_instructions aspect. The path to use for this is
         aspect[recruitment_instructions].facet[facet_id].recruit[recruit_id].
         (For a limit, replace "recruit" with "limit".)
         The [recruit] and [limit] tags now support id keys for this.
   * Added side_name= attribute in [side], it's now no longer possible to
     specify the current_player attribute in the [side] wml.
   * unit filters, specially in wesnoth.get_units now have a limit= attribute
     to limit the number or matched units.
   * Added new attribute "registered_users_only" to MultiplayerServerWML which indicates
     that only registered users should be allowed to join the game
   * wesnoth now does a stricter check for unit type ids.
   * In multiplayer,  victory events now fire at the end of the scenario if at least one
     human player has won, this fixes OOS in mp campaigns.
 ### Lua API
   * wesnoth.match_unit can now take a location (rather than a unit) as
     the optional third parameter. This will cause the filter to consider
     the unit to be on that location instead of its actual location.
     This even works for units on a recall list.
   * wesnoth.highlight_hex is no longer deprecated, but its effect is
     slightly different from the old one. It outlines a hex, nothing more.
   * wesnoth.select_hex is now deprecated in favour of the new
     wesnoth.select_unit (or u:select). The effect is almost the same
     (with the exception that it does not outline the hex if true is
     passed as the second argument), but this name change was done to
     emphasize that it acts on a unit, more than a location.
   * New wesnoth.set_time_of_day function which sets the current time
     of day, taken either as the time ID (eg "second_watch") or the index
     of the time in the overall schedule (eg, 1 would be dawn in the default
     schedule). Optional second argument takes a time area ID, to set
     local instead of global time.
   * New wesnoth.add_fog and wesnoth.remove_fog functions allow changing fog
     on the map. The [lift_fog] and [clear_fog] tags now use this.
   * New wesnoth.add_sound_source, wesnoth.remove_sound_source, and
     wesnoth.get_sound_source functions to allow manipulation of sound
     sources. The [sound_source] and [remove_sound_source] now use these.
   * New wesnoth.log function for printing log messages. The [wml_message]
     and [deprecated_message] tags now use this.
   * New wesnoth.name_generator function builds a name generator and returns
     it as a callable userdata. Both the original Markov chain generator
     and the new context free grammar generator are supported
   * New wesnoth.get_end_level_data() function which can be called in a
     victory, defeat, or scenario_end event to access (or change) how
     the scenario ends.
   * Fix wesnoth.erase_unit failing if the unit was on a recall list.
   * WML tables defined in Lua now accept string keys with array values
     (where "array" is a table whose keys are all integers). This joins
     the elements of the array with commas and produces a single string
     value. eg {x = {1,2,3}} is equivalent to {x = "1,2,3"}.
   * wesnoth.effects table can now be used to alter the behaviour of
     built-in effects - for example, to add a new feature to
     [effect]apply_to=attack. It also now supports effect descriptions,
     for use by the [trait] tag.
   * Additional fields in unit proxy:
     * usage, cost - self-explanatory
     * traits - list of the IDs of all traits
     * abilities - list of the IDs of all abilities
   * Additional fields in table returned by wesnoth.get_terrain_info:
     * icon, editor_image, light
   * Additional fields in unit type proxy:
     * race, id, alignment
     * attacks, which returns the same thing as unit.attacks
     * abilities, same as unit.abilities
   * Additional field in side proxy:
     * faction (read-only), faction_name (read-only)
   * LuaAI:
     * The table returned by check_*() now has a "result" field which
       gives a description of the action's result; similar to "status"
       but more descriptive.
     * Target tables now use a descriptive name for "type", instead of
       an integer. This applies to the elements of the table returned by
       ai.get_targets() and also to the elements of tables returned by
       Lua goals. (However, Lua goals that used integers will continue
       to work for now.)
     * There are some compatibility-breaking changes to Lua candidate
       actions - see the release notes or the wiki for full details.
     * Lua AI code can now access the AI routines through the global ai
       object. This object is only accessible to AI code; it does not exist
       in the general Lua global scope.
     * When executing Lua goals, aspects, or candidate action evaluations,
       the ai is in "read-only" mode - the read_only key is set to true,
       and functions that change the gamestate, such as ai.attack(), are
       missing from the table. (ai.check_*() functions are still present.)
     * The ai.aspects table provides access to every aspect known by the
       engine, including several that previously did not have corresponding
       ai.get_*() functions. You access them as ai.aspects.avoid, for example.
       The table is read-only and raises an error if you attempt to write to it.
     * The way to create Lua candidate actions has changed a little. Old code
       will require minor changes.
     * New wesnoth.micro_ais table contains the loaders for all Micro AIs.
       New loaders can easily be installed by add-ons. See any built-in
       micro AI (in ai/micro_ais/mai-defs/) for an example of how to do this.
     * The attacks aspect can now be defined as a Lua aspect. The code
       should return a table with keys "own" and "enemy", each of which may
       be either a unit filter table or a function which takes a unit as a
       parameter and returns true or false.
   * Added wesnoth.game_events.on_mouse_move/on_mouse_actions callbacks
     (fr #22635)
   * Added wesnoth.special_locations
   * [lua] tags now also support the [args] subtag outside events.
   * added lua_function= attribute in location filters
   * Added on_event.lua which is an eaiser to use wrapper for
     wesnoth.game_events.on_event
 ### Multiplayer
   * Hornshark Island: simplified multiplayer faction determination
   * Added "Registered users only" checkbox to multiplayer configuration dialog which
     when checked, only allows registered users to join the game
 ### Wesnoth formula engine
   * Formulas in unit filters can now access nearly all unit attributes
     The following attributes were renamed (old names still work, for now):
     leader -> canrecruit
     total_movement -> max_moves
     movement_left -> moves
     states -> status
   * Nearly all unit type, side, weapon, and terrain attributes available
     to Lua code are now also exposed to WFL. The exceptions are mainly
     translatable strings.
   * Unit and side WML variables are now accessible under "wml_vars".
     Since WML variables don't easily translate to formula variables, the
     special attributes __all_children, __children, and __attributes provide
     specialized views of the variables config as a list, string-list map,
     and string-value map, respectively.
   * The 'special' attribute of weapons was renamed to 'specials', and it now
     contains the special IDs rather than their translateable names.
   * New syntax features:
     * String interpolation syntax. Within a formula string (enclosed in
       'single quotes'), the syntax [some_formula] interpolates the result
       of the inner formula into the string. (The simplest use case is
       interpolating the values of variables.)
     * String can now escape special characters:
       ['] single quote, [(] open square bracket, [)] close square bracket
     * New 'in' operator which tests if a list contains an item or if a map
       contains a key.
     * New concatenation operator a..b which works on strings and lists
     * New range operator a~b which produces a list of consecutive integers
       This can also be used for "list slicing" - eg my_list[3~5] returns
       a new list containing elements 3 through 5 of my_list.
     * Lists can be used as an index for a list. This is "selection indexing"
       and returns a new list with only the elements specified by the indexing
       list.
     * Function definitions (using the def keyword) are now supported in all
       formula contexts, which means that they can be used outside FormulaAI.
       However, non-FormulaAI functions are currently local to the formula
       that declares them.
     * Maps containing string keys that are valid identifiers can now be
       indexed with the dot operator instead of the indexing operator.
     * Strings can now be indexed via 'string'.char[n]. Also supported are
       'string'.word[n] and 'string'.item[n] (the latter splits on commas)
   * Changes to core functions:
     * head() takes an optional argument - if present, a sublist is returned.
     * abs(), max(), min() now work on decimal numbers
     * reduce() function can specify an optional initial accumulator
       This will be returned for an empty list instead of null.
     * substring() function can now accept a negative size parameter
       This counts backwards from the specified offset
       A size of -1 is the same as 1.
     * if() can take two arguments; returns null if the condition is false
     * tomap() will now invert the effect of tolist()
     * debug_print() now shows in chat area if debug mode is on
   * New core functions:
     * Trig functions tan, acos, asin, atan have been added. (Sin and cos
       existed since at least 1.9 but were undocumented until very recently.)
     * Other common math functions - root(), sqrt(), cbrt(), log(), exp()
     * hypot(x,y) function calculates sqrt(x*x+y*y) with minimal error
     * pi() returning the circle ratio
     * tail() - opposite of head()
     * reverse() function for strings and lists
     * zip() function - converts [[1,2,3],[4,5,6]] to [[1,4],[2,5],[3,6]]
     * take_while() function returns items from a list until the first one
       that fails a condition
     * find_string() locates a substring within a string
     * replace() replaces a sequence within a string
     * type() function checks the type of a formula result
     * distance_between() - this was promoted from FormulaAI to core
     * pair() function that produces a key-value pair suitable for
       passing to tomap() - this also means key-value pairs are now
       serializable (relevant in FormulaAI)
     * sgn(), trunc() and frac() functions for decimal numbers
   * Map generator engine:
     * makes now use of the new context free grammar name generator
     * ported name generation from english.cfg to [naming]
   * Bugfixes:
     * Dice operator is now synced (where possible)
     * Modulus (%) operator now works on decimal numbers
     * Exponentiation (^) operator is now right-associative
     * Fix several math operations returning a very large negative number when
       the operation was invalid (for example, (-2) ^ 0.5).
       Now they return null instead.
   * Formula debugger (accessed with the debug() function):
     * Now works again, but is skipped unless in debug mode
     * No longer explodes on formulas using less-than
     * Some better information and formatting in the execution trace
     * You can now step through "where" variable assignments
   * FormulaAI no longer starts recruiting if a formula evaluates to the
     string 'recruit'.
   * Deprecated:
     * The fai/faiend keywords are deprecated as part of a move to clearly
       define the difference between "Wesnoth Formula Language", the language,
       and "FormulaAI", the AI engine that uses the language. For now they
       still work, but any future code should use wfl/wflend instead.
       Use of the .fai file extension is still fine for FormulaAI code;
       for other formula code in a separate file, .wfl is recommended instead.
 ### Miscellaneous and bug fixes
   * Resolve translated logo images not being used (bug #24357)
   * Ported the "hexometer" tool from Bash to Python 3
   * Recognize hotkey release events
   * Allow changing keybindings for scrolling the map.
   * Fix the move-to-targets candidate action of the default AI ignoring tunnels
   * Fix two rare bugs in the goto candidate action that resulted in goto moves
     by other units being skipped after a unit could not get to its goal.
   * Replace wmlxgettext tool with new python3 implementation by Nobun:
     https://github.com/AncientLich/wmlxgettext-unoff/
   * Debug commands that create units now do so for the currently controlled
     side instead of always side 1.
   * Fixed bug #24696 (Nightstalk ability not working)
   * Ported the following scripts to Python 3: TeamColorizer, about_cfg_to_wiki,
     campaign2wiki, terrain2wiki
   * Wesnoth now ignores unknown arguments that XCode may pass when testing.

## Version 1.13.4
 ### Language and i18n
   * Updated translations: British English, Russian
 ### Graphics
   * Improved or new terrain graphics: Mine Walls (replaces Hewn Cave Walls).
 ### User interface
   * Fix vertical alignment of individual attacks listed in the Attack Unit
     dialog.
   * Removed extra padding below unit stats in the Attack Unit dialog.
 ### Miscellaneous and bug fixes
   * Fix non-deterministic crashes in the Attack Unit dialog resulting from
     invalid memory references (regression introduced in 1.13.3).
   * Fix uninitialized variable in event handling code (bug #24498).

## Version 1.13.3
 ### Greatly improved SDL 2 support. SDL 2 is now used by default build when building. This fixes the following bugs, among others:
   * Bug #18112: Color cursors cause slow mouse movement at menus
   * Bug #19666: When I resize windows during dialog I lose the menu buttons
   * Bug #20332: Cursor not mapping correctly on Retina display when in Windowed mode
   * Bug #23820: SDL 2 alpha blending issues
   * Bug #23821: Text input is broken in GUI1 under SDL 2
   * Bug #23908: SDL 2 SDL_BlitSurface causes crashes
   * Bug #23918: UI graphics garbled on OS X 10.11 El Capitan
   * Bug #23934: Resize actions are laggy
   * Bug #24138: SDL 2 crash on resize after starting a game and returning to the title screen
   * Bug #24209: Screen becomes black upon minimise and restore
   * bug #24212: SDL 2 build handled input incorrectly once window focus is lost when menus are open
   * Bug #24213: SDL 2 build leaves menu items stuck if window dimensions change while open
   * Bug #24214: SDL 2 build handles fullscreen toggle badly
   * Bug #24260: Menu and Action buttons disappear on resize
   * Bug #24261: Area under Objectives not redrawn on resize
   * Bug #24294: Doubled-up GUI1 dialogs don't redraw the secondary in SDL2
   * Bug #24477: Segfault when launching Credits
 ### Campaigns
   * Liberty:
     * Added some animations for the Rogue Mage line.
     * Fixed Relana appearing as male.
   * Sceptre of Fire:
     * Added some animations for Dwarvish Miner.
     * Improved sprite for Dwarvish Caravan.
   * Son of the Black-Eye:
     * Added some animations for the Orcish Shamans.
   * Under the Burning Suns:
     * Updated sprites for Naga Hunter, Naga Guardian line, Crab Man.
     * Crab Man changed to Monster Crab.
 ### Graphics
   * Improved or new terrain graphics: Stones with Sand Drifts, Igloo Village,
     Adobe Village.
   * Added option for toggling off water animations to Preferences -> Display.
 ### Language and i18n
   * New translation: Asturian
   * Updated translations: British English, Galician, Russian, Swedish
 ### Sound effects
   * Fixed various subtle timing problems with attack sounds.
 ### Terrains
   * New terrain: Desert Mountains (Mdy), Impassable Desert Mountains (Mdy^Xm),
     Ruined Adobe Village (^Vdr).
 ### User Interface
   * GUI1 comboboxes now use the thinner menu frame style.
   * Implemented a new GUI2 Attack dialog
   * Added gui2 comboboxes.
   * Removed prompt when purging the WML cache from Preferences.
   * Implemented a new GUI2 Preferences dialog
   * Implemented a new font scaling option on the Display panel.
   * Selecting an entry in the friend/ignore list panel now copies it to the
     input field; this makes it easier to edit friend/ignore notes.
 ### WML engine
   * Added new event "unit placed", which triggers when (and regardless of how)
     a unit appears on the map.
   * Added support for color= in [unstore_unit] and [print]
   * removed network and network_ai controlles, whether a side is networked is
     now stored in the is_local attribute.
   * Eventnames (received in wesnoth.game_events.on_event) now have all their
     spaces replaced with underscores.
   * lua can now read/write the 'persistent' attribute of sides.
   * lua can now read/write the 'alignment' attribute of units.
   * Added {CURRENT_FILE} and {CURRENT_DIRECTORY} macros.
   * add support for relative dirs in wesnoth.dofile/require
   * Added name= and write_name= attributes in [item]
   * Added description_alignment= key to [campaign]
   * Fixed [put_to_recall_list] not working correctly (bug #24390)
   * Wesnoth now does a stricter check in variablenames and doesn't allow
     empty indexes like "a[].b" anymore.
 ### Lua API
   * Added wesnoth.read_file
   * wesnoth.dofile/require now allow .. in relative paths.
   * unit.level and unit.upkeep can now be changed via lua unit proxies.
   * wesnoth.find_path is now available in lua map generators and allows border=yes/no parameter.
 ### Miscellaneous and bug fixes
   * Fix the new log code on Windows to actually use Unicode-aware functions
     in a couple of places so Wesnoth does not quit on startup when trying to
     relocate the log file to a path with Unicode characters (bug #22897,
     definitely fixed this time).
   * Decreased high memory consumption caused by the animated water.
   * Fix bug #23108: exclude aborted attacks from statistics
   * imgcheck now runs on Python 3
   * Fix bug #15259: Secondary click uses control-click instead of command-click in OS X
   * New hi-res icon using new logo for OS X
   * Removed the "wmlmove" Python tool
   * Fixed [event] in [unit_type].
   * Fixed oos caused by mp replay turn feature.
   * Fixed oos bugs caused by plattform dependent rounding from double to int in lua.
   * Fixed custom (lua-defined) scenario tags beeing removed from [replay_start]
   * Fixed savefile bloat caused by unit variations (walking corpses)
   * Fixed preferences file bloat caused by null-command hot-keys (bug #21969)
   * Fixed clearing of default hot-keys not working (bugs #21983/#22218/#23981)
   * Fix [for] not correctly handling the case when the array length changes
     during iteration
   * Fix variables in [message][command] (bug #24288)

## Version 1.13.2
 ### Add-ons client
   * Warn user if attempting to upload an addon with a version lesser than or
     equal to a published version.
 ### Campaigns
   * Delfador's Memoirs:
     * Added defeat condition for death of the last undead veteran in
       'Showdown in the Northern Swamp' (bug #23668)
   * Eastern Invasion:
     * Fixed scenario events not working right on easy difficulty in 'Captured'.
   * The South Guard:
     * Deoran is now the grandson of the Haldiel in HttT instead.
   * Tutorial:
     * Fixed transition to second scenario.
   * Under the Burning Suns:
     * Gave Garak a new ability called Teaching (at the start of every turn,
       his experience points are transferred to adjacent units on the same side)
 ### Editor
   * Add Recent Files menu with a list of recently saved or loaded maps or
     scenarios, up to a custom limit (by default 10) that can be set in
     Advanced Preferences.
   * Fixed Player Start labels not being updated with repeat map generations
     (bug #20036)
 ### Graphics
   * New animated water.
   * New standing animation for the Dwarvish Runesmith
   * New generic portraits for the Troll and Troll Whelp
   * New game logo
 ### Language and i18n
   * Updated translations: British English, French, Galician, Hungarian,
     Italian, Japanese, Latvian, Polish, RACV, Scottish Gaelic, Slovak, Spanish
   * Fixed crashes during start-up on Windows resulting from add-ons containing
     erroneous textdomain declarations (bug #23839).
   * Spelling and grammar review across all official campaigns.
 ### Lua API
   * It is now possible to implement custom [effect]s with wesnoth.effects
   * Changed interface for the wesnoth.synchronize_choice function
   * Added support for unit.level field (read only)
   * Added support for unit.advancements field (bug #23677)
   * Added support for unit.parry and unit.accuracy fields
   * Added support for current.event_context.unit_x/y fields (bug #23507)
   * Added wesnoth.set_dialog_focus function
   * Added wesnoth.set_dialog_visible function
   * Added wesnoth.show_message_dialog function
   * Added wesnoth.show_popup_dialog function
   * Added wesnoth.deselect_hex function
   * Added wesnoth.is_skipping_messages and wesnoth.skip_messages functions
   * New parameter write_to_mods in wesnoth.add_modification
   * Added wesnoth.random function
   * helper.shuffle is now synced
   * Remove wesnoth.get_unit(underlying_id)
   * Add wesnoth.get_unit(string_id)
   * Change wesnoth.message so that it can display translatable strings
   * Change wesnoth.put_unit so that the unit is passed as the first parameter
   * Add wesnoth.erase_unit, which replaces wesnoth.put_unit when called without
     a unit
   * Add wesnoth.unit_vision_cost
   * Add wesnoth.unit_jamming_cost
   * Add methods to proxy unit metatable:
       matches, to_map, to_recall, clone, extract, advance, add_modification,
       resistance, defense, movement, vision, jamming, ability, transform
     All are equivalent to a similar wesnoth.* function, but are called as
     unit:fcn_name(arguments) instead of as wesnoth.fcn_name(unit, arguments)
   * Add wesnoth.races[race_id].traits
   * Add wesnoth.unit_types[unit_type_id].traits
   * Add optional third argument to wesnoth.match_unit, which is used for
     the $other_unit variable in the filter.
   * pairs() and ipairs() now work on vconfig userdata objects
   * Add helper.get_nth_child
   * Add helper.child_count
   * Add helper.child_array
   * wesnoth.remove_time_area no longer takes a comma-separated list of time
     area ids.
   * wesnoth.add_time_area no longer warns about commas in time area ids
   * unit.variables can now access sub variables.
   * Added wesnoth.get/set_side_variable to store variables in a [side]-
   * Added read/write fields carryover_bonus/carryover_add/carryover_percentage
     in lua sides.
 ### Music and sound effects
   * Updated music track "Frantic", new one by Stephen Rozanc.
   * New sounds: dwarf hit and die, ink, mud fist and glob.
 ### Terrains
   * Removed unit elevation from N-S hanging bridges
   * Added unit elevation to NW-SE and SW-NE stone chasm bridges
   * Added unit elevation to NW-SE and SW-NE plank bridges
   * Hanging, stone chasm, and plank bridges are now displayed in-game simply
     as "Bridge", retaining their descriptive names in the editor as per
     convention.
   * Fixed spurious "could not open image 'terrain/.png'" error messages caused
     by terrains without a minimap image (symbol_image) such as those from the
     Special category in the editor (Impassable Overlay, etc.).
 ### User interface
   * It is now possible to switch from replay directly into normal play
     (bug #23833).
   * The game now shows a notification on remote clients if a sides takes very
     long to do a local choice (bug #23297).
   * It is now possible to replay a turn in mp games by loading autosaves.
   * The game now asks for confirmation when attempting to quit during the game.
   * Add "unit status=..." command to debug console to add/remove unit statuses
   * GUI2:
     * stacked_widget can optionally display a single layer at a time. This
       may be used to implement dialogs with multiple pages or tabs.
     * Widgets which are children of invisible or hidden parents can no longer
       be interacted with even if the children themselves are still internally
       visible.
     * Added support for tristate buttons/toggle panels or more generally
       n-state buttons/toggle panels
   * Added a version dialog button to the title screen, replacing the Paths
     option previously found in Preferences -> General.
 ### WML engine
   * controller= in side filters can now be used in mp games for unsynced code
   * Added [effect] apply_to=max_expereince set=<value>
   * Added enable_if= to mod and era events
   * Added $varname?default_value| in variable substitution
   * Fixed side_for= parameter in [message]s with input
   * New actionwml tag [on_undo] contains actionswml that is executed when the
     current action is undone
   * New actionwml tag [unsynced] executes its contents in an unsynced context
     where for example
     [set_variable]rand= will return unsynced results
   * [campaign] now supports [event] subtags which are added to every scenario
     of the campaign similar to [modification] [event]s
   * Added support for mod_x,mod_y= in [terrain_graphics].
   * Added support for has_flag= in terrain graphics [variant].
   * Added category= to [label] - allows grouping labels so that players can
     show/hide them
   * Add female_text= to [animate_unit] and [unstore_unit] for easier
     translating
   * Add female_message= to [message] for easier translating of lines spoken by
     generic units
   * AMLAs in [modifications] now use [advancement] tags instead of [advance]
     tags.
     This means you can add an AMLA to a placed unit by simply using its
     definition macro, for example {AMLA_DEFAULT}.
   * [get/set_global_variable]'s side= attribute now defaults to "global"
     (bug #23686)
   * [team] share_view=yes/no, share_maps=yes/no was replaced with
     share_vision=all/shroud/none
   * Add exclude_amla= key in [advancement] which disables the advancment if the
     unit has already taken certain other advancements.
   * The WML preprocessor now writes warnings to stderr for macros redefined
     without #undef, to help detect unintentional name clashes.
   * Fix macro definition line numbers being offset by 1 in WML preprocessor
     messages involving macros.
   * Added WML menu item and event handler views to the Gamestate Inspector
     dialog.
   * The Gamestate Inspector now displays empty WML variables too (including
     containers/arrays).
   * Added new possibilities for [effect]:
     * apply_to=alignment - change a unit's alignment
     * apply_to=max_attacks - change a unit's attacks per turn
     * apply_to=recall_cost - change a unit's recall cost
     * apply_to=vision, apply_to=jamming - change a unit's vision/jamming points
     * apply_to=new_advancement - add new advancement possibilities (either
       units or AMLAs)
     * apply_to=remove_advancement - remove advancement possibilities (either
       units or AMLAs)
     * apply_to=attack - add set_ versions of all existing increase_ keys
     * apply_to=attack - add increase_movement_used and set_movement_used to
       change the number of movement points the attack consumes
   * Ability to patch movetypes to account for custom terrains or damage types
   * Removed y offset by -1 from [message]'s scroll-to-unit logic.
   * Add [found_item] ConditionalWML to check if an [object]id= ActionWML has
     been taken
   * New auto-stored WML variable $other_unit usable in the following contexts:
     * [filter_adjacent] - $other_unit refers to the $this_unit of the enclosing
       filter (In weapon specials and unit abilities, the unit owning the
       ability.)
     * [filter_self/opponent/attacker/defender] (weapon specials)
       $other_unit refers to the other unit in the attack
       (eg in [filter_self], it's the opponent)
     * [affect_adjacent][filter] (unit abilities)
       $other_unit refers to the unit owning the ability
   * New rotate operators for directions: dir:cw and dir:ccw
     (This is useful mostly in conjunction with variable substitution.)
     These operators are applied after the existing '-' operator that takes the
     opposite direction.
   * Adjacency filters in abilities and weapon specials now support count= and
     is_enemy=
   * New implementations of backstab and leadership specials using $other_unit.
     In particular, leadership is now a single macro and it is not supported to
     give a unit a leadership ability of a different level (eg level 4
     leadership on level 2 unit). The backstab= key is deprecated.
   * Add new looping tags: [for], [foreach], [repeat]
   * Add new flow control tags: [break], [continue], [return]
   * Added a new [difficulty] tag for defining a campaign's difficulty level
   * Add new syntax for [option], similar to the new difficulty syntax
   * Add [test_condition] ActionWML that tells why a conditional failed (for
     debugging)
   * Add [remove_time_area] WML tag which takes a comma-separated list of time
     area ids.
   * [time_area] no longer warns about commas in ids when not using remove=yes.
   * Add [random_placement] ActionWML
   * In [set_variables] you can now mix [value] and [literal], and even [split]
   * Add [resource] tag which contains [event] and [lua] tags smimilar to
     [modification] but hidden from the user.
   * Removed name= attribute in [side].
   * Add support for [endlevel] bonus=<number>
 ### Editor
   * Added Category field and color sliders to the Edit Label panel.
 ### Miscellaneous and bug fixes
   * Fix $this_unit auto-stored variable not working correctly in most SUFs
   * Made Documents\My Games\WesnothX.Y the default user config+data dir on
     Windows to replace the broken <current working dir>\userdata default that
     has never been acceptable practice. Portable installs are now required to
     explicitly use . or  .. (e.g. `--config-dir .\userdata`) to force the
     user config+data dir path to be relative to the current working dir (see
     also bug #23753).
   * Wesnoth now uses combined stdout+stderr log files on Windows, moved to
     <user data dir>\logs during initialization and first created in the user's
     temporary files directory defined by Windows. Log files are named like
     "wesnoth-<TIMESTAMP>-<PID>.log" and up to 8 log files are kept along with
     the latest session's log file. This avoids issues caused by SDL 1.2's
     built-in redirection code not being Unicode-aware (fixes bug #22897).
   * Removed legacy filesystem API implementation.
   * Fixed Generate Map dialog bug that caused last choice of map
     generator to not be actually selected (bug #23711).
   * Fixed unbound memory read in internal time formatting code with
     specially-crafted input.
   * Child wesnothd processes spawned by the Host Network Game option on
     Windows now display console output directly instead of using stdout.txt
     and stderr.txt.
   * Remember last selected modifications separately for single and multiplayer.
   * Fixed SDL2 compilation issues.
   * Fixed RECRUIT_UNIT_VARIATIONS core WML macro leaking an internal temporary
     variable ($recruited_unit_random_variation).
   * Fixed unit [resistance] and [jamming_costs] not being considered for sync
     check.
   * Fixed problems with slow/poison/petrify sounds (bug #23024) and made the
     sounds play automatically when the status is inflicted in combat, instead
     of being played by attack animations.
   * Fixed OOS on random maps, where clients placed sides in different castles.
   * Fixed some (not all) OOS caused by modifying a sides controller by wml.
   * Fixed wml menu items becoming unsynced in later scenarios.
   * The game now automatically detects whether to show the game connect screen
     between scenarios.
   * Fixed possibility of corrupting saved games in certain instances,
     eg if an add-on tries to set an invalid variable
   * Fixed bug 23060: unit stat tooltips do not show.
   * wmllint, wmlscope, wmlindent and wmllint-1.4 now run on Python 3
   * Text boxes tab completion now lists friends and whisperer nicks for easier
     answer (bug #9742)
   * Avoid crash when planning moves on planned recruits (bug #18637)
   * Fixed cases of wrong unit type used in planning moves (bug #20299)
   * Fixed hang when attempting to make a screenshot from a non-existent map via
     command-line (bug #20900)
   * Fixed Cancel Orders not working when loading MP game (bug #22133)
   * Fixed broken Oasis terrain help entry (bug #23023)
   * Fixed load game hot-key not working in the main menu (bug #23215)
   * Added user's leave notification for ingame players

## Version 1.13.1
 ### Security fixes
   * Disallowed inclusion of .pbl files from WML, independent of extension
     case (CVE-2015-5069, CVE-2015-5070, bug #23504).
 ### AI
   * Fast Micro AI: exclude hidden enemies from attack evaluation by default;
     add optional key attack_hidden_enemies to override this
   * Lua AI helper functions: fix bug crashing the AI when number= is not set in
     a unit attack definition
   * Micro AIs: add a warning and avoid crash when a non-existing side is used
     in the [micro_ai] tag
 ### Campaigns
   * Dead Water:
     * The Stun effect now expires at the stunned unit's side turn end
   * Under the Burning Suns:
     * The Stun effect now expires at the stunned unit's side turn end
 ### Editor
   * Redesigned Generate Map dialog to use a real listbox and remember the
     last choice during the same editor session.
   * Fixed stack corruption bug in the Side Setup dialog.
 ### Graphics
   * New generic portraits for the Walking Corpse and Soulless
 ### Language and i18n
   * Updated translations: Galician, Scottish Gaelic, Russian
 ### Lua API
   * New function wesnoth.get_viewing_side
   * New function wesnoth.remove_dialog_item
 ### Multiplayer
   * A New Land:
     * Removed the ability to select individual factions when using map
       settings as this breaks player recruit lists (bug #23593).
   * Era names no longer support formatting markup in the game setup screen.
   * Removed the Silver Mage from the available leaders for the Rebels faction
     in Age of Heroes.
   * Fixed "Accept whispers from friends only" not working with the default
     lobby UI, and added a warning every 5 minutes for individual rejected
     senders.
   * Allow custom colors in MP connect dialog (bug #23629).
   * It is now possible to change a side's controller by WML (for example with
     [modify_side]) in a MP game.
   * Changing teams by WML in a MP game causes less bugs (bug #23028).
   * MP campaigns now behave more like single-player campaigns.
   * Various fixes in the MP game setup screens (bugs #23641, #23509, #23496,
     #23462).
 ### Units
   * Fixed the Shuja not having the default AMLA.
 ### User interface
   * Fixed minimap buttons appearing without contents or in the wrong state
     during WML start events until they are interacted with or control is given
     to the player for the first time or some other unspecified thing happens.
   * Force uniform font rendering settings across X11 and Apple OS X, avoiding
     color glitches resulting from incorrect applications of subpixel hinting
     (bug #20337).
   * Hide mod options from the user command prompt dialog in the MP lobby when
     not authenticated as a mod.
   * Fixed unit bars, ellipses, and orbs disappearing for individual units in
     replay mode when using Skip Animations/Quick Replays if they moved without
     attacking or otherwise switching to a new animation.
   * Chat Log dialog now starts on the last log page when there are multiple
     pages.
   * Fixed Wesnoth's fonts remaining in use by Windows after abnormal exits, as
     well as being available to other applications.
   * Started showing on which difficulty levels a campaign was completed.
   * Modifications for single-player campaigns are now selectable directly in
     the single-player Campaigns menu.
   * The game now automatically detects whether to show the game setup screen
     in single-player mode.
 ### WML engine
   * Added support for [object] duration=turn end
   * New or updated image path functions:
      * ~BW(): converts an image to black and white
      * ~NEG() now supports 0,1 or 3 arguments, allowing solarization effects
      * ~SWAP(): swaps the RGBA channels of the image
      * ~CROP(): now supports negative x and y values.
   * Added support for SLF in [label] when used inside an event
   * Added synced=yes/no (default yes) to [set_menu_item]
   * [options] is now recognized inside [campaign]
   * type=sp/mp/hybrid now must be specified for [modification]s
 ### Miscellaneous and bug fixes
   * Removed abandoned libana network API implementation.
   * Fixed bug #23201, toggle icons display error in replay.
   * Added --wconsole switch to allocate a console on startup (Windows-only).
   * Added cwesnoth.cmd wrapper script to start Wesnoth with a console
     displaying stdout+stderr (Windows-only).
   * Updated mainline campaigns and multiplayer scenarios to use [filter] status=
     instead of [filter] [filter_wml] [status]
   * Fixed a segfault in [move_units_fake]
   * Fixed unbound memory read in the MP map selection screen that could lead
     to a segmentation fault or other abnormal behavior (bug #23606).
   * Made silence.ogg larger to work around a crash involving the multiplayer
     lobby with music and sound enabled (bug #23633, bug #23599) with libvorbis
     builds affected by Debian bug #782831.
   * Moved [role] tag to Lua (fix for bug #23630)
   * Removed broken Python port of wmlxgettext from data/tools/.
   * Default to non-strict compilation with CMake.
   * It is no longer possible to undo moves before the last (manual) shroud
     update.
   * Debug commands are now synced.
   * Fixed GUI2 toggle panels in some way that the author of this changelog
     entry chose not to specify.
   * Removed some config reloads, especially when leaving games or entering
     the single-player Campaigns menu.
   * It is now possible to use WML to disable/enable quick 4 MP leaders in the
     Default era and the time over advantage dialog by setting a WML variable.

## Version 1.13.0
 ### Security fixes
   * Fixed arbitrary file read from WML/Lua API (CVE-2015-0844, bug #23440).
 ### Add-ons client
   * The Update All button is now displayed on all Add-on Manager views instead
     of just the Upgradable filter view, and enabled only when there is at
     least one add-on that may be upgraded.
 ### Add-ons server
   * Add-on metadata pattern blacklisting implemented.
   * Major internal refactoring done.
 ### AI
   * New Micro AI: Fast AI
   * Big Animals Micro AI: bug fix for units not attacking when [avoid_unit] is
     not set
   * Bottleneck Micro AI: bug fix for case when allied units are present
   * Coward Micro AI: new optional key attack_if_trapped=
   * Forest Animals Micro AI: bug fix for wander terrain lying on border hexes
   * Herding Micro AI: bug fix for dogs sometimes having moves left at end
   * Lurkers Micro AI: fix bug in wander terrain selection
   * Lurkers Micro AI: bug fix for attack error when lurker runs into ambush
   * Lurkers MAI: bug fix for attempting to attack petrified units
   * Messenger Escort Micro AI: new optional parameters [filter],
     [filter_second] and invert_order=
   * Stationed Guardian Micro AI: make guard_x/y= optional keys
   * Stationed Guardian Micro AI: bug fix for unreachable stations
   * Messenger Escort Micro AI: bug fix for escort units blocking messenger's
     progress
   * Several Micro AIs: fix a variety of rarely occurring but serious bugs, such
     as invalid savegames or disabling the AI from working for the rest of the
     turn or after changing the Micro AI settings.
   * Bug fix for requiring unnecessary keys for removal of several Micro AIs
   * Fixed a bug that made it impossible to change or delete Micro AIs after a
     game had been reloaded (bug #21750). This was a general bug in the RCA AI
     mechanism and applied to other CAs as well, but it was most visible in the
     Micro AIs.
   * Some internal reorganization of Micro AI code to facilitate customizing
     them for UMC and adding functionality from other versions of Wesnoth. Some
     code cleanup for consistency, readability and speed, as well as fixing of
     some minor and subtle bugs.
   * Fix bug in Random Recruit Micro AI: the AI can now handle custom castle
     terrain independent of its terrain code
   * Fix bug in Return Guardian and Goto Micro AIs when there is no path to
     the goal hex
   * Several fixes of translatable strings in Micro AIs
   * Micro AIs: only display on-screen error messages when in debug mode
   * Add conditional inclusion of AI macros to AI cfgs in data/ai
 ### Campaigns
   * Changed all occurrences of {FLAG_VARIANT ragged} to {FLAG_VARIANT6 ragged}
   * Singleplayer campaign creation now utilizes the create, configure,
     and connect engines from the multiplayer codepath.
   * Eras, mods, and options are now available for sp campaigns.
   * Fix sighted events firing too early in several scenarios. Fixes bug #22466
   * Use the new bigmaps for A Tale of Two Brothers, Delfadors Memoirs, The
     Hammer of Thursagan, Northern Rebirth and the epilogue of Legend of
     Wesmere.
   * Remaining cases of old-style note and carryover information in [objectives]
     have been converted to a newer style.
   * A Tale of Two Brothers:
     * Updated music playlists.
   * Descent into Darkness:
     * Fixed various issues with player and enemy gold and income in
       'Descent into Darkness'.
     * Fixed Darken Volk's ellipse in scenarios 'A Small Favor part 3' and
       'Alone at Last'.
     * Fixed Darken Volk being completely passive in 'Alone at Last'.
     * 'Return to Parthyn' now has variable content depending on whether
        necromancy was used in 'Saving Parthyn'.
   * Eastern Invasion:
     * In 'Captured', stolen gold won't be mentioned if the player had no
       carryover gold.
     * Updated maps for scenario 14, 16 and 17b.
     * New animation for Ravanal's shadow wave.
     * Added the Skeleton Rider line.
     * New map for 'Mal-Ravanal's Capital', as well as updated
       maps for scenario 2 and 4b.
     * Tweaked and rebalanced all scenarios up to 'Two Paths'.
   * Heir to the Throne:
     * Tweaked and expanded music playlists for all scenarios.
     * Fixed missing message in 'The Siege of Elensefar'.
     * Hide unit variations that should not be listed in the help browser.
   * Liberty:
     * Fixed possibility of no viable routes around guards in 'Hide and Seek'.
     * Tweaked resistances for the Skeleton Rider line.
   * Northern Rebirth:
     * Fixed Sister Thera and Father Morvin respawning into the recall list if
       the other is on the north or west map edges.
     * Sister Thera and Father Morvin will no longer speak dialog on their death
       after 'The Pursuit'.
     * The maps for 'Infested Caves', 'Clearing the Mines', and 'The Pursuit'
       have been completely redone.
     * All scenarios up to 'Old Friend' have been significantly tweaked and
       rebalanced.
   * Sceptre of Fire:
     * Balancing changes for 'The Dragon'.
   * Son of the Black Eye:
     * S2: adjusted starting villages; scenario now ends if Kapou'e is already
       on the signpost when the messenger comes back
     * S3: make objectives consistent between before and after trolls appear
     * S6: remove AI controller.  It doesn't work with sides without a leader
     * S6: reduce randomness of unit types unloading from galleons
     * Minor balance tweaks to scenarios 7, 8, 12, 14, 15, 17 and 18.
     * Made allied leaders less likely to get themselves killed.
     * Minor changes to objectives and messages in scenarios 7, 8 and 14
   * The Hammer of Thursagan:
     * Fixed missing objective in 'The Court of Karrag'.
     * Rebalanced scenarios 'Invaders', Mages and Drakes' and 'Fear'.
     * Set Masked Dwarvish Lord range attack same as for Dwarvish Lord
     * Fixed some messages not being shown in custom events in 'The Underlevels'
     * 'The Court of Karrag' and 'Epilogue' feature a new map.
   * The Rise of Wesnoth:
     * Hide unit variations that should not be listed in the help browser.
   * Tutorial:
     * Rewritten with an eye to being less constricting and more informative.
   * Under the Burning Suns
     * Fixed recruited Desert Archers being always male.
     * Fixed malformed rand range errors during AI turns on medium difficulty
       in 'A Stirring in the Night'.
     * Fixed bug #22800: Cloaked Figure appears in strange location.
     * Fixed bug #22799: No allies killed by enemy reinforcements.
     * Fixed bug #22790: Various problems with dehydration.
     * Hide unit variations that should not be listed in the help browser.
 ### C++ Engine
   * Purge "human_ai" controller type. This is a fixup of bugfix #18829 below.
 ### Editor
   * Added an entry for the terrain description help to the context menu.
   * Default hotkey bindings for brushes (1-5)
   * Disabled the swap palette button when the unit palette is active.
   * Disabled the tod schedule and playlist menus in pure map mode.
   * Support for displaying the saved status of the selected area.
   * Fixed error messages about missing UI elements.
   * Fixed falcon race missing an icon due to having incorrectly-named image
     files.
 ### Graphics
   * New Ancient Lich baseframe and animations
   * New standing animation (NE and SE) for the Revenant
   * New standing animation for the Dwarvish Stalwart
   * Smooth unit movement over terrains with elevation such as keeps and
     bridges (bug #20635).
   * Fixed bug #22045: Only blit neutral surfaces.
   * [item] images are now subject to local ToD lighting effects instead of
     just the current map-wide ToD lighting (bug #22215).
   * Disabled "alpha thresholding" in the bilinear interpolation algorithm
     used for all sprites and images. (This was introduced as a workaround
     for an old bug.)
   * Add a new bilinear interpolation implementation, which properly performs
     alpha weighting when averaging colors, and rename old one as the legacy
     version.
   * Align unit overlays to the sprite rather than the occupied hex.
   * Added Brazier and Lit Brazier embellishment terrains
   * New beach waves terrain animation.
   * New images for the dummy, barrel, and green cloak items
   * Updated Snowy Orcish Castle and Keep terrain images
 ### Help browser
   * Several sections have been rewritten or had their layout improved
   * Added trait list with links to the help page for unit types
   * Added Defense Cap column to the Terrain Modifiers table in unit help pages
     for units with a defense cap on at least one terrain type, which also now
     include a note to this effect in their special notes.
   * Fixed a bug which caused the terrain section topic not to properly generate
     a list of its contents.
   * Add a new help section to the terrains topic which explains about mixed
     terrain types.
   * When using the terrain description feature with a mixed terrain type, the
     game will autogenerate a formatted explanation of its best / worst movement
     / defense properties.
   * Added automatic help entries for all eras, fulfilling Feature Request
     #22107
   * Fixed in-game help descriptions of default factions, using content from
     wiki
   * Added automatically generated lists of races and alignments to the
     descriptions of factions
   * Autogenerated Time of Day Schedule section
   * Unit types that do not include any visible (hide_help=no) variations no
     longer generate topic sections.
 ### Language and i18n
   * Updated translations: Chinese (Simplified), Chinese (Traditional), Czech,
     French, Galician, German, Greek, Hungarian, Italian, Lithuanian, Polish,
     Portuguese, Russian, Scottish Gaelic, Slovak, Spanish, Vietnamese
   * Fixed cmake pot-update target on Debian and other systems that do not use
     bash as the default /bin/sh.
 ### Lua API
   * Upgraded Lua to version 5.2.3.
   * Fix bug #21761: wesnoth.synchronize_choice will now give a warning when
     the table returned by the user-specified function is not completely valid,
     when wesnoth is running in debug mode (--debug command line flag).
   * Added new function wesnoth.get_all_vars().
   * Add (tentative, preliminary) support for lua scripting of wesnoth
     application functions
   * Enable ilua "strict mode" by default. This safety feature helps to catch
     errors caused by mistyped variable names, and improves the behavior of the
     lua interpreter console. See 1.13.0 RELEASE NOTES for more details.
   * Add multiplayer client scripting (join lobby, chat, host games, save games,
     reload games) and mp unit tests based on this
   * Add functions in wesnoth.map_location to perform map location arithmetic
     using the same functions the C++ engine does
   * Enabled support for the bit32 library (bitwise operations)
   * Added support for treeview wigets in lua gui2 dialogs.
   * Added support to define wml conditional tags in lua. See the
     wesnoth.wml_condtionals table as described on the wiki.
 ### Multiplayer
   * Fixed the Set Password option during game creation not having an effect
     due to a misplaced WML attribute in the client's command for the server
     (bug #23015).
   * Fixed missing scenario title in lobby for reloaded MP campaigns.
   * Added "no mirror" and "no ally mirror" options to the MP configure screen.
   * Multiplayer content [era], [modifications] now carry version info of their
     associated add-on, and may have "addon_min_version" fields set which
     specify backwards compatibility with earlier versions.
   * Missing content for games hosted in the mp server may now be retrieved
     automatically by clicking on these games in the mp lobby.
   * Ported the password prompt displayed when joining a password-protected MP
     game to GUI2 (bug #23455).
 ### Music
   * Change main menu track to Journey's End.
 ### Replays
   * Added a button that allows playing a single move in replay mode.
 ### Units
   * Increased the experience requirement for the Rami from 32 to 39
   * Increased the experience requirement for the Saree from 56 to 64
   * Changed sound timings for Khalifate melee attacks
   * New chill tempest animation for Lich/Ancient Lich
   * Add 'elemental' trait to Mudcrawler line
   * Fix sound timings for drake fire animation macros
   * Assigned capitalized translatable names to Walking Corpse and Soulless
     variations (bug #22902).
   * Hid alternate Great Wolf variation from help as it is identical to the
     default variation stats-wise.
 ### User interface
   * Added a context menu option for command mode.
   * Added a Classic theme which restores the 1.10 UI.
   * Made orb and minimap colors configurable in Preferences.
   * Remove 'allow_new_game=no' entries from random map new game list
   * Fixed bug #22095: An assertion failure in the gamestate inspector.
   * Changed: A listbox can now update its size when rows are added.
   * Changed: Avoid listboxes to handle mouse clicks twice.
   * Fixed bug #22144 + #22046: An assertion failure with empty labels in
     a listbox.
   * The :inspect dialog now uses the same function as saved games to generate
     WML in text form instead of a simplified version.
   * Added a button to copy the currently displayed content from the :inspect
     dialog to clipboard.
   * WML array elements are now displayed with subscripts in the :inspect
     dialog.
   * Added a button to copy the in-game Chat Log dialog contents to clipboard.
   * Fixed the WML load error dialog not displaying an add-on name instead of
     falling back to its directory name if the add-on contains an outdated
     _info.cfg file lacking an [info]title= value.
   * Fixed most of the minimap buttons and the End Turn button appearing
     without contents or in the wrong state during WML start events until they
     are interacted with or control is given to the player for the first time.
   * Added a new subdialog in Preferences to clean or purge the WML cache.
   * Fixed AI engine names in the MP game setup screen being translated to the
     language selected when Wesnoth was started rather than the current
     language (bug #22092).
   * Added a new "Advanced Settings" button and dialog to campaign select.
   * Added new wml attributes to listbox: has_minimum and has_maximum.
   * Added a "Disable automatic moves" preference to disable automatic movements
     at the beginning of a turn.
   * Fixed lower button row padding for GUI1 dialogs including Statistics and
     the Add-ons Manager (bugs #22379, #22791).
   * Fixed mouse tracking issue with workaround by changing the default window
     resolution for OS X to 800 x 600 (bug #20332).
   * Removed the "Replay viewer" text label from the replay controller theme,
     because it caused problems for translators and was unnecessary
   * Fixed bug #22337: Bug in inspect long arrays
   * Added a new "Alerts" configuration screen under Preferences > Multiplayer,
     refactored all of the mp alerts / desktop notifications for customizability
   * Removed "desktop_notifications" and "lobby_sounds" advanced preferences,
     as they are subsumed by the Preferences > Multiplayer > Alerts screen now.
   * Added support for hyperlink rendering and clicking in gui2 labels.
   * Added lua interpreter console, accessible via a hotkey
   * Add lua console button to gamestate inspector
   * Add command history and history expansion to lua console, based on optional
     support from GNU readline history library.
   * Add support to evaluate expressions automatically in lua console, based on
     a lua lib "ilua", and "experimental compilation" technique.
   * Fixed game falling back to desktop if a user declines to load a previous
     version save from within a game
   * Fix bug #22984: Sliders (GUI1) focusing improperly in non-game contexts,
    and not responding to left-right key press
   * Wesnoth now ships with Bold and Oblique forms of its default font, Deja Vu
     Sans, and these are used in many menus and in the in-game help for styling
     text where previously we used SDL_TTF. Fixes bug #22376.
 ### WML engine
   * New image path functions:
      * ~XBRZ(): Takes one argument, the scaling factor (1-5). Uses the xBRZ
        scaling algorithm added in this release.
      * ~SCALE_SHARP(): Anologous to ~SCALE() but using the nearest neighbor
        scaling algorithm instead.
      * ~PLOT_ALPHA(): No arguments, plots the alpha channel in greyscale.
        Mainly useful for diagnostics of other function combinations, or
        debugging the engine itself.
      * ~WIPE_ALPHA(): Again mainly useful for diagnostics.
      * ~ADJUST_ALPHA(): Takes either a % or an 8-bit integer, and "multiplies"
        the alpha channel by this value.
      * ~SEPIA(): Gives the image a sepia tint.
      * ~NEG(): Gives the image a photographic negative effect.
   * Added customizable recall costs for unit types and individual units,
     using the new recall_cost attribute in [unit_type] and [unit] (bug #13074).
   * Added support for [elseif] tags inside [if]
   * Schema validator and preprocessor #warning messages now conform better to
     the new WML parser/preprocessor diagnostics format introduced in version
     1.11.10.
   * Added define= functionality to scenarios, eras, and modifications.
   * [unit] uses passed [advancement]s instead of unittypes advancements if
     [advancement] were given in [unit].
   * allow [lua] tags inside [modification] and [era] (which have the same
     effect as [lua] tags inside [scenario]).
   * enable side_for= in key [message] with [option]s.
   * added [effect] apply_to=fearless/healthy which is now used by
     healthy/fearless traits instead of hardcoding the id of those traits in c++.
   * added support for a shuffle key in the [music] music to allow selecting
     between random and non-random music play (bug #19653)
   * Fixed a bug that prevented [animate_unit] from displaying death or victory
     animations for those units that filter them based on weapon (eg. Wose)
   * New WML tags: [put_to_recall_list] and [sync_variable].
   * Fixed nested macros emitting incorrect bindings to the default 'wesnoth'
     textdomain at the end of a substitution instead of the parent textdomain,
     this breaking localization of subsequent strings (bug #22962).
   * Sounds for slow and poison are automatically played when slowing/poisoning
     a unit, and should no longer be played by the attack animations.
   * force_lock_settings defaults to yes in [scenario].
   * Moved all preprocessor diagnostics from the 'config' log domain to
     'preprocessor'.
   * Dropped support for [filter_side] in [gold] and [modify_ai], [theme] name=
     in place of id, and [object] duration=level.
   * Allowed direct modification of unit.ellipse variable even if ellipse= is set
     in [unit_type] or by an [effect]
   * Add accelerate=yes/no key in [delay]
   * Add support for status= key in SUFs
   * Add "disallow_shuffle" key in [side], which causes the "shuffle sides"
     feature always to skip this side, even if it is human playable.
   * Add "require_scenario" key in [scenario], [multiplayer], which causes a
     multiplayer game to require that all participants have the add-on installed.
 ### Miscellaneous and bug fixes
   * replace 'fight_on_without_leader'=yes/no with defeat_condition=no_leader/
     no_units/always/never which allows the wml developer to decide when a side
     is considered defeated
   * Fix Fisher-Yates implementation of Lua helper.shuffle (bug #21706)
   * Fixed issues with the ~BG() image path function not always working with
     cached images.
   * Idle controlled sides now serialize as human controlled.
   * Fixed bug #20876: A segfault in cut_surface.
   * Dropped support for AmigaOS, BeOS, and OS/2.
   * Increased the sound mixer channel allocation from 16 to 32, thereby
     raising the limit for simultaneous sound effects from 5 to 20, and
     simultaneous UI sounds from 1 to 2.
   * Fix bug #21758: "Ready not blocked while player pick faction."
   * Fix bug #18829: "ai sides show up as "controller=network" on remote
     clients". This issue is the source of some difficulties with mp campaigns
     which occur when the campaign is reloaded from a non-host side, or after a
     player rejoins from observer status. Hopefully, reloading campaigns is
     easier after this.
   * Fix bug #21797: "Mandatory WML child missing" when leaving a reloaded game.
   * Fix bug #21808: Cannot join a reloaded game as an observer.
   * Fixed halos glitching through locations that become shrouded after the
     halo is rendered for the first time.
   * OS X user data directory is now ~/Library/Application Support/Wesnoth_1.13.
     For compiling Wesnoth using Xcode, a new version of
     wesnoth_compile_mac_1.13.zip is also required.
   * Fix bug #21257: Lagging animations with skip AI animations and fog/shroud.
   * Improved unicode handling on windows for characters outside the Basic
     Multilingual Plane.
   * Fix bug #3856: The turn dialog used in hotseat MP play now applies
     a blindfold for the duration of the dialog.
   * Petrified units are no longer displayed in the "Damage versus" tooltip.
   * Fix bug #21759: "timer refreshed too often when time runs out"
   * Use one combo box instead of check boxes for replay options "skip replay"
     and "enter blindfold". This fixes the mp lobby in width <= 800 resolutions.
     Fixes bug #21888.
   * MP server now commits controller changes to the replay rather than updating
     as we go along. Among other things these means that players that join an
     scenario with ai sides which has already started won't have corrupted
     controller types which would prevent them from successfully saving and
     reloading in the future.
     We have also moved all "controller tweaks" associated to the start of MP
     games to server-side rather than having a mix of client and server code.
   * Fixup user-displayed strings associated to replay options, idle state
   * OS X: mark Wesnoth as not high-resolution capable. This greatly improves
     performance on retina devices.
   * Upgrade Xcode project to enable both i386 and x86_64 builds
   * Fixed problems with idle controller type in networked mp.
   * Fix bug 21459 by making dropped sides default to idle.
   * Fix bug 21882 by introducing "fight_on_without_leader" boolean attribute
     of [side] tags and refactoring check_victory to use this.
   * Changed: Don't use the random generator for units with no names.
   * Fix bug #21910: code for game is ready bell is corrected, simplified, and
     moved to mp_connect::process_network_data.
   * Deliver desktop notifications in tandem with "game is ready" bell and
     also the "game has begun" bell.
   * Fix bug in which blindfold could cause OOS.
   * Fix bug #21914: allow drake walking corpse variation to move on unwalkable
   * Fix unnecessary "Mandatory WML Child not found" error when replay file
     doesn't have a [carryover_sides_start] tag
   * Server now generates PR 121 compliant replay files.
   * Fix bug #21025: replay controller doesn't execute play_next_side properly
   * Fix bug #21916: ready blocked at inappropriate times
   * Fix bug #21931: controllers bugged in (basic campaign) when networked
   * Fix bug #21883: make sure movement animations don't cycle with fog on
   * Fix bug #21316: make subframes within standing animations cycle by default
   * Fix bug #21967: fix crash when unit modification to traits has empty id
   * Fix bug #19258, 21962: WML variables spuriously copied to replay_start
   * Fix implementation bug in random number generator: rand_pool_ is now an
     unsigned long rather than an unsigned int.
   * Fix bug #21491: fix drag+drop for unit movements
   * Fix bug #21448: make premoved units selection like in Wesnoth 1.10
   * Fix bug #21372: fix unit move continuation if enemy discovered
   * Added a Python/Tkinter based GUI to allow running wmllint, wmlscope and
     wmlindent in an easier way
   * Some commandline output which was helpful for noninteractive ai testing
     is redirected from std::cout to a logdomain "aitesting".
   * Added WML unit test capabilities, accessible from commandline using -u
     switch. These are useful for unit testing the WML / lua api.
   * Support for total conversions, so called "cores".
   * Fix bug #22030: correct index of weapon special disable in attack GUI
   * Fix bug #21964: assertion failure when using "controller" attribute with
     a number.
   * Add [do_command], which takes the body of a [command] tag from ReplayWML,
     (some possibilities not allowed), and executes the corresponding action.
     It is replay safe and triggers WML events as appropriate.
   * Reallow selection of another unit on same side without deselect first
   * Add "strict mode". Using --log-strict=[severity] causes wesnoth to throw
     a game_error exception when anything is written to a log as severe as
     that. For example --log-strict=warning causes both warnings and errors
     to generate runtime exceptions. This is intended for unit tests.
   * Fix bug #21867: team flag colors not refreshed after making use of
     [modify_side] color= unless a new flag set is also provided.
   * Fix a bug where in movement records, "skip_sighted" was sometimes
     spelled as "skip_sighed".
   * Fix bug #22020: make base units with variations help entry clickable
     from the 'Unit Description' menu entry or from side pane.
   * Fix bug #21977: ready blocked on scenario transition, when
     allow_new_game=yes
   * Add "MAKE_ENUM" macro to simplify parsing of WML options.
   * "MAKE_ENUM" macro signals WML parsing errors when the game is run
     in debug mode, and an illegal value for an attribute is encountered.
   * Fix bug #21397 Reloading may cause a side's turn is not initialized.
     This bug affected only networked mp, at least since version 1.8.
   * [modify_side] may now alter the value of [side] defeat_condition.
   * Fix bug #22116: unlock movement when attacking enemy+discovering in fog
   * In mp the other players now also proceed to the next scenario if one human
     player wins.
   * Fix bug #21397: "Saving and loading may cause a side's turn is not
     initialized".
   * Fix bug which caused no units to be displayed when reloading an end of
     scenario save, by stripping them from the save file. However, this does
     not fix bug #15545, which was the reason that this behavior was introduced.
   * Fix bug #22123 "Replays don't reset map when reset button is pressed,
     causing OOS"
   * Wesnoth reports an error if it evaluates [if] and finds no [then], [else],
     [elseif] children, as a chat message from lua.
   * Fix bug #22134: Campaign prefix not used in mp campaign saves
   * Made the error messages sent to stderr when the core data dir is
     incorrectly set more helpful.
   * Fix bug #20126: Recursive preprocessor includes cause infinite loop
   * Added 'faction_lock' and 'leader_lock' to SideWML to be used in MP Connect
     screen. Fixes bug #21978.
   * Fix bug #22231: partial moves now able to be continued in whiteboard
   * Added wmi_pager object to control how wml menu items are displayed in
     context menus, and allow to display more than seven.
   * Fix an inefficient implementation of unit::invisible, in an effort to
     address slow performance problems:
     http://forums.wesnoth.org/viewtopic.php?f=4&t=12139&start=180#p569931
     (Also see gfgtdf's commits trying to optimize the minimap loop)
   * Made it so <exe location>/../ (non-Windows) or <current working dir>/../
     (Windows) are also considered possible data directories if they contain a
     data/_main.cfg file, intended to help with cmake builds.
   * Fix bug #21723: team-specific items displayed to wrong players / at wrong
     times
   * When parsing command line args, if we fail to parse, give an error message
     rather than crashing with no explanation.
   * Fix bug #22305: assertion failure in unit_display.cpp
   * Fix bug #22086: wrong or missing minimap previews displayed b/c of problems
     with saveindex
   * Fixed the SCATTER_UNITS macro so that it may no longer attempt to place
     units at the map borders.
   * Fixed bugs in [filter_vision] reported here:
     http://forums.wesnoth.org/viewtopic.php?f=21&t=40702 [filter_vision] is now
     a check for *any* side matching the filter to have appropriate vision.
   * Add [has_ally], [has_enemy] to work around issues reported here:
     http://forums.wesnoth.org/viewtopic.php?f=21&t=40702 In a side_filter,
     [has_ally] and [has_enemy] are corrected / simplified versions of
     [allied_with] / [enemy_of], this is an extension of the bugfixes for
     [filter_vision].
   * Fix [side] share_view/maps always beeing false for non-first levels in mp
     campaigns
   * Fixes start-of-scenario multiplaer saves (#22068)
   * Fix bug wherein "lobby sounds" advanced preference didn't do anything for
     the normal (GUI 1) lobby.
   * Fix bug wherein dbus notifications did not have a wesnoth icon
   * Fix bug wherein chat history synopses weren't working right in the dbus
     notifications
   * Default to classic minimap unit/village color coding (colored by side).
   * Default to classic minimap terrain representation (satellite view).
   * Display the tooltip for actions browsed in the hotkey preferences dialog.
   * Enabled action icons in the hotkey preferences dialog.
   * Fix bug #21717: "F5->reload wml tree" doesn't work in editor.
   * Fix bug #21298: Minimap shows invisible overlays
   * Remove carryover WML (this may make a reappearance in 1.13.0 but it is
     considered premature now, please see github PR discussion for more info)
   * Fix bug #22465: Minimap not redrawn after turn dialog.
   * Fix bug #22306: move_unit moves a unit even when it shouldn't
   * Fix minor bug where toggling accelerated speed caused two lines of print
     messages to display on top of one another. Reported here:
     http://forums.wesnoth.org/viewtopic.php?f=5&t=40745
   * Fix bug #22611: loading a theme with nonexistent button images would cause
     the program to close.
   * Fix bug #22487: advanced preferences descriptions not showing up
   * Fix bug #22646: Tooltips broken in replay viewer
   * Fix bug #22643: Cannot compile with boost 1.56
   * Fix issue where the chatlog for a replayed game could not be opened in
     single player. The chatlog can now always be opened.
   * Fix bug #22745: choose_track function crashes the game in certain cases
   * Fix bug #22650: nontranslatable strings displayed in tooltips for terrain
     icons
   * Fixed Gameplay -> Time of Day help topic displaying the Dawn ToD picture
     where the Second Watch picture should be used instead (bug #22537).
   * Fixed mishandling of nested subnamespaces by the [clear_global_variable]
     WML action causing an assertion failure (bug #21093).
   * Fix bug #22443: Scenario Settings table has inaccurate Start Gold info in
     certain cases
   * Fix Grand Knight image being distorted in the sidebar
   * Fix bug #22251: Map labels not appearing in mp
   * Fix bug: lobby sounds not all playing. This was fixed by adding a second
     dedicated channel for UI sounds, and moving the "Ready to start" sound to
     the turn bell channel.
   * Bug fix: another instance of "overlapping messages", this time with the
     planning mode activation hotkey
   * Fix bug #21400: Use unit 'image' with higher priority than 'image_icon'
     for animation frames with no specified image.
   * New filesystem implementation based on boost filesystem
   * Added "--noaddons" command-line argument, prevents any add-ons from being
     loaded.
   * Added "--render-image" command-line argument, similar to --screenshot, but
     instead of a map, it takes a wesnoth imagepath and generates at bitmap
     with the filters applied.
   * New gettext implementation based on boost locale. This raises min boost
     requirement to 1.48, and adds a new dep (locale, filesystem was also added
     above)
   * New image scaling routine imported from the xBRZ project (HqMAME on
     sourceforge)
   * Now may save screenshots and other images as pngs, using SDL_SavePNG lib
     (from github)
   * New RNG implementation using boost::mt19937, and boost::random_device for
     seeds. The boost random library is now a build requirement.
   * Add "lua" random map generation
   * Multiplayer server can now handle scenarios with more than 9 sides
   * [modify_side] controller= now works in networked mp. If a null-controlled
     side becomes alive by [modify_side] controller=ai/human then the currently
     active client will be assiged controll
   * Fixed a stingstream syntax error that caused the build to fail in Visual
     Studio 13
   * Fixed hotkeys for changing tips in titlescreen (bug #18926).
   * Fixed problems building wesnothd using cmake if not building the game
   * Added several new options to GUI.pyw
   * Fixed a bug in which sides without a leader, but not defeated due to
     defeat_condition set to no_units_left or never couldn't capture villages
   * Fixed several Mac OS specific bugs in GUI.pyw
   * Fixed bug 22987 for filtering on era and mods in the Multiplayer lobby.
   * Fixed bug 23117 Invert breaks lobby filtering if Apply Filter is off.
   * Fixed bug 23426 Show terrain description hotkey works also for shrouded
     hexes.
   * Fix bug #23243: Segfault while clicking during [delay] in prestart.
   * Remove support for legacy style unit abilities descriptions.
   * Fixed bug #23445: set default build type in cmake to "Release" to ensure
     that the game is not unoptimized
   * Increased minimum version of FriBiDi to 0.10.9 in cmake to match scons
   * Changed how FriBiDi is found in cmake to using pkgconfig
   * Made the file chooser dialog (used for e.g. loading maps in the editor)
     use uniform path delimiters on Windows, fixing issues with browsing to
     parent directories (bug #23224).

## Version 1.11.11
 ### Add-ons server
   * Filenames with whitespace in them are no longer allowed.
 ### AI
   * Non-default multiplayer and Micro AIs:
     * Fixed bugs which could lead to the AIs being disabled for the rest of the
       turn if WML events removed or changed units during the AI turn
     * Improved error reporting of invalid AI actions
 ### Campaigns
   * Eastern Invasion:
     * Updated maps for scenarios 12, 14, 16 and 17a.
     * Fixed a bug in 'Captured' which can cause the beginning to make no
       sense.
   * Under the Burning Suns:
     * Fixed broken Divine Incarnation unit type in The Final Confrontation.
 ### Language and i18n
   * Updated translations: German, Italian, Slovak
 ### WML engine
   * Bug #21643: Removing fog from a single hex no longer makes the hex ugly.
   * WML files whose names contain whitespace trigger preprocessor errors.
   * Bug #21722: Event handlers with multiple names never fired.
 ### User interface
   * Corrected most of the issues left with the new default theme.
     * Reintroduced the alignment, race and side being shown in the sidebar.
     * Adjusted the theme to the size and shape of the new minimap frame images.
     * Certain changes to the used text colors, sizes and alignment.
   * Non-team labels no longer remove team labels that were present in the
     same hex.
   * New colors for the Light Red and Dark Red minimap markers.
   * Bug #21724: 'none' is now a special case for [unit_type] ellipse
 ### Miscellaneous and bug fixes
   * Units can no longer be moved in linger mode (bug #21450).
   * Changed: Updated valgrind suppression file.
   * Fixed color issues with font_rgb in unit status labels in themes.
   * Labels are now removed when shroud/fog is removed, rather than waiting
     for a new turn (bug #21434).
   * Percent signs show when describing traits that increase damage or attacks
     by a percentage (bug #21577).
   * Linux dbus notifications: Only last 5 messages are remembered, and they
     are displayed with the most recent ones first.
   * Fixed bug #21736: MP create screen always defaulting to top entry.

## Version 1.11.10
 ### Add-ons client
   * Fixed faulty add-on _info.cfg files causing the game to display obscure
     error messages or crash to desktop.
   * Generated _info.cfg files now contain the list of dependencies for an
     add-on as well (needed by wesnoth_addon_manager).
   * Python wesnoth_addon_manager client:
     * The dependencies= attribute and the [feedback] tag in .pbl files are
       now properly supported (bug #21189).
     * Generated _info.cfg files now contain the same information as the
       game's built-in client (type and title were missing).
 ### AI
   * Hang Out Micro AI: default AI [avoid] aspect is now taken into account
   * Fixed problems with several Micro AIs that sometimes produced OOS errors
 ### Campaigns
   * Eastern Invasion:
     * New world map.
     * Rewrote scenario 'Training the Ogres' and renamed it to
       'Capturing the Ogres'.
     * Rewrote scenario 'Captured'.
   * Heir to the Throne:
     * Fixed Delfador clobbering whichever unit happens to be standing on
       31,11 at the end of The Bay of Pearls, causing it to disappear
       forever.
     * S15 (The Lost General): fix bug of sighted events firing too early
   * Legend of Wesmere:
     * Fixed missing journey map background in story screens.
   * Liberty:
     * Updated sprites for Shadow Mage line.
   * Son of the Black Eye:
     * Rebalancing of the campaign is now complete
   * Under the Burning Suns:
     * Updated animation WML of all campaign specific units
 * Editor:
  * Fixed: Drawing the offmap area for small resolutions.
 ### GUI2
   * Added: FAI-function handling in GUI2 widgets.
   * Added: A new tooltip window.
 ### Language and i18n
   * Updated translations: Scottish Gaelic
 ### Lua API
   * Config of current era is now available in a Lua table in MP games
   * Config of any era can be requested by id, also a list valid era ids
 ### Multiplayer
   * Fix for bug #21405, in a series of features:
     * The abort option presented to the host when a player disconnects
       from a networked game is now a "save and abort" option.
     * New Idle controller status: Sides may now be set in an "idle" state
       by the host when a player disconnects from a network game. This does
       not give any player control or vision. To proceed with the game, the
       host must reassign the side's controller using :control, :droid, or
       :give_control as usual. (give_control existed but was not documented)
       Related to this, there are new commands :controller which query the
       controller status, and :idle which toggles the idle status.
     * New "Blindfold Replays" option: Observers may check a box in the
       lobby so that if they join a game, they will be "blindfolded" and see
       only a black screen until they are given control of a side.
 ### Units
   * New baseframes for Jundi, Muharib, Batal, Qatif-al-nar, Qudafi, Rasikh.
 ### User interface
   * New UI for displaying errors detected during the core and add-on WML
     loading process (parser and preprocessor errors), including the
     ability to copy the report to clipboard.
   * New UI for displaying the notification that a screenshot or map
     screenshot was successfully saved to disk, including options to open it
     in an external application, copy the path to clipboard, or browse the
     screenshots folder.
   * Force grayscale antialiasing for text rendered using Cairo/Pango (e.g. by
     GUI2) on Windows to work around ClearType-induced glitches (bug #21648).
   * Fixed bug #21584: Properly redraw the minimap when the minimap is
     resized.
   * Fixed: Enable blurring in the title screen.
   * Added descriptions to the options in Preferences -> Display -> Themes.
   * New sound played to signal the start of an MP game.
 ### WML engine
   * WML loading phase errors are reported to stderr in a new indented format.
   * Implemented [true] and [false] ConditionalWML tags, which describe a
     condition that always yields true or false, respectively.
   * Fixed: Disallow change and remove sections without an id in the ThemeWML.
   * Added [theme] description attribute for including a description of the
     theme that will be displayed in Preferences.
   * [theme] name attribute is now expected to be translatable and used only
     for the theme selection UI. Existing [theme]s need to be converted to
     have a separate 'id' attribute.
   * [endlevel] now has two optional subtags [next_scenario_settings],
     [next_scenario_append], which can be used to reconfigure next scenario.
   * Optimizations made to the game events engine. Slower machines may notice
     an improvement during movement (when enter/exit_hex events are triggered).
     The optimization is more effective when relatively few events have variables
     in their names.
 ### Miscellaneous and bug fixes
   * Fixed: A compilation warning with DEBUG_WINDOW_LAYOUT_GRAPHS.
   * Added -Wold-style-cast to the CMake strict flags.
   * Made sure that cmake does not add -NDEBUG for release builds since this
     flag breaks building.
   * Updated screenshots used inside the ingame help and fixed description of
     orbs.
   * Fixed bug #21659: lua location_set:empty now works as described
   * Users now get a warning if they start a multiplayer scenario through the
     title screen load button, as this may cause eras and modifications not
     to work correctly in subsequent scenarios of an mp campaign.
   * wmllint can now update base terrain aliases in UMC after the changes in
     versions 1.11.8 and 1.11.9. This conversion is applied to the aliasof,
     mvt_alias, and def_alias attributes under the [terrain_type] tag.
   * Parser warnings when skipping over Unicode BOMs are now printed in stderr
     with the file location and substitution trail when available.

## Version 1.11.9
 ### Add-ons client
   * Display the first and last upload dates in the Description dialog.
 ### Add-ons server
   * Record the first upload date and time for new add-ons.
   * Removed ancient compatibility code used only for add-ons stored by some
     1.5.x versions.
 ### AI
   * Default AI: Gold saving is turned off by default again
   * New macros AI_SAVE_GOLD and AI_SAVE_GOLD_DEFAULT for easy enabling of
     recruitment gold saving in specific scenarios
   * Wolves Micro AI: new optional parameter attack_only_prey=
 ### Campaigns
   * Heir to the Throne:
     * Gave Li'sar a new ability "initiative" (grants adjacent allies first
       strike in melee).
   * Son of the Black Eye:
     * Rebalancing of the campaign continues and is done for Scenarios 1 (End
       of Peace) through 12 (Giving Some Back).  In addition, the following
       not directly balance related changes have also been made:
     * S3: remove AI controller right-click menu option
     * S4 & S9: make AI attack enemies 1 XP from leveling so that it is not
       possible to block key locations with such units
     * S10 & S12: don't give huge unannounced gold bonus to AIs
 ### Editor
   * Added Impassable Overlay and Unwalkable Overlay terrains to the obstacle
     group.
   * Added Snowy Human City village terrain to the frozen group.
   * Added Cave Path terrain to the flat group.
   * Added Dry Hills terrain to the fall group.
 ### Language and i18n
   * Updated translations: Dutch, Portuguese, Scottish Gaelic
 ### Lua API
   * Added wesnoth.set_dialog_markup function (patch #2759).
 ### Multiplayer
   * Updated map: Ruins of Terra-Dwelve.
 ### Terrains
   * Made Snowy Encampment, Snowy Orcish Castle, Snowy Encampment Keep and Snowy
     Orcish Keep aliases of both castle and frozen terrains
 ### Units
   * Decreased the strikes of the Dwarvish Lord's hatchet attack from 2 to 1.
   * Fixed subtle magenta TC for the Giant Mudcrawler sprites not being
     enabled in-game.
 ### User interface
   * Restored the old control scheme as the default
   * Fixed hidden variations of unit types (hide_help=yes) being listed in the
     help browser when they shouldn't.
   * Gray-out GUI1 scrollbar upwards scrolling button by default when starting
     with the view scrolled to the top.
   * Truncate long Advanced Preferences entries with ellipses to avoid
     situations where the listbox is wider than the Preferences dialog frame
     (bug #19482).
   * Team color is now applied on the Unknown unit icon in the game Status
     Table regardless of whether the side's leader unit supports team color.
 ### Miscellaneous and bug fixes
   * Added -Wno-documentation-deprecated-sync to the CMake pedantic flags.
   * Fixed several Doxygen issues found by Clang 3.4.
   * Fixed possible invalid memory access issue in the MP sides configuration
     code causing crashes for some users (bug #21449).
   * Fixed broken image references in the Gameplay -> Time of Day help topic.
   * The internal variables used by the LIMIT_RECRUITS WML macro are now
     cleared on victory.
   * Fixed missing log error message for invalid music tracks set with
     play_once=yes (bug #21479).
   * Don't force the .gz suffix on every entry of the save_index (bug #20849).
   * Fixed a bug in [filter_vision] in SUFs that caused a hidden unit under
     fog/shroud to produce a false positive.
   * A lack of ToD schedule no longer causes segfaults (bug #21489).
   * SLF work again when x XOR y is specified (bug #21488).
   * Selecting off-map hexes, then hovering over a unit no longer causes
     the game to crash (bug #21351).
   * Changed: Added -Wextra-semi to pedantic compilation.
   * Changed: Added -Wconditional-uninitialized to pedantic compilation.
   * Fixed NULL pointer dereference when viewing units in the Recall Unit
     dialog including nonexistent/unreadable images in their overlays, while
     not in debug mode.

## Version 1.11.8
 ### Add-ons client
   * Introduced new add-on type "SP/MP Campaign" for campaigns with
     "type=hybrid."
   * Fixed invalid file size data from the server crashing the client on the
     network transfer progress dialog (bug #20893).
   * Added support for specifying a feedback page URL in the .pbl file when
     publishing an add-on, currently intended for associating add-ons in the
     official add-ons server with topics from forums.wesnoth.org; this is
     achieved by including a [feedback] block with a topic_id=<number> key in
     it.
   * Redesigned Add-ons Description dialog, including support for displaying
     add-on feedback page URLs.
 ### Add-ons server
   * Fixed mishandling of inaccessible add-on packs resulting in multiple data
     conversion errors and stalling clients (bug #20893).
   * Added support for managing and emitting add-on feedback page URLs to
     clients ([server_info] feedback_url_format option in the server
     configuration file).
 ### AI
   * RCA AI: fix bug #21334: surrounded units don't attack
   * Coward Micro AI: new optional parameter [filter_second]
   * Simple Attack Micro AI: new optional parameter weapon=
   * Wolves Micro AI: fix bug that sometimes kept predators from attacking
   * Lua AI: new replay-safe action ai.synced_command()
   * ai.cfg: fix MEDIUM to NORMAL in attack_depth macro
 ### Campaigns
   * all: convert many wmllint magic comments from "recognize" to "who" and
     "whofield",
   * Dead Water:
     * New world map.
   * Delfador's Memoirs:
     * Updated sprite and animations for the Wose Shaman.
   * Descent into Darkness:
     * S3: set aggression=1 for Side 4 to avoid wrong choice of attack
   * Eastern Invasion:
     * Made Dacyn use teal TC and Mal-Ravanal blue TC, to make them fit the
       portraits more.
     * Updated maps for scenario 1-7.
   * Heir To The Throne:
     * Increased Li'sar's lvl3 hitpoints from 52 to 62.
     * Implemented the portrait variations for Delfador and Asheviere.
     * Changed Kaylan's portrait and gave him teal team coloring.
     * Changed the flaming sword so it's now a 25% increase to damage, instead
       of changing the damage to 15-4
     * Added a new mechanic to Sceptre of Fire. By standing still for a turn,
       Delfador can now tell the player the shortest path to the Sceptre.
     * Fixed Konrad's level 1's attack animation giving an 'image not found'
       error.
     * Fixed Konrad's dying words event.
   * Legend of Wesmere:
     * S9: set aggression=1 for Side 4 to avoid wrong choice of attack
   * Liberty:
     * New world map.
     * S5: set aggression=1 for Side 3 to avoid wrong choice of attack
   * Northern Rebirth:
     * S5a: dialogue tweaks
   * The Rise of Wesnoth:
     * New world maps.
     * Redesigned scenario 'A New Land'.
   * The South Guard:
     * S6a: fix ogre's last words event
     * S6b: set aggression=1 for Side 2 to avoid wrong choice of attack
   * Son of the Black Eye:
     * Rebalancing of the campaign continues and is mostly done for Scenarios 1
       (End of Peace) through 8 (Silent Forest).  In addition, the following
       not directly balance related changes have also been made:
     * S1: the AI enemy can now also recruit bowmen
     * S4: give the player control of the Side 3 orcs in the center castle
     * S6: unload units from transport galleons preferentially onto land hexes
     * S7: use Simple Attack Micro AI to have scorpions spread poison
     * S8: use Healer Support Micro AI for elvish healers
     * S16: Kapou'e gets his own castle at the start of the scenario to
       eliminate dependence on luck during the first turn
     * S17: add a warning that the AI will receive reinforcements
     * S18: no linger mode at the end of the last scenario
     * Minor updates to messages (grammar and prose) and objectives.
     * Updated sprites and animations for the Orcish Shamans.
 ### Graphics
   * New and updated animations for the Loyalist Horseman.
 ### Language and i18n
   * Updated translations: Catalan, Chinese (Traditional), Dutch, Galician,
     Japanese, Latin
 ### Lua API
   * Added flag, flag_icon, and village_support fields to wesnoth.sides table
     elements.
   * Made wesnoth.sides[n].hidden a read-write field.
   * New lua proxy table "wesnoth.game_config.mp_settings" for access to
     MP specific settings, such as era, scenario name, and timer
 ### Multiplayer
   * Unit names and genders are synced in MP games.
   * Added new CampaignWML attribute "require_campaign". If set to "yes",
     players not having campaign installed won't be able to join the game.
   * New eras: the Default+Khalifate and Age of Heroes+Khalifate eras are now
     available.
 ### Replays
   * Replays include the prestart and start events again.
   * Unit names and genders are synced between games and replays.
   * Play/stop buttons are disabled again at the end of a replay.
   * The 'reset replay' button works correctly and does not cause OOS
     errors any more.
 ### User interface
   * Removed the possibility to undo unit recruits because it caused oos.
   * Added a party full bell to the MP game configuration screen, played once
     all human player slots have been taken.
   * Change layout for advertized games in the MP lobby and add map icon.
   * Moved color cursors option to Advanced Preferences.
   * Always hide and disable color cursors option on Mac OS X since it's known
     to cause severe lags that render the cursor unusable.
   * Unit overlays are now displayed in the Recall dialog, both on the list
     and the description panel.
   * Made filtering controls on the MP create screen functional.
   * Removed the MP custom options dialog; all options are now shown directly
     on the configuration screen.
   * Removed the MP modifications dialog; modifications are now displayed
     directly on the creation screen.
   * The "Compressed saves" and "Compress savegames using bzip2" options in
     Preferences -> Advanced have been replaced by a single option,
     "Compressed saved games", that lets the user pick between gzip (default),
     bzip2, and no compression. Users who previously enabled bzip2 compression
     will need to do so again.
   * Hide eras menu in MP Create for campaigns which have
     "allow_era_choice=no".
   * Introduced side's name in MP Connect.
   * Middle click scrolling is now based on distance from initial click instead
     of the centre of the screen.
   * Make sliders able to be scrolled with the mouse wheel
   * Allow advanced preference booleans and mp modifications to be toggled
     via double click
   * Fixed slight scrolling glitches with credits sections with multi-line
     headers (e.g. those generated for campaigns with multi-line titles).
 ### WML engine
   * WML variable turn_number is set correctly (to 1) in prestart and start
     events.  Previously, it retained its last value from the previous scenario
     until after the start event.
   * [scroll_to] and [scroll_to_unit] now take an optional side filter.
   * [trait] now accepts a "generate_description=" attribute, allowing the
     auto-generated effect descriptions to be turned off.
   * [modify_side] can now change a side's flags and status bar icon using the
     "flag" and "flag_icon" attributes also accepted in [side] definitions
     (bug #18454).
   * [store_side] now stores the "flag", "flag_icon", and "village_support"
     attributes from sides.
   * New macros RECALL and RECALL_XY
 ### Miscellaneous and bug fixes
   * Pango markup is applied correctly and consistently in button tooltips.
   * Fixed mishandling of invalid Pango markup resulting in previous messages
     being displayed instead in e.g. [message] (bug #20996).
   * Added wmllint code for recognizing unit id fields in macros, added
     non-attribute lines to local_sanity_check, added unknown speaker check.
   * Refactored code in wmltools to create a macro-parsing function.
   * Added era descriptions.
   * Fixed file chooser dialog (used in the map editor and for locating the
     wesnothd executable) interpreting special markup at the beginning of file
     names such as "#foo.map".
   * Fixed bug with modifications dependency check dialogs (bug #21365)
   * Fixed bug with scrollbar overlaying mp description text (bug #21364)
   * Fixed bug with help units not making links (bug #21339)
   * Split command line option --config-dir into --userconfig-dir and
     --userdata-dir, with --userconfig-dir defaulting to --userdata-dir's
     value on some platforms.
   * The color_adjust_blue_ attribute in [display] tags of saved games has
     been renamed to color_adjust_blue. Since it is only non-zero following a
     [color_adjust] action in a WML event, only mid-scenario saved games
     created with previous versions may present minor color issues after this
     change.
   * Fixed sound sources removed while the sound effects volume is zero
     (either in Preferences -> Sound or through the [volume] WML action)
     persisting and escaping the sound source management code (bug #21426).
   * The negative sign is no longer dropped when formula AI prints numbers
     between 0 and -1.

## Version 1.11.7
 ### Add-ons client
   * Add-ons downloaded with clients from this version onwards are shown
     in the Remove Add-ons with their original titles from the add-ons
     server now, instead of artificially generating titles by replacing
     underscores from file names.
   * Fixed color key disparities between the Add-ons Manager dialog and the
     add-on Description dialog.
   * It is now possible to choose the neutral/fallback add-on sorting in
     the Add-ons Manager dialog used when no columns have been sorted by
     the user.
   * Report errors caused by faulty local .pbl files properly instead of
     presenting them as network errors.
   * Fixed regression from 1.11.0 resulting in memory leaks and/or
     crash-to-desktop situations when failing to connect to an add-ons
     server.
 ### Add-ons server
   * Implemented read-only mode option (read_only boolean attribute in
     server config WML, defaults to "no").
 ### AI
   * Recruitment CA:
     * New Recruitment CA located in src/ai/recruitment is now default CA
     * Created new AI cfg "Strong AI (RCA)" with stronger recruitment
     * Created new AI cfg "Old Recruitment CA" in ai/dev/ to use the old CA
   * External CAs are now fully usable
     * The persistent data variable works and is accessible across CAs
     * Parameters can be passed to eval/exec functions
   * Micro AIs:
     * New Simple Attack Micro AI
     * All MAIs changed to using external CAs.  This means that no engine
       definitions are needed any more and that MAIs can be combined at will
     * Got rid of animal_type=, guardian_type=and recruiting_type= keys.  The
       individual Animal, Guardian and Recruiting MAIs are now separate AIs
       that are selected with the ai_type=key
     * Patrol, Guardian and Hunter MAIs are now side-wide CAs (not BCAs) and
       support either the id= key or [filter] tag for unit selection
     * Messenger MAI now works with units (both messenger and escort) without
       weapons and when path to next waypoint is entirely blocked
   * Experimental AI:
     * Added new CAs Retreat, Move to Enemy and Village Hunting
     * Algorithm improvements for several CAs
   * Fixed time_of_day aspect so it matches time-of-day ids, not names
     (e.g. "Morning" vs "morning"), so most use cases of ToD-dependent AI
     configuration work again (suspected version 1.7.4 regression).
   * Move_Leader_To_Keep CA will now move all leaders back to a keep
     (multiple leader support for recruitment)
   * Refactored the AI-Test-Suite completely (/utils/ai_test/)
   * Changed AI descriptions in ai/dev/ displayed in MP computer player menu
   * New macro AI_CONTROLLER_ALLOW_UNIT_CONTROL
   * Remove macro RCA_STAGE
 ### Campaigns
   * A Tale of Two Brothers:
     * Crop campaign icon to fit campaign selection dialog (fixes bug #20935)
   * Dead Water:
     * Converted animation WML to the new syntax
   * Delfador's Memoirs:
     * Converted animation WML to the new syntax
   * Descent Into Darkness:
     * Converted animation WML to the new syntax
   * Eastern Invasion:
     * Converted animation WML to the new syntax
   * Heir To The Throne:
     * Converted animation WML to the new syntax
     * Gave Moremirmu a matching holy sword attack icon (bug #21248).
   * Legend of Wesmere:
     * Removed now redundant MP code.
     * Added new CampaignWML attributes: "type", "min_players", "max_players".
     * Added new ScenarioWML attributes: "new_game_title", "force_lock_settings".
     * Adjusted "controller" and side recruits for MP.
     * Now uses default recruitment instead of experimental FAI-recruitment
       in 02_Hostile_Mountains.
   * Liberty:
     * New AI for wolf riders in "The Raid"
     * Converted animation WML to the new syntax
     * Gave the Death Squire the submerge ability.
     * Increased the Death Squire's blade resistance from 10% to 40%,
       pierce resistance from 30% to 60% and lowered its impact resistance
       from -10% to -20%.
     * Greatly decreased the Death Squire's HP from 66 to 44.
   * Northern Rebirth:
     * S5a: added dialog for dungeon signpost moveto
   * Sceptre of Fire:
     * Converted animation WML to the new syntax
   * Son of the Black Eye:
     * Rebalancing of campaign started to make it more difficult in accordance
       with its status as an expert level campaign.  Scenarios 1 - 3 done so
       far.  In addition, the following, not directly balance related changes
       have also been made:
     * S3: scenario now ends after dwarf leader is killed (no need to continue
       to the signpost any more)
     * S3: add possibility to give Grüü instructions
     * S6: new Lua AI code for the transports with somewhat different behavior
     * S14: Kapou'e cannot recruit troll whelps any more (this scenario only)
     * S14: "It is now Grüü's turn" displayed correctly on Side 4 turn
     * S16: fix recalling of alternative units if Jetto/Inarix have been killed
     * S18: Howgarth's death now results in defeat, as stated in objectives
     * S18: Shan Taum death dialog shown correctly if Kapou'e kills him himself
     * Minor updates to many messages, objectives, moves, AIs etc.
     * Message images that would cover the speaker moved to the right
     * Converted animation WML to the new syntax
   * The Hammer of Thursagan:
     * Converted animation WML to the new syntax
   * The Rise of Wesnoth:
     * Converted animation WML to the new syntax
   * The South Guard:
     * Converted animation WML to the new syntax
   * Tutorial:
     * Converted animation WML to the new syntax
   * Under the Burning Suns
     * Fixed several bugs with missing/incorrect dialog
     * Azkotep now correctly possesses Garak if only his champion is killed
 ### Engine
   * CampaignWML support for MP including difficulties, extra defines etc.
   * Use the same codepath to initialize every MP campaign's scenario, thus
     allowing the display of a functional MP "Connect/Wait" screen before the
     next scenario.
   * Reload game config for non-host players in order to match the host's
     config, if possible. Reloads will only happen for MP campaigns using
     CampaignWML syntax.
   * Imagepath function "~ROTATE": Support for rotating by any degree.
 ### Graphics
   * Full animations for the Dwarven Thunderer line.
   * New standing animation for the Dwarvish Berserker.
   * New north-facing standing and attack animations for the Loyalist Spearman.
   * New ranged attack animations for the Dark Adept line.
 ### Language and i18n
   * Updated translations: British English, German, Greek, Indonesian, Italian,
     Korean, Lithuanian, Portuguese, Vietnamese
   * Improved internationalization of notifications on Windows and OS X.
   * Introduced translations for months and weekdays, which are used when user
     does not have locale installed.
   * Show am/pm designations, if required, even when locale doesn't support
     them.
 ### Lua API
   * Added scroll_to_leader field (read/write) to wesnoth.sides table
    elements.
 ### Multiplayer
   * Fixed clearing map data when there is no shroud and level is sent
     with "store_next_scenario".
   * Update game's side data, slots and state in server during next scenario
     initialization.
   * Fixed reserved sides being counted as available in server.
   * Added "controller_lock" in SideWML. The lock provides a way to be sure
     that sides will be played with a controller which was assigned in WML.
     I.e. if "controller" was set to "ai", it won't be possible to select any
     other controller for a side. However, if "controller" was set to "human",
     it will still be possible to assign any player, local, network or
     reserved (if applicable) controller.
   * All multiplayer locks in SideWML (e.g. "team_lock"), now uses
     "force_lock_settings" as their default value.
 ### Unit changes and balancing
   * Gave the Death Knight the submerge ability.
   * Increased the Death Knight's blade resistance from 10% to 40%,
     pierce resistance from 30% to 60% and lowered its impact resistance
     from -10% to -20%.
   * Lowered the Death Knight's HP from 66 to 63.
   * Lowered the Chocobone's hills defense from 50% to 40%,
     mountain defense from 60% to 50%, fungus defense from 60% to 40%,
     forest defense from 50% to 30%, and village defense from 60% to 40%.
   * Increased the Chocobone's melee damage from 9-2 to 11-2.
   * Increased the Chocobone's blade resistance from 10% to 20%.
 ### User interface
   * Players can now assign hotkeys to wml menu items in the preferences menu.
   * Help Browser:
     * Units with variants are shown as sections with their childs as topics.
     * Links to the siblings in unit variants.
     * Links to the base unit of none variants.
   * MP "Create" screen has been revamped by moving scenario configuration
     widgets to a new MP "Configure" screen, introducing a description box,
     adding a combo selection for different types of game, and adjusting it to
     work well with MP campaigns.
   * MP "Configure" and "Connect" screens have been adjusted to allow to see
     all scenarios and sides in the debug mode.
   * MP "Lobby" game item has been updated to display information about
     campaigns correctly.
   * In-game Chat Log dialog now supports text search on timestamps, nicks,
     and messages.
   * Possible fix for clipboard integration issues with GUI2 widgets on X11.
   * Fixed potential misbehavior (including invalid memory access) from the
     tray notifications code on Windows.
   * New Game Paths dialog displaying filesystem locations used by the game
     to read or write data, accessed from Preferences -> General.
 ### WML engine
   * New [disable] weapon special.
   * New variation_id attribute with the function of former variation_name.
   * variation_name is now the translatable name of the unit variant.
   * The "variation=" attribute works now in [unit_type] and serves
     as the default variant.
   * [store_side] now stores the scroll_to_leader attribute from sides.
   * Added new CampaignWML attributes: "type", "min_players" and "max_players".
   * Added new ScenariowML attributes: "new_game_title",
     "remove_from_carryover_on_leaders_loss" and "force_lock_settings".
   * Allow numerical values for SideWML "controller" attribute.
   * Allow WML menu items to use hotkeys alongside/instead of the menu.
   * Really added sighted events for ambushing units.
   * [scroll] now takes an optional side filter.
   * Some support for negative healing. This is not guaranteed to work correctly
     in all cases, but it does restore the basic functionality that was
     (probably accidentally) in 1.10.
   * Overhaul of the game events engine. Fixes some obscure bugs, like #21031,
     and grants feature request #18713.
   * Added new set_icon attribute for [effect] apply_to=attack.
   * New attribute in [unit_type]: healed_sound. These
     attributes allow for customization of the sounds played when the unit
     is healed.
 ### Miscellaneous and bug fixes
   * Added -256 and -512 color shifts to FADE_TO_BLACK and FADE_TO_BLACK_HOLD
     macros to account for ToD color shifts greater than -31, guaranteeing
     complete darkness.
   * Unit WML frames with image modifications are now shown correctly for
     hits/death.
   * Fixed compare_images.py (called by wesnoth-optipng) and pofix.py to
     function with python-3.2.
   * Refactored and split the MP "Create" and "Connect" screens' code into
     gui and engine parts.
   * Lock faction, leader, and gender selections in MP "Connect" if default
     values are provided and "force_lock_settings" is set to "yes".
   * Fixed runtime error due to mismatched function call conventions in
     set_preferences_dir() when built with MSVC++ 2010 and a relative path
     to My Documents was passed with --config-dir in the command line.
   * Changed: Added -Wno-deprecated-register to strict compilation.
   * Added command line option "--multiplayer-repeat <arg>".
     A game started with --multiplayer will be repeated <arg> times.
     This is useful for batch testing.
   * Animation WML: Fix sound start time in additional frames without
     requiring a duration 1 frame workaround.
   * Animation WML: Fix halo in additional frame persisting without blank
     hex workaround at start and end if a shorter frame.
   * New MISSILE_FRAME_FIREBALL_XY macro, used in red mage line.
   * Fixed crash on delete of last save.
   * The MISSILE_FRAME_MUZZLE_FLARE_MISS macro nowrequires an X and Y
     argument.
   * Added placehoder attack animations to the Fire Dragon.
   * New MISSILE_FRAME_CHILL_WAVE and MISSILE_FRAME_SHADOW_WAVE macros,
     used in the dark adept line.
   * New knife sound for thunderer line.
   * Fix bug #20936 with shuffle sides and incorrect initial side villages.
   * Fix bug #20564: crash of replays of games started from scenario start save
   * Fix bug #20124: animation not updated on [un]petrifying units with WML tags
   * Fix bug #21264: Loading a turnsave in campaign sets negative gold to 0
   * Fix unreported bug of Wesnoth sometimes crashing on killing unit under
     cursor from context menu in debug mode

## Version 1.11.6
 ### Add-ons client
   * The Description popup displays the selected add-on's dependencies now,
     using color-keyed statuses.
   * Circular dependencies are no longer treated as erroneous in terminal
     output.
 ### AI
   * Micro AIs:
     * New Hang Out Micro AI
     * Any number of Micro AIs can now be combined on the same side
     * Goto MAI: new parameters avoid_enemies=, ignore_units= and ignore_enemy_at_goal=
     * Goto MAI: [filter] is now an optional parameter
     * ca_id is now an optional parameter for all MAIs
     * ca_score is now an optional parameter for almost all MAIs
     * Bug fixes for Patrol, Messenger, Multipack Wolves and all Guardian MAIs
     * [goal] tags in [ai] blocks with turns= or time_of_day= will now work.
 ### Campaigns
   * An Orcish Incursion:
     * New world map.
   * Descent Into Darkness:
     * New world map.
   * Legend of Wesmere:
     * convert to "wmllint: who" magic comments
   * The South Guard:
     * Fixed double "lich found" event in Choice in the Fog
 ### Language and i18n
   * Updated translations: Galician, Japanese, Lithuanian, Russian, Scottish Gaelic
 ### Lua API
   * Added: function wesnoth.find_cost_map().
 ### Units
   * Added description for the Wose race.
 ### User interface
   * Unit ellipses are now automatically updated if a unit gains or loses its ZoC
 ### WML engine
   * If [recall] cannot find a unit to recall, the message is logged at the
     "info" severity level in the "wml" domain (instead of the "error" level in
     the "engine" domain). This means the message will be suppressed by default.
   * Allowed [modify_side] to modify the scroll_to_leader key
 ### Miscellaneous and bug fixes
   * Creating a unit via debug mode now clears fog/shroud around the unit.
   * [move_unit_fake] now accepts an optional force_scroll= attribute (def. to
     'yes') that allows scrolling the viewport even when [lock_view] is in
     effect or Follow Unit Actions is disabled in Advanced Preferences.
   * [move_unit] accepts an optional force_scroll= attribute like
     [move_unit_fake] above, defaults to using the [move_unit_fake] default.
   * Added: Feature in pathfind.xpp to build a cost map.
   * wmllint: turn on globbing on Windows, create "who" and "unwho" magic
     comments, bugfixes
   * wmllint-1.4: add an enhanced and bugfixed version of wmllint 1.4 for
     porting old add-ons

## Version 1.11.5
 ### Add-ons client
   * Do not bump the download count for add-ons that are currently installed
     and are being reinstalled or upgraded
 ### Add-ons server
   * Restricted names (not titles) for newly uploaded add-ons to ASCII hyphens,
     underlines, and alphanumerical characters. Previously uploaded add-ons
     are exempt from this change.
 ### AI
   * New macro EXPERIMENTAL_AI for using Experimental AI in both SP scenarios and MP maps
   * Recruiting in Micro and Experimental AIs: allow more terrain codes for castles/keeps
   * Improved/Added: Command 'lua wesnoth.debug_ai([side]).ai' will now give access to the
     ai-table of [side].
   * Improved AI behavior when using goto_x / goto_y in WML
   * Micro AIs:
     * Zone Guardian: Add optional station_x,station_y= keys
     * Goto: don't exclude leader by default
     * Bug fixes in Return Guardian, Healer Support and Goto MAIs
   * Experimental AI: fix bugs that disabled recruiting for some UMC weapon specials
 ### Campaigns
   * All:
     * replaced deprecated aspects with [goal] aspect
   * Heir to the Throne:
     * Updated Li'sar's movetype and resistances to match her armor.
   * Legend of Wesmere:
     * Scenario 05: Elvish Horse Archers can now carry the treasure
     * Scenario 09: the player's loyal units stay in the recall list, instead of
       being stored and then unstored in scenario 14
   * Under the Burning Suns:
     * Scenario 8 (Out of the Frying Pan): Fix messenger AI (Lua errors and wrong goal)
 ### Engine
   * Made game config loading more modular by extracting appropriate
     functionality from game controller and adding it to a new class.
     Wrapper functions for editor's and game's config loading were added.
 ### Graphics
   * New scorpion baseframe (replaces old image set)
   * New skeletal dragon baseframe (replaces old image set)
   * New cavalryman line baseframes (replaces old image set)
   * New elven cavalry units baseframes (replaces old image set)
   * New paladin, knight, grand-knight, lancer baseframes (replaces old image set)
   * New giant mudcrawler baseframe and animations (replaces old image set)
 ### Language and i18n
   * Updated translations: French, Galician, Latin, Lithuanian, Old English,
     Portuguese, Portuguese (Brazil)
 ### Lua API
   * location_set.of_pairs() now can take coordinates defined by x/y= keys as well
   * Fixed the x1,y1 variables in enter/exit_hex events handled via
     wesnoth.game_events.on_event.
 ### User interface
   * Added notification support for Windows
   * Made it so that the "Back to Turn X" and "Back to Start" menu items work
     with .bz2 saves
   * Removed Dfool and Experimental themes
   * Major visual overhaul:
     * Most UI elements have new images
     * Button sizes have been standardized; regular buttons are 108x22 (H22), menu
       buttons are 100x20 (H20), square buttons are either 25x25 or 30x30 depending
       on purpose.
     * Regular and menu buttons are now in subdirectories of images/buttons/
     * Button icon overlays are now in images/icons/
     * Icons can now be overlaid on a team-colorable base provided in images/buttons.
     * New team color ranges have been introduced for this purpose.
   * Added possibility to kill unit under cursor with context menu in debug mode.
 ### WML engine
   * Added new aspect 'advancements' which with lua engine can handle a
     function return type of the form f(x, y) -> String. 'advancements'
     tells the AI to what unit a given unit should advance to.
   * Fix "Shuffle sides" incorrect behaviour: children inside [side] were also
     swapped.
   * Fixed incorrect image path function name in error messages generated by
     using ~CROP() with negative coordinates.
   * Fixed abilities sometimes affecting self during movement even when they are
     flagged as not affecting self (bug #20755).
   * Added deprecation messages to aspects [target], [protect_unit], [protect_location],
     protect_leader=, protect_leader_radius=
   * Fixed bug #20836: advancing units can fire events in [harm_unit] if fire_event=yes
   * Advancing units can be animated by [harm_unit] if animate=yes
   * Added deprecation message to aspect [goal] when using "name=protect_my_unit".
 ### Miscellaneous and bug fixes
   * Fixed minor issue with Drake Clasher animations
   * Added a new playlist FULL_MUSIC_PLAYLIST, which contains all Wesnoth tracks
     in alphabetical order
   * Added -Wno-null-conversion to the CMake pedantic flags.
   * Changed: Decreased the pango cairo dependency to version 1.21.3.
   * Changed: Mark system headers as system headers in CMake.
   * Fixed the recall list sometimes getting confused when dismissing a recall.
   * Prevent unchecked memory access in cut_surface()
   * Remove game controllers: new and abstract. Were never properly implemented.
   * Remove --new-syntax command line option. Was never properly implemented.
   * Several wmllint additions: remove backslashes and userdata/ from paths, convert
     data/campaigns/ to data/add-ons/, give more detailed message for color spec to
     Pango fix-up, delete files on Windows before rename.

## Version 1.11.4
 ### AI
   * Reverted new Lua AI persistent storage mechanism for external CAs that
     would crash Wesnoth under certain circumstances in 1.11.3. This will be
     re-committed in an upcoming release.
   * Micro AIs
     * Healer Support MAI now respects RCA AI [avoid] aspect.

## Version 1.11.3
 ### AI
   * Micro AIs
     * New Goto and Zone Guardian MAIs
     * Change SUF/SLF names to [filter]/[filter_location] and variants thereof
     * Add optional SUF to Healer Support MAI
 ### Campaigns
   * Descend into Darkness:
     * Fix the surviving orc leader starting poisoned after scenario 5.
 ### Language and i18n
   * Updated translations: Chinese (Traditional), Indonesian, Italian, Polish,
     Portuguese, Russian, Vietnamese
 ### User interface
   * Fixed 'end turn' button's state in MP and title2 issues.
   * Fixed (bug #17220): Cursor in gui2 text box now behaves appropriately after
     text box overflow occurs.
   * Change Difficulty checkbox in Load Game dialog in GUI1 gets disabled if
     change in difficulty of a loading game won't take effect (bug #20381).
   * Fixed 'end turn' button's state issue: button stays disabled after the first turn.
   * Fixed bug #20592: dialog (wml_message.cfg) is shifted by the
     distance between the left edge of the screen and the game map
   * Overhaul of the editor's gui
 ### WML engine
   * Fixed invalid memory access issues caused by subnamespaced persistent WML
     variables (bug #20385).
   * fix negative gold carried over (bug #20676)
   * Allow filtering on controller= in SSFs. Cannot be used in networked multiplayer.
   * When controller in 'side' tag is ai, use 'no' as default value
     for 'allow_player' attribute.
 ### Miscellaneous and bug fixes
   * Switched to git version control. Hosting moved to SourceForge
   * Improved: Autorevision based revision numbers in CMake.
   * Don't let petrified units reveal ambushers

## Version 1.11.2
 ### Add-ons client
   * Fixed bug #20518: identical add-ons titles not supported
   * Add a button to the Filter Options dialog to toggle all currently
     displayed add-on categories at once
 ### AI
   * Experimental Multiplayer AI
     * Improve recruitment, notably first turn choices and units with poison
       and charge
     * Improved selection of units for village stealing
     * Remove dependency on AI-demos add-on
     * Fix bug when playing on maps with a turn limit
     * Fix bug handling regeneration
     * Minor improvements in switching between castles
     * Add healer support micro AI to improve healer use
     * Improved village capturing
     * Retreat badly injured units more effectively
   * New [micro_ai] tag, 18 different Micro AIs, and 14 test scenarios
     * This includes AIs for 7 different animal behaviors, bottleneck defense,
       2 different guardians and a coward, healer support, lurkers,
       messenger escort, patrol, protect and move a unit, and 2 alternative
       recruiting strategies.
     * Documentation at https://wiki.wesnoth.org/Micro_AIs
   * New leader_ignores_keep AI aspect that lets AI leader take part in the same
     AI moves as the non-leader units.
   * Rename passive_leader_shares_keep candidate action to leader_shares_keep.
     The old syntax still works for backward compatibility (for both CA and
     macros).
   * Fix minor bug in leader_shares_keep candidate action that occurred
     under some circumstances when the AI leader does not have full MP
 ### Campaigns
   * Sceptre of Fire:
     * Allow game to continue after completing scenario (reported in forums)
   * Son of the Black Eye:
     * Prevent infinite loop if fewer than two transport ships (bug #20389)
 ### Graphics
   * New bigmaps for the title screen and campaigns HttT, TSG, SoF and SotBE
   * Fix layering error with bridges
 ### Language and i18n
   * Updated translations: British English, Chinese (Traditional), Dutch,
     Estonian, French, Galician, German, Italian, Japanese, Latin, Lithuanian,
     Portuguese, Portuguese (Brazil), Slovak, Vietnamese
 ### Lua API
   * new wesnoth.get_time_stamp() function
   * new helper.shuffle() function
   * The wesnoth.transform_unit() function no longer performs a full heal. It
     will (still) enforce the resulting unit's maximum hit points, though.
   * The wesnoth.transform_unit() function will automatically remove poison from
     a unit that is immune after transforming.
 ### Multiplayer
   * Moved new lobby option in Preferences -> Multiplayer to Advanced
     Preferences and clarified description
   * MP command-line mode now works correctly, including replays, random maps
     and default values (bugs #19853, #19877, #19883, #19895 and #20009)
   * New --ignore-map-settings MP command-line option
   * Fixed some issues with Dark Forecast skipping spawns and failing to end.
 ### User interface
   * Allow copying the selection in the old (default) lobby using
     Ctrl+C/Command+C (bug #5877)
   * Color coded the resistance table in the hp display's tooltip.
   * Scale down unit baseframes larger than 72x72 in the Recruit and Recall
     dialogs to prevent all list entries from being enlarged to fit
   * Tooltip for the movement points display shows the movement costs.
   * Updating the shroud after delaying shroud updates is done gradually instead
     of instantly.
   * Fixed (bug #18970): Moving a unit after closing a click dismiss dialogue.
   * Increased bottom padding for story screen text when it reaches the bottom
     of the screen.
   * Fix viewport centering issues with actions such as the Next Unit command,
     and the [scroll_to], [scroll_to_unit], and [message] WML actions (bug #18793).
   * Removed the "move unit to hexfield" feature.
   * Rearranged :inspect dialog elements to provide the data visualization
     panel with more horizontal space.
 ### WML engine
   * [unit_overlay] and [remove_unit_overlay] now return a more meaningful
     error message if the image= key is missing
   * When not replacing values, [effect] apply_to=defense will now modify
     absolute values instead of signed values (bug #20242). This allows for
     cleaner WML when the unit type is not necessarily known in advance.
   * Split the 'not_living' unit status into 'unpoisonable', 'undrainable' and
     'unplagueable'. 'not_living' now acts on the whole group
   * The bugs with sighted events have been resolved.
   * A unit's maximum hit points are more regularly applied. This affects
     [transform_unit], [effect]apply_to=type, and [effect]apply_to=variation.
   * Poison is automatically removed from immune units after using
     [effect]apply_to=type or [effect]apply_to=variation.
   * Fixed bug #20401: [remove_unit_overlay] did not work when the image used an
     image path function that took multiple parameters.
   * Refactored [base_unit] to be more robust in oddball cases.
   * Improved support for setting id= within a [variation].
   * Added support for square bracket expansion in animation strings I.E
     halo="pic[1-2,5]:[10,20,30]" expands to halo="pic1:10,pic2:20,pic5:30"
     this is used for halos, team flags, animated terrains and unit animations
   * Changed default unit halos and macros to use new square bracket expansion
   * Fixed bug #20468: Update the owned villages when using [replace_map].
   * Changed [vision_costs] to default to movement costs on a per-terrain basis.
 ### Miscellaneous and bug fixes
   * The undo stack is preserved across a save-reload.
   * Removed several unused private member variables.
   * Fixed the present in-game command line autocompletion feature so it
     actually works without using :debug
   * Removed recognition of the --smallgui command line option, which has been
     superfluous since 1.9.5.
   * Made wmllint recognize victory_string, defeat_string,
     gold_carryover_string, and notes_string (all from the [objectives] tag) as
     attributes that need to be made translatable
   * Added -Wdocumentation to the CMake pedantic flags.
   * Make drakes fly on volcano tiles (bug #20485).
   * Added a FOREACH macro to emulate C++11 for(... : ...) style loops.
   * Fixed bug #20290: No longer terminate due to a corrupt savegame.
   * Fixed bug #19970: No longer terminate due to corrupt preferences.
   * Changed: Increased the pango cairo dependency to version 1.24.4.
   * Fixed a display artifact (halos not cleared) when WML moves a unit.
   * Improved the display of vision costs in the unit help.
   * Fixed wmllint, wmlscope and wmlindent not working correctly on Windows if a
     command line argument ends with a backslash
   * Fixed: Compilation with CLang 3.2 and libc++.
   * Added: Autorevision based revision numbers in CMake.

## Version 1.11.1
 ### AI
   * New AI: Experimental AI
     * Improved recruitment, castle and village management over current default AI
 ### Campaigns
   * Dead Water:
     * Removed duplicated loyalty overlay (that now is in core), and used
       {IS_LOYAL} macro
     * Stunned units are now marked with a status icon
   * Delfador's Memoirs
     * Add dummy side to cutscene to avoid crash at end of story part
       (bug #20208)
   * Eastern Invasion:
     * New set of portraits
   * Legend of Wesmere:
     * Scenario 02: Replaced the moveto events for checking the
       dwarvish borders with enter_hex events
     * Scenario 08: Add dummy side to cutscene to avoid crash at end of story
       part (bug #20208)
   * The Rise of Wesnoth:
     * Fix bug #16772: AI won't attack
   * Under the Burning Suns:
     * Keep a spawned unit from disappearing on reload in scenario 6b
     * Stunned units are now marked with a status icon
     * Fix bug #19303: dwarf sometimes spawns in cave wall
 ### Graphics
   * New portraits: Tentacle of the Deep
   * Updated brown lich alternative portrait
   * Fixed glitches caused by larger-than-hex standing units with certain frame
     image effects when image= is not explicitly specified (bug #20099)
   * Fixed standing unit animation glitches caused by [move_unit_fake] on
     adjacent locations (bug #20098)
   * The special ellipses for leader and hero units now support team coloring.
 ### Language and i18n
   * New translation: Scottish Gaelic
   * Updated translations: British English, Chinese (Traditional), Croatian,
     French, Galician, German, Hungarian, Italian, Latin, Lithuanian, Polish,
     Portuguese (Brazil), Russian, Slovak, Ukrainian
 ### Lua API
   * new wesnoth.have_file() function
   * wesnoth.get_time_of_day() now works when specifying a turn when the number
     of turns is unlimited
 ### Multiplayer
   * Silver Mage no longer allowed as a leader in Age of Heroes
   * Added support for modification tags
   * Added support for dependencies between eras, scenarios and modifications
   * New [options] tag to allow MP add-ons to add their own settings on the game
     creation screen
 ### Networking
   * Handle an exception in the SDL_net-based network code (bug #20205)
 ### Terrains
   * Added Unwalkable Overlay terrain (^Qov)
   * Added Dense Palm (^Ftp), Savanna (^Fts), and Rainforest (^Ftr) terrains
   * Updated graphics for palms (^Ftd), tropical forest (^Ft),
     plank bridge (^Bp*), stone bridge (^Bs*), and chasm bridge (^Bcx*)
 ### User interface
   * Healing animations are now played when poison is cured.
   * Moving units to a selected hex field
     * Units in reach of a hex field are highlighted during selection.
     * Highlighting and labeling the selected hex field with the amount of in
       range units.
     * Left click on a controlled one triggers the move.
   * The recruit and recall commands are restored when right-clicking on a
     leader, but with new semantics -- only that leader's recruits/recalls will
     be presented as options.
   * Fog/shroud clearing has been reworked to be more timely and consistent.
   * The statistics window can now show per-scenario statistics.
   * The sidebar now includes specials when reporting a weapon's damage.
   * The sidebar now includes all specials when reporting a weapon's number of
     attacks (not just swarm).
   * Active/inactive highlighting for abilities and weapon specials in the
     sidebar.
   * Fix broken MP game creation dialog on low resolutions
   * The "repeat recruit" command now refers to the last recruit by the current
     side, rather than the last recruit by the current game client.
   * Refinements to undo stack management, sometimes allowing moves to be undone
     after "update shroud now" (if those moves did not affect fog/shroud).
 ### Whiteboard
   * Don't end turn if executing all actions in another way than using the
     "end turn" button/hotkey. (bug #19901)
 ### WML engine
   * The recall, recruit, prerecall, and prerecruit events will now block
     undoing unless they contain [allow_undo].
   * The cost of a recall/recruit is now paid between the prerecall/prerecruit
     and recall/recruit events. (FR #16711)
   * Sighted events should be reliable, provided the player does not activate
     delayed shroud updates (which is still a major caveat).
   * Added [effect] apply_to=overlay
   * Added [effect] apply_to=experience
   * Added [terrain_type] max_light= and min_light=.
   * Standardize weapon filters, supporting special=, [and], [or], and [not]
     wherever weapons can be filtered.
   * Image path functions again evaluated left-to-right. Fixes bug #20196.
   * Support for [swarm] causing attacks to increase as health decreases.
   * Inactive names and descriptions for abilities and weapon specials will
     default to the active name/description if omitted.
   * Fix a bug with [store_time_of_day] so that it treats the provided variable
     as a container instead of an array (overwrite instead of append).
   * Added [side]suppress_end_turn_confirmation= for those (rather nonstandard)
     scenarios where players often skip their turns.
 ### Miscellaneous and bug fixes
   * Fix invalid memory access crash resulting from deleting all saved games
     in the Load Game dialog
   * Redesigned the hotkey preferences dialog
   * Removed two Khalifate leftovers (Hakim portrait and KHALIFATE_NAMES macro)
   * Ambush now works for desert palms and dead great trees (bug #20207)
   * Hex field size and default terrain are wml configurable
   * RCA AI renamed from 'testing' to 'default'
   * Fix OOS when dismissing a recall in a multiplayer campaign (bug #19924).
   * Fixed a bug disallowing the left shift and meta key to be detected in
     the hotkeys
   * Added -Wuseless-cast to the CMake pedantic flags.
   * Added -Wc++11-compat to the CMake pedantic flags.
   * Added bzip2 support for savefiles. (new dependency)
   * Fix OOS when observe scenario 2 in a multiplayer campaign (bug #20217).
   * Synchronize underlying_unit_id in MP campaigns (bug #20227)
   * Fixed: Compilation with Boost 1.51.0 (Gentoo bug #440742).
   * Added -Wnoexcept to the CMake pedantic flags.
   * Fixed a rare case where a player could exercise (very) limited control of
     another (human) player's units in a hotseat game.
   * Added project files for CodeLite.

## Version 1.11.0
 ### Add-ons client
   * The Update Add-ons dialog has been replaced with an alternate view mode
     for the main Add-ons Manager
   * The Add-ons Manager now allows filtering add-ons by installation status
     (all, installed, not installed, upgradable)
   * Add-ons in the Add-ons Manager are initially sorted by title instead of
     the order they were originally uploaded to the server
   * Add-ons with broken/unavailable dependencies are reported to the user
   * The add-on dependencies prompt reports all recursively-resolved
     dependencies, not just the direct dependencies
   * Add-on installation, upgradability, and publish status is reported as a
     footnote for every list entry, plus a field in the Description dialog
   * The Add-ons Manager filter textbox is preserved during the same
     connection (i.e. while installing multiple add-ons)
 ### Add-ons server
   * Reject add-ons with names or titles that begin with a WML text format
     markup character (any of *, `, ~, {, ^, }, |, @, #, <, &)
   * Switched to port 15006 for 1.11.x and adjusted all tools to this port
 ### AI
   * In akihara_recruitment, analyzing battle field - we take village as
     important spot. The aim of it is to list terrain couple according to these
     important spot so we can evaluate battle simulation on them.
   * In akihara_recruitment, the struct situation became a class.
   * New AI formula 'aki_eval' for testing a battle evaluation
   * AI now properly considers the expected damage from poison when attacking
     using poisoners.
   * Adding a new scenario for the ai-arena-small in order to test the new AI.
   * Adding new files for the new AI (ai/akihara/recruitment.?pp)
 ### Campaigns
   * Added a note to all final scenarios, stating which one is the last scenario
   * A Tale of Two Brothers:
     * Fix bug #19949: Turns left counter initially incorrect when objectives
       change and turn limit is extended
     * The Chase: fixed some missing messages if no unit with role=speaker is
       present
   * Dead Water:
     * Updated the Stun weapon special code to use [object] duration=turn
   * Delfadors Memoirs:
     * Showdown in the Northern Swamp: added to conditional notes in objectives
       about who should kill Iliah-Malal
   * Dead Water:
     * Changed uses of "ai turn" to "side # turn" ("[event]name=ai turn" breaks
       replays)
   * Descent into Darkness:
     * Fix most possible cases for Darken Volk becoming "stuck"
       in 09_A_Small_Favor3.
   * Eastern Invasion:
     * Revisions to dialogue and part of the story.
   * Heir to the Throne:
     * Implemented FR #19418: the Orcs destroy the villages near Konrad's castle
       when capturing them in The Elves Besieged
     * Fixed bug #19531: scroll to reinforcement units in Blackwater Port
     * Fixed bug #19517: The injured sergeant in HttT: Cliffs of Thoria will now
       always become visible when he is discovered.
   * Legend of Wesmere:
     * Fix bug #19577 - some terrain was not snow covered
     * Fix bug #19565 - yetis remaining under fog when they appear in scenario
       10
   * Liberty:
     * Fixed unknown unit type errors in 04_Unlawful_Orders
   * Sceptre of Fire:
     * Searching for the Runecrafter: updated the "sighted location" code to
       make use of [filter_vision], plus a SSF
     * Keep recall list at end of scenario 7
   * Son of the Black Eye:
     * Removed Al'Brock and Flar'Tar death as defeat condition in objectives in
       Civil War
     * Fixed bug #19684: Kapou'e says a different message if he kills the first
       Elf in Silent Forest
     * Silent Forest: the messages that appear upon killing the first Elf can be
       translated differently depending on the Elf's gender
     * Fixed bug #19686: fixed two variable checks that prevented some strings
       from being displayed in Back Home
   * The Hammer of Thursagan:
     * Fixed bug #19743: in Mages and Drakes, Perrin has two different messages
       about apprentices, depending on the difficult level
     * Karrag now gets enough gold to actually recruit when met
   * The South Guard:
     * Fixed bug #19758: Urza Afalas' sighted event is fired also when the Lich
       is seen, and if the player allies with the Elves, then both Urza Afalas
       and Mal M'Brin must be defeated
   * Tutorial:
     * Applied patch #3203: Allow to end the scenario via the right-click menu.
     * Fixed bug #19316: Narrator messages are now translated.
     * Replaced $unit.type by $unit.language_name to make a string completely
       translatable
     * Fixed bug #19367: Incomplete i18n in wesnoth-tutorial
   * Under the Burning Suns:
     * Made it so that Kaleh gets the default AMLA after he's been fully
       upgraded
     * Fixed flood appearance over wooden boards
     * Update scenario code to take advantage of modern WML features
     * Fix bug #19630
     * Fix some minor unreported bugs
     * Minor spelling and grammar corrections
     * Made it so that all units are rehydrated quietly upon victory in scenario
       two
     * Updated the Stun weapon special code to use [object] duration=turn
     * The Human Commander unit now uses the smallfoot movetype instead of
       woodland
     * Display dehydration status in the sidebar
     * Ensure player always has positive gold leaving the cave in Out of the
       Frying Pan
     * New AI for human messenger in "Out of the Frying Pan"
     * Taking all items is now optional
     * Finding the map in scenario 3 is now useful
     * Healers now prevent dehydration instead of removing it, so they no longer
       heal dehydrated units
     * New art for Dark Assassin
 ### Editor
   * New gui theme, fixes the editor being broken on low resolutions.
     * Smaller font for displaying the terrain information.
     * Less space between the map coordinates and the terrain information
       (no need to display defense value)
     * More width sidebar
       * Features one more editing tool per row.
       * Additional brush
       * Button for default zoom switching.
     * Space for a second row of editing tools.
   * Terrain group selection from a menu, saving a lot of space.
   * Terrain palette
     * is scaled to fit at all resolutions.
     * Removed the scrolling feature which is no longer needed.
   * Improved performance of the Editor Settings ToD lighting controls
 ### Engine
   * Refactored scenario transition code
   * Fixed bug #19599: Engine keeps redundant unit.ai_special_ information.
 ### Graphics
   * New graphics for the Spectre.
 ### Help menu
   * Added a new Add-ons section explaining add-on usage basics
   * Added more elaborate descriptions for the Units, Abilities and Weapon
     Specials sections, with the intent of reducing confusion when they are
     initially empty (feature request #13914)
 ### Language and i18n
   * Fixed two untranslatable strings in the "new" MP lobby
   * Fixed an untranslatable string in the Multiplayer Commands help topic
   * Fixed untranslatable Cancel button label in the add-ons server network
     status dialog (bug #19659)
   * New translations: Ukrainian
   * Updated translations: British English, Chinese (Simplified),
     Chinese (Traditional), Dutch, Estonian, Finnish, French, Galician,
     German, Greek, Hungarian, Indonesian, Irish, Italian, Japanese, Korean,
     Latin, Lithuanian, Norwegian, Old English, Polish, Portuguese (Brazil),
     Russian, Slovak, Spanish
 ### Lua API
   * Upgraded Lua from 5.1.4 to 5.2.0
   * new: field wesnoth.game_config.mp_debug
   * new: setter for wesnoth.sides[i].color
   * Deprecated the following functions from the wesnoth table,
     all of which have better replacements:
     get_side, get_side_count, get_unit_type_ids, get_unit_type,
     register_wml_action
   * Changed: Extended support for toggle_button.
   * Fixed: wesnoth.find_reach does no longer replace a passed private lua
     proxy unit with the on-map unit at the same location
   * new: helper.round function
   * wesnoth.scroll_to_tile() now takes a fourth optional argument (boolean)
     specifying whether to ignore the scroll speed setting in Preferences and
     instantly warp to the given location
   * Added wesnoth.lock_view(), taking a boolean argument specifying whether
     to lock gamemap view scrolling (so the user cannot scroll, while WML/Lua
     actions still can, i.e. for cutscenes), or unlock it
   * Added wesnoth.view_locked(), returning a boolean true value if gamemap
     view scrolling has been locked, and false otherwise
   * Added == operator for (private, on-map, recall) lua proxy units,
     checking for unit identity
   * Added wesnoth.get_villages() function
 ### Multiplayer
   * A New Land:
     * Made it so that the "Elvish Shaman" option in the elvish unit selection
       menu gets translated. Fixes bug #19677.
   * Fix an accidental terrain type change in Isar's Cross
   * Fix attacker side being human in 6p_Team_Survival (bug #19400)
   * Ignore Pango markup in map descriptions (bug #19210)
   * Canceling the sides setup screen when hosting a MP game now brings the
     host back to the game configuration screen first instead of returning
     immediately to the lobby or (for hotseat) titlescreen (bug #7130)
   * The Set Password action is no longer shown for local hotseat games
     (bug #10784)
   * Changes to the time of day schedules of Fallenstar Lake and Silverhead
     Crossing
   * Random leader is default selection when picking faction
 ### Music and sound effects
   * Replaced some of the wolf hit sounds with lower-pitched ones
 ### Terrain
   * New Syntax for terrain maps
   * New tropical forest images
   * New palm forest terrain
   * Deprecated flowers (Ggf) and Volcano (Qv) removed - use Gg^Efm and Mv
     respectively
   * Fix bridge/rail transitions to each other
   * Add new rail terrain type; no existing unit can move on it, so their
     move/defense values are unchanged
   * New stone, hanging, and wooden chasm bridges
   * Fix bug #19753, missing hex transition
 ### Unit changes and balancing
   * New extra_define ENABLE_WOLF_ADVANCEMENT to optionally enable Wolves
     advancement to Great Wolves and Direwolves
   * New extra_define ENABLE_TROLL_SHAMAN to optionally enable Troll Whelps
     advancement to Troll Shamans (not in multiplayer)
   * Lowered General's experience to 150 when extra_define DISABLE_GRAND_MARSHAL
     is used
 ### User interface
   * Restored leader unit image in Load Game dialog (bug #18683)
   * Added a "Back to Start" option to the game menu to load the current
     scenario's start save if it is available (feature/bug #18027)
   * Added tooltips to Load Game dialog (feature/bug #18249)
   * Added a Reset All button to Hotkey Settings dialog in preferences
     (feature/bug #3797)
   * Fixed excessive idle CPU usage in story screens without story text
   * Map editor now displays invisible overlay terrains on main map
   * Made add-ons with markup characters at the start of their titles
     display normally in the add-ons management dialogs (e.g. without
     colors)
   * Re-fogging does not occur in the middle of the player's turn.
   * Fixed provided saved game filenames being ignored when requesting to
     save a MP game due to a network or OOS error (bug #19562)
   * Fixed bug #19538: Filters matching 0 saved games cause crash
   * Clicking a unit does not reveal adjacent hidden units (bug #19381).
   * Simultaneous ambush, sighted and teleport failed messages will no longer
     cover each other up
   * Added: A character limit width constrain for a text
   * Stop showing a unit's potential moves before moving in a move & attack,
     rather than after (more consistent with regular movement)
   * Changed: Made the id for a tooltip and helptip mandatory.
   * Added: Helper code to assist in widget placement.
   * Added: New widget pane.
   * Added: New widget viewport.
   * Added: New dummy widget instance.
   * Added: New control matrix.
   * Trying to initiate movement (or an attack) before previous movement is
     finished no longer unselects the current unit (bug #19734). (The new
     movement command is still deliberately ignored.)
   * The recruit and recall commands no longer appear in the context menu
     for shrouded or (visibly) occupied hexes.
   * Fixed bug #19844: Block recruiting into shrouded hexes.
   * Fixed bug #19783: Disable commands while WML menu items are executing.
   * Fixed bug #19533: Both damage tooltips now take into account local factors.
 ### Whiteboard
   * Fixed bug #19626: segfaults on window resize
   * Fixed bug #19369: Using planning mode can cause losing ability to move my
     units
   * Fixed bug #19408: Crash shortly after executing invalid multi-turn move
   * Fixed bug #19581: Leader can still move after a planned recall
   * Shortened all context menu items
   * Fixed bug #19619: In local games, units keep ghosted appearance during
     opponent's turn
   * Fixed bug #19613: Multiple attacks after planning attacks in whiteboard
   * Fixed bug #19618: Problems with ending turn with impossible moves
   * Fixed bug #19615: Can see part of opponents' planned moves in local
     multiplayer game
   * Refactored wb::side_actions
   * Refactored the highlighter and the visitor
 ### WML engine
   * new key: [unit]/[unit_type] vision=<number>, decouples movement and sight
     range.
   * new tag: [movement_type] [vision_costs], used for calculating sight range
     if present.
   * new action tag: [clear_menu_item] id=...
   * new key: [set_menu_item][command]delayed_variable_substitution=yes|no
   * Removed support for the deprecated "colour=", "debug_border_colour=",
     and [colour_adjust]
   * Fixed bug #18996: Increase random number generation range.
   * Made it possible to disable the credits screen at the end of a campaign
     by specifying end_credits=no in the final [endlevel] action, or in the
     [campaign] definition (defaults to yes)
   * Added [modify_side] color= attribute, which changes a side's team color
     range (feature/bug #18772)
   * Removed support for the deprecated [removeitem]
   * Implemented ~BLEND(r,g,b,o) image path function, which blends the image
     with a specified RGB color according to certain opacity (factor or
     percentage) (feature/bug #11590)
   * [find_path] now returns a "hexes" key instead of a "length" key inside
     the output array
   * Introduce inline SSF support in action tags [allow_recruit],
     [disallow_recruit], [set_recruit], [gold] and [modify_ai]
   * [gold][filter_side] and [modify_ai][filter_side] are deprecated, use inline
     SSF instead
   * Action tags [modify_side], [place_shroud] and [remove_shroud] now default
     to "all sides" instead of side=1 if left empty.
   * Introduce support for [filter_owner]<SSF> in SLFs. For villages. Keeps
     backwards compatibility for inline owner_side= at the cost of
     confusing syntax (due to (possible) duplicate side information).
     This also adds SSF support in [store_villages].
   * Added support for resistance_multiplier= key in [harm_unit]
   * Fixed bug #19498: [modify_unit] duplicating units if x,y changed
   * Added inline SLF support in [scroll_to], by using the first matching
     location
   * Introduce inline SSF support in [objectives] and [show_objectives]
   * The [drain] weapon special now supports value=, multiply=, divide=, add=
     and sub=
   * Added [heal_on_hit] for healing/harming the user by a fixed amount
   * Drained HP amounts can now be negative.  Trigger this by setting
     a negative value in the [drain] or [heal_on_hit] weapon special
   * Negative drain amounts will not take a unit below 1 health
   * Added [show_if] support to [objectives] [note]
   * New tags: [lift_fog] and [reset_fog]
   * New keys: reset_maps= and reset_view= added to [modify_side].
   * Using more than 4 multiply effects no longer wraps to negative integers
   * Added: support for ranges of sides in SSF
   * The [filter_vision] tag of the SUF now uses a SSF
     (viewing_side= still works, but is deprecated and
     should be renamed to side=)
     Semantics for empty side information changes from
     "all enemy sides" to "all sides"
   * [filter_vision] now works in location filters as well as unit filters,
     and has an additional key (respect_fog) for locations
   * Support for [effect]unit_type= and [effect]unit_gender= has been
     removed, use [effect][filter]type= respectively [effect][filter]gender=
     instead
   * Border hexes are included when using radius= in the SLF of [remove_shroud]
     and [place_shroud] (they were already included for directly matched hexes).
   * Retrofitted CLEAR_FOG and UNCLEAR_FOG macros to make use of [lift_fog] and
     [reset_fog]
   * Replaced variable i by TRANSFORM_UNIT_i in macro TRANSFORM_UNIT
   * Fix $owner_side in capture events fired due to unit movement
   * Introduce [item]redraw=yes|no (def yes) parameter
   * Added [object]duration=turn
   * Pushed LOOT macro from LoW, THoT and TRoW in core
   * Adds new WML attribute for configurable village support (upkeep)
   * Change [object]duration=level to [object]duration=scenario
   * The race attribute in SUFs can now take a comma-separated list
   * New image path function: ~ROTATE()
   * Fixed some cases where [find_path] did not restore $this_unit
   * Added: Allow better error messages for missing WML children.
   * [scroll_to] and [scroll_to_unit] now accept an optional boolean immediate=
     attribute (defaults to false) specifying whether to ignore the scroll
     speed setting in Preferences and instantly warp to the selected location
   * Added [lock_view] and [unlock_view] WML actions, for locking and unlocking
     gamemap view scrolling (so the user cannot scroll, while WML/Lua actions
     still can, i.e. for cutscenes)
   * Fixed CALL_FUNCTION macro (bug #19805)
   * Added [effect] apply_to=halo
   * Converted [store_villages] from C++ to Lua
   * Introduced [unstore_unit]animate=yes|no (def. yes) parameter
   * Made the display of ability and weapon special descriptions smarter, so
     those descriptions no longer require (and should no longer have) the name
     of the ability/special as the first line of the description.
 ### Miscellaneous and bug fixes
   * Fix wrong preferences path suffix (1.1 instead of 1.10) on Linux and other
     platforms using XDG layout (no compiled-in preferences path override,
     bug #19318)
   * Fixed unit sound animation timings wherever {SOUND:SLOW}, {SOUND:POISON},
     and several macros from animation-utils2.cfg are used (see bug #19274)
   * Restart is no longer required to toggle desktop notifications
   * Display the savegame version when warning the user about unsupported or
     mismatched versions (bug #7243)
   * Implemented feature request for difficulty changing during campaigns.
     (see bug #10978)
   * The saved games cache file is now save_index instead of save_index.gz, and
     it is compressed when the Compressed Saves option is enabled in Advanced
     Preferences (patch #3115)
   * Show base terrain description if none for overlay (bug #19411)
   * Added wesmage tool to test SDL image manipulation functions.
   * Fixed: A possible NULL-pointer deference in get_unit_type_function.
   * Changed: Default man installation directory now confirms to FHS.
   * Added: New floating point emulation code.
   * Make Wolf Rider and Wolf raise their heads if and only if unit is
     submerged: forums.wesnoth.org/viewtopic.php?f=4&t=36212
   * Changed: Add a small cache for reading files, giving a minor improvement.
   * Added: Helper tool to create images for unit tests.
   * Added: Image manipulation unit tests.
   * Changed: Rewrote the surface blending algorithm, and improving its speed
     using NEON intrinsics on processors supporting NEON (the Pandora).
   * Fixed bug #19503: "maximum auto saves" setting now works correctly.
   * Fixed: A preprocessor bug looking beyond the end of a buffer.
   * Fixed: Binding a temporary in the network code, possibly causing
     crashing.
   * Fixed bug #19658: In replays, units sometimes refresh movement when it is
     not their turn
   * Added: Enabled BREAKPOINT and WES_HALT macros on the Pandora.
   * Fixed bug #19678: Escape the pipe-symbol in the wiki: TerrainCodeTableWML.
   * Fixed bug #19469: Missing scenario hash results in Remote scenario always
     added to game description
   * Fixed bug #19322: Empty sides not being listed at the status table
   * Fixed bug #19681: Use the created cache instead of rebuilding it all the
     time
   * Fixed bug #16544: Fail to read `empty' cache files.
   * Fix bug #19681: Cache is never valid
   * Added shroud_data to the inspection window (FR #19623).
   * Fixed: grids now recursively search for widgets by pointer.
   * Fixed: Wrong current side number after side turns (bug #19735)
     It also affected the lua field wesnoth.current.side
   * Added: Fail macros FAIL and FAIL_WITH_DEV_MESSAGE.
   * Added: Helper code to make it easier to write enumerate stream operators.
   * Added: UNREACHABLE_CODE macro.
   * Added: TELEPORT animation macros usable by any units
   * Added: CMake option ENABLE_SHARED_LIBRARIES.
   * Fixed: Allow strict compilation with CMake using the upcoming gcc-4.8.
   * Fixed bug #19795: OOS when [message][option] is used in a (single-player)
     start event when side 1's controller is null.
   * Changed: Enabled strict compilation for Clang in CMake.
   * Added: ENABLE_PEDANTIC_COMPILATION option for CMake.

## Version 1.10.0
 ### Campaigns
   * Legend of Wesmere:
     * Scenario 03: Fixed bug #19067: Control over Galtrid's side stays with the
       player after the arrival of Kalenz
     * Scenario 22: Fixed bug #19236: Cleodil was missing and no recall list
   * An Orcish Incursion:
     * Scenario 3: select a new unit with the Adviser (sic) role for dialogs if
       the original unit selected at the start of the scenario died
   * Under the Burning Suns:
     * Scenario 5: Fixed bug #19303: one of the dwarves was spawning in a wall.
   * The Rise of Wesnoth:
     * Peoples in Decline: fixed a message not shown when the Sea Serpent appears
 ### Editor
   * Added Etheral Abyss terrain (Qxua) to the Cave category, too
   * Updated the terrain palette icons for winter/fall single or mixed forested
     hills so they show the default base Hhd instead of Hh
 ### Graphics
   * Fixed some spearman attack animations being too slow in some directions
 ### Language and i18n
   * Updated translations: Chinese (Simplified), Chinese (Traditional), Czech,
     Finnish, French, Galician, German, Hungarian, Italian, Latin, Lithuanian,
     Old English, Polish, Serbian, Slovak, Spanish
 ### Lua API
   * Extend and improve wesnoth.select_hex, deprecate wesnoth.highlight_hex
 ### Multiplayer
   * Allow tab completion of player names in commands (bug #19284)
   * Fixed bug #19260: 'villages_value' -> 'village_value' in 5p_The_Wilderlands
   * Fixed eras other than the default breaking 2p_Hornshark_Island. If a player
     has a faction not in the default era, a standard set of units (aimed at
     Khalifate) is used now.
   * Fixed bug #19083: allow attack to happen normally without causing OOS if
     turn time runs out in attack selection dialog.
   * Fix some unit images being cropped in 2p_Aethermaw
 ### Terrain
   * Removed tags ignored by engine (as reported by lipk in
     http://forums.wesnoth.org/viewtopic.php?f=21&t=35832 )
   * Fixed bug #18601: change swamp water so it looks as good as it did in 1.8
 ### User interface
   * Draw gold hex cursor above all terrain when no unit visible
 ### WML engine
   * Improve [select_unit] to match its original intention (bug #19224)
   * Improve error handling in case of invalid maps passed to
     [replace_map] or [terrain_mask]
   * Disable wml menu items in linger mode without debug mode (bug #16262)
 ### Whiteboard
   * Fixed bug #18635: Percentage not displayed for units selected at their
     future position
   * Fixed bug #19142: attacks can be simulated between units (for which this
     shouldn't be possible)
   * Fixed bug #19221: Assert when a whiteboard move-attack wins a scenario
   * Fixed bug #19222: After 'delete planned action', the unit is almost
     invisible
   * Fixed turn not finishing when moves were planned for future turns
 ### Miscellaneous and bug fixes
   * Fixed wmlindent writing CRLF end of lines on windows
   * Fix tutorial units without xp bar
   * Reset game credits instead of appending on WML cache refresh (bug #19292)
   * Fix wmllint check for "unknown xy referred to by id" not working

## Version 1.9.14
 ### AI
   * Fixed bug #18962 and bug #19214: AI leaders are no longer slow to select
     attacks.
 ### Campaigns
   * Sceptre of Fire:
     * New set of portraits
 ### Editor
   * Fixed overpainting of transparent tile icons on the editor palettes on the
     sidebar
   * Fixed terrain palette shrinking to the size of small categories when
     switching maps (bug #19218)
   * Fixed various crashes related to drawing/filling operations (bug #18928)
 ### Language and i18n
   * Changed font used for CJK languages to DroidSans
   * Updated translations: British English, Chinese (Simplified), Czech, Dutch,
     Estonian, French, Galician, German, Hungarian, Latin, Norwegian, Serbian,
     Slovak, Spanish
 ### WML engine
   * Fix store_unit clearing the used variable before its filter can reference it
     (bug #19203)
   * Check for argument image file existence before creating a MASK or
     BLIT image mod (bug #19208)
   * New key [object]delayed_variable_substitution=yes|no (default no)
     as a workaround for bug #18893 (fixes bug #19225)
   * Made empty {} preprocessor directives be handled more gracefully
     (bug #19233)
   * Fixed bug #19213: [harm_unit] incorrectly uses ToD bonus
   * The turn bell and autosaves are not triggered anymore in certain situations
     after [endlevel] has been issued
 ### Miscellaneous and bug fixes
   * Fixed bug #19032: Poison and level up AI defense placement calculation
   * Fixed bug #19245: wesnoth(6) man page doesn't describe the --campaign*
     options correctly
   * Enable local ToD lighting option by default.

## Version 1.9.13
 ### Campaigns
   * Northern Rebirth:
     * Infested Caves: integrated sighted events with moveto events
     * To the Mines: fixed Hamel not having the hero overlay
     * Clearing the Mines: fixed Dwarves recruitment not being disallowed upon
       winning
     * The Pursuit: integrated a sighted event with moveto event
     * The Pursuit: replaced another sighted event with ON_SIGHTING
     * Elvish Princess: fixed Ro'Arthian and Ro'Sothian not having the hero
       overlay
     * Introductions: used ON_SIGHTING instead of sighted event
     * Stolen Gold: gave Krash the expendable leader overlay
     * Stolen Gold: fixed one of the two loyal Drakes having two IDs
     * Stolen Gold: fixed Hidel missing his portrait
     * Get the Gold: gave Eryssa the expendable leader overlay
     * The Eastern Flank: made the Gryphon fly away and then return
     * Showdown: removed hero overlay from Ro'Arthian and Ro'Sothian when they
       are leaders
     * Showdown: fixed Hamel's death not causing defeat
     * Epilogue: fixed music not being played
   * Eastern Invasion:
     * The Escape Tunnel: integrated sighted event with moveto event
     * Captured: integrated a sighted event with moveto event
     * Weldyn Besieged: fixed a typo in Konrad's last breath event
   * Sceptre of Fire:
     * Caverns of Flame: integrated a sighted event with moveto event
   * Son of the Black Eye:
     * The Desert of Death: integrated a sighted event with moveto event
   * Legend of Wesmere:
     * Ka'lian Under Attack: fixed a lua error in AI code
 ### Editor
   * Fixed missing brightening of selected hexes under the brush
 ### Language and i18n
   * Updated translations: British English, Czech, Dutch, French, Galician,
     German, Hungarian, Korean, Latin, Lithuanian, Old English, Polish,
     Russian, Serbian
 ### Multiplayer
   * Fix user interface set to side 1 when entering linger mode
     (bug #15847)
 ### Unit changes and balancing
   * Changed the Lancer's and the Deathblade's AMLA requirements to conform to
     the current AMLA standards
 ### User interface
   * Removed blur from the add-ons description dialog
   * Fixed bug #19121: Make click dismiss work properly.
   * Fixed bug #17961 and #18686: Shows of big portraits on smaller screens.
   * Fixed bug #19118: Default theme: coordinates overlap with the clock status
     panel on small horizontal resolutions. Also fixed for the widescreeen
     theme.
   * Made it so that "AM"/"PM" show up in the default and widescreen themes
     on the lowest supported resolutions.
 ### Whiteboard
   * Display turn numbers on multi-turn planned moves
   * Fix attacks not finishing but still being transmitted over the network when
     executing all actions
 ### WML engine
   * Fixed: ~L() not handling lightmap having different width. Now rescaled.
 ### Miscellaneous and bug fixes
   * Fixed: compilation with clang 3.0 in C++11 mode.
   * Changed: Sort order of campaigns uses a stable sort now.
   * Fixed bug #18832: Fixed ~BLIT() to access images out of bounds.
   * Fixed: ToD local light for RGB values bigger than 128.
   * Fixed: hex-cut of images in :layers debug tool. Also removed empty images
   * Forward ported a new version of multiplayer chat log history dialog
   * Fixed bug #19188: Turn dialog always speaks of Konrad in tutorial
   * Added: NativeClient port.
   * Fixed: crash when using fullscreen on Mac OS using SDL nightly builds.
   * Optimized screen update (zooming, ToD change, etc)
   * Optimized framerate by removing a lot of empty terrain images
   * Optimized perfomance cost of complex local ToD areas
   * Fixed bug #16571: Multiplayer login error with certain username or
     password formats

## Version 1.9.12
 ### Language and i18n
   * Updated translations: British English, Czech, French, German, Hungarian,
     Indonesian, Italian, Latin, Latvian, Old English, Russian, Slovak
 ### Multiplayer
   * Fixed missed side init if controller changes at side progression
     (bug #16299)
   * Fixed user interface not updated if controller changes (bug #19056)
 ### Whiteboard
   * Fix invalid actions not being detected until another action is created
   * Erase invalid actions when you try to execute them
   * Disable access to Suppose Dead action until we can find a better UI for it
   * Fix bug #18774: Recalling with the whiteboard enabled causes crash
   * Fix bug #19061: Crash on starting most campaign scenarios
   * Fix getting "not enough gold" message when executing planned recruits/recalls
   * Ghost the unit at starting position of planned moves
   * On mouseover, display orb and xp bar on planned moves with accurate status
   * Mouseover on last planned move of a unit displays accurate movement left
   * Re-enabled multi-turn moves
 ### Miscellaneous and bug fixes
   * Fixed bug #19095: fixed a gcc warning when compiling under OpenBSD
   * Fixed bug #19096: fixed linker errors on OpenBSD when using the CMake
     build system

## Version 1.9.11
 ### Campaigns
   * Sceptre of Fire:
     * A Bargain is Struck: added missing hero overlays to Alanin and the
       caravans
     * A Bargain is Struck: added find_vacant=yes to avoid unstoring Alanin
       upon Rugnur or another player's unit
     * The Dragon: fixed Rugnur being recalled not fully healed
     * The Dragon: integrated two sighted events with moveto events
 ### Engine
   * Fixed bug #18918: the create unit dialog was sometimes capturing village
     incorrectly
 ### Language and i18n
   * Updated translations: British English, Dutch, French, Finnish, Galician,
     Indonesian, Italian, Korean, Latvian, Lithuanian, Old English
 ### WML engine
   * Added [secondary_unit] SUF for filtering the recalling leader in [recall]
     action WML
   * Fixed red, green and blue keys in 24-hour ToD not maching the default
     schedule colors
   * Reworked [harm_unit]'s damage calculation code to avoid leadership ability
     altering the damage value
 ### Miscellaneous and bug fixes
   * Fixed disappearing theme UI buttons after changing fullscreen/windowed mode
     or resolution in Preferences during a game
   * Fixed define handling of wmlxgettext (bug #18622)
   * Changed: Try to recover from broken pango-markup.

## Version 1.9.10
 ### AI
   * Fixed: Display of recruitment_ignore_bad_combat and
     recruitment_ignore_bad_movement is no longer swapped (bug #18839)
 ### Campaigns
   * Descent into Darkness:
     * A Small Favor, part 1: used 24 hour ToD
   * Son of the Black Eye:
     * To the Harbor of Tirigaz: used 24 hour ToD
   * Under the Burning Suns
     * Subterranean Struggle: no longer possible to win by allowing allies to
       die
     * Across the Harsh Sands: the Black Hand oasis is no longer possible to
       enter without triggering the bandits
 ### Editor
   * Made sure lighting setting changes are applied immediately when closing the
     settings window when automatic map view updates are disabled
   * Starting position tool menu now displays coordinates of existing positions,
     and initially highlights the current player number assigned to the selected
     hex
   * Add a menu item "Refresh WML" to reload terrain WML
 ### Graphics
   * New Animations:
     * Goblin spearman run se
     * Merman fighter attack se
     * Tinted the slowed snail icon to match the new slowed unit color
   * Terrain:
     * Tall encampment keep can now be placed next to the tent encampment keep
     * Forest/hill terrain combinations no longer appear black on the minimap
   * Default team color always applied last; fixes bug #18817
   * Removal of Khalifate unit images
 ### Language and i18n
   * Made it so that all of the Hangul Syllables block is covered by the
      font-loading code.
   * Made it so that en_US translations are loaded if they exist (bug #18507)
   * Updated translations: British English, Czech, Dutch, Finnish, Galician,
     Indonesian, Irish, Italian, Japanese, Korean, Latin, Lithuanian,
     Old English, Portuguese (Brazil), Russian, Slovak, Swedish
 ### Lua API
   * add function wesnoth.get_starting_location
   * The side metatable provides also the side number
     (like wesnoth.sides[i].side, which would be i)
   * add function wesnoth.debug
   * Added: Support for clickables (button and repeating button) to
     wesnoth.set_dialog_callback (patch #2763)
   * Allowed function debug.traceback()
   * wesnoth.set_village_owner takes a bool argument determining
     whether to fire capture events
   * add field image_mods in proxy units
   * add field undead_variation in wesnoth.races
 ### Multiplayer
   * New map: Aethermaw
   * Updated maps: Hamlets, Howling Ghost Badlands, Thousand Stings Garrison
 ### Terrain
   * Oasis may be placed on any terrain, aliased to base
   * New terrain macro: MOUNTAIN_SINGLE_RANDOM
   * New rotting variant for wooden bridge
 ### Unit changes and balancing
   * All mounted units now have forest defines capped at 30%. This reduces their
     defense on forested hills from 40% to 30%
   * Forests now give best defense and worst movement on all terrain,
     not just hills - fixes bug #18216
   * Fixed some Khalifate unit animation glitches
   * The XP required for an AMLA is now 50*level (and 25 for level 0) instead of
     always 150.
   * Increased the HP of the Great Mage from 55 to 60
   * Increased the ranged attack of the Footpad from 4-2 to 5-2
   * Removal of the Khalifate units, faction and era
 ### User interface
   * Removed waypoints UI feature
   * Added an option to disable the "loading save from a different version"
     confirmation dialog
   * Line-wrap author names in the add-on description dialog (bug #18691)
   * Made the Delete Add-on (in the server) option request confirmation from
     the user before proceeding
   * The add-ons download list shows up again after publishing/deleting a
     single add-on
   * Added option in advanced preferences that allows the twelve-hour clock
      format to be used
   * Reenabled "delay shroud updates"
   * Changed: Disable pango markup in unit names (bug #17788)
   * It is now possible to remove multiple installed add-ons at the same
     time
   * Slowed units are now tinted to be recognizable at a glance
   * Fixed: Removed old markup style from OOS messages (bug #18387).
   * Fixed: OOS ignore toggle in the save dialog (bug #18330).
 ### WML engine
   * Readded the liminal alignment
   * Added four-difficulty versions of certain macros: QUANTITY4,
     ON_DIFFICULTY4, TURNS4, GOLD4, INCOME4, and ATTACK_DEPTH4
   * added support for SSF to SUF via a [filter_side] subtag
   * added support for [enemy_of]<SSF> to SSF
   * added support for [allied_with]<SSF> to SSF
   * New [find_path] tag, a WML interface to the pathfinder
   * Add inline SSF support to [store_starting_location]
   * Add support for [capture_village][filter_side]<SSF>
   * Added FACING, which adds facing= to a previous [unit] codeblock
   * Added VARIATION, which adds variation= to a previous [unit] codeblock
   * Deprecated the Add-on.cfg style in favor of Add-on/_main.cfg, except for
     the case of a single-file add-on
   * [illuminated_time], which has been obsolete for a long time, is no longer
     valid. Using it will cause errors to be thrown
   * Reintroduced support for [unit][event]s (was until 1.7.10, bug #16259)
     Such events are no longer forcefully variable substituted before inserting
     into the scenario wml
   * Fixed "error parsing image modifications" message caused by subsequent
     image mod additions using the add attribute in [effect] apply_to=image_mod
   * Made $this_unit in filter_recall work
   * Added IS_EXPENDABLE_LEADER, which gives a unit the expendable leader icon
   * Added $this_unit support to [harm_unit] and [find_path]
   * Settings by [color_adjust] no longer expire at turn start or over save/load
     cycles
   * Implemented bullet= for [objectives], [objective], [gold_carryover], and
     [note]
   * Implemented red=, green=, and blue= for [objective], [gold_carryover], and
     [note]
   * Added [theme] hidden= attribute that makes a theme unavailable for selection
     in Preferences or :theme (defaults to no)
   * Implemented $second_unit being bound to the acting leader in recall/recruit events
   * Introduce [redraw]clear_shroud=yes|no and [redraw]<SSF> support
   * Introduce [race]undead_variation as a default for the race's [unit_type]s
   * $owner_side describes the previous side owning a village
     in capture events (FR bug #13567)
   * Introduce [capture_village]fire_event=yes|no (def no) whether to fire
     any capture events (was previously always yes)
   * Made [move_unit] respect image modifications applied by EffectWML to single units
   * Introduce [unit_type]image_icon key to override image for 72x72 icons
     (FR bug #15466)
   * Added IS_LAST_SCENARIO macro, for use in [objectives] dialog.
   * Fix [objectives]silent= not working initially in a scenario (bug #18927)
 ### Miscellaneous and bug fixes
   * Add --language/-L commandline option to set the language for that session
   * Fixed: Avoid copying of singular iterators in the whiteboard code
   * Fixed bug #10969: Made it possible to switch themes from Preferences in the
     main menu
   * Fixed bug #16111: gold carryover if loading a save created in linger mode
   * Fixed bug #16508: remaining time of day areas that should affect map
     borders in mainline campaigns and MP scenarios
   * Fixed bug #18399 (part 1): Compilation with boost 1.47 (bug #18399's patch)
   * Fixed bug #18399 (part 5): Compilation with the clang 2.9 compiler (bug
     #18399's patch)
   * Fixed bug #18695: Fixed preload event not being fired
   * Fixed bug #18701: Evaluate key length even if intervening WML children
     don't exist
   * Fixed bug #18704: Make the create unit dialog give the created unit a valid
     gender for that unit type
   * Fixed bug #18766: Fixed a problem where version comparisons (including, but
     not limited to #ifver/#ifnver directives) could cease to work until Wesnoth
     was restarted
   * Fixed debian bug #636193: Fixed compilation on all Debian architectures
   * Fixed handling of #ifver and #ifnver preprocessor directives in wmllint
   * Removed CANYON and its associated terrain macros
   * The credits screen no longer mangles image path functions in background
     image lists
   * Fixed a rare glitch causing the menubar and sidebar buttons to appear
     initially as solid color rectangles at the beginning of a scenario start
     event
   * Fixed bug #18681: glitch with local time of day lighting
   * Fixed bug #18892: random crash when loading replays
   * Fixed bug #18882: Compilation with libpng-1.5.5

## Version 1.9.9
 ### AI
   * Fixed bug #16117: added a way to supress E_NOT_REACHED_DESTINATION to lua
   * Fixed bug #16247: modify SoF 8 to let the dragon attack aggressively
     api via an optional boolean parameter - ai.move_full(from,to_x,to_y,true)
   * Fixed bug #18057: AI leaders should now attack when they should
   * Fixed bug #18122: AI leaders set to defensive will now follow goals anyway
   * Fixed bug #18356: AI leaders now won't move to avoided keeps
   * Applied patch #2846 by thonsew: let AI forget about invisible enemy units
     in villages during get_villages phase (bug #18101)
 ### Campaigns
   * Fixed remaining deprecation warnings about empty side=
   * Under the Burning Suns:
     * Fixed Naga Sentinel gaining an AMLA after 32 XP instead of the usual 150
     * Gave to Desert Shydes and Desert Stars 30% defense on void terrain
 ### Engine
   * Fixed bug #16173: Using n or cl while a fake unit is moving causes the game
     to segfault by creating game_display::fake_unit as an exception safe
     interace to the fake_units
   * Improved unit_map lookup from std::map to boost::unordered_map to decrease
     lookup times
   * Fixed bug #16151: Discover new units on recruit
   * Fixed bug #17780: Allow objects to 'increase' damage to 0
   * Fixed bug #18098: now recruits and recalls capture village if recruited or
     recalled on village castle hex
 ### Graphics
   * Fixed bug #18524: [replace_map] doesn't force window repainting
   * Fixed bugs #18504 #18493 and #18017: Time of Day and fading interact poorly
   * Fixed bugs #18475 and #17292: Mage of Light and Sorceress Halo/animation
     glitches.
   * Terrains:
     * Added Gray Coastal Reef, Tropical Coastal Reef terrain
     * Swamp no longer prevents embellishments from being drawn
     * Fixed bug #15940: graphics glitch: pillared wall terrain covers great
       tree
 ### Language and i18n
   * Updated translations: British English, Estonian, French, Galician, Greek,
     Hewbrew, Indonesian, Irish, Latin, Macedonian, Old English, Russian,
     Vietnamese
 ### Lua API
   * Added: function wesnoth.set_dialog_active (patch #2767)
   * Expanded wesnoth.races entries to return the wml object a race was
     constructed from via the __cfg field.
   * New function wesnoth.get_traits returning a table holding the global traits
     known to the engine.
 ### Multiplayer
   * Updated maps: Aethermaw, Hornshark Island, Sablestone Delta, Thousand
     Stings Garrison
   * A New Land:
     * Fixed 'agriculture' not getting translated sometimes and research
       for agriculture not getting counted if the research order wasn't changed
       (bug #16477)
     * Options to share knowledge with those who have learned everything about
       whatever are no longer shown (bug #14822).
   * Added "chat_message_aging" advanced preference to allow setting the
     ingame chat message aging interval
 ### Music and sound effects
   * New track "Battle Epic" by Doug Kaufman
 ### Unit changes and balancing
   * New units: Great Wolf; Direwolf
   * Lowered swamp defense of Cuttlefish and Sea Serpent from 60% to 40%
   * Decreased cost of Giant Rat from 13 to 6
   * Various changes to the defense, movement and resistances of the Giant Rat
   * Increased the XP requirements of the Arif from 40 to 47
   * Increased the cost of the Falcon from 10 to 12
   * Decreased the beak and claw attack of the Falcon by one each to 2-3 and 5-1
   * Decreased the HP of the Falcon from 19 to 18
   * Increased the XP requirements of the Falcon from 20 to 24
   * Increased the XP requirements of the Hakim from 33 to 39
   * Decreased the HP of the Jundi from 36 to 32
   * Increased the XP requirements of the Jundi from 35 to 44
   * Increased the cost of the Khaiyal from 20 to 21
   * Decreased the blade resistance of the Khaiyal from 30% to 20%
   * Decreased the pierce resistance of the Khaiyal from 0% to -10%
   * Decreased the impact resistance of the Khaiyal from 10% to 0%
   * Increased the melee attack of the Mighwar from 7-5 to 8-5
   * Decreased the HP of the Mighwar from 57 to 53
   * Increased the melee attack of the Monawish from 6-4 to 7-4
   * Decreased the HP of the Monawish from 45 to 41
   * Decreased the HP of the Naffat from 32 to 28
   * Increased the XP requirements of the Naffat from 38 to 44
   * Increased the cost of the Naffat from 17 to 19
   * Decreased the melee attack of the Qatif-al-nar to 10-2
   * Decreased the ranged attack of the Qatif-al-nar to 8-3
   * Increased the XP requirements of the Rasikh from 40 to 150
   * Decreased the melee attack of the Tineen to 10-3
   * Decreased the impact resistance of the Falcon line from 0% to -10%
   * The Falcon line now gets 2 traits, one normal trait and the feral trait
   * The Naffat line is no longer able to get the strong trait
   * Converted units with khalifatelightfoot to khalifatefoot movetype:
     * Increased the fire resistance from -10% to 0% (Hakim only)
     * Increased the impact resistance from -20% to -10%
     * Increased the village defense from 50% to 60%
     * Removed the khalifatelightfoot movetype
   * Changes to khalifatefoot movetype:
     * Increased the frozen defense from 20% to 30%
   * Changes to khalifatearmoredfoot movetype:
     * Increased the shallow water and swamp mp from 2 to 3
     * Decreased the mushroom grove defense from 50% to 40%
   * Changes to the khalifatehorse movetype:
     * Increased the shallow water, swamp, cave, and mushroom grove mp cost to
       4 in all cases
     * Increased the forest mp cost from 2 to 3
     * Decreased the frozen mp cost from 4 to 2
     * Decreased the swamp defense from 30% to 20%
     * Decreased the cave defense from 40% to 20%
     * Decreased the mushroom grove defense from 40% to 20%
     * Increased the frozen defense from 10% to 30%
     * Increased the impact resistance from -10% to 0%
   * Changes to the khalifatearmoredhorse movetype:
     * Increased the shallow water, swamp, cave, and mushroom grove mp cost to
       4 in all cases
     * Increased the reef mp cost from 2 to 3
     * Decreased the frozen mp cost from 4 to 2
     * Decreased the cave defense from 40% to 20%
     * Decreased the mushroom grove defense from 40% to 20%
     * Increased the frozen defense from 10% to 30%
     * Increased the swamp defense from 10% to 20%
     * Decreased the hill defense from 60% to 50%
 ### User interface
   * Fixed Preferences dialog glitch on < 600 px tall resolutions (i.e. 800x480)
   * Made Hotkeys configuration dialog fit on < 600 px tall resolutions (i.e. 800x480)
   * Various minor improvements for consistency
   * Whiteboard planning system:
     * Added support for planning multi-turn moves
     * Added the option to hide allies' plans during a network game
     * Made action numbers colored according to team color
     * Made plan execution halt upon discovering hidden units
 ### WML engine
   * Added [event]id= support (to protect against duplicates)
   * Added [event]remove=yes|no support
     (to remove events that have an id set)
   * Implemented sub= and divide= for abilities and weapon specials
     (patch #2857)
   * new attribute replay_save= in [endlevel]. Defaults to yes, allows
     WML authors to disable replay saving for a scenario. (bug #18026)
   * [harm_unit] always uses wesnoth.float_label to avoid that an incorrect
     weapon filter may prevent floating the damage label
   * new key heal_full=yes|no (def no) in [effect]apply_to=type
   * Added SLF support to [event][item] and [remove_item].
     [remove_item]: IMPORTANT SYNTAX CHANGE: Default x,y=$x1,$y1
     is no longer supported, but instead all locations on the map chosen if
     no SLF keys are supplied. No wmllint rule in official release but patch
     in bug #18522.
     [event][item]: If x = and y= aren't explicitely given no longer throw an
     error but choose locations according to SLF behavior.
   * Allowed [modify_turns]current= to change to an earlier current turn
   * Added [primary_attack] support to [harm_unit]
   * Added wml tag [store_items]
   * [harm_unit] now supports a harmer unit and experience calculation.
   * Added support for 24 hour ToD.
   * Added WML validation system based on schema validation.
   * Enabled validation for GUI WML.
 ### Miscellaneous and bug fixes
   * Teach wmllint to fix deprecated implicit side=1 in [store_gold], [gold]
     [remove_shroud], [place_shroud], [modify_side], [modify_ai] actions
   * Fixed bug #17150: fix naming confict with OpenBSD macro by renaming
   * Added "-Wno-strict-aliasing" to the default compiler flags
   * Whiteboard planning system:
     * Made the whiteboard respond better to :droid, :give_control, etc.
     * Removed chat notification upon activating the whiteboard
   * Added advanced preference to ignore the encountered units
     list and show all unit types in the game Help

## Version 1.9.8
 ### Campaigns
   * Fixed deprecation warnings about "empty side="
 ### Language and i18n
   * Updated translations: British English, Estonian, Indonesian, Irish,
     Latin, Old English, Russian, Vietnamese
 ### Lua API
   * added support for slider and progress_bar widgets to
     wesnoth.get_dialog_value
   * added support for text_box, slider, and progress_bar widgets to
     wesnoth.set_dialog_value
   * new wesnoth.races table
   * wesnoth.get_terrain_info can now retrieve the editor_name field
 ### Multiplayer server
   * Handle incoming [whiteboard] data
   * Treat inactive forum accounts as unregistered users to prevent abuse
     of unverified registrations
 ### User interface
   * Converted New Folder dialog to GUI2
   * Moved Animate Map option from Advanced Preferences to Display
   * Moved Reverse Time Graphics display option to Advanced Preferences
   * Moved Scroll Tracking of Unit Actions display option to Advanced
     Preferences, renamed to Follow Unit Actions
   * Moved Unit Standing Animations option from Advanced Preferences to
     Display
   * Removed window decorations from GUI2 lobby
   * Sort the Recruit menu by localized unit type names (feature #18294)
   * Whiteboard planning system:
     * Added a new planned action type: "Suppose dead"
     * Changed behavior of invalid actions (no longer immediately discarded)
     * Disabled undo while planning mode is active
     * Made whiteboard plans visible to allied network players
 ### Terrains
   * Tweaked gameplay-visible names of terrain types, and restored many
     missing ones.
 ### Miscellaneous and bug fixes
   * CMake build system:
     * Disabled building of libana by default
     * Removed "-Wno-strict-aliasing" from the default compiler flags
   * Fixed bug #18117, saved games with Qatif-al-nar became corrupted
   * Fixed bug #18120, where Khalifate units were not getting the default AMLA
   * Patch #2663: Make wesnoth-optipng work on systems with BSD stat
   * Started working on the new asio based network subsystem. boost.asio
     (along with boost.thread and boost.system) is now a dependency for
     the client no matter whether ANA is enabled.
   * Support for gamepads/joysticks

## Version 1.9.7
 * Graphics:
    * Terrains:
      * Modified Deep Water tiles for greater contrast with Shallow
      * New Dead Great Tree
    * Portraits:
      * Drake Warden
 ### Language and i18n
   * Updated translations: Afrikaans, British English, Chinese (Traditional),
     Galician, German, Greek, Indonesian, Irish, Japanese, Korean, Latin,
     Lithuanian, Old English, Portuguese (Brazil), Serbian, Spanish, Swedish,
     Vietnamese
 ### Lua API
   * proxy getters and setters for unit attributes extra_recruit
     and advances_to
   * new function wesnoth.add_known_unit
   * new proxy getters for sides: fog, shroud, hidden, name, color
   * new function wesnoth.get_time_of_day
   * new functions os.clock, os.date, os.time and os.difftime
 ### Multiplayer
   * New "Shuffle sides" option in MP creation list, allowing to randomize
     player to side assignment (patch #1937 by Quetzalcoatl)
 ### User interface
   * Fix starting location labels being initially invisible in the map editor
     (bug #17956).
   * Fixed bug #18000, #18099: Show a wrongly entered MP password and crash
     upon editing this text.
 ### WML engine
   * added mode=replace to [modify_unit] to replace rather than merge unit subtags
     (does not apply to object, trait, effect, or advancement)
   * new attribute team_name= in SSFs
   * added [event][filter_side]<SSF keys> support
   * added support for inline SSF to [chat]
   * added support for inline SSF to [store_gold]
   * added support for inline SSF to [store_side], added attribute
     "side" in the created array
   * introduced [has_unit]search_recall_list=yes|no (def no) parameter in SSFs
   * support for leader specific recruit lists
     * [unit] extra_recruit= -- defines a unit with a specific recruit list
     * [event] [allow_extra_recruit] [filter] [/filter] type=
        -- adds unit types to a leader's recruit list
     * [event] [disallow_extra_recruit] [filter] [/filter] type=
        -- removes unit types from a leader's recruit list
     * [event] [set_extra_recruit] [filter] [/filter] extra_recruit=
        -- assigns a new recruit list to the leader
   * support for leader specific recall filters
      * [unit] [filter_recall] <SUF>
        -- The unit can only recall units which pass the SUF
   * Removed the Liminal alignment
   * Fixed: a divide by zero in the calculate_map_ownership_function function
   * Possibly fixed: rounding errors when using ^ in formulas (bug #18165)
   * Animation will now cycle according to a WML parameter, use with caution
   * Toplevel [tunnel] tags are now ignored rather than cause assertion
     failures (bug #18201).
 ### Miscellaneous and bugfixes
   * Fixed: issues with singular variant iterators
   * Fixed: the Wescamp script download part
   * Fixed the ingame command line not accepting
     characters accessible via AltGr (certain keyboard layouts)
     on windows systems
   * Started using Boost.Program_options for command line parsing (new
     dependency)
   * Commandline syntax changes:
     * --ai_config renamed to --ai-config
     * --new_storyscreens renamed to --new-storyscreens
     * --no-delay renamed to --nodelay
     * --campaign option split into --campaign, --campaign-difficulty and
       --campaign-scenario
     * split optional comma-separated defines list from --preprocess= (or -p=)
       to --preprocess-defines= option
     * dropped --log alias for --log-error
   * Fixed: Compilation on kfreebsd (Debian bug #626313)
   * Fixed: CMake Subversion revision script causing build errors with MSVC.
   * Fix time of day not changing in time area (bug #16584, bug #17543)

## Version 1.9.6
 ### Campaigns
   * The Hammer of Thursagan:
     * Fixed time over event in the High Pass
       (http://forums.wesnoth.org/viewtopic.php?f=4&t=33435)
 ### Graphics
   * Portraits:
     * Added portrait for Khalifate Hakim.
   * Terrains:
     * New Igloo village
 ### Language and i18n
   * Updated translations: Afrikaans, British English, Dutch, French, Galician,
     German, Greek, Hungarian, Irish, Japanese, Latin, Lithuanian, Old English,
     Portuguese (Brazil), Slovak, Spanish, Swedish, Vietnamese
 ### Lua API
   * max_attacks (read) and attacks_left (read/write) field of lua proxy units
   * new function wesnoth.compare_versions
   * new function wesnoth.get_sides
 ### Multiplayer
   * New era: the default+Khalifate era adds a new faction for multiplayer play.
   * New maps: 2p Thousand Stings Garrison, 2p Arcanclave Citadel, 6p Volcano.
   * Updated maps: Caves of the Basilisk, 2p Hamlets, The Freelands, Silverhead
     Crossing, Sablestone Delta, Fallenstar Lake, Den of Onis.
 ### Unit changes and balancing
   * Increased the pierce attack of the Orc Archer from 5-3 to 6-3.
   * Decreased the HP of the Cavalryman from 38 to 34.
   * Decreased the HP of the Dragoon from 53 to 49.
   * Decreased the HP of the Cavalier from 68 to 64.
   * Changed the 'feral' trait to give 50% instead of 40% defense on villages.
 ### User interface
   * Patch #2625: added a GUI interface to changing control in multiplayer
     games. The command to access it is currently :give_control
 ### WML engine
   * Patch #2610: changed default for turns in [scenario] tag to -1 (unlimited)
   * Introduced [recall]check_passability=yes|no key (default yes)
     for placing units only on suitable terrain when recalling.
   * Extended [heal_unit] to also "heal" moves, attacks, statuses
     and several units at once.
 ### Miscellaneous and bugfixes
   * Changed : replaced all sticky excpetions by lua jailbreak exceptions
     (fixes bug #17743).
   * Applied patch #2611: removed redundant own_side attribute
   * Applied patch #2600: improved MP creation screen logging
   * Added: New gui2 iterator framework.
   * Patch #2624: Solved enemy leaders never appearing on status table on
     foggy/shrouded maps, even when visible.
   * Fix linker issues with cmake and scons.

## Version 1.9.5
 ### Graphics
   * Portraits:
     * New portrait for Drake Enforcer/Thrasher.
   * Terrains:
     * Moved the ruined desert castle to core.
     * New and improved swamp villages.
     * New and improved tropical villages.
     * New ruined desert keep.
     * New stones and sand drifts embellishment.
     * New tall encampment keep.
     * New water lilies embellishment.
   * Other:
     * New elf-style flag.
 ### Language and i18n
   * Added missing Windows locale associations
   * Fixed Windows locale association for tr/Turkish
   * New translation: Old English.
   * Updated translations: British English, Chinese (Simplified),
     Chinese (Traditional), Czech, Dutch, Finnish, French, Galician, German,
     Irish, Italian, Japanese, Portuguese (Brazil), Slovak, Spanish, Vietnamese
   * Updated DejaVu Sans to 2.33
 ### Multiplayer
   * Don't show team labels to observers (feature request #9648).
   * Recalculate map labels to account for team changes when switching players
     with :control.
   * Renamed /adminmsg command to /report to better reflect its use.
 ### User interface
   * Added: circle to the gui2 canvas.
   * Added: new tip class for tooltips and helptips.
   * Reimplemented: the tooltips use the new tip class and look much better
     (bug #14818).
   * Fixed: the tooltips no longer stack when the MP dialog is opened
     (bug #16915, bug #16670).
   * Implemented: the helptips.
   * Changed: the scroll wheel, in gui2 code, now also follows the mouse focus
     instead of the keyboard focus.
   * Fixed again: Not showing the twml_exception dialog when gui2 was called
     from Lua (bug #17405).
   * Fix old multiplayer lobby glitches caused by the loadscreen code when
     skipping to the lobby with -s <server> command line.
   * Add 1.25, 1.75 and 3.0 animation speed factors to display preferences
     (feature request #15713).
   * Implemented: The expose event in gui2.
   * Fixed: Image widget now honors its minimum and maximum size.
   * Fixed: Black lines in the minimap.
   * Fixed: tooltips no longer capture the keyboard (bug #17797).
   * Changed: Allow underline in the gui2 font style.
   * Fixed: Not wrapping of transient dialog text (bug #17945).
 ### WML engine
   * Added support for map_passable and leader_passable for [placement]
   * Allow [color_range] and [color_palette] nodes to be inserted at top-level
     by add-ons to globally define custom ranges and palettes.
   * New [tunnel] tag to create teleports between two SLFs for units matching
     a SUF. The [teleport] ability retrofitted to use this tag.
   * New [allow_end_turn] and [disallow_end_turn] commands to enable/disable
     the human players' ability to end their turn from the user interface
     (feature request #13141).
   * [side] tags may now contain [leader] tags to create their leader(s), as
     opposed to mixing the leader's attributes with the side attributes.
   * New wml action tag [transform_unit], like the {TRANSFORM_UNIT..} macro.
   * [unstore_unit] now accepts a fire_event= key to control firing of
     (post) advance events and a check_passability= (default yes, previously
     it was always no/non-existent) key controlling whether to check for
     suitable terrain when placing units
   * Renamed [teleport]ignore_passability= to check_passability= to get rid of
     a confusing negation.
   * Introduced [move_unit]check_passability= (default yes, previously it was
     always yes/non-existent) key to allow disabling the check for suitable
     terrain.
   * Added TAKE_IT_STRING and LEAVE_IT_STRING arguments to PICKUPPABLE_ITEM
   * Added an engine-defined WESNOTH_VERSION macro that expands to the Wesnoth
     engine version string.
   * New #ifver/#ifnver preprocessor macro to compare Wesnoth or UMC-defined
     version numbers as in '#ifver WESNOTH_VERSION >= 1.9.5'.
   * The tags [remove_shroud] and [place_shroud] now take comma-separated lists
     of sides.
   * The [gold] tag now takes a comma-separated list of sides.
   * Added automatically stored variable this_unit to [modify_unit]
     for self-reference via $this_unit
 ### Miscellaneous and bugfixes
   * Fixed: g++ compiler warnings.
   * Added: cmake target to build the gui design pdf.
   * Removed support for TinyGUI: Devices with a resolution below 800x480 are
     not supported anymore.
   * Changed: The minimum screen resolution is 800x480, no need to use
     --smallgui anymore.
   * Reverted hotkey for clearing cache for Mac OS X back to F5. Control-F5
     still works (and is necessary when in windowed mode).
   * Let cmake use absolute locale dirs when set to an absolute path.
     (Windows always uses a relative path.) (patch #2280)
   * Make it impossible to build Wesnoth without the editor.
   * Fixed a replay OOS issue caused by ToD areas defined outside of events
     (bug #17783).
   * Changed: when loading a file fails to open try with a .gz file.
   * Unit invalidation is processed in parallel using OpenMP
   * Allow redirection of the logger.
   * Polished gui2 code.
   * Fixed: Newer versions of FriBidi were no longer recognized.
   * wmlindent now handles #ifhave, #ifnhave, #ifver and #ifnver properly in
     WML.

## Version 1.9.4
 ### AI
   * Fixed bugs #15861, #16223, #17206: fix passive_leader and
     passive_leader_shares_keep.
 ### Campaigns
   * Delfadors Memoirs:
     * Fixed bug #17273: Made difficulty selection conform to the style of all
       other mainline campaigns
   * Descent into Darkness:
     * New set of portraits.
   * Eastern Invasion:
     * Fixed bug #15950: Made 11_Captured remove units from recall list,
       preventing units being 'healed' upon load.
 ### Engine
   * Fixed bug #17355: split team initialization into two parts to prevent
       wrong determination of allied sides.
 ### Formula language
   * Added substring function.
   * Added length function, to determine the length of a string.
   * Added concatenate function.
   * Added sin (sine) function.
   * Added cos (cosine) function.
 ### Graphics
   * Terrain: added transitions for the wood floor.
 ### Language and i18n
   * New translation: Irish
   * Updated translations: Dutch, Finnish, Galician, German, Hebrew, Japanese,
     Korean, Lithuanian, Russian, Slovak, Vietnamese
 ### User interface
   * Fix alignment of text labels in certain confirmation dialogs (e.g.
     Quit Game/Editor)
   * Fix behavior of add-ons download dialog on double-click/enter
     (bug #17345)
   * Several improvements to the gui2 progress bar.
   * New add-ons description dialog with further details, including bundled
     translations.
   * Add new gui2 drawing widget.
   * Fix gui2 lines drawing glitch, which happens in some rare cases.
   * Fixed: Not showing the twml_exception dialog when gui2 was called from
     lua (bug #17405).
   * Fixed: Properly validate the height of a portrait (bug #17399).
   * Increase text area dimensions on story screens and improve space use on
     smallgui configurations.
 ### WML engine
   * New [harm_unit] tag for damaging, and eventually killing, units.
   * [allow_recruit], [disallow_recruit] and [set_recruit] now accept a
     comma-separated list for side=.
   * Unit types, units, and unit effects, can use small_profile= in
     addition to profile= to precisely describe portrait locations.
   * New ~BG(color) modifier for setting the background color of an image.
   * Made [inspect] tag work even without debug mode.
   * [move_unit_fake] now accepts an image_mods= attribute, specifying
     a list of path functions to be applied to the moving fake unit.
 ### Miscellaneous and bugfixes
   * Fix --data-dir command line option
   * Better detect mouse button state when window is activated.
   * Change wiki comment format.
   * Polish wiki_grabber.py code.
   * Names of attack ranges are now read from the range_$RANGE keys in
     data/hardwired/english.cfg. (feature #17395)
   * Names of attack types are now read from the type_$TYPE keys in
     data/hardwired/english.cfg.
   * Un-hardwired the [language] block in data/hardwired/english.cfg, and
     moved the file out of that subdirectory.
   * Add update-po4a-man and update-po4a-manual targets to cmake.
   * Added: Extra validate macro VALIDATE_WITH_DEV_MESSAGE.
   * Fixed: Link to libintl with cmake (bug #17152).
   * Fixed: Better cmake detection for older FriBidi versions (bug #17151).
   * Added: wiki_grabber.py the wml_reference description comment class.
   * Removed support for the "autotools" build system

## Version 1.9.3
 ### Campaigns
   * Descent into Darkness:
     * Allow Darken Volk to open gates in A Small Favor part 3 (bug #17250)
   * Legend of Wesmere:
     * Fixed the recursion of the scenario 4 bug.
     * Added the Elvish Horse Archer as alternative advancement for the scout
     * (Singleplayer only) Added the Dwarvish Runemaster as advancement for the
       fighter.
   * The Rise of Wesnoth:
     * New portraits for Jevyan, Typhon and Rithrandil.
 ### Graphics
   * Terrain:
     * Fixed display of UMC castles (which were being overdrawn by regular human
       castles).
     * Added ruined cottage and ruined hill village.
     * Added a fence embellishment terrain.
 ### Language and i18n
   * Updated translations: Dutch, Finnish, German, Italian, Japanese, Korean,
     Portuguese (Brazil), Spanish, Vietnamese
 ### Multiplayer
   * Fixed Siege Castles' description to state the correct map size, 36x36
     rather than 40x30 (bug #15835)
 ### Multiplayer server
   * Increased username length limit from 18 to 20
 ### User interface
   * Converted some dialog boxes to GUI2
   * Campaign difficulty descriptions must use Pango markup now
   * Added a new hotkey sequence (by default unassigned) to toggle animated map
     mode (feature #15976).
   * Removed bottom border from character [message] dialogs.
   * Improved the width of portraits in the wml_message once the maximum text
     width is reached.
   * Fixed display of unit-specific image mods on the Status Table dialog
     (bug #16285)
 ### WML engine
   * Created tag [petrify] (bug #17077). Moved [unpetrify] to lua. Syntax
     changed from [unpetrify][filter]<SUF> to [unpetrify]<SUF>.
   * New [floating_text] tag for creating floating text similar to the damage
     and healing numbers.
   * Introduced [recall]fire_event=yes|no (default no) parameter (fixes
     bug #17083).
   * Deprecated the following macros: FLOATING_TEXT, CAPTURE_FILTERED_VILLAGES,
     SET_OBJECTIVES, VICTORY_CONDITION, DEFEAT_CONDITION, ON_EVENT, ON_PRESTART,
     ON_START, ON_SIDETURN, ON_TURN, ON_VICTORY, ON_DEFEAT, ALLOW_UNDO,
     ON_TILE_ONCE, SIDE_PLAYER, SIDE_COMPUTER, AMLA_TOUGH, ITM_BOOK1, ITM_TREE1,
     RECALL_OR_CREATE_UNIT, RECALL_OR_CREATE, ITM_GLOWING_BRAZIER, and REDRAW.
   * Make [select_unit] highlight= (def. yes) work as intended for displaying
     the selected unit's reach (bug #16819)
 ### Miscellaneous and bugfixes
   * Fixed the submerge ability not working on all deep water terrains.

## Version 1.9.2
 ### Campaigns
   * Descent into Darkness:
     * Made 'Alone at Last' easier.
   * Legend of Wesmere:
     * Map updates.
     * Implemented gold carryover between the multiplayer chapters.
     * Added extra keeps to keep up with the growing number of leaders.
     * Fixed scenario 04 not being playable.
     * Corrected the objectives of scenario 3.
     * Bug fix for the fleeing orcs in scenario 07.
   * Heir to the Throne:
     * Fixed a bug causing Warven in 'Cliffs of Thoria' not able to move through
       mountains.
   * Liberty:
     * New set of portraits.
     * Changed Relnan's character to a woman.
     * New (unanimated) sprites for the Rogue Mage unit line.
     * Various balancing changes to the Rogue Mage unit line.
   * Northern Rebirth:
     * Make it possible to choose whether a unit should pick up the
       Rod of Justice or not.
   * The Rise of Wesnoth:
     * New set of portraits, except for Jevyan and Rithrandil.
     * Gave Jevyan's familiar a custom unit type.
   * Sceptre of Fire:
     * Thursagan can now advance to Dwarvish Arcanister.
     * New (unanimated) sprites for the Dwarvish Miner.
   * Son of the Black Eye:
     * Changes to the objectives and gameplay of 'Clash of Armies'.
   * The South Guard:
     * New portrait for Mal M'Brin.
 ### Editor
   * Verbose terrain names can be specified using terrain.editor_name to
     be displayed in the editor as "<verbose name>/<common name> (<underlying>)"
     (bug #16450)
 ### Graphics
   * Terrain:
     * Any Castle or Keep except Dwarvish can now be combined without large gaps
       or extra walls.
     * New graphics for wooden bridges.
     * Added variant of chasm bridge for over water.
     * New transitions from all Hills and Mountains to Water.
     * Underground and chasm friendly volcanoes.
     * A new desaturated color of shallow water (Wwg).
     * Much improved lava transitions.
     * A new desaturated ocean color (Wog).
     * New Waterfall automatically placed between chasm and water or swamp.
     * New Sand-to-Water transitions
     * Wave animations on sand
     * Added Ruined Human City terrain
   * Animations:
     * Standing anims: Fencer, Spearman, Dwarf Guard.
     * Idle anims: 2 for the Spearman.
     * Fixed drakes flying or not flying over all the correct terrains.
   * Portraits: Drake Blademaster, Hurricane Drake, Drake Flameheart, alternate
     Swordsman.
   * Units: New base frame and animations for Mudcrawler.
 ### Language and i18n
   * Updated translations: Chinese (Traditional), Czech, Dutch, Galician,
     German, Indonesian, Italian, Japanese, Korean, Lithuanian,
     Portuguese (Brazil), Russian, Shavian, Slovak, Slovenian, Vietnamese
 ### Unit changes and balancing
   * Decreased the physical resistances of the Dwarvish Scout and Dwarvish
     Pathfinder to 10% and those of the Dwarvish Explorer to 20%.
   * Decreased the village defense of Chocobone from 60% to 40%.
   * Moved the Fire Guardian unit to core.
   * Fixed villages on snow and sand hills providing worst instead of best
     movement.
   * The complete Dwarvish Runesmith line moved from SoF to core along with new
     sprites.
 ### User interface
   * Fixed: Addon dialog in title screen shows last host again.
   * Fixed: Addon download progress dialog shows the correct addon name again.
   * Fixed: Fullscreen hotkey works again in the title screen.
   * Fixed: Termination of the game when making the title screen small
     (bug #16724).
   * Fixed: A resize glitch causing resize events to be lost.
   * Changed: All buttons in the title screen now have a hotkey.
   * Changed: Refresh cache hotkey is CTRL+F5 on the Mac by default now.
   * Fixed: Don't trigger an assertion failure if no tips are defined
     (bug #16731).
   * Fixed: Enter no longer shows the credits in the title screen.
   * Changed: The title screen now has a maximum width for the tips text.
   * Changed: Improved the layout of the title screen.
   * Fixed: Changing the language updates map and logo in title screen
     (bug #16631).
   * Fixed: No longer cut off large title screen logos (bug #16632).
   * Refresh cache (F5) works in editor.
   * Add a "Save All Maps" menu item and hotkey in editor.
   * [object] description messages are now shown using GUI2, allowing usage of
     Pango markup (bug #16859).
   * Whiteboard planning system:
     * Fixed: Crash when creating a planned move on Windows (bug #16705)
     * New "Execute all actions" command bound to CTRL+y
 ### WML Engine
   * id= in SUFs now accepts a comma-separated list.
   * [capture_village] now accepts a full SLF.
   * events can be fired depending on a condition using [filter_condition].
   * Added support for SLF to [terrain]. Note that the SLF's terrain= key is
     not valid; terrain= specifies the new terrain instead like it used to do.
     Filtering for terrain can be done with the [terrain][and]terrain=
     workaround.
   * Renamed [removeitem] to [remove_item].
   * added new parameters directional_x and directional_y to animations
   * added new parameters auto_vflip and auto_hflip to animations
   * Made it so that units affected by [hide_unit] don't appear on the minimap
     (FR #16796)
   * New [store_reachable_locations] tag for storing the locations reachable (by
     movement, attack or vision) of units.
   * New [select_unit] tag, with optional fire_event (def. no) and hilight_hex
     (def. yes) attributes (FR #16819)
   * New [message] scroll= attribute to specify whether the game view should
     scroll to the speaking unit (defaults to yes) (FR #16843)
   * New key 'random_start' (default = yes) in [terrain_graphics][image]
     allowing to disable random animation shift in animated terrains.
   * Added a new key "primary" to animation to separate frames that should be
     treated as primary from frames that shouldn't
   * Made it so that if several [advancefrom] tags for a certain base unit are
     encountered, the lowest experience= keys from these is chosen (so if
     there's only one [advancefrom] referencing this base unit the experience
     needed can be increased).
 ### Miscellaneous and bugfixes
   * Changed: Lowered severity of some gui2 timer log messages.
   * Units created in debug mode now play their recruit animation (FR #16766).
   * Fixed: properly update cmake revision numbers (bug #16483)
   * Fixed: hotkeys dialog in editor is big enough to display them correctly.
   * Fixed: Shifted windmill animation (bug #16529)
   * Files matching *.wesnoth and *.project in add-ons are now ignored by
     default when uploading

## Version 1.9.1
 ### AI
   * Fixed bug #16585: made AI move in targeting phase even if for some of the
     'best' units moves to targets are impossible
 ### Campaigns
   * Unified the campaign description of the number of scenarios. Now the
     number reflects only the battle scenarios of each campaign.
   * A Tale of Two Brothers:
     * New portraits for Arne and Bjarn.
   * Dead Water:
     * Fixed the last scenario not working due to an 'unknown scenario' error.
   * Descent into Darkness:
     * Fixed the 'A Small Favor' scenarios being unbeatable.
   * Eastern Invasion:
     * Increased Owaec's hitpoints and attack stats, and added a new weapon
       special to his morningstar on levels 2 and 3.
   * Legend of Wesmere:
     * Splitted the multiplayer port into five chapters with 3 difficult levels
       each.
       * (Beta)  Chapter one, a two player campaign, ends after scenario 3.
       * (Beta)  Chapter two, a three player campaign, ends after scenario 7.
       * (Alpha)  Chapter three, a four player campaign, ends after scenario 13.
       * (Alpha) Chapter four, a four player campaign, ends after scenario 17.
       * (Alpha) Chapter five, a two player campaign, ends with the single
                 player version.
       * There is no savefile compatibility between saves from an older version
         of Wesnoth.
     * Scenarios
       * Scenario one's objectives changed.
       * Completed a rewrite of scenario 5, changing the objectives, ai and
         game mechanism slightly.
       * Scenario 7
         * Shortened by letting the orcs flee if their numbers went too low.
         * The player can choose between two different starting positions for
           Olurf.
       * Rebalanced Scenario 14.
     * Miscellaneous and bug fixes:
       * Renamed some of the locations at the Kalian to fit with the map
         changes.
       * Fixed scenario number 5 where the gold carrier's overlay was not
         removed.
       * Fixed the bug with the army split up before scenario number 9.
       * Map and coding updates regarding the new terrain types and graphics.
       * Added keeps to every scenario to match the number of leaders
         (singleplayer) or sides (multiplayer).
       * Removed the obselete Haldric the second portrait.
   * The Rise of Wesnoth:
     * Made surprise enemy spawns appear in a less immediately dangerous way in
       'The Midlands', 'The Swamp of Esten', 'Peoples in Decline',
       'A Rough Landing', 'The Vanguard', 'Return of the Fleet' and
       'Rise of Wesnoth'.
     * Clarified the early finish bonus conditions in 'Fallen Lich Point' and
       'Sewer of Southbay'.
     * Allowed Merman Hunters to be recruited alongside Merman Fighters.
   * Sceptre of Fire:
     * Fixed bug #16542: Alanin not appearing in the epilogue.
   * The South Guard:
     * Fixed a bug causing a freeze at the beginning of 'The Long March'.
 ### Editor
   * Added a standard click sound to brush bar buttons (bug #15635)
 ### Graphics
   * New animations: Merman Hunter ranged animation and defence, Warrior se
     attack and defence, Drake Flare and Flameheart leadership.
   * New portraits: Inferno Drake, Mermaid Initiate alternate, Goblin spearman
     alternate
   * Terrain:
     * Tropical Ocean added
     * Improved transition between void and off-map and with both and the
       oceans.
     * Snowy Human City Village added
     * Muddy Quagmire (Sm) added - alias of swamp
     * Old Desert Mountains replaced with a non-green version of the mountains
     * Slightly tweaked dirt colors and new, smooth dirt transitions
     * Better transitions for Cobbled Road, Clean Cobbles, and Overgrown Path
     * Fixed hard edge on immpassible mountains clouds.
     * Fixed Transition of leaf litter to water.
     * Fixed hard edge on top of cave beams
     * Fixed lava drawing extra transition on off-map
     * Any Castle or Keep except Dwarvish can now be combined without large gaps or extra walls.
   * Better rendering of unit in water: transparency decreases with depth.
 ### Language and i18n
   * Updated fonts: DejaVu 2.32
   * Updated translations: Chinese (Simplified), Chinese (Traditional), French,
     German, Hungarian, Indonesian, Japanese, Lithuanian, Polish, Russian,
     Slovak, Vietnamese
 ### Multiplayer
   * Updated maps: 4p Hamlets.
   * Updated most of the maps taking advantage of the new terrains.
   * Side vision is now switched before the healing phase of the turn (only
     visible in hotseat)
   * Re-added the old MP lobby.
 ### Terrain WML
   * Updated height adjust of desert, orcish, and snowy keeps.
   * Fixed broken aliasing of the wooden floor.
   * Stop using negative unit height adjust for water terrains.
   * Chasm bridges can now be placed on any terrain, and the lit-by-lava
     variants are automatically used when placed over lava.
   * Removed village terrains: ^Voha, ^Voh, ^Vhms, ^Vhm, ^Vcha, ^Vch, ^Vcm. See
     the village aliasing change listed below.
 ### Unit changes and balancing
   * Decreased the melee and ranged attack of the Footpad from 5-2 to 4-2.
   * Decreased the XP requirement of the Mage from 60 to 54.
   * Decreased the XP requirement of the White Mage from 150 to 136.
   * Decreased the XP requirement of the Mermaid Priestess from 150 to 132.
   * Added the Ghast unit from DiD to core with new base frame and animations.
   * Made the Giant Rat have a normal AMLA instead of an AMLA with no fullheal.
   * All bats are now given a 'feral' trait which caps their defense on villages
     to 40% and also making them receive only one random trait.
   * All villages except for water and swamp villages are now aliased to both
     village and the terrain they're placed on, giving best movement and defense
     of both.
 ### User interface
   * Changed: the title screen is now gui2 (bugs #12906, #12908 and #15987).
   * Use red/green color for damage in sidebar when modified by bonus/malus
   * Placing a waypoint on a capturable village will now make the unit pause
     there to capture it (FR#16603)
   * Fixed bug #16653: Avoid markup when calculating the text length for
     ellipse text (Debian bug #547476).
   * Damage type tooltip now also shows damage after resistance calculation.
 ### Whiteboard
   * Added cost display for planned recruits and recalls
   * Fixed bug #16554 : Infinite attacks with the whiteboard
 ### WML Engine
   * Added tag [kill][secondary_unit] for specifying the killing unit.
   * Added a LOW_MEM define to WML to be able to adapt WML to low memory
     builds.
   * Added event: side turn X
   * Added event: turn X refresh
   * Added [chat] tag for outputting public or private messages to the chat.
   * Added events: turn end and turn X end
   * Added events: side turn end, side X turn end, side turn X end,
     and side X turn Y end
   * Added "variations" key and "@V" symbol in [terrain_graphics] (syntax is
     not final, and may change later)
   * Fixed a bug causing the PUT_TO_RECALL_LIST macro to freeze the game.
   * Modified [set_variable] "divide" so that it always performs a
     floating-point divide.
   * Allow time_area to define local time of day on map border (bug #16508)
   * Allowed negative defense values as a way to set upper bounds,
     e.g. village=-60 means that a unit cannot have less than 60 def (more
     than 40% def) on terrains containing villages.
 ### Miscellaneous and bug fixes
   * Removed: statistics upload code.
   * Changed: compiler mode set to c++98
   * Optimize terrain rules which speed up cache creation and loading
   * Optimize rendering of flying units
   * Fix crash when attacking in fog (using teleport+attack to a fogged village)
   * Fixed a bug causing turn counter in the objectives dialog to duplicate when
     the objectives are viewed several times.
   * Replace "working peasant" (indicating missing images in debug mode) by
     half-transparent "Image not found"
   * Improved rendering algorithm: reduced memory requirements and a much faster
     render loop; very noticable on big maps
   * Improved framerate by removing useless 20ms delay between frames.

## Version 1.9.0
 ### AI
   * Fixed bug #15994 : Formula AI candidate actions specified in [side][ai]
     don't always work.
   * Fixed bug #16406: Broken AI of SoF 1, and improved upgrade procedure for
     old-style AI config.
 ### Campaigns
   * An Orcish Incursion:
     * Made 'Valley of Trolls' easier on the easiest difficulties.
   * Dead Water:
     * New campaign added to mainline from the Wesnoth-UMC-Dev Project's
       repository (Intermediate level, 13 scenarios)
   * Descent into Darkness:
     * 'A Small Favor' (part 1) now gives an early finish bonus.
     * Dela is no longer invulnerable in 'Alone at Last' but cannot be safely
       assassinated.
   * Eastern Invasion:
     * New (unanimated) sprites for Owaec.
     * Made it slightly easier to rescue the knights in 'Mal-Ravanal's Capital'.
     * Increased the turn limit in 'Two Paths' from 18/16/14 to 18/17/16.
   * Heir to the Throne:
     * Made 'Cliffs of Thoria' much easier.
     * Made 'Isle of the Damned' slightly easier.
   * Liberty:
     * Increased the difficulty of 'Unlawful Orders', 'The Grey Woods' and 'The
       Hunters' and 'Glory'.
   * Northern Rebirth:
     * In Showdown, you recover full gold only if Sisal survived the previous
       scenario.
   * Son of the Black Eye:
     * Increased the turn limit in 'Black Flag' from 30/25/20 to 30/28/24.
   * The South Guard:
     * A new set of story art by Scavenger.
     * In 'Vengeance', the final objective is now revealed at the beginning of
       the scenario.
     * New images for the Infantry Lieutenant and Commander by Rhyging5
   * Scenarios in Eastern Invasion, Liberty, Northern Rebirth, Son of the
     Black-Eye and The Rise of Wesnoth which feature computer-controlled allies
     now allow you to affect their behavior via the right-click menu.
 ### Graphics
   * Added new portraits for: the Orc Archer, Crossbowman, Slurbow, Sayer,
     several new Orc Grunt line alternates, Draug and one alternate, Ghoul,
     Skeleton Archer line, Giant Mudcrawler, Orc Leader/Ruler, alternate Leader
     and Sovereign, alternate for Loyalist Swordsman, Drake Fighter, Burner.
   * New animations (not yet including leading animations) for Drake Flare
     and Flameheart, Dwarf Lord ranged attack, Dwarf guard melee attack, Dwarf
     Pathfinder idle, Ruffian attack and defend, Spearman s and se attack.
   * New base frame and animations for Armageddon Drake.
   * Fixed bug causing Drake Clasher's animation to display incorrectly.
   * Added missing help version for Sea Serpent portrait.
   * Items & scenery: New anvil, and revised trash and lighthouse
   * Two new flag styles.
   * Animate terrain in editor
   * New advanced preference to use a local ToD color-shift
   * Added framework allowing to draw various arrow styles on the map.
 ### Language and i18n
   * Updated translations: British English, Catalan, Chinese (Simplified),
     Chinese (Traditional), Czech, Dutch, Estonian, Finnish, French, Galician,
     German, Hungarian, Indonesian, Italian, Japanese, Latin, Latvian,
     Lithuanian, Polish, Russian, Serbian, Spanish, Slovak, Vietnamese
   * Updated DejaVuSans to 2.31
   * Fixed bug #15653: Made untranslateable strings translateable.
   * Fixed bug #15843: Made untranslateable strings translateable.
   * Fixed bug #15934: Flushed image cache when changing language
   * Fixed bug #15937: Made untranslateable strings translateable.
   * Make map labels store translatable strings, so when the language changes,
     the labels also use the new translation.
 ### Multiplayer
   * Updated map: Ruins of Terra-Dwelve.
   * Fixed bug #15865: missing WML Child error.
   * Added the winner of the first Wesnoth map competition, the 2 player map
     "Elensefar Courtyard" by krotop.
 ### Music and sound effects
   * New version of "Northerners" by Stephen Rozanc (TreizeCouleurs)
   * New sounds for wolves and wolf riders. Removed all old wolf-* sounds.
 ### Terrain
   * all villages except water and swamp villages can now be placed on any base
     terrain
   * orcish, elven and human snow villages are now aliased to village,snow
     instead of just to village
   * orcish and human snow hills villages are now aliased to village,snowhills
     instead of just to village,hills
   * animated terrain animations are not synchronized anymore
   * reworked macros to handle animations
   * added new stone-wall-lit terrain
   * made all animated terrains use the new macro system
   * added a new "snowy castle" terrain
   * added new mausoleum scenery
   * added new "snowy fort"
   * added new "desert castle"
   * added new "tropical water"
   * added new "dead grass" terrain
   * improved dry grass "savanna" terrain
   * improved green grass "grassland" terrain
   * improved "desert road" terrain
   * added new "dark dirt" terrain
   * added new "small stones" embellishment
   * added new "small mushrooms" embellishment
   * fix for dwarven castle to cave wall alignment issue by Alarantalara
   * made encampment and orc forts use regular dirt rather than a custom terrain
   * created new editor group: "embellishments", and added desert plants to
     that group
   * improved snow terrain
   * added "leaf litter" terrain
   * added new "mine rail tracks" terrain
   * added 3 new variants for log cabin
   * added impassable snow mountains
   * adjusted the color of various terrains for a more coordinated appearance
   * added new dry hills terrain
   * new ford that works with animated water
   * improved cobbled road now also used for castle (not keep) floor
   * new etherial abyss terrain added to mainline
   * flower base terrain is deprecated, now available as an overlay
   * added new clean cobbled road terrain
   * added castle to chasm transitions
   * improved human city village
   * Dark flagstones mainlined from UtBS
   * added castle to lava chasm transitions
   * chasm type terrain now blend nicely with void and off-map
   * new banks for transition between flat or cave terrains and water
   * adjusted all underground terrain to harmonize with other terrains
   * improved mini-map appearance for most chasm, wall and grass terrains
   * stone path now matches other terrains
   * added wooden floor terrain
   * added mushroom farm terrain
   * added volcano terrain.
   * added a special blend transition for mountains next to chasms
   * added snowy encampment
   * special snowy chasm appears next to snow terrains
   * added earth-toned chasm
   * added earthy cave floor
   * added hewn cave wall, earthy cave wall, and earthy hewn cave wall.
   * animated water and swamp now use a double-sided transition to blend gradually into each other
 ### Terrain WML
   * rename TRANSITION_RESTRICTED and TRANSITION_RESTRICTED2 to
     OVERLAY_ROTATION_RESTRICTED and OVERLAY_ROTATION_RESTRICTED2
   * All unused macros are slowly removed to try to get a logical and complete subset of macros
   * Allow to use local ToD terrain variants in [time_area]
   * ToD key in terrain [variant] now accept a list of ToD
   * New key 'set_no_flag' in [tile] which combines 'set_flag' and 'no_flag'
 ### Units
   * Giant Rat moved from DiD to core.
 ### User interface
   * Added the era AI in the list of AI shown when opening a game
   * Added dialog for installation of add-on dependencies
   * Removed the unused gui2 menu bar
   * Added a window register framework
   * Enabled the --new-widgets MP create dialog again
   * Enabled the hidden join observe buttons per row
   * Enabled alignment in labels
   * Avoid resizing when next or previous button is pressed in the
     --new-widgets title screen
   * Added gui2 progress bar widget
   * Added "animated" logo to the --new-widgets title screen
   * Fixed the language is refreshed after the language is changed in the
     --new-widgets title screen
   * Implemented bug #15623 (patch #1568): On change resolution screen,
     widescreen resolutions are now marked as such
   * Added tooltips to the --new-widgets title screen
   * Added a place holder upload statistics button in the --new-widgets title
   * Fixed bug #15716: Lobby crash when refreshing with filtered out games
   * Fixed bug #15768: Avoid parts of the previous game show in the lobby
   * Fixed bug #15727: Allow wml message titles to wrap
     screen
   * Added the total number of villages to the status table lists
   * Added a new attack dialog, available for testing with --new-widgets
   * Patch #1645: Fixed a bug sending keyboard events to deactivated controls
   * Deprecated the resize flag for gui2 image, use the resize_mode instead
   * Patch #1639: Added handlers for keyboard (arrow keys) to move gui2 sliders
   * Added new experimental list box implementation, available for testing
     with -DGUI2_EXPERIMENTAL_LISTBOX
   * Damage in sidebar now also take account of ToD and leadership
   * More info in the weapon tooltip (damage bonus calculation and swarm effect)
   * Terrain defense tooltip show terrain's info and defense calculation
   * Level tooltip shows next advancements
   * Each trait and special attack has its own tooltip
   * Most sidebar tooltips have now a category indication
   * Tooltips of ellipsed text are grouped in the last visible item's tooltip
   * Increase a little tooltip's opacity
   * Render grid above foreground terrain
   * Clicking on some elements of sidebar now open the related help page
   * Display weapon stats in recruit/recall dialog the same way as in sidebar
   * Accuracy/parry have its own line and tooltip in sidebar.
   * Add first, last, play and back buttons to storyscreens
   * Remember recall list sorting order (FR #16149)
   * accelerated movment speed to 200ms per hex
   * --logdomains accepts a filter argument and uses multiline output
   * New mouseover image instead of simply highlighting the hex
   * Tab completion for :commands and units search function
   * Fixed #15781: On maps with statues player can pick statues team
   * Fixed move+attack not interrupted when ambushed at destination
   * Stop disabling mouse during attack+move
   * Added the whiteboard planning system (GSoC project), see release notes for details.
   * Allowed viewing terrain defense for the selected unit outside of your turn.
 ### WML Engine
   * Added wml action tag: [modify_unit]
   * Added wml action tag: [move_unit]
   * Deprecated [set_variable]'s random key, use rand instead
   * Renamed [unit][status] healable to unhealable so it can default to 'no'
   * Added 'side X turn refresh' and 'side X turn Y refresh' events
   * Add ~DARKEN() counterpart to ~BRIGHTEN()
   * Implement min_value for [illuminates]
   * Added lua functions wesnoth.get_side_count() and wesnoth.is_enemy(a,b)
   * Add 'recall_cost' key for [side], to override [game_config]'s default
   * Add [replace_schedule] tag, which replaces the time of day schedule
   * Trying to include a missing macro/file is now a fatal error
   * Added #ifhave/#ifnhave for testing existence of files and directories
   * Added [volume] tag, which allows game volume to be changed during scenarios
   * Prototype support for [set_global_variable]
   * Prototype support for [get_global_variable]
   * Prototype support for [clear_global_variable]
   * Added scroll_to_leader attribute to side tag.
     Default value is 'yes' (bug #15921)
   * Draw the map border over _off^_usr tiles too.
   * Add 'immutable' key to [label], defaulting to true (feature #16078)
   * Added search_recall_list key to [have_unit] tag
   * Added [move_units_fake] tag
   * Added reveal_map key to [endlevel] tag
   * Rename all "colour" keys to "color" (in [side] and [label]), same for
     "colour_lock"
   * Allow a [case] value to take comma-separated values
   * Move fog/shroud image definition into game_config.cfg
   * Add new game_config keys "hex_brightening", "hex_semi_brightening",
     "mouseover_image" and "selected_image" to tune mouse interface
   * Fixed bug #16219: Handled ToD areas in a LIFO way, so that it is possible
     to override them without first removing them
   * [effect] violate_maximum= (for use when increasing HP) takes a real boolean
     value now instead of taking any non-empty value as "true".
   * Allow checking out terrain defense for units when it's not your turn.
   * New image path function: dst~BLIT(src[, x, y]) blitting src image on dst
     image at coordinates (x,y)
   * Fix bugs in 'illuminates' when using non-standard values. Now max and
     min_value only clamps the effect of the illuminates bonus, but ToD and
     terrain effect can pass them.
   * Added [gold_carryover] tag to the [objectives] tag.
   * Added [note] tag to the [objectives] tag.
   * Added caption= and show_turn_counter= keys to the [objective] tag.
   * New WML macros: ON_DIFFICULTY (a macro that makes using different values
     based on difficulty simpler), ON_SIGHTING (a substitute for sighted events)
   * Removed WML macros: NEUTRAL_SIDE
 ### Miscellaneous and bug fixes
   * Added a network library for asynchronous server & client applications (ANA)
   * Rewrote the network module using this (ANA) library
   * Added help entry when new unit is created directly in the recall list
   * Defaulted log level to warning again
   * better fix for bug 14765 now that string freeze is off
   * Fixed picking the proper locale, the problem only occurred on some
     systems
   * Added a way to compile wesnoth on windows by using CMake + MSVC9.
   * Added the possibility to specify absolute paths for "--config-dir"
   * Added more command line arguments for starting a campaign + scenario
   * Added command line "--preprocess" to preprocess a specified file/folder and
     output the result
   * Added command line "--preprocess-input-macros" to specify extra input macros
   * Added command line "--preprocess-output-macros" to output the preprocessed
     macros to a file
   * Added command line "--data-dir" to explicitly override the data directory
   * Fixed the ping timeout not waiting for the default ping interval when
     ping timeout is not set to 0
   * Fixed a bug in scoring of AI recall list. Patch by billynux.
   * Strip whitespace characters from .ign patterns (bug #15902)
   * Fixed wesnoth_addon_manager's support for .ign files (bug #15846)
   * Never allow uploading *.pbl files (case-insensitive) to the add-ons
     server from the regular game client.
   * When warning the player about installing add-ons with missing dependencies,
     make 'OK' and 'Cancel' work as expected (bug #15960)
   * Fixed tab completion not working in the new lobby (bug #14730)
   * Fixed compilation for g++ 4.5
   * Cleaned up the gui2 code at various places
   * Don't crash if a [story] [part] [if] misses [then] or [else] (bug #16028)
   * Fix a crash on OS X caused by passing invalid utf8 to pango (bug #16020)
   * Hidden weapons (attack_weight = 0) no longer skew the best weapon selection
   * Prevent dereferencing freed memory when reporting malformed maps, i.e. when
     using an invalid terrain type
   * Fixed attack predictions for combats with a unit leveling up thanks to a kill
   * Fix rare layer's order bug about unit drawn above big south-west unit
   * Improvements to make Wesnoth compile better with g++-4.5 in C++-0x mode
   * Added ":undiscover" to clear all your discovered units from help
   * Added ":turn" to change the current turn/time of day in debug mode
   * Added ":turn_limit" to change the turn limit for a scenario in debug mode
   * Fix a crash when a sighted event killed a unit just before a fight
   * Fixed bug #16171: Disable commands during [animate_unit]
   * Fixed bug #16235: Avoided displaying an empty menu and therefore choosing
     a random weapon, when there is none
   * Fixed bug #16243: Added detection for server replays, as they are missing
     the core [lua] tags
   * Fixed bug #16261: Added test for invalidated death due to positive hp
     after the 'die' event
   * Fixed bugs when a waypoint is unreachable
   * Fixed attacker still getting resting bonus after attack using movement_used=0
   * Fixed [store_time_of_day] for earlier turn
   * Fixed _off^_usr not using the tile_image of theme.
   * Patch #1727: Fixed revision.hpp generation with cmake 1.8(.2)
   * Added debug command ":foreground" to better visualize foreground terrains
   * Added debug command ":layers" displaying various layer info from the hex under
     the mouse.
   * Reducing cache loading for title screen, --test, --editor and --load
   * Removed Lua dependency, the source is now in the source tree
   * Introduced a new allignement called "Liminal". Those units fight best during the twilight times of day.
   * Fixed #16343: wmllint wants to add translation markers to the empty string in description=
   * Reduce individual memory cost of each terrain image

## Version 1.8.0
 ### AI
   * Fixed bug #14247: Make formula AI behave correctly if the side has only 1
     potential recruit.
   * Added support for candidate actions written in lua.
 ### Campaigns
   * Descent into Darkness:
     * Added new Giant Rat base frame and animations.
   * Legend of Wesmere:
     * Fixed bug #15631: Scenario 3: arrival of Kalenz failed
     * Fixed bug #15679: Scenario 17: leader of side 2 is missing
     * Fixed bug #15680: Scenario 18: wrong recruitment options
   * Under the Burning Suns:
     * Scenario 2: speed up AI turn.
 ### Graphics
   * Added new Cave Spider and Cuttle Fish graphics
 ### Language and i18n
   * Added new translations: Serbian Ijekavian, Serbian Ijekavian Latin
   * Updated translations: Czech, Finnish, French, German, Hungarian, Japanese,
     Latvian, Lithuanian, Russian, Serbian, Spanish, Slovak
 ### Multiplayer
   * Updated maps: Cynsaun Battlefield
 ### Music and sound effects
   * Fixed bug #15668: The lobby will play a random music playlist, configured
     by [lobby_music], instead of looping the main menu song
   * Fixed bug #15669: The titlescreen will play a random music playlist,
     configured by [titlescreen_music], instead of looping the main menu song
     First song played will still always be the main_menu theme.
 ### User interface
   * Worked around bug #15561: Resizing the lobby made the items in the game
     listbox too small
 ### Miscellaneous and bug fixes
   * Added the first draft of the gui2 design documentation
   * Defaulted log level to error again
   * Fixed bug #13882: Map which is invalid aborts map selection
   * Fixed bug #14114: Checksum operations fail against certain scenario events
   * Fixed bug #15545: Recall list gone after loading savegame created in
     linger mode
   * Fixed bug #15598: Can't move units after reloading game
   * Fixed bug #15601: Replay crashes when replay log messages are enabled
   * Fixed bug #15656: OOS errors in LoW from differing starting gold
   * Fixed a segfault due to missing seed-attribute of an attack in a replay
   * Fixed a graphics bug with the Spearman's attack anim noted by zookeeper
   * Worked around bug #13333: Limit the maximum length of the mp command
     dialog as workaround for bug (This workaround is only implemented for
     Windows and Mac)
   * Worked around a rare assertion failure when resizing the lobby

## Version 1.7.15-1.8rc1
 ### AI
   * Set RCA AI to be the default AI for single-player campaigns.
   * Fix bug #15390: add a try_delete action to modify_ai which has
     'delete if exists, don't complain if not exists' semantics
   * Fix bug #15013: make AI gotos persist between turns when set by
     WML, make the AI don't use gotos for normal moves.
 ### Engine
   * Fix bug #15542: if game encounters a base_unit that refers to a
     unit that the game cannot find, throw exception instead of
     failing assertion.
   * Add a list of team units and a dedicated unit mode to gamestate
     inspector (launched by :inspect command and [inspect] tag).
 ### Language and i18n
   * Updated translations: Chinese (Traditional), Czech, German, Hungarian,
     Japanese, Serbian
 ### Multiplayer
   * Fix bug #15541: fix OOS on [unit] tag generating different
     traits because of usage of local RNG instead of MP RNG.
   * Fix bug #15560 for Dark Forecast: fix OOS in Dark Forecast caused by
     unit advancement not properly synced across the network.
 ### Music and sound effects
   * Added new music track, "Weight of Revenge" by Doug Kaufman
 ### User interface
   * Improved resizing of a window when the contents don't fit, fixes the
     window scrollbars in the MP lobby
   * Fixed redraw invalidation issues in the MP lobby

## Version 1.7.14-1.8beta7
 ### AI
   * Allow to write AI components in LUA
   * Implemented FR #15465: 'protect' goal was split into protect_location,
     protect_unit, protect_my_unit [goal] tags. protect_my_unit can be used
     as a direct replacement for protect_leader.
 ### Graphics
   * Fixed bug 15344: missing ice to nothing transition
   * Fixed weird side effect of long first frame in standing anims
   * Fix bug 15366 : overlay terrains badly interacting with submerge and height
     adjustments
   * Fix bug 15544 : bad transitions between roads and deserts with overlay
 ### Language and i18n
   * Updated translations: Catalan, Chinese (Traditional), Czech, Estonian,
     Finnish, German, Greek, Italian, Japanese, Lithuanian, Polish, Serbian
 ### Multiplayer
   * Make allow_changes attribute truly work
   * Fixed players getting different side colours across scenarios of mp
     campaigns
   * Fixed bug 14754: Host can start game before the client has selected a
     leader (Debian bug #555964)
   * Fix bug #15380 (cl in multiplayer local game doesn't work)
   * Fix bug #15382 (Player doesn't get transported to the next scenario)
   * Fix bug #15383 (Multiplayer Campaigns can't be loaded from savegame)
   * Fix bug #15391 (Warnings in multiplayer games)
   * Fix bug #15398 (Multiplayer Campaign aborted after endlevel)
   * Fix bug #15399 (Leaders of the ai sides in LoW multiplayer scenario2 are
     missing)
   * Allow a 1-sides game to be started (Debian bug #568029)
   * Fixed A New Land not working when there are empty sides
 ### User interface
   * Rewrote the sizing code of the tree view widget
   * Don't show turn dialog once the level has ended
   * Fix the empty games in the MP lobby game list
   * Fix redraw glitches of the scrollbars when resizing a widget
   * Fix a crash when using the scrollwheel (bug #15156)
   * Fix an issue where the lobby chat log didn't resize properly
   * Fix the translation of certain lobby strings
   * Enable the scrollwheel for the tree view
   * Enable the scrollwheel for the scroll label
 ### Miscellaneous and bug fixes
   * Fix bug #15429 (Units created by WML can only get the Neutral alignment);
     this also affected MP leaders
   * Fix UB when closing a window, caused by the children of the window
     accessing the destroyed window members

## Version 1.7.13-1.8beta6
 ### AI
   * Port [protect_leader], [protect_unit], [protect_location] to new-style ai
     config, which is a goal with name=protect, which accepts a SLF [criteria].
   * Changed names of AI log domains, to have a more uniform naming style.
     Most names became shorter.
 ### Campaigns
   * Fixed a bug in several scenarios causing some enemy units to disappear when
     loading a save
 ### Engine
   * Fix bug #15146: made kill event with animate="yes" recheck the presence of
     unit before animating, fixing the assertion failure (in case the unit is
     removed by other wml events like last breath)
   * All unit-related images are team colored, this includes missiles and haloes
 ### Graphics
   * Add and wire two new Drake attack icons.
 ### Language and i18n
   * Updated translations: Czech, Finnish, French, German, Hungarian, Italian,
     Lithuanian, Polish, Portuguese (Brazil), Russian, Serbian
   * The manual now does support translations of alternative texts for images
     (bug #14874)
 ### Multiplayer
   * An early test version of the multiplayer port of "Legend of Wesmere"
     is available when starting wesnoth with the commandline argument "--debug"
 ### User interface
   * Add a new tree view widget
   * Use tree view widget as test in campaign dialog (needs --new-widgets)
   * Use tree view widget in the lobby
 ### Miscellaneous and bugfixes
   * Fixed serveral issues found by cppcheck

## Version 1.7.12-1.8beta5
 ### AI
   * Fixed unit_at formula ai function to return null on null input
 ### Campaigns
   * Fixed a bug in several scenarios causing some enemy units to disappear when
     loading a save
 ### Language and i18n
   * Updated translations: Czech, Finnish, French, German, Hebrew, Italian,
     Latin, Russian, Shavian, Slovak, Spanish
 ### Multiplayer
   * Non-human null controllers are no longer set to ai
   * Don't allow a 0-sides game to be started (Debian bug #563310)
 ### Music and sound effects
   * New version of sad.ogg (Sad music) by Tyler Johnson
 ### User interface
   * Switched back to the tiled background for gui2
   * Improved to looks in tiny-gui
   * Add a new transient error message
   * Convert several old style message dialog to the new style
   * Allow listboxes to add rows at every place instead of at the end only
   * Improved the speed of the new lobby
 ### WML Engine
   * Removed bogus merging of the old unit type's movetype when advancing
     (fixes bug #15055)
   * [unit] upkeep now accepts the 'free' value as a synonym for 'loyal'
     It was widely used this way and worked because its integer value is 0
 ### Miscellaneous and bugfixes
   * Fix errors in tutorial when the player unexpectedly kills certain enemy
     units in scenario 2 (bug #15037)
   * Fix orcs being able to play their turn twice in tutorial scenario 2
     (bug #14926)
   * Fix missing dialog in tutorial scenario 1 (patch #1399)
   * Fix scrolling during animation (bug 13106)
   * Fix drakes tipping their wings in the water when flying
   * Fix some assertion failures when showing/hiding listbox items
   * Fixed serveral issues found by cppcheck
   * Fixed drawing glitches with tiled gui2 windows in the lobby
   * Fix some doxygen warnings
   * wesnoth-optipng can now process selected files given on command line
   * Allow listboxes to better request the update of their contents

## Version 1.7.11-1.8beta4
 ### Language and i18n
   * Updated translations: Chinese (Simplified), Estonian, French, Latvian,
     Lithuanian, Portuguese (Brazil), Russian, Serbian, Slovak, Spanish
   * Manual: updated CSS style to cover Docbook markup for GUI elements
 ### User interface
   * Add a new repeating button widget
   * Scrollbar buttons now keep scrolling when kept pressed down
   * Optimized the speed of the --new-widgets game load dialog
   * Waypoints of multi-turns moves are now saved between reload
 ### WML Engine
   * Rework of semantics of [unit] tags. Added 'placement' attribute.
     Fix bugs #14373, #14444, #14451 and other 'duplicated unit' issues in LoW
 ### Miscellaneous and bugfixes
   * Scrollbar containers now use the button super class
   * Allow a gui2 timer delete itself in its callback
   * Fix various bugs when a unit has more MP than its maximum
   * Fix not redrawing a grid when set to hidden
   * Add helper functions to show/hide rows in a listbox

## Version 1.7.10-1.8beta3
 ### Campaigns
   * Under the Burning Suns:
     * Fix locations of some items in "In the domain of the dwarves"
       (bug #14925)
 ### Graphics
   * Added attack icon for Drake Ram attack, and for UtBS's Giant Ant.
 ### Language and i18n
   * Updated translations: French, German, Hungarian, Italian, Latvian,
     Lithuanian, Polish, Portuguese (Brazil), Russian, Serbian, Slovak
 ### User interface
   * Don't reserve space for scrollbars in message dialogs
   * Ctrl-f for fullscreen works again in the MP lobby (bug #14759)
   * Resizing the MP lobby no longer crashes randomly
 ### WML Engine
   * Fix bug #14859: [time_area] created by event are not saved
   * Allow modifications to change unit ellipse
 ### Miscellaneous and bugfixes
   * Add a minimap cache for gui2
   * Add a new super class for the button
   * Added a new gui2 timer engine
   * Add hotkey support for gui2
   * Change Drake Flare and Flameheart weapon names to match new weapons
   * Converted the hover tooltips to use the new timer engine
   * Fix bug #14865: move+attack into hex with ambusher causes crash
   * Fix the addon upload script to include the translate flag (patch #1387)
   * Fix the gui2 unit tests
   * MP lobby refresh to the new timer engine
   * Optimize AI recruitement and movement phases
   * Reduced header dependencies
   * Remove spurious hover error messages
   * Shorter "Initializing Display" phase when staying in same campaign/MP

## Version 1.7.9-beta2
 ### AI
   * new [limit] subtag of [value] of ai_default::recruitment implementation of
     recruitment aspect - allow easy limiting of number of concurrent recruits
     of specific type in the field
   * new rate_action formula_ai function which returns a rating of
     attack analysis.
   * values of most ai aspects are now readable from formula ai (aggression,
     avoid, attacks, attack_depth, caution, grouping, leader_aggression,
     leader_value, number_of_possible_recruits_to_force_recruit, passive_leader,
     passive_leader_shares_keep, recruitment_ignore_bad_movement,
     recruitment_ignore_bad_combat, recruitment_pattern,
     scout_village_targeting, support_villages, village_value,
     villages_per_scout)
   * Fixed Bug #14768: made AI observe changes in allowed recruits, preventing
     situations where AI does not recruit because it thinks that it can not do
     so.
 ### Campaigns
   * Northern Rebirth:
     * Fixed a few graphic bugs with map items
   * The Rise of Wesnoth:
     * Removed the undead trait from several custom bat units
 ### Graphics
   * New animations for the Chocobone.
   * Change which Orc Grunt portrait appears is the default.
   * New portrait for Orc Warrior
   * New portrait for Hamel (tHoT)
 ### Language and i18n
   * Updated translations: Czech, French, German, Italian, Latin, Lithuanian,
     Polish, Portuguese (Brazil), Russian, Serbian, Slovak.
 ### Multiplayer
   * Updated maps: Caves of the Basilisk, Hornshark Island, Howling Ghost
     Badlands, Sablestone Delta
 ### Music and sound effects
   * Updated music tracks: Legends of the North, Breaking the Chains
 ### User interface
   * Show selected item after a listbox resize (bug #13995)
   * Increasing the size of the MP lobby works properly (bug #14759)
   * Fix waypoints ignored for multi-turns moves
   * Toggle waypoint now works for each one, not just the last.
   * Reclick on the selected unit now clear all waypoints
   * Logo coordinates on the title screen now relative to center of the logo.
 ### WML Engine
   * Added [open_help], fixes Bug #11061 (forgot to commit that one long time
     ago)
 ### Miscellaneous and bugfixes
   * Optimize "Initializing teams" loading phase
   * Undraw floating labels when a gui2 dialog closes (bug #14816)

## Version 1.7.8-beta1
 ### Campaigns
   * Under the Burning Suns:
     * Do not allow player units to get pass the Dwarf Ghost
       without completing the side quest in Tunnels of the Trolls
       (bug #14379)
     * Fixed a few visual glitches in some scenarios
 ### Graphics
   * New base frames for Drake Flare, Flameheart
   * New portraits for Grand Knight (alt), Lancer, Orc Grunt (two alts)
 ### Language and i18n
   * Updated translations: Czech, Dutch, Finnish, German, Hungarian, Italian,
     Latin, Lithuanian, Portuguese (Brazil), Russian
 ### User interface
   * Instead of "crashing" upon invalid markup try to show the raw text
   * Found a better fix for truncating the campaign description (bug #14328)
   * Fix storyscreen buttons occasionally disappearing (bug #13779)
   * Fix not being able to cancel the different version message (bug #14438)
   * Fix not being able to close corrupted file dialog (bug #13764, #14058)
   * Improve display order of unit healing (patch #1343)
   * Fix the new-widgets addon list dialog
   * Add fallback scrollbars if a window doesn't fit (bug #13180)
   * Switch to the new MP lobby
   * Fix a NULL pointer deferring in the hover code
   * Protect against widgets being smaller as expected causing images with
     negative sizes (bug #14525)
 ### WML engine
   * Added two array element lookup macros, LOOKUP_INDEX and LOOKUP_VALUE
   * Event "turn refresh" is now fired at turn 1 too
 ### Miscellaneous and bugfixes
   * Using a hotkey to reload during an attack no longer disables the mouse
     (https://www.wesnoth.org/forum/viewtopic.php?f=4&t=27616)
   * Removed some unused Drake macros from animation_utils
   * Add recruitment anims for the Sky and Hurricane Drakes
   * Removed the old stats code (Debian bug #555276, CVE-2007-2383,
     CVE-2008-7720)
   * undo+redo a multi-turn move now restores the assigned destination

## Version 1.7.7
 ### AI
   * Filtering of allowed attackers/defenders in 'attacks' aspect.
   * Fix a serious enough bug in default ai targeting. The bug caused the AI,
     in certain situations, to make weird shuffle-doing-nothing moves and not
     seek enemies or their villages
   * Optimize AI targeting phase.
 ### Language and i18n
   * New translations: Shavian
   * Updated translations: Czech, Dutch, Finnish, Estonian, German, Hungarian,
     Italian, Lithuanian, Russian, Serbian, Slovak, Spanish.
   * Fix a broken markup in the Italian translation (bug #14506)
 ### Multiplayer
   * Updated map: The Manzivan Traps
 ### Units
   * Removed Elder Wose and Shock Trooper from random_leader of the default era.
   * Removed Ancient Wose and Iron Mauler from random_leader of the AoH era.
   * New animation WML and macros for the Drakes
   * Increased the XP required to advance for the Orcish Assassin from 30 to 34.
   * Changed the cold resistance of the naga line from -20% to 0%.
 ### User interface
   * Enabled the new event handler by default now
   * Allow markup in a campaign description (bug #14435)
   * Escape no longer closes the new title screen
   * In recall dialog, use different color and brightness for XP and level
   * In unit list, display invisibility status and change order of columns
   * Update the mouse position after closing a dialog, so the dialog below
     'knows' the proper mouse location
   * Fix double click events to be send to the wrong window
   * Fix scrollbars to show up when not needed (bug #13996)
   * Avoid truncating the last line in campaign description (bug #14328)
 ### WML engine
   * Added 'side Y turn' and 'side Y turn X' events
   * Make status "hidden=yes" default, no need to do it manually in WML anymore
 ### Miscellaneous and bugfixes
   * Add-ons download list now takes filtering into account for
     displaying descriptions
   * Various code cleanups
   * Fixed a lot of issues found by cppcheck.
   * Fixed "Revert all changes" feature in the map editor (bug #14266)
   * Fixed broken illumination overlay in ToD sidebar image
   * gui2 dialogs are now shown in an exception safe way
   * Improve cmake po-update so it doesn't add too many dummy updates
   * po-update2-xx now properly updates all domains
   * Fixed sometimes incorrect update of unit/savegame view when using filter
   * Debug-created units now also have traits and name
   * Fixed broken ambush/invisibility at first turn or at unit creation
   * Fixed ambushing unit not directly visible when discovered
   * Fixed time tooltips (bug #13886)
   * Fixed halo render glitches (bug #14405)
   * Renamed the cmake foo2 targets to foo

## Version 1.7.6
 ### AI
   * Stabilized syntax of [modify_ai] tag
   * Reorganized AI macro library
   * Added 'move leader to target' candidate action
   * Added recall capabilities to default ai.
   * Added recall formula ai function and ai.recall_list formula ai attribute.
   * Modified default ai recruitment to recall good units from recall list.
   * Modified default ai recruitment to consider enemy potential recruits
     during recruitment.
   * Added the capability to control leader goal of allied leaders.
     Enabled this for certain LoW scenarios.
 ### Animations
   * new animations to help drakes to take off and land : pre-movement,
     post-movement, draw_weapon, sheath_weapon
 ### Formula AI
   * New 'reduce()' formula function
 ### Graphics
   * New portraits for Merman Spearman, Bat, Merman Netcaster, Nightgaunt,
     Spectre, Shadow
   * Added a couple of missing frames for the Inferno Drake
   * Show HP/XP bars during leveling animation.
   * When a unit reach a new level, a floating label indicates it.
 ### Language and i18n
   * Updated translations: Czech, Dutch, Estonian, Finnish, French, German,
     Italian, Lithuanian, Russian, Serbian, Spanish.
   * New translations: Vietnamese
 ### Multiplayer
   * New map: 4p Ruins of Terra-Dwelve
 ### Music and sound effects
   * Added new music track, "Into the Shadows" by Tyler Johnson
   * Fixed bug #14239 (check_fogged ignored by sound sources)
   * Implemented FR #14246 (check_shrouded= for sound sources)
 ### Units
   * Made units with the 'healthy' trait always rest heal but take normal
     damage from poison.
   * Changed the Drake Glider movetype to give 40% defense almost everywhere.
   * Gave the Fire Dragon 100% fire resistance.
   * Updated the descriptions for the Drake Fighter, Glider and Burner lines.
 ### User interface
   * new gamestate inspector debug dialog (via 'inspect' command
     and '[inspect]' tag)
   * Rename easy_close to click_dismiss
   * Automatically try to resolve blocked multi-turn moves
   * Better visually differentiate name, type and race in sidebar
   * Add colorized terrain defense info in sidebar
   * In attack dialog, split damages and chance to hit and color the later.
   * The Remove and Update add-ons options are disabled when there are no
     add-ons installed
   * Add a Description button to the add-ons downloader
   * For move+attack mouse click, now show the attack dialog before the move.
   * In sidebar, add current bonus/malus info from alignement.
   * New option to enable/disable move interruption when an ally is sighted
 ### WML engine
   * Fix silent=yes for objectives
   * Allow [story] [part] blocks to specify the title box alignment
     with title_alignment=
   * Implemented FR #14246 (visible_in_shroud= for [label])
   * Modified Lua handling of action handlers and WML objects.
   * Added a lua_function= attribute to standard unit filters.
 ### Miscellaneous and bugfixes
   * Fix broken "Skip Ai moves" option.
   * Changed upload log format and defaulted the new uploader.
   * Fix bug #13268 (corrupted replays due to undo of recall/dismiss)
   * Weapon name change for Drake Enforcer.
   * Fixed the unit tests from 'hanging'
   * Fix bug #14160 (carryover percentage in [endlevel] ignored
     when victory_when_enemies_defeated="yes")
   * Various cleanups to the gui2 code
   * Remove position info from unit list
   * Avoid an assertion failure if haloes are disabled (bug #14297)
   * Fixed linked widgets not getting deregistered upon destruction
   * Fix huge fonts on storyscreens with tiny GUI
   * Fix cmake po-update not doing line wrapping properly
   * Fix bug #14241 (mp campaign timer settings not carried over to
     next scenario)
   * Started with the a new event handler for gui2
   * Fix unit facings after moving (bug #14336)
   * Fix unit facings after undo/redo
   * Improved the teamcoloring script for images.

## Version 1.7.5
 ### Campaigns
   * Legend of Wesmere
     * Scenario 21 redesigned
 ### Graphics
   * New base frame and animations for the Drake Blademaster.
 ### Language and i18n
   * Setup for tracking localized images.
   * Updated translations: Chinese (Traditional), Lithuanian, Serbian.
 ### Units
   * Fixed problems with the Drakes introduced in 1.7.4 (wrong resistances and
     a possible crash when advancing to the Inferno Drake)
 ### Miscellaneous and bugfixes
   * Optimized the cmake building if both game and tests are enabled
   * Switched to new stats upload mechanism so that stats.wesnoth.org should
     soon provide usefull data
   * Optimize pathfinding on 1MP terrains.

## Version 1.7.4
 ### AI
   * Formula AI debugger (uses -new-widgets)
   * New 'debug()' formula function
   * Fixed crashes and infinite loops on AI turn
 ### Animations
   * Movement have the number of steps done in value and the number of step left
     in value_second, this allows take-off and landing animations
 ### Campaigns
   * Legend of Wesmere
     * Scenario 3 redesigned
     * Fixed wrong or unclear scenario objectives
     * Scenario 16: Reduced the number of wolf riders
 ### Editor
   * Better support for conflicting terrain letters across add-ons, now
     in the event of a conflict the terrain will appear in all terrain
     groups as opposed to appearing in one of them multiple times.
   * Added remembering of the show terrain codes and coordinates options
 ### Graphics
   * New portraits for Ancient Wose, Ruffian, Master-at-arms, Naga
     Warrior/Myrmidon, Grand Knight, Merman Hunter.
   * Updates to Peasant, Spearman and Swordsman portraits.
   * New melee animation for Thunderguard, Dragonguard.
   * New base frame and animations for Inferno Drake, Fire Drake.
 ### Language and i18n
   * Updated translations: Chinese (Traditional), German, Lithuanian, Russian,
     Serbian
 ### Music and sound
   * "Journey's End" from Mattias Westlund
   * "Over the Northern Mountains" from Mattias Westlund
   * Added horn signal sound effects
 ### Units
   * New weapon names for a number of drake attacks to account for changes to
     the sprites.
 ### User interface
   * Removed the hotkey to enable/disable mouse scrolling
   * Added a horizontal listbox
   * New basic support for specifying waypoints (via a new hotkey 'w')
 ### WML Engine
   * Support for [show_if] inside [message]
   * Added ability to change the share_maps team attribute using [modify_side]
     tag. Be sure to use shroud=yes for that side
   * [side] team_name is now a comma-separated list of teams the side is on
   * [modify_side] user_team_name no longer requires that team_name also be
     modified
 ### Miscellaneous and bugfixes
   * Removed obsolete code for implicit linked widgets for the listbox
   * [part] caption= is no longer supported; prepend the CAPTION macro
     to story text instead
   * Fixed 100% CPU usage on storyscreens with no text (e.g. map screens)
   * Fix ODR issues in gui2
   * Remove the boost 1.33 code in the unit tests
   * Fix regression about broken undo after a multi-turn ("goto") move
   * Fix regression about pathfinding poorly using the teleport ability
   * Added extra wiki comment and updated the extractor
 ### Animations
   * Movement have the number of steps done in value and the number of step left in value_second.
   * new animations pre_movement_anim and post_movement_anim to allow take-off and landing animation

## Version 1.7.3
 ### AI
   * New AI configuration syntax
 ### Campaigns
   * Two Brothers
     * Replaced campaign specific portraits with mainline portraits
   * Under the Burning Suns
     * Fixed bug #13978: UtBS: "stun" ability doesn't modify enemy ZoC
   * Legend of Wesmere
     * Story only scenarios now display the speaker's name in the title.
     * Ambient sound effects.
     * Redone the campaign's objectives.
     * Added the "Dwarfish Scout" to the player's recruit list.
     * Scenario 19 now requires the player to destroy Saurgrath.
     * Scenario 14 redesigned.
     * Scenario 23 implemented Santi's plan to let half the army defect.
     * Map updates (mostly new alias terrains added).
     * Replaced the campaign image with a transparent one (Thanks to Kitty).
 ### Editor
   * Renamed editor2 to editor pretty much everywhere
 ### Graphics
   * New portrait for the Duelist, Mermaid Enchantress/Siren,
     Priestess/Diviner, Merman Fighter/Warrior, Fencer, Drake
     Glider, Merman Hoplite, Goblin Impaler, Rouser, Merman Triton,
     Cavalier, Direwolf Rider, Paladin, Deathblade, Thug, Bandit,
     alternate Goblin Rouser, Revenant, Naga Fighter, Troll.
   * New unit graphics and animations for the Troll Hero, Drake Glider, Sky
     Drake, Hurricane Drake, Drake Burner, Drake Fighter, Drake Warrior.
 ### Language and i18n
   * Updated translations: Czech, Dutch, Estonian, French, German, Hungarian,
     Lithuanian, Polish, Slovak
   * Updated Drake Clasher, Arbiter and Slasher descriptions.
 ### Multiplayer
   * Added a /q alias for /query
   * New lobby interface in gui2, testable work in progress available with
     --new-widgets. There are known issues with window resizing and low
     resolution support.
   * Server-side generation of random numbers for MP (combats)
   * No MP compatibility with earlier clients and wesnothd
   * --no-srng command line switch is available to turn off server RNG
     support (and rquirement) in a client. Only games played with clients
     pre-srng or with srng disabled as well can work without OOS. This is a
     testing feature that will likely be removed prior to 1.8.
   * Room support via /join and /room commands, better support in the
     experimental new lobby. See https://www.wesnoth.org/wiki/MultiplayerRooms
     for details.
 ### Multiplayer server
   * Added server-side RNG support. Old clients can still play as normal.
   * Added room support to the server.
 ### Unit balancing
   * Added Dwarvish Stalwart, Elder Wose, Shock Trooper and White Mage to
     random_leader of the default era.
   * Added Ancient Wose, Dwarvish Sentinel, Iron Mauler and Mage of Light to
     random_leader of the AoH era.
 ### Unit renames
   * Drake Gladiator -> Drake Thrasher
   * Drake Slasher -> Drake Arbiter
 ### WML Engine
   * return 0 for length of arrays inside nonexistent containers (bug #13734)
   * [end_turn] now waits for the event to finish before ending the turn
   * it is now possible to use Pango markup in storyscreen text
   * it is now possible to specify a custom title= in [story] [part]s
   * [story] [part]s' text placement may be changed with text_layout=; it
     currently supports "top", "middle" and "bottom" (default)
   * [story] [part]s accept the sound= attribute for a list of sound files
     from which a single one is chosen and played once
   * [story] [part]s accept a caption= attribute as a text header
   * new tags [store_unit_type] and [store_unit_type_ids] were added
   * allow direct WML unit modification of "halo" attribute
 ### Miscellaneous and bugfixes
   * [music], [sound], [sound_source] and [label] should work again on
     WML prestart events; this means most music playlists should work again
     (bug #14039)
   * 'Recruit' and 'recall' don't appear in context menus unless the
     leader is on a keep. (bug #13856 and #13855)
   * No longer "crash" upon invalid pango markup
   * It's now possible to display the area a widget uses, for debug purposes
   * The widget in a stacked widget now grows properly
   * Merged code for FDO and KDE desktop notification; only dbus is needed
   * Added --screenshot parameter to take map screenshots without
     initializing a GUI.
   * Added --new-uploader parameter for testing the new experimental log
     uploader.
   * Fix gcc 4.4 compilation errors (Debian bug #539546)
   * Added a work-around a listbox assertion failure in the new MP lobby

## Version 1.7.2
 ### Campaigns
   * Son of the Black Eye
     * changed Orcish Shaman movetype from smallfoot to orcishfoot
 * Graphics:
    * New portrait for male and female Footpad, male and female Outlaw,
      Horseman, Drake Clasher, Goblin Wolf Rider, Goblin Pillager, Dwarf
      Explorer, Dwarf Scout, Cavalryman, Skeleton.
    * New unit graphics and animations for the Dwarvish Scout, Pathfinder,
      and Explorer; Drake Clasher, Slasher, Warden, Gladiator, Enforcer.
 ### Language and i18n
   * Updated translations: Czech, Dutch, Estonian, Finnish, French, German,
     Italian, Lithuanian, Polish, Russian, Serbian, Slovak
 ### Terrains
   * New terrain: drake village
 ### User interface
   * Add an icon to show whether or not the user finished a campaign
   * Fixed bug #13831: Bug with team labels
   * Added a new experimental stacked widget widget
   * Added support for Growl notifications
   * Added support for desktop notifications using KDE's
     org.kde.VisualNotifications DBus service
   * Changed "Toggle Full Screen" button to say "Full Screen" (bug #13909)
   * Added cost of units recruited, recalled, killed, and lost, to the
     statistics window (patch #1190)
   * Fixed bug #13626: Bug about no option to turn on/off save delete
     confimation. (patch #1192)
   * Allow [advanced_preference]s to be of the 'int' type, appearing as a
     slider
   * Add better configurable linked widgets in the new gui
   * Allow to make items in a listbox invisible (needed for filtering)
   * Initial new loby user interface, available for testing with --new-widgets
 ### Miscellaneous and bugfixes
   * Fixed language switch not affecting unit descriptions (bug #13827)
   * Fixed teleporting to impassable terrain (bug #13795)
   * Fixed an issue where teleporting could leave a unit halo
   * Fixed [set_variable]'s rand and random when operating on 0..0 (return 0)
   * Allow items to be removed from a listbox
   * Fixed objective dialog not recognizing markup (bug #13888)
   * Fix minimap no clipping in the new wiggets
   * Added preprocessor macros to the wiki grabber tool

## Version 1.7.1
 ### AI
   * Reworked AI code to allow easier creation of AI components.
   * New AI: Composite AI
   * (Optional) new AI configuration syntax.
   * Basic reimplementation of an old C++ AI as a 'candidate action'-based composite AI
   * Autodiscovery of available AI configurations from data/ai/ais.
   * (In debug mode) Autodiscovery of available AI configurations from data/ai/dev
 ### Campaigns
   * Delfador's Memoirs: new portrait for Lionel.
   * The Hammer of Thursagan: new portraits for Angarthing, Ratheln.
 ### Graphics
   * New portrait for Knight, Ghost.
 ### Language and i18n
   * Updated translations: Chinese (Traditional), Czech, Finnish, French,
     German, Greek, Hungarian, Icelandic, Lithuanian, Polish, Russian,
     Serbian, Turkish
 ### User interface
   * Removed the old obsolete layout algorithm in the new widgets
   * Added unit tests for the new widgets
   * Improved the layout algorithm not to show scrollbars when they make the
     situation worse
   * Add a new transient message dialog
   * Add a new multi page widget
   * Increase the lineheight in the new dialogs
   * Add a blur precommit function to the canvas
   * Improved the redraw algoritm and added more asserts
   * Objectives now use pango markup
   * Replace the campaign dialog with a new gui one (debian bug #497655)
   * Removed the hidden option to disable the tips of the day
   * A click on a slider now properly sets the position
   * WML generated messages, labels and sounds are skiped during replay (bug #13519)
   * Added support for desktop notifications (GTK/libnotify only for now, patch #1179)
   * Added a hotkey to toggle team ellipses (fr #7763)
 ### WML Engine
   * Made new turn, turn X, side turn and turn refresh events synchronous.
     (bug #10603)
   * Petrified units no longer heal. (bug #13513)
 ### Miscellaneous and bugfixes
   * Add strict compilation to cmake
   * Let cmake also use the CXXFLAGS and CFLAGS environment variables
   * Fixed a segmentation fault with storyscreens using [if] (bug #35959)
   * Properly translate dialog title (bug #13761)
   * Fixed a problem with easy close that caused buttons to be ignored
   * Properly translate unit name and mark strings translatable (bug #13751)
   * Properly redraw logo on the loading screen (bug #13758)
   * Set window title before setting video mode (bug #13756)
   * Fixed dates in the load box being untranslated (bug #13782)
   * Fixed networking code regression that could lead to "Client disconnected"
     errors when creating a MP game
   * Minimum version for Boost is now 1.35

## Version 1.7.0
 ### AI
   * Added command-line option ai_config<number>=<value>
   * Fixed incorrect handling of poisoning attacks when suggesting best attack
     in user interface
   * Added basic history and hot-redeployment capabilities to in-game console.
   * Changed AI Lifecycle handling. Console AI is now persistent between
     invocations.
   * Added AI Arena test map to test AIs in interactive mode ( ai_arena_small )
   * Changed interaction between default AI and Formula AI - made default AI
     fallback to formula recruitment if "recruitment" config option is set in
     AI config.
 ### Campaigns
   * Delfador's Memoirs:
     * New campaign added to mainline from the Wesnoth-UMC-Dev repository.
       (Novice level, 24 scenarios)
   * Eastern Invasion:
     * Fixed the liches not being revealed properly in 'Weldyn Besieged'
   * Heir to the Throne:
     * Made snow fall gradually every turn in 'Northern Winter'.
     * Enforce a 7-hex starting castle to prevent units from appearing inside
       a wall in 'The Scepter of Fire' (bug #13377).
     * Made the number of enemies to defeat depend on the difficulty in
       'Test of the Clan'.
   * Under the Burning Suns:
     * Kaleh advancement:
       * Gave the Hero variation more HP.
       * Reduced XP requirements for the Captain variation.
       * Disabled the sword2 (and thus armor) AMLA for the Captain variation.
       * Enabled the bolas AMLA for all variations.
       * Added the camouflage AMLA back.
 ### Editor2
   * New feature: exporting of selection coordinates to system clipboard
   * Made auto terrain transition mode tri-state: on (editor2's on), partial
     (1.4 editor's on / editor2's off) and off (1.4's off).
   * Made "wesnoth -e FILE" work like "wesnoth -e --load FILE" (load the map)
   * Made "wesnoth -e --load DIR" (and consequently "wesnoth -e DIR") set
     DIR as the default map directory for the current session, and start with
     the map load dialog open
   * Added ability to load maps referenced in scenario files
   * Added ability to work with maps embedded in scenario files (saving works
     in a limited scope)
   * Moved the clipboard actions to a context menu available in paste mode
   * Added a terrain sampler feature -- ctrl+click when in paint or fill mode
 ### FormulaAI
   * Fixed bug #13230: added debug_float FormulaAI function to allow debugging
     via floating popups on the specified hex
   * Added run_file FormulaAI function to allow running .fai scripts directly
     from in-game console
   * Fixed bug #13295: made enemy_units formula return only those units which
     are not incapacitated (for example, it now ignores petrified units )
   * Implemented FR #13348: added timeofday_modifier function to formula AI
   * Added attacks_left attribute to unit callable
   * New variable: my_attacks with all possible attacks
   * Added a new type of candidate move : strategic
   * Added suitable_keep FormulaAI function to allow easier selection of
     suitable keep for leader
 ### Graphics
   * New type of animation : "recruiting" used by leaders when recruiting
     units
   * New portrait for Orc Grunt, Dwarf Fighter (alternative), Goblin Spearman,
     Ogre/Young Ogre, Trapper, Ranger, Huntsman
   * Image path functors:
     * New functor, ~NOP(), which does nothing.
     * The ~RC() image functor does not accept the special palette switch
       ~RC(palette1=palette2) syntax anymore. ~PAL(palette1>palette2) should
       be used instead
 ### Language and i18n
   * New translations: Icelandic
   * Updated translations: British English, Catalan, Chinese (Simplified),
     Chinese (Traditional), Czech, Dutch, German, Finnish, French, Hebrew,
     Hungarian, Indonesian, Italian, Lithuanian, Norwegian, Polish, Russian,
     Serbian, Slovak, Swedish, Turkish
   * Updated CJK character range
   * Fixed word wrapping in CJK languages (patch #1140 from sylecn)
   * Remove female^ descriptions of Outlaw, Footpad, Fugitive, Dark Adept
   * The term 'stoned' has been replaced with 'petrified'; also the
     related verb.
 ### Multiplayer server
   * Implemented automatic saving of game replays.
   * Implemented the adminmsg command to allow players to send messages to
     currently available admins. (FR #9218)
   * Fixed bug #7547: Add possibility to unban/unmute in a multiplayer game
 ### Savegames
   * Providing a new simpler interface for dealing with savegames
 ### Terrains
   * New terrains: orcish fort, orcish village
 ### User interface
   * Fixed bug #13257: Attack dialog always uses the active name of a weapon
     special
   * Fix missing faction column when waiting that the host start the game,
     plus some other unwanted changes there (bug #13243)
   * Make delayed shroud updates and scenarion objectives to change and
     show information for viewing team. (help in bug #13313)
   * Making the game return to the add-on install dialog just before
     installing one, and with its entry selected
   * Implemented FR #13321: added "unit advances=N" command to debug console
     to immediately level-up selected unit N times
   * Improved the sorting of the XP and traits columns in the unit list dialog
     (part of bug #13360)
   * New console command ":discover" make the help considers that you
     discovered all the units of the current campaign/era
   * Debug action "Change unit side" allows to change owner of empty village
     and is now renamed "Change side".
   * Debug actions "Create unit" and "Change side" used on unit in a village
     now causes a capture of that village.
   * Rewrote the layout algoritm (still a work in progress)
   * Fixed an multi-character UTF-8 handling bug in the password textbox
   * Changed the tmessage dialog to be able to show four buttons and added
     an extra helper function to show the dialog.
   * Add a "Factions" section in help. Only show current era's informations.
 ### WML Engine
   * Added [show_objectives] tag and allowed [show_if] tag in [objective]
     tags. (bug #13042)
   * Made moveto events set $x2,$y2 to the source hex. (bug #13140)
   * Story screen and parts allow using [switch], [if], [wml_message]
     and [deprecated_message] consistently at [part] and [story] levels.
     (feature/bugs #13170, #13290)
   * Added a "preload" WML event type.
   * Added ability to change the share_view team attribute using [modify_side]
     tag. Be sure to use fog=yes for that side  (bug #13320)
   * Added support for Lua scripts in WML event actions ([lua] tag)
   * [modify_side] can now change the share_view attribute
   * new tag [hide_help] in [units] allows to hide units in help when you can't
     change these unit types (main use: hide mainline units).
   * [stone] and [unstone] have been replaced with [petrify] and [unpetrify];
     the units' "stoned" status variable has been renamed to "petrified"
   * Added the 'sub' key to [set_variable] to subtract from the variable.
   * [era] and [multiplayer_side] now allow a 'description' key, and show it
     in help pages (but currently empty)
   * Filter on attacks now uses a 'damage' key, also support range like 1-9.
   * Attacks with damage=0 are now correctly supported.
 ### Miscellaneous and bugfixes
   * Added --ignore-fatal-errors option to wmlunits
   * Added --rng-seed command line option to specify a value to seed the random
     number generator with
   * Changes to logging:
     * New logdomain : uploader to see stat-upload related actions
     * The --log-domain command-line option now allows for a bit of
       pattern-matching, e.g., "--log-info=ai/*"
   * Fixed compilation with -D_GLIBCXX_PARALLEL
   * Fixed handling of floating-point WML constants on localized Windows
     (impacts "submerge" animations and AI settings)
   * Fixed control transfer after player leaves MP game (bug #13238)
   * Fixed a bug with auto-stored WML variables not being restored correctly
   * Fixed missing unit graphics when loading a start-of-scenario savegame.
   * Improved random village naming
   * The add-ons directory, <preferences>/data/campaigns, has been renamed
     and it is now <preferences>/data/add-ons
   * Enabled hinting for texts displayed by Pango/Cairo (bug #13399)
   * Enabled non-default fonts for Pango/Cairo (bug #13399)
   * Made dismissing of recallable units undoable
   * Sped up pathfinding (and therefore AI)
   * fps info used in debug mode show info about number of redrawn hexes.

## Version 1.6a
 ### User interface
   * Remove a no longer valid assert (bug #13217)
   * Make sure the dialog always uses the full map area width
 ### Multiplayer and AI
   * Fixed bug #13222: Do not use the FormulaAI by default since it could lead
     to the AI doing nothing
   * Fixed bug #13218: infinite loop in the AI if it persistently tries to move
     unit to occupied square
   * Moved FormulaAI related debug output into the info log level

## Version 1.6
 ### Campaigns
   * Under the Burning Suns:
     * All portraits now have a transparent background (bug #13135)
 ### Graphics
   * New or updated unit graphics for the Mermaid Priestess and Enchantress
     unit lines
 ### Language and i18n
   * updated translations: Finnish, French, German, Lithuanian, Polish,
     Portuguese (Brazil), Russian, Slovak, Spanish
 ### User interface
   * Grey game titles out when we're missing the era
   * Fix a dialog size problem returning invalid sizes (bug #13203)
 ### WML Engine
   * Increase the map size limit to 1000 by 1000
   * Added an 'always_display' key to [advancement] to make it possible to show
     the advance dialog even with just one option.
   * weapon filters now recognize the id= attribute of specials (bug #13193)
 ### Miscellaneous and bug fixes
   * Fixed bug #13204: NR: Death event doesn't re-spawn Malifor as expected
   * Fixed bug #13198: Corrupt replay in MP
   * Fixed bug #13199: Map generation in mp fails when hills and size of hills
     sliders are at max
   * Fixed bug #13179: Unit's move have sometimes a jumpy start
   * Stop resetting the alternate default zoom level after each reload

## Version 1.5.14
 ### Campaigns
   * Descent into Darkness:
     * Increased the experience requirement for the Ancient Lich from 150 to 250
   * The South Guard:
     * Made Deoran not dismount in 'Into the Depths' anymore
   * Under the Burning Suns:
     * Simplified Kaleh's AMLAs to have consistent requirements and effects.
 ### Graphics
   * New portrait for the male and female Assassin, Gryphon Rider, Longbowman,
     Master Bowman, Dwarf Runemaster.
   * New variations of desert plants
 ### Language and i18n
   * updated translations: Catalan, Chinese (Simplified), Czech, Dutch, Finnish,
     German, Hebrew, Hungarian, Lithuanian, Norwegian, Polish, Russian, Slovak
   * updated DejaVuSans to 2.29
   * replaced sazanami-gothic.ttf and wqy-zenhei-gb2312.ttf with wqy-zenhei.ttc
     (version 0.8.38-1)
 ### Music and sound effects
   * Added the new music track "Siege of Laurelmor" by Doug Kaufman
 ### User interface
   * Fix an assertion failure when an unexpected mouse button was used
     (bug #13126)
   * Fixed bug #13161: Inactive weapon special name and description not used
   * Fixed bug #13123: When you're downloading add-ons they do not appear in
     the campaigns list until BfW is restarted
   * The size of the message dialogs now really depends on the size of the map
   * Started with a new layout engine for the new widgets (bug #13180)
   * Fix few bugs about selecting an unit during another unit's move
   * Fix a regression: again allow to use right-click to cancel drag&drop
 ### WML Engine
   * Increase the map size limit to 1000 by 1000
 ### Miscellaneous and bug fixes
   * Fix another campaign replay bug (#13139)
   * Fix AI bug #13165 (leader too aggressive)
   * Fix WML [advancefrom] bug (#13176)
   * It is now possible to create units with random genders in debug mode
   * Reduce the fontconfig dependency to 2.4.1

## Version 1.5.13
 ### Graphics
   * New portrait for the Woodsman, Bowman, male Thief
 ### Language and i18n
   * updated translations: Chinese (Simplified), Czech, French, Finnish,
     Hungarian, Polish, Russian, Slovak, Turkish
 ### Multiplayer
   * The engine now send the exact path used by units, preventing visual
     differences between clients.
 ### User interface
   * Fixed a crash when trying to use scrollbars in an invisible widget
   * Fixed the setting of the ellipse_mode in ttext
   * Improve the showing of ellipses in the textbox (bug #13083)
   * Fix bad anti-aliasing of in-game MP chat (bug #12938)
 ### Miscellaneous and bug fixes
   * Fix another savegame cache corruption
   * Fixed bug #13099: MP lobby player list becomes inaccurate over time
   * Fix bug #13118: OOS/replay bug when moving near ambushed units
   * Better fog update after "slow" and "stone" attacks
   * Updated Doxyfile to 1.5.6 format
   * Fix regression about missing sighted event triggered by AI in replay
   * Renamed some [ai] attributes to use correct English spelling.
     Support for their misspelled forms will be removed in 1.7.0.
     * simple_targetting -> simple_targeting
     * scout_village_targetting -> scout_village_targeting

## Version 1.5.12
 ### Add-on server
   * Ignore case on add-on (file)names (bug #13080)
   * Disallow uploading of add-ons with no title, type, author, version or
     description specified and warn about invalid versions
 ### Graphics
   * New portrait for the female Thief
   * New tiles for summer and fall deciduous and mixed forests
   * Deciduous and mixed forests now used in several campaigns (not all, yet)
   * New tiles for dirt
   * Fixed a lot of small terrain transition glitches
 ### Language and i18n
   * updated translations: Chinese (Simplified), Czech, French, German, Italian,
     Polish, Portuguese (Brazil), Spanish, Turkish
 ### Multiplayer
   * Updated maps: Howling Ghost Badlands, Sullas Ruins, Isar's Cross
 ### Music and sound effects
   * Fix a regression which caused the endlevel music to be played even when
     skipping linger mode (e.g. result=continue or continue_no_save).
 ### Miscellaneous and bug fixes
   * Removed last binaryWML references by making the save_index gzip
     compressed. (We can still receive binaryWML via the network.)
 ### User interface
   * Listbox columns have the same width again
   * Fix false "click" sometimes triggered at end of unit move (bug #12712)
   * Now clear fog/shroud before an attack when doing a move+attack action
   * Fix broken auto-undo when trying several move+attack with the same unit
   * Changed the widgets in the in game message dialog
   * The multiplayer dialog shows tooltips again
   * The textbox history now uses CTRL+TAB and CTRL+SHIFT+TAB
   * Properly set the cursor when opening a dialog (bug #12961)
   * Fixed bug #13029: Problem with mouse-over unit identification in replays
   * Added experimental campaign selection dialog (Only available when
     starting with --new-widgets.)
   * Properly reset the scrollbar mode when resizing (bug #13018)
   * Fix unwanted double-clicks (caused by 1-pixel drag&drop)
 ### WML Engine
   * Fix incorrect or doubled "sighted" events when delaying shroud update
   * Fix sometimes missing or doubled "select" events.
   * Fixed bug #13090: Make movement_costs < 1 behave like movement_costs = 1
 ### Miscellaneous and bug fixes
   * Align all network buffers on 4 bytes
   * Partial fix for bug #13092: avoid a case of invalid iterator usage
     in FormulaAI
   * Fix savegame cache corruption (bug #12815/Debian bug #483782)

## Version 1.5.11
 ### Campaigns
   * Descent into Darkness:
     * Removed the custom python AI used in 'A Haunting in Winter'
 ### Campaign server
   * Reject add-ons with an empty type.
   * Give warning note when the add-on version is invalid.
 ### Editor2
   * Fixed / worked around several filebrowser issues (save/load dialogs)
     (bugs #13033, #12771, #12625)
   * Fixed terrain palette issue when the palette was not full (bug #13012)
 ### Graphics
   * New or updated unit graphics: Merman Fighter line, Merman Hunter line
   * New idle animations: Heavy Infantryman
   * New portrait for the Javelineer, Gryphon, Elvish Sylph.
   * Moved and renamed story screen files:
     * titlescreen/landscapebattlefield.jpg -> story/landscape-battlefield.jpg
     * titlescreen/landscapebridge.jpg -> story/landscape-bridge.jpg
     * titlescreen/landscapecastle.jpg -> story/landscape-castle.jpg
 ### Language and i18n
   * updated translations: Czech, French, German, Hebrew, Lithuanian, Polish,
     Russian, Spanish
   * Fixed a problem that could lead to wrong cache being used, leading to
     some strings not being shown as translated (bug #12568)
 ### Multiplayer
   * server:
     * Create the fifo group accessible (instead of only user accessible).
 ### Music and sound effects
   * Added a campfire sound file for use with [sound_source] in
     sounds/ambient/.
   * The game will no longer restart the music on a scenario change or game
     load if the currently played track is on the new scenario's playlist
 ### User interface
   * Fixed the assertion failure which could happen when clicking on a button.
     (bug #12927)
   * Allow chat command quoting as '/ /command'.
   * Fixed a bug which made listboxes the wrong size and caused visual
     glitches. (bug #12940)
   * Fixed colors in [message]. (bug #13019)
   * Made the maximum used width for the message text 650 pixels.
   * Added UI sounds to the new widgets. (bug #12748)
   * Fixed a glitch in the text drawing after showing markedup text.
     (bug #13023)
   * Fixed bug #13034: "/me" messages cut of first character in multiplayer
     game chat.
   * Fixed a glitch where the Gryphon wing was drawn over the fog and
     reachmap
   * When moving up or down in a listbox with a horizontal scrollbar, the
     listbox no longer changes the position of the horizontal scrollbar
   * Added experimental new addon download dialog (Only available when
     starting with --new-widgets.)
   * Fix a broken reference to the Myrmidon portrait.
   * Removed team_name checks for chat display.
 ### WML Engine
   * Fixed bug #13024: Conditional [allow_undo] not always working right
   * Map size is now hard-limited to 200 by 200
   * attribute hidden in [side] allows to hide a side from status table,
     it also can be changed by [modify_side] (FR #12814)
   * Removed support for PythonAI to handle CVE-2009-0367

## Version 1.5.10
 ### Campaigns
   * The South Guard:
     * Made 'Into the Depths' a bit easier
   * Under the Burning Suns:
     * fixed a major 1.5.7 regression with Kaleh's AMLAs, that made them
       useless after advancing him to L2 (rel. bug #12911)
     * fixed dehydration in scenario 2 triggering double-advancement for
       dehydrated units when leveling up (rel. bug #12911)
     * fixed some bad logic around the "Dust Devil dance" scene in scen.
       2.
     * made all human-controlled major characters unrenamable.
 ### Graphics
   * New or updated unit graphics: Elvish Enchantress, Orcish Slayer,
     Naga Fighter, Naga Myrmidon
   * New idle animations: Dwarvish Thunderer, Naga Fighter
   * New portrait for the Halberdier, dwarf Ulfserker line
   * Unit HP bar decrease progressively when unit gets hit
   * Rewrote the rendering engine which should fix the drawing order
 ### Language and i18n
   * new translations: Marathi
   * updated translations: British English, Dutch, French, German, Hungarian,
     Italian, Russian, Slovenian, Spanish, Swedish
 ### Multiplayer
   * Added support for observers to pause and continue the game at any time
   * Fixed bug #12896: Map generator does not sync between clients when
     advancing in MP campaigns
   * Updated maps: Clash, Cynsaun Battlefield, Den of Onis, Silverhead Crossing,
     Howling Ghost Badlands, The Wilderlands, Forest of Fear
   * Removed Necrophage from the Undead Random Leader List.
 ### Terrains
   * Enabled mixed deciduous-pine forest terrains
   * Forested hills variations now available for the deciduous forests
   * New merman village terrain variations
 ### User interface
   * Fixed an exception when certain characters weren't escaped
   * Removed the gamma correction option (code is only disabled not removed)
   * Fixed the assertion failure with long options (bug #12970)
 ### WML Engine
   * [replace_map] changes the map completely, changing the map size is
     posible - off-map units go into the recall list
 ### Miscellaneous and bug fixes
   * Fix bug #12946: [menu_item]/[command] losing function when called again
   * Fix flickering of units in the second part of the tutorial (bug #12923)
   * Fix high CPU usage in multiplayer lobby due to inefficient handling of
     friends and ignores lists.
   * AmigaOS4: Minimum stack cookie to prevent stack overflow (patch #1107)
   * Additional screenmodes when SDL can't guess them (patch #1108)
   * Python AI works again. (bug #12955, bug #12913)
   * Fixed the :query command to work while in a multiplayer game, not only
     when in chat (as /query)
   * Fixed invalid memory access with AMLAs (bug #12911)
   * Closing a message with escape caused the message to skip on the next
     execution of that event (unless it required input)

## Version 1.5.9
 ### Campaigns
   * Descent into Darkness
     * In 'Endless Night', the enemy's gold now increases faster
   * The South Guard
     * Made 'Tides of War' a bit easier
   * Sceptre of Fire
     * Fix the player's keep disappearing in 'Gathering Materials'
     * New graphics for the Dwarvish Miner
   * Under the Burning Suns:
     * New base frames for the Desert Sentinel and Prowler
 ### Graphics
   * Removed the black background from some remaining old portraits
   * New portrait for Dwarf Thunderer, Dwarf Dragonguard, Human Pikeman, Dwarf
     Fighter, Dwarf Lord, Dwarf Guard, Mermaid Initiate, Dwarf Sentinel
   * Image-path functions:
     * All image path functions except RC() and TC() can now be stacked
       correctly and the order of precedence is from left to right. This
       may impact performance slightly.
     * New function, SCALE(W,H): scale down an image to the specified width
       (W) and height (H) for rendering. Negative or null (zero) numbers are
       ignored when scaling in the respective direction
   * Restructured portraits directory to be based on unit types rather than
     authors, added an authors file for recording portrait credits
 ### Language and i18n
   * updated translations: Catalan, Chinese (Traditional), Czech, Dutch,
     French, German, Hungarian, Italian, Polish, Slovak, Spanish, Valencian
 ### Multiplayer
   * Made "fog of war" and "random start time" default to on.
   * Fixed the Default Era quick leaders not getting the -5% HP reduction
   * Made leaders with 4 MP receive the quick trait also in Age of Heroes
 ### Terrains
   * New summer, fall and winter deciduous forest terrains
 ### User interface
   * Hide the "Network Player" option for Local MP Games (bug #12596)
   * Portraits with an icon are TC'ed again
   * All [message] based dialogs are now converted to the new dialogs
 ### WML Engine
   * Prevent duplicate id conflicts when cloning units with WML (bug #12894)
   * [time_area], in EventWML, can now accept comma-separated lists of area ids
     for removing, but not for inserting; in the latter case, only the first id
     is considered to avoid stacking time_areas over a single location by
     accident
   * [fire_event] children tags [primary_unit] and [secondary_unit] now
     take a real Standard Unit Filter instead of a x,y coordinate pair
     When a filter matching multiple units is used, only the first matching
     unit will be taken for firing the event. Recall list filters and
     invalid locations are silently ignored
   * [set_variable] can now handle both integer and floating point values
     properly. The functions round, ipart and fpart were added (bug #12546)
 ### Miscellaneous and bug fixes
   * Fix clients not agreeing on remaining movepoints (bug #12467)
   * Fix summoning of other players recruit list on turn 1 (bug #12783)
   * Fix [item] overlays with visible_in_fog=yes magically becoming
     visible_in_fog=no (the former default for a missing attribute) when loading
     saved games - because the game was looking for a fogged= attribute
     instead on loading
   * Made [item] overlays always default to visible_in_fog=yes if not
     specified
   * Made [label] overlays always default to visible_in_fog=yes if not
     specified

## Version 1.5.8
 ### Campaigns
   * An Orcish Incursion
     * Converted to the new gold carryover system
   * Eastern Invasion
     * Fixed victory not triggering correctly in 'The Crossing'
     * Fixed missing time of day schedules in several scenarios
     * Fixed various problems in 'Northern Outpost'
   * The Hammer of Thursagan
     * Converted to the new gold carryover system.
   * The Rise of Wesnoth
     * Renamed "Lord Dionli" to "Lady Dionli".
   * Son of the Black Eye
     * Made scenarios 'Silent Forest' and 'Shan Taum the Smug' harder.
   * Legend of Wesmere
     * Fixed the bug with savegames in scenario 3
 ### Graphics
   * New portraits for Royal Guard, Heavy Infantry, Iron Mauler, Sergeant,
     Lieutenant, General, Grand Marshal with 1 alternate, Saurian Augur line,
     Saurian Skirmisher line, Troll Whelp, Death Knight
   * Fixed drawing glitches in the listboxes
   * Fixed several glitches with the new portrait dialog
   * Optimized animation and invalidation of idle/standing units
   * Removed the black background from most of the old portraits
 ### Language and i18n
   * Fixed a small bug that was causing the translations engine to look
     at badly mixed up paths when trying to locate UMC translations,
     rendering them unusable (bug #12872)
   * Fixed translation of 'A New Land' not being shown
   * updated translations: Chinese (Simplified), Czech, Danish, German,
     Hungarian, Italian, Japanese, Lithuanian, Russian, Slovak, Spanish
   * updated DejaVuSans to 2.28
 ### Multiplayer ui
   * Added a gui front-end to various commands, brought up by double clicking
     a player name, replacing the old whisper dialog
   * Automatically select the game the selected player is in
   * Try to preserve the selected player when the player list is updated
   * Added an option to avoid auto-scroll to enemy units during opponent's
     turn
   * Cleared the login dialog mess: Now there is a single dialog to display
     error messages from the server and to choose a different username and/or
     password
   * Added the possibility to provide a password in the main multiplayer dialog
   * Added an option to save the password to the preferences
 ### Networking
   * Ensure SDL_net is initialized before attempting to send upload_log
 ### Unit changes and balancing
   * Updated most units' defense and movement values on coastal reef
 ### User interface
   * Pressing backspace in a textbox with selection now clears the selection.
   * Scrollwheel mouses can scroll the new listboxes
   * New advanced preference to choose whether middle-click scrolls or warps
   * Fixed a glitch which closed a new dialog on the mouse up event of the old
     dialog (the effect is percieved as the new dialog not showing up)
   * Spacebar closes the dialogs without a scrollbar again
   * Pressing escape in a new dialog, behaves like in the old dialogs again
   * The new portrait dialogs can get higher so the scrollbars are less often
     visible
   * Added a password box that hides its input
   * The new portrait dialogs can now show the old images as well and also the
     right hand side images
 ### WML engine
   * add an "animate" key to the [teleport] event
   * add a resistance_anim block used whenever a unit uses a [resistance]
     ability on a neghbour
   * deprecated tag [debug_message]; use [wml_message] instead, which
     now offers the debug/dbg logger and uses the "wml" log domain
     (i.e. --log-*=wml should be used instead of --log-*=notifs)
   * The text output created by [wml_message] to stderr is echoed in-game
     using the chat user interface like [deprecated_message] and other
     Invalid-WML messages; the level of verbosity is controlled by the log
     level for the "wml" log domain
   * the animate_unit action can now animate multiple units
   * the animate_unit action can now change the direction the unit is facing
     with a SLF
   * add a [replace_map] map="file" event that can load a map of a different
     size in a scenario
   * make [remove_shroud] and [place_shroud] use true Standard Location Filters,
     not just location lists/ranges (bug #12869).
 ### Miscellaneous and bug fixes
   * Added the sunset feature to the new dialogs.
   * Fix the calculate feature in the test scenario.
   * Fix an endian issue which rendered text wrong on big endian machines
   * Fix a bug with temporary objects not being removed at next scenario
   * Fix replays with empty starting positions.
   * Prevent access to invalidated memory after the first download when
     updating all installed add-ons (bug #12837)
   * Adjusts drain game mechanics to match attack prediction (bug #7702)

## Version 1.5.7
 ### Campaigns
   * Descent into Darkness
     * Removed debris from this campaign's copy of the map of the
       Great Continent, and converted Parthyn's marker (which was black)
       to a brownish marker.
   * Eastern Invasion
     * Converted to the new gold carryover system.
     * Gave the undead flag to relevant sides.
     * Fixed Dacyn and Owaec not being loyal.
     * In 'Mal-Ravanal's Capital', reworked how the prisoner knights work: now
       they're placed on the map in cages with guards next to them, and the
       player must kill the guard to release the prisoner.
     * Removed debris from this campaign's map of the Great Continent,
       and converted all black markers to brownish markers.
   * Heir to the Throne
     * Added an idle animation for L2 Princess.
     * Fixed the dwarves attacking the player in 'The Lost General'.
     * In 'Bay of Pearls', the player receives recruitment of all lvl1 mermen
       units if both enemy leaders are defeated, and only Mermen Fighters if
       only one of them is.
     * Fixed not being able to recruit mermen in 'Isle of the Damned'.
   * Liberty
     * Converted to the new gold carryover system.
     * Gave the undead flag to relevant sides.
   * The Rise of Wesnoth
     * Changed Familiar's portrait to match his new unit type.
     * Fixed a good amount of funny inconsistencies with Familiar's
       unit type.
     * Fixed broken death events.
     * Lady Outlaw does not lose her experience on 'Return to Oldwood'.
     * Restored missing leading animations for Noble Commander and
       Noble Lord, and gave leading animations to Wesfolk Outcast line.
   * Sceptre of Fire
     * In 'Searching for the Runecrafter', the turn limit is now increased when
       finding Thursagan.
     * In 'Gathering Materials', all enemy sides now fight each other.
   * Son of the Black Eye
     * Cleaned up the forests in the Far North map; removed a lot of debris from
       said map.
   * The South Guard
     * Made 'Into the Depths' a bit easier on the easiest difficulties.
   * Two Brothers
     * Converted to the new gold carryover system.
   * Under the Burning Suns
     * Converted to the new gold carryover system.
     * Made Elyssa loyal+quick instead of quick+intelligent.
     * Gave the Shyde/Star appropriate healing frames.
     * Added updated Desert Hunter sprites.
   * Legend of Wesmere
     * Recoded Olurf in scenario 02
     * Marked the guards in scenario 05 to be killed.
     * Many Bug fixes(recruiting, village ownership)
     * Made Olurf's side persistent.
     * New 400*400 pixel sized portrait images.
     * All portraits are now displayed without scaleing.
     * Updated music playlists.
     * Flipped the Ka'lian map horizontaly.
     * Updated overview map to LoW's time.
     * Redone journey that is displayed on the overview map.
 ### Editor2
   * The map editor is automatically started if the binary file used
     to start Wesnoth contains "editor" somewhere in its name (useful
     for symlinks).
 ### Graphics
   * Gave a nicer ranged attack animation to Troll Rocklobbers.
   * New attack icons: spiked mace.
   * New or updated unit graphics: Gryphon Rider, Gryphon Master, Revenant,
     Naga Warrior.
   * New portraits for Peasant, Swordsman, Spearman.
   * New image-path functions:
     * ~CS(R,G,B): do a simple color-shift by adding the argument values
       to each color channel.
     * ~R(v), ~G(v), ~B(v): mangled forms of ~CS() for single channel
       manipulations.
     * ~O(percentage%), ~O(factor): change an image's opacity by a
       percentage value (0%-100%-*%) or a numeric factor (0.0-1.0-*),
       respectively. Using factors/values greater than 1.0/100% causes
       pixels with non-zero alpha value to have it increased.
     * ~BL(radius): blur an image by the given radius.
   * Added the transparent version of Kitty's portraits.
   * Rewrote the drawing engine for the new widgets.
   * Enabled the new dialogs when a new portrait is available.
   * Properly update animated halos (bug #11965).
 ### Language and i18n
   * Renamed the --dummylocales parameter to --dummy-locales.
   * Fixed the po extraction tool to also include the id field.
   * updated translations: Chinese (Simplified), Czech, Danish, Dutch, Finnish,
     French, Galician, German, Hungarian, Italian, Lithuanian, Polish, Slovak,
     Spanish, Turkish
 ### Terrain
   * New graphic variations for Oasis (Dd^Do) terrain.
   * Added a new invisible impassable overlay terrain, which can be painted on
     top of any (non-layered) terrain to make the hex impassable.
 ### Unit changes and balancing
   * Decreased the range attack of the Drake Flare from 7-4 to 6-4.
   * Decreased the range attack of the Drake Flamehart from 9-4 to 8-4.
   * Increased the cost of the Goblin Spearman from 8 to 9.
   * Decreased the cost of the Sergeant from 20 to 19.
   * Decreased the HP of the Sergeant from 36 to 32.
   * Decreased the HP of the Lieutenant from 48 to 40.
   * Decreased the melee attack of the Lieutenant from 9-3 to 8-3.
   * Decreased the HP of the General from 58 to 50.
   * Decreased the HP of the Grand Marshal from 68 to 60.
 ### User interface
   * Recruit, recall, unit-list and create-unit dialogs display the
     selected unit/unit-type's race.
   * Improved appearance of progress bars besides the loading-screen
     one.
   * Rewrote the layout algorithm for the new widgets.
   * Fixed the font loading when the data directory is overridden.
   * Removed text wrapping around background image in story screens; it is
     better to always use the whole screen width if possible (fixes the
     first story screens in Descent into Darkness and Under the Burning Suns).
   * Tweaked the algorithm that extends the darkened area below long
     story texts so that it gives a more consistent appearance.
   * Added a border and blurring to story screens' text blocks.
   * Improved wrapping in the new widgets.
   * Improved easy close handling.
   * Added a horizontal scrollbar widget.
   * Made the border scrolling threshold a (hidden) preference and made it
     default to 10 instead of 5 pixels.
   * Don't display movement costs for units that have "infinite" costs on
     terrains. The recognition margin is 5 MP.
 ### WML engine
   * Allow inline formulas "$( ... )" to be used in general WML
   * Ignore whitespace at the beginning or end of event names
   * Added the 'round' key to [set_variable].
   * New attribute to_variable= in [unit] to spawn directly into a variable
   * New attributes x,y= in [unstore_unit] to override unit location
   * It is possible to override the end-of-campaign screen text
     and its duration using end_text and end_text_duration attributes
     respectively, in [endlevel] or [campaign] tags. end_text_duration
     is measured in milliseconds. (feature #10449)
   * [modify_turns] now works in scenarios without a turn limit. (bug #12715)
   * Direct modifications to a unit using [unstore_unit] will now be kept
     when advancing to the next scenario.
   * If there is a file called _initial.cfg in an included directory, and
     no _main.cfg, _initial.cfg is guaranteed to be processed before other
     files in the directory.
   * [endlevel] has been expanded by the carryover_report=, save= and
     linger_mode= attribute.
 ### Miscellaneous and bug fixes
   * Compressed start-of-scenario saves are properly recognized by the
     load-game dialog again.
   * Fixed replays for single-player campaigns (bug #12005).
   * Fixed the restart-replay button causing an assertion failure.
   * New memory allocator introduced to the engine. It should
     produce 5-10% savings in memory usage on 64-bit systems.
     The allocator is disabled by default.
   * Included extra headers for certain g++ versions (patch #1113).
   * Fixed random memory corruption/assertion failure/massive leak due
     to story  screens.
   * Fixed [role] not working when no type was specified (bug #12660).
   * Fixed compilation with Python 2.6 (bug #12638 (included a patch)).
   * Fixed spammy multiplication of advancement options for units
     affected by the [unit_type][advancefrom] clause (observed in
     add-on Invasion_from_the_Unknown from the wesnoth-umc-dev
     trunk) each time advancements were evaluated.
   * Improved the drawing routine for the ttext class.
   * Pressing Escape to exit the game from the title screen now requires a
     discrete keypress (bug #12747)
   * Properly implement the CFLAGS for autotools.

## Version 1.5.6
 ### Authoring tools
   * trackplacer is a pygtk tool that makes it possible to visually edit
     the journey tracks on Wesnoth campaign maps.
 ### Campaigns
   * Descent into Darkness
     * Added a set of story art.
     * Made Malin's side use the ostensibly obscure undead flags after his
       banishment
   * Eastern Invasion
     * In 'Weldyn Besieged', the positions of the liches are now randomized and
       their names hidden until you attack them (patch #1109).
   * Legend of Wesmere
     * Newly added to mainline: Intermediate level, 24 scenarios.
       (Imported from wesnoth-umc-dev.)
   * Son of the Black Eye
     * In 'Clash of Armies', fixed all the merman spawns not working correctly
       and limited the number of simultaneous enemy gryphon recruits on the
       easiest difficulty.
   * The South Guard
     * Rewrote 'The Long March'.
     * Made each turn past turn 6 that passes in 'Pebbles in the Flood' give
       you some concrete benefit in 'Tides of War'. Also, the Council of Westin
       now always appears, but the exact turn depends on the length of
       Gerrick's defense.
   * Under the Burning Suns
     * Made use of the undead flag for undead sides
     * All known bugs (e.g. in Keratur's appearances and related to Kaleh's
       AMLAs) have been fixed.
     * Garak now gets into a duel of champions in "A Stirring In The Night".
     * The Quenoth elf graphics have been refreshed; most of their sprites
       now match the size of other 2.0 sprites and have been team-colored.
     * Fixed the problem with Nym's defense animation
   * Only the new gold carryover system is now available in Descent Into
     Darkness, Heir to the Throne, Scepter of Fire, Son of the Black Eye and
     The South Guard.
 ### Editor2
   * Changed the base-terrain key modifier to shift from alt.
     This fixes an issue with some window managers grabbing the event.
   * Added basic mask applying feature
   * Added basic mask creation ("diff") feature
   * New multiple document interface, allows more than one map to be open at the
     same time. Experimental, can be turned off in the editor settings dialog.
 ### Graphics
   * New or updated unit frames: Troll Shaman, Naga Fighter.
   * New animations: Deathblade idle, Orcish Assassin idle, Mage magic missile,
     Silver Mage magic missile and teleport.
   * Fixed some parts of alternative flag sets which were not correctly
     team colored.
 ### Help menu
   * ability upgrades for max-level advancements are now listed (bug #10337)
 ### Language and i18n
   * Fixed a problem on OSX where the locales detection didn't
     work; all languages are now enabled for OSX.
   * updated translations: Catalan, Czech, Danish, Dutch, Finnish, Galician,
     German, Hungarian, Italian, Lithuanian, Racv, Slovak, Spanish, Turkish
   * new translations: Valencian (the previous non-standard translation has
     been renamed to RACV).
   * Fixed Wesnoth looking for editor translations in the wrong place
     (bug #12426)
   * updated DejaVuSans to 2.27
 ### Multiplayer
   * New multiplayer map: 4p Underworld.
   * Revised maps: Caves of the Basilisk, Hornshark Island, Silverhead
     Crossing, Sullas Ruins, Weldyn Channel, Alirok Marsh, Island of the
     Horatii, Castle Hopping Isle, King of the Hill, The Wilderlands.
 ### Music and sound effects
   * Changed sound sources' default fade and full volume ranges to 3 and 14
     hexes respectively; the former defaults were less than adequate.
   * Renamed music files:
     * nr-sad.ogg -> sad.ogg
     * main_menu.ogg -> transience.ogg
     * main_menu_new.ogg -> main_menu.ogg
 ### Unit changes and balancing
   * New units: Dread Bat, Royal Warrior (character)
   * Gave the Goblin Pillager the same bite attack as the Wolf Rider.
   * Updated movement and defense values on the reef terrain for many units.
   * Added the 'fearless' trait to the Heavy Infantryman line.
 ### User interface
   * Story parts that have show_title=yes and text at the same time have
     now the text's background drawn.
   * The new widget library now also supports closing a dialog with a mouse
     click without a close button.
   * The new widget library now has rudimentary support for wrapping long
     texts.
   * Added experimental support to show the new style dialogs when units
     talk in game. (Only available when starting with --new-widgets.)
   * Increased/fixed responsiveness of color cursors on "The End" screen
     when color cursors are enabled.
 ### WML engine
   * Added "border" parameter to [terrain_mask] that overlays on the border
     in addition to the playable map area. The used mask must have the same
     border_size as the map (i.e. currently 1), else this will be ignored.
   * Restore x1, y1, x2, and y2 after events fired from events.
   * WML variable expansion can be used inside story [part] tags
     (feature request #10398).
   * When fire_event=yes, [kill] now also fires the 'last breath' event.
   * SingleUnitFilters can now check for empty keys such as role=$null
 ### Miscellaneous and bug fixes
   * Fixed addon update version logic (patch #1110).
   * Fixed a flaw which caused sound sources to be forgotten after saving
     and loading a game (bug #11495).
   * Fixed crashes on move_unit_fake with unit types which cannot move
     over terrains that are part of the path.
   * Generic units get unique non-underlying ids again (bug #12558).
   * Fixed a bug where "name=" attribute was not recognized in SUF

## Version 1.5.5
 ### Campaigns
   * A new experimental gold carryover system is now supported in Heir to the
     Throne, Sceptre of Fire and Descent into Darkness: the choice appears in
     the difficulty level menu when starting the campaign.
   * Son of the Black Eye:
     * New portraits for Al'Brock, Flar'Tar and Howgarth.
   * Two Brothers:
     * Rewrite to include story screens by Stefan
     * Improved scenario texts and dialogs
 ### Editor2
   * Added editor-specific settings dialog
   * Lighting setting similar to that of old editor, with presets from a
     (hardcoded) list of time_of_days. Can change the settings and see the
     effects without having to close the settings dialog. The preferences
     are shared with the old editor (FR #11408 / Debian bug #458305).
   * Auto update transitions option is stored in the preferences
   * Removed map flipping
   * Added clipboard flipping
   * Command line: added "--load FILE" support to "-e" (FR #12299)
 ### Graphics
   * New or updated unit frames: Walking Corpse swimmer, Soulless swimmer
 ### Music and sound effects
   * New music track "Heroes Rite" by Doug Kaufman.
   * New music track "The Deep Path" by Gianmarco Leone.
 ### Language and i18n
   * Fixed a problem under Windows where the locales detection didn't
     work; all languages are now enabled for Windows.
   * Made it possible to translate ability names according to the
     unit's gender (feature #11982). Not yet activated for unit type.
   * updated translations: Czech, Finnish, French, Galician, German, Hungarian,
     Lithuanian, Polish, Slovak, Turkish, Valencian
 ### Map editor and terrains
   * Removed wesnoth-editor. Use editor2.
   * Added a new "Snow Mountains" terrain (graphics still incomplete).
 ### Multiplayer
   * Revised maps: Fallenstar Lake, 2p Hamlets, Hornshark Island, The Freelands,
     Castle Hopping Isle.
   * In default era, all leaders with 4 MP now receive the quick trait.
 ### Units
   * Balancing changes:
     * Made units with the healthy trait take a quarter less damage from
       poison instead of half.
     * Increased the 'smallfly' movement cost over fungus from 1 to 2.
     * Changed the race of the Wolf Rider line from goblin to wolf.
     * Added the traits weak, slow and dim and made the goblin race use them.
     * Made the goblin race get 1 random trait instead of 2.
     * Increased the movement of the Goblin Spearman line from 4 to 5.
     * Increased the melee attack of the Goblin Spearman from 4-3 to 6-3.
     * Increased the melee attack of the Goblin Impaler from 7-3 to 8-3.
     * Increased the melee attack of the Goblin Rouser from 6-3 to 7-3.
 ### User interface
   * Various minor cleanups and refactoring of the new widgets.
   * Added a new scroll label widget.
   * Allowed usage of wildcards (? and *) in friend and ignore lists.
   * Allowed usage of lists in /friend, /ignore and /remove commands. (feature
     #7492)
   * Added a new image widget.
   * Added a feature to draw hex coordinates and/or terrain code on every hex.
     Available in-game via :sc and :tc commands, and in editor2's menu.
   * Move the attack indicator on it's own layer and show at a higher layer so
     its view won't be obscured by the terrain (bug #12401).
   * Basic support for keyboard controls in main menu (arrow keys + enter)
     (fr #3835)
 ### WML engine
   * New command, [store_time_of_day], makes it possible to store ToD info
     in a WML array/container.
   * Changed underlying_id in unit to be unique number
   * New command, [end_turn], ends the current side's turn.
   * Add scale_background key to [story] (bug #10738)
 ### AI
   * Made it possible to use formula AI for recruiting when using default ai
   * Changed defaults for AI parameters: village_value, villages_per_scout,
     protect_leader, protect_leader_radius, leader_value
   * Added new ai paramater number_of_possible_recruits_to_force_recruit which
     tells AI when it must move leader back to keep.
   * Improved leader movement and protection.
   * Improved village grabbing and protection.
   * Made 'caution' to control how dangerous place AI tries to attack
 ### Miscellaneous and bug fixes
   * The config dir can now be changed when defining APPDATA_USERDATA at
     compile-time to default to a dir under %APPDATA% on windows.
     When specifying --config-dir on the command line %APPDATA% is used even
     without that define.
   * Fixed the list of the unit's attacks getting cut off in the right panel
     (bug #12297).
   * Fixed wesnoth not to try to add duplicate ids to unit_map
   * Fixed a bug where the old unit type's abilities and attacks weren't
     completely cleared during level-up
   * Improved performance of 'recall' user action when disallow_recall
     is set to true in scenarios.
   * Various code cleanups
   * Removed the requirement for files in ~campaigns to have a corresponding
     directory.

## Version 1.5.4
 ### Editor2
   * Rotate clipboard 60 deg. cw/ccw, ctrl+r/ctrl+shift+r respectively
     (cmd instead of ctrl on Mac). Active in the "paste" mode.
   * A drag operation only creates one undo action instead of many.
   * Partial undo feature to keep the old behavior of drag undo available
   * More visible selection - draw a special overlay on every selected hex.
   * Allow different map generators to be used in map -> generate map
     (FR #3950)
   * Reverse the preprocessor logic in src/ to default to building editor2,
     with a DISABLE_EDITOR2 define to build without it.
 ### Graphics
   * New portraits: Silver Mage (male), Mage of Light (male).
   * fixed the parts of the undead flags which were not TC'd
 ### Language and i18n
   * updated translations: Chinese (Traditional), Czech, French, Galician,
     German, Hungarian, Italian, Lithuanian, Polish
 ### Music and sound effects
   * Added a new music track "The Dangerous Symphony" by Gianmarco Leone.
 ### Units
   * Balancing changes:
     * Made units with the healthy trait take half the damage from poison.
     * Decreased the ranged attack of the Bowman from 7-3 to 6-3.
     * Added the marksman special to the ranged attack of the Orcish Assassin
       line.
     * Decreased the melee attack of the Orcish Assassin from 8-1 to 7-1.
     * Decreased the melee attack of the Orcish Slayer from 10-2 to 9-2.
     * Increased the cost of the Walking Corpse from 7 to 8.
     * Increased the cost of the Soulless from 11 to 13.
 ### User interface
   * Made the "slowed", "poisoned" and "stone" floating label translatable
     according to the affected unit's gender. (bug/feature #11957
   * Parts of the new widget libary are deemed stable enough for testing and
     have been started to replace the old code.
 ### WML engine
   * fix bug #11988: Events with multiple types are multiplied
   * added a new image-path function: CROP(x,y,w,h) - it extracts a slice
     of a graphic with the requested dimensions. (feature #12067)
   * allow variable substitution in event names
   * allow spaces or underscores to be used interchangeably in event names
   * In event attack filters ([filter_attack] and [filter_second_attack]),
     weapon= attribute has been deprecated in favour of AnimationWML-like
     attack filtering, which allows to filter damage type, range and weapon
     specials. Backwards compatibility with weapon= will be removed in 1.5.5.
     * As a consequence, [fire_event] has had [primary_unit] weapon= and
       [secondary_unit] weapon= deprecated too; use [primary_attack] and
       [secondary_attack] blocks instead.
   * In events triggered by unit fighting ('attack' et al), the weapon info
     is automatically stored in the 'weapon' and 'second_weapon' WML
     variables; this works just like 'unit' and 'second_unit'.
   * fix various minor bugs with attack filtering
   * Fixed linewrapping with not to wrap markups (bug #11946 and bug #11945)
   * fix bug where max_experience of stored units was not the true max when
     playing with under 100% exp. settings
   * Added support for labels and items that are hiden by fog (patch #1101)
   * Made it possible to use another color palette as second parameter in
     RC(A=B) image path function by using '=' instead of '>' as parameter
     separator; this allows simple color replacement that RC(A>B) did not
     allow.
   * prevent some negative/nonsense values in direct WML unit modifications
   * Renamed the advanceto key in [unit_type] to advances_to in order to be
     consistent with its own and [unit]'s internals.
   * implemented FR #11817: Allow [item]s to be visible only to specified teams
     (patch #1100 by Broodkiller)
 ### Networking
   * removed null termination character from end of packet send by wesnoth
 ### Miscellaneous and bug fixes
   * Applied changes suggested by bug #11676 to wesnoth-optipng.
   * Changed side_drop handling not to automaticaly assign AI for side if
     leader is dead (bug #12186)
   * Changed the behavior of [modify_turns] value= to default to -1
     (no turn limit) instead of 50 if an invalid string is passed instead
     of a number.
   * Fixed [modify_turns] causing weird behavior when used in prestart
     or start events to change current turn.
   * Made config cache code available to eveverywhere in wesnoth
   * Fixed [modify_turns] giving the unwanted side-effect of changing
     turn limit when only current= modifier was used.
   * Fixed OOS bug when giving control and having move in undo stack.
   * Fixed [modify_turns] not updating $turn_number when current= was
     used.
   * Fixed crash when ai moves units next to level 0 hiden unit (bug #12252)
   * Fixed loading ai parameters. MP side defination now overwrites
     era values (bug #12171)
   * Fixed MP saves to transfer correct completion state to remote clients
     when loadin game (bug #10385)
   * Fixed can move enemy units (causing OOS) (bug #11205)
   * Fixed a few inconsistencies related to scenarios which are not at
     turn 1 at the beginning (namely start autosave detection and initial
     triggering of a matching "turn *" event).
   * Removed the compiler work arounds for MSVC6.
   * Added some more compiler workarounds for MSVC9.
   * Removed the --small-gui build option, starting Wesnoth with the
     --smallgui parameter achieves the same
   * new --nomusic commandline option to disable music for the session

## Version 1.5.3
 ### Campaigns
   * Descent into Darkness
     * Changed how the beginning of the scenario 'Descent into Darkness' works.
     * Changed how the ice breakage works in 'Beginning of the Revenge'.
     * Many minor fixes and tweaks to various scenarios.
     * New graphics for Apprentice Necromancer and Dark Mage.
     * New indoor maps for 'A Small Favor' parts 2 and 3.
   * Sceptre of Fire
     * Fixed a bug that allowed one to finish 'The Dragon' by defeating both
       enemy leaders.
     * Fixed a coordinate bug that caused 'Hills of the Shorbear Clan' to not
       end when it was supposed to.
   * Son of the Black Eye
     * Fixed the shamans not getting removed from the recall list when they
       should.
     * New portraits for Inarix, Jetto and the old orcish shaman.
     * Reworked the scenarios 'Civil War' and 'Coward'.
 ### Editor2
   * Future replacement for wesnoth_editor, launched from title menu or via
     wesnoth -e. Most old editor's features are duplicated, with some
     improvements.
   * Built by default in scons, cmake and autotools, if building manually
     see RELEASE_NOTES.
   * See https://www.wesnoth.org/wiki/Editor2 for details and known issues.
 ### Language and i18n
   * updated fonts: DejaVuSans 2.26
   * updated translations: Chinese (Traditional), Danish, Finnish, Galician,
     German, Italian, Latvian, Lithuanian, Polish, Russian, Slovak, Spanish
 ### Multiplayer
   * Removed the "Great War" MP era
   * Revised maps: Caves of the Basilisk, Den of Onis, Fallenstar Lake,
     Silverhead Crossing, Sullas Ruins, The Freelands, Alirok Marsh, Island of
     the Horatii, 3p Morituri, Blue Water Province, 4p Hamlets, Lagoon,
     4p Morituri, The Wilderlands, Waterloo Sunset
 ### Music and sound effects
   * Engine automatically plays special music on defeat or victory; default
     lists from which an option is randomly chosen at runtime are provided as
     the default_victory_music and default_defeat_music attributes in
     [game_config] node; this may be overriden per-scenario using the
     victory_music and/or defeat_music attributes on its code - it can also
     be overriden in a [endlevel] block by providing a music= attribute with
     the desired list (feature request #11203).
 ### Python AI
   * Added ai_init.py and ai_launcher.py to make it easier to customize
     AI environment and startup. Both are used by the new embedded python
     implementation.
   * reversed builtin is now allowed. This allows for random.shuffle to be
     called.
 ### Terrains
   * Added a new attribute, hidden=<boolean> (default: no), to [terrain],
     which prevents the map editor from offering the user a particular
     tile type for drawing.
   * Made user-made content graphic rules be parsed before core ones, so
     custom tiles look the same in-game as they do in the editor.
   * Set hidden=yes on main terrain archetypes (Gt, Qt, At, Xt).
 ### User interface
   * Added an experimental add-ons update interface.
   * Added initial drag&drop support for button widgets
   * Fixed a broken translation which gave all females the prefix female^ on
     alignment description when using the "C" locale or any translation that
     is missing that string.
   * Make the load and recruit hotkey use the ctrl instead of cmd key on non
     Mac systems
 ### Campaign server
   * Made campaign server encode CRs in old content in first startup
 ### WML engine
   * Fixed the filtering for x,y=recall,recall in [store_unit] and [kill] tags
   * Fixed the bug where any changes to the primary unit in the "advance"
     event were totally ignored
   * Added some convenience macros for units: {TRANSFORM_UNIT}, etc.
   * Extended [modify_turns] to allow scenario designers to change the
     current-turn number dynamically with current=<number>.
   * It is now possible to use [time_area] to add or remove new local ToD
     areas and their schedules during scenario events. Removal requires to
     associate an id. to [time_area]s, or it won't work. Uses Standard
     Location Filter to match locations.
 ### Miscellaneous and bug fixes
   * Add some gcc-4.3.0 compilation fixes (patch #1083)
   * Added a MSVC 9 project file and some fixes for that compiler (patch #1093)
   * Add-on uploads by default will skip all dot-files (.*), which are hidden
     files or directories in UNIX filesystems.
   * Downloaded add-ons' meta information is stored in <add-on dir>/_info.cfg
     rather than <add-on dir>/info.cfg now; both the in-game client and the
     wesnoth_addon_manager Python script have been updated for this.
   * Fixed bug #12094: Event "last breath" cannot be triggered for attackers
   * Fixed [filter_attack] and [filter_second_attack] not being effective
     in die/last_breath events.
   * Fixed FULL_HEAL WML macro not working properly.
   * Fixed NR Krash inexplicably disappears from recall list (bug: #11612)
   * Fixed segmentation fault when wml modified and killed attacker and
     defender in a battle
   * Fixed Wesnoth crashing after an attacker/defender_hits/misses event if
     the WML kills one unit and replaces/unstores the other

## Version 1.5.2
 ### campaigns
   * Son of the Black Eye
     * new set of portrait art
     * in 'Black Flag', there's now several galleons transporting enemies to the
       beach, and no watch towers
     * in 'Saving Inarix', you can now more precisely control when you destroy
       the bridge
     * many assorted small tweaks to most scenarios
     * when leading the Great Horde, all units now require 1 less upkeep
   * Tutorial
     * Fixed players side name to same as leaders name (bug: #10114)
 ### formula AI
   * added variant_iterator to iterate over variant elements
   * modified choose, filter, find and map functions to use variant_iterator,
     so they work correctly with a variant_map type
   * new formula AI functions: contains_string, find_shroud, movement_cost,
     tolist, tomap, is_unowned_village, max_possible_damage_with_retaliation
   * added possibility to fallback to human player
   * new ai object members: side, my_recruits, recruits_of_side, time_of_day,
     units_of_side, villages_of_side
   * moved most of formula AI releated code from ai.cpp to formula_ai.cpp
   * new unit_type_callable and attack_type_callable classes
   * new unit_callable members: undead, attacks, abilities, traits
   * various fixes and improvements to formula functions
   * added new formula test scenario
 ### language and i18n
   * new translation: Latvian
   * updated translations: Arabic, Catalan, Chinese (Traditional), Czech,
     Finnish, French, German, Greek, Italian, Lithuanian, Russian, Serbian,
     Slovak
   * languages in the language selection are now sorted on name instead of
     code.
   * unit alignment can be displayed in its gender-specific form in languages
     which require/support it (bug #11956)
 ### multiplayer
   * Added support for multiple sides per client in MP start.
   * Added support for reserving slots when reloading game.
   * Fixed timer end warning not to play in opponents turn. (bug: #11517)
   * Teams are now translated locally for all clients. (bug: #3676)
   * Fix an old pathfinding bug causing an inconsistency between the path used
     and the one showed to others players. It was only a visual error, but
     sometimes caused to see a enemy pass through your invisible units (or in
     their ZoC). Now all players see the same real path. Also valid for replay.
   * When joining a game, display your assigned starting position and the new
     team coloring also shows your assigned color.
 ### units
   * balancing changes:
     * increased the ranged attack of the Bowman from 6-3 to 7-3
     * decreased the melee attack of the Bowman from 6-2 to 4-2
     * decreased the cost of the Bowman from 15 to 14
     * increased the cold resistance of the Fencer line from 0% to 10%
     * decreased the 'orcishfoot' movement cost over frozen from 3 to 2
 ### user interface
   * Rewrote the textbox history saving of the new widget library. This rewrite
     is incompatible with the old version, but since the library is still in
     development, no compatibility layer has been added.
   * Added gui to choose where server binary is
   * Added type-a-head search to file chooser dialog
   * Add a column for traits in the recall dialog.
   * Add a filter textbox in the recall dialog.
   * When filtering units, clear the unit preview when none matches.
   * Delete/dismiss the last item of a dialog stop closing it
   * Tooltip texts now support blank lines and special markup characters
   * Add-on publish/delete options have now special icons and colors.
   * Display the name of the add-on in the "Add-on installed" dialog
   * The unknown unit icon used in status table is now team colored
   * In multiplayer lobby, all unit's images are now team colored.
   * In help, with debug mode, broken hyperlinks are now visible in red and
     when clicked open a error message showing the bad topic reference.
   * new debug command ":choose_level" or ":cl" to directly go to a scenario.
   * new debug command feature, ":next_level Some_Id" to directly go to
     scenario having id=Some_Id.
   * 'Create Unit' dialog pre-select the last created unit type
   * Fix incorrect downloaded add-on when using filter
   * Fix missing last publish/delete add-on option
   * Fix few broken hyperlinks in help (and some were working in english only)
   * Fix a regression about not working 'unit description' with hide_help=yes
 ### Python AI
   * get_variable now allows a default value to be passed to the call. If the
     key is not found, the default value is returned. This is fully backward
     compatible.
   * set_variable exception handling has been fixed to return an exception in
     the current call frame.
   * Many wesnoth module functions now release the python GIL when it is both
     safe to do so and the function call takes long enough where it also
     makes sense. More changes coming.
   * A new global boolean variable, 'restricted' is now set before the user AI
     script is invoked. This variable indicates if it is running inside of
     a restricted python environment or not.
   * A new class of unrestricted scripts are now listed. Previously only
     scripts which have #!WPY at the top are allowed. If only allow safe python
     scripts is disabled, scripts which start with #!UNSAFE_WPY are also shown
     to users. This allows AI authors to specifically target either a
     restricted or unrestricted environment.
   * New "system" class attributes are exposed in the restricted environment.
     These include; '__call__', '__copy__', '__deepcopy__', '__name__',
     '__repr__' and '__str__', in addition to the old __init__ method.
   * ValueError can now be caught.
   * Added a new AI, bruteforce_unsafe.py, which uses both function caching
     (memoize pattern) and psyco to speed execution. It uses the new UNSAFE_WPY
     tag and is not selectable unless "Only Run Safe Python AIs" is disabled.
     This version of bruteforce runs 1x-14x FASTER than the stock python AI;
     *heavily* dependent on the map and number of units involved.
   * Per the python API documentation, many functions now return boolean values
     rather than ints. Minor modernization effort to use new C-API macros
     provided via python 2.4+. This change is fully backward compatible for
     scripts which properly treat the return value as a boolean.
   * Initial wail directory checkin. It's only a teaser at this point. ;)
   * bruteforce_wail.py has been added - showing how easy it is to move a
     well written bot from the wesnoth module to the wail module.
 ### WML engine
   * When examining stored units, now the attacks, max_hitpoints, max_moves,
     and max_experience are the "real" values and can also be modified.
   * new attribute count= for [have_unit] and [have_location] conditionals
   * new mode "insert" for [set_variables]
   * max_attacks in [unit] now also works for values bigger than 1
   * the "zoc" key works for [unit_type] too, and for [unit] accepts other
     boolean values than 1 and 0 (bug #11889).
   * new effect apply_to=type to transform the unit to a different type
   * remove redundant unit.value (use unit.cost instead)
   * preserve unit.role in next scenario (bugs #3697, #4124, and #11329)
   * fix various problems with [set_variables] (bugs #11980 and #11981)
   * Add a new key "image" in [faction], used as icon in faction selection.
     The old hack consisting to add the image's url in the faction's name is
     still supported but doesn't have the new team coloring.
 ### miscellaneous and bug fixes
   * Fixed droiding not to make wesnoth think player is observer (bug: #9675)
   * Fixed statitics not add turn data from previus scenarios (bug: #11719)
   * Fixed manager to initialize before gamestate in playcontroller (bug: 11935)
   * Removed persistance from team configuration (bug: #10916)
   * Fixed text parser again strip cr from configs. Added CR and 254 to be
     escaped in campaign upload. All UMC has to be reuploaded to server
   * Made automaticaly generated macro reference easier to naviagate and link to
     (patch: #1076)
   * Allow [unit_type] num_traits=0 to override race's num_traits.
   * Small optimization of scrolling and things out of their hexes.
   * Added a command-line parameters "--smallgui" to allow lower resolutions.
   * Missing images are now replaced by a "men-at-work" sign (in debug-mode)
   * Better pathfinding: now always prefer straight path and is also more
     stable between little mouse's moves.
   * Added the pango cairo dependency
   * Various code cleanups

## Version 1.5.1
 ### campaigns
   * Descent into Darkness:
     * fixed a crash in 'Return to Parthyn'
   * Eastern Invasion:
     * fixed a crash in 'Xenophobia'
   * Heir to the Throne:
     * allowed Kalenz to take the flaming sword
   * The Rise of Wesnoth:
     * fixed the Wesfolk Outcast line's distract ability not working
     * increased the gold you get from not letting Lady Outlaw join you in the
       second scenario
     * removed the outlaw princess and outlaw queen and instead made Jessica
       only have one unit line
   * Son of the Black Eye:
     * fixed a crash in 'Dwarvish Stand'
   * An Orcish Incursion
     * new storyline and dialogues to scenario 1
     * new storyline and dialogues to scenario 2
     * made the AI do mixed recruits in scenario 2
     * new storyline and dialogues to scenario 3
     * added a form of gold bonus to scenario 3
     * new storyline to scenario 4
   * Under the Burning Suns
     * fixed event spawning elves in outer villages
 ### game engine
   * poison no longer prevents resting
 ### graphics
   * new portraits: Red Mages, Dark Adepts, White Mages
   * new ford graphics from Syntax_Error
   * added a sea serpent portrait by Pic
   * various improvements to the new widget library
 ### language and i18n
   * new translations: Arabic, Friulian, Macedonian
   * updated translations: Chinese, Czech, Danish, Dutch, Estonian, Finnish,
     French, German, Hungarian, Italian, Japanese, Polish, Russian, Serbian,
     Slovak, Spanish, Turkish
   * langcode change: moved gl_ES to gl
   * the new widget library supports disabling of rows in a listbox, this is
     used to deactivate not available languages. This feature is experimental
     and only available when started with --new-widgets (bug #11212)
   * fonts: DejaVuSans 2.25
   * manual: updated screenshots to match UI rearrangements
 ### multiplayer
   * revised maps: Den of Onis, Sablestone Delta, The Freelands
   * raised default per-Turn MP timer bonus to 60 seconds
 ### Python AI
   * Implemented a function which detects if a location is on the map border.
   * Implemented a function which gives the game's gold parameters.
   * Extended safe.py environment to expand AI's language capabilities; including:
     collections, functools, Queue, sets, time, and the upcoming wail module.
     Use of chr, hash, lambda, ord, and super (new style classes) are now allowed
     Control of safe_exec can now be toggled from the wesnoth binary.
   * Added new advanced option, "Only Run Safe Python AIs". When disabled, the safe_exec
     environment is disabled for all running AIs. Use caution when disabling this option!
   * Try/Except clauses are now allowed. A subset of builtin exceptions are available.
     ArithmeticError, AssertionError, AttributeError, BaseException,
     StopIteration, IndexError, KeyError, NameError, RuntimeError,
     RuntimeWarning, and ZeroDivisionError
   * Exceptions can now be raised by user code.
 ### terrains
   * Fixed city village not being alias of the village terrain type; this was
     causing a duplicate "Village" terrain being displayed in the defense ratios
     section of unit descriptions in game help
   * Reorganisation of the terrain macros including more consistent image
     naming. The old macros are kept for now for compatibility.
   * New terrain: cave path
   * Fixed a ToD coloring bug preventing to use red channel below 8 (on 255)
   * Fixed some bugs with location-specific terrain-graphic rules
 ### units
   * balancing changes:
     * changed the healthy trait to give 1HP and 1HP per level instead of 2HP
     * decreased the ranged attack of the Mage of Light from 15-3 to 12-3
     * decreased the melee attack of the Merman Warrior from 8-4 to 10-3
 ### user interface
   * Add a hotkey "Custom Command" and a command "custom <command>" to assign
     a command to this hotkey. Also accept sequence of commands ("this; that").
   * Add a command "alias <name>[=<command>]". To set or show shorter alias to
     a command. Also accept incomplete command and pass parameters (ex: "alias
     hp=unit hitpoints", then use "hp 100"). Saved between sessions.
   * Add a filter text box for the "Create Unit" dialog
   * Various improvements of filter text box, now words separated by space give
     a filter doing an "and" combination of them.
   * Added "Host Network Game" option back to multiplayer menu
   * Fixed chatlog not to so lobby joins if set in preferences
   * Also highlight the fog under the mouse (not just the terrain)
   * debug command "unit" now uses equal sign again ("unit hitpoints=100")
   * Add-ons download dialog:
     * The class of each add-on is displayed in the downloads dialog
     * It is possible to filter what add-ons are displayed by text (matching
       against any of the information fields)
   * Advanced prefences allows user to toggle allowing potentially unsafe python AIs
 ### WML engine
   * titlescreen is now randomly loaded
   * removeitem now can take an image key so that overlays can be removed one at
     a time. (patch #1067)
   * fix [teleport] capturing villages with the wrong side (bug #11683)
   * SideWML recruit= no longer locks the faction except if explicitely wanted
     with the new key faction_from_recruit=yes
   * add "default_base" parameter to overlay terrains, which specifies the
     base terrain the editor draws by default.
   * New [heal_unit] action.
   * Make it possible to change base and overlay separately in [terrain] and
    [terrain_mask] actions.
   * rename the "hides" status as "hidden"
   * add a new image path function : GS to greyscale the image
   * [store_map_dimensions] also stores map border size now.
   * renamed old [attacks] special to [swarm]
   * created [attacks] special with expected behaviour and usage similar to
     [damage] special
   * add a warning about future end of support for implicit prefix "units/" in
     images url.
   * add a switch (delayed_variable_substitution, defaults to 'yes') allowing to
     choose wether variables in [event] spawned inside an [event] are substituted at
     spawn or execution time. Fixes bug #11843 introduced by a change in this behavior.
 ### wesnothd
   * added selective ping support - saves server bandwidth
   * updated pings to use new simple_wml
   * pings now sent using raw method to they only get compressed once for batch
   * added expiration time to bans
   * added restart command to server that does graceful restart
   * added option to do graceful shut down for server
 ### campaignd
   * made campaign server use gzip compression for networking
   * detect client connection mode and use same for sending data
   * made campaign server use gzip for storing addons
   * added configuration option to choose gzip compression level
   * made it possible to store content authors' e-mail addresses
     for administrative purposes
   * authors can specify the type of their content ('type'
     field in .pbl files)
 ### miscellaneous and bug fixes
   * Client now sends 'selective_ping="1"' during login
   * fixed parser bug that prevented loading binary data strings
   * fixed issues with campaign info in non-compressed saved games
     (bug #11386)
   * Implemented the option to use the mouse selection clipboard in X11
     the new widget library also uses it.
   * fixed multiplayer_connect to handle leave_game command from server
   * improved reloading of game configs after installing or removing addons
   * fixed threading bug in upload_logs
   * dissallow_observers is on as default if side has controller=null
   * Fixed null-pointer reference in network code
   * Made networking code check system buffer size and limit send length
   * Made wesnoth start wesnothd for lan games
   * Implemented send_file to reduce memory usage when sending files
   * Implemented send_file using sendfile() system call
   * Fixed tokenizer not to strip CR from quoted string because it would
     destroy images transfered over network
   * Added possibility to use per fight EV statistics proposed by maboul
   * Added smoother FPS limit to server
   * Fixed a memory leak in networking code
   * Removed bug introduced in 1.5.0 that allowed use of :debug commands during
     network play
   * added some includes to fix compilation problems with Sun Studio 12
     (patch #1066)
   * Fixed unstoned units not having their attacks at their first turn
   * Prevent steadfast ability from lowering resistance if already over 50%
   * <user data dir>/data/campaigns and <user data dir>/data/units
     directories are npw created on first run
   * added command line parameter --config-path, which prints the path to the
     config directory and exits
   * fix a bug with variable substitution time of [event]-within-[event]
   * fix a crash with [ai] in [modify_side]
   * prevent infinite recursion when [kill] fire_event=yes is used to kill
     primary_unit on its own 'die' event (Gna! bug #11207 / Debian bug #448193
   * Optimize titlescreen and credits rendering.
   * Give its name to the "Fog clearer" (fixing blank in "Create Unit" dialog)
   * fix bug with filter + delete savegame (Gna! bug #11779)
   * Optimize cache validation and first display initialization.
   * corrected setlocale() usage on win32
   * Fixed [binary_path] declarations not being recognized inside the MULTIPLAYER
     #ifdef for eras
   * Silence the warning "error display: could not open image ''"
   * Optimize fullscreen toggle and windows resizing.
   * Fix various rendering bugs in 8bpp display mode.
   * fix bug #11630 (preventing redo and rename during opponent's turn).
   * fix a regression about missing default portrait in dialogs.
   * fix false move-cursor around enemies after some dialog events.
   * fix some unit decapitation when placed on a keep
   * wesnoth_addon_client now uses gzip compression and can receive (untested)
     zlib compressed data. Tons of various additional changes required to
     make this work. Tons of additional core wml parsing bugs have been
     fixed by various others and my self. -oracle
     Ran pylint against code - some results applied to code.
   * made the image cache LRU, with a high limit of 600 images per cache

## Version 1.5.0
 ### campaigns
   * synchronize all campaign ids with their directory name
   * greatly updated the scenario music playlists in The Rise of Wesnoth,
     Descent into Darkness and The South Guard
   * The Hammer of Thursagan:
     * instead of leadership, the dwarvish loremaster unit line now has a
       similar but unique inspire ability
   * Under the Burning Suns:
     * Minor cosmetic fixes
   * Northern Rebirth:
     * Major WML cleanup and optimisation
     * New respawn mechanism for white mages
   * Liberty:
     * Fix graphic artifact bug #11438
 ### Formula AI
   * added support for Formula AI language, more info available at:
     https://www.wesnoth.org/wiki/FormulaAI
 ### graphics
   * Fixed broken TC on transport-galleon and drake walking corpse graphics
   * New portraits: Elvish Sorceress
   * added a light shadow on the bar of the loading screen
   * new dungeon wall terrain
   * new city-themed village variation
   * new progressive parameter for animations: submerge
   * new progressive parameter for animations: x and y
   * image_diagonal now also works with normal [frame]
   * sub-frames now obey terrain height
   * selecting a poisoned unit now does the selection anim properly
   * convert the render engine to a z-ordered based engine, this allows to
     have multiple layers and the calculation and rendering are separated
     which means things can be drawn in front of units.
   * removed the now unused tilestack based drawing.
   * added a new experimental gui engine available when starting with
     --new-widgets. The engine doesn't do much yet.
 ### map editor
   * Fixed not working "Update transition" and made "Delay transition update"
     directly trigger an update when toggled off.
   * add-ons can now make their custom terrains available in the editor
   * Draw base terrain under an overlay when ALT is pressed
   * Implemented adjustable lighting levels (time of day)
     (debian bug #458305 / gna bug #11408)
 ### game engine
   * Implemented lazy loading for unit_types
 ### language and i18n
   * new translation: Croatian
   * updated translations: Chinese, Chinese (Traditional), Czech, Danish,
     Dutch, Finnish, French, Galician, German, Greek, Hungarian, Italian,
     Lithuanian, Japanese, Polish, Russian, Serbian, Slovak, Spanish, Swedish,
     Turkish, Valencian
   * updated fonts: DejaVuSans 2.24
 ### multiplayer
   * added maps: Howling Ghost Badlands
   * revised maps: Sablestone Delta, Sullas Ruins,
     Silverhead Crossing, The Freelands, The Manzivan Traps
   * removed maps: Amohsad Caldera
   * removed the Wesbowl scenario
   * added the "A New Land" scenario by Bob_the_Mighty
 ### units
   * balancing changes:
     * decreased the cost of the Giant Scorpion from 32 to 22
     * increased the HP of the Drake Flare from 54 to 55
 ### user interface
   * placement of the counters and displays of the top panel optimized
   * Refactored the in-game console and added help function (patch #1036)
   * Refactored chat /commands and added help.  Made :commands a superset
     of chat /commands
   * optional cancelling of unit orders in load-game dialog (patch #1024)
   * fixed sub-optimal multi-turn pathfinding which avoided ZoC too much
   * smarter pathfinding: if same MP cost, prefer terrains with better defense
     and empty hexes (less frequent multi-turn moves blocked by a friend)
   * New full map screenshot feature (no default hotkey).
   * Screenshots show a pop-up with the url and size of the file created.
   * When trying to define an already used hotkey, now tell where it's used.
   * Load-game dialog displays a campaign's translated name rather than its
     internal id now
   * the hardcoded resolution list now also includes tiny gui and small gui
     resolutions if compiled with those switches
   * more comprehensive terrain type naming to avoid confusing displays such
     as 'Road (Grassland)' or 'Impassable mountain (Cavewall)'
     This also applies to the WML keys used in the stats:
     - grassland becomes flat
     - tundra becomes frozen
     - canyon becomes unwalkable
     - cavewall becomes impassable
   * Added new commandline options (patch #1031)
     -s or --server [host] connect to host specified or to the first server
     in preferences
     -c or --campaign skip menu and show campaign selection menu
     --with-replay replays the file loaded with --load option.
 ### WML engine
   * new tag [insert_tag] to place dynamic WML content
   * Extended most C++ checks for boolean WML attributes to use
     utils::string_bool() rather than true/false or yes/no string comparisons;
     as a result, they should accept true non-zero values, "on", "yes" or "true"
     strings; and "0", "off", "no" or "false" are considered false values.
   * A variable string's length can now be obtained with [set_variable]
     string_length=
   * Implemented [variable] boolean_not_equals= for simmetry with [variable]
     boolean_equals=
   * Implemented [store_map_dimensions], which saves the map size in a variable
     with values 'height' and 'width'
   * [modify_side] can set AI parameters now, using the [ai] syntax implemented
     for [side] declarations (patch #984)
   * allow 100% defense instead of cutting off at 99%, if out of range set to
     100% instead of 50 (debian bug #467253)
   * The UnitWML [unit] tag is changed to [unit_type].  [unit] will still
     be accepted for backwards compatibility until 1.5.3.  wmllint can safely
     do this up-conversion.
   * In SingleUnitWML and SideWML, the description= attribute is now id=.
     description= will still be accepted for backwards compatibility.
     wmllint can safely do this up-conversion.
   * In SingleUnitWML and SideWML, the user_description= attribute is now
     name=. user_description= will still be accepted for backwards
     compatibility.  wmllint can safely do this up-conversion.
   * In SingleUnitWML and SideWML, the generate_description= attribute is
     now generate_name=. generate_description= will still be accepted for
     backwards compatibility. wmllint can safely do this up-conversion.
   * In UnitWML, the unit_description= attribute is now description=.
     unit_description= will still be accepted for backwards
     compatibility.  wmllint can safely do this up-conversion.
   * In ThemeWML, the unit_description= attribute is now unit_name=.
     unit_description= will still be accepted for backwards compatibility.
     wmllint can safely do this up-conversion.
   * In AnimationWML, the [unit_filter] tag is now [filter], the
     [secondary_unit_filter] tag is now [filter_second], and the
     [attack_filter] tag is now [filter_attack], and the
     [secondary_attack_filter] tag is now [filter_second_attack]
   * In EventWML, the [special_filter] tag is now [filter_attack] and the
     [special_filter_second] tag is now [filter_second_attack]
   * In the Standard Unit Filter, the [wml_filter] tag is now [filter_wml]
   * it's now possible to specify with loop= how many times a sample
     associated with a sound source should be played
   * locations stored in variable now include ownership information for villages
   * new [switch] conditional command
   * new [fire_event] event tag to fire any custom event
   * new [unit_side] theme element: this is the side of the current unit (flag)
   * Preprocessor now logs when it encounters an undefined macro, and when it
     fails opening a file.
   * Implemented prerecall/recall. Now prerecruit/recruit will no more trigger
     on recall events
   * Add support for overlay terrains (terrains which can be placed above any
     base terrain)
   * Add multiple types for events
   * Allow [clear_variable] to clear multiple variables (separated by comma)
   * Fixed bug #11286 "Sighted events not firing when turning off delay shroud
     updates". Which also means that user may fire sighted events in a new way.
 ### miscellaneous and bug fixes
   * Changed logging to have less overhead when it is turned off (patch #1038)
   * Fixed error message for broken add-ons (bug #11078)
   * INSTALL now describes the scons build.  autotools, while still present,
     is deprecated.
   * Removed spurious -R linker switch from command line generated by the Boost
     M4 macros
   * wmlxgettext now prints some context information about the strings
     extracted (patch #993)
   * Fixed network_worker to spawn at least one thread (bug #11238)
   * added VICTORY_AND_DEFEAT_MUSIC macro - not wiring it in in trunk
     content, in the hope that we get engine support for this soon, but it's
     here to use in the meantime
   * remove an ancient wml update program which has been obsoleted by wmllint
   * added the boost regex dependency
   * added the sdl-ttf dependency
   * added the zlib dependency
   * default gender selection now works correctly for units with
     only a female variant.  Fixes bug #11197.
   * Fixed a border case of unit portrait advancement on which generic portraits
     would be treated as character-specific (affected female Elvish Archer line;
     it also theoretically affected units with [variation]s using different
     portraits)
   * allow configure to be started from directories other as the top dir
     (patch #1002)
   * added -Werror -Wno-unused -Wno-sign-compare to the default compiler
     flags, can be disabled with --disable-strict-compilation.
   * learn the underlying terrain so you'll not have a blank line in the unit
     stats if you encounter a terrain aliased to an unencountered terrain
   * fixed an alignement issue which caused a SIGBUS on a Sparc (debian bug
     #426318)
   * wesnoth-pngcrush is now superseded by wesnoth-optipng
   * clean up gcc 4.3 warnings so the game now builds on fedora rawhide
     with warnings treated as errors. A few bugs were found in the
     process as well.
   * "fog" and "shroud" debug commands now also update the minimap, and
     "shroud" stop shrouding your units.
   * Fixed shroud not directly cleared in minimap after a move+attack action.

## Version 1.4
 ### language and i18n
   * new translation: traditional Chinese
   * updated translations: Catalan, Chinese, Czech, Danish, Dutch, Esperanto,
     Finnish, French, German, Hungarian, Japanese, Lithuanian, Polish, Russian,
     Slovak, Spanish, Turkish, Valencian
 ### graphics
   * more elvish portraits
 ### multiplayer
   * revised maps: Cynsaun Battlefield
   * messages will no longer default to the private channel when observing
     or joining a new game
   * fixed an invalid vector access, which could happen since linger mode
     didn't reset the player_numer_ (bug #11094)
   * random maps can no longer crash if two teams have the same starting
     position (that happened if 2 teams didn't get a starting position.)
     bug #11166.
   * when the number of players for a random map can't be placed the engine
     tries maximal ten times before giving up.
 ### sound
   * added new music track, "Knalgan Theme" by Ryan Reilly.
   * added new music track, "The King is Dead" by Mattias Westlund.
   * updated music track, "Traveling Minstrels" by Mattias Westlund.
   * changed the default story screen music from loyalists.ogg to revelation.ogg
   * added new music track, "Nunc Dimittis" by Jeremy Nicoll.
   * added new music track, "The City Falls" by Doug Kaufman.
 ### WML engine
   * fixed a bug where addons with recusive INCLUDES could no longer be loaded
     (bug #11160)
 ### miscellaneous and bug fixes
   * units with a death sound but no death animation now play their death
     sound correctly
   * selection animations are not directional when standing animations are
     disabled (bug #11151)
   * Fixed parser problems with \r\r in files
   * cleanups and a compiler fix based on patch #911
   * the detection of the savegame version was done after parsing the savegame
     this could lead to crashes when loading 1.2 savegames (debian bug 467088)
   * fix a bug where the random map generator could place a keep on the border
     (bug #11150)
   * Fixed UI sounds toggle crash on Windows
   * when a new unit is spawned when the attacker dies and the new unit has
     a lower number of weapons then the weapon number used in the attack an
     wml_exception was thrown (bug #10926).
   * truncate names in the load dialog utf8 aware, might be related to
     bug #11186
   * fix bug #11187 play_once=yes and fix musical cross-fading

## Version 1.3.19
 ### map editor
   * activate border transitions in the editor using more translucent images
     than used ingame
 ### language and i18n
   * updated translations: Chinese, Czech, Danish, Filipino, French, Hungarian,
     Italian, Lithuanian, Polish, Turkish
 ### miscellaneous and bug fixes
   * Fixed chat log viewing crash after undo
   * Fixed era events loading
   * Idle, standing and selection animations are not accelerated anymore
   * All animations are disabled when the LOW_MEM compile switch is activated
   * correct FIREBALL_MISSILE not being synchronized properly. All UMC users
     need to check units using that macro
   * correct most missiles not being properly (bug #11134)
   * fix some female units using male frames during missile attacks
 ### replays
   * fixed "play side turn" not working (bug #11118)
   * at least partly fix "replays always corrupt" (bug #10976)

## Version 1.3.18
 ### campaigns
   * Northern Rebirth:
     * fix Sister Theta not appearing after freeing her (bug #11083)
 ### language and i18n
   * updated translations: German, Italian
 ### campaign server
   * Strip CRs from uploaded data
 ### miscellaneous and bug fixes
   * Made unit checksum only test for important variables

## Version 1.3.17
 ### campaign server
   * added more logging
   * remembers the last maintainer upload IP
 ### campaigns
   * Liberty
     * Made the guards in Hide and Seek behave more intelligently.
   * The South Guard
     * Completely redesigned the internal workings of 6b The Long March.
 ### graphics
   * more elvish portraits
 ### language and i18n
   * updated translations: Catalan, Chinese, Danish, Esperanto, French, German,
     Italian, Japanese, Slovak, Spanish, Valencian
 ### multiplayer
   * revised maps: Caves of the Basilisk, Sablestone Delta, Silverhead Crossing
 ### tutorial
   * fixed the recall refunding/re-doing in scenario 2 (bug #10993)
   * scripting of the AI moves now check for units in target hex (prevents bug
     #6745)
   * fixed some problems with the grunt (Dumbo) dialogue
   * decorated map with new terrains
 ### user interface
   * tinygui:
     * fixed fontsize for the clock
     * removed $RACE entry when using tinygui
     * fixed replay theme in tinygui to display correctly
     * get closer to the normal theme when tinygui is active and the
       resolution bigger than 400x300
   * reduce minimap in resolutions <1000x620 to allow displaying all attacks
     from units with many attacks (eg the sylph)
   * add support for 800x480 when using the configure option
     --enable-small-gui, not everything displays correctly (preferences,
     mp-creation/lobby, ...), but normal gameplay should work perfectly
   * improved editor theme so that it works in 800x600, too
   * increase space for terrains in the mapeditor by rearanging items a little
   * quick replays no longer scroll to leaders
   * scrolling to the leader the very first time happens instantly now
   * Various tunings about the transparency of hp/xp bars (avoid confusion like
     in bug #11030) and keep them highlighted for the selected unit too.
 ### WML engine
   * unstoring a leader for a side without a leader sets the leader for
     that side (bug #11048)
 ### miscellaneous and bug fixes
   * replays of midgame saves get the right recall list now (bug #10868)
   * Fixed controller problems when reloading MP game (bug #11046 and bit more)
   * Made scrollarea to stay in bottom if it is resized (bug #6974)
   * Fixed reference to invalid iterator in unit_cycle (bug #7991)
   * Fixed side_drop server message handling to correctly restart turn
   * Fixed network code crash after failed socket operation
   * Fixed random start ToD (bug #11056)
   * Fixed an assert crash when a fake_unit_move starts outside of the screen
   * Fixed temporary disappearing of hp/xp bars of victorious unit (bug #11055)
   * Fixed flickering hp/xp bars of ghost and other invisible units.
   * Placement of random terrain images is more random.
   * Optimization of damage floating labels
   * Fixed update of the minimap and starting positions when using the "delay
     transitions update" option.
   * chasm and lava bridges are now proper aliases of cave and chasm

## Version 1.3.16
 ### campaigns
   * Two Brothers: set the leader of scenario one to passive to make it not
     too easy to win
 ### language and i18n
   * updated translations: Chinese, Czech, Dutch, Filipino, Finnish, German,
     Greek, Hungarian, Italian, Lithuanian, Polish, Slovak, Spanish, Turkish
   * replaced the font used for the chiense translation (gkai00mp.ttf) with a
     subset of WQY (just the gb2312 part is included) as requested by the
     chinese maintainer
 ### map editor
   * the editor handles errors with the old unsupported map format more
     graceful (bug #11023)
 ### multiplayer
   * revised maps: The Freelands
   * don't display 'Remote scenario' for reloaded games in the multiplayer
     lobby which is wrong in most cases (fixes bug #10882), the display
     of 'Reloaded game' is disabled for now because of the string freeze
   * Fixed MP saves loadind to choose correct human side (bug #10894)
   * display reloaded games in yellow instead of green in the game list
     as they are also a kind of already running games
   * Made era not required while loading save game
   * Removed bogus client commands about takeing side
   * Fixed control change when giving own team (bug #6639)
   * when loading a savegame don't offer to take the non-player sides (bug
     #10746)
 ### units
   * balancing changes:
     * decreased the village defense of Bats from 60% to 40%
     * decreased the forest defense of the Ranger from 70% to 60%
     * decreased the forest movement cost of the Ranger from 2 to 1
     * decreased the shallow water, mountain, swamp and snow movement cost of
       the Ranger from 3 to 2
 ### user interface
   * During dialogs the speaker is shown in the sidebar and highlighted.
   * During ai moves the source hex is no longer highlighted.
   * show unit standing animations and idle animations are now separate options
   * Removed broken "Host network game" option from multiplayer menu (bug #10800)
   * Fixed network game creation return back to create dialog after failed
     savegame loading
   * Fixed a small glitch with fog and shroud in the corners (bug #10831)
   * share_view now looks at whether your allies have shared their view,
     before whether or not you saw the view of your allies was determined by
     your own view was shared (bug #10994)
   * slight tuning of hp/xp bar: not anymore hidden under leader crown and the
     HP bar's scaling is changed (now the cap is ~80hp instead of 70)
   * new red footprints images for move cost > 3
 ### WML engine
   * fixed an off by one error in [scroll_to]
   * unified the two different max_loop counters and used highest maximum
     (65536).
   * Fixed abilities filtering to test [filter] allways
 ### terrains
   * added stone bridge terrain over lava and chasms.
 ### miscellaneous and bug fixes
   * Fixed networking not to timeout with slow connections but timeout faster
     with lost connection (partialy fix bug #10967)
   * Fixed client side ping timeout check if downloading or uploading
   * Moved destruction of conditional object before the mutex. This should
     fix random crash in network disconnect.
   * Fixed reference to invalid pointer in attack::attack
   * pressing shift affects acceleration immediately
   * More gcc 4.3 fixed
   * Remember cleared hotkey
   * Added some toys&whisles to unit tests
   * Added networking unit tests
   * Optimized MP chat log building
   * Optimized tokenizer to speed up loading config files
   * Hide race sections having only units with "hide_help=true"
   * Fixed ai handling of unit without attacking weapons (bug #10886)
   * Optimize roads placing of random map
   * when a unit miss an animation, the engine will base the replacement on
     the standing animation instead of the standing frame
   * enabled caching images for lowmem unconditionally since it seems to save
     memory (bug #11022)
   * disable stricker terrain validation, since a rogue scenario may avoid
     somebody to create any game (bug #11024)

## Version 1.3.15
 ### language and i18n
   * updated translations: Catalan, Chinese, Czech, Danish, Dutch, French,
     German, Hungarian, Italian, Lithuanian, Polish, Spanish, Turkish
   * updated DejaVuSans to 2.23
 ### graphics
   * new elvish portraits by Kitty
 ### multiplayer
   * revised maps: Cynsaun Battlefield
   * fixed bug #10777: Cannot cancel a multiplayer password prompt
   * fixed bug #10779: Rejoining game causes major problems
   * Fixed :control command to work
   * Fixed control change not to set wrong team if we are playing now
   * damage statistics for the current turn also works for observers now
   * MP loads now era_id from save file (bug #10878)
   * skip replay no longer skips the story for players only for observers
     (bug #9538)
   * observers without the addon installed can watch the second scenario of
     a MP campaign again (bug #10794)
 ### user interface
   * Made ESC clear hotkey when changing hotkeys
   * Made quick replay skip messages
   * linger mode overlay is also drawn over fog and shroud (bug #10811)
   * Units are deselected before they move.
   * The next move can be prepared before the current attack/move
     animation finishes (bug #7132).
   * If a move is interrupted, footsteps are drawn again.
   * Tuned some details of the mouse handling.
   * Fixed [message] not to close if it has input (bug #10846)
   * fixed chat color when message is added by turn_info (bug #6846)
   * added a more graceful handling of maps without a header (bug #10787)
 ### WML engine
   * Fixed order of sighted and moveto events (bug #9560)
   * now allow the use of $unit inside [show_if] and [location_filter]
     in the [set_menu_item] tag
   * fixed weapon_specials filter_opponent to work in attack selection (bug #8333)
   * fixed crash in battle events if unit was killed or teleported (bug #10801)
   * fixed era events not to be added if we are loading save game (bug #10772)
   * some filtering was added to the [animate_unit] action to actually make it
     useful
 ### miscellaneous and bug fixes
   * Fixed turn timer and end_turn commands order (bug #10849)
   * Fixed boost test compile with 1.34.1
   * Made unit to hideable by others units (bug #10877)
   * Make wesnoth work properly again if the datadir contains ../
   * Fixed some type of addon not being uninstallable (bug #10788)
   * fixed adjacent units to update if unit affects them
   * converted NO_MAP_DATA to new map format
   * fixed unchecked vector access
   * make sure the python campaign client list shows a space between the
     headers if the size of the column is smaller than the header (eg uploads)
   * make sure a unit with more hitpoints than its maximum doesn't terminate
     the game with an assertion error (bug #10876)
   * various bug fixes and code cleanups
   * added some extra headers for the upcoming gcc 4.3 (debian bug #462708)
   * image scaling on tinygui was broken, this has been fixed
   * wescamp script allows the same values for true as utils::string_bool()

## Version 1.3.14
 ### campaigns
   * The Hammer of Thursagan:
     * 'Invaders' if there weren't enough free tiles to spawn all indigs the
       game would enter an infinite loop (bug #10621)
   * Two Brothers:
     * updated the music playlists in all scenarios to include the new music
   * Under the Burning Suns:
     * Completely new scenario 3
     * Few bugfixes to scenario 6
 ### graphics
   * overlays are properly drawn at the top row (eg forrest) (bug #10238)
   * mountains no longer have a hill as base
   * when a leader dies the villages, which are no longer are owned, are
     properly redrawn (bug #9136)
   * sometimes the first hitpoints left percentage and the unscatched had a
     difference of 0.1%, this has been fixed (bug #9122)
 ### language and i18n
   * updated translations: Chinese, Danish, Dutch, Finnish, French, German,
     Italian, Lithuanian, Slovak, Spanish
   * added gkai00mp.ttf font (needed for the Chinese translation)
 ### multiplayer
   * revised maps: Den of Onis, Weldyn Channel
   * renaming a unit no longer generates an OOS error (bug #7864)
   * if in a MP campaign the endlevel was continue(_no_save) it could happen
     that the client left before the host uploaded the new scenarion, this
     has been fixed
   * when joining a MP game the recall list wasn't loaded which leads to
     OOS errors in campaigns (bug #10624)
   * :droid can now use the additional parameters on & off to enable/disable
     the AI status of a side instead of toggle it (FR #9676)
   * fixed the MP campaign gold carry over (bug #10677)
   * MP campaign start of scenario saves can be loaded again (bug #10058)
   * if a MP side has no colour defined fall back to the default side colour
 ### sound
   * timer bell in MP starts when there are 20 seconds left and fades in
     gradually for 10 seconds (fr #10559)
   * fixed timer bell not always playing (bug #10559)
   * it was possible to add the same track to the playlist twice which
     could lead to an infinite loop (bug #10112)
 ### units
   * balancing changes:
     * increased the XP requirement of the Vampire Bat from 14 to 22
     * decreased the HP of the Vampire Bat from 17 to 16
 ### user interface
   * the apple key works as shortcut modifier again (bug #10586)
   * made "Show lobby joins of friends only" the default preference
   * new menu option added to save replays manually
   * show damage statistics for the current turn in the statistics window
   * opening the action menu in linger mode no longer crashes the game
     (bug #10438)
 ### WML engine
   * new tag [text_input] for [message] (patch #921)
   * new tag [filter_vision] for Standard Unit Filter
   * it is now possible to play [sound] repeatedly using a "repeat" attribute
   * Added boolean variable disallow_observers= to side defination
   * Random factions can be defined on a subset of the non-random faction
    (FR #10600)
 ### miscellaneous and bug fixes
   * set the default resistance to 100 (no resistance) instead of 0 (immune)
     (bug #10661)
   * when loading a unit some traits didn't get applied correctly but got
     fixed in a later state. This could lead to some units not leveling
     properly (bug #10304)
   * validate vector access for colours (bug #10622)
   * Fixed OOS on unit advancement if wml had battle/die events (bug #10590)
   * added serveral fixes to compile with Sun Studio compiler (patch #911)
   * bumped autoconf version requirement to 2.60 since it's needed for the
     boost tests (bug #10636)
   * fixed a problem where two [recall]s in a SP game could get an out of sync
     recruitment
   * avoid triggering an assertion when a duplicated side definition exists
     (bug #7252)
   * Fixed OOS if [message] menus cause advancement of a unit
   * the previous "max-saves" slider is now used as a slider to determin the
     maximum number of auto-saves to keep, default value is 10, so the 11th
     oldest and all older auto-saves will be deleted
   * the MP server now always sends gzipped data
   * Rewrote the AI village assigning algorithm, instead of using brute force
     it now tries to optimize before resorting to brute force (bug #7215).
   * parser exceptions are now displayed visually (patch #914)
   * always ask for permission before overwriting an already existing replay
     file (bug #10689)
   * don't automatically save a replay for every game an observer visits
     if the option 'Save Replay on SP/MP Victory ..' is enabled (bug #10690)
   * changed option 'Delete Saves on SP/MP Victory ..' to remove only
     Auto-Saves to avoid the removal of manually saved files from unrelated
     games
   * make sure the team number is reset in single player linger mode
     (bug #10692)
   * fullscreen now defaults to false
   * "hide_help" now hides the unit from the list in help, but still allow to
     see its description (using context-menu or hotkey)
   * change the default port for the campaign server to 15005 to seperate the
     new 1.4.x way from the old possibly broken content from the trunk server

## Version 1.3.13
 ### campaigns
   * A Tale of Two Brothers:
     * what the correct passwords for 'Guarded Castle' are is now randomized
   * Liberty:
     * 'Hide and Seek' mostly rewritten and it now has a bigger map with
       randomly placed guards
   * The South Guard:
     * now the player has to kill the fake M'Brin in 'Choice in the Fog' instead
       of just attacking once
 ### language and i18n
   * updated translations: Chinese, Czech, Danish, Finnish, French, German,
     Greek, Hungarian, Italian, Lithuanian, Polish, Portuguese (Brazil),
     Valencian
   * updated DejaVuSans to 2.22
 ### map editor
   * resize actions can be undone again (bug #10216)
 ### multiplayer
   * revised maps: Hamlets
   * recruitment OOS is fatal now (bug #9723)
   * the server now stores the next scenario send but the host, thus the
     clients no longer restart in the previous scenario
   * the next scenario now properly loads the recall list
 ### sound
   * new or improved sounds: ogre hit and die, MP chat
 ### units
   * all lvl 2 outlaw units can now advance to lvl 3 in all campaigns and MP
   * the default AMLA for all max-level units is now +3 max HP, +20% max XP and
     healing to full
   * the standard AMLA XP limit upped from 100 to 150
   * replaced the fullheal AMLA of the Necrophage with a feeding ability, giving
     it +1 max HP for every living enemy killed
 ### user interface
   * display the race in the unit preview panel (with gender variation support).
   * allow to use team labels also for 1-player-teams (bug #9747)
   * changing the langugage now sets the version number in the title
     properly.
   * show 'back to round xxx' also in 800 x 600 resolution
   * a multiplayer map with allow_new_game causes the selection of a map to
     show the wrong minimap (bug #9425)
   * the "1 turn to reach" of the movement hint is now used only for multi-turns
     move, instead icons show ZoC and village capture ending the move.
   * turn to reach numbers also works when bigger than 9 and without parentheses
   * display an icon into the movement hint where the unit can be invisible and
     the sidebar icon is now about the current status of the unit
   * fix glitches with the sidebar status icons (slowed, poisoned..) when the
     mouseover leave the unit
   * middle mouse button now do a progressive scrolling when kept pressed, speed
     and direction depend of the the position of the mouse relatively to the
     center of the view
   * in help, sections are now opened by clicking on the book icon or
     double-clicking on its title
   * each section has now an associated page with general info and links to its
     topics
 ### WML engine
   * new event "last breath" will be triggered when a unit dies, but
     before the animation is played
   * allow ThemeWML to display the race.
   * new "random_gender=" key (boolean) to single units declarations in WML;
     the engine will generate a random gender for the spawned unit just like
     when it's recruited
   * updated {GENERIC_UNIT ...} macro to take advantage of random_gender
   * fixed [filter_radius] to support [and][or][not] and radius=
   * added rand= for [set_variable] which can be used in MP, the user has to
     make sure it stays in sync
   * units spawned with unit in events can now have random traits and also the
     names are the same (bug #10501)
   * load the recall list with a start of scenario save (bug #10444)
   * Added support for side specific [message] (fr #7427)
   * a floating text can now be specified within animation frames using the
     text= and text_color= keys
 ### miscellaneous and bug fixes
   * various bug fixes and code cleanups
   * added gzip and gunzip command line parameters
   * replaced the 'Binary Saves' option with 'Compressed Saves' and now
     writes gzip files
   * added a "ping_timeout" preference to allow adjusting the default timeout
     value ('60') from the preferences file
   * rewrote the network code to also receive gzipped data:
     * The clients send gzipped data to the MP server.
     * The MP server can send gzipped data depending on a hidden parameter
       which is off by default.
     * The campaign server sends and receives binary_wml.
   * fixed a crash when the attacker dies while performing a desparate attack.
     The crash seemed only to happen under Windows but valgrind also
     complained (bug #10496)
   * loading a content with invalid wml does not terminate the game
   * increased the precission off the mouse in the top row of the editor
     (closes bug #10219)
   * Fixed code to filter sent items when undoing (bug #10588, needs testing)
   * Avoid an assertion failure triggered in the replays
   * speed-up the cache validation by skipping directories named "images" and
     "sounds", WML authors must avoid to put cfg files there (or manually
     refresh their cache if they really want to place and edit cfg files there)
   * using "next-unit" on the last unit now triggers the selection effect each
     time (instead of half the times)
   * fix some latency for clearing fogged corners on shrouded maps
   * optimization of the pathfinding and unit loading
   * avoid deferring null pointers in the multiplayer lobby

## Version 1.3.12
 ### campaigns
   * The Rise of Wesnoth:
     * 'A New Land' can now be won by waiting till turns run out
 ### language and i18n
   * updated translations: Czech, Danish, Finnish, French, Italian, Polish,
     Swedish
 ### map editor and terrains
   * added village count to editor
 ### multiplayer
   * revised maps: Caves of the Basilisk, Cynsaun Battlefield, Den of Onis,
     Fallenstar Lake, Hamlets, Hornshark Island, Sablestone Delta, Silverhead
     Crossing, Sulla's Ruins, The Freelands, Weldyn Channel, Alirok Marsh,
     Island of the Horatii, 3p Morituri, Blue Water Province, Castle Hopping
     Isle, Clash, King of the Hill, Lagoon, Loris River, 4p Morituri, Paths of
     Daggers, Siege Castles, The Wilderlands, Xanthe Chaos, Forest of Fear,
     Amohsad Caldera, Hexcake, Waterloo Sunset, 8p Morituri, Merkwuerdigliebe
 ### sound
   * added new music track, "Vengeful Pursuit" by Jeremy Nicoll.
   * added new music track, "Variations on an Elvish theme" by Doug Kaufman.
 ### units
   * balancing changes:
     * increased the HP of the White Mage from 32 to 35
     * decreased the ranged attack of the White Mage from 7-4 to 9-3
     * increased the melee attack of the White Mage from 6-1 to 6-2
     * increased the HP of the Mage of Light from 42 to 47
     * changed the ranged attack of the Mage of Light from 9-4 to 12-3
     * decreased the melee attack of the Royal Guard from 12-4 to 11-4
     * decreased the melee attack of the Merman Warrior from 8-4 to 10-3
     * decreased the pierce melee attack of the Merman Triton from 11-4 to 14-3
     * decreased the blade melee attack of the Merman Triton from 11-4 to 19-2
     * decreased the melee attack of the Naga Myrmidon from 8-6 to 9-5
     * increased the melee attack of the Lich from 5-3 to 8-3
     * increased the melee attack of the Ancient Lich from 6-4 to 8-4
     * increased the defense of the Bat line on all terrains from 50% to 60%
 ### user interface
   * changed the default setting for the turn bell to on
   * removed the turn_cmd preference option
   * Savegames now have a prefix indicating the campaign they are from if
     the campaign WML declared an abbrev= tag.
   * in linger mode a new overlay is used to see the difference between
     playing and lingering
   * abilities and traits now show a blank line if missing, to ease units
     comparison
   * the numbers keys also change the reachability in N turns for selected enemy
     (works with simple mouseover too)
 ### WML engine
   * New standard unit filter keys:
     - defense: chance to be hit on current terrain by normal weapons
     - movement_cost: movement cost on current terrain
   * the preprocessor now ignores filenames with '..' in them
 ### miscellaneous and bug fixes
   * fix a compilation bug on Windows (usleep not defined)
   * addon interface: added support for an ignore file (.ign) to configure what
     shouldn't be uploaded on the addon server (it's useful if you use a svn
     checkout for example)
   * fixed some small interface glitches on MP saved games loading and setup screen
   * fixed some more possible utf-8 related terminations
   * fixed WML added anims being forgotten on save/load
   * fix an animation freeze when selecting a fighting unit
   * prevent unit selection (and sending "select" event) when commands are locked.
   * optimization of the loading of units WML data.
   * optimization of "Create Units" and "Recruit" dialog.
   * fixed incorrect number of total villages for villages at right and bottom border
   * fix bug #8991: Load a second savegame crash when using --test option
   * added a new drawing framework only used for linger overlays at the
     moment, other stuff can be converted later

## Version 1.3.11
 ### campaigns
   * Eastern Invasion
     * in 'The Drowned Plains', undead no longer spawn near you randomly but
       instead they are placed on the map at the beginning of the scenario and
       remain immobile and hidden until the player steps next to them
     * Epilog throneroom scene now has a horse-canter sound effect for Owaec.
   * Scepter of Fire
     * in 'The Dragon', the player now gets a starting castle where to recruit
       and recall normally
 ### campaign server
   * fixed a bug which broke uploads with a .cfg file next to the campaign
     directory
   * finalized wescamp integration
 ### map editor and terrains
   * fixed a bug which prevented the editor to load maps with a Windows EOL on
     Windows, other platforms where not affected.
   * added a 'Mushroom Grove lit' (on cave) terrain using (string 'Uu^Ufi')
 ### graphics
   * added user-contributed graphic updates for 'storm trident', 'ankh' and
     'staff' pick-up items
 ### language and i18n
   * updated translations: Chinese, Czech, Danish, Dutch, French, Galician,
     German, Polish, Portuguese (Brazil), Swedish
 ### multiplayer
   * fixed segmentation fault on loading saved games from the multiplayer user
     interface
   * fixed random leaders not getting random genders as expected by design
 ### WML engine
   * implemented the #ifndef directive, the opposite of #ifdef, in the
     preprocessor, to parse the following block only when the symbol is not
     defined
   * fix the recall list duplication bug (bug #10183)
   * [modify_side] can now override the recruit list, just like [set_recruit];
     should be the preferred method at some point.
   * the gold carryover can now be modified from wml (fr #10144)
 ### units
   * balancing changes:
     * increased the melee attack of the Saurian Oracle from 5-2 to 4-3
 ### user interface
   * The attack dialog displays the range between weapon's info
   * The footsteps of a teleporting unit shows haloes on teleport points
   * More readable colors for dark map labels (bug #10271)
   * Tips of the day have attributions, a "Previous" button and a fixed size.
 ### miscellaneous and bug fixes
   * added some extra headers for gcc 4.3 (based on patch #842)
   * Fix bad fog update when passing by or stopping on a village
   * Fix various bug when teleporting a unit to a fogged village (bug #10273)
   * Display a message when a teleport fails (exit village has an hidden ally)
   * Warning message on the standard output when using tiles with bad size

## Version 1.3.10
 ### campaign server
   * it's now possible to delete a campaign with the master password
   * added initial support for the wescamp translation integration
     (not finished yet)
   * You can now supply a .pbl file within the directory for a campaign
     as _server.pbl, rather than externally at the same level as the directory.
     Both the client code in the game and the Python client in utils/ have been
     enhanced to allow this.
 ### campaigns
   * Eastern Invasion
     * Fixed bug #10195, related to Holy Amulet's description
     * some gold balancing for 'Xenophobia' and 'Lake Vrug'
   * Liberty
     * in 'The Raid', the accompanying peasants are no longer loyal. Also, the
       scenario now gives an early finish bonus
     * turn limits of 'A Strategy of Hope' lowered, to prevent massive amounts
       of carryover gold
   * Northern Rebirth
     * Balancing polishing and bug fixing.
     * Disabled undead branch for the upcoming 1.4 string freeze, as it is far
       from finished right now, and will not change before the string freeze.
     * Made auto recalled units loyal.
   * Sceptre of Fire
     * 'Gathering Materials' now has more varied terrain, an early finish bonus,
       and the gold and coal piles now behave better: each pile now only gives
       one load, miners carrying a load have an icon on them and when dying they
       drop their load, so another miner can pick it up
     * in 'Outriding the Outriders', villages (except the first one) now give
       you two units instead of one
     * Khrakrahs is no longer allied with the hostile elves and dwarves
   * Under the Burning Suns
     * Kaleh now uses AMLA's for custom advancement
     * renamed the files and scenario id's to reflect scenario titles
     * new title image and some initial story images
     * redesigned the Dust Devil unit to make it less overpowering
     * gave Elyssa 1 move across sand
     * fixed AI recruitment in 'Across the Harsh Sands', made Lost Souls
       easier to beat back on MEDIUM and HARD, and fixed the bug with Holy
       Water (bug #10254)
     * bugfix for blocker in 'In the Tunnels of the Trolls'
 ### graphics
   * polished ankh prop picture from Liberty, and moved to core, replacing
     old ankh-necklace.png used by Eastern Invasion
   * updated unit graphics: Necromancer, Dark Adept, Dark Sorcerer, Fugitive,
     Huntsman
   * new or improved animations: saurian augur line magic attack
   * various shadow under units updates for consistency
 ### sound
   * new or improved sounds: skeleton hit and die
 ### language and i18n
   * updated translations: Chinese, Czech, Danish, Finnish, French, Galician,
     German, Greek, Hungarian, Italian, Lithuanian, Polish,
     Portuguese (Brazil), Russian, Spanish, Swedish, Valencian
   * updates fonts: DejaVuSans (2.21)
 ### map editor
   * fixed a bug when two dimensions of a map were modified, the editor
     could crash (bug #10216)
   * fixed a bug when shifting the map, the starting positions weren't
     updated correctly (bug #10216)
 ### multiplayer
   * revised maps: Cynsaun Battlefield, Den of Onis, Hamlets, Silverhead
     Crossing, Sulla's Ruins, Weldyn Channel, Blue Water Province
   * new multiplayer scenario added: Dark Forecast, random survival for up to
     two players
   * removed option to enforce female gender on side leaders, replaced with a
     gender choice combo-box in the MP side setup UI (client and host), which
     can choose between random, male, and female, when available.
   * the server sends a periodical 'ping' to all players to detect ghosts
   * implemented reloading of the server config on SIGHUP
 ### sound
   * "Legends of the North" (legends_of_the_north.ogg) moved from NR to mainline
   * A new pair of victory and defeat themes by Ryan Reilly added.
 ### units
   * balancing changes:
     * increased the movement points over tundra (snow) and deep water from 1
       to 2 for 'drakefly'
     * added the 'fearless' trait to the Ghoul line
 ### user interface
   * selecting your own unit makes it flash briefly and emit a select sound
   * you can now also select enemy units and see their possible path, terrain
     defense and turn to reach.
   * changed the mute hotkey from ctrl-m to ctrl-alt-m
   * renamed "Advanced Mode" button on graphics preferences to "Change Resolution"
   * renamed "Advanced Mode" button on sound preferences to "Advanced Options",
     and "Normal Mode" to "Standard Options"
   * fix some inconsistencies between mouse selection and "next unit" selection
   * double-click in dialog now triggers a "press button" sound
   * fix an exploit using the "black stripes" of a unit with teleport ability
     to detect enemy villages under fog
   * fix the wrong "(1) turn to reach" on already captured village
 ### WML engine
   * extended [store_side] behavior, so that it additionally writes the following:
     fog, shroud, colour, user_team_name, controller, recruit (the recruit list),
     village_gold (the income per-village).
   * extended [modify_side] behavior, to contemplate fog, shroud, controller,
     recruit and village_gold settings.
   * new key "contains=" in [variable] conditions to check the presence of a
     substring in a variable value
   * new tag [debug_message], which takes as paramaters "logger=" and "message="
     to silently output messages to stdout or stderr.
   * new key "variation=" in [move_unit_fake] statements to add the possibility
     of making a fake unit look like one of its variation while moving - this
     also reflects in a change to the MOVE_UNIT macro, which now can use gender
     and variation too.
   * maps now have a user definable border
   * Changed the mute hotkey from ctrl-m to ctrl-alt-m.
   * On victory, all old saves for the scenario except the start one
     are now deleted (rather than just autosaves as formerly). This
     is done before replay saving, if that is enabled. The preference
     name has changed from delete_autosaves to delete_saves.
   * Animation engine allow multiple type of frames (a la "missile frames")
     for all types of anims, you can have multiple such anims using different
     frame names
   * a new type of animation is triggered when a unit is selected. (WML
     support to come shortly)
   * in story [image,] a new key scaled=yes/no (default=no) allows to scale
     the image like the background.
   * prevent some illegal select events on enemy units.
 ### miscellaneous and bug fixes
   * added a new log domain 'mp_server'
   * added a new log level 'debug'
   * various bug fixes and code cleanups

## Version 1.3.9
 * campaigns:
  * Descent into Darkness
    * in 'Peaceful Valley', goblins may no longer spawn when you recapture a
      village you had lost
  * Heir to the Throne
    * new graphics for the battle princess
    * in 'Crossroads', the orc ambushers now hide immobile on the map from
      the beginning of the scenario, instead of being spawned randomly when
      stepping on hills
  * Liberty
    * new story images
    * a proper bigmap
    * music changed in several scenarios
  * Northern Rebirth
    * New map for Eastern Flank
    * Fixed bug where Father Marcus regenerates in the wrong place
      in one the death events.
    * Rescaled Rakshas' portrait to the proper size.
    * Balancing of opening event in 'Settling Disputes'
    * Various minor polishing and bug-fixes.
    * Extensive balancing, polishing and bug fixing on undead branch.
  * Son of the Black Eye
    * Fixed bug in 'The Coward' where an enemy unit speaks as if
      he is on your side.
    * A bit of dialogue polishing in 'The Coward'
    * Jetto now joins you even if you don't release him during the scenario
  * An Orcish Incursion
    * Campaign heavily revised and added to the distribution
  * general
    * no longer use the external_binary_data dir for including campaign and
      difficulty level icons and images, but reference the images directly
 ### graphics
   * new animations: rogue death, orcish leader line leadership
 ### language and i18n
   * updated translations: Czech, Danish, Dutch, Finnish, French, German,
     Italian, Lithuanian, Polish, Swedish
 ### map editor
   * a right click in floodfill mode now performs a flood fill.
   * new icon for: village, castle
   * new rotate function (FR/bug #3870): copy&rotate selected area using mouse
     cursor as center (or "center of mass" if the cursor is not on map)
   * important optimization of the builder engine (which recalculates
     transitions after a map change)
   * restore the random map generator (was broken in 1.3.8)
   * fix several bugs with the "delay transition update" option (undo and
     multi-hexes operations)
 ### WML engine
   * new key "centered=yes" in [story] to use center of images (like dots)
     on the given map coordinates.
   * kill the obsolete "flip" key in [story], use imagepath function
    "~FL(horiz/vert)" instead
 ### user interface
   * preserve aspect ratio of the minimap (FR/bug #9999)
   * the debug "Create unit" dialog have now two columns (race and type)
     and is correctly sorted.
   * progressive parameters can be specified both in [animation]
     and in [frame]
   * The linger mode "Next Scenario" button renamed to "End Scenario"
   * improved the display of the trait descriptions
 ### multiplayer
   * renamed maps:
     * Blitz to Weldyn Channel
     * Charge to The Freelands
     * Meteor Lake to Fallenstar Lake
     * Triple Blitz to Alirok Marsh
   * revised maps: Caves of the Basilisk, Weldyn Channel
   * minimum number of turns reduced to 1
   * new option to allow female leaders by default instead of male leaders
   * fixed a crash if the client receives invalid utf-8
   * fixed a server side bug with could cause invalid utf-8
     being send to a client.
   * display the era id for not installed eras in the lobby.
   * display the scenario id for unknown scenarios in the lobby.
 ### units
   * balancing changes:
     * created undead variations for the 'bat' and 'gryphon' race
 ### miscellaneous and bug fixes
   * various bug fixes and code cleanups
   * fixed a glitch where an item halo shifted position when zooming
   * map label length is correctly determined for multibyte characters
     and thus the limit has been reduced to 32 characters again (bug #6855)
   * now using :n in debug mode will skip past linger mode as intended
   * fix a bug with [endlevel] result=continue_no_save
   * fixed a bug where some strings were resized in utf-8 unaware mode
   * fix bug #9021 (team color names not directly translated when
     switching language)
   * optimize the random map generator (especially for high village density)

## Version 1.3.8
 ### campaigns
   * Descent into Darkness
     * fixed a bug causing the growth ability of Ghast to not always work
     * fixed a bug in 'A Small Favor - Part 2' preventing one random passage
       from opening up
   * Eastern Invasion
     * new portrait for Konrad II
   * Liberty:
     * fixed the undead transformations not working in 'Unlawful Orders'
     * clarified the objectives in 'The Gray Woods'
   * Northern Rebirth
     * Colored portraits for Sisal and Rakshas
     * Fixed bug in 'Old Friend' where Tallin looses bonus HP, MP and
       experience gained from the Rod of Justice
   * Sceptre of Fire
      * fixed lava expansion in 'Caverns of Flame' not working
   * Son of the Black Eye
     * Fixed bug in 'Saving Inarix' where the blown up bridge wasn't shown
       properly (bug #9817)
   * The South Guard
     * a large number of assorted little polishing tweaks and improvements
     * a somewhat nicer-looking village flag
     * Deoran now has a lvl 3 advancement, and no special AMLA options
     * new sprites for Sir Gerrick
     * removed the leader and hero ellipses until it's decided whether they
       should be used in all campaigns
     * Map for "A Choice In The Fog modified so mermen are actually useful.
     * "Tidings Good And Ill" is now playable even if you didn't recruit
       elves in the previous scenarios (bug #9944).
     * fixed bugs with units sometimes not starting with max HP and movement at
       the beginning of some scenarios.
     * in 'Tidings Good And Ill', you can now recruit and recall the elf
       escorts of your choosing instead of having them automatically picked
       from those you had in the previous scenario
     * in 'Tidings Good And Ill', defeating the naga queen is now mandatory
     * 'Into The Depths' has been completely rewritten.
     * there is a new scenario, 'Return To Kerlath', between 'Into The Depths'
       and 'Vengeance' on the elf branch.
     * trimmed the large unused portions of the map in 'Vengeance' away
     * in 'Vengeance', removed the death of Minister Hylas as a defeat condition
   * Under the Burning Suns
     * fixed the missing dialogue parts of elves fighting mud crawlers
       in 'The Morning After'.
     * made all tent villages yield elvish refugees (possibly)
       in 'The Morning After'.
     * fixed the stables encounter dialogue to always appear in order
       in 'The Morning After'
     * 'Across the Harsh Sands' - dehydration causes incurable damage (except
        through visiting an oasis) and fractional loss of attack strength.
     *  Adjusted some of the encounters and rewrote the Lost Soul appearance
        logic in 'Across the Harsh Sands'.
     * Added some dialogue to explain the ghost reappearance when they were
       defeated the night before to 'Across the Harsh Sands'.
     * 'A Stirring in the Night' - village events work correctly now (bugs
       #9947 and #9915), Added animated and impassable campfire terrain.
   * general
     * fixed many scenarios erroneously using the wrong difficulty level symbol
       in #ifdefs (MEDIUM instead of NORMAL)
 ### graphics
   * new graphics for the highwayman
   * nicer image for the illuminates aura and a new campfire aura
   * no more idle animations for units next to an enemy unit
   * new button and slider in preferences to switch unit idle animations
     on and off and set their frequency
   * no grid on off-map tiles
   * nicer transitioning between desert road, desert and desert villages
   * a new alternative terrain for lava, one which isn't drawn in a pit
 ### language and i18n
   * manual: switched to a brand new manual, now using some strange format
     to generate it and ending in a nice .html page
   * updated translations: Bulgarian, Danish, Dutch, Finnish, French, German,
     Hungarian, Italian, Japanese, Polish, Portuguese (Brazil), Russian,
     Swedish, Valencian
   * new translation: Serbian (Latin version)
   * added sazanami-gothic font (needed for the Japanese translation)
   * updated DejaVu font to 2.20 (no new characters)
   * made all keeps be called "Keep", all castles be called "Castle" and
     all roads be called "Road", instead of using variant names (such as
     "Elven Castle" or "Desert road")
 ### multiplayer
   * revised maps: Charge, Den of Onis, Meteor Lake, Silverhead Crossing,
     Triple Blitz, Clash
   * 'village_gold' is no longer stored when 'use_map_settings' is used
   * the server sends the game id and not only the game name if a player joins
     a game to avoid ambiguity in case of multiple games with the same name
   * show observer team chat messages in the 'Chat Log' window
   * implement linger mode for multiplayer
 ### units
   * balancing changes:
     * gave the Fugitive the concealment ability (village hiding)
     * increased the XP requirement to advance to the Fugitive from 77 to 120
     * decreased the melee attack of the Fugitive from 12-2 to 11-2
     * decreased the ranged attack of the Fugitive from 8-4 to 7-4
     * changed the race of the Vampire Bat line from 'undead' to 'bats'
     * changed the movement type of the Vampire Bat line from 'undeadfly'
       to 'fly' (with adjustments)
     * added a magical ranged 7-2 arcane attack to the Dark Adept
     * added a magical ranged 9-2 arcane attack to the Dark Sorcerer
     * added a magical ranged 12-2 arcane attack to the Necromancer
     * added a magical ranged 9-3 arcane attack to the Lich
     * added a magical ranged 9-5 arcane attack to the Ancient Lich
     * increased the arcane resistance of the Ghoul line from -40% to 20%
     * increased the arcane resistance of the Ghost line from -30 to -10%
 ### user interface
   * OK in the status menu replaced with more informative "Scroll To".
   * add an "Animate Map" option in advanced preferences, to switch flag
     and terrain animation off
   * in menus, remove the lingering mouseover highlighting when mouse leave it
   * for the unit preview in recruit/recall dialog, use the same text coloring
     as in the main side panel.
 ### help system
   * the unit list is now organized by race sub-sections.
   * each ability, special weapon and race description page, display a
     list of hyperlinks to the related units (bug #9567)
   * unit pages have now a field "race" with a link to related race page.
   * hide the "Unknown Unit" page from the left-panel
   * topic with an id starting with a "." are not shown in the left-panel
   * the encountered units list is now correctly updated when using ":debug"
     and ":nodebug" commands
   * faster inital loading of the help.
   * fix some incorrect "open section" UI sound when clicking on links
 ### WML engine
   * new event "turn refresh" occurs after healing, calculating income, and
     restoring movement
   * now tag [filter_adjacent] is supported in the Standard Unit Filter (SUF)
   * now tag [filter_adjacent_location] is supported in the Standard Location
     Filter (SLF)
   * remove the unused [neighbour_unit_filter] animation filter, now that SUF
     does it for us (and better)
   * a minus sign in front of a cardinal direction now reverses it ("-s"="n")
   * now radius expansion is handled last in Standard Location Filters;
     previously it was handled last except before [and], [or], and [not]
   * fix a bug with array.length side-effects causing empty arrays to increase
     to size 1
   * the WML for attack animations has been moved from the [attack] block to
     the [unit] block
   * new effect apply_to=new_animation to add animations to unit
   * the [race] tag has now an "id" key for WML operations. The old "name" is
     now used for user display purposes (currently only in help sections).
     If id is missing, name will be used. There is also a new "description" key
     displayed in the race help page (but currently not used by mainline races)
   * new potentially useful global macros: FULL_HEAL, HIGHLIGHT_IMAGE,
     CLEAR_FOG, NO_SCROLL_TO_AI_LEADER
 ### miscellaneous and bug fixes
   * various bug fixes and code cleanups
   * remove some useless messages from the standard output
   * user-made campaign translations now work again for Windows (bug #9926)
   * fix TC imagepath function for non-playing side and add-on/campaigns icons
   * remove a 500ms delay between AI turns
   * switch to observer viewpoint in linger mode in multiplayer (bug #4072)
   * optimize unit's moves on map with shroud or fog

## Version 1.3.7
 ### Campaigns
   * Son of the Black Eye
     * Fixed bug in 'Saving Inarix' where no user_description
       is generated for Inarix
   * Northern Rebirth
     * Colored portraits for Sister Theta and Ro'Arthian
     * Fixed bug in Anita's death event where Tallin speaks instead of her.
     * Fixed bug in 'Old Friend' where Rakshas' portrait doesn't appear.
     * Balancing and text changes in 'Old Friend'
   * Sceptre of Fire
     * Fixed a crash in the final scenario
   * Under the Burning Suns
     * Fixed invalid side bugs in 'Hunting Trolls'
     * Fixed an invalid terrain bug in 'A Long Night'
     * Fixed the dehydration in 'Across the Harsh Sands'
 ### build system
   * Fixed autoconf/automake builds so that --disable-python-install
     will not try to create the default install directory for python
     stuff when it isn't needed. Otherwise if you aren't root, make
     errors out.
   * If --enable-display-revision isn't enabled SVNREV is no longer
     defined in the build process.
 ### language and i18n
   * updated translations: Danish, Finnish, French, German, Greek, Japanese,
     Lithuanian, Polish, Portuguese (Brazil), Russian, Serbian, Spanish,
     Swedish
   * updated DejaVu font to 2.19
 ### multiplayer
   * new map: Mokena Prairie
   * revised maps: Hamlets, Meteor Lake, 4p Hamlets, Paths of Daggers,
     Loris River
   * improvements to the sound of the countdown timer
   * the countdown timer alert can now start sounding while dialogs are open
   * unit descriptions are no longer evaluated for the recruitment checksum
     and thus avoiding an OOS error when different languages are used.
     The change is incompatible with older trunk versions fixes (bug #9472).
   * Show the (possibly bogus) GPV and fog settings of games with "Use map
     settings" on in a darker font. (patch #771 by uso)
   * the random start ToD option is now properly saved and loaded
     when use map default is used (bug #9532)
   * Changed the Id of the Halberdier from "Halbardier" to "Halberdier"
   * New name for the "Soul Shooter": "Banebow"
   * multiplayer lobby: highlight the names of the players which have joined
     the selected game (bug #7471)
   * multiplayer lobby: tab completion works for all player names now,
     not only for those which are not in a game (bug #9350)
   * enable "teamchat" for observers
   * when use 'map settings' is selected, map settings can no longer be
     changed, if not defined in the map the general default is chosen
   * when cancelling the MP create the changed preferences are no longer
     saved
   * when the MP create is accepted with 'use map settings' the map setting
     parameters are no longer stored as the new preference
   * the recommended settings are added to all standard multiplayer maps
     so that you get these values if you activate 'Use map settings'
 ### graphics
   * fixed a glitch with the undo of recruit or recall on high places.
   * Improve rendering of "black stripes": don't hide footsteps or fog,
     and apply it also on off-map tiles when active.
   * Improve rendering of grid: draw under fog, shroud and "black stripes"
   * Fixed bug #9646: ToD changes not applied to mainmap in replay mode.
   * Remove ToD coloring and brightening of off-map background tiles
   * fix inconsistant highlighting of fog transitions
   * slightly bigger and multi-hex attack direction indicator, fixed
     incorrect scale update when zooming.
   * improve footsteps : better beginning, angles and end, reduce
     cpu cost, fix incorrect left-right sequences and clean some images
   * fixed the glitch which would occur on some maps in the top right
     corner and bottom left corner, these tiles were rendered as void
     instead of a normal terrain.
   * improved sand and desert terrains
   * remove the bars, orb and ellipse of dying animations.
 ### sounds and music
   * new or improved sounds: bat hit, MP countdown timer
 ### map editor
   * the grid is scaled properly again and no longer shown in the offmap area
   * the resize option can now also use the surrounding tiles to expand the
     map in a smart way, this is the default.
   * the map editor can now also shift the origin of the map when resizing.
   * tiles can now have their own image in the editor, defaults to the minimap
     image. Also added some placeholder art.
 ### user interface
   * Enable "Save Game" and "View Chat Log" menu entries in replay mode.
   * Add an additional line below the minimap in the "Multiplayer->Create game"
     screen that displays the size of the selected map. (patch #776 by uso)
   * Add a tooltip with the experience modifier for the units XP display.
   * add an empty attack icon in attack dialog when needed (this also removes
     an error message, bug #9570)
   * better "choose attack direction using the last highlighted hex" on pack
     of units (bug #9649)
   * attack/move cursors and attack direction indicator are not used when
     observing (bug #9610)
   * 'help' and 'more' button in the title screen now also have a tooltip
     (bug #9618 patch #790)
   * added a hotkey for clearing the chat messages (patch #770)
   * Remove the "black stripes" of the enemy that we want to attack.
   * in tiny-gui, now the movement hint also displays the terrain defense.
   * the numbers of "show enemy moves" are now on top of fog and doesn't mix
     with those of the movement hint
   * the sidebar HP tooltip now show resistances of the current unit
   * also highlight the destination hex before an attack+move mouse action
   * more up-to-date mouse state (cursor, highlight, ADI) after some events:
     undo, attack dialog or right-click cancel
   * need only one right-click to open context-menu on an unselected enemy
   * selecting an enemy doesn't directly cancel its reachable zone
   * there's a new "Game Settings" window which shows the basic game settings
     for each player, accessible through the "More" button in the "Status Table"
   * in help, the links to unencountered units now works but have a "(?)"
     and point to an "Unknown Unit" page explaining why
   * the default zoom key now toggles between default and last used zoom. The
     switch is also faster (cached)
   * the unit list now colors the stats of units.
 ### WML engine
   * added effect types new_ability and remove_ability using [abilities] subkey
   * now [base_unit]id= inside [unit] can extend upon existing unit types
   * new tag [filter_radius] to allow greater control over radius expansion
     when using the standard location filter
   * fix a bug where containers could only be stored at the top-level
   * fix a bug where both x and y had to be specified in location filters
   * better handling/reporting of invalid WML variable type usage
   * new key find_in= to allow searching a variable of previously
     stored locations when using the standard location filter
   * new key find_in= to allow searching a variable of previously
     stored units when using the standard unit filter
   * new extra_defines= key to define in campaigns some other preprocessor
     symbol *before* the files are repreprocessed
   * fog and shroud tiles are now defined by the TerrainWML (not [game_config])
   * new key store_location_as= to store generated chamber item locations
   * the border has been made themable
   * Instead of having an empty upkeep type as the default when reading a
     config file being used to normally give leaders a 0 upkeep, now leaders
     (canrecruit=1) are specifically exempted from upkeep and the upkeep
     type defaults to "full" when read from config files the same as for
     using the recruit function. This makes documenting how upkeep works
     simpler (it currently wasn't really correct) without having to make
     leaders loyal. It also will work better for campaigns where the leaders
     change (not that there are any now) so that the current leader is
     exempted and any exleaders pay for upkeep (unless the are specically
     marked as loyal).
   * make random_traits default to "yes". Leaders can't get random traits
     yet, because it breaks MP, but it does give campaign units traits
     by default. It also allows specifying some traits and letting the
     rest fill in randomly. Some traits are always forced on for units
     that can get them (undead and mechanical).
   * not_living tag removed from race, as this information is provided
     by the undead and mechanical traits.
   * the default for turn limit has changed to unlimited, if you have scenarios
     which rely on the old default of 50 please add a turns parameter to it
   * In help, remove the useless "None" terrain type from the terrains list of
     an unit
 ### team color
   * allow color ranges to be defined on-the-fly (like color palettes)
   * now [side] colour=<string> is valid (previously only int)
 ### miscellaneous and bug fixes
   * added a .desktop entry for the editor so that it is shown
     in the kde/gnome menu
   * rewrote the config merge routine (should improve parse time slightly)
   * various code cleanups
   * Isle of Anduin renamed to Isle of Alduin to avoid copyright problems.
   * the assertion 'str.size() <= 4' no longer happens, instead the terrain
      is read as 'void' and an ingame message is shown (bug #9609)
   * IMPORTANT! End-of-scenario no longer takes you immediately to the
     next scenario or the lobby. Instead, you linger in browse mode --
     menu commands for chat, saving games, etc. are available.
     Clicking end-of-turn ends the linger and takes you out.
   * fixed a bug-cheat allowing super-ranged attack in some special cases
     (thanks jgp93)
   * fixed incorrect displayed reachable zone when moving next to an enemy
     in special ZoC cases (skirmish or lvl0)
   * It seemed it's ambiguous whether the GPL license as supplied by
     Wesnoth means GPL 2 or GPL 2+, since Sirp's orginal intent was
     GPL 2+ the license has been changed accordingly.
     (See https://mail.gna.org/public/wesnoth-dev/2007-07/msg00014.html)
   * When advancing from Dark Sorcerer to Lich, you now get the undead
     trait. The way this is fixed can add random traits if the new unit
     type allows more traits then the unit currently has. This would
     cause a problem for multiplayer, but no advancement paths currently
     have this case. You do get to keep your current traits, even if you
     wouldn't get them in the new unit type.
   * the minimum savegame protection is ignored when either the savegame
     or the current game has 'test' as version
   * Add 2 debug commands: "fps" for showing framerate, and "benchmark"
     to force a continuous refresh of the screen (for testing the real fps)
   * fixed bug keeping highlighted the previous selected hex after a
     right-click cancel on map using fog/shroud
   * slightly optimize the showing of the attack dialog
   * some hotkeys (*,~,{,},^,|,@,#,<,&) are now visible in the hotkey settings

## Version 1.3.6
 ### language and i18n
   * updated translations: Danish, Finnish, French, German
 ### multiplayer
   * the random start ToD option is now properly saved and loaded (bug #9532)
   * the map in the lobby could be randomly invisible due to an uninitialized
     variable (bug #9555)
   * the automatic unit description used a non-MP safe way, this has been
     fixed
   * the unit name generation could with different locales call get_random()
     a different number of times. This lead to different names and traits.
     Changed to call random a fixed number of times which fixes the traits.
 ### miscellaneous and bug fixes
   * various code cleanups
   * proper handling of description autogeneration for per level [effect]
     on level 0 units
   * changed the order of which the permanent modifications of a unit are
     evaluated to (amlas, traits, objects)
     (previous order: objects, traits, amlas)
   * updated the copyright info in the source files also made it explicit
     Wesnoth is GPL 2 only (Wesnoth was already GPL 2 only)
   * Music transitions have now smooth transitions based on m_before end
     ms_after wml tags

## Version 1.3.5
 ### campaigns
   * Heir to the Throne
     * The Elvish Lord's and Elvish High Lord's faerie fire attacks have been
       changed from cold to arcane and reduced from 8-3 and 8-5 to 7-3 and 7-5
       respectively
     * The obsolete Cockatrice unit has been removed
     * The bugs in HttT's SoF scenario have been fixed
   * Sceptre of Fire
     * changed Haldric II a bit and gave him new a sprite and animations
   * fixed teamcolourless fake unit moves in TRoW, HttT, EI, SoF, NR,
     TSG and TB
 ### map editor
   * new checkbox for the "delay transition update" option
   * map code has been refactored and separated from the main game
 ### graphics
   * new animations: elvish scout idle
   * fix bug #9398 (attacking units always above defending units)
 ### sounds and music
   * removed unused sounds: firearrow.wav, hatchet.ogg
 ### language and i18n
   * updated translations: Czech, Danish, Finnish, French, Galician, German,
     Greek, Indonesian, Japanese, Lithuanian, Polish, Spanish, Swedish
   * updated DejaVuSans font to version 2.18
 ### multiplayer
   * revised maps: Blitz, Cynsaun Battlefield, Hamlets, Sablestone Delta,
     Silverhead Crossing, Sulla's Ruins, Blue Water Province, Clash
   * fix MP crash on next scenario, thanks to Rhuvaen
   * option to suppress lobby minimaps is gone.
     Its champion concluded it was pointless.
 ### units
   * new "mechanical" trait, meant to show mechanical units are immune to poison
   * balancing changes:
     * changed the 'resilient' trait from +3HP +10% to +4HP + 1HP * unit level
     * decreased the HP reduction of the 'quick' trait from 10% to 5%
     * decreased the HP addition of the 'healthy' trait from +3HP to +2HP
     * added the 'quick' trait back to the Clasher line
     * decreased the blade and impact resistance of saurians from 0% to -10%
     * decreased the fire resistance of saurians from -10% to -20%
     * increased the pierce resistance of saurians from 10% to 20%
     * increased the HP of saurians by 4HP
     * increased the XP requirement of the Saurian Skirmisher and Augur by 2
     * increased the melee attack of the Ruffian from 4-2 to 5-2
 ### user interface
   * allow unfocused widgets to steal the focus instead of just borrowing it
   * frequency of idle animations halved.
   * Help topics for units now have 'advances from' links
   * added experimental new transition between map and background
   * add colors to the statistics of units in help (Hajo's patch #764)
   * now scroll to the selected leader in status table
 ### WML engine
   * now ConditionalWML handles [and], [or], and [not] with in-order precedence
     (this is part of an effort to standardize the behavior of several
     different types of filters)
   * the syntax for logical OR-filters is now "cond1[or]cond2[/or]"
     instead of "[or]cond1[/or][or]cond2[/or]"
   * fix a bug where empty conditionals returned false
   * now [special_filter] supports [and],[or], and [not] (instead of just NOT)
   * now standard unit filter supports [and],[or], and [not] (was just [not])
   * [unstore_unit] can now try to level a unit and does so by default.
     This time added for real, the replay can also handle it(bug #7426)
   * new times= key to apply [effects] more than once (default=once,
     other possible value=per level, i.e. the effect is multiplyed
     by the level of the unit).
 ### miscellaneous and bug fixes
   * fix renames causing OOS when made after moves or recruits
   * fix a minor glitch when selecting the leftmost menu heading
   * added some extra headers for the upcoming gcc 4.3 (debian bug #417764)
   * changed the default plague weapon special macro to always spawn a WC
   * scrolling speed is at 1.3.3 speed again
   * fix a growing cache bug with stoned units facing west.
   * in tiny-gui, fix badly scaled hp/xp bars at default zoom level

## Version 1.3.4
 ### campaigns
   * Two Brothers
     * made the 2nd guards event in scenario 3 depend on if the first answer
       was correct, now the 2nd event triggers later when the first ends in a
       fight
     * added one row of terrain in the 3rd and 4th scenario to make it look
       better with the new "boardgame style"
   * Heir to the Throne
     * edited the maps of the scenarios: "The Elves Besieged", "Blackwater
       Port", "Bay of Pearls", "Crossroads", "The Siege of Elensefar",
       "Dwarven Doors", "The Lost General", "Elven Coucil"
     * adapted the names of the maps and cfgs of the HttT to match the
       scenario identification keys and the order of the scenarios
     * added the map of "Cliffs of Thoria"
     * changed one of the Orcish Warlords into a Sovereign and another one
       into a Slurbow for the variety in "Dwarven Doors"
     * fixed the bug that made the undead leader unable to recruit because of a
       misspelled unit type in his recruit list in "The Lost General"
   * The Rise of Wesnoth
     * added the level 3 outlaws ( Hunter, Outlaw Ranger, Fugitive, Highwayman )
       as a campaign - specific units in The Rise of Wesnoth
     * added 2 extra turns to finish "Temple of the Deep" on all difficulties
   * Eastern Invasion
     * Reduced the difficulty of "Northern Outpost" scenario by making the
       villages spawn averagely 1 less outlaws per village
     * Changed the way Ovaec joins the protagonist in "Two Paths".
       Now he keeps all the XP he might get in "Northern Outpost"
   * Descent into Darkness
     * Fixed the unknown unit type bug in "Forever and ever, amen", the bug
       that made the Foolish heroes have 30 hitpoints and a broken dialog,
       all in the same scenario
 ### map editor
   * the minimum map size is reduced to 1
   * fixed the random map generator
 ### graphics
   * added sickle and scythe attack icons
   * added weapon-shop tent and oak tree
   * added automatic side-coloring of custom flags
   * added a general backgound to make the game look better
     on really big screens
   * added a blur effect to most ingame dialogs
   * new animations: troll whelp idle, young ogre idle, skeleton idle,
     human loyalist general idle, human loyalist pikeman idle,
     elvish fighter idle, master at arms crossbow and defend
   * small improvements of footsteps: no time-of-the-day coloring and
     better scaling
 ### language and i18n
   * updated translations: Czech, Danish, French, German, Italian,
      Japanese, Polish, Spanish, Swedish
   * new translations: Lithuanian
 ### units
   * added a wolf "monster"
   * balancing changes:
     * healthy units now can rest even when they move
       instead of having a double resting bonus.
     * increased the HP of the Skeleton Archer from 30 to 31
 ### WML engine
   * removed deprecated keys image_healing and image_halo_healing
   * empty user_description now remains empty unless generate_description set
     (bug 8522).
   * empty user_team_name remains empty (bug 9310).
   * set_variable has the key time=stamp now, to get a timestamp in milliseconds
   * move some terrain masks definition in [game_config]
   * made defense_weight work again for values > 0
   * fixed some minor bugs with "sighted" event, "die" event, and [kill]
   * now [allow_undo] should work as expected in sub-commands such as [then]
 ### user interface
   * most popup windows are now buttonless with 'click anywhere to continue'
     behavior; to indicate this, such windows are translucent.
   * maps smaller than the screen are now shown centered on the screen
   * the movement hint now display a "(1)" turn to reach, where the unit
     will have no moves left (distance, village or ZoC)
   * fixed bug with the number of turns to reach when crossing several ZoC
   * ability to view a list of MP servers with the Join Game dialog
   * fixed blinking tooltips when clock is updated (bug #9209)
   * footsteps are now erased at the end of the unit's move
   * reduced "sea sickness" effect when scrolling at accelerated speed
   * allow to select item in menu with right-click
   * right-click outside of a cancelable dialog to close it
   * added experimental new background.
     read comment before draw_background() in display.cpp
   * the movement hints use multiline, colors and bigger font
     to display terrain defense and turns to reach
   * restore search of no-team-only map labels
   * ability to delete Add-Ons
   * make disabled buttons more obviously disabled
   * allow to choose the direction from where attacking, even when you
     are just near the enemy unit.
   * new indicator for the attack direction
   * new method to choose attack direction, use now the last highlighted hex
     instead of clicked triangle.
   * improve units list dialog: add a cancel button, fix a bug when selecting
     the first unit, preselect the on-map-selected unit and highlight on map
     the selected one.
 ### Miscellaneous and bugfixes
   * fixed a lag in the path rendering when there is a lot of units (bug #9268)
   * fix bug #4299: word wrap for menus with very long option strings
   * various bugfixes and code cleanups
   * removed zipios support
   * poisonned/stoned text is properly centered again
   * fixed various bugs with stoned units, now no moves left and blind.
   * fixed a bug causing instantaneous move of unit when using acceleration
   * fixed briefly invisible unit when scrolling to show a move
   * fixed an invalid letter in the random winter scenario
   * fixed a invalid memory access if a map has width of 0 (bug #9301)
   * reduce memory usage for the default zoom level
   * fix various cheat-bugs to detect hidden enemies: using resistance info
     (bug #9119), search by name (bug #9314), change of hidden icon (bug #9288)
   * fix jerky overlay of moving unit, now displayed as bars and crown.

## Version 1.3.3
 ### campaigns
   * Northern Rebirth
     * Completed Scenario 'Ray of Hope'.
   * Sceptre of Fire:
     * imported from Wescamp
   * Son of the Black Eye:
     * Merged into mainline for testing.
   * Descent into Darkness
     * added to the pack
     * new Troll Shaman graphics
   * Under the Burning Suns
     * new WML in first and second scenario
     * tweaked second scenario map
     * removed a bunch of custom units
     * removed a bunch of custom terrains
     * new playable unit in scenario 2
     * new dehydration logic in scenario 2
     * new ambush logic in scenario 2
     * new Troll Shaman graphics
   * The South Guard
     * improved Deoran graphics
   * The Rise of Wesnoth
     * new Wose Sapling, Warrior King graphics
 ### graphics
   * the leader crown don't hide anymore the top of hp/xp bars (bug #9120)
   * better flag icons for the status bar
   * taller flags that don't get hidden behind units so easily
   * fixed some drawing glitches in the top row (bug #8739 and #8071)
   * it's now possible to zoom in till the theoretical minimum of 4 pixels
     per hex.
   * hp/xp bars in tiny gui have better proportions
 ### sound and music
   * new or revised sounds: troll hit & die
   * added sounds for when a unit is slowed or poisoned
   * added a music track containing only silence (for stopping all music instead
     of just changing it)
 ### language and i18n
   * updated translations: Bulgarian, Chinese, Czech, Danish, French, German,
     Italian, Polish, Spanish, Swedish
   * updated man pages: Danish
   * fixed word wrapping in tooltips for Asian languages (or very long words)
   * updated DejaVuSans font to version 2.17
 ### units
   * balancing changes:
     * converted the cold melee attack of the Lich and Ancient Lich to arcane
     * decreased the arcane melee attack of the Wraith from 7-4 to 6-4
     * removed the 'fearless' trait from humans, orcs and mermen
     * added 'firststrike' weapon special to the pierce attack of Drake Clasher
     * decreased the arcane ranged attack of the White Mage from 8-4 to 7-4
     * decreased the arcane ranged attack of the Mage of Light from 10-4 to 9-4
 ### multiplayer
   * added maps: Xanthe Chaos, Auction-X
   * revised maps: Den of Onis, Hamlets, Meteor Lake, Sablestone Delta,
     Silverhead Crossing, Blue Water Province, Castle Hopping Isle, Loris River,
     Crusaders Fields, The Manzivan Traps
   * Team Survival: the teams are now set correctly and translatable
 ### map editor
   * drawing terrain no longer erase starting positions
   * added an option "None" in the starting position menu
   * fixed a bug when overwriting starting positions
   * shorter starting time
 ### WML engine
   * removed deprecated special= support in [effect]
   * removed deprecated 1.2 map format support
   * [store_villages] and [store_locations] now use the standard location
     filter, meaning they accept all the possible keys of [filter_location]
   * [store_villages] must now use owner_side= instead of side=
   * replace [own_village] with macro {OWN_VILLAGE X Y SIDE}
   * new conditional tag [have_location], tests true if any location passes
     the standard location filter provided inside the tag
   * now the standard location filter supports [and] and [or] tags
   * new key flag_icon for [side] to change the flag icon in the status bar
 ### user interface
   * fixed incorrect active flag in the status bar when using custom flag
   * restore a lost feature: if acceleration is on, pressing shift uses normal speed
   * fixed bug #9071 (incorrect behavior of the "delete" key at end of line)
   * fixed various acceleration bugs (things not being properly accelerated)
   * restored status bars during fights
   * floating numbers now take acceleration into account
   * display defense in the status bar of static units (towers) or
     units placed on impassable terrain
   * allow middle click on scrollbar to jump to a position.
 ### WML tools and scripts
   * New tool, wescamp_import, automates the shuffling of translation files
     and textdomain strings, need to import a campaign from WesCamp
     to mainline.
   * New tool, change_textdomain, makes it easy to change the name of UMC
     after import to mainline.
   * Old campaign_delete.pl and campaigns_client.pl have been replaced
     with a new campaigns_client.py.  Main new features are
     (a) you can download sets of campaigns using RE wildcards, and
     (b) It's possible to give a BfW version in place of a port number
     and it will select the right port.
 ### Miscellaneous and bugfixes
   * 1.2 savegames are no longer compatible and thus not loaded
   * fixed a crash if an invalid scenario was loaded (bug #9049)
   * fixed bug 8522.  Units without a name will be named after the unit_type.
   * fixed bug #9103 (AI causes crash when using static units like towers)
   * fixed a crash if a units poison attack has a cth of 0 (bug #9020)
   * no longer allow undo if a traitless unit reveals fogged and/or shrouded
     terrain (bug #9171, patch #739)
   * fixed a bug where loading didn't clear the halo data (bug #9144)

## Version 1.3.2
 ### campaigns
   * Heir to the Throne
     * In Home of the North Elves, Eonihar the rider will join you permanently
       and the other riders that find you will be controlled by the AI
     * updated images for Delfador
     * made the narrator talk about gryphons at the end of Northern Winter only
       if the player has actually gotten the eggs
   * Tutorial
     * merged all unit images used in the tutorial from httt
     * increased number of turns from 20 to 26 in the 2nd scenario
   * made some units not appear in the unit help anymore (invisible helper
     units, duplicate desert elves and such)
   * The South Guard
     * better placement and duration of story images
     * graphical enhancements including a new ethiliel portrait
     * added 'Ruffian' an L0 outlaw to give the player something easy to kill.
   * Under the Burning Suns
     * Scenario 3: Made orc ambush happen on a random turn
   * Northern Rebirth
     * Version 18.6 merged in from WesCamp.
 ### graphics
   * added the --max-fps command line switch
   * changed the drawing logic so it no longer delays when the drawing is
     lagging
   * improved the look of the main menu and tips-of-the-day boxes
     in the title screen
   * bugfix: units will now fade out properly at the end of death animations
   * rewrote the halo render engine which solves a few minor glitches,
     but most importantly speeds up the drawing of the halos.
   * units are scaled properly when zooming also with tiny gui (bug #8703 and
     bug #6570)
   * haloes in tiny gui are now scaled (before unscaled, thus a factor 2 bigger
     as intended)
   * the movement text is also scaled now (bug #6876, patch #693)
   * fix glitches when scrolling the map if zoomed out (bug #8768)
   * fix the right border glitch of the map when zooming (bug #6060 and others)
   * flying units are no longer buried in a castle (patch #709)
   * fix the position of some halos when unit has a height offset (patch #710)
   * Submerged units are no longer drowned when zooming.
   * scenery
     * added animated windmill, fancy and damaged tents, icepack, and shipwreck
   * fix problems with colour cursor in fullscreen (slow speed, bug #7555 and
     bug #6052)
   * new color cursors
   * fixed some scrolling corner cases (eg. fights happening partially
     offscreen)
   * automatic scrolling accelerates now (like a physical mass would)
 ### sound
   * new or improved sounds: hatchet
   * sounds for user interface events
   * experimental ambient sounds for when night falls and morning arrives
   * "Breaking The Chains" (freedom.ogg) moved from NR to mainline
   * All music tracks now have complete Ogg tags.
 ### language and i18n
   * updated manual: French
   * updated translations: Bulgarian, Chinese, Czech, Danish, Dutch, French, German,
     Greek, Hungarian, Italian, Norwegian, Polish, Portuguese (Brazil), Spanish,
     Swedish
   * new translations: Indonesian
   * updated DejaVuSans font to version 2.16
   * removed FreeSans support since all the codes needed should be coverd by
     DejaVuSans now
 ### multiplayer
   * revised maps: Blitz, Castle Hopping Isle, Wilderlands
   * The countdown timer is now coloured properly when the remaining time is
     less than 2 minutes.
   * observer can save a replay of the game again
 * units:
    * balancing changes:
      * converted the cold resistance of the Elvish Sorceress line
        to a holy resistance
      * decreased the holy resistance of the Orcish Assassin line from 20% to 0%
      * remove the 'quick' trait from the Clasher line
      * decreased the holy resistance of the Sky Drake from 20% to -30%
      * decreased the holy resistance of the Hurricane Drake from 20% to -30%
      * decreased the holy resistance of the Mermaid Priestess from 40% to 20%
      * decreased the holy resistance of the Mermaid Diviner from 60% to 40%
      * decreased the holy resistance of the Dwarves from 20% to 10%
      * increased the holy resistance of the Ghost line from -60% to -30%
      * enabled Lich and Ancient Lich to go on deep water and gave them submerge
      * Level 0 Ruffian & Woodsman added.
 ### terrain system
   * added layer support to the terrain letters, two layers are supported
 ### map editor
   * added an option to delay the complete redraw of the map on command.
   * fixed a bug causing the loss of editor's hotkeys when loading a map
 ### WML engine
   * new [own_village] condition to check if someone owns a particular village
   * new [store_villages] tag to store village locations
   * new [store_turns] tag to store the number of turns remaining
   * all [store_*] tags get a default variable name
   * use complex variable substitution almost everywhere
   * new key for [set_variable], literal=, to avoid variable substitution
   * [effect] can now toggle the zoc
   * [effect] can now apply new portrait images and unit type descriptions
   * [effect] can now filter on unit_gender
   * new key for [variable], boolean_equals=, to test boolean equality
   * remove some old backward compatibility support
   * set_name in attack modification [effect] no longer change the weapon's
     description, use the new set_description instead
   * hit=yes in anim is now an alias of hit=hit,kill instead of hit=hit
     this should fix the sliding bug and fix more bugs than it creates
   * friendlier boolean matching for unit [filter] and unit [wml_filter]
   * fixed [special_filter] not working properly (#8212)
   * fixed [variables] not working properly for Multiplayer mode
   * new [set_menu_item] action, to allow custom WML items in the context menu
   * fixed a crash when a WML event killed a unit while dying in an attack
     which the attack engine didn't expect (bug #8814)
   * new key for [time], sound=, to specify a list of sounds that can play
     when ToD changes
   * more powerful [filter_location], now accepts radius=, [not], and
     [filter] to match units at the location
   * added [deprecated_message] so WML can also trigger the deprecated
     messages; used to tag macros scheduled for removal
 ### user interface
   * new sounds for user interface events
   * added the option to show warnings about deprecated WML usage
   * added the "ignore all" option to continue observing during massive OOS
   * show total amount of visible villages (patch #698)
   * added drag & drop to move units
   * mousewheel now by default scrolls vertically,
     and horizontally if alt is pressed.
   * added support for horizontal mousewheel (patch #644)
   * don't change the wait cursor when moving on the map (bug #8915).
   * update the highlighted hex after a scrolling with keys or mousewheel
 ### WML tools and scripts
   * Reorganized: these now live in data/tools/
   * New tool, macroscope, generates cross-reference reports on macro usage.
     Among other things, it can be used to find unresolved macro and
     resource-file references, or spot unused macros and resource files.
   * New tool, upconvert, handles converting maps and resource file names
     between versions.  It replaces map_convert.pl, which is now gone.
     It should be used to update UMC, though it doesn't do a complete job
     (which requires some human judgment).
   * New tool to create the unit tree in html, as used for units.wesnoth.org
     (written by elricz)
   * The old find-unused-images and find-unused-sounds scripts are deleted
     (replaced by macroscope).
   * The Perl random map generator (unmaintained since 2003, only
     generated old-style single-letter maps) has been removed.
 ### miscellanous changes and bug fixes
   * a friendly healer will now stop poisoned unit to lose HP
   * a unit that dies while attacking will now correctly play its own death
   * turn bell can sound if other soundFX are turned off
   * the first turn starts again with the proper time of day (bug 8637)
   * removes the scaling handling in the halo, removing quite some code
   * fixed various problems found by valgrind, note some invalid access
     are now protected by asserts, so there might be more crashes instead
     of silent memory corruptions (bug #8715, bug #8756 and bug #8839)
   * fixed beginning-of-scenario saves containing scenario WML (bug #8698)
   * fixed "End Turn" button clickable after Ctrl-F (bug #6556)
   * fixed autosaves not getting deleted for last campaign scenario and
     multiplayer (bug #8762)
   * Performance: wml compression is done in the helper threads
     instead of the main thread
   * Added some protection against crashing when attacking with units with
     a high or negative amount of hitpoints (bug #6154, bug #7955, bug #8541)
   * fix skirmisher filtered by terrain not working properly.
   * fixed Ethiliel not showing up in the scenario "Vengeance" of TSG
   * with a move attack action if the moveto event moves the defender
     Wesnoth would crash (part 1 of bug #8882)
   * fix missing filterings on some animations
   * renamed holy damage type to arcane
   * increased required version of python from >=2.3 to >=2.4
   * fixed detection of installed python versions to work on systems
     that do not have python installed at /usr/ (like MacOSX using fink)

## Version 1.3.1
 ### Campaignd
   * first argument is number of threads to use, defaults to 5
 ### campaigns
   * Eastern Invasion:
     * Drowned Planes: the dragon starts on grassland
     * The Crossing: fixed a bug which prevented the undead leader spawned
        at turn 8 to have a keep
     * The Duel: the keep is replaced by a castle tile instead of grass
   * Heir to the Throne
     * removed Mountain Pass and Valley of Statues from the repository
     * added underpinnings of an unlockable, optional, bonus scenario
     * Battle_for_Wesnoth: allow Asheviere to recruit non-scout units
     * In Elven Council the leaders are no longer standing in a keep
       but in a normal castle tile
   * The South Guard
     * Proven by the Sword: tell Deoran to go to Westin if the citadel
       is secured (fixes #7360)
   * Two Brothers
     * updated the portrait of Bjarn to match the new mage colors
     * removed Skeletons as enemies in the first mission on easy, and
       Skeleton Archer as enemy on hard
     * added Ghouls as enemies for all undeads
     * some map updates to make them easier (2nd mission) or look better
       (all the other missions)
 ### editor
   * allow maps to be loaded from scenario files and written back to them
   * upon entering a new starting position the underlaying terrain
     remains unchanged
   * when overwriting a starting position and undo that action
      only the terrain was undone, not the starting position that's fixed
   * flipping a map over the Y axis no longer resizes the map
   * flipping a map over the X axis uses slightly different filler rules
   * added the paste option to the edit menu
   * added the option to split the terrains is groups
   * the brush size in non-drawing mode is ignored
   * added an alpha blended preview of the selected terrains
     upon drawing. The optimal alpha value needs some tuning, so
     there's a setting, which is not stored in the preferences
 ### graphics
   * missiles now take unit_height_adjust into account
   * improved layering system for terrain graphics
   * fixed terrain selection probabilites (patch #617)
   * stoned units do not use a separate cache
   * no more "darkened" cache. we now overlay terrain/darken.png
   * added the option to draw overlays on terrains filter by location,
     selected and mouseover hex
   * items with a halo have their halo hidden when under a shroud (bug #8523)
   * the visibility icon of an unit is now properly updated (bug #8107)
   * terrain
     * added peaks to the impassable mountains
     * added bridges crossing swamp and deep water
     * added farmland
     * added elvish castle
   * unit art
     * added a recruit animation for Skeleton
     * new death animations: Orcish Assassin, Grunt, Undead Ghoul,
       Ancient Wose, Elder Wose, Thief
     * changed name on Dwarvish Warrior art files to Dwarvish Steelclad,
       to conform to unit .cfg file name
     * new attack icons: undead axes, undead crossbow, pick axe, glaive,
       elven staff, mace-and-chain
     * new baseframes: all Mages, Footpad
     * new and changed animations:
       * Elvish Hero: melee attack
       * Elvish Sorceress line: magic shield
     * updated portraits: Troll
   * other images
     * animate-able campfire
     * crown icons for heros and expendable allied leaders
     * star shaped "ellipses" for leaders and hero units
     * updated and added many misc items
     * modified hex-grid
     * slightly modified logo
   * fix gryphon not being in the center of their hex
   * fixed incorrect WML in some units causing female variations to use male
   * fixed some cases where fog was updated without being redrawn
 ### language and i18n
   * updated translations: British English, Catalan, Czech, Danish, Dutch,
     Estonian, French, German, Greek, Italian, Polish, Russian, Serbian,
     Slovak, Swedish
   * updated DejaVuSans font to 2.15
   * fixed huge list of spelling mistakes in the en_US version
   * allow sighted messages to be translated with correct plural form
     (bug #8161)
   * moved the translation for campaign-specific units into the
     campaign textdomain (some campaigns did still use the
     wesnoth textdomain for their units)
   * made the unit generated names translatable
   * switch to mainly using single space
 ### multiplayer
   * game management
     * replace "Downloading next level..." by "Receiving data..."
       when scenario don't have next_scenario
     * replace "Downloading next level..." by "Downloading next scenario..."
     * fixed controller change not sent to all clients (bug #8138)
     * client now tells the server if a game ended in victory or defeat
     * configurable castle size for random map generator (patch #598, FR #3232)
     * team names translation now supported
     * sped up the lobby, the lag is much lower now (patch #676 and patch #677)
   * scenarios
     * scenarios can set faction, recruit, leader, and some other
       initial settings previously ignored in multiplayer
     * added option to have random time of day in begin of multiplayer game
   * maps:
     * new multiplayer maps: The Manzivan Traps, 4p Hamlets
     * revised maps: Blitz, Charge, Den of Onis, Hamlets, Meteor Lake,
       Sablestone Delta, Sulla's Ruins, Wesbowl
     * made the AI recruit less scouts by setting the village_per_scout
       parameter
   * Wesbowl: the respawned units are fully healed again (bug #7776)
 ### Python AI
   * Added various input validations
   * Set Python errors upon error
   * added support for optipng optimization in the compilation process
 ### terrain system
   * The entire underlaying system has been converted to a new system,
     this might render some UMC broken, read this forum thread for more details
     https://www.wesnoth.org/forum/viewtopic.php?t=14910
   * Terrain of the starting position can be changed in the map,
     the automatic conversion to a keep is no longer done
   * New definition for the maps in terrain-graphics
   * All keeps have their own letter
   * Added wildcard matching support in a few cases
   * Added support to customize the village mouse-over texts
     depending on the ownership
   * Added a conversion script to convert old maps
 ### units
   * balancing changes:
     * implemented the holy damage type redesign as discussed on the mailing list
     * increased the pierce resistance of the Gladiator line from -10% to 0%
     * decreased the cold resistance of the Armageddon Drake from -30% to -50%
     * increased the HP of the Longbowman from 45 to 51
     * decreased the XP requirement of the Longbowman from 80 to 68
     * increased the HP of the Master Bowman from 58 to 67
     * decreased the HP of the Arch Mage from 57 to 54
     * removed the Shock Trooper line from random_leader
     * increased the melee attack of the Shyde from 4-2 to 6-2
     * increased the ranged slow attack of the Shyde from 7-2 to 6-3
     * increased the ranged magical attack of the Shyde from 7-3 to 8-3
     * increased the melee attack of the Enchantress from 5-2 to 6-2
     * increased the melee attack of the Sylph from 5-3 to 6-3
     * increased the ranged slow attack of the Sylph from 6-4 to 6-5
     * increased the HP of the Sylph from 58 to 60
     * increased the XP requirement of the Guardsman from 42 to 47
     * increased the HP of the Stalwart from 48 to 54
     * increased the XP requirement of the Stalwart from 65 to 85
     * increased the HP of the Sentinel from 56 to 68
     * removed the Stalwart line from random_leader
     * increased the XP requirement of the Gryphon Rider from 36 to 38
     * increased the HP of the Gryphon Master from 46 to 51
     * decreased cost of the Mudcrawler from 9 to 5
     * Dark Adepts no longer get the 'strong' trait
     * decreased the HP of the Shadow from 26 to 24
     * added the skirmisher ability to the Shadow line
     * increased the melee attack of the Walking Corpse from 5-2 to 6-2
     * made the Walking Corpse line get the fearless trait
     * increased the cost of Walking Corpses from 6 to 7 gold
     * increased the HP of the Goblin Rouser from 26 to 31
     * increased the melee attack of the Goblin Rouser from 5-3 to 6-3
     * increased the moves of the Troll Rocklobber from 4 to 5
     * decreased the HP of the Troll Rocklobber from 53 to 49
   * adapted WML to use [if] statements to differentiate hit and miss frames
   * new traits
     * healthy (+3HP, +4 rest healing), fearless (ignore unfavorable ToD)
   * new units
     * added the Armageddon Drake (not used in any mainline eras)
     * added dwarvish walking corpse/soulless variations
   * added a generic macro to have a standard filter for all idle anim
   * fixed a bug which prevented a Mermaid Siren picking up a storm trident
   * fixed a bug which caused the slow effect being applied twice (bug #8458)
 ### user interface
   * use "Save Replay" instead of "Save Game" when asking to save a replay
     (bug #7256)
   * fixed enemy Gold shown twice in debug mode in status table (bug #6895)
   * added an option to hide minimaps in the multiplayer lobby
   * added menu items for saving the current map
   * added turbo speed setting and changed ui accordingly
   * turn bell moved to sound tab in preferences
   * new volume slider for the turn bell
   * disabled inactive sliders instead of hiding them
   * sliders can be adjusted with keyboard (left and right arrow)
   * sidebar reports related to gold, units, villages and time
     get greyed out during other team's turns
   * improve the tiny gui support (resolution of 320x240 pixels)
   * when sighting friendly units, the message is now blue
   * show leader name in statistics title (patch #604, FR #6932)
   * show the team a side belongs to in the status table.
   * show leader's name and colour at status table even when that team is fogged
     (patch #605)A
   * changed the zoom hotkeys from 'z', 'x' and 'c' to '+', '-' and '0'
   * fix the completion when controlling multiple sides (bug #8101, patch #653)
   * smarter focus handling when user input is irrelevant to the current focus
     but relevant to another widget
   * fix the "keylogger" effect when joining the MP Lobby
   * menus can now stay scrolled to the bottom if they were already scrolled
     to the bottom
   * scrolling performance improved
   * overall game performance improved when many units are visible
   * better performance when mouse is over a unit that sees many enemy units
   * show full username at bottom of screen when moused over in MP Lobby
   * animations draw faster now and the speed is independent of the resolution
   * fog/shroud is cleared after attack-move (bug #7131)
   * made some tooltips visible again (bug #6702 and bug #8380)
   * send a whisper message to a player when double clicked on the player list
 ### friends list
   * added a friend list in MP lobby, they will display a special icon in lobby,
     and you can be warned when they join the lobby even when not receiving
     lobby joins
   * added an option to specify what you want to see as joins: all, friends,
     none
   * added a sub tab of multiplayer to the preferences menu to view and edit
     your friends and ignores list
   * if you choose to sort the lobby list your current login name will be
     displayed on the top of the list
 ### WML engine
   * fixed era events not working on non hosts
   * added the the ability of passing a conditional statement in the [option]
     child of [message] if show_always=no to show this option only if the
     statement is passed.
   * added location range filter to [filter_location] in standard unit filter.
   * added prerecruit event that takes place after the unit is created but
     before it is displayed.
   * added ability to define color ranges and palettes in scenario, units,
     and unit modification effects.  Added ability to add image modifications
     in a unit modification [effect] tag
   * SUF can now filter on anything that is also available throught
     unit variables, using a [wml_filter] block
   * overhaul of the team coloring engine.  Now it is possible to apply
     multiple TC to an image.
   * added fourth color in team_rgb definitions for representative color
     in minimap
   * added a horizontal/vertical flip function to ImagePathWML
   * added a 'hide_help' key that prevents a unit type from being listed
     in the in-game help (bug #5701)
   * added an 'allow_new_game' key (default=yes) to prevent [multiplayer]
     scenarios from showing up in the multiplayer game creation interface
     (FR #6397)
   * new [set_specials] tag to allow custom specials in [effect] tags
   * new 'remove_specials' key to allow the removal of specials in [effect] tags
   * obsolete the 'set_special' key in [effect] tags
   * correct handling of UTF8 filenames under windows
   * animations can now use standard unit filters
   * animations can now use standard unit filters on the secondary unit
   * animations can now use standard unit filters on any neighbouring unit
   * fighting animations can now be filtered by swing number, damage done and
     normal attack filters (both attacker and defender)
   * made some animation parameters progressive (allows a value to be changed
     gradually to another value over time)
   * new progressive parameters: 'alpha' (opacity of a frame),
     'offset' (the position relative to the faced hex), 'blend_color'
     and 'blend_ratio' (to mix a given color with the frame)
   * units can now have a recruit animations that will be played on unit recruit
   * units now have idle animations, played when they've been idle for some time
   * units now have leveling up animations
   * units can now have victory animations, usable with the WML tag
     'victory_anim'
   * units can now have healed animations, used when they get healed
   * units can now have poison animations, used when they suffer poison damage
   * [event]s can now be written also inside [era]s (the [event]s are included
     in every scenario played using that era)
   * new operations for set_variable: divide and modulo
   * Animations now use duration= tag instead of begin= and end= (backward
     compatible)
   * Animations now have a frequency= filter to allow to tweak relative rarety
   * [message]s now use the duration= tag instead of [delay]ing afterwards
   * the filtered unit can be accessed in SUF using $this_unit
   * the primary unit can be accessed in events using $unit
   * the secondary unit can be accessed in events using $second_unit
   * added support for random_start_time in [scenario] (feature request #8024)
   * extended label to support team only and colors
   * gender-specific forms for trait names are now possible.
   * setting a WML variable to a random value now works correctly in MP games
   * text displayed when a unit is ambushed is now customizable with the new
     alert= key of the [hides] ability (FR #8264 2.)
   * unit animation terrain filter works properly again.
   * unit animation terrain filter now supports wildcard matching.
   * the encountered terrains are now written in a new format.
     this means the user has to rediscover all available terrains.
   * fixed a bug which returned wrong values if a random number was negative
   * max level advancements now raise the advance and post_advance events
   * [effect] can now remove attacks with the remove_attacks key
   * [unstore_unit] can now try to level a unit and does so by default (bug #7426)
 ### sound
   * new or revised sounds: morning star, holy magic.
   * add advanced sound tab to pref allowing you to play with sample_rate and
     buffer_size options
 ### misc
   * --validcache runtime switch to force assumption that cache is valid
   * add two new debug command set_var and show_var to check variables
     within the game easily
   * better handling of SDL_ListModes return code (no user impact)
   * first turn of a game is saved again (fixes bug #7909 and bug #8117)
   * reduce CPU usage by removing calls to SDL_GetTicks for idle animations
   * changed network thread management to be more scalable, both with
     an upper and lower number of threads
   * enabled python as default, to disable it use --disable-python
   * various code cleanups
   * replaced KDE/Gnome desktop support with generic freedesktop standard
   * if a unit gains enough XP to gain 2 levels, this will be done directly

## Version 1.2
 ### campaigns
   * Two Brothers:
     * updated portraits for Arne and Bjarn
   * Under the Burning Suns:
     * changed some maps to fix glitches
     * fixed some image paths
     * new naga hunter portrait from Ranger M
     * uses the new lava transitions
 ### graphics
   * terrain:
     * new ice transition
     * modified swamp color
   * units:
     * new team-colored base-frames: grand marshal, human assassin, sergeant
     * halos for elvish shaman line attacks
     * new lightning bolt for Delfador
     * new fireball attack
     * new attack icons for wose crush and troll hammer
   * misc:
     * new icons for preferences
     * updated hex grid
 ### language and i18n
   * new man pages: Russian
   * updated translations: Catalan, Dutch, French, German, Greek, Italian,
     Norwegian, Russian, Slovenian
   * updated DejaVuSans font to 2.13
 ### multiplayer maps
   * revised maps: Blitz, Charge, Den of Onis, Hornshark Island, 4p Morituri,
     Meteor Lake, Lagoon
 ### multiplayer
   * fixed some automove bugs (bug #7494 and bug #7251)
   * fixed passing controller in mp campaign (bug #7936)
   * added help strings so that long map names can be read
     from the MP Create screen
   * used small bold text for "Load Game..." item
   * support added for allow_new_game=no (for MP Campaigns)
 ### misc
   * fixed the AI usage of attack_weight (bug #7953)
   * the displayed terrain now has a space before the opening bracket
   * fixed a crash when image is missing (patch #647, thanks to pauli)
   * fixed a crash when attacking a sea serpent (bug #8075) (patch #648)

## Version 1.1.14
 ### campaigns
   * Eastern Invasion:
     * added a starting position for the seventh enemy (bug #7918)
   * Heir to the Throne:
     * fixed the signpost image in the narration (patch #638)
   * The Rise of Wesnoth:
     * fixed bad glitches in the map Rise of Wesnoth
   * The South Guard:
     * made it possible to finish "Into the Depths"
       even without enough gold to pay the trolls
 ### graphics
   * terrain:
     * new special transition between chasm and dwarvish castle
     * new lava transitions (looks like completely new lava)
     * fixed glitches with impassable mountains at the edges of maps
     * resolved all glitches with cave walls
   * units:
     * new teamcolored baseframes: Halberdier, Heavy Infanfantry,
       Pikeman, Shocktrooper, Siegetrooper
     * new baseframe: Yeti
     * some new skulls for skeleton units
   * misc:
     * new attack icon for wooden sword
     * new dwarven gate
     * new portrait for Asheviere
     * new, better looking hexgrid
 ### language and i18n
   * switch back to non-utf8 locale definition to fix some problems,
     --enable-dummy-locales should work again
   * updated man-pages: French, German
   * updated translations: Czech, Esperanto, French, Greek, Italian, Russian
 * multiplayer maps:
    * revised multiplayer maps: 3p Morituri, 4p Morituri, Blue Water Province,
      Den of Onis, Hexcake, Isar's Cross, Merkwuerdigliebe, Meteor Lake,
      Sablestone Delta, Sulla's Ruins
 ### misc
   * fixed a crash in the recall event_handler
   * fixed game crashes during AI turn (#7988)
   * fixed graphic glitch with large units
   * fixed sighted event with delay shroud during move&attack
     and also only move
   * add script to strip ICC profiles from images
   * modified wesnoth-pngcrush to use optipng instead of pngrewrite
     (a lot slower than the old script, but good compression results)

## Version 1.1.13
 ### graphics
   * updated races to support team color: elves, goblins, lizards, ogres,
     orcs, trolls, woses, many dwarves, many humans, many undead, and galleons
   * new baseframes: elvish shaman line, troll whelp line, wose line, galleons,
     ogres, many other units
   * new or improved death, attack, and defense animations for various units
   * new attack icon: ballista
   * terrain
      * various minor terrain and item improvements
      * resolved the worst glitch between chasm and cave-walls.
   * additional star-shaped ellipses for designating special units.
 ### language and i18n
   * updated manpages: French
   * updated manual: Italian
   * updated translations: Czech, Esperanto, French, German, Italian
   * updated DejaVuSans font to 2.12
   * reworked language files
   * added descriptions to the weapons of some campaign units
     to make sure they are correctly translated
 * multiplayer maps:
    * revised multiplayer maps: Charge, Hamlets, Hornshark Island, Silverhead
      Crossing, Sulla's Ruins, 3p Morituri
 ### misc bugfixes
   * several minor bugs
   * other bugs people did forget to mention that they were fixed
   * unit frames are now centered instead of aligned on upper left corner

## Version 1.1.12
 ### user interface
   * move chat line slider bottom (Multiplayer options)
   * display started game with vacant slot with yellow font
   * fix allow to move other player's (bug #6451)
 ### sound
   * add sample_rate option (read only) to solve cracking sound
     on some sound card (Intel HDA, cmipci, ...) (bug #7507)
 ### graphics
   * new portraits: Human Bowman
   * unit animations: Elven Hero's melee attack, Dragonguard,
     Saurian mages, Young Ogres, Troll Warrior, Troll Rocklobber
   * team color: Trolls, Woses, Orcish Assassin, Thief, Rogue and
     other outlaws
   * new/modified attack icons: dark magic, drake, javelins, ...
   * other: new sceptre
 ### multiplayer
   * send a server message each time side controller change (fr #7358)
   * fixed green game bugs
   * fixed new host doesn't get control when original host left (bug #7351)
   * fixed crash when transfering a side (bug #7346, bug #7455)
   * fixed message bell sounding even on ignored messages (bug #7378)
 ### language and i18n
   * new manpages: Dutch
   * updated translations: Esperanto, French, German, Greek, Italian,
     Norwegian, Japanese, Polish, Slovenian, Turkish, Valencian
   * updated MANUAL: Czech, German
   * updated DejaVuSans font to 2.11
 * multiplayer maps:
    * revised multiplayer maps: Hamlets, Meteor Lake, Sulla's Ruins,
      Silverhead Crossing, Isar's Cross, Paths of Daggers
 * engine changes
    * fixed the terrain defense alias to be used properly
    * fixed aggression calculations (bug #7432)
 ### misc bugfixes
   * fixed malfunctioned sighted event during shroud (bug #4398)
   * fixed stats in replay counted not from 0 (bug #7245)
   * fixed replay end showing scenario objectives (bug #6937)

## Version 1.1.11
 ### multiplayer
   * fixed no vacant slots in MP lobby for reloaded game (bug #7286)
   * fixed transfering control (bug #6577, bug #7046)
   * fixed Computer vs. Computer odd side effects (bug #7156)
 ### user interface
   * fixed wrong turn number in multiplayer lobby with re-loaded game (bug #7229)
   * fixed graphical glitches when sending a single '*' message (bug #7197)
   * fixed blue orbs when given control of another side (bug #6352)
   * fixed program freeze when campaign server is unreachable (#bug #6291)
 ### Python AI
   * there is now wesnoth.get_version()
 ### WML engine
   * [event]s can now be written also inside [era]s
     (the [event]s are included in every scenario played using that era)
   * complex missiles frames are now in macros
 ### language and i18n
   * updated translations: Bulgarian, Catalan, Dutch, French, German,
     Italian, Japanese, Russian, Spanish, Swedish
   * updated MANUAL: Japanese, Swedish

## Version 1.1.10
 ### campaigns
   * Heir to the Throne
     * made it a lot harder to kill Li'sar in Ford of Abez
       by giving her reinforcements more often
   * The South Guard
     * Animation for Deoran's Mace attack (from special AMLA)
 * language and i18n:
    * new translations: Valencia
    * updated translations: Czech, English (British), Finnish, German, Italian,
      Japanese, Polish, Portuguese (Brazil), Slovak, Serbian, Swedish
    * new or updated man pages: Czech, English (British), French, German,
      Italian, Japanese, Portuguese (Brazil), Slovak, Swedish
    * removed (outdated) man pages: Hungarian
    * updated MANUAL: Swedish
 ### multiplayer maps
   * revised multiplayer maps: Den of Onis, Hamlets, Meteor Lake, Sablestone
     Delta, Island of the Horatii, Castle Hopping Isle, Siege Castles, King of
     the Hill, Loris River, Forest of Fear, Waterloo Sunset, Merkwuerdigliebe
   * fixed the leader lists in Wesbowl to match current factions
 ### multiplayer
   * scenarios can set faction, recruit, leader, and some other
     initial settings previously ignored in multiplayer
 ### WML engine
   * now "ai turn" events are fired for all AIs (#6738)
   * recruitment_pattern inside [ai] can now change mid-scenario (#6669)
 ### Graphics
   * New dwarven castle
   * New Swamp overlay tiles
 ### Python AI
   * there is now unit.stoned

## Version 1.1.9
 ### campaigns
   * Heir to the Throne
     * fixed gold typo in Dwarven Doors (#6481)
     * Isle of the Damned: removed merman recall on HARD
   * Two Brothers
     * fixed showing the objective "kill the mage to get the key"
       if you already obtained the key (#6704)
 ### multiplayer maps
   * revised multiplayer maps: Sablestone Delta, Den of Onis
 ### language and i18n
   * updated translations: Dutch, Esperanto, Finnish, French, German, Italian,
     Japanese, Norwegian, Polish, Russian, Serbian, Slovak, Slovenian, Spanish
   * new manual: Japanese
   * updated manual: Portuguese (Brazil)
 ### units
   * balancing changes:
     * removed Necrophage from random_leader
   * added profile key to female thief to prevent use of male image
 ### user interface
   * changed the hotkey for 'end turn' from 'alt+space' to 'ctrl/cmd+space'
     because it interferes with several common window managers
   * changed the hotkey for 'repeat recruit' from 'ctrl+shift+r' to
     'ctrl+alt+r' since shift toggles accelerated mode and produced
     an animation glitch with this hotkey; this needs to be considered
     for future hotkey changes/additions (see bug #6312)
 ### misc
   * fixed --with-desktopdir and --with-icondir,
     it still needs --with-kde or --with-gnome (#6499)
   * correct handling of UTF8 filenames under windows
   * multiplayer game management: client now tells the server
     if it ended in victory or defeat

## Version 1.1.8
 * campaigns
   * Eastern Invasion:
     * replaced unit "Lord" by "General" (#6132)
   * The South Guard:
     * fixed a few broken image paths
     * scenario 2:
       * made the enemy leader water-phobic, so he won't charge fishes
       * moved away a footpad so he won't steal the keep at turn #1
     * Jarek no longer acts loyally (#6408)
   * Heir to the Throne
     * fixed fire sword that can't be picked up (#6234)
     * increased income for Snow Plains enemy
     * removed obsolete maps
     * Dwarven Doors balancing tweak: AI gets more income, less gold
     * Home of the North Elves: only need to resist one turn, not all turns
   * Under the Burning Suns:
     * scenario 3: Undead leaders can't move on "very hard" difficulty
   * The Rise of Wesnoth
     * fixed several more portrait issues
     * standardized "speaker" usage in messages
     * removed recall of knights in underground scenarios
     * added hero icon to Lt Aethyr in Clearwater Port
     * fixed missing galleon images
     * fixed missing story image in Rough Landing
     * fixed missing music in the last scenario
 * graphics
   * new attack icons: Dragonstaff, Drake Flaming Claws, Faerie Fire,
     Human Fist, Scimitar, Whip
 * language and i18n:
   * updated translations: British English, Catalan, Czech, Dutch, Esperanto,
     Finnish, French, German, Hungarian, Italian, Japanese, Latin, Norwegian,
     Polish, Portuguese (Brazil), Russian, Slovak, Slovenian, Spanish, Swedish
   * updated man pages: Swedish, German
   * updated manual: Italian
 * multiplayer maps:
   * revised multiplayer maps: Amohsad Caldera, Blitz, Caves of the Basilisk,
     Charge, Den of Onis, Hamlets, Hornshark Island, 4p Moritori,
     Merkwuerdigliebe, Sablestone Delta, Silverhead Crossing
 * music & sound
   * new or improved sounds: elf hit, yeti hit & die, magic missiles, poison,
     stoned, gryphon hit & die & shriek, orc hit & die, small orc hit & die
 * engine changes
   * added support for proper healing animations
   * team colouring now applied correctly everywhere, such as in recruit dialogs
   * new (optional) image path syntax to allow team colouring of images
     anywhere, for example in campaign icons
 * AmigaOS4 support (patch #564)
 * user interface
   * disable inactive sliders instead of hiding them
   * sliders can be adjusted with keyboard left and right
   * improved the parsing speed of the help dialog

## Version 1.1.7
 * campaigns
   * Heir to the Throne
     * added 2 mermen to Bay of Pearls on hard
     * lowered turn limits on Scepter of Fire to something sane
 * language and i18n:
    * updated translations: French, German, Portuguese (Brazil),
      Spanish, Swedish
 * multiplayer maps:
    * revised multiplayer maps: Amohsad Caldera
 * units
    * balancing changes:
      * removed White Mage and Mage of Light from random_leader
      * removed special defense values from the Merman Hoplite
      * decreased the melee attack of the Merman Javelineer from 8-3 to 8-2
      * increased the melee attack of the Merman Entangler from 7-3 to 8-3
      * increased the ranged attack of the Merman Entangler from 7-3 to 8-3
      * increased the melee attack of the Necrophage from 6-3 to 7-3
      * decreased the XP requirement of the Necrophage from 66 to 40
 * music & sound
   * new or improved sounds: lich hit & die, bite, tail
 * graphics
   * new village flag animation
   * some units and animations updated for team color support
 * fixed bug that caused the game to freeze when a leader
   takes part in a fight

## Version 1.1.6
 * campaigns
   * Heir to the Throne
     * added a secret, powerful item to one of the later scenarios
     * ensured minimum gold of 500 to start campaign climax scenarios
       (after Elven Council)
     * expanded list of units that can obtain the Fire Sword
     * added an option dialog to let you choose who gets the Fire Sword
   * The Rise of Wesnoth
     * fixed missing objectives for Cursed Isle
   * The South Guard
     * fixed Naga position in Hard mode for Tidings Good And Ill
 * multiplayer maps:
   * renamed Smallolof to Amohsad Caldera
   * revised multiplayer maps: Caves of the Basilisk, Cynsaun Battlefield,
     Sulla's Ruins, Hornshark Island, 3p Morituri, Blue Water Province,
     Castle Hopping Isle, Forest of Fear, Amohsad Caldera, Hexcake
 * units
   * balancing changes:
     * removed special defense values from the Merman Fighter line
     * decreased impact resistance from 30% to 10% for the
       Heavy Infantryman line
 * graphics
   * updated armor sprite
   * new attack icons: Cleaver, Pitchfork, Quarterstuff, Wooden Sword
   * new tropical forest
 * user interface
   * added several default hotkeys
   * changed the hotkey for 'Update Shroud Now' from 'ctrl+k' to 'S'
   * moved the hotkey definitions into its own file
   * added illuminated cave to help.cfg
   * added illumination description to lava in help.cfg
 * language and i18n:
   * updated translations: Finnish, French, German, Hungarian,
     Latin, Polish, Portuguese (Brazil), Swedish, Turkish
   * updated MANUAL: Swedish
   * updated man pages: German, Swedish

## Version 1.1.5
 * WML engine
   * all [event] tags shall perform complex substitution
   * new key in [advancement] to prevent AMLA if the unit
     can advance to another unit.
 * language and i18n:
   * updated translations: Czech, Finnish, Hungarian, Italian,
     Latin, Norwegian, Portuguese (Brazil), Spanish, Swedish
 * campaigns
   * Tutorial
     * the hero icon is explained
   * Heir to the Throne
     * not being able to recruit gryphons is further explained
       in Northern Winter
     * made Dwarven Doors entrance accessible to mountedfoot
   * The Rise of Wesnoth
     * gave hero icons to unkillable heroes (possibly missing some)
     * fixed lack of death objective for Lord Typhon in A Final Spring (#6108)
     * Added new terrain to Temple of the Deep and The Ka'lian
     * Removed some cave terrain in The Sewer
 * multiplayer maps:
   * config file update: Sulla's Ruins
   * revised multiplayer maps: Charge, Meteor Lake, King of the Hill,
     Loris River, Paths of Daggers, Siege Castles, Waterloo Sunset
 * units
   * fixed zoc/no zoc ellipses
   * balancing changes:
     * Increased HP of the Footpad from 28 to 30
     * Increased melee and ranged damage of the Footpad from 4-2 to 5-2
     * Increased HP of the Outlaw from 40 to 42
     * Increased melee damage of the Outlaw from 7-2 to 8-2
     * Added marksmanship to the Drake Glider branch
     * Hurrican and Sky Drakes now have same defense and movement costs
       on fungus and in caves like other drakes.
 * music & sound
   * new or improved sounds: holy magic, thorns, pincers
 * miscellaneous
   * added a generic scenario music macro and table of contents to utils.cfg
   * added an advanced preference to disable mouse scrolling
   * --no-delay and --exit-at-end options added for benchmarking
 * graphics
   * new attack icons: Curse, Drake Claws, Human Crossbow, Human Throwing
     Dagger, Ink, Orcish Bow, Orcish Crossbow, Orcish Spear, Slam, Tentacle,
     Thunderstick, Undead Dagger, Waterspray, Web

## Version 1.1.4
 * language and i18n:
   * updated translations: Czech, German, Slovak
 * help menu
   * It is now possible to specify generated topic sorting by title
     separate from sorting all of the topics. This allows a sorted list
     of generated topics to be placed after a fixed list of topics.
   * The Abilities and Traits sections were changed so that the overviews
     come first
   * Fixed a bug where the wrong ability description could be extracted
     from a unit type with multiple abilities. This was causing an
     incorrect description for Illuminate in Help.
   * Units are now sorted first by race and then by name.
 * WML engine
   * modify ranged animation timing, all animations (attack, defend,
     missile, block have the same clock)
   * leading_image is deprecated, please use the new [leading_anim] animation
 * multiplayer maps:
   * revised multiplayer maps: Charge
 * units
   * fixed a problem with unit type description being overwritten
     by the unit description (name) when saving and reloading.
   * tweaked the movement type of Nagas and Ghosts

## Version 1.1.3
 * campaigns
   * Two Brothers
     * made the 2nd scenario easier on easy: more starting gold,
       4 extra villages, +10 income, less villages for the ai
     * added a hero icon for the dark adept
     * improved dialogs in the 2nd scenario
     * added a move_unit_fake to have Brenda not appear out of nowhere
       (easy only)
     * improved dialogs in the 3rd scenario
   * The South Guard
     * improved/additional dialog
     * new portraits
   * Heir to the Throne
     * Blackwater Port: secret bonus for beating the leader on hard
     * Isle of the Damned: adjusted temple contents to sync up with difficulty level
     * Scepter of Fire: major rebalance to relax the pace of the level
     * Home of the North Elves: army camps are invincible, elves defend you better
     * Marked heroes with {IS_HERO} macro
     * Increased minimum gold for most of campaign Act 2
   * The Rise of Wesnoth
     * Fixed portrait paths
     * Fixed map image paths
     * Switched to use of profile key in single unit definitions
     * Minor text cleanups
   * Under the Burning Suns
     * New unit artwork for Kaleh and Nym
     * Shrunk portraits to fit the Wesnoth standard of other campaigns
     * Removed Skirmish Ability from Desert Rider and Desert Outrider
     * Added Skirmish Ability to Desert Horseman
     * Increased melee damage of Desert Horseman from 4-3 to 5-3
     * Increased HP of the Desert Horseman from 45 to 50
     * Minor text cleanups
 * graphics
   * impassable mountain terrain
   * portraits: troll, assassin, dwarf-guard from old UtBS
   * death animations: vampire bat, goblin impaler, sea serpent, skeleton
   * new attack animations: goblin impaler, orcish slurbow & archer,
     elder mage, master-at-arms, white mage
   * timing in most animations changed to make them look smoother
   * minor changes or filler frames added to several animations
   * new attack icons: bow, club, dark missile, druid staff, elven sword,
     entangle, flaming sword, frenzy, lance, human sabre, human throwing
     dagger, mace, morning star, mud glob, orcish dagger, orcish greatsword,
     orcish sword, pike, pincers, plaguestaff, sling, thorns, torch, touch,
     wail, zombie touch
   * hitpoint distribution graphs under Damage Calculations
   * improvements to loadscreen progressbar
   * new selection style for menu listboxes
   * many units and animations updated for team color support:
   * units can now have standing animations that are used when standing
 * multiplayer maps:
   * added multiplayer maps: 8p Morituri, Waterloo Sunset, Caves of the Basilisk,
     Isar's Cross, Merkwuerdigliebe
   * removed multiplayer maps: Divide and Conquer
   * revised multiplayer maps: Blitz, Charge, Cynsaun Battlefield, Den of Onis,
     Hamlets, Hornshark Island, Silverhead Crossing, Sullas Ruins, Island of the
     Horatii, 3p Morituri, Clash, Lagoon, The Wilderlands, Crusaders' Field,
     Sablestone Delta, King of the Hill, Loris River, Paths of Daggers,
     Siege Castles, Waterloo Sunset
 * music & sound
   * new or improved sounds: thunderstick and dragonstick, fist, club, hatchet,
     throwing knives, zombie hit, bow, orcish bow + fire bow, wolf bite, claws,
     wose attack & hit & die, dwarf hit & die, sling, big sling, crossbow +
     fire crossbow, sword, drake hit, naga hit & die, axe, torch, big dark magic,
     gryphon hit & die, human hit & die & female versions & old male versions,
     goblin hit & die, horse hit & die
   * minor edits to several sounds
   * support for randomly chosen variations of sounds
 * language and i18n:
   * new translations: Filipino, Galego
   * updated translations: Czech, Dutch, Finnish, French, German, Italian,
     Korean, Norwegian, Portuguese (Brazil), Romanian, Spanish, Swedish, Turkish
   * updated man pages: Swedish
   * updated manual: Swedish
 * units
   * fixed the resistances of the Sky Drake line
   * increased the holy resistance for units with holy attacks
     to 40% for level 2 and 60% for level 3
   * balancing changes:
     * decreased HP of Drake Flare from 62 to 54
     * decreased HP of Drake Flameheart from 82 to 72
     * increased HP of Fire Drake from 54 to 63
     * increased HP of Drake Inferno from 68 to 82
     * decreased HP of Elvish Fighter from 34 to 33
 * Wesnoth engine
   * new unit display graphic engine...
   * added unit cost to help
 * Wesnoth server
   * disallowed names, accepted versions, redirected versions, proxy versions,
     and ip bans can now use '*' and '?' for wildcard matching
   * host changes automatically if game creator leaves => no disconnections
 * WML improvements
   * interpret the variables/arrays in a few more tags
   * floating damage/heal or status can be printed over a modified unit using
     the 'text', 'red', 'blue', and 'green' keys in [unstore_unit]
   * store_locations now stores terrain as well as x,y
   * [effect] can now apply_to movement_costs, defense and resistance
   * when no missile frame is specified, we don't use a default arrow anymore
 * multiplayer settings:
   * two additional MP timer settings, time reservoir limit and action bonus:
     * Reservoir prevents the turn timer from exceeding an upper limit.
     * Action bonus is added after turn for each attack, capture, and recruit.
   * share view/map option removed from multiplayer create screen
   * slider for each player to adjust their base income
 * misc
   * fix alt key under Mac OS X
   * help contents available from title screen
   * preferences available from multiplayer lobby
   * new /help command in mp lobby to get info on available commands
   * :mute command to silence observers
   * new --turns= commandline parameter
   * conditional statements in animation syntax
   * intelligent autosaves and menu integration

## Version 1.1.2
 * campaigns
   * campaigns removed from mainline: Son of the Black Eye, The Dark Hordes
   * campaigns added to mainline:
     * The South Guard
     * Under the Burning Suns
   * new tutorial replacing the old one
   * Eastern Invasion
     * rewrote dialogue in Lake Vrug, Evacuation, and The Drowned Plains
     * redid cave maps to take advantage of new terrains
     * redid some outdoor maps to be more interesting
     * gave an early finish bonus in scenario Captured
   * Heir to the Throne
     * switched over to [music] tag and removed outdated music choices
     * new story art
     * Isle of Anduin - added destruction to the map
     * Northern Winter - added turns for balancing
     * Scepter of Fire - enemies have less gold for balancing
     * Swamp of Dread - smaller map for faster but more intense gameplay
     * minor gameplay improvement for Swamp of Dread
     * removed Mountain Pass and Valley of Statues from the campaign
     * started to update dots on the big map
   * The Rise of Wesnoth
     * Replaced all occurences of the Demilich with the normal Lich
     * Updated path information
     * updated music tags to new format and removed outdated music choices
   * The South Guard
     * bugfixes, and clarification of objectives
     * switched to using the {IS_HERO} macro
     * all the unit names are now translatable
     * made translatable some forgotten strings
     * switched over to [music] from music=
       got rid of deprecated music and wired in new music.
     * Proven By The Sword: fixed recall of Javelineers
     * removed the unneeded Dread Lich
     * added custom SG flag
     * altered the first scenario so mermen are more useful/interesting.
   * Two Brothers
     * switched to using the {IS_HERO} macro
     * changed music in all scenarios
     * allowing to recruit Footpads to be better able to fight skeletons
     * 2nd mission: added new objective after seeing the Dark Adept
     * 3rd mission: changed guards to be outlaw units
     * 3rd mission: 6 extra turns after killing the leader, this should allow
       to free Bjarn even if Arne was not involved in the main battle
   * Under the Burning Suns
     * new unit images: Crawling Horror
     * copying other Desert Elves, increased melee damage and
       decreased ranged damage for Desert Shaman/Druid/Shyde/Star line
     * increased cost of Desert Fighters from 14 to 15
     * increased melee damage of the Desert Horseman from 6-5 to 7-5
     * The below changes echo mainline elf edits listed in units section:
       * decreased price of the Desert Archer from 18 to 17 gold
       * decreased HP of the Desert Avenger from 60 to 55
       * increase Desert Avenger movement and/or defense on shallow water,
         swamp and mountains
       * increased melee damage of Desert Avenger from 10-3 to 11-3
       * decreased ranged damage of the Desert Avenger from 10-4 to 9-4
       * increased HP of the Desert Captain from 44 to 47
       * decreased ranged damage of the Desert Captain from 5-3 to 4-3
       * decreased melee damage of the Desert Champion from 13-4 to 10-5
       * decreased ranged damage of the Desert Champion from 8-4 to 8-3
       * increased HP of the Desert Fighter from 32 to 34
       * increased XP requirement of the Desert Fighter from 38 to 40
       * increased HP of the Desert Sharpshooter from 45 to 47
 * graphics
   * added 'ellipse=blah' tag to units which will select the four images
     from the misc directory for use as an ellipse: misc/blah-bottom.png,
     misc/blah-top.png, misc/selected-blah-bottom.png,
     misc/selected-blah-top.png
   * new encampment graphics
   * moved hero icon from the South Guard into main directories
   * new attack icons: animal fangs, axe, baneblade, battle axe,
     beak, claws-animal, claw-undead, entangle, faery touch, fireball,
     fist-troll, fist-skeletal, hammer, hatchet, human-greatsword,
     iceball, lightbeam, lightning, magic missile, necromatic staff,
     magic staff, shield, spear, trident
   * new portrait for Drake Clasher, Giant Scorpion
   * improved runemaster graphics from Sceptre of Fire
   * new modified Wesnoth map for HttT (larger, more legible)
   * new "units" icon for the menu
   * new unit images: Lord
   * Death animations: dark adept, yeti, skeleton archer death
   * Several updated/new terrain transitions
 * language and i18n:
   * new translations: Korean, Vietnamese
   * updated translations: British English, Catalan, Czech, Dutch,
     German, Finnish, French, Hebrew, Italian, Norwegian, Polish,
     Portuguese (Brazil), Spanish, Swedish, Turkish
   * updated man pages: Swedish
 * multiplayer maps:
   * all multiplayer maps now use a playlist to specify their music
   * revised multiplayer maps: Blitz, Charge, Cynsaun Battlefield,
     Den of Onis, Divide and Conquer, Hamlets, Hornshark Island,
     Meteor Lake, Silverhead Crossing, Triple Blitz, Bluewater Province,
     Clash, Siege Castles, Hexcake, The Wilderlands
 * music & sound
   * multiplayer no longer uses faction specific music, music is now
     specified per scenario like it is done in the singleplayer mode
   * new music: gameplay01, gameplay02
   * updated music: gameplay03
   * removed music: wesnoth-3, wesnoth-4, wesnoth-6, wesnoth-7, wesnoth-8
   * new sounds: turn end bell, drake hit&die, dwarf hit&die, short fanfare,
     additional female-hit, hiss-die, hiss-hit, holy attack, 3 ghostly wails,
     3 gryphon, miss, 2 orc-die, 3 skeleton, 2 troll hit&die, water blast,
     wolf hit&die, 4 dark magic, 2 lightning, 12 human hit&die, flame hit&miss
     3 new merfolk hit&die
   * sound added to units:
     * death sounds: all drakes, dwarves, orcs & goblins, all humans,
       monsters, merfolk, nagas, saurians, trolls, all skeletons, woses
     * lightbeam attacks, elves, gryphons, some ghost attacks
 * units
   * back in mainline: Orcish Leader, Orcish Ruler, Orcish Sovereign
   * Swapped names for necromancer (now a L3) and dark sorcerer (now a L2)
     (no gameplay change)
   * renamed Saurian Tribalist/Icecaster to Augur/Oracle
   * gryphon rider and master race change to dwarf
   * balancing changes:
     * decreased HP of the Inferno Drake from 71 to 68
     * increased melee damage of the Inferno Drake from 11-2 to 12-2
     * decreased HP of the Drake Flameheart from 84 to 82
     * decreased melee damage of the Drake Flameheart from 12-3 to 11-3
     * increased ranged damage of the Drake Flameheart from 8-4 to 9-4
     * increased XP requirement of the Drake Fighter from 38 to 42
     * increased HP of the Drake Warrior from 55 to 60
     * increased melee damage of the Drake Warrior from 10-3 to 11-3
     * increased HP of the Drake Blademaster from 70 to 80
     * increased melee damage of the Drake Blademaster from 14-3 to 16-3
     * increased XP requirement of the Drake Clasher from 40 to 45
     * increased HP of the Drake Gladiator from 62 to 66
     * increased melee impact damage of the Drake Gladiator from 9-3 to 10-3
     * decreased HP of the Drake Enforcer from 87 to 85
     * increased melee blade damage of the Drake Enforcer from 10-4 to 11-4
     * increased melee impact damage of the Drake Enforcer from 12-3 to 14-3
     * increased HP of the Drake Slasher from 57 to 62
     * increased melee pierce damage of the Drake Slasher from 16-2 to 17-2
     * increased HP of the Drake Warden from 79 to 82
     * increased melee blade damage of the Drake Warden from 15-3 to 16-3
     * increased melee pierce damage of the Drake Warden from 22-2 to 23-2
     * increased HP of the Sky Drake from 40 to 45
     * increased HP of the Hurricane Drake from 50 to 58
     * increased resistances of the Sky Drake line to those of the Drake Glider
     * decreased XP requirement of the Saurian Ambusher from 70 to 55
     * decreased cost of the Bandit from 27 to 23
     * increased HP the Rogue from 36 to 40
     * increased HP the Assassin from 42 to 51
     * decreased movement cost for swamp for the Poacher line from 3 to 2
     * increased the XP requirement of the Dwarvish Fighter from 38 to 41
     * increased HP of the Dwarvish Steelclad from 55 to 59
     * increased melee blade damage of the Dwarvish Steelclad from 10-3 to 11-3
     * increased melee impact damage of the Dwarvish Steelclad from 13-2 to 14-2
     * increased HP of the Dwarvish Lord from 75 to 79
     * increased melee blade damage of the Dwarvish Lord from 14-3 to 15-3
     * increased melee impact damage of the Dwarvish Lord from 17-2 to 19-2
     * increased HP of the Dwarvish Thunderguard from 42 to 44
     * increased HP of the Dwarvish Dragonguard from 52 to 59
     * changed the damage type of the Dwarvish Dragonguard from impact to blade
     * decreased HP of the Elvish Avenger from 60 to 55
     * decreased ranged damage of the Elvish Avenger from 11-4 to 10-4
     * increased HP of the Elvish Sharpshooter from 45 to 47
     * increased XP requirement of the Elvish Fighter from 38 to 40
     * increased HP of the Elvish Captain from 44 to 47
     * decreased ranged damage of the Elvish Captain from 6-3 to 5-3
     * decreased melee damage of the Elvish Champion from 12-4 to 9-5
     * increased experience requirement of the Cavalryman from 34 to 40
     * decreased melee damage of the Dragoon from 7-4 to 6-4
     * decreased ranged damage of the Dragoon from 15-1 to 12-1
     * decreased XP requirement of the Dragoon from 110 to 95
     * increased experience requirement of the Fencer from 37 to 42
     * decreased ranged damage of the Duelist from 15-1 to 12-1
     * increased HP of the Lancer from 48 to 50
     * increased HP of the Javalineer from 45 to 48
     * increased resistances of the Javalineer to the one of the Pikeman
     * increased melee damage of the Halberdier from 14-3 to 15-3
     * decreased HP of the Royal Guard from 75 to 74
     * increased HP of the Master Bowman from 50 to 58
     * increased ranged fire damage of the Orcish Crossbow from 9-2 to 10-2
     * increased HP of the Orcish Slurbow from 50 to 56
     * increased ranged fire damage of the Orcish Slurbow from 14-2 to 15-2
     * increased HP of the Orcish Warrior from 52 to 58
     * increased HP of the Orcish Warlord from 80 to 78
     * decreased melee damage of the Orcish Warlord from 13-4 to 15-3
     * increased ranged damage of the Orcish Warlord from 5-3 to 8-2
     * decreased cost of the Orcish Warlord from 56 to 48
     * increased experience requirement of the Troll from 60 to 66
     * increased experience requirement of the Troll Whelp from 33 to 36
     * decreased ranged damage of the Troll Rocklobber from 20-1 to 17-1
     * increased HP of the Elvish Fighter from 32 to 34
     * decreased ranged damage of the Elvish Champion from 9-4 to 9-3
     * decreased price of the Elvish Archer from 18 to 17 gold
     * increased movement cost/defense on various terrains for the Ranger line
     * increased movement of the Wose line from 3 to 4
     * increased HP of the Elder Wose from 60 to 64
     * increased melee damage ot the Elder Wose from 18-2 to 19-2
     * decreased experience requirement of the Elder Wose from 160 to 100
     * decreased experience requirement of the Bone Shooter from 90 to 80
     * increased ranged damage of the Bone Shooter from 9-3 to 10-3
     * increased ranged damage of the Soul Shooter from 12-3 to 14-3
     * decreased HP of the Dark Sorcerer from 58 to 48
     * increased ranged damage of the Dark Sorcerer from 12-2 to 13-2
     * increased experience requirement of the Dark Sorcerer from 80 to 90
     * decreased HP of the Lich from 66 to 60
     * decreased HP of the Necromancer from 75 to 70
     * increased ranged damage of the Necromancer from 15-2 to 17-2
     * decreased melee damage of the Death Knight from 12-4 to 11-4
     * increased HP of the Draug from 54 to 68
     * increased melee damage of the Draug from 11-4 to 12-4
     * increased cost of the Draug from 40 to 50
     * decreased melee damage of the Blood Bat from 6-3 to 5-3
 * WML improvements
   * [if] now supports [and] and [not] tags
   * new {IS_HERO} macro to display a hero icon for new units
   * new {DEFAULT_MUSIC_PLAYLIST} macro to specify gamemusic 01 to 03
     and main_menu.ogg as playlist
   * moved much of about into WML as well as enabled campaign [about]
   * added an #undef preprocessor directive
   * added 'profile' key to [unit] variables
   * added 'image' key to [unit] variables
   * added [modify_turns] to [event]. Takes 'value' and 'add' as keys.
   * added new key to [side]: allow_player; if set to 'no' then this side
     will not appear in the creation screen. Some maps with more sides
     than starting positions may be broken.
   * added [store_side] tag, which takes 'variable' and 'side' as keys,
     and stores data to variable.gold, variable.income, variable.name,
     and variable.team_name.
   * added [label] inside [event]
   * added 'zoc' key to unit types
   * added 'set_hp' and 'set_total' to unit modifications
   * allowed increase on hp to use %.
   * attack animations can now be filtered on hit and miss
   * [effect] can modify attack/defense weight
   * change in the [sound] tag. sounds must now be part of animation
 * miscellaneous changes
   * added a very simple progress bar while loading
   * added a --load FILE command-line option, where FILE is a saved game
     in the standard save game directory.
   * reordered macro calls in game.cfg to fix no-AMLA bug for campaigns
   * {abilities.cfg} call moved from utils.cfg to game.cfg for consistency
   * Send message on player ban or kick
   * Show a message when a player leaves a game.
   * WML-generated units with "random_traits" now show the traits description
   * flags on village don't "flicker" at the end of their animations
   * really fix bug 3388, now really centering on image when clicking it
   * tab completion now completes to smallest substring,
     and shows possible completions
   * added whisper (/msg) support for lobby and observers in game
   * Added ignore list on MP
   * MP server only accepts nicks made from alphanumeric and
     underscore characters
   * force a refresh after calls to ONSCREEN scrolls,
     to avoid refresh problems.
   * add sort_topics command to the WML of help
   * check in ai code that ai doesn't use the same unit to attack twice

## Version 1.1.1
 * campaigns
   * reordered ranks to reflect relevance to game world
   * new (beginner) campaign: Two Brothers
   * Heir to the Throne
     * Gryphons are not available until you find dwarves to ride them
     * Fixed behavior of sleeping gryphons in Gryphon Mountain
     * Changed confusing dialog on Home of the North Elves (#5006)
     * Standardized wording for bonus victory objectives
     * Lost General map now slightly more friendly to smallfoot units
     * Sceptre of Fire - more villages and dirt road near starting castle
     * Underground scenarios generally more interesting/enhanced story
     * Text cleanups
   * The Rise of Wesnoth
     * replaced Sea Orc with Naga Fighter in Final Spring, Clearwater Port,
       and Rise of Wesnoth
     * Fire dragon is a level 5 unit
     * Text cleanups
   * migrated unique units, scenarios, maps, and images to data/campaigns
 * campaign server
   * changed default port for campaign server to 15003 to separate 1.1 and 1.0
 * gameplay changes:
   * improved default weapon selection on player attack (patch #500)
   * attack_weight=0 disables the attack when attacking
   * defense_weight=0 disables the attack when defending
   * attack_weight is now functional (#5014)
   * added a new terrain "Mushroom Grove" (id=fungus)
   * changed several terrains to be "mixed", having worst movement and
     best defense: Snow Forest, Snow Hill, Dunes (renamed from Desert Hill)
   * added new schedule: deep underground (-30%, very dark)
 * graphics:
   * New castle tiles
   * New sand tiles
   * New terrain: Rockbound Cave, Mushroom Grove, Illuminated Cave Floor,
     Great Tree.
   * Added Drake, Gladiator, Enforcer, Slasher and Warden death animations.
     All Drake unit baselines fixed
   * Added item bones.png
 * language and i18n:
   * new translations: Indonesian
   * new manual: Indonesian
   * updated translations: British English, Chinese, German, Hebrew,
     Italian, Norwegian, Polish, Portuguese (Brazil), Russian, Slovenian,
     Swedish, Turkish
   * updated man pages: English, German, Swedish
 * multiplayer maps:
   * revised multiplayer maps: Blitz, Charge, Den of Onis, Silverhead Crossing,
     Sulla's Ruins, Cynsaun Battlefield, Morituri, Triple Blitz, Castle Hopping
     Isle, Clash, King of the Hill, Lagoon, Smallolof, Forest of Fear
   * added multiplayer maps: Loris River, Paths of Daggers
   * removed multiplayer maps: Across the River, An Island,
     The Valley of Death, Battle World
   * renamed multiplayer maps: 1v1v1Hex -> Island of the Horatii
 * music:
   * support for more than one music file per scenario with adjustable spacing
   * new music: gameplay03.ogg
 * units
   * gave names to mermaids
   * deleted Wall Guard and Gate (no longer used)
   * updated descriptions: Goblin Rouser
   * removed the classic era
   * units and balancing changes:
     * made all undeadfoot units that can move on deep water have ambush there
     * several changes to the movetype of the Walking Corpse and Soulless
     * Walking Corpse and Soulless variants now also differ in HP
     * decreased the Dark Adept resistance to cold from 20% to 0%
     * disallowed Lich type units to go on deep water
     * decreased the woodlandfloat tundra movement from 2 to 1
     * decreased armoredfoot cold resistance from 20% to 10%
     * increased Pikeman HP from 52 to 55
     * increased Swordsman HP from 52 to 55
     * increased the ranged attack of the Cavalier and Master at Arms
       from 17-1 to 20-1
     * increased Orcish Archer fire attack damage from 6-2 to 7-2
     * decreased the melee attack of the Orcish Slayer from 12-2 to 10-2
     * decreased the XP requirement for the Footpad from 42 to 36
     * increased the melee attack of the Gryphon Master from 14-2 to 15-2
     * increased Dwarvish Steelclad HP from 50 to 55
     * increased Dwarvish Guardsman HP from 35 to 42
     * increased Dwarvish Guardsman ranged attack from 4-1 to 5-1
     * increased Dwarvish Stalwart HP from 39 to 48
     * increased Dwarvish Sentinel HP from 45 to 56
     * increased Dwarvish Sentinel resistances to blade and pierce
       from 20% to 30%
     * decreased the grassland defense from 50% to 40% for the Sentinel
     * decreased the village defense from 60% to 50% for the Stalwart
       and Sentinel
     * increased the forest and sand defense from 30% to 40% for the
       Stalwart and Sentinel
     * added Elder Wose, Saurian Icecaster and Soothsayer as leaders
       in default era but not as random_leader
     * added many level 2 units and the Ancient Wose as leaders in AoH
       but not as random_leader
   * flags in status bar are now properly team colored
 * user interface
   * added new image in advancement tag that is displayed in advancement dialog
     and changed old image to icon in advancement tag.
   * centered title screen logo for 1024x768 (still off-center for 800x600)
   * prevent icons in themes from being scaled up (only scaling down is allowed)
   * fix menu capitalization consistency (#4972)
   * reduce scrolling during unit movement
   * make default zoom work properly
   * unit speed is back to 1.0 speed
   * upgraded font from DejaVuSans 1.12 to 2.2, has Greek support
   * TAB completion in MP lobby
   * Name of the game shown next to nick for unavailable players
   * avoid useless scrolling down in chat window of mp lobby
 * WML improvements
   * attack ranges renamed to "melee" and "ranged", new ranges can be used
     (retaliation will use any attack of the same range)
   * starting_villages macro updated for added village types
   * split factions of multiplayer.cfg into data/factions to ease making
     combined eras without having to maintain mainline factions
   * abilities are now more configurable in a [abilities] tag, old ability=
     tag deprecated (patch #498), cure is available via [heals] ability tag
   * all currently used abilities added as macros in data/abilities.cfg
   * abilities can receive time of day and terrain [filter] (currently only
     [hides], [steadfast], [illuminates])
     - ambush and nightstalk are available via the new [hides] ability tag
   * added a optional random_leader= tag inside a [multiplayer_side] to
     specify a different list of randomly selectable leaders
   * added level as a possible argument for standard unit [filter]
   * terrain can be given a "light" flag to specify local light modification
   * combined terrain can now take "best of" or "worst of" values
     for combining
   * choice policy for combined terrain can be separately specified
     for def and mvt
   * added random_traits key for the create_unit event.
   * [female] and [male] unit tags now inherit from [unit].
   * Added new events 'attack_end', which occurs after a battle is ended,
     'attacker_hits', which occurs when the attacker hits the defender,
     'defender_hits', 'attacker_misses', and 'defender_misses'.
   * Added [special_filter] and [special_filter_second] to events.
     These allow some events to be filtered by weapon used and by terrain.
   * Added 'mode' tag to [store_unit]; allows appending to variable.
 * Miscellaneous
   * added a require_amla element to [advancement] tags.
   * anonymous data logging for single player campaigns, see stats.wesnoth.org
   * zipios++ can now be disabled by passing --without-zipios to configure
   * functions to easily create one frame animations, for deprecated images
   * improved (underwater) merfolk village
   * (internal) make attack animations work like all other animations
   * backport latest SDL_ttf change, fix ascender/descender issue for DejaVu
   * fix crash when using zoom keys in the main lobby
   * new utils: codecomp codeextract codeglyphs codelist, for font analysis
   * login names in MP are now limited to 18 characters
   * added {RANDOM_SIDE} and {DEFAULT_SCHEDULE} macros
   * added a hotkey to clear all labels in MP games
   * fixed the default time limit values, and changed them to 300/150

## Version 1.1
 * campaign server
   * support new "timestamp" attribute
   * save persistent attributes in the campaign data
     * "name" is treated as directory name, so use campaign_name (#4525)
     * SECURITY: ensure file and directory names on upload match campaign name
   * support filtering on campaign_list requests: name, last update time
     (before or after), language of translation
   * campaign_list response now contains the time the request was processed
   * campaign_list response now contains information about translations
   * fixed problem with duplicate translations being saved with each upload
   * added a passphrase change function
   * deleting campaigns, campaign file is now deleted, not reduced to zero size
 * campaigns:
   * Eastern Invasion
     * fixed Undead fighting each other in Weldyn Besieged (#4386)
     * fixed Undead not recruiting after leader change in The Crossing (#4393)
     * The Drowned Plains: abandonded villages no longer require village.png
   * Heir to the Throne
     * fixed Delfador talking to himself in Valley of Statues (#4440)
     * fixed Home of the North Elves inaccurate victory condition (#4644)
     * Isle of Anduin: reduced difficulty, added mage tips, move_unit_fake fix
     * Blackwater Port: added tips about loyal units, move_unit_fake fix
     * Bay of Pearls: traits for mermen, you get a hunter and initiate, added
       ford to help units cross, hunter and initiate available after winning
     * Isle of the Damned: replaced some forest with road, can recall mermen,
       Moremirmu explains if he will join you or not, hard difficulty has fog
     * Muff Malal's Peninsula: less gold/more income for Muff for XP milking
       on easy, blood bats on hard, choice of fight vs run is better explained
     * Crossroads: added story dialog
     * Princess of Wesnoth: clarified victory conditions
     * Plunging into the Darkness: removed unnecessary side 2 (enemy) leader
     * Dwarven Doors: true exit now random, bandit easter egg now reveals exit
     * changed Snow Plains map to use snow terrain
     * Lost General: side 2 ally allows you to recruit guardsmen (for dwarf
       balancing)
     * Choice Must be Made: used swamp village instead of cave villages,
       story, music
     * Snow plains: music key added
     * Swamp of Dread: used swamp village instead of cave village, music
     * Home of the North Elves: rebalanced to make you RUN! rather than fight;
       dialog
     * Elven Council: added dialog
     * Test of the Clans: simplified victory conditions, dialog
     * Epilogue: corrected dialog about Li'sar's father
   * The Rise of Wesnoth
     * Adjusted loss event in Clearwater port to coincide with time over
     * Fixed bugs related to cuttlefish destroying the bridges
   * Son of the Black Eye
     * cleaned up prestart events to stop units hopping around at map setup
     * Desert of Death leader without starting position replaced with no_leader
     * Clash of Armies mermen leader now created in event, not in side setup
     * fix glitches (part of #4385)
     * fix saurian deaths not triggering loss in Saving Inarix (#4803)
     * general tidying of scenario files
     * fix shamans losing experience (#4792)
   * The Dark Hordes
     * Underground Pool no longer has units off map and in walls
 * gameplay changes:
   * backstab now works if opposite unit is non-incapacitated enemy of defender
   * prevented stoned units from being healed
   * slow does not remove an attack anymore
   * slowed units see the damage they deal halved immediately
   * unit modifications now round toward original value instead of truncating
   * changed resilient trait bonus from +7hp to +10%+3hp
   * decreased strong trait hitpoints bonus from 2 to 1
   * added Fischer clock to MP games to limit turn duration
 * graphics:
   * HP and XP bar modifications
     * movement ball separated from bars
     * moved to left so xp bar under the ball
     * xpbar now changes color differently for AMLA then for regular leveling
     * bar colors and text color in report now forced to match.
   * team color:
     * units can now have colors redefined to match team colors:
     * team color can be defined in the side tag with "team_rgb=r,g,b"
     * team color enabled for: Knight, Grand Knight, Lancer
   * new title screen and logo
   * changed the storm trident attack icon from fireball to lightingbolt
   * new attack icon for the fireball
   * portraits:
     * new Li'sar, Drake Burner, Drake Fighter, Drake Glider, Elvish Shaman,
       Ghoul, Mermaid Initiate, Merman Fighter, Merman Hunter, Naga Fighter,
       Orcish Assassin, Saurian Skirmisher
     * all Drake, Merman, Naga and Saurian upgrades now use level one portraits
     * Necrophage and Orcish Slayer upgrades now use level one portraits
     * Updated portraits for Haldric and Aethyr in TRoW
   * new unit images: Konrad level 1, Tentacle of the Deep
   * new death animations: Initiate, Mage, Pikeman, Red Mage, White Mage,
     Tentacle of the Deep, most Drake units
   * new tiles: swamp, cave wall, paved road, variable-width canyon (#8817),
     snow village (human & elven), underground village, mountains, hills,
     snow hills, dwarven village, additional elven and human village variants
   * bilinear interpolation of images (better interpolation at high zoom level)
   * new movement animation: elvish fighter
 * language and i18n:
   * added support for right-to-left languages (patch #470)
   * made the list of languages configurable via the locale WML
   * new manual: Chinese; updated manual: English, German, Swedish
   * updated man pages: Swedish
   * new translations: Hebrew, Esperanto, Portugese
   * updated translations: Afrikaans, British English, Chinese, Czech, Dutch,
     French, German, Hungarian, Italian, Japanese, Polish, Portuguese (Brazil),
     Russian, Slovak, Slovenian, Spanish, Swedish, Turkish
   * fixed Undead used as both trait and race name (#4295)
   * fixed loading non-binary savegame displaying messages in English (#4454)
   * help changes
     * added brief description of Shroud and Fog of War
   * documentation: switch doxygen templates from CVS to SVN, Savannah to Gna!
 * multiplayer server
   * server: add administration functions
   * multiplayer game with "empty" as side no longer causes OOS (#4464)
   * new multiplayer maps: Crusaders Field, Cynsaun Battlefield, 4p Morituri
   * revised multiplayer maps: Divide and Conquer, Sablestone Delta, 1v1v1Hex,
     Silverhead Crossing, Sullas Ruins, Charge, Blitz, Clash, Hamlets,
     Meteor Lake, Den of Onis, Smallolof, Castle Hopping Isle, Hornshark Island
   * updated multiplayer map cfg files: Clash, Hexcake, Smallolof
   * fixed gamelist diff bug
   * made "Load Game ..." first map option (was last), for better visibility
   * added map size to MP lobby (patch #484)
   * added scenario and era info to MP lobby
   * let observers be visible in slot combos
   * new multiplayer preference tab in the preference window (patch #444)
     * add an option to timestamp chat in MP games (patch #444)
     * add an option to choose the number of chat lines displayed (patch #444)
 * music and sound:
   * new main menu music
   * support for victory and defeat music incl. music files
 * units
   * AMLA for most units now adds 25% to required XP for next AMLA
   * new unit: added the Goblin Rouser
   * removed obsolete units: Merman, Naga, Saurian, Saurian Warrior,
     Sea Hag, Triton
   * units and balancing changes:
     * changed Cuttlefish tentacle attack to swarm.  changed HP from 52 to 67
     * changed Halbardier pierce attack from 9-4 to 14-3
     * changed Halbardier blade attack from 11-4 to 19-2
     * blade attack of Halbardier does not longer have firststrike
     * increased Master Bowman melee damage from 7-3 to 8-3
     * decreased the HP of the Dragoon from 58 to 53
     * changed the movetype of the General and Grand Marshal back to smallfoot
     * changed the resistances to physical damage for the General to 10% and
       for the Grand Marshal to 20%
     * decreased Merman line resistance to cold from 60% to 20%
     * decreased Merman Hunter experience needed to level from 38 to 35
     * decreased Elvish Shaman cost from 16 to 15 gold
     * increased the HP of the Elvish Hero from 48 to 51
     * decreased Elvish Scout experience needed to level from 37 to 32
     * decreased Elvish Rider melee attack from 6-4 to 7-3
     * decreased Elvish Rider ranged attack from 7-3 to 9-2
     * increased Elvish Rider HP from 42 to 46
     * decreased Elvish Outrider melee attack from 8-4 to 7-4
     * decreased Elvish Outrider melee attack from 7-4 to 8-3
     * increased Elvish Outrider HP from 50 to 57
     * decreased the cold resistances of Woses from 20% to 10%
     * decreased Drakefly snow defense from 30% to 20%
     * decreased Drake Slasher HP from 65 to 57
     * decreased Drake Warden HP from 88 to 79
     * increased Drake Gladiator HP from 59 to 62
     * increased Drake Enfoncer HP from 79 to 87
     * decreased the cold resistances of dwarves from 20% to 10%
     * decreased the blade resistances of dwarves from 30% to 20%
     * decreased Dwarvish Steelclad blade resistance from 40% to 30%
     * decreased Dwarvish Fighter hammer attack damage from 9 to 8
     * made the Dwarvish Guardsman line have 2 traits again
     * decreased the HP of the Thunderer from 37 to 34
     * decreased the Gryphon Riders movement from 9 to 8
     * removed Thug from Knalgan faction
     * added the Dwarvish Stalwart and Necrophage as a leader in default era
     * added different movetypes to variations of WC/Soulless (corpses gain
       physical resistances of killed unit)
     * added drain on the melee attack of the Ghost
     * increased Ghost's HP from 17 to 18
     * synchronized Ghost line's physical resistances to 50%
     * decreased Ghost line shallow water movement cost from 3 to 2
     * decreased Ghost line deep water movement cost from 4 to 2
     * increased Shadow HP from 24 to 26
     * increased Nightgaunt HP from 32 to 35
     * increased the HP of the Skeleton Fighter tree by 4
     * changed the Deathblades resistances to those of the other Skeletons
     * decreased Skeleton line shallow water movement cost from 3 to 2
     * decreased Skeleton line deep water movement cost from 4 to 3
     * decreased the tundra defense of Undeads (undeadfoot) from 40% to 30%
     * increased the HP of the Ghoul line by 5
     * decreased the attack damage of the Ghoul line by 1
     * increased the fire resistance of the Ghoul line from 0% to 10%
     * Ghoul line can no longer move into deep water
     * increased the HP of the Vampire Bat from 14 to 17
     * decreased the cost of the Blood Bat from 24 to 21
     * increased the HP of the Blood Bat from 24 to 27
     * decreased village defense of Wolf Rider, Goblin Knight/Pillager to 50%
     * increased Goblin Pillager HP from 40 to 44
     * increased Goblin Knight HP from 45 to 49
     * increased Direwolf Rider HP from 55 to 61
     * increased Orcish Archer fire damage from 5-2 to 6-2
     * increased Orcish Crossbowman fire damage from 8-2 to 9-2
     * increased Orcish Slurbow fire damage from 12-2 to 14-2
     * decreased the tundra defense of Nagas from 30% to 20%
     * decreased the XP requirement for the Goblin Spearman from 22 to 18
     * added the Goblin Rouser as an advancement to the Goblin Spearman
     * Northerners can now recruit Rocklobber in Age of Heroes era
     * removed Saurians as leaders in default and Great War era
     * removed level 2 leaders from AoH era
     * added the Dwarvish Sentinel as a leader in AoH era
     * increased the holy resistance of the scuttlefoot movetype from 0% to 20%
     * made Fireball, Gate, Galeons & Watch Tower unpoisonable and unplaguable
     * recosting and minor stats changes for many level 2+ units:
       https://www.wesnoth.org/forum/viewtopic.php?p=121154#121154
 * user interface:
   * added font colors to theme
   * reorganized side-bar information
   * fix untranslated unit create dialog (#4424)
   * changed recall to show recall list even when gold < 20
   * added advancement and AMLA indicator icons and tooltips to dfool theme
   * allow pausing during replay
   * SECURITY: check safety of campaign download names: if any unsafe
     names are found, install is aborted before old version is removed
   * hitpoint update delays are now capped at 1 second (#4070)
   * reduce unnecessary scrolling in replays (#3152)
   * stop teleport glitches due to insufficient screen updates (#4234)
   * theme chooser window added to preference graphics dialog
   * fix broken colour wait cursor on Mac OS X (#4729)
   * clicking on unit portrait in status pane now centers on unit (#3388)
   * show numbers of each enemy which can reach tiles in "Show Enemy Moves"
   * don't show "The End" after viewing a won multiplayer replay (#4166)
   * new option to toggle server messages when observing games
   * semi transparent HP/XP bar when unit is not selected (patch #485)
   * play an "anonymous" music during enemy turns to avoid the music
     giving hints
   * added the possibility to play "victory" and "defeat" music
   * enable down arrow on scrollbar when resizing (#4809, patch #487)
   * better movement highlighting (patch #488)
   * stop save dialog complaining about bad name when pressing cancel (#4835)
   * pressing 'n' wont take you out of path selection mode (#4833, patch #490)
   * fix broken tooltip in the status bar for plague(argument)
   * added the "/me" command to MP lobby
   * don't show illuminated time of day on fogged/shrouded tiles
 * utils
   * added weblist.pl and webtgz.pl web interface programs
   * wml.pm uses substr instead of split: reduces memory usage for large strings
   * wml_net.pm inserts instead of appending: should be more efficient
   * added prkill script, to calculate probabilities to kill in a skirmish
   * added campaign_passphrase.pl for changing passphrases
   * added campaign_delete.pl for deleting campaigns
 * WML improvements:
   * random map generator now uses island_size (#4458)
   * fixed incorrect savefile name after continue_no_save (#4439)
   * fix bad leader starting position causing crash
   * tighten up checks on leaders and units placed on map at start of scenario
   * add a backup save_id to use if both save_id and description are empty
   * fix duplicate key values being concatenated: just replace with last value
   * units can now have [extra_anim] tag, and [animate_unit] event to trigger it
   * added swarm attack special
   * new advance and post_advance events, trigger before/after unit advancement
   * get_random avoids nested [random] tags: keep to limit in compressed saves
   * experience_modifier: only last value kept; to max experience now rounded
     with lower limit of 1
   * defensive_image* deprecated, use defensive animations instead
     (still work, but issue warnings, new macros are provided)
   * using [frames] directly in an [attack] tag is deprecated.
     please enclose attack animations in [animation] tags
   * [store_unit] now leaves variable untouched if filter does not match
   * units can now have [movement_animation] tags, these are filtered
     on both terrain and direction
 * various other changes:
   * --with-preferences-dir configure option for coexistence of multiple versions
   * experimental Python API for AI.
     See https://www.wesnoth.org/wiki/ReferencePythonAPI for more information.
   * various bug fixes and code cleanups
   * better support for MacOSX filesystems

## Version 1.0rc1
 * language and i18n:
   * updated translations: Afrikaans, Bulgarian, Catalan, Finnish,
     German, Greek, Hungarian, Japanese, Portuguese (Brazil), Russian,
     Slovenian, Spanish, Turkish
   * new manual: Hungarian, updated manual: English, Swedish
   * updated man pages: English, German, Swedish
 * campaigns:
   * The Dark Hordes
     * Crelanu's Book no longer ends if enemy leaders are killed
     * Underground Pool no longer ends if enemy leaders are killed
     * Underground Pool can now only be won with Gwiti or Tanar (#14495)
   * Son of the Black Eye
     * Watch Towers can no longer be recalled in Clash of Armies
     * Saving Inarix no longer ends if enemy leaders are killed
     * tweaked Black Flag and Desert of Death maps
     * made End of Peace, Toward Mountains of Haag, and Black Flag easier
   * Eastern Invasion
     * fix Konrad II's lines not being spoken in The Council (#14500)
     * change the holy waters to "holy amulets" (also #14539, #14559)
     * Drowned Plains now has fog (#14499)
   * Heir to the Throne
     * fix Delfador losing experience at Siege of Elensefar (#14508)
 * units:
   * changed Orcish Shamans from 20 to 23 HP (Novice 24, Old 22)
   * changed all Orcish Shamans from 60 to 70% defense on villages
   * fixed incorrect reference to Demilich image (#14542)
 * fixed long filenames overflowing editor file chooser dialog (#13855)
 * fixed filenames with spaces causing garbled errors (#14494)
 * multiplayer maps now make better use of team names from settings
 * fixed possible memory corruption during "sighted" WML events (#14426)
 * fixed the Windows shortcuts being used in textboxes under Mac OS X (#14377)
 * more AMLA fixes, and it now increases current hitpoints by 3 (#14530)
 * BeOS networking fixes
 * multiplayer lobby changes (including #8179)
 * fixed images in downloaded campaigns not being shown until restart (#11797)
 * editor now supports changing language, with Alt-l (#11966)
 * savegames now store ai_algorithm (#14558)
 * server now supports --threads option to specify number of threads
 * fixed spurious "timeout" when uploading campaigns
 * capture events that trigger further events can no longer be undone (#13895)
 * editor now supports --bpp to specify bit depth, default now 24 (#14551)
 * broken scenarios can no longer make multiplayer unplayable (#14409)
 * various bug fixes and code cleanups

## Version 0.9.7
 * language and i18n:
   * updated manual: French, Italian
   * updated translations: Afrikaans, Basque, British English, Catalan, Czech,
     Dutch, French, Finnish, German, Greek, Hungarian, Italian, Latin, Polish,
     Russian, Slovak, Slovenian, Spanish, Swedish, Turkish
   * fixed Ruby of Fire object description (#14311)
   * fixed untranslatable Rise of Wesnoth scenario name
   * new man pages: Hungarian
   * changing languages now updates Tome of Wesnoth on Linux into new language
 * campaigns:
   * The Dark Hordes: there are now more turns available in the last scenario
 * units:
   * increased the damage done by the Dread Lich from 6-4 to 7-4
 * user interface
   * text input box position now changes if resolution is changed (#12466)
   * hourglass cursor now works on Mac OS X (#14298)
   * fixed scrollbar problems in multiplayer lobby (#13089)
   * better checking if recruiting is possible (#13638, patch #4267)
   * some fixes for Experimental and Default themes
   * fixed observer icon corruption (#14320)
   * user campaign translations are now available without a restart (#14388)
   * adjusted multiplayer lobby titles
   * don't show images larger than 80x80 in campaign server list (#12007)
   * fixed progress bar overflow (#14403)
   * game names can no longer contain formatting characters (#14246)
   * fix campaign story text overflowing due to narrow width (rest of #13961)
 * graphics
   * fixed Flaming Sword missing attack image (#14346)
   * updated Goblin Impaler, Gryphon, Gryphon Master, Gryphon Rider
 * ambush and nightstalk now works on turn 1 for leaders (#13671)
 * AI now understands that AMLA units don't heal on advancing
 * AI in multiplayer can now recruit more widely in all eras (rest of #14096)
 * fixed a network related freeze (#14359) and other networking changes
 * fixed connect to server hanging on Mac OS X
 * fixed memory leaks in multiplayer lobby (part of #13981)
 * various bug fixes and code cleanups

## Version 0.9.6
 * language and i18n:
   * new manual: Turkish
   * updated manual: German, Russian, Swedish
   * updated translations: Afrikaans, Basque, British English, Catalan, Czech,
     Danish, Dutch, Finnish, French, German, Hungarian, Italian, Latin, Polish,
     Portuguese (Brazil), Russian, Slovak, Spanish, Swedish, Turkish
   * updated man pages: English, German, Swedish
   * fixed some translated strings not being used (#14094)
   * improved layout of some parts of help (#14075)
   * fixed several typos and errors (including #14228,#14207,#14076,#14077)
 * campaigns
   * Eastern Invasion: revisions to Weldyn Under Attack and Evacuation
   * The Rise of Wesnoth: fixed glitch in The River Road (forum thread 7148)
 * graphics and sound
   * new unit portraits for Elvish Shaman, Ghoul, Grand Knight, Halberdier,
       Li'sar, Mermaid Initiate, Merman Fighter, Merman Hunter,Naga Fighter,
       Nagini Fighter, Orcish Assassin, Royal Guard, Saurian Skirmisher
   * revised unit images and/or animations for Direwolf Rider, Duelist,
       Elvish Shyde, Fencer, Grand Knight, Heavy Infantryman, Horseman,
       Knight, Lancer, Master at Arms, Paladin, Shock Trooper
   * fixed missing storm trident image (#14111)
   * new observer icon
   * fixed wrong ice missile image used by Elvish High Lord (#14132)
   * fixed Lich using wrong image (#14139)
   * replaced tooltip picture (#14134), note still english and not translatable
   * fixed broken scepter of fire and scepter attack icons (#14208,#14231)
 * user interface
   * fixed fonts sometimes not being found, eg. on BeOS
   * fixed campaign dialog with no campaigns trying forever to download (#14078)
   * fixed some damage calculations not showing %-to-kill
   * fixed game music continuing to play on the title screen (#14080)
   * added hotkey ctrl-j to show objectives (#7830)
   * fixed more problems with observers (also #14239)
   * fixed scrollarea and textbox glitches in MP lobby (#13011,#14124)
   * menus now can acquire keyboard focus (#13118)
   * info text (defense-%, #-turns) is no longer obscured by terrain (#13995)
   * tips of the day now only change when playing a game (#14235)
   * hotkey for taking a screenshot now works in editor also (#13841)
   * show wait cursor for long operations, if not using color cursors (#12670)
   * part-fix campaign text overflowing due to width being too narrow (#13961)
   * fixed tooltips disappearing when using dfool's theme (#14084)
   * prestart events now happen before the player is shown the map (#12957)
 * revised multiplayer maps: Bluewater Province, Charge, Hamlets,
     Hornshark Island, Meteor Lake, Sablestone Delta
 * fixed AI units not getting rest-healing, causing replay errors
 * fixed configure options --with-icondir and --with-desktopdir (#14100)
 * changed Merman hoplite to bring it up to date with changes to steadfast
 * fixed Dark Adept female version glitches (#14073,#14122,#13986)
 * made checksums work on AMD64
 * revised network code, try to use non-blocking I/O if available
 * new description= attribute of [attack], name="x" now implies
     description=_"x" and icon="attacks/x.png" if not specified (#12788,#14113)
 * AI in multiplayer can now recruit healers and mixed fighters (#14096)
 * fix AMLA healing (#13921); recruited/recalled units are now always healed
 * various bug fixes and code cleanups

## Version 0.9.5
 * language and i18n:
   * new translation: Hebrew (no rtl support added in yet)
   * updated translations: British English, Catalan, German,
     Hungarian, Polish, Spanish, Swedish, Turkish
   * fixed linebreaks not working properly for Chinese translation (#13352)
   * updated several unit descriptions
   * added GPL and part of MANUAL to help
   * added more descriptions and images to in-game help (also #12322)
   * fixed some untranslatable attack names (part of #12788)
   * Battle Princess can now have translation different to Princess
   * fix terrain list and time of day cross-references in unit help
 * graphics and sound improvements:
   * fixed layering of multi-hex tiles (#13586)
   * removed Mac OS X icon border (#12928)
   * updated images for Elvish Druid, Elvish Scout, Elvish Shaman,
     Elvish Shyde, Grand Knight, Master Bowman
   * new and revised portraits for The Dark Hordes and Konrad
 * campaigns
   * Eastern Invasion
     * fixed missing background in intro (#13901)
     * campaign plot and dialogue extensively revised
   * Son of the Black Eye
     * fixed untranslatable strings in Saving Inarix
     * start of making Clash of Armies harder: arrival of more reinforcements
   * The Dark Hordes: terminate the campaign at this point as Part 1
   * Heir to the Throne: stop Bugg being resurrected in Bay of Pearls (#13950)
   * old campaign downloaded from server is now removed when updating (#13874)
 * multiplayer:
   * rejoining game as observer, with same nick, no longer causes out of synch
     (#12824 and #12997, still subject to confirmation)
   * fixed chat messages from allies not being shown in multiplayer (#13657)
   * added Use map settings option to preset scenario suggested values (#10669)
   * show game settings in the game list
   * fixed lobby corruption when trying to join a game that disallows observers
   * fixed create-game-then-cancel lobby corruption (#13522)
   * tweaked layout of game creation screen
   * fixed non-fatal errors exiting lobby (#13783)
   * choosing vacant slot then joining all computer game is no longer allowed
   * cancelling faction selection now goes back to scenario setup (#9376)
   * ending game now returns to lobby (#13348)
   * more robust network code: ghost connections should no longer linger forever
 * new units: added Drake Enforcer
 * unit balancing and modifications
   * increased Elvish Scout ranged attack damage from 5-2 to 6-2
   * increased Cavalryman attack damage from 5-3 to 6-3
   * reduced dwarvishfoot and mountainfoot defense on tundra from 40% to 30%
   * simplified AMLA, using global macros in amla.cfg
 * multiplayer scenarios: removed Icy Waters, added Smallolof,
   modified Sulla's Ruins map
 * user interface improvements:
   * fixed failed recruit attempt clearing undo stack (#13833)
   * fixed recall not clearing shroud (#13824)
   * fixed spurious warning when loading a saved game and ending turn (#12986)
   * sound is handled better, and can be set as a preference (rest of #11669)
   * fixed editor insisting on path without spaces, for saving maps (#13919)
   * changed hotkeys for end turn (alt-e->alt-space) and open/load game
     (ctrl-l->ctrl-o)
   * replaced Vera and Bepa-Roman typefaces with DejaVuSans version 1.12
 * fixed toupper causing build to fail on OpenBSD (#13709)
 * fixed replay corruption due to capture events not saving randomness (#13666)
 * keep send and receive connection statistics separate for each socket
 * more responsive networking code, less subject to network errors
 * fixed campaign upload with no passphrase not storing generated passphrase
 * tag [redraw] now forces redraw and is no longer a no-op (#11079)
 * fixed repeated attempts to open non-existent font files
 * various bug fixes and code cleanups

## Version 0.9.4
 * language and i18n:
   * updated translations:
     * Basque, British English, Catalan, Chinese, Dutch, French,
       German, Hungarian, Russian, Swedish, Turkish
   * fixed help description of income and upkeep (#13565)
   * updated several unit descriptions
   * standardized on "magi" as plural of "mage"
   * document that Woses receive no traits (#13630)
   * replay suggested save game now translated (#13558)
   * fixed some typos (#13714,#13715,#13716,#13717,#13718)
   * added terrain descriptions to in-game help
   * fixed blank terrain entries in unit help
   * ignore LANGUAGE in environment: broke translations (forum thread 6614)
   * added Swedish man pages to the list of man pages to be installed
 * unit balancing and modifications:
   * max level units given After-Max-Level-Advancement (AMLA) of 3 HP/100XP
     added to max HP, with no auto-heal; Necrophage has healing AMLA
     with no bonus HP
   * changed Naga Warrior attack from 5-5 to 7-4
   * decreased Thug hitpoints from 36 to 32
   * decreased Dwarvish Ulfserker hitpoints from 40 to 34
   * changed Elvish Shaman and Druid to the state before 0.9.3
   * decreased Elvish Shaman cost from 17 to 16 gold
   * removed obsolete units Mounted Captain, Mounted Commander
 * graphics and sound improvements:
   * new or revised images for Dark Sorcerer, Dwarvish Thunderguard, Necromancer
   * new death animations: Elvish Sharpshooter, Elvish Marksman
   * default colour depth is no longer forced to 16 bits
   * new tiles for some terrain in the editor terrain list and mini map
 * WML improvements:
   * #define's in user campaigns are now local to each campaign (patch 4138)
   * correctly preprocess files that do not end with a final newline
 * campaigns
   * Heir to the Throne:
     * made campaign easier on "Easy" level
     * removed duplicate file inclusion - campaign should now start faster
   * Eastern Invasion:
     * revised Approaching Weldyn for quicker gameplay
     * tweaked Undead Crossing
     * removed duplicate file inclusion - campaign should now start faster
   * The Dark Hordes:
     * cleaned up old id tags; added portraits for Tanar, Nati and Gwiti
     * removed duplicate file inclusion - campaign should now start faster
   * The Rise of Wesnoth:
     * removed duplicate file inclusion - campaign should now start faster
     * fixed Lt. Aethyr death events not triggering in Rise of Wesnoth (#13848)
   * Son of the Black Eye: revised map for Desert of Death
 * server
   * now supports redirection based on client version number
   * can now act as proxy
 * editor
   * changed the layout of the terrain palette (#11965)
   * relocated and enlarged the buttons (#11967)
   * maps can't be saved with illegal characters (#10704)
   * replaced the buttons under the minimap with a toolbar
 * build process
   * added quotes to configure.ac (forum thread 6747)
   * new utility extractsources to extract lists of sources from Makefile.am
   * new utility makemam to construct Makefile.am from Makefile.am.in and
     files containing lists of sources
   * fix libpng not being detected on NetBSD (#13808)
 * user interface improvements:
   * show scrollbar in multiplayer lobby when there are many players (#13521)
   * multiplayer lobby player list no longer resets to start (#13345)
   * fixed game host sometimes not recognized as side owner (forum thread 6783)
 * remove redundant {utils} and {~utils} references (#13843)
 * various bug fixes and code cleanups

## Version 0.9.3
 * user interface improvements:
   * restored dialog titles for multiplayer screens (#13049)
   * --nosound commandline option no longer initializes sound (part of #11669)
   * players can now control multiple sides in multiplayer
   * prevented user campaigns with errors from making the game unstartable
   * user campaigns can no longer redefine standard units
   * multiplayer replays no longer show 'The End' and credits (part of #13375)
   * fixed off-by-one error in Unit List (#13323)
   * fixed multiple Returns registered by some dialogs (part of #13332)
   * "Observers" setting for multiplayer games is now obeyed again (#13374)
 * language and i18n:
   * fixed 'make dist' not removing stamp-po files, causing broken translations
   * updated translations:
     * British English, Catalan, Finnish, German, Hungarian, Swedish, Turkish
   * updated MANUAL: English (also synched with wiki), German, Swedish
   * man pages: updated English and German, added Swedish
   * added descriptions to all multiplayer maps (#9763)
   * user visible error messages are now translatable (#12605)
   * added files to POTFILES.in: fix untranslatable strings (#13350)
   * fixed untranslatable strings on Windows (#13347)
   * fixed user campaign translations not working on Windows (#11848)
 * campaigns
   * Eastern Invasion:
     * made Captured and Approaching Weldyn easier
     * reduced the turn limit in The Drowned Plains
   * Heir to the Throne:
     * made campaign easier on "Easy" level
     * fixed Hasty Alliance crash: avoid cavewall endpoints (part of #13334)
     * updated story image 4 to remove glitch (#13385)
   * Son of the Black Eye:
     * made To The Harbour of Tirigaz and Black Flag easier
 * graphics and sound improvements:
   * new or modified images for Dwarvish Thunderer, Elvish Outrider,
     Elvish Rider, Elvish Scout, Giant Mudcrawler, Goblin Knight,
     Goblin Pillager, Mudcrawler, Orcish Warlord, Wolf Rider
   * new death animations for Dwarvish Thunderer, Elvish Archer,
     Elvish Avenger, Elvish Ranger, Giant Mudcrawler, Mudcrawler, Spearman,
     Young Ogre, Nightgaunt, Spectre
   * new orc portraits, now used in Heir to the Throne
   * new unit portraits for Bandit, Grand Knight, Halberdier, Horseman,
     Javelineer, Knight, Lancer, Paladin, Peasant, Pikeman, Royal Guard,
     Swordsman, Thug
   * new portrait of Asheviere
   * updated icons
 * terrain graphics
   * new multihex snow forest
   * new snow transition
   * new ice tiles
   * new dirt tiles
   * new villages
 * new units
   * added Orcish Slurbow and Dark Sorcerer
 * unit balancing and modifications:
   * applied 0.9.2 changes to Thief, Elvish Archer, Elvish Ranger,
     Elvish Avenger to female units also
   * steadfast now doubles resistances up to 50%; vulnerabilities are unaffected
   * decreased Dwarvish Guardsman line resistance to cold from 20% to 10%
   * decreased Dwarvish Stalwart defense in grassland from 50% to 40%
   * increased Dwarvish Ulfserker cost from 18 to 19
   * restored Dwarvish Berserker resistances from 0.8.11
   * changed Thug attack from 6-3 to 5-4
   * increased Poacher experience needed to advance from 24 to 29
   * increased Orcish Shaman (Novice and Old also) hitpoints from 18 to 20
   * increased Pikeman attack from 9-3 to 10-3
   * increased Master Bowman hitpoints from 46 to 50
   * increased undeadspirit (Ghost line) resistance to fire from 0% to 10%
   * decreased Ghost, Shadow and Nightgaunt resistance to impact from 60% to 50%
   * changed Ghoul and Necrophage resistances: impact -10% -> 0%,
     fire -20% -> 0%, cold 60% -> 40%
   * decreased undeadfoot movement cost on mountains from 4 to 3
   * decreased undeadfoot movement cost on hills from 3 to 2
   * Demilich now advances to Dread Lich (previously Ancient Lich)
   * increased Naga hitpoints from 30 to 33 and experience from 25 to 32
   * increased Naga Warrior hitpoints from 40 to 43
   * decreased Naga Warrior experience needed to advance from 70 to 66
   * decreased Naga Warrior attack from 6-5 to 5-5
   * decreased Merman Hunter ranged attack from 6-3 to 5-3
   * decreased Merman Spearman ranged attack from 7-4 to 6-4
   * decreased Merman Spearman melee attack from 6-3 to 6-2
   * decreased Merman Spearman hitpoints from 45 to 40
   * decreased Merman Netcaster hitpoints from 42 to 40
   * increased Merman Netcaster ranged attack from 7-2 to 9-2
   * increased Merman Fighter hitpoints from 32 to 36
   * increased Merman Warrior hitpoints from 48 to 50 and attack from 7-4 to 8-4
   * decreased Elvish defense in swamp from 40% to 30%
   * increased Elvish Shaman ranged attack from 3-2 to 4-2
   * decreased Elvish Shaman melee attack from 3-2 to 2-2
   * decreased Elvish Druid melee attack from 4-2 to 3-2
   * Elvish Shaman and Druid are now able to inflict slow through melee
   * decreased Elvish Shaman cost from 18 to 17
   * increased movement cost of movetype "fly" units in caves from 2 to 3
   * increased Drake Clasher line movement cost in caves from 1 to 2
   * decreased Drakes' defense in caves from 40% to 30%
   * increased Revenant hitpoints from 40 to 43
   * increased Deathblade hitpoints from 32 to 35
   * increased Skeleton Archer ranged attack from 4-3 to 6-3
   * increased Skeleton Archer experience needed to advance from 30 to 35
   * increased Bone Shooter melee attack from 5-2 to 6-2
   * increased Bone Shooter ranged attack from 7-3 to 9-3
   * increased Soul Shooter melee attack from 6-2 to 8-2
   * increased Soul Shooter ranged attack from 11-3 to 12-3
   * Rebels can now choose White Mage as leader in Default era
   * Rebels can now choose Mage of Light as leader in Age of Heroes era
   * Nagas can no longer move on mountains
 * multiplayer scenarios
   * added Sablestone Delta
   * modified Den of Onis, Hornshark Island, Meteor Lake, Silverhead Crossing
 * server
   * no longer rejects: control command with "Side not found"
   * write a summary of commands when a fifo command is not recognized
   * fix assertion failure in network_worker.cpp
 * move_unit_fake with illegal coordinates now logs error instead of
   causing assertion failure crash (part of #13334)
 * various bug fixes and code cleanups

## Version 0.9.2
 * user interface improvements:
   * sped up frame rate when scrolling the map
   * connecting to a server now shows dialog that allows the user to cancel
     the connection rather than blocking (#12614)
   * sped up help
   * added hotkey for cycle to previous unit (shift-N) (part of #10703)
   * added hotkey for hold position (shift-space) (patch 3994)
   * made the right Command key on Mac OS X work like the left one
   * made menu widgets sortable
   * made network dialogs show progress of data transfers again
   * added display of unit defense over the terrain when choosing move
   * added visual cue for movement in specific terrain when choosing move
   * added %-to-kill to Damage Calculations
   * fixed female units not appearing in help (broken since 0.9.0)
   * added support for unit portraits in help (forum thread 6017)
   * reduced required width of weapon area in help
   * fixed items appearing in traits description (#12603)
   * preserve trait ordering to distinguish quick,resilient and resilient,quick
   * improved layout of objectives dialog
   * made the text of disabled buttons grayed out
   * made room for observers in DFool theme (#13027)
   * added clock to DFool theme (#10650)
   * added Experimental theme
   * tweaked multiplayer lobby
   * improved position and size of 'users' menu in multiplayer lobby (#13120)
   * selecting colors for multiplayer sides now works correctly (#13255)
   * typing a chat message quickly no longer lags the game (#12097)
   * pasting multiline text now discards lines after the first (#12282)
   * better checking for 'control' command arguments in multiplayer (#13086)
   * added 'Advanced' preferences: 'binary save files', 'show combat'
   * fixed village name being shown over shroud (#10690)
   * made ordering of terrain data consistent (#10665)
   * display error if save cannot be completed (eg. disk full) (#13232)
   * fixed halo position when unit is in water (#12493)
   * fixed titlescreen background disappearing on switch to fullscreen (#11863)
   * disabled mousewheel scrolling during combat and unit movement (#12021)
   * fixed pathfinding issues with respect to unreachable hexes (#11480, #13295)
 * campaign improvements:
   * Eastern Invasion:
     * Unexpected Appearance: fixed Dacyn not being [recall]-ed (#12830)
     * Evacuation: units left on the wrong side of river now really die (#10619)
     * Northern Outpost: killing enemies in "wrong" order is now a win (#12922)
     * Northern Outpost: added a Holy Water bottle
     * Captured: fixed bugs (#10512, #12998), but replaced scenario anyway
     * Drowned Plains: new map and scenario modification, bug fixed (#13013)
   * Heir to the Throne:
     * Siege of Elensefar: fixed thieves not appearing (forum thread 5719)
   * Son of the Black Eye:
     * prevented the shamans from being recallable (#11932)
     * Siege of Barag Gor: killing enemy leader is no longer a win
     * End of Peace: increased turn limit 24 -> 26 due to Lieutenant upgrade
     * Desert of Death: fixed invalid type=RogueAssassin on Hard
     * removed duplicate file inclusion - campaign should now start faster
     * more accurate difficulty labels
     * Toward Mountains of Haag: fixed enemy making "never pushed so far" speech
 * new units
   * added Drake Warden, Hurricane Drake and Drake Blademaster
 * unit balancing and modifications:
   * slow now affects units with 2 or more attacks (not just 1 remaining)
   * slow now works with berserk and is persistent across berserk rounds
   * changed Lieutenant attack from 6-3 to 9-3
   * reduced Fencer cost from 18 to 16
   * changed Iron Mauler attack from 22-2 to 25-2
   * changed Soulless attack from 7-2 to 7-3
   * changed Soulless attack from plague to plague(Walking Corpse)
   * Walking Corpse created by plague can advance to Soulless again (#13056)
   * reduced Skeleton Archer cost from 15 to 14
   * changed Skeletal Dragon resistances (only used in Eastern Invasion)
   * tweaked Watch Tower, Pirate Galleon and Transport Galleon and removed
     their multihex attacks (only used as real units in Son of the Black Eye)
   * reduced Dwarvish Fighter cost from 17 to 16
   * reduced Dwarvish Fighter line Axe (blade) damage by 1 point
   * reduced Dwarvish Thunderer cost from 19 to 17
   * reduced Gryphon Rider cost from 25 to 24
   * Dwarvish Ulfserker stats reverted to those from version 0.8.11
   * increased Thief cost from 12 to 13
   * changed Elvish Lord and Elvish High Lord's ranged attack from fire to cold
   * changed Elvish Ranger melee attack from 7-4 to 7-3
   * changed Elvish Avenger melee attack from 10-4 to 9-4
   * increased Walking Corpse cost from 5 to 6
   * trolls can no longer get "intelligent" random trait
   * increased Troll Whelp experience needed to advance from 32 to 33 XP
   * increased Troll experience needed to advance from 52 to 60 XP
   * Goblin Knight -> Direwolf Rider upgrade reduced from 150 to 65 XP
   * added flaming arrow to orcish archer line as a new attack
   * reduced Saurian Skirmisher movement to 6, increased its cost from 14 to 15
   * changed Drake Gladiator from 65 to 59 HP
   * changed Drake Gladiator pierce resistance from 10% to -10%
   * changed Drake Slasher from 59 to 65 HP
   * changed Drake Slasher pierce resistance from -10% to 10%
   * increased Drake Slasher and Drake Gladiator cost to 45
 * Doc Paterson's modifications to the MP map catalogue
   * added "Divide and Conquer", "Silverhead Crossing", "Meteor Lake",
     "Den of Onis", "Hamlets", "Hornshark Island", "Sulla's Ruins",
     "1v1v1Hex" (3p), "Lagoon" (4p), "The Wilderlands" (4p),
     "3-player Morituri", "3-player Blitz (Triple Blitz)"
   * removed "Broken Bridge", "Battle For Weslin Bridge",
     "Princess's Battlefield", "Three Rivers", "The Isles of the Damned",
     "The Isle of Anduin", "Dwarven Wasteland"
 * unit graphics and sound improvements:
   * new death animations for: Ancient Lich, Ancient Wose, Arch Mage,
     Assassin, Bandit, Battle Princess, Blood Bat, Bone Shooter, Bowman,
     Cavalier, Cavalryman, Cave Spider, Chocobone, Cockatrice,
     Commander, Cuttle Fish, Dark Adept, Dark Queen, Dark Spirit,
     Deathblade, Death Knight, Deathmaster, Demilich, Direwolf Rider,
     Dragoon, Duelist, Draug, Drake Burner, Drake Clasher, Drake Fighter,
     Fire Drake
   * new or modified graphics for Dragoon, Dwarvish Dragonguard,
     Elvish High Lord, Elvish Lord, Halberdier, Lady Parandra, Noble Lord,
     Royal Guard, Soulshooter, Swordsman
   * fixed several image files being referred to by wrong name
   * new Soulless variation images
   * added flaming arrow images
 * language and i18n:
   * language fixes and polishing (English) (also fixed #12613, #10714)
   * updated MANUAL
   * new translations: Serbian
   * updated translations: Afrikaans, British English, Catalan, Chinese,
     Czech (#12864), Estonian, Finnish, French, German (also fixed
     #13147), Greek, Hungarian, Italian, Japanese, Polish, Russian,
     Slovenian, Spanish, Swedish, Turkish
   * updated gettext support to GNU gettext 0.14.4
   * removed the intl/ directory, since libintl is now widespread enough
     for gettextize to default to not installing it
 * WML improvements:
   * new WML preprocessor, allows for nested parentheses in macros (#10995)
   * note= attribute for [objectives], shown as footer, eg. for hints (#12927)
   * increase_attacks= attribute of [effect] now allows percentages (#13033)
   * new [random] representation as list allows complex scenarios to be saved
     (forum thread 5659)
   * better diagnostics on parsing: show file inclusion sequence
   * added [scroll_to] (patch #3388, forum thread 3235)
   * added [advancefrom] (patch #3625, forum thread 4186)
   * [recall] tag can now work even if the recruiter is not in a keep
     (#10543, #11735, #12974)
   * next_scenario: tentative start of MP campaign support
   * tidied up game.cfg and traits.cfg; game startup should now be faster
 * editor improvements:
   * fixed editor file chooser when starting directory has many files (#11698)
   * the starting position in the editor now starts counting from 1 (#10625)
 * improvements and bug fixes of the logging system
 * fixed replays with idle_ai, as seen in user scenario Rebellion (#12943)
 * saving during an AI unit's turn no longer makes that unit disappear (#13023)
 * fixed Windows build crashing when trying to recruit units (#12926)
 * tutorial start and end scenario savegames can now be loaded (#10332)
 * various bug fixes and code cleanups (including #13264 #12954 #12734 #13263)

## Version 0.9.1
 * user interface improvements:
   * added a shortcut for making screenshots
   * added support for Home/End in menus, ctrl-a/e/u in textboxes
   * fixed the formatting of wrapped text lines being lost
   * fixed the screen being incorrectly redrawn when resizing
   * fixed bug #12654 causing prestart events to be shown to the player
   * fixed bug #12758: gendered unit types repeated in attack type tooltip
   * fixed bug #12612: no description in help for units at first encounter as upgrade
   * fixed excess space in MP "/me " emotes
 * campaign improvements:
   * fixed bug in Northern Winter (HttT) where the Orcs attack each other
   * fixed the 'knockback' for WML Plague in TRoW
   * fixed missing music in the Crossroads (HttT)
 * multiplayer improvements:
   * fixed the lobby chat window not wrapping text
   * fixed the lobby chat staying at top when changing lobby screens
   * fixed the multiplayer "wait" lobby screen not correctly displaying empty and vacant slots.
   * made renaming units visible on replays and on multiplayer games
   * fixed bug #12759: "number of turns" not being remembered when creating multiplayer games
   * fixed bug #12791: Statistics doesn't reset in multiplayer games.
 * terrain improvements:
   * new forest tiles
 * unit balancing:
     * Dwarvish Guardsmen line removed as leaders in multiplayer
     * Dwarvish Guardsmen line only get 1 trait instead of 2
     * revert Drakes pierce resistance to 110%
     * revert Orcish Assassin to 3-3 ranged attack
     * reduced resistance of Ulfserker and Berserker
     * Elvish Rider: increase melee to 6-4, increase ranged to 7-3
     * Elvish Outrider: increase melee to 8-4, increase ranged to 7-4
 * language and i18n:
   * updated English and German manpages
   * language fixes and polishing (English)
   * fixed the description of traits in the help & tutorial
   * fixed user-campaigns not being translatable anymore
   * fixed some untranslatable strings
   * new translations:
     * Turkish
   * updated translations:
     * Afrikaans
     * British English
     * Catalan
     * Chinese
     * French
     * German
     * Hungarian
     * Italian
     * Japanese
     * Slovenian
     * Swedish
   * added a ./configure --enable-dummy-locales option for Wesnoth to generate
     its own set of i18n locale files and use them instead of the potentially
     missing system files (highly experimental)
   * made the width of the non-breaking space in Vera font be half that of
     the normal space, instead of twice that width.
 * added support for num_traits in individual unit_types overriding race num_traits
 * fixed the missing 'elder-mage-halo7.png' in the Elder Mage
 * fixed the advanceto field in Dark_Adept.cfg
 * fixed some OoS errors when chatting while a fight is taking place
 * fixed the spurious "statistics verification failed" messages
 * fixed bug #12611: modifying villages with [terrain] interacting badly with capturing them.
 * fixed bug #12698: segfault with --decompress on relative path
 * various bug fixes and code cleanups

## Version 0.9.0
 * user interface improvements:
   * added alternative theme: DFool
   * added theme dialog, launched with ":theme"
   * made help use single click instead of double click
 * campaign modifications
   * tutorial polishing
   * updated 'The Siege of Elensefar', 'Mountain Pass', 'The Swamp of Esten',
     and 'A Final Spring' to use the new ford terrain (Heir to the Throne & The Rise of Wesnoth).
   * modified the behaviour of the thieves in 'The Siege of Elensefar' (Heir to the Throne)
   * updated Heir to the Throne and The Rise of Wesnoth to use the ruined keeps where appropriate
   * dialog improvements in Eastern Invasion
   * new map for Drowned Plains (Eastern Invasion)
   * changed the Wesfolk Leader's guards to poachers in 'The Fall' (The Rise of Wesnoth)
 * multiplayer improvements
   * rewrote multiplayer lobby code
   * changed Broken_Bridge to use new ruined castles
   * added new multiplayer maps: Icy Waters, Clash
   * changed King_of_the_Hill to use a Dwarvish Castle
   * chat messages are now displayed in the user's colour when prefixed with /me
   * split the map and scenario file for 'An Island'
   * disabled debug mode for multiplayer games
   * made it possible to join games using eras which only are on the host-side
   * made it possible for the server to redirect the clients to another host
     (client-side support only for now)
 * terrain improvements
   * improved transitions:
     * swamp
     * keep to keep
   * added new terrains:
     * ruined castle
     * sunken ruin
     * swamp-ruin
     * river ford
     * lava
 * unit graphics and sound improvements
   * updated images for several units: Cave Spider, Haldric, Lady Jessica,
     Mermaid Priestess, Mermaid Diviner, Merman Hunter, Merman Javelineer,
     Merman Spearman,Walking Corpse, Elvish Fighter, Captain, Marshal, Hero,
     Champion, Archer, Marksman, Sharpshooter, Ranger, Avenger, Shyde, Orcish Archer,
     Orcish Crossbowman, Human Spearman
   * added drake, mounted, saurian, swimmer, troll, and wose variations to Walking Corpse
   * updated Haldric's portrait in The Rise of Wesnoth
   * added graphics for generic Lich-Lords, the Archmages in 'The Sewer of Southbay',
     and the Lady Outlaw (The Rise of Wesnoth)
   * added support for animated deaths, and added animations for Skeleton and
     Revenant, and all Walking Corpse variations
   * fixed several image files being referred to by wrong name
   * removed sound of thrown spear from Drake Clasher spear melee attack
   * added get_hit_sounds to Elder Mage (Delfador) and Elvish Lady (Parandra)
 * unit balancing:
   * Soulless: dropped cost to 11, increased  HP to 28
   * Merfolk: defense on water and swamp reduced by 10%.
   * Merman Hunter cost: 13 -> 15. Melee attack: 5-2 -> 4-2.
   * Naga: defense on shallow water reduced by 10%.
   * Naga Fighter cost: 13 -> 14.
   * Deep sea creatures: defense on water reduced by 10%.
   * Orcish Assassin: increased cost by 1 gold
   * Troll Whelp:  increased cost by 1 gold
   * Gryphon units: improved defense on mountains.
   * Steadfast dwarvish units: increased movement speed.
   * Dwarvish Thunderer: improved melee attack to 6-2, and 3 more HP
   * Dwarvish Guardsman:  improved attack to 5-3.
   * Dwarvish Fighter: 2 more HP
   * Berserkers: made berserk only active on offense.
   * Elves: added the race specific 'dextrous' trait (increases damage on ranged attacks)
   * Drakes: made them lawful
   * Drakefly: increase pierce vulnerability to 20%
   * Drakefoot: increase pierce vulnerability to 10%
   * Drake Burner: decrease HP to 42, changed ranged attack to 6-4
   * Drake Clasher: decrease HP to 43
   * Drake Fighter: decrease ranged attack to 3-3
   * Fire Drake: decrease HP to 54, decrease ranged to 7-5
   * Drake Flameheart: decrease HP to 84, increased ranged to 8-4
   * Drake Flare: decrease HP to 62, increased ranged to 7-4
   * Drake Gladiator: reduce HP to 65
   * Drake Glider: increase cost to 16, increased ranged to 3-3
   * Inferno Drake: decrease HP to 71, increase ranged to 8-6
   * Sky Drake: increase cost to 27, increased ranged to 5-3
   * Drake Slasher: reduce HP to 59
   * Drake Warrior: reduce ranged attack 5-3
   * made 'loyal' a trait which is only given to units explicitly by the scenario designer
 * language and i18n:
   * language fixes and polishing (English)
   * fixed hero names not being translatable
   * made text strings automatically choose an appropriate font
   * fixed untranslatable strings
   * support to display Chinese and Japanese translations
   * new translations:
     * Afrikaans
     * Chinese
     * Estonian
     * Japanese
   * updated translations:
     * Basque
     * British English (100%)
     * Catalan
     * Dutch (100%)
     * Finnish
     * French
     * German (100%)
     * Greek
     * Hungarian
     * Italian
     * Latin,
     * Norwegian
     * Polish
     * Portuguese (Brazilian)
     * Russian
     * Slovak
     * Slovenian
     * Spanish
     * Swedish (100%)
 * WML modifications
   * rewrote WML parser with stricter grammar and better error handling
   * changed the [movement costs] WML tag into [movement_costs]
   * added generate_description to [unit], to allow automatic generation of user_description
   * made [capture_village] accept a location range
   * added support for [objectives] event actions, to set objectives.
   * added the ability for sides to have different objectives.
   * added UNIT, UNDEAD_UNIT, and PLACE_IMAGE macros to utils.cfg
 * changed plague to allow plague(unit_type) as well as undead_variation
 * more progress towards a completely-usable 320x240 PDA port (campaigns should now be playable)
 * fixed bug that made the ai recruit badly
 * fixed auto-move using up all of the remaining moves when
   there are no empty hexes on the unit's path (#9649)
 * fixed merged castles in random maps (#8848)
 * fixed villages in the rightmost column not getting a name (#11152)
 * fixed drawing error in the titlescreen (#12534)
 * added debug command to give gold to current side
 * fixed user campaigns appearing twice with zipios enabled
 * slight cleanup of the terrain graphics code
 * added a --disable-game configure option to avoid compiling and installing the game
 * share the object files between all the programs to halve the time of a full compilation
 * disabled "End turn" button when it is irrelevant.
 * added OS X ScrapManager support
 * improved threading of campaign server
 * fixed the dialog menus being larger than the screen
 * made the game remember the last server used for downloading campaings
 * when building out of the source tree, configure now creates the translations/
   directory in the source tree, but does not attempt to do so if it is read-only
 * code refactoring and dependency cleanups
 * various bug fixes and code cleanups

## Version 0.8.11
 * new translation:
   * British English
 * updated translations:
   * Basque
   * Catalan
   * French
   * German
   * Italian
   * Latin
   * Slovenian
   * Swedish
 * in language-selection dialog, consistently use native name for all languages,
   with the (hopefully) official latin transcription for those that cannot be displayed
   in the default font (russian, bulgarian, greek)
 * language fixes and polishing (English)
 * fixed slovene being wrongly localized as sl_SL insteof sl_SI
 * campaign fixes and changes:
   * fixed Delfador appearing in the water when turns run out in 'The Bay of Pearls' (HttT)
   * updated maps for: 'Northern Winter' & 'Mountain Pass' (HttT)
   * updated 'Evacuation' & 'Captured' (EI)
   * fixed the talking tentacle in 'Temple of the Deep' (TroW)
   * switch to the new Merfolk and Nagas in TRoW and HttT
 * unit fixes and changes:
   * fixed several units using non-existing sound files
   * added sounds (existing) to several attacks
   * updated all uses of the 'old fireball' to use the 'new fireballs'
   * added new images and animations for all mages
   * updated the attack frame sequences for the Drakes
   * added attack icons for 'slam' and 'ballista'
   * obsoleted all instances of the old 'Mermen' & 'Naga' unit lines
   * modified defense weight for Drake Slasher
   * decreased hitpoints for Gate
   * renamed 'Elvish Lady Parandra' 'Elvish Lady'
   * made all mages use staff (impact) as melee attack
   * added support for directional attack animations
 * fixed zipios support so that user campaigns custom images and sounds can be loaded
 * fixed locale-dependent numeric input/output bug, it allowed units to walk on the water,
   and prevented the AI from being correctly configured in campaigns
 * use fixed-point arithmetics in critical functions, so that wesnoth can
   reasonably run on an FPU-less machine (eg. ipaq PDA)
 * fixed the width of 'HP' and 'XP' in the right side panel
 * fixed some multiplayer connect bugs
 * fixed the AI not going through the no-ZoC of lv0 units when it should have
 * code cleanups and bug fixes

## Version 0.8.10
 * user interface tweaks
 * updated translations:
   * catalan
   * french
   * german
   * latin
   * russian
   * swedish
 * updated manpages
 * language fixes and polishing (English)
 * updated images for footpad and dwarvish sentinel
 * scenario revisions for 'The Heir to the Throne' campaign:
   * Dwarven Doors & Siege of Elensefar- Decreased the AI's gold
   * Bay of Pearls - Added an extra keep square on EASY
   * Sceptre of Fire- Reduced that AI's income on HARD
   * Northern Winter- Updated the map to use the snow tiles
   * tweaks to several maps
   * difficulty balancing for most scenarios
 * scenario revisions for 'Son of the Black Eye' campaign:
   * End of Peace
 * scenario revisions for 'The Eastern Invasion' campaign:
   * Escape Tunnel
 * scenario revisions for 'The Rise of Wesnoth' campaign:
   * reduced the AI's gold in 'A Harrowing Escape'
   * allowed the AI to initially recruit scouts in several scenarios
   * altered / fixed the recruitment pattern in several scenarios
   * tweaks to 'The Plan' & 'Epilogue' (both non-playable)
 * really removed ZoC from lv0 units this time
 * fixed editor to work with zipios support enabled
 * fixed zipios support to honor path to data given on command-line
 * fixed zipios support crashes when running from CVS
 * new cmd line option --fps to display frames/second refresh in game
 * modified how damage is calculated (RATE)
 * made unit movement smoother
 * fixed poison not being cured when the unit had too much HP (#11565)
 * fixed bugs when replacing player with observer in mp (#11231)
 * fixed several pathfinding bugs (#11769)
 * fixed cave random map generation, especially Scepter of Fire (#11748)
 * fixed dialogs shown at end of MP game (#11830)
 * fixed units not being correctly displayed on minimap for odd x-locations.
 * added several fortunes
 * code cleanups and bug fixes

## Version 0.8.9
 * new translations:
   * bulgarian
   * latin
 * updated most translations
 * new or updated portraits:
   * Konrad, Delfador, Li'sar and Kalenz (Heir to the Throne)
   * Gweddry and Dacyn (Eastern Invasion)
 * added missing animations for:
   * Mermaid Initiate, Mermaid Enchantress and Mermaid Siren
   * Ancient Wose
   * Goblin Direwolf Rider and Goblin Knight
   * Gryphon
   * Skeleton Archer, Revenant and Death Knight
   * Troll Warrior, Troll Hero and Great Troll, Young Ogre
   * Grand Knight, Lord and Master Bowman
   * Cuttlefish
 * new or overhauled graphics for:
   * Footpad, Outlaw, Thief and Rogue
   * Orcish Shamans, Elvish Lord and Elvish High Lord
   * Ghost, Nightgaunt, Shadow, Spectre and Wraith
 * new female units for:
   * Footpad, Thief, Rogue and Assassin
 * added new tiles:
   * desert
   * desert hills
   * desert mountain
   * desert villages (adobe and tent)
   * desert road
   * desert oasis
   * savanna
   * tropical forest
   * tropical forest village
 * added icon for petrified units
 * fixed missing diagonal projectiles on many units
 * removed old unused images:
   * time-of-day
 * downloaded campaigns no longer require a restart to be able to play them
 * new units:
   * Javelineer
 * unit balancing:
   * Naga fighter: increased hitpoints from 27 to 30
   * Merman Hoplite: increased resistances and damage
   * Mermaid Initiate/Enchantress/Siren: reduced number of attacks,
     increased damage, removed special resistances
   * Mermaid Initiate: increased experience needed to level, other minor tweaks
   * Swordsman: increase resistance to blade and impact
   * Royal guard: increase resistance to blade and impact
   * Pikeman: increase resistance to pierce, change attacks number and damage
   * Halberdier: increase resistance to pierce, increase blade damage, decrease pierce damage
   * Saurian Tribalist, Saurian Icecaster, Saurian Soothsayer: remove 'skirmish'
   * Saurian Skirmisher: Increase cost to 14 gold
   * Wraith, Spectre: reduce resistance to blade, pierce, impact by 20%, increase HP by 10%
   * Poacher: Reduce cost to 14
   * Tentacle of the Deep: Change movement type to 'float'
   * Flying creatures can now cross canyons
   * Removed ZoC from level 0 units
 * language fixes and polishing (English)
 * added 'x' to the terrain letters reserved for campaign writers
 * scenario revisions for 'The Eastern Invasion' campaign:
   * Lake Vrug
   * Two Paths
   * Northern Outpost
   * Elven Alliance
   * Unexpected Appearance
   * Escape Tunnel
   * Tribal Warfare
   * Approaching Weldyn
 * scenario revisions in Heir to the Throne:
   * Blackwater Port- Make the port look like more of a port, other minor changes that mute each other out
   * The Isle of Anduin- Extend shallow water around the Isle, remove the Saurians in favour of Goblins
   * The Bay of Pearls- Flip the map so the Ocean is on the West side
   * Muff Malal's Peninsula- Make it a peninsula, give Konrad the option of fighting (bonus) or fleeing
   * Isle of the Damned- Touched up the map so it is clear that the islands
     are islands, made the islands tropical, and moved the temples
   * The Siege of Elensefar- New map, and other changes
   * The Crossroads- New map, weaker more diffuse ambushes, and less gold for the Orcs
   * The Princess of Wesnoth- Minor map changes, and made the scenario harder
   * Set result=continue in 'Plunging into the Darkness'
 * scenario revisions in The Rise of Wesnoth:
   * fixed {CROSS} in 'Temple of the Deep'
   * fixed sign glitch
   * reduced the difficulty of TRoW on 'Hard' particularly, and reduce
     the difficulty slightly on 'Easy' & 'Normal'
   * A New Land - Sync with the map from Bay of Pearls
   * Cursed Isle- Sync with the map from Isle of the Damned, make the temples match locations
   * Made 'Peoples in Decline' and 'Rough Landing' look tropical
 * multiplayer improvements
   * multiplayer game settings (map name, gold per village, ...) are saved now
   * added several missing units
   * Random Map (Desert) uses new desert environment; also tweaked
   * make the cave in 'An Island' (MP) be 'underground'
   * enlarge side descriptions for remote player during game creation
   * added support for player nick tab completion and /me-substitution in game chat
 * theme syntax improvements:
   * added support for specifying a reference rectangle when using relative-positioning syntax
   * added support for theme inheritance; avoids duplicating mostly-unchanged code
 * added a "droid" game command, and allow for computer player to do a campaign
 * added a "canrecruit" filter
 * updated manpages and added support for translating them
 * "," is no longer the column separator, it does not have to be escaped
   anymore in some translations, "=" is the new separator (#10368)
 * take advantage of libzipios++ if available, to read cfg files,
   maps, images, sound effects, and fonts from zip files.
   Call "make zip-install" to install data zipped.
 * removed drain allowing to go past maximum hp
 * removed long-range attacks (Pirate Galleon, Transport Galleon, Wall Guard, Watch Tower)
 * fixed various bugs with respect to multiplayer game setup
   (#10896, #11236, #11265, #11442, #11527)
 * added new pathfinder (Redsun)
 * fixed some pathfinding bugs
 * fixed untranslatable strings
 * fixed "damage inflicted" statistic (incorrectly computed when defending)
 * fixed that move-and-attack allowed attacking stone units
 * fixed the side turn event being fired incorrectly
 * fixed repeat recruit hotkey not respecting WML recruit changes
 * removed support for turning off genders
 * removed obsolete tools: make_translation merge_translations
 * removed obsolete id= for messages in TRoW & HttT
 * code cleanups and bug fixing
 * added a wesnoth "fortunes" file

## Version 0.8.8
 * campaign server stores size of campaigns
 * user interface improvements:
   * consistently compute minimum allowed zoom value according to screen size
   * improved readability of chat messages on light backgrounds (#10900)
 * made it possible to move and attack with a single click
 * graphics improvements:
   * portraits for all TRoW characters that speak in more than one scenario added
   * a portrait for Queen Asheviere in HttT
   * hotseat and vs AI multiplayer icons
   * new or modified unit images: orcish warlord, soul shooter, bone shooter
   * preparations for mountain and desert village graphics
   * new attack icons: bone arrow stab, cleaver, morning star, crush
 * scenario fixes for 'Eastern Invasion' campaign:
   * Weldyn Under Attack (#11051)
   * Capture
   * fixed end of campaign settings
 * scenario fixes for 'Son of the Black Eye' campaign:
   * Saving Inarix
 * scenario fixes for 'The Rise of Wesnoth' campaign:
   * The Vanguard, fixed chest placement
   * make the chests squeak
 * scenario fixes and balancing for 'Heir to the Throne' campaign:
   * A Test of the Clans
   * Return to Wesnoth
 * unit balancing:
   * Goblin Pillager: reduced HP
   * Goblin Impaler: reduced HP
   * Gryphon Rider: reduced cost
   * Drake Clasher: reduced cost, increase resistance to pierce
   * Drake Gladiator: reduced cost, increase resistance to pierce
   * Drake Slasher: reduced cost, increase resistance to pierce
   * Drake Burner: increase cost
   * Elvish Scout: increased ranged damage, reduced defense in forest
   * Elvish Rider: reduced defense in forest
   * Elvish Outrider: reduced defense in forest
   * Soul Shooter: melee changed from impact to pierce
   * Bone Shooter: melee changed from impact to pierce
   * Mage: reduced cost
   * minor adjustments to Drakefoot & Drakefly (& the Drakes in general)
   * LESS_NIMBLE_ELF macro
 * removed obsolete units:
   * Cavalry
   * Goblin Direwolver
   * Heavy Infantry
   * Merman Lord
   * Orcish Crossbow
   * Scout
 * new units:
   * Draug
   * Naga Fighter
   * Naga Warrior
   * Naga Myrmidon
   * Saurian Flanker
   * Deathblade
 * changed Dragoon's and Cavalier's pistol to crossbow.
 * renamed 'Saurian' to 'Saurian Skirmisher' and 'Saurian Warrior' to 'Saurian Ambusher'
 * language fixes and polishing (english)
 * revised MANUAL
   * english
   * swedish
 * updated translations:
   * catalan
   * french
   * german
   * greek
   * hungarian
   * italian
   * polish
   * portuguese
   * russian
   * slovak
   * spanish
   * swedish
 * multiplayer improvements
   * Charge map
   * CastleHoppingIsle map updated
 * give Woses, Saurians, Ogres, & Drakes names
 * give Saurians & Drakes traits
 * give the Drakes music in Multiplayer
 * new rectangle syntax allowing width/height and relative positionning in themes
 * new --enable-tinygui configure flag for adventurous PDA users
 * fixed units incorrectly not using the spear icon
 * fixed missing diagonal projectiles on several units
 * fixed Gwiti's animation (#10926)
 * fixed translations being searched for in installdir when running in builddir
 * added support for concatenating strings in wml files, for the benefit of i18n
 * fixed code for handling objects (#10904, #10954, #10963)
 * fixed many untranslatable strings
 * fixed layering of desert and dirt
 * fixed multiplayer setup screen being broken on low resolutions (#10919)
 * fixed resizing multiplayer lobby creating some graphic glitches
 * fixed --enable-lite for current image location
 * fixed wmlxgettext not working properly with msdos line-endings in multiline strings
 * fixed documentation (#10999)
 * fixed zoom behavior (#9890)
 * fixed unit teleportation (#10588, #11213)
 * fixed several crashes (#10959, #11102, #11115, #11158)
 * improved 320x240 resolution support
 * support for unrenamable units
 * install wmlxgettext so that it can be used by user campaigns
 * rewrote hotkeys code
 * rewrote widgets code
 * image and data-file cleanups
 * code cleanups
 * WML bug fixes

## Version 0.8.7
 * user interface improvements:
   * display savegame version in load dialog
   * shadows for labels to make them visible on each type of terrain
   * help system speed optimizations
 * graphics improvements:
   * bridge, ice, castle floor, grassland
   * unit images and attack animations
   * Haldric, Burin, Edmond
   * The Green Isle map (The Rise of Wesnoth)
 * in-game help updates and enhancements
 * scenario balancing for 'Heir to the Throne' campaign:
   * Crossroads
 * scenario balancing for 'The Eastern Invasion' campaign:
   * Weldyn under Attack
   * Northern Outpost
 * increased 'drakefoot' and 'drakefly' defense on grassland, sand and forest
 * unit balancing:
   * Halbardier: increased melee damage
   * Pikeman: increased melee damage
 * new units:
   * Horse Lord
   * Mermaid Diviner
   * Mermaid Enchantress
   * Mermaid Initiate
   * Mermaid Priestess
   * Mermaid Siren
   * Merman Entangler
   * Merman Fighter
   * Merman Hoplite
   * Merman Hunter
   * Merman Javelineer
   * Merman Netcaster
   * Merman Spearman
   * Merman Triton
   * Merman Warrior
   * Mounted Captain
   * Mounted Commander
 * language fixes and polishing (english)
 * new translations:
   * slovenian
 * updated translations:
   * danish
   * dutch
   * french
   * german
   * greek
   * hungarian
   * italian
   * norwegian
   * polish
   * portuguese
   * russian
   * slovak
   * spanish
   * swedish
 * allowed translating female unit names differently than male ones for languages needing it
 * multiplayer improvements
   * winter, desert and marsh random map generators
   * map descriptions
   * new maps: Broken Bridge
   * experience requirement now uses 10% increments
   * updated lobby
 * added naming for coastal villages
 * re-enabled italics and underlining for texts
 * fixed road layering in terrain-graphics.cfg
 * fixed assumption that "no advance" == 500 XP required in help code
 * fixed bugs in 'The Eastern Invasion: Weldyn Under Attack' scenario
 * fixed ice layering in terrain-graphics.cfg
 * fixed recruit/recall bug multiplayer
 * fixed gold not being reset in multiplayer
 * fixed wmlxgettext for header generation
 * fixed showing incorrect teams when loading a saved multiplayer game
 * fixed compile error on 64bit systems
 * fixed 'Drake Slashers' first strike
 * fixed AI not being aware that units in villages can not be plagued
 * fixed some missing unit descriptions
 * fixed lots of untranslatable strings
 * fixed female units lacking gender attribute
 * fixed wmlxgettext bugs
 * fixed window not always refreshing on resizing
 * fixed network threading bugs
 * fixed bug in vgettext
 * fixed assertion failure in playturn.cpp
 * fixed long-standing strings in editor
 * added specifying the maximum length of the text widget in show_dialog
 * added displaying help strings in smaller font if they don't fit in the screen
 * 320x240 resolution with USE_TINY_GUI
 * added an ellipsis on the button text when it does not fit on the button
 * moved menu strings into wesnoth-lib textdomain
 * moved tips from english.cfg to tips.cfg
 * per-language cache files
 * forced config reload upon language change
 * switched networking code to use the new logging system
 * switched image.cpp to use the new logging system
 * improved network threading
 * added server facility to record the way games end
 * improved 'Out of Sync' detection
 * imported SDL_ttf into the Wesnoth source tree, removed the dependency
   to SDL_ttf, added a dependency to libfreetype2
 * fixed memory corruption errors on font rendering in SDL_ttf
 * image and data-file cleanups
 * code cleanups

## Version 0.8.6
 * user interface tweaks
 * graphics improvements:
   * leadership frames
   * attack frames
   * grassland-to-water transitions
   * pier, bridge-ends
 * increased experience required to advance for Haldric and Jessica
   (both from The Rise of Wesnoth campaign)
 * reduced 'Elvish High Lord' movement to 6
 * tweaks for 'undeadfly' movetype: neutralized blade, pierce and impact resistances
 * tweaks for 'spirit' movetype: increased resistance against blade, pierce and impact
 * 'undeadspirit' movetype (copy of 'spirit' movetype with vulnerability to 'holy')
 * unit balancing:
   * Blood Bat: reduced hitpoints
   * Ghoul: reduced movement, reduced hitpoints
   * Ghost: switched movetype to 'undeadspirit', increased hitpoints
   * Necrophage: reduced movement, reduced hitpoints
   * Nightgaunt: switched movetype to 'undeadspirit', increased hitpoints
   * Shadow: switched movetype to 'undeadspirit', increased hitpoints
   * Spectre: switched movetype to 'undeadspirit', increased hitpoints
   * Wraith: switched movetype to 'undeadspirit', increased hitpoints
   * Vampire Bat: reduced hitpoints
 * new units:
   * Grand Marshal
 * language fixes and polishing (english)
 * updated translations:
   * brazilian
   * catalan
   * danish
   * greek
   * hungarian
   * italian
   * portuguese
   * spanish
   * swedish
 * multiplayer improvements
   * new lobby
   * leader indicator
   * random map generator improvements
   * scrollbar for map selection
   * threading for multiplayer server to get rid of freezes
 * fixed schedules textdomain
 * fixed villages being over tree tops
 * fixed 'Direwolf Rider' incorrectly having 'smallfoot' movetype
 * fixed 'Elvish Shaman' healing halo
 * fixed showing faction leader incorrectly in multiplayer games when loading from savegame
 * fixed long names in lobby going over the reserved area (#10414)
 * fixed 'Duelist' white border problem
 * fixed untranslated female elvish marksman (#10599)
 * fixed hex detection
 * fixed multiplayer game aborting when player joined with "random" faction (#10604)
 * fixed savegames for beginning of tutorial not loading
 * fixed missing fire_event in  'Son of the Black Eye: Saving Inarix'
 * fixed Wesbowl
 * fixed compiling errors in the zip-directory (#10189, #10362)
 * fixed lag when chatting in multiplayer lobby
 * fixed --with-kde and --with-gnome
 * fixed 'About' translations being duplicated for no real good reason
 * fixed untranslatable strings in help
 * fixed some abilities not being translatable
 * fixed holywater bug in 'The Rise of Wesnoth: Cursed Isle'
 * fixed top toolbar leaking ownership information for villages under fog of war
 * fixed victory conditions for 'Son of the Black-Eye: Silent Forest'
 * fixed victory conditions for 'The Rise of Wesnoth: Sewer of Southbay'
   and 'The Rise of Wesnoth: Fallen Lich Point'
 * support for bridges ending on water
 * switched using gettext-standard multi-textdomain
 * suppressed warnings from msgfmt
 * po-header cleanups
 * added support for desktopdir and icondir customization for --with-kde and --with-gnome
 * logging system improvements
 * campaign image-files hierarchy cleanup
 * code cleanups

## Version 0.8.5
 * campaign server (campaignd) and capability to download campaigns from campaign server(s)
 * sorting of campaigns by rank
 * campaign descriptions (text + images) in the campaign selection dialog
 * user interface tweaks
 * :refresh command
 * graphics improvements:
   * forests
   * attack icons
   * unit images and animation frames
   * teleportation animations
 * tutorial updates
 * scenario balancing for 'Heir to the Throne' campaign
   * Return to Wesnoth
 * scenario balancing for 'The Rise of Wesnoth' campaign
   * pretty much all scenarios
 * scenario balancing for 'The Eastern Invasion' campaign
   * An Unexpected Appearance
   * Weldyn under Attack
   * Tribal Warfare
 * drakefoot movetype
 * remade Drakes and removed obsolete ones
 * added hammer attack to Dwarvish Lord
 * new units:
   * Drake Flare
   * Drake Gladiator
   * Drake Glider
   * Elvish Enchantress
   * Elvish Sorceress
   * Elvish Sylph
   * Goblin Impaler
   * Goblin Spearman
   * Inferno Drake
   * Saurian Icecaster
   * Saurian Soothsayer
   * Saurian Tribalist
   * Sky Drake
   * Troll Rocklobber
 * unit description updates
 * language fixes and polishing (english)
 * added FreeSans font
 * updated translations:
   * basque
   * catalan
   * czech
   * danish
   * dutch
   * finnish
   * french
   * german
   * greek
   * italian
   * norwegian
   * polish
   * portuguese
   * russian
   * slovak
   * spanish
   * swedish
 * AI improvements
   * less inclined to attack after moving
   * more inclined to make risk-free attacks
 * multiplayer improvements
   * added increments to sliders
   * allowed choosing your leader
   * new maps: Blitz, Castle Hopping Isle, An Island
   * gave Goblin Spearman to northeners and removed Saurian from them
   * gave Saurian to Drakes on 'Age of Heroes' and 'default' eras
   * added Saurians to 'Classic' era
 * added new AI parameter: attack_depth
 * made units only die if they have 0 or less hitpoints after the event has been fired
 * tweaked Drake recruitment patterns (AI)
 * fixed 'make clean' not cleaning everything
 * fixed 'make uninstall' not working because of syntax error
 * fixed 'system default language' selection
 * fixed several graphic glitches
 * fixed a bug where tiles from terrain_graphics rule did not have
   the per-rule image when only defined through [map]
 * fixed bug with [not] tags in filters
 * fixed missing semicolon in sound.cpp
 * fixed tutorial talking of elves having 70% defense on grass
 * fixed many untranslatable strings in e.g. statistics, statusbar tooltips, units
 * fixed missing movetype for 'Pirate Galleon' and 'Transportation Galleon'
 * fixed General crossbow attack graphics glitch
 * fixed "wait for start" lobby dialog
 * fixed units not submerged when moving/fighting
 * fixed bug where [modify_side] couldn't change a side's team name properly
 * fixed victory and defeat conditions on 'The Eastern Invasion: Approaching Weldyn' scenario
 * fixed fixed illegal character 'x' in 'The Eastern Invasion: Weldyn Under Attack' scenario
 * fixed defeat conditions on 'The Eastern Invasion: Escape Tunnel' scenario
 * fixed knight appearances bug on 'The Eastern Invasion: Mal-Ravalans Capital' scenario
 * fixed double '{BIGMAP_BLACKWATER_PORT}' in 'Heir to the Throne: Blackwater Port' scenario
 * fixed gold and load game problem
 * fixed divide-by-0 error in AI
 * fixed statistics dialog seemingly endless loop
 * fixed firing events from a kill event possibly causing crash
 * fixed AI allies updating shroud and fog at the end of their turn
 * fixed bug in 'show enemy moves'
 * fixed pressing 'shift' while scrolling to your leader would halt scrolling
 * fixed many deprecated units in scenarios
 * fixed statistics not being reset between multiplayer games
 * fixed forest<->snow forest transitions
 * fixed crash in editor
 * fixed bug in replay with disbanding units
 * fixed playing multiplayer and then playing a campaign could not load the multiplayer game up again
 * fixed 'Messanger 0f Doom' in 'The Eastern Invasion: Weldyn under Attack' scenario
 * restored "Ambushed!" text in 'Heir to the Throne: Crossroads' scenario
 * added a way to specify the actual position of multi-hex tiles
 * added option to preferences file, 'unit_genders', which determines
   whether units with different genders should be used
 * added internationalization parameter for Mac OS X
 * automatic computation of font sizes relatively to SIZE_NORMAL;
   make TINY mode really tiny to better help with UI downsizing work
 * logging system improvements
 * made it possible to have [terrain_graphics] rule as childs of [scenario]s
 * enabled gamestart speedup by not unnecessarily loading campaigns
 * support for teleportation animations
 * preparations for removing obsolete units
 * translation cleanups and improvements
 * code cleanups

## Version 0.8.4
 * changed vision range based on potential move
 * changed charge, backstab and steadfast abilities use true doubling
   and halving rather than additive percentage calculations
 * more sound effects
 * new and improved graphics:
   * snow forest tiles
   * unit images
   * tower
   * units images and animations
   * attack icons
 * new scenarios for 'Heir to the Throne' campaign:
   * Battle for Wesnoth (final scenario)
 * scenario balancing for 'Heir to the Throne' campaign:
   * Crossroads
   * Test of the Clans
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Silent Forest
 * plot changes in 'The Eastern Invasion' campaign
 * scenario balancing for 'The Eastern Invasion' campaign:
   * Approaching Weldyn
   * Lake Vrug
   * Northern Outpost
   * The Duel
   * The Outpost
   * Two Paths
 * epilogue for 'The Rise of Wesnoth' campaign
 * new scenarios for 'The Rise of Wesnoth' campaign:
   * Rise of Wesnoth (final scenario)
 * scenario balancing for 'The Rise of Wesnoth' campaign:
   * A Final Spring
   * A Harrowing Escape
   * The Vanguard
   * Troll Hole
 * spirit movetype, with high resistances against "physical" attacks, slowed on open water
 * dwarvishfoot movetype
 * unit balancing:
   * dwarves: switched movetype to 'dwarvishfoot'
   * treefolk (Woses): reduced pierce, impact and cold resistances
   * undeadfoot: "deep walking" (movement in deep water)
   * Dwarvish Guardsman: reduced blade, pierce, impact, fire and cold resistances,
     reduced cost, reduced number of ranged attacks
   * Dwarvish Stalwart: reduced blade, pierce, impact, fire and cold resistances,
     reduced number of ranged attack, increased ranged damage
   * Ghost: switched movetype to 'spirit', halved hitpoints, reduced movement
   * Lich: increased cost
   * Necromancer: increased cost
   * Nightgaunt: switched movetype to 'spirit', halved hitpoints,
     reduced movement, reduced number of attacks, backstab
   * Orcish Archer: reduced cost
   * Peasant: increased cost, increased melee damage
   * Spectre: switched movetype to 'spirit', halved hitpoints,
     reduced movement, backstab
   * Shadow: switched movetype to 'spirit', halved hitpoints,
     reduced movement, reduced number of attacks, backstab
   * Vampire Bat: increased cost
   * Walking Corpse: increased hitpoints, increased melee damage
   * Wose: reduced melee damage
   * Wraith: switched movetype to 'spirit', halved hitpoints, reduced movement
 * new units
   * Dark Queen
   * Dwarvish Sentinel
 * language fixes and polishing (english)
 * updated translations:
   * basque
   * czech
   * danish
   * finnish
   * french
   * german
   * hungarian
   * italian
   * norwegian
   * spanish
   * swedish
 * status for translations (gettext) - http://gettext.wesnoth.org
 * splitted po-files into domains (gettext)
 * multiplayer improvements:
   * restored Thug to Knalgan Alliance on default era
   * added many 2nd level units to factions on 'Age of Heroes' era
 * made hosting games work elegantly when wesnothd is running on the same machine
 * added new @ prefix to {inclusion} in config files to make it search both
   in the user's preferences dir and in the main data dir
 * fixed typo causing 'Troll Whelp' being recruitable in 'The Dark Hordes: A New Chance' scenario
 * fixed alignment and abilities not being shown in panel
 * fixed hitpoints and experience being not translatable in default theme
 * fixed escape not working in game lobby
 * fixed the tools in src/tools not building
 * fixed 'no start position for side 1' in 'The Eastern Invasion: Drowned Plains'
 * fixed unselected units being too dark
 * fixed missing tag in 'The Eastern Invasion: The Duel'
 * fixed saving with illegal characters in the filename
 * fixed cycling units with 'n' showing invisible enemy units while it is their turn
 * fixed units not disappearing properly when they die
 * fixed recall working incorrectly
 * fixed missing Vera.ttf msgid from pot-file
 * fixed the problem with parse errors occuring in the units descriptions
 * fixed empty terrain names occuring in the help system
 * fixed Delfador appearing in 'The Dark Hordes: Inside The Tower'
 * fixed about/credits not being translatable
 * fixed 'Orcish Ruler' appearing in 'The Rise of Wesnoth' campaign
 * fixed 'Heavy Infantryman' recruiting bug in 'The Eastern Invasion:
   Mal-Ravanals Capital' scenario
 * fixed editor not being translatable
 * fixed Parandra missing in 'Heir to the Throne: The Elven Council' scenario
 * fixed king going to swimming on 'The Rise of Wesnoth: A Final Spring' scenario
 * fixed past-the-end issue on botched UTF-8 strings
 * fixed scroll_to_unit issues on 'The Rise of Wesnoth' campaign
 * fixed build procedure to not require having gettext installed
 * fixed scanning enemy unit locations even when they are under the fog
 * workaround for Reiser4 feature bug (#10264)
 * workaround for BeOS gettext implementation
 * workaround for Mac OS X gettext implementation
 * re-implemented enhanced sound quality fix
 * added error dialog when a campaign has corrupt WML
 * added better error messages for machines that cannot bind to local ports
 * switched relevant flags from --enable-* to --with-* in autotools
 * autotools re-organizations for better BSD support
 * editor dependency cleanups
 * help system optimizations
 * village placement optimizations
 * compile speed optimizations
 * text rendering optimizations
 * squashed many compile warnings
 * code cleanups and refactoring

## Version 0.8.3
 * information about encountered terrain added in the help system
 * removed the unit description dialog in favor of in-game help
 * user interface tweaks
 * added damage taken and inflicted with expected values to statistics
 * new and improved graphics:
   * dwarvish castle
   * swamp, deep water, shallow water, snow, cave wall, cave floor,
     ice, lava, dirt, forest, bridge, road
   * chest, nest
   * portraits and unit animations
   * preferences icons
 * updates and enhancements to tutorial
 * scenario balancing for 'The Dark Hordes' campaign:
   * The Skull of Agarash
   * Underground Pool
 * new scenarios for 'The Eastern Invasion' campaign:
   * The Drowned Plains (replaces 'Peasant Revolt')
 * scenario balancing for 'The Eastern Invasion' campaign:
   * Captured
   * Evacuation
   * Weldyn Under Attack
 * new scenarios for 'The Rise of Wesnoth' campaign:
   * The Vanguard
   * Return of the Fleet
 * scenario balancing for 'The Rise of Wesnoth' campaign:
   * Temple in the Deep
 * replaced Duelist's pistol with crossbow
 * more female units
 * unit balancing:
   * Bone Shooter: reduced experience required to level
   * Dwarvish Fighter: gave hammer attack
   * Dwarvish Steelclad: gave hammer attack, increased experience required to level
   * Elvish Lord: changed to 2nd level unit, reduced hitpoints, reduced cost,
     reduced melee and ranged damages, advances to Elvish High Lord
   * Soul Shooter: increased ranged damage
   * Woses: reduced cold resistance
   * mountainfoot (Dwarves, Giant Spider): reduced movement cost on forest,
     sand, shallow water and swamp
 * new units
   * Dwarvish Guardsman
   * Dwarvish Stalwart
   * Elvish High Lord
 * language fixes and polishing (english)
 * switched to using gettext, following languages have been converted:
   * catalan
   * french
   * norwegian
   * spanish
 * multiplayer improvements:
   * turn_cmd preferences option which can be pointed to a command executed
     at the start of the player's turn
   * replace AI or local player with observer
   * allowed recruiting Ghoul
 * Great War (multiplayer era) changes:
   * White Mage as leader of Alliance of Light
   * Necromancer as leader of Alliance of Darkness
   * removed Wose, Gryphon Rider and Dwarvish Thunderer from Alliance of Light
 * added 'steadfast' ability
 * added automatic path search to [move_unit_fake]
 * fixed Windows compile errors
 * fixed Mac OS X compile error
 * fixed syntax error appearing in the help about units under some translations
 * fixed help about units/abilities/weapon specials not being sorted correctly
 * fixed number of turns in 'The Dark Hordes: A New Chance'
 * fixed editor where nothing was displayed until terrains were rebuilt
 * fixed Elvish Shaman missile image
 * fixed AI sometimes not respecting 'turns' parameter properly
 * fixed bad recruitment in 'The Dark Hordes: A New Chance'
 * fixed Kalenz appearing in 'The Dark Hordes: Underground Pool'
 * fixed call to non-existing macro in 'Heir to the Throne: Valley of Death'
 * fixed lacking ids in the female part of units files (#10065)
 * fixed conflict in terrain-graphics.cfg
 * fixed multi-hex images loading incorrectly when first loaded
   under an different zoom level than the standard one
 * fixed map labels appearing during story
 * fixed missing #ifdef around config.h inclusion
 * fixed poisoned units not looking poisoned when selected (#10094)
 * fixed unit with 'non-living=yes' attacking a unit with a plague attack,
   and the unit with the plague attack kills it, a new plague unit will be created (#10049)
 * fixed displaying 'hit t to continue' even when unit has finished movement
 * fixed shroud being revealed at the beginning of movement while sometimes
   it was revelead at the end of movement (#10046)
 * fixed bad ifdefs in 'The Dark Hordes: Inside the Tower' scenario
 * fixed bugs in menu scrolling
 * fixed {RANDOM} bug in 'The Rise of Wesnoth' scenarios
 * fixed floating labels sometimes not displaying on accelerated mode
 * fixed "drain" not working on final blow (#10038)
 * fixed bad victory conditions in 'The Rise of Wesnoth: The Dragon' scenario
 * fixed some units shadows not being semi-transparent
 * fixed crash bug that could occur after undoing
 * fixed a little bug where a space could disappear near EOL in the help system
 * changes in #ifdefs to prevent the game from using the X11 clipboard on MacOSX
 * reverted sound quality improvement as it caused instability on some systems
 * help system shows everything when wesnoth is running in debug mode
 * added support for 2- and 3-sided transitions on the TERRAIN_ADJACENT_NORTH macro
 * added possiblity for terrains to be defined as 2-layered,
   with a base part and an overlay part
 * added possibility to flip overlayed image
 * optimized menu scrolling
 * other minor speed optimizations
 * multiplayer file hierarchy cleanup
 * new file hierarchy for portraits
 * changed building static binary to use libtool when available
 * applied BeOS clipboard support patch
 * applied BeOS native settings path support patch

## Version 0.8.2
 * new and improved graphics:
   * canyons
   * signpost
   * grassland
 * improved sound quality
 * improved in-game help
 * new "vi" commands: clear, w, wq, q, debug, n
 * new tutorial
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Black Flag
   * Silent Forest
 * storyline fixes to 'The Eastern Invasion' campaign
 * scenario balancing for 'The Eastern Invasion' campaign:
   * Weldyn Under Attack
   * The Crossing
   * Mal-Ravanals Capital
   * The Arena
 * scenario balancing for 'The Rise of Wesnoth' campaign:
   * Temple of the Deep
   * Lizard Beach
   * Sewer of Southbay
   * A Harrowing Escape
 * unit balancing:
   * Bandit: increased melee damage
   * Dark Adept: increased ranged damage, reduced cost
   * Drake Slave: increased melee damage
   * Drake Worker: increased melee damage
   * Noble Commander: increased hitpoints
   * Noble Fighter: increased hitpoints and melee damage
   * Noble Lord: reduced hitpoints, reduced melee and ranged damage
   * Noble Youth: increased hitpoints, increased movement, increased melee damage
   * Outlaw: reduced hitpoints, reduced number of melee attacks, increased ranged damage
   * Outlaw Princess: reduced hitpoints, reduced experience required to level,
     reduced number of melee attacks, increased ranged damage, replaced 'ambush' with 'skirmisher'
   * Outlaw Queen: increased hitpoints, replaced 'ambush' with 'skirmisher',
     reduced number of melee attacks, increased ranged damage and number of ranged attacks
   * Soulless: increased melee damage
   * Thug: increased melee damage and reduced number of attacks
   * Walking Corpse: increased melee damage
 * new units
   * Drake Hatchling
   * Drake Warmage
   * Great Troll
   * Skeletal Dragon
   * Troll Hero
   * Orcish Leader
   * Orcish Sovereign
 * multiplayer improvements:
   * 'Great War' era
   * limit to username length
   * banning for multiplayer server games
 * language fixes and polishing (english)
 * support for multi-animation attacks
 * support for per-scenario configurable village flags,
   sample black-eyed banner for Son of the Black Eye campaign
 * support for relative {./includes} in WML files
 * added recruit event
 * fixed enemies being on wrong side in 'The Dark Hordes: A New Chance' scenario
 * fixed image::locator not building on MSVC6
 * fixed sending incorrect packet length causing wesnoth and wesnothd to abort
 * fixed a bug where a network user may crash other game abusing recursion
 * fixed a bug where a network user may crash other games using specially
   crafted packets, reading out-of-bounds data
 * fixed segfault when a [story] background image was not found
 * fixed narrator messages not being displayed sometimes
 * fixed autoconf macro for checking OGG support in SDL_mixer
   (it depended on private symbols not always included by distributions)
 * fixed markovian name generation not playing well with UTF-8 names
 * fixed map border not getting changed when using [terrain] to change terrain (#9840)
 * fixed extraneous dots path (#9903)
 * fixed shift due to the way coordinates are now interpreted
 * fixed seeing invisible units when they move (#9488)
 * fixed bug with exiting caves in 'The Eastern Invasion: Captured' scenario
 * fixed bugs in 'The Eastern Invasion: Evacuation' scenario
 * fixed holy water in 'Heir to the Throne: Valley of Death' scenario (#9930)
 * fixed problems with cache invalidation
 * fixed file descriptor leak
 * fixed 'gender' not working with move_unit_fake
 * fixed grÃ¼Ã¼ losing experience when joining
 * fixed units moving through water not being submerged while moving
 * terrains tiles are now cut according to the hexagonal shape, to avoid some graphical glitches
 * workaround a SDL_TTF bug that makes it crash when presented an invalid UTF-8 string
 * limited length of schema items in compressed WML to make a DoS attack harder to perform
 * dummy config.h file for platforms where ./configure is not used
 * file hierarchy cleanups

## Version 0.8.1
 * new campaign: The Rise of Wesnoth (17 fully playable scenarios)
 * user interface improvements:
   * new ellipses to indicate unit selection
   * tweaked theme
   * tweaked change video mode dialog
   * return can be used to navigate through help topics and sections in help browser
   * a box is drawn around images in help browser
 * speed scaling for different types of terrains
 * new and improved graphics:
   * lighthouse
   * signpost
   * maps
   * flags
   * mountains
   * grassland
   * desert
   * castle
   * shroud
   * fog of war
   * attack icons and frames
 * new musics: main menu
 * intro for 'The Dark Hordes'
 * new scenarios for 'The Dark Hordes' campaign:
   * Underground Pool
 * new scenarios for 'The Eastern Invasion' campaign:
   * Evacuation
   * Peasant Revolt
   * Approaching Weldyn
   * The Council
   * Weldyn Besieged
   * The Duel
 * scenario balancing for 'The Eastern Invasion' campaign:
   * Captured
   * The Crossing
   * An Elven Alliance
   * The Escape Tunnel
   * Northern Post
   * Training the Ogres
   * Two Paths
   * The Undead Border Patrol
   * Undead Crossing
   * An Unexpected Appearance
   * Weldyn Under Attack
   * Lake Vrug
   * Tribal Warfare
   * The Outpost
 * gave names to Trolls
 * gender support for units
 * increased hitpoints and required XP for Drake Warrior
 * new units
   * Drake Flameheart
   * Fire Dragon
   * Giant Mudcrawler
   * Haldric
   * Mudcrawler
   * Tentacle
   * Female Outlaw
   * Outlaw Princess
   * Outlaw Queen
   * Peasant
   * Soul Shooter
   * Wall Guard
   * Warrior King
 * multiplayer improvements:
   * implemented feature to tell observers when a new player has arrived in the lobby
   * disabled observers from setting labels
   * playername checks, stripping spaces at the end of name
   * limited maximum length of observer messages
   * added server commands: msg, kick, ban, unban, status, metrics
 * language fixes and polishing (english)
 * updated sample translation
 * updated translations:
   * catalan
   * czech
   * french
   * german
   * slovak
   * spanish
   * swedish
 * made it so variables can be used to set the number of turns in a game
 * changed set_flag rather verbose syntax to set_flag=blah,foo,bar
 * added [then] and [else] tags under [object] tags
 * obsoleted [bigmap] tag, in favor of a unified [story] syntax
   * display of background image, text, and title
   * support of [if] [then] [else] functionality (can be nested)
   * support of images overlay
 * switched all scenarios to new [story] syntax
 * added exploder and cutter tools
 * fixed the bug with multiplying help sections
 * fixed the problem where the forward and back button was shown briefly
   when opening the help browser
 * fixed --disable-debug not working as expected
 * fixed victory condition bug in 'Saving Inarix'
 * fixed problem where the editor would go slower and slower the more it was used
 * fixed a potential problem with a help browser recursive introduction section
 * fixed incorrect #ifdefs in 'Training the Ogres' and 'Weldyn Under Attack'
 * fixed some lacking death declarations
 * fixed bug where tooltips could be left behind after 'load game' dialog
 * fixed bug where themes with multiple resolution specifiers could be displayed incorrectly
 * fixed missing [removeitem] for Delfador's staff
 * fixed bug where multiplayer games that disallowed observers didn't work properly
 * fixed items not removed after being picked up
 * fixed victory conditions in 'The Siege of Barag Gor'
 * fixed attack animation not being complete
 * fixed absence of animation when units without attack frame perform attack
 * fixed obsolete Heavy Infantry unit still used in multiplayer
 * fixed colour cursor not reappearing on some window-managers
   when the user re-enters a Wesnoth window
 * fixed duplicate units: Cavalry, Cavalryman, Scout
 * fixed 'layers' not working as intended in built terrains
 * fixed terrain glitches, added a default base-terrain
 * fixed graphic glitches
 * fixed macro errors
 * fixed edges of the map not getting cleared of shroud and fog of war
 * fixed unit description on sidebar not getting updated when cycling units with 'n' (#8798)
 * fixed segfault in display.hpp
 * fixed dismissing unit from recall list not refreshing preview
 * fixed show_enemy_moves showing moves for petrified units (#9290)
 * fixed [not] tag not working properly
 * fixed bug where clicking a menu item could generate multiple events
 * fixed duplicate battleworld.cfg
 * fixed bug in WML with using value instead of equals in [if] (#9398)
 * fixed visual glitches caused by copy-paste
 * fixed bug in intro which segfaulted when no "file" attribute in [image] tag
 * small fixes to unit display in help browser
 * pngcrushed images
 * gettext implementation preparations
 * moved deaths.cfg from data/ to data/scenarios/Heir_To_The_Throne/
 * moved items to their own hierarchy: images/items/
 * adding a limit to number of characters in a textbox
 * adding a maximal line width for text to be drawn, to circumvent a SDL / SDL_ttf bug
 * implemented relative directory support
 * cleaned a bit the textbox code
 * refactoring of all animation code in unit_display and halo
 * added animated.hpp, and the animated<T> template
 * added proper support for animated time-of-day alternative images
 * added support for animated terrain images
 * added support for animated flags on villages
 * cleaned the terrain-graphics.cfg - terrain graphics rule precedence
   is now deprecated, use terrain layers
 * reverted castle tiles not being expanded to the border
 * replacing all SDL_Surface*, and scoped_sdl_surface,
   with shared_sdl_surface, which was renamed surface
 * partial rewrite of image.?pp for speed: looking up images was what was slowing the game
 * inlining some stuff for performance reasons
 * optimizing halo movement when unit moves
 * optimizing invalide_all redraws
 * concentrate all version and server changes for release-time on top of configure.ac
 * improved making binary packages
 * removed an obsolete feature in terrain graphics engine that was never used
 * removing commented-out obsolete code in display.cpp
 * code cleanups

## Version 0.8
 * added Drake flying animations
 * reduced experience needed to advance for Drake Burner and Drake Fighter
 * increased experience needed to advance for Drake Clasher
 * new units
   * Drake Slasher
 * new translations:
   * czech
 * multiplayer improvements:
   * balanced 'Battle for Weslin Bridge' map
   * added Drakes to 'Age of Heroes' era
 * updated translations:
   * french
   * hungarian
   * italian
   * slovak
 * fixed graphic glitches on forest and cave tiles
 * fixed some name typos in Tutorial
 * fixed editor theme

## Version 0.7.11
 * various help browser improvements
 * user interface improvements
 * improved clipboard functions and support for X11
 * scenario balancing for 'Heir to the Throne' campaign:
   * The Scepter of Fire
   * A Choice Must Be Made
   * The Elven Council
   * Home of the North Elves
   * Swamp of Dread
   * The Valley of Statues
 * multiplayer improvements:
   * chat log
   * allowed spaces in usernames
   * new map: Battle for Weslin Bridge
   * new map: Forest of Fear
   * new map: Castles
 * translation updates:
   * hungarian
 * made The Eastern Invasion campaign translatable
 * added icons for preferences and multiplayer dialogs
 * fixed crash when loading an empty map
 * fixed transition problems with forests
 * fixed wesnothd crashes
 * fixed underlining bug
 * fixed bugs where some objects were not working properly
 * fixed human village not fitting within the mask
 * fixed clashing msg ids
 * fixed isolated keeps having void graphics
 * fixed tile overlays not displaying in grey if the tile is out of range
   of the selected unit's movement
 * fixed bugs with roles not working properly
 * fixed coherence of usage for elves:
   * Archer, Marksman, Sharpshooter -> archer
   * Ranger, Avenger -> mixed fighter
   * Hero, Captain, Champion, Marshal -> fighter
 * non-editable multi-line textboxes can have areas of text highlighted
   (and thus copied and pasted)
 * support for defensive animations
 * tweaks to terrain graphic ordering
 * added engine support for max-level units advancing further
 * code refactoring

## Version 0.7.10
 * help browser, hotkey is <F1>
 * new tabbed preferences dialog
 * basic clipboard functionality, does not yet work on all platforms
 * improved and added new graphics: unit animations, healing animations, tent, forest
 * traits removed from treefolk (Woses)
 * treefolk piercing, impact and cold resistance increased
 * Wose renamed to Elder Wose, Sapling Wose renamed to Wose
 * Wose hitpoints and melee damage increased
 * reduced cost for Drake Burner, Drake Fighter, Drake Mage and Drake Slave
 * new units
   * Drake Beak
   * Drake Worker
   * Dwarvish Ulfserker
   * Fire Drake
   * Gryphon Master
   * Lancer
 * scenario balancing for 'Heir to the Throne' campaign:
   * Dwarven Doors
   * Mountain Pass
   * Plunging Into the Darkness
 * new scenario for 'The Dark Hordes' campaign:
   * Confrontation
 * scenario balancing for 'The Dark Hordes' campaign:
   * Brother Against Brother
   * Crelanus Book
   * Inside the Tower
   * Mages and Elves
   * The Skull of Agarash
 * new scenario for 'Son of the Black Eye' campaign:
   * Clash of Armies
 * multiplayer improvements
   * added Drakes to Wesbowl
   * added Drakes to 'Classic' era
 * AI improvements
   * made AI attack when getting ambushed
 * language fixes and polishing (english)
 * translation updates:
   * catalan
   * dutch
   * german
   * italian
   * spanish
 * editor improvements:
   * added some command line flags, such as --datadir
 * enabled alternate castle/keep tiles: encampment
 * gate "unit" (stripped from The Eastern Invasion/Captured scenario)
 * [binary_path] WML element which allows specification of a path
   where binary files may appear
 * made WML {inclusion} non-recursive
 * fixed segfault when filtering units at invalid locations
 * fixed SDL version checking bug in Mac OS X
 * fixed color selection in multiplayer not working
 * fixed bug where underlying widgets could 'show through' the load dialog
 * fixed load dialog eating lots of processing power
 * fixed assertion failure/infinite loop in AI logic
 * fixed bug in WML
 * fixed segfault
 * fixed bug in generating random maps
 * fixed wesnothd crashing due SIGPIPE
 * disabled some font settings to work around bug in SDL_ttf
 * added CASTLE_AND_SIMPLE_KEEP macro, where the keep is composed of only a unique tile with walls
 * added BUILDING_ macros to add houses, tents etc. over other terrains
 * added CASTLE_FLOOR macro
 * transitions may be now specified between a terrain and another one (in terrain-graphics.cfg)
 * enabled castle-to-forest transitions for encampments
 * relocated 'Heir to the Throne' maps in they own subdir, and gave them an explicit names
 * relocated 'Son of the Black Eye' maps in they own subdir, and gave them an explicit names
 * relocated 'The Dark Hordes' maps in they own subdir, and gave them an explicit names
 * relocated multiplayer maps in they own subdir, and gave them an explicit names
 * relocated tutorial maps in they own subdir, and gave them an explicit names
 * added dir for user created campaigns
 * added basic statistics output to wesnothd
 * changed HttT maps to use encampments
 * removed unused maps
 * libpng checks for configure
 * changed the RGB to Grayscale function to use a more correct formula

## Version 0.7.9
 * new campaign: The Eastern Invasion (Loyalists, 15 scenarios)
 * compiling requires SDL version >= 1.2.7
 * user interface improvements:
   * tip of the day
   * added new help strings
   * new attack dialog
   * new load game dialog
   * made 'end turn for active unit' (pressing <space>) recoverable
   * in game load game: menu -> load game
   * gamma correction
   * skip AI moves option
   * show haloing effects option
   * resized some dialogs
 * improved and added new graphics: units, attacks animations,
   healing animations, attack icons, encampment, castle, haloes
 * scenario balancing for 'Heir to the Throne' campaign:
   * Elves Besieged
   * Isle of Anduin
   * The Siege of Elensefar
   * Crossroads
   * Valley of Death
   * Ford of Abezz
   * Dwarven Doors
   * Northern Winter
   * Valley of Statues
 * new units
   * Cavalier
   * Drake Burner
   * Drake Clasher
   * Drake Fighter
   * Drake Guard
   * Drake Mage
   * Drake Petit
   * Drake Slave
   * Drake Warrior
   * Dwarvish Dragonguard
   * Dwarvish Runemaster
   * Sergeant
   * Silver Mage
   * Vampire Lady
   * Young Ogre
 * removed Dark Apprentice, Evil Necromancer and Soldier as they are no longer used
   (replaced by Initiate, Deathmaster and Spearman)
 * Yeti's race changed to monster
 * nenamed 'Cavalry' to 'Cavalryman'
 * renamed 'Goblin Direwolver' to 'Direwolf Rider'
 * renamed 'Heavy Infantry' to 'Heavy Infantryman'
 * renamed 'Orcish Crossbow' to 'Orcish Crossbowman'
 * unit balancing:
   * Ancient Wose: increased cost
   * Ancient Lich: reduced hitpoints, reduced ranged damage
   * Arch Mage: reduced hitpoints, reduced ranged damage, removed teleport ability
   * Battle Princess: reduced melee damage
   * Deathmaster: reduced ranged damage
   * Death Knight: reduced melee damage
   * Demilich: reduced ranged damage
   * Dwarvish Berserker: reduced cost
   * Dwarvish Lord: reduced melee damage
   * Dwarvish Steelclad: increased cost
   * Elder Mage: reduced hitpoints, increased cost
   * Elvish Avenger: reduced number of melee attacks
   * Elvish Champion: reduced melee damage
   * Elvish Lord: reduced melee damage
   * Elvish Marshal: reduced melee damage, reduced number of ranged attacks
   * Direwolf Rider: increased melee damage, reduced number of melee attacks
   * Great Mage: reduced melee damage, reduced ranged damage, removed teleport ability
   * Halbardier: reduced melee damages, gave 'first strike' ability
   * Heavy Infantry: reduced melee damage
   * Initiate: reduced ranged damage
   * Iron Mauler: reduced melee damage
   * Lord: reduced melee damage
   * Mage of Light: reduced ranged damage
   * Master Bowman: reduced ranged damage
   * Orcish Warlord: reduced melee damage
   * Pikeman: reduced melee damage, gave 'first strike' ability
   * Princess: reduced melee damage
   * Rogue: increased melee damage
   * Royal Guard: reduced melee damage
   * Shock Trooper: reduced melee damage
   * Spearman: reduced ranged damage, gave 'first strike' ability
   * Wose: increased cost
 * language fixes and polishing (english)
 * translation updates:
   * dutch
   * french
 * multiplayer improvements:
   * balanced 'King of the Hill' map
   * improved village naming
 * AI improvements
   * added new AI parameters
   * various AI improvements
 * editor improvements:
   * new brush image, file chooser folder image
   * changed menu names to 'file' and 'edit'
   * delete file button in file chooser
   * two terrains may now be selected in the palette, one is used for filling when
     moving selections and cutting/pasting and will be used when drawing with the right button
   * removed context menu, function of the left button is chosen in the right panel
     and operations that do not need a location to work are chosen from an additional menu
   * added a new level of brightened images
   * support for translations
   * fixed proper checks for validity not being done when loading or reverting a map
   * fixed mouse wheel and middle-click not working as within the game when used on map area
   * fixed BeOS crash in map editor
   * fixed the problem where the same sequence of random maps was generated
   * fixed a problem where the hotkeys from the game would appear in the editor
   * fixed a problem where the selected terrain and brush size was
     not remembered through operations that affected the whole map, such as flip and resize
   * fixed a problem where the palettes in the editor would not refresh
     correctly after the windows was resized
   * fixed editor handing different color depths incorrectly
 * new item: trapdoor
 * continue_no_save feature
 * added WML feature to append an element to another using [+element] syntax
 * made userdata/data/utils loaded in early in game.cfg to allow
   scenario designers to define utility files
 * names for Nagas
 * fixed multiline text box not wrapping lines at word boundaries
 * fixed loading/saving of multiplayer games that have a double-quote (") in the chat text
 * fixed spaces following non-ASCII characters disappearing in intro (#9163)
 * fixed clashing IDs in 'Blackwater Port'
 * fixed a bug where alpha disappeared from multi-hex .png files on loading sub-tiles
 * fixed terrain not updating correctly when dynamicly changed via the WML [terrain] event (#9237)
 * fixed the "random probability" in "terrain-graphics" rules not being random enough
 * fixed builder code failing when asked for hexed outside the map (#9168)
 * fixed crash when undoing moves
 * fixed incorrect positioning of haloes
 * fixed crash in wesnothd
 * made intro.cpp use the unicode routines in language.cpp to split UTF-8 sequences in character
 * added support for animated on-map items
 * added support for different colour depths
 * added a cut_surface function in sdl_utils, that extracts a surface "subset" from another surface
 * improved terrain transition code
 * updated terrain-graphics.cfg file to use modules in sub-directory to reduce bloat in the main file
 * terrain builder code optimizations
 * added macros so that keep graphics are now selected according to the adjacent castle tiles
 * added XCF source files and some tools for castle building, these can be found in tools/
 * moved weapon icons to their own hierarchy: images/attacks/
 * moved projectiles/missiles to their own hierarchy: images/projectiles/
 * increased the horizontal padding of button widgets
   when having a label that is wider than the minimum width
 * added a trigger to the test map, when activated Orc leader can recruit
 * decreased size of saved games by removing some unnecessary information
 * converted all scenarios to new AI syntax
 * removed unnecessary diagnostic

## Version 0.7.8
 * changed experience gauge, it is now shown next to hitpoints bar
 * search works with coordinates, use it like: /30,15
 * weapon type's tooltip shows its effectiveness against all known enemies on the battlefield
 * improved and added new character portraits, unit images, attack icons,
   missile images, fog and shroud images
 * halo effects
 * new sounds
 * enhanced blade, impact and cold resistance for Cavalry and Dragoon
 * scenario balancing for 'Heir to the Throne' campaign:
   * Crossroads
   * Valley of Death
 * scenario balancing for 'The Dark Hordes' campaign:
   * A New Chance
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Shan Taum the Smug
 * multiplayer improvements
   * added 'share maps' option
   * alt-m to chat public
   * shift-m to chat with allies
   * showing basic game settings
   * random map option to build road between castles
   * improved random map generator speed
   * naming of villages and terrain features on random maps
   * user written maps can be easily loaded as scenarios in multiplayer
   * added facility to play maps in multiplayer that don't have scenario data specified for them
   * lobby sound effects for new messages, players joining and departing lobby
   * in game creation player slots are now occupied by 'computer player' as default
   * empty player slots, allowing empty castles
 * editor improvements:
   * added flipping
   * starting positions now move as they should through copy/paste and click-and-drag
   * random map generator settings are now saved through generations
   * fixed labels on map being visible through certain dialogs
   * cleaned up editor theme
 * translation updates:
   * french
   * polish
   * spanish
 * AI improvements
   * improved grouping
   * made grouping behaviour configurable
   * added defensive grouping
 * fixed crash when generating a map with 0 villages
 * fixed map (shroud) and statistics sharing not working between allies
 * fixed AI getting stuck and never finish making its move
 * fixed units in swamp villages being submerged
 * fixed resizing and changing video modes not working  while in lobby
 * fixed hotkeys not working while in lobby
 * fixed saving being impossible when save games have accented characters on MacOSX
 * fixed segfault in random map generation
 * fixed Out of Sync bugs
 * fixed bug where pressing 'n' while a unit is attacking would transfer damage to another unit
 * fixed entering a chat message while the message is selected would keep
   the area selected, and cause a crash upon next keypress (#9114)
 * fixed chat messages being dropped
 * fixed text ellipsis (triple dot being added after too long text) not being UTF-8-aware
 * fixed invalid UTF-8 sequences being generated by wstring_to_utf8,
   causing display bugs with some accented characters on some platforms
 * fixed word_wrap_text not being UTF-8 aware
 * ignore unicode value of cursor-moving keys in text boxes (arrow keys, home, end);
   so they do not cause problems in platforms where SDL puts one (that is: MacOSX)
 * removed alpha keyword from unit data, changed image files to have it
 * added font::make_text_ellipsis function, that works like word_wrap_text,
   but instead adds ellipsis on too long text
 * changed portable_isspace() so it only returns true on ASCII spaces
 * including a directory in WML will recurse into subdirs
 * WML support for '~' notation to include files in ~/.wesnoth/data
 * support for different images for unit movement
 * weslang, to help translation updates
 * added file choose widget
 * small changes to package description in Slacknoth
 * removed unnecessary voluminous diagnostics
 * code refactoring

## Version 0.7.7
 * added 'search' action which goes through labels and unit names, hotkey is '/'
 * added 'continue move' action, hotkey is 't'
 * made game more responsive when AI or another player is playing
 * 'end of turn' sanity check
 * added experience gauge, meld with hit points gauge
 * support different resolutions for 'fullscreen' and 'windowed'
 * various UI cleanups
 * added and improved village images, hill images, unit images, healing images
 * resized ellipses and made them float on top of water when unit is partially submerged
 * General melee damage increased
 * Elvish Outrider made 3rd level unit, increased hitpoins and ranged attack damage
 * Goblin Pillager melee damage increased
 * renamed Lizardman to Saurian
 * regeneration replaced with skirmish for Saurians
 * new units:
   * Elvish Rider
   * Saurian Warrior
   * Necrophage
 * new scenario for 'Son of the Black Eye' campaign:
   * Saving Inarix
 * new scenario for 'The Dark Hordes' campaign:
   * A New Chance
 * scenario balancing for 'Heir to the Throne' campaign:
   * Bay of Pearls
   * Blackwater Port
   * Isle of Anduin
   * Isle of the Damned
   * Muff Malal's Peninsula
   * Princess of Wesnoth
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Towards Mountains of Haag
   * To The Harbour of Tirigaz
   * Black Flag
   * Desert of Death
   * End of Peace
   * Silent Forest
   * The Siege of Barag Gor
 * translation updates:
   * french
   * spanish
 * AI improvements
   * fighting with leader
   * teached AI that all keeps can be used for recruiting, not just his home keep
   * share castle with allies, make room for ally leader to recruit
   * various improvements to make AI smarter
   * added recruitment_ignore_bad_movement option
   * added recruitment_ignore_bad_combat option
 * multiplayer improvements
   * Wesbowl
   * King of the Hill
   * added scrolling and wrapping support for chat in lobby
   * shared vision option
   * made village placement fairer in random map generation
   * game setup improvements
   * random maps have now less forest
 * editor improvements:
   * added map resizing
   * added revert from disk functionality
   * selection of terrain is kept through undo and redo operations
   * preferences dialog
   * setting and removal of starting positions are now undoable operations
   * fixed name of the currently selected terrain not being displayed
   * fixed compile problems on some architectures
   * code cleanups
 * implemented multi-hex tiles
 * implemented 'not' tags in WML unit filters
 * fixed 'show enemy moves' and 'best possible enemy moves'
   showing in context menu when no enemies visible
 * fixed resizing window in the title screen not working properly
 * fixed basic hotkeys not working in title screen
 * fixed terrain type and position problems with 800x600 resolution
 * fixed game creator not getting ally messages
 * fixed font redrawing for the font used in the slovak and polish translations
 * fixed typos in 'Shan Taum the Smug' that caused scenario to have only two turns
 * fixed crashes when units with invalid sides appear, by rejecting such
   invalide sides in the unit's constructor
 * fixed bug where 'cancel' button would appear under multiplayer games
 * fixed preferences widgets disappearing when changing video mode
 * fixed tooltips from game setup appearing in-game
 * fixed hotkey not updating immidiately in the hotkeys dialog when changed
 * fixed assertion failure in playturn.cpp
 * fixed deleting units from recall list not updating unit preview pane
 * changed all WML files to UTF-8, code now assumes all WML files being UTF-8
 * made input of search strings and messages use a textbox that overlays the map,
   which allows these operations to be performed asynchronously
 * added caching of data files to speed game loading
 * added a "fire_event" in [kill] tags that allow die events to be processed if specified
 * re-ordered attacks: short range attacks are now always before long range attack on all units
 * removed old castle images
 * wesnoth_zip

## Version 0.7.6
 * added and improved unit images, leadership and healing images
 * added testing mode to move units on fog/shroud maps without updating the fog/shroud
 * lobby logo updated
 * new dialogs
 * increased Wose defense in woods
 * increased Dwarf defense in castle
 * scenario balancing for 'Heir to the Throne' campaign:
   * Test of the Clans
 * multiplayer improvements:
   * made observers see perspective of active player
   * allowed observers to talk
   * game creator can chat while setting up game
   * players can chat with other game members when waiting for game to start
   * random faction
 * editor improvements:
   * allowed setting start pos for player 0
   * load map and new map menu items and hotkeys added
   * flood fill functionality added
   * different brush sizes added
   * new map and load map added, editor now starts with an empty map
   * save as functionality added
   * speed improvements: palette is only redrawn when needed
   * showing coordinates and terrain type of the hex on mouse over
   * start positions of players are now displayed as labels on the map
   * painting something on top of starting position now removes that starting position
   * cancel in new map dialog confirmation returns to dialog instead of map
   * added ability to mark tiles and fill marked tiles with selected terrain
   * cut and paste of marked terrain is now possible
   * drag and drop of marked terrain added
   * fixed tiles on the border of the map updating incorrectly when nearby tiles are drawn
   * fixed a bug where odd number of terrains where not drawn correctly in the palette
   * fixed dynamically generated terrain not showing correctly
   * fixed filename being not correctly remembered on load and save as
   * code cleanups and refactoring
 * added [store_unit], [unstore_unit], [while], [store_starting_location],
   [store_locations] and [store_gold] tags to WML
 * added idle AI
 * added image_healing' and 'image_leading' animation code
 * added floating labels to indicate healing/poisoning of units
 * added ability to highlight several hexes
 * added support for action images
 * changed toggleable menu items use the checkbox graphics
 * fixed Li'sar dying in Hasty Alliance not resulting in a loss
 * fixed game creator being able to click [I'm Ready] before all
   networked players have joined when loading game from save
 * fixed first player gold reset to 100 in multiplayer games
   when starting gold set less than 100
 * fixed character's name overlapping their profile when they speak
 * fixed teleporting units not clearing shroud/fow when they teleport
 * fixed Orcish Ruler movetype not being orcishfoot
 * fixed non-interactive mode not working with eras
 * fixed units moving slowly
 * fixed opening sequence in 'Isle of Anduin'
 * fixed cycling units not showing attack options
 * fixed units with drain not getting more than the max hitpoints
 * fixed Battle Princess not being able to get Scepter
 * fixed "invalid font characters" for slovak translation by adding
   'font=Bepa-Roman.ttf', translators should ensure this line is preserved
   when upgrading translations
 * fixed support for UTF-8 languages on text-boxes
 * fixed wesnoth.desktop (FreeDesktop shortcut definition file),
   which did contain invalid UTF-8 sequences
 * fixed experience modifier not working correctly when loading saved multiplayer game
 * fixed pressing escape key not closing dialogs which had only 'ok' button
 * fixed observers getting victory/defeat message at the end of the game
 * fixed statistics not working correctly when loading with replay
 * fixed screen menu flickering when dialog is cancelled
 * fixed negative numbers not displaying minus in front of them
 * fixed bug (u$win only) which caused defeat at the end of Valley of Death,
   even when you should have been victorious
 * fix to preprocessor
 * fixed some compile warnings
 * various improvements to wesnothd
 * relocated 'Heir to the Throne' scenarios to data/scenarios/Heir_To_The_Throne/
 * relocated 'The Dark Hordes' scenarios to data/scenarios/The_Dark_Hordes/
 * added ability to remove tiles from the border cache
 * added definitions of new pure virtual methods in hotkey.hpp
 * added wstring_to_string and string_to_wstring routines in language.cpp,
   which convert strings according to the current charset()
 * added toggleable action support to hotkey.[c|h]pp & playturn.[c|h]pp
 * added code to wesnothd to prevent spoofing of messages
 * added code to wesnothd to stop malicious observer clients issuing commands for players
 * converted hex calculations to fixed point instead of floating point
 * changed hex size to 72x72, converted images
 * added -fno-omit-frame-pointer to autotools to prevent possible crashes due gcc bug
 * added -fno-omit-frame-pointer to slacknoth script
 * added KDevelop 3.x project file for Wesnoth

## Version 0.7.5
 * many map editor improvements
 * changed 'dark gray' player colour to 'orange'
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Shan Taum the Smug
 * enabled accented characters in textboxes for LATIN-1 charset
 * made it impossible to label shrouded hexes
 * fixed OGG checks in --enable-lite
 * fixed clicking to the right of a character in a textbox causing segfault
 * fixed crash when pressing 'escape' in 'advance unit' dialog
 * moved scorpionfoot movetype to game.cfg
 * added unhandled exception code

## Version 0.7.4
 * added and improved unit images and castle images
 * changed day/night hueing to make them more attractive
 * reduced Dwarvish Fighter hitpoints to 36
 * reduced Dwarvish Lord hitpoints to 75 and changed ranged attack to 10-2
 * increased Dwarvish Steelclad hitpoints to 50 and melee attack to 11-3
 * new units:
   * Lizardman
 * AI improvements
   * fixed AI attacking from silly places
   * improved AI recruiting, decisions are now based on terrain and enemy unit structure
   * trained AI to take advantage of backstab
 * multiplayer improvements:
   * added Lieutenant to Loyalists in Age of Heroes era
   * 9 player support in random map generator
   * unlimited turns
 * translation updates:
   * catalan
   * french
   * italian
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Desert of Death
 * made scrollbars faster by avoiding unnecessary redraws
 * added HOME and END keys support in textboxes
 * added selecting text on textboxes using keyboard or mouse
 * implemented mid-scenario changing of some side detail(s)
 * attack and defense weighting improvements
 * improved macros handling arguments with space
 * added attack_weight and defense_weight in [attack] tags so that
   units can choose a different weapon upon attack or defense
 * [have_unit] can now use a location filter
 * fixed victory_when_enemies_defeated attribute wasn't serialized properly
 * fixed statistics showing incorrectly on all but scenario 1
 * fixed infliced damage labels being not always displayed
 * fixed strange name-endings in name generator
 * fixed bug where Scepter of Fire scenario is loaded instead
   of scenarios after the Scepter of Fire
 * fixed Scepter of Fire being winnable by killing all enemies,
   now you need to get the scepter
 * fixed messages and options not found by make_translation (#7415)
 * fixed keeps located on the border of the screen having broken graphics
 * fixed bug in Desert of Death scenario
 * fixed textbox-content slow to update when it was larger than the textbox size
 * fixed editor handing keys improperly
 * fixed zooming messing up labels
 * fixed bug that caused crash in [kill] event
 * fixed alignments not being atomic strings
 * fixed networking bug
 * added fixes for OSX from Sithrandel
 * added configure --enable-lite
 * minor modifications to slacknoth
 * reverted editor indentation to BSD style
 * added checking of return values in constructors
 * code refactoring: simplified the 'display' module
 * code cleanups

## Version 0.7.3
 * added undoing recall
 * improved context menu
 * added and improved unit images, castle images and attack icons
 * added berserk attack ability and gave it to Dwarvish Berserker
 * Dark Adept ranged attack made stronger and cold resistance improved
 * Footpad made a bit easier to advance, faster movement, melee and ranged attack made stronger
 * Outlaw given faster movement
 * Orcish Ruler attack changed to sword, race changed to orc
 * Walking Corpse made a bit easier to advance
 * Goblin Knight experience needed to advance decreased to 150
 * new units:
   * Dwarvish Thunderguard
   * Iron Mauler
   * Soulless
   * Wose Sapling
   * Wose
   * Ancient Wose
 * new scenarios for 'Heir to the Throne' campaign:
   * Test of the Clans
 * scenario balancing for 'Heir to the Throne' campaign:
   * Dwarven Doors
 * scenario balancing for 'Son of the Black Eye' campaign:
   * Toward Mountains of Haag
   * Silent Forest
 * multiplayer enhancements:
   * added "private" messaging between allies
   * improvements to chat system
   * added experience multiplier slider in game creation
   * added eras in game creation
 * language fixes and polishing (english)
 * new translations:
   * catalan
 * updated translations:
   * polish
   * slovak
   * spanish
 * added ability to define 'time of day' for areas
 * added treefolk movetype
 * added terrain multiple aliases
 * allow multiple sides (comma separared) to be specified in unit filters
 * [have_unit] now honors location
 * new time of day image masks which get laid over hexes when it's that time of day
 * fixed on-map labels not printing black on light coloured terrains
 * fixed ellipses not fading when a unit dies
 * fixed bug where if the host changed their name, it would show that name
   to users connecting to the game
 * fixed bug in lobby where top-left of screen would not be drawn properly
 * fixed bug with AI not getting villages
 * fixed some compiler warnings
 * fixed Dwarvish Berserker and Yeti using blunt attack type, which does not exist
 * fixed Mage of Light aura slowing the game
 * fixed bugs with tooltips
 * fixed network bug
 * minor autotools fixes
 * autotools basic support for static building
 * added autotools configure checks for signed char
 * slacknoth improvements
 * added recovering connections framework to network protocol

## Version 0.7.2
 * show possible enemy moves view (context menu -> show possible enemy moves)
 * added and improved unit images
 * new unit side ellipses
 * scenario balancing for 'Heir to the Throne' campaign:
   * Dwarven Doors
   * Muff Malal's
 * Dwarvish Berserker attack made stronger
 * multiplayer enhancements:
   * improved multiplayer chat system
 * language fixes and polishing (english)
 * updated translations:
   * french
 * fixed attack miscalculation for offensive units which ignored time of day
 * fixed typo in Toward Mountains of Haag
 * fixed many 'end of line' warnings
 * fixed active player not receiveing labels and messages immediately
   while it was his/her turn
 * fixed networking problems which caused out-of-syncs
 * made mountains 'dark terrain'
 * made on-map labels disappear on end of level
 * reports handle multiple images & strings per report each with their own tooltips
 * added diagnostics to help tracking out-of-sync errors
 * added network diagnostics
 * autotools clean up
 * renamed wesnoth.png to wesnoth-icon.png

## Version 0.7.1
 * changed damage calculations to use additive percentage calculations
 * resting stacks with heal and cure
 * 'Sword of Fire' changed melee weapon with fire damage
 * made the game more interactive during the AI's turn
 * added and improved unit images, missile images and attack icons
 * merman and swamp villages, ice terrain, dirt and swamp transitions,
   trash heap (minor terrain item)
 * new fog of war images
 * underground daytime images
 * fading effect between daytime changes
 * attack stat tooltips
 * user interface polishing
 * support for cursors
 * on map floating labels ('set label' in context menu)
 * floating text for damage inflicted and status effects
 * shortcut keys are displayed next to menu items
 * undo, redo and next unit added to context menu
 * added deleting saved games
 * added deleting units from recall list
 * added fight calculations
 * added in-game statistics
 * new scenarios for 'Heir to the Throne' campaign:
   * Return to Wesnoth
   * Valley of Statues
 * scenario balancing for 'Heir to the Throne' campaign:
   * Blackwater Port
   * Hasty Alliance
   * Scepter of Fire
 * new campaign 'Son of the Black Eye' (orcs):
   * End of Peace
   * Toward Mountains of Haag
   * The Siege of Barag Gor
   * To the harbour of Tirigaz
   * Black Flag
   * Desert of Death
   * Silent Forest
   * Shan Taum the Smug
 * General melee attack number of strikes increased by one
 * Princess (Li'Sar) changed to 2nd level unit
 * Youth (Konrad) changed to level 0 unit
 * reduced Sleeping Gryphon's number of attacks to same with Gryphon
 * improved swimmer (Merman, Naga, etc) movement on swamp, grassland, sand and tundra
 * new units:
   * Battle Princess
   * Cockatrice
   * Giant Scorpion
   * Goblin Direwolver
   * Grand Knight
   * Orcish Shaman
   * Yeti
 * new special units:
   * Orcish Ruler (hero in the Son of the Black Eye campaign)
   * Pirate Galleon
   * Transport Galleon
   * Watch Tower
 * multiplayer enhancements:
   * random map generator enhanced
   * village number slider in random map generation is now proportional
   * users in lobby can see minimap of non-shroud games being played
   * number of available positions in a game is shown in the lobby
   * current turn of a game underway is shown in the lobby
   * eye of the observer
 * translations can specify font to use
 * added Bepa-Roman.ttf which has cyrillic characters
 * updated translations:
   * danish
   * hungarian
   * portuguese-brazilian
   * slovak
 * defenders will choose their weapons more intelligently
 * de-elvished Gryphon Rider description and set race to human
 * fixed Cave Spider ranged attack to have slow specialty
 * fixed minor glitch in tutorial id messages
 * fixed random map generator creating invalid small maps
 * fixed result=continue showing victory message
 * fixed units not getting resting if they ended turn with space
 * fixed enemies getting healed after loading, resting serialized in unit
 * fixed Merman Lord -> Triton in some scenarios
 * fixed allow/deny observers being not respected in multiplayer game creation
 * fixed multiplayer replays not working
 * fixed random map generator retain the 'number of players' setting
 * fixed PNG and OGG checking bugs in configure for MacOSX and Windows (with cygwin)
 * fixed crash on empty maps
 * fixed problem with units having movement reset
 * fixed 'right column inaccessible' bug
 * fixed "X_ShmPutImage" crash on GNU/Linux
 * made it less likely for castles to appear in the sea on randomly generated maps
 * improved network protocol (breaks backward compatibility)
 * added optimization to make the game load faster for people using the default english locale
 * slacknoth now ignores an insignificant makefile error

## Version 0.7
 * added new sounds, unit images and attack icons
 * polished unit descriptions for recently added units
 * old tutorial strings removed
 * fixed Lieutenant -> General advancement path
 * fixed multiplayer out of sync bugs
 * fixed swedish

## Version 0.6.99.5
 * many of the unit images were drawn using a SNES game graphics as basis:
   all images which we thought might be considered as derived work have been redrawn
 * theme update, scrollbars
 * added new sounds, unit images and attack icons
 * invisibility "wears off" when enemy unit comes adjacent (#6879)
 * allowed swimming units to move slowly on hills
 * resting restores 2 hitpoints
 * scenario balancing for 'Heir to the Throne' campaign:
   * Gryphon Mountain
   * Northern Winter
   * Swamp of Dread
 * undead campaign updates and new scenarios:
   * Brother Against Brother
   * The Skull of Agarash
   * Mages and Elves
   * Inside the Tower
   * Crenalu's Book
 * new units:
   * Bowman
   * Longbowman
   * Master Bowman
   * Footpad
   * Trapper
   * Spectre
   * Initiate
   * Dark Spirit
   * Deathmaster
   * Demilich
   * Nightgaunt
 * Elvish Archer and Swordsman made more powerful
 * Elvish Marksman, Elvish Sharpshooter and Orcish Archer ranged attack improved
 * Gryphon, Evil Necromancer, Lich and Ancient Lich made weaker
 * Dark Apprentice advances to Deathmaster
 * Evil Necromancer advances to Demilich
 * Lich does not advance to Ancient Lich anymore
 * removed Gryphon Rider from Rebels and gave it to Knalgan Alliance
 * language fixes and polishing (english)
 * new translations:
   * finnish
   * portuguese-brazilian
   * slovak
 * updated translations:
   * brazilian
   * danish
   * french
   * hungarian
   * norwegian
   * swedish
   * spanish
   * french
 * added ctrl+P as hotkey for 'Preferences'
 * campaigns can customize difficulty level icons
 * mage of light lights up hexes around him
 * fixed many image transparency glitches (caused by recent alpha channel implementation)
 * fixed teleporting displaying incorrectly where unit can move
 * fixed Konrad dying in Siege of Elensefar not ending the game immediately
 * fixed AI bug with skirmishers (#8041)
 * fixed crash when you press escape when a menu is displayed (#7863)
 * fixed unit descriptions leaking thru fog of war (#7843)
 * fixed bug where units would regain hitpoints after reloading the game (#7842)
 * fixed slow down right before a unit arrives at its destination hex,
   due to redrawing of the minimap
 * fixed ghosted usernames on server (#7655)
 * fixed focus not returning Konrad on new turn (#7188)
 * fixed wrapping of text in opening sequence (#7878)
 * fixed bug where on large maps, the cursor wouldn't point at the correct hex
   toward the eastern side of the map
 * fixed scroll speeds being set to insane values
 * fixed bug where AI could see potential movement of invisible enemy units
 * fixed bug where Blackwater Port was easier than it was meant to be on 'normal' level
 * fixed crash in maps with fog of war
 * fixed transparency issues in dwarven doors graphic
 * fixed issue with escape button skipping past messages where the user must make a choice
 * fixed bug where shroud wasn't saved/loaded properly (#7922)
 * fixed tutorial id
 * added random to set_variable
 * added victory_when_enemies_defeated
 * made it easier for different AI algorithms to be added
 * added Doxygen documentation to ai_interface
 * autotools fix: 'make uninstall' removes data directories if they are empty
 * slacknoth script does a 'make clean' before compiling source,
   strips binaries and adds package info
 * refactored widget class
 * removed some debug cerrs
 * added VC++6 project files to CVS

## Version 0.6.99.4
 * added alpha channel
 * new tutorial
 * theme update
 * more unit graphics, animations and attack icons
 * new music added
 * resting implemented: units that don't do anything for a turn get few hitpoints back
 * 'Mage of Light' gives advantage also at dawn
 * multiplayer changes
   * Knalgan Alliance: dwarves and their allies
   * Rebels can't recruit dwarves or thieves anymore
   * ally village ownership information is shared over fog of war (#7154)
 * synced MANUAL with Wiki
 * language fixes (english)
 * new translations:
   * brazilian
 * sea creatures in Ford of Abez do not attack Li'sar
 * made energy bars fade out when a unit is killed
 * made circles disappear after a unit is killed
 * made recruiting update fog of war and shroud
 * made it so you can't create units that are off the map
 * fixed flaming sword not being found
 * fixed missing race=elf for 'Elvish Captain'
 * fixed bug where Elf wouldn't do anything in 'Mages and Elves' scenario
 * fixed units disappearing on event (#7032)
 * fixed need to be able to escape opening conversation (#7798)
 * fixed problem where some menus would have scroll buttons displayed in top-left corner
 * fixed problem with textbox not processing properly
 * fixed illegal memory access
 * fixed flop() to work properly
 * workaround to SDL bug
 * removed definition of log2 which clashes with some compilers
 * removed use of auto_ptr to help with compilation on older compilers
 * added a draw_wrapped_text method font.[ch]pp

## Version 0.6.99.3
 * data compression for network games
 * more unit graphics and animations
 * tutorial objectives added
 * scenario balancing for 'Heir to the Throne' campaign:
   * Sceptre of Fire
 * made it possible to recruit on any keep tile, not just a starting keeps
 * intelligence (trait) now reduces experience needed to advance by 20 %
 * quick (trait) now reduces maximum hitpoints by 10 %
 * added orcishfoot (orcs, goblins), "orcs" are now faster on hills and mountains
 * changed orcish units previously using smallfoot to use orcishfoot
 * mountainfoot (dwarves) made a bit faster on forest
 * mountainfoot village and castle defense increased
 * undeadfoot (undead) made faster on swamp
 * undeadfoot swamp defense increased
 * undeadfoot can now move on mountain, though very slow
 * woodland (elves) made a bit faster on mountains
 * new units:
   * Outlaw
   * Bandit
 * AI improvements
   * retreating, re-grouping
   * recognizing allies
   * attack selection improvements
   * grouping improvements
   * defending leader
   * fixed leader abandoning keep when it was not safe
   * fixed AI leaving units idle when it has a large number of units
 * updated translations:
   * french
 * improved castle placement on random maps
 * added 'new turn' event
 * added 'side turn' event
 * fixed healing units that had full hitpoints (#7574)
 * fixed problem where [hide_unit] still displays unit's hitpoint bar
 * fixed unit names being different in networked multiplayer games
 * fixed bug where temporary items were lost on level up or when saving and reloading the game
 * fixed bug which caused dialogs that extended past screen boundaries to crash program
 * fixed bug where unterminated #define would crash the game
 * fixed "Two Delfadors" bug
 * attempt to fix hitpoint bars not displaying properly on Mac OS X
 * relocated images under misc/: flamesword, holywater, sceptreoffire, staff

## Version 0.6.99.2
 * nightstalk ability added
 * added turn-to-stone special weapon ability
 * invisible units are now revealed when enemy comes adjacent to them
 * poisoned units are shown with a green hue
 * updated MANUAL
 * new units:
   * Thug
   * Poacher
 * increased resistance values for all Dwarvish units
 * increased cost of 'Dwarvish Fighter' to 17
 * reduced 'Dwarvish Lord' movement to 4
 * skirmish ability given to Fencer and Duelist
 * scenario balancing for 'Heir to the Throne' campaign:
   * Home of the North Elves
 * minimap scrolling
 * new music added
 * theme updated:
   * new menus
   * added menu buttons to top left of screen
   * slider updated
   * new buttons
   * status icons in unit information panel
 * multiplayer improvements:
   * Loyalists can recruit Horseman
   * Rebels can recruit Dwarves
 * wesnothd.6 manual updated
 * credits, people ordered by role
 * tweaked look of tooltips
 * made it so the icon for the paladin's sword is used when the paladin attack
 * added Dwarvish Steelclad attack frame
 * escape save game filenames to allow special characters on them
 * removed show_ai_move options, since there is a serious bug with it
 * fixed poison healing bug
 * fixed bug where killing an enemy that caused an [endlevel] event
   to occur prevented advancement by the unit killing
 * fixed bug where commander would carry over movement from previous level
 * fixed storyline image for Isle of the Damned
 * fixed adjacent terrain for pier terrain
 * fixed bug that caused Wesnoth to fail to compile in ia64
 * fixed crash when connecting to outdated servers
 * fixed problem with changing video modes on machines that use emulated bpp
 * fixed crashing on missing intro image
 * fix to bug in tiles_adjacent()
 * fixed "Leving" -> "Leaving" in Shadow description
 * fixed problem where after being disconnected from the server,
   players would be unable to reconnect without quitting the client
 * fixed game not notifying when save game failed
 * fixed round indicator does not show completely (#7500)
 * attempt to fix STL-related compile error on Slackware
 * added find_visible_unit, to find a visible_unit by location in a unit_map
 * initial implementation of developer documentation (doxygen)
 * refactored some AI code

## Version 0.6.99.1
 * extended tutorial
 * two new music tracks
 * new user-interface
   * support for themes
   * enhanced support for resolutions below 1024x768 (e.g. 800x600)
   * very simple chatting is now possible in-game (mouse right-click menu -> speak)
   * new time of day images, attack icons, new buttons, checkboxes,
     panel has been split to top and right-side panel...
 * medium difficulty level is used by default when starting new campaign
 * snapshot saving, faster loading if you skip replay
 * more unit graphics, animations and sounds
 * healers can heal allies
 * undeads can not be drained anymore
 * new units:
   * Shadow
 * dart damage type changed from pierce to blade,
   this affects 'Orcish Assassin' and 'Orcish Slayer'
 * fixed description for 'Wolf Rider'
 * changed 'Evil Necromancer' advanceto 'Lich'
 * Mage stats changed: hitpoints increased to 24, ranged attack does now 6-3 (magic),
   60 experience needed to advance
 * reduced movement for Arch Mage and Great Mage to 4
 * Dwarvish Lord changes: renamed 'tomahawk' to 'hatchet', removed leadership,
   increased resistance against blade, piercing and impact
 * Necromancer changed to 'human' (movement_type is now 'smallfoot'),
   experience needed to advance increased to 80
 * Lich experience needed to advance increased to 250
 * 'Ancient Lich' Melee damage decreased to 8-4
 * free units don't get traits anymore, however they are free in future scenarios
 * new scenarios for 'Heir to the Throne' campaign:
   * North Elves
 * scenario balancing for 'Heir to the Throne' campaign:
   * Hasty Alliance
   * Sceptre of Fire
   * A Choice Must Be Made
 * new scenarios for 'The Dark Hordes' campaign:
   * Mages and Elves
 * scenario balancing for 'Dark Hordes' campaign:
   * Brother Against Brother
   * The Skull of Agarash
 * units will stop moving when they see new units (shroud/fow)
 * moves that do not reveal shroud/fow are undoable
 * when a leader dies, all villages for that side become neutral and you
   can not anymore capture villages, you can still "neutralize" villages of other players
 * 'show team colours' now uses colored circles to identify units sides
 * added 'show team colours' option to preferences dialog
 * added some explanatory tooltips for some unit attributes on sidebar
 * added in flag to show whose turn it currently is
 * added end turn confirmation when units can still move (requires
   editing preferences-file - see WesnothPreferences on https://wiki.wesnoth.org for more information)
 * added 'unit list' to context menu
 * restored 'save game' in the context menu (#7172)
 * multiplayer improvements:
   * enhanced multiplayer game setup
   * random maps
   * made basic messaging possible using 'Speak' from main menu
   * 'delayed map sharing' between allies
   * when player quits or gets disconnected from multiplayer game,
     game creator chooses to replace them with ai/local player or abort
   * loaded games should now start properly and with right settings
   * there are now flags (and colors) for 10 teams
   * minimap shows FoW and units
   * configurable wesnothd port see 'wesnothd --help'
   * client support for non-standard server ports, syntax: server.address.here:port
 * AI improvements
   * leader seeks for healing as long as his keep is safe
   * leader captures nearby villages in some situations
   * leader requests help when attacked
   * attack is chosen more effectively
   * movement algorithm changes
   * restructured AI code
 * man-page for wesnoth_editor
 * new translations:
   * swedish
 * updated translations:
   * danish
   * dutch
   * french
   * spanish
 * internationalization string fixes
 * made make_translation and merge_translations compile
 * fixed "Computer vs Computer" not in string_table (#7295)
 * improved speed of reading cfg-files
 * units in water tiles are displayed partially submerged
 * added support for height adjustment of units
 * added snowed hills and forest - H and F
 * default selection for weapons when attacking
 * all event tags are now handled in the order in which they appear
 * teleport event
 * added several new toys for scenario developers to play with (wesnoth --test to see all thats done)
 * allowed items to set status flags on units
 * tweaked unit cfg-files for "weapon namespace uniformity" - needed for attack icons
 * implemented new 'team_name' tag to show alliances to replace 'enemy' tag
 * adjusted the way in which images are blitted
 * implemented new widget system with sample: textbox
 * borders of game field show dummy terrain instead of black areas
 * made images use proper transparency, instead of using black as transperant colour
 * pressing space really ends turn for a unit (#7311)
 * menu arrows are disabled when we are on top or in the bottom of menu (#7389)
 * allow scrolling to fogged zones clicking the minimap
 * fixed pressing 'n' (next unit) returning to same unit (#7308)
 * fixed player being able to see enemy unit list on enemy turn (#7312)
 * fixed unit list not showing data in correct columns for units without name (#7313)
 * disabled observers using the in-game speak (#7156)
 * seeing new areas when there is FoW now really prevents undo (#7177)
 * FoW dies when unit dies, FoW is recalculated when unit dies in combat (#7114)
 * fixed client-hosted multiplayer bug where more than two player games were not possible
 * fixed some multiplayer saving/loading bugs
 * fixed some bugs with loading replays
 * fixed problem with saving at the end of the scenario pointing to the previous scenario
 * fixed problem with units having goto orders at the beginning of scenarios
 * fixed seeing footprints of other team's units
 * fixed clicking on minimap losing focus on your unit (#7402)
 * fixed problem with old units being displayed in status bar
 * fixed bug where capturing a village with a leader would not change the village to your side
 * fixed bug where scenario objectives in loaded games would sometimes have weird boxes on the end
 * fixed bug which could cause a crash when a unit advances
 * fixed problem where changing between fullscreen/windowed mode in preferences
   could do strange things
 * fixed start game in windowed mode -> tutorial
   -> ctrl-f to change to fullscreen -> quit to main menu -> preferences
   -> click button to change to full screen -> close preferences dialog -> crash
 * fixed bug where very large dialogs could crash the game
 * fixed some problems with too-large dialog boxes
 * fixed bug where an invalid side specification for a unit could crash the game
 * fixed problem in Ford of Abez with monsters attacking Li'sar
 * music on intro now has less interference
 * credits now run smoother
 * fixed Wraith appearing squashed in the panel (#7185)
 * fixed WML parser bugs
 * fixed misspelled 'The Siege of Elensefar' scenario filename
 * fixed visualization problems with in fmunoz & ettin's names in Credits
 * fixed strange text handling in intro (#7418)
 * fixed Pikeman advanceto typo (#7337)
 * fixed client connecting forever after a client disconnection (#7195)
 * many network and wesnothd code fixes and changes to make it a bit more stable
 * moved network code to mp_connect
 * autotools:
   * changed AC_ARG_ENABLE in server, editor and tools to fix bug
   * checks for png support in sdl_image and ogg support in sdl_mixer
   * application icon and menu entries in KDE with --enable-kde
   * application icon and menu entries in Gnome with --enable-gnome
   * removed autoconf version 2.57 prerequisite
   * adding autogen.sh, so CVS users don't need autotools installed
 * a workaround to allow users not using 16 bpp to change video mode
 * changed drawing routines to use more SDL routines,
   and less direct manipulation of surfaces
 * removed unnecessary diagnostics
 * code refactoring and cleanups

## Version 0.6.1
 * removed plague ability from Wraiths
 * decreased Lich's melee damage by one
 * hitpoints are displayed in red/white/green depending on 1/3, 2/3, full energy
 * display experience in green when unit is near advancement
 * basic support for 800x600 resolution has been added, it still needs a lot work
 * translated manuals:
   * norwegian
 * new translations:
   * dutch
 * autotools: enabled changing datadir name with ./configure
 * autotools: configure stops and gives error messages if libs & includes aren't found
 * fixed Crossroads scenario lacking victory and defeat conditions
 * fixed 'A Choice Must Be Made' defeat conditions
 * fixed bug causing crash on scenario completion
 * fixed 'Storm Trident' mislocation on 'Bay of Pearls'
 * fixed resetting player starting gold, gold per village, player race,
   and player type when loading multiplayer game
 * change the way images are handled to fix resizing (and other potential) bugs
 * fixed problem where gold was reset to 100 on some scenarios
 * made it so both holy water items in the valley of death work properly
 * fixed bug where using items such as the 'Storm Trident' and
   'Sceptre of Fire' could crash the game
 * fixed bug that occurred when the display was zoomed very far out
 * fixed reference counting problem
 * code cleanups

## Version 0.6
 * radical storyline changes, this breaks saves from older versions
 * fog of war
 * enemy units do not vanish anymore when enemy leader is killed
 * new scenarios for 'Heir to the Throne' (Konrad's Tale):
   * Isle of the Damned
   * Crossroads
   * Northern Winter
   * The Lost General
   * Hasty Alliance
   * Sceptre of Fire
   * A Choice Must Be Made
   * Snow Plains
   * Swamp of Dread
 * scenario balancing:
   * Bay of Pearls
   * Isle of Anduin
   * Dwarven Doors
   * Mountain Pass
   * Valley of Death
   * Ford of Abez
 * support for multiple campaigns
 * new campaign started: 'Hordes of the Undead'
 * multiplayer improvements:
   * show minimap when selecting map for new game
   * configurable starting gold per side
   * configurable 'gold per village'
   * teaming (alliances)
   * configurable number of turns
   * configurable fog of war
   * configurable shroud
 * autotools has been taken in use, old Makefiles can be found under utils/
 * updated docs - INSTALL, MANUAL
 * man-pages for wesnoth and wesnothd (contributed by Debian package maintainer) - doc/man/
 * added a script for easy Slackware package creation - utils/slacknoth
 * added random map generator (requires Perl) - utils/random_map.pl
 * added in repeatable recruit using ctrl+shift+r
 * hotkey configuration from GUI
 * confirm overwriting of save games
 * savegames now display and are sorted by date and time
 * income calculations are now done at the beginning of each player's turn
 * made it so units that can no longer advance have experience needed displayed as '-'
 * allow use of escape to exit out of ok/cancel and yes/no dialogs with negative results
 * double-click support in menus
 * new units:
   * Heavy Infantry
   * Shock Trooper
   * Sea Hag
   * Goblin Pillager
   * Death Knight
   * Elvish Scout
   * Pikeman
   * Dwarvish Steelclad
 * Konrad now starts as Fighter, which is 1st level unit
 * undead are now immune to poison and plague
 * reduced movement for Elvish Lord
 * reduced cost of Mage
 * changed Mage attack to fire based
 * 'Mage of Light' has now both cure and illumination
 * Increased shaman slowing attack damage, decreased unit cost
 * changed 'Red Mage' to neutral alignment
 * removed 'Elvish Outrider' as evolution from Horseman
 * Scout has been renamed to Cavalry
 * Cavalry stats adjusted, increased cost by one, reduced movement by one,
   reduced experience needed to advance
 * Horseman now needs more experience to advance
 * 'Vampire Bat' now needs more experience to advance and costs more
 * 'Mage of Light' attack is now a bit more powerful
 * fixed max experience for Orcish Warlord
 * 'Blood Bat' unit description clarified
 * Halberdier changed to 3rd level unit
 * Spearman now advances to Pikeman
 * Increased fire resistance for 'Red Mage' branch units
 * Increased holy resistance made higher for Holy units
 * auto-naming (currently elves and humans only) of units and renaming of units
 * units now have races defined in cfg-files
 * more and improved unit graphics and animations
 * changed team 6 color to purple
 * new translations:
   * german
   * hungarian
 * updated translations:
   * danish
   * french
   * spanish
 * translated manuals:
   * french
   * italian
   * spanish
   * german
 * changed "AI" in the english config to "Computer Player"
 * fixed problem with stripping of non-latin characters at end of string in config files
 * allow internationalization of traits
 * made AI more intelligent
 * scenario building:
   * added [remove_shroud]
   * added [allow_recruit]
   * added [teleport]
 * added in support for some special effects: flashing and scrolling/tremors
 * added coding support for displaying different terrain images
   (particularly useful for villages) at different times of the day
 * fixed minor bugs in unit configuration files for Elvish Sharpshooter and Fencer
 * fixed problem where 'goto numbers' can overwrite part of the right panel
 * footsteps disappear as you walk over them
 * added engine support for weapons that can reach multiple hexes
 * added some utility macros to WML
 * added a rotate function, you can quickly rotate a SDL_Surface by any angle
 * text is drawn on screen progressively instead of all at once during introductory sequence
 * fixed problems with map updating on map scene
 * fixed bug with 'slow' attacks sometimes causing saved games to be corrupt
 * fixed bug with crashing when switch to full screen from preferences menu
 * updated MapEditor so it compiles again
 * fixed tiny maps bug
 * fixed editor so it compiles again
 * fixed problem with items on status table displaying on top of each other
 * fixed problem with server crashing on startup
 * converted combat calculations to use fixed-point math
 * fixed crash with more than 6 sides
 * fixed compile error with older versions of SDL
 * made time of day images update properly
 * fixed assertion failures
 * fixed problem where slamming screen against right side and holding down
   right cursor key would cause hexes to shift around
 * made mouse wheel and middle-click work again
 * resolved conflicts in game.cfg
 * made it so flags are overlayed on top of villages properly
 * made image cache flush occur when switching video modes,
   in an effort to solve colour distortion problem on MacOSX
 * fixed seg fault bug
 * fixed energy bar images
 * changed energy movement orbs
 * white spaces were changed to _ in scenario IDs and next_scenarios
 * fixed problem with loading saved games with duplicate roles
 * fixed bug in AI path finding
 * fixed problem with undos/redos causing assertion failure
 * fixed bug with drawing tiles
 * after a goto command is given, the unit is deselected
 * fixed minor multiplayer bug
 * fixed tool tips to work again
 * fix hitpoint bar problem on MacOSX
 * added diagnostic to attack calculations
 * added error logging and correction for combat related syncing errors
 * client_type=ai in preferences-file make player join
   networked multiplayer games as AI player
 * added some comments to code
 * added icon to Windows distribution

## Version 0.5.1
 * more and improved unit graphics and animations
 * added show_ai_moves and show_combat options for preferences file
 * in 'Valley of Death' Li'Sar now arrives with Spearmen instead Swordsmen
 * renamed Soldier to Spearman
 * made it so shroud displays properly on the minimap
 * added support for UTF-8
 * extensive changes to the way event handling works
 * make multiplayer lobby look better
 * made advancement of units on a remote machine random
 * made display draw more efficiently
 * made it so AI players have an empty description in multiplayer games
 * added in some additional assertion checks
 * made some templated code more portable across compilers
 * fixed problem with AI attacking units not always advancing levels
 * map file format is now correctly oriented

## Version 0.5
 * low traffic wesnoth-releases mailing-list,
   subscribe at http://mail.nongnu.org/mailman/listinfo/wesnoth-releases
 * network multiplayer
 * wesnoth server
 * more sound effects and musics
 * more and improved unit graphics and animations
 * portraits when major characters are talking
 * more story graphics
 * many compile errors squashed
 * updates to Danish, French and Spanish translations
 * updates and fixes to editor
 * snow and desert terrains, improved graphics for grassland
 * scenario difficulty adjustment/balacing
 * new scenarios:
   * The Blackwater Port
   * Plunging Into the Darkess
 * new units:
   * Elvish Shyde
   * Dark Adept (Necromancer wannabe)
   * Dragoon
   * Soldier
 * Swordsman was changed to level 2 unit
 * Gryphon claws attack is now blade
 * reduced movement for Dwarvish Berserker, Dwarvish Lord,
   Orcish Assassin, Orcish Slayer and Orcish Crossbow
 * elven units are now neutral
 * changed 'night vision' to ambush ("invisibility" at forest)
   for Elvish Ranger and Elvish Avenger
 * changed Elvish Outrider weapon to sword (no more charging)
 * human units are now lawful
 * renamed Assasin to Assassin
 * changed Orchish Archer and Orcish Crossbow melee attack to blade
 * elusivefoot now take more damage when hit
 * fixed teleporting to work properly
 * many small unit adjustements to balance units
 * lightbringer ability renamed to illuminates
 * flying units are now harder to hit
 * allow keystroke repeat when holding down in text entry fields
 * added toggling of grid with control-g
 * fixed cycling to goto units
 * made it so when unit is selected, pressing a number n on the keyboard
   will show how far that unit can move in n turns
 * added mousewheel support
 * added center-on-middle click
 * fixed bug with replays and moving onto villages
 * added shroud
 * removed timeout in intro sequence
 * button transparency problems fixed
 * fixed energy bar display problem
 * fixed position on minimap when clicking
 * make fights go faster when accelerated
 * improved AI
 * added tooltips for text that cannot fit
 * added turn dialog and turn bell options
 * added in 'status table'
 * added better dialog for when unit is recruited
 * added new dialogs and right-side panel
 * path highlighting (footsteps)
 * menu borders updated
 * added in showing of how many turns a goto command will take.
   Mousing over terrain with unit selected will show defense % of the terrain
   and movement cost for that unit
 * made preferences accessible from the title screen
 * added configurable scrolling speed
 * added in colour cycling for day/night
 * removed deprecated display::get_minimap_location() function
 * fixed choose_weapon assertion failure
 * got rid of use of spaces in saved game names
 * refactored menu class into its own source file
 * updated scenarios to use the 'next_scenario' system
 * fixed problem with round()
 * getting rid of scenario numbering, preparing for less linearity
 * added PREFERENCES_DIR preprocessor symbol which specifies name of preferences directory
 * fixed some crashes when loading preferences.
 * added configurable day/night scheduler
 * removed unit status dialog from unit description as it's redundant

## Version 0.4.8
 * lots of bugs fixed
 * translations updated and some new translations
 * AI improvements - grouping, waiting for reinforcements
 * new tile graphics for grassland, shallow water, deep water, sand and keep
 * more unit animations, sounds and descriptions
 * reduced movement for advanced units, they don't need to be both
   more powerful and faster than lesser units
 * horseman is now neutral unit
 * paladin sword attack is now holy
 * wolf rider, goblin knight, vampire bat and blood bat attack is now blade
 * new healing/curing, paladin and shaman have healing, white mage and druid have cure
 * go-to implemented
 * ask to save replay at end of scenario
 * autosave ("crash recovery")
 * customizable hotkeys, configured in preferences-file
 * pressing window close button quits the game
 * scenario objectives are shown at the start of scenario
 * terrain descriptions now use 'terrain (underlying terrain)' format
 * when a unit reaches border of screen we center map on unit instead of scrolling
 * validate saved games with version number
 * screen resolution can be changed in preferences
 * allow window to be dynamically resized when running windowed
 * use video hardware surfaces where possible (Windows seems to have trouble
   with hardware surfaces so game always uses software surfaces when running on Windows)
 * code cleanups
 * lots of compiler warning eliminated
 * sourcecode moved to src/

## Version 0.4.7
 * patched the units config files with miyo's patch to clean up the structure
 * made it so the game will dump core if it segfaults
 * fixed crash reported by zas when a unit attacks
 * you can now zoom in and out when the AI is moving
 * added improved keyboard control that allows keys to be set in the locale settings.
   Keyboard shortcuts (in the English locale) changed
 * added ability to make different game-paths - e.g. the scenario
   you play could be determined by how you won last scenario.
   Dialogs can now be popped up which ask the player to make a selection,
   and the selection they make determines how the game progresses.
   (But no part of the game actually uses this feature yet)
 * added scenario objectives in scenario 10
 * now when the AI attacks an enemy and doesn't kill it, it will move
   other nearby units toward the place where the combat took place
 * if an AI sees only combats that result in loss nearby,
   it will request reinforcements from nearby
 * made it so that if the macro WESNOTH_PATH is defined, the game
   will look for its data files at that path. You can thus compile with
   e.g.  -DWESNOTH_PATH=\"/usr/local/games/wesnoth\"
   -- the Makefile does not currently take advantage of this
 * added benchmarking of times to perform various operations in the game
 * made leaders start in a 'keep'. (But need a better image for the keep
 * added difficulty level settings for scenarios up to scenario 9
 * added (partially done) Italian translation
 * added 'skip turn' option, accessible by pressing space -
   will end the selected unit's turn, and go to the next unit that has moves left

## Version 0.4.6
 * converted over to using png images instead of bmp
 * fixed up difficulty levels for 'normal' on scenarios 3 and 4 as reported by miyo
 * made it so healing animations and sounds don't play during replays
 * removed name generation code, since it wasn't fast enough
 * added sounds to a number of Elvish units
 * made it so that when the attack-selection dialog is displayed, the
   attacking unit is displayed in the right side bar,
   so that one can easily compare the attacking and defending units
 * fixed bug where after a scenario is loaded, on new turn it wouldn't scroll to the leader
 * made the display area for units bigger when recalling, to fit in units like the Druid
 * reduced Mage's hitpoints from 25 -> 18
 * added in plague ability and gave it to walking corpse and wraith.
   A unit with plague will create a new unit of their own type whenever they kill an enemy unit
 * tweaked recruitment pattern on scenario 6
 * made marksman and sharpshooter both very bad at close range
 * increased power of Mage of Light's attack
 * made it so you get a gold bonus when you complete scenario 9
 * fixed scenario 7 to say 'survive for 2 days'
 * changed Swordman -> Swordsman and Beserker -> Berserker
 * made some speed changes which will hopefully help for people who
   are finding it slow on startup and after selecting difficulty levels
 * fixed display problem where background of menus that had scroll arrows looked displaced
 * corrected facing of dwarvern units
 * added transition hexes at the edge of the scenario to make the edges of the map look nicer
 * added a 'show grid' option

## Version 0.4.5
 * added mine image for scenario 11 provided by fmunoz
 * added some missing headers to source files, problem pointed out by zas
 * made the animation for healing take a little longer
 * added new shortcut, control-F alternates between full screen and windowed mode
 * added buttons 'next' and 'skip' to introduction sequence
 * added two new images from fmunoz to introduction sequence
 * added new preferences dialog, which contains volume controls for music and sound effects
 * got rid of flicker on title screen when you cancel selection of a dialog box
 * fixed a number of drawing bugs which caused strange lines to appear
   across the screen sometimes, and caused the game to display badly when zoomed out alot.
   Zooming out should now work perfectly
 * marksman (trait) to hit chance reduced to 60 %
 * night vision (trait) added to Elvish Ranger and Elvish Avenger

## Version 0.4.4
 * fixed assertion failure if you try to start a campaign and then cancel
   on the difficulty level settings
 * added 'merge_translations' tool which will merge an old version of a
   foreign language translation to the current English translation, making a
   new translation that has foreign language strings where they are available,
   and English strings otherwise
 * fixed crash if you opened a menu that had some empty strings in it
   (for instance clicking preferences in the French version)
 * changed recruitment so that now a leader can only recruit if they are on
   a starting location. They can recruit units on any vacant castle tile connected
   to the starting location they are on. The player can choose a location
   to recruit onto by selecting it when accessing the recruit menu. TODO -
   Need a graphic to distinguish the starting hex from other castle hexes
 * changed maximum items displayed in a menu before up/down buttons appear from 10 to 18
 * fixed bug in scenario 8 where objectives would not be displayed
 * fixed bug in scenario 9 where major characters could die without loss
 * changed it so that when writing a configuration file,
   [/element-name] will be used to end an element instead of [end]
 * fixed corrupted save file bug reported by miyo
 * made the White Mages that join you in level 7 leave you
   at the end of the level, as they are meant to
 * changed advancement animation colour to black for chaotic units
 * made it so that when an AI-controlled unit moves,
   its details are displayed on the sidebar, as suggested by miyo
 * implemented algorithm so that if an AI is moving and the map has to be scrolled,
   frames will be skipped if necessary to make the AI moving at a decent speed
 * added Danish translation
 * added improved French translation
 * made it so that if you mouse-over a unit, it will be displayed
   in the unit details on the sidebar
 * changed healing so that a healer can only heal up to 12 hitpoints per turn.
   Added animations and sound effects to healing
 * added in AI type 'guardian', which is a unit that will stay in position
   until enemies come in range at which point it attacks.  Now in scenario 3
   there is a cage with many mermen in it that has 2 naga guardians around it
 * added scenario 10 provided by Shroud and scenario 11 provided by fmunoz
 * added in test version of a naming algorithm that will automatically name
   created units that was submitted as a patch by a wesnoth user

## Version 0.4.3
 * made it so that when a directory is scanned for files,
   only files ending in .cfg will be used.
   This is mainly to stop vim swap files from being used
 * changed AI's movement routines to make it slightly more intelligent
 * added in difficulty levels - easy, medium, and hard
 * scenarios 1 and 2 now have easy/medium/hard difficulty levels implemented for them
 * when an enemy dies, its energy bar now fades out with it
 * added 'turbo' mode in preferences area.
   In turbo mode, the operation of the shift key is inverted.
   Turbo mode and full screen mode settings are now saved to the preferences file
 * made the time of day go dawn - day - day - dusk - night - night as suggested by miyo
 * fixed up bugs in the AI's pathfinding, the AI should now be substantially smarter
 * allowed setting of custom target units for the AI

## Version 0.4.2
 * if you hold shift, the game won't scroll at all, it'll jump between locations
 * added attack animations for necromancer and mage
 * added in Lohari's images with corrected shadows
 * added in Lohari's crossed daggers for battles, instead of the cross
 * added in Paladin's patch to highlight the hex of the unit that is currently selected
 * used new Makefile provided by zas
 * added setting of window title as suggested by zas
 * changed so that holding shift skips fading in recruiting units
 * increased cost of Naga from 8 -> 11 gold to make scenario 3 easier
 * fixed bug with recalling in mid-level saved games -
   should save alot of game corruption issues
 * made it so attack sounds do not play while loading game
 * added animations for Goblin Knight, Wolf Rider, and Troll Whelp
 * made it so Glordorf in Scenario 2 doesn't join the player
 * added in new missile images done by fmunoz
 * fixed bug where merman's storm trident would run out when the merman advances,
   or at the end of the level
 * page up and page down can be used to maneuver through menus
 * fixed bug reported by Jaramir, where exiting a multiplayer game
   would cause the save state to be remembered
 * fixed bug reported by Jaramir, where recruiting or recalling a unit
   wouldn't update your gold immediately
 * added better guarantees that when an AI attacks a unit,
   the unit being attacked won't be mostly off the screen
 * changed Necromancer to level 2
 * fixed bug where quitting the game by pressing escape during
   opening dialog sequence would cause the game to crash
 * added facility suggested by miyo where an entire directory can be scanned
   to look for configuration files. Re-arranged files in data/ to utilize this
 * changed configuration files to allow [/tagname] to end a tag instead of [end].
   Added better error handling for bad configuration files
 * added utility make_translation which when run, will construct a
   sample translation with all the strings that can be translated in it
 * removed Outrider's spear/charge attack and replaced it with a sword
 * added sorting to recall list

## Version 0.4.1
 * added music and sound support. The game has one song, provided by ZhayTee+.
   Added some sample sounds for Elvish Fighter attacking
 * fixed up bug where if you loaded a mid-level game, and then saved
   at the end of level, the save would be corrupt
 * fixed bug where the display mode would always be displayed as 'windowed'
 * make the game report an error message if switching between windowed/full screen fails
 * make the game handle switches between windowed and full screen even if exact colour depth can't be matched
 * arrow keys can now be used to maneuver menus, and enter can be used for 'ok and yes' in dialogs
 * fixed bug that caused a crash at the end of the tutorial
 * fixed bug where moving a unit along the border edges would go very slowly

## Version 0.4
 * fixed a bug with unit description images being messed up if you zoom in
 * fixed time to load a game
 * fixed missiles up to point in the correct direction
 * fixed naming of Shallow Water and Deep Water (used to be 'ocean' and 'coast'
   and not internationalized properly)
 * removed bug where a dialog could overwrite part of the sidebar on the right
 * now you have to 'kill' Li'sar in scenario 6 before she'll surrender
 * added more cleaning in Makefile
 * use new dynamically generated buttons based
 * use anti-aliased fonts
 * addition of a multi-player mode that allows hot seat multiplayer games to be played
 * added button 'n' which takes you to the 'next' unmoved unit.
   You can cycle through all your unmoved units by pressing 'n' repeatedly
 * added 'preferences' item in options menu.
   The one preference at the moment is to be able to toggle between windowed
   and full-screen from within the game
 * changed rectangle on the map of Wesnoth to cross
 * completed scenario 9
 * added gryphon rider unit and put them in scenario 9
 * added resistance tables and terrain movement and defense tables to 'unit description'
 * added unit descriptions to many more units
 * when a unit is recruited, it now fades in
 * AI leaders will now attack you if you move next to them
 * the ai now targets units that are close to advancing more,
   and tries to protect and advance its own units better
 * mage's attack changed to 8-2
 * human fencer unit added
 * wraith's damage taken up from 7 to 8
 * changed movement of red mage, arch mage, and great mage from 7 to 6.
   With teleporting, 7 movement is too powerful. May even reduce to 5 later

## Version 0.3.4
 * fixed display bug which kept the game from working on many machines
 * added Elvish Shaman and Druid units
 * added transperancy for some units
 * added undo and redo using the 'u' and 'r' keys
 * tweaked the 'valley of death' scenario
 * changed some unit values

## Version 0.3.3
 * fixed problem in scenario 2
 * implemented facing of units

## Version 0.3.2
 * incorrect references to images fixed
 * basic support for animated combats added
 * size of the energy bar scaled to the hitpoints a unit has

## Version 0.3.1
 * basic items support added
 * support for units to have different 'traits' added
 * combat is now animated
 * some redraw problems fixed
 * better interface design

## Version 0.3
 * many game rules changed/tweaked
 * speed improved
 * AI improved. (Although the AI sometimes does go a little slowly now)
 * new scenarios added
 * new unit types added
 * lots of tweaking, fixing of minor things

## Version 0.2.1
 * many redraw bugs fixed
 * new scenarios added
 * many new graphics added that were contributed by Paco
 * infinite recall bug fixed
 * recalling now costs 20 gold pieces. Gold from previous scenarios carries over,
   and there is a bonus for finishing a scenario early
 * better transitions between tiles added (graphics for this not complete though)
