if SERVER
SSUBDIRS = server
endif

if TOOLS
TSUBDIRS = tools
endif

if EDITOR
EPROGRAMS = wesnoth_editor
endif

SUBDIRS = . $(SSUBDIRS) $(TSUBDIRS)
bin_PROGRAMS = wesnoth $(EPROGRAMS)

pkgdatadir=$(datadir)/@DATADIR@

#############################################################################
#    Wesnoth                                                                #
#############################################################################

wesnoth_SOURCES = about.cpp \
                  actions.cpp \
		  ai.cpp \
		  ai_attack.cpp \
		  ai_move.cpp \
		  cavegen.cpp \
		  config.cpp \
		  dialogs.cpp \
		  display.cpp \
		  events.cpp \
		  filesystem.cpp \
		  font.cpp \
		  game.cpp \
		  game_config.cpp \
		  game_events.cpp \
		  gamestatus.cpp \
		  hotkeys.cpp \
		  image.cpp \
		  intro.cpp \
		  key.cpp \
		  language.cpp \
		  log.cpp \
		  map.cpp \
		  mapgen.cpp \
		  mapgen_dialog.cpp \
		  mouse.cpp \
		  multiplayer.cpp \
		  multiplayer_client.cpp \
		  multiplayer_connect.cpp \
		  multiplayer_lobby.cpp \
		  network.cpp \
		  pathfind.cpp \
		  playlevel.cpp \
		  playturn.cpp \
		  preferences.cpp \
		  race.cpp \
		  replay.cpp \
		  reports.cpp \
		  sdl_utils.cpp \
		  show_dialog.cpp \
		  sound.cpp \
		  statistics.cpp \
		  team.cpp \
		  terrain.cpp \
		  theme.cpp \
		  tooltips.cpp \
		  unit.cpp \
		  unit_types.cpp \
		  video.cpp \
		  widgets/button.cpp \
		  widgets/combo.cpp \
		  widgets/menu.cpp \
		  widgets/scrollbar.cpp \
		  widgets/slider.cpp \
		  widgets/textbox.cpp \
		  widgets/widget.cpp \
                  about.hpp \
		  actions.hpp \
		  ai.hpp \
		  ai_attack.hpp \
		  ai_move.hpp \
		  cavegen.hpp \
		  config.hpp \
		  dialogs.hpp \
		  display.hpp \
		  events.hpp \
		  filesystem.hpp \
		  font.hpp \
		  game.hpp \
		  game_config.hpp \
		  game_events.hpp \
		  gamestatus.hpp \
		  hotkeys.hpp \
		  image.hpp \
		  intro.hpp \
		  key.hpp \
		  language.hpp \
		  log.hpp \
		  map.hpp \
		  mapgen.hpp \
		  mapgen_dialog.hpp \
		  mouse.hpp \
		  multiplayer.hpp \
		  multiplayer_client.hpp \
		  multiplayer_connect.hpp \
		  multiplayer_lobby.hpp \
		  network.hpp \
		  pathfind.hpp \
		  playlevel.hpp \
		  playturn.hpp \
		  preferences.hpp \
		  race.hpp \
		  replay.hpp \
		  reports.hpp \
		  scoped_resource.hpp \
		  sdl_utils.hpp \
		  show_dialog.hpp \
		  sound.hpp \
		  statistics.hpp \
		  team.hpp \
		  terrain.hpp \
		  theme.hpp \
		  tooltips.hpp \
		  unit.hpp \
		  unit_types.hpp \
		  util.hpp \
		  video.hpp \
		  widgets/button.hpp \
		  widgets/combo.hpp \
		  widgets/menu.hpp \
		  widgets/scrollbar.hpp \
		  widgets/slider.hpp \
		  widgets/textbox.hpp \
		  widgets/widget.hpp

#############################################################################
#    Editor                                                                 #
#############################################################################

wesnoth_editor_SOURCES = editor/editor.cpp \
			 actions.cpp \
			 ai.cpp \
			 ai_attack.cpp \
			 ai_move.cpp \
			 config.cpp \
			 dialogs.cpp \
			 display.cpp \
			 events.cpp \
			 filesystem.cpp \
			 font.cpp \
			 game_config.cpp \
			 game_events.cpp \
			 gamestatus.cpp \
			 hotkeys.cpp \
			 image.cpp \
			 intro.cpp \
			 key.cpp \
			 language.cpp \
			 log.cpp \
			 map.cpp \
			 mapgen.cpp \
			 mapgen_dialog.cpp \
			 mouse.cpp \
			 network.cpp \
			 pathfind.cpp \
			 playlevel.cpp \
			 playturn.cpp \
			 preferences.cpp \
			 race.cpp \
			 replay.cpp \
			 reports.cpp \
			 sdl_utils.cpp \
			 show_dialog.cpp \
			 sound.cpp \
			 team.cpp \
			 terrain.cpp \
			 theme.cpp \
			 tooltips.cpp \
			 unit.cpp \
			 unit_types.cpp \
			 video.cpp \
			 widgets/button.cpp \
			 widgets/menu.cpp \
			 widgets/textbox.cpp \
			 widgets/scrollbar.cpp \
			 widgets/slider.cpp \
			 widgets/widget.cpp \
			 actions.hpp \
			 ai.hpp \
			 ai_attack.hpp \
			 ai_move.hpp \
			 config.hpp \
			 dialogs.hpp \
			 display.hpp \
			 events.hpp \
			 filesystem.hpp \
			 font.hpp \
			 game_config.hpp \
			 game_events.hpp \
			 gamestatus.hpp \
			 hotkeys.hpp \
			 image.hpp \
			 intro.hpp \
			 key.hpp \
			 language.hpp \
			 log.hpp \
			 map.hpp \
			 mapgen.hpp \
			 mapgen_dialog.hpp \
			 mouse.hpp \
			 network.hpp \
			 pathfind.hpp \
			 playlevel.hpp \
			 playturn.hpp \
			 preferences.hpp \
			 race.hpp \
			 replay.hpp \
			 reports.hpp \
			 sdl_utils.hpp \
			 show_dialog.hpp \
			 sound.hpp \
			 team.hpp \
			 terrain.hpp \
			 theme.hpp \
			 tooltips.hpp \
			 unit.hpp \
			 unit_types.hpp \
			 video.hpp \
			 widgets/button.hpp \
			 widgets/menu.hpp \
			 widgets/textbox.hpp \
			 widgets/scrollbar.hpp \
			 widgets/slider.hpp \
			 widgets/widget.hpp

AM_CXXFLAGS = @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\"
AM_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @SDL_MIXER_LIBS@ @SDL_NET_LIBS@ @SDL_TTF_LIBS@ 
