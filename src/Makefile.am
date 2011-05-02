AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS =
bin_SCRIPTS =

if GAME
bin_PROGRAMS += wesnoth
endif

if SERVER
bin_PROGRAMS += wesnothd
endif

if CAMPAIGNSERVER
bin_PROGRAMS += campaignd
endif

if TESTS
bin_PROGRAMS += test
endif

CLEANFILES = revision.hpp

if TOOLS
bin_PROGRAMS += exploder cutter
bin_SCRIPTS += wmlscope wmllint wmlindent wesnoth_addon_manager
CLEANFILES += $(bin_SCRIPTS)
endif

noinst_LIBRARIES = libwesnoth-core.a libwesnoth.a

pkgdatadir=$(datadir)/@DATADIR@

INTERNALLIBS = -lwesnoth-core -lwesnoth

THELIBS = -L. $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(SDL_NET_LIBS) \
	$(SDL_TTF_LIBS) $(SDL_LIBS) $(LIBINTL) \
	$(BOOST_IOSTREAMS_LIBS) $(BOOST_REGEX_LIBS)

wesnoth_source = \
	about.cpp \
	actions.cpp \
	addon_checks.cpp \
	addon_management.cpp \
	ai/actions.cpp \
	ai/composite/ai.cpp \
	ai/composite/aspect.cpp \
	ai/composite/component.cpp \
	ai/composite/contexts.cpp \
	ai/composite/engine.cpp \
	ai/composite/engine_default.cpp \
	ai/composite/engine_fai.cpp \
	ai/composite/engine_lua.cpp \
	ai/composite/goal.cpp \
	ai/composite/rca.cpp \
	ai/composite/stage.cpp \
	ai/configuration.cpp \
	ai/contexts.cpp \
	ai/default/ai.cpp \
	ai/default/attack.cpp \
	ai/default/contexts.cpp \
	ai/default/move.cpp \
	ai/default/village.cpp \
	ai/formula/ai.cpp \
	ai/formula/callable_objects.cpp \
	ai/formula/candidates.cpp \
	ai/formula/function_table.cpp \
	ai/formula/stage_rca_formulas.cpp \
	ai/formula/stage_side_formulas.cpp \
	ai/formula/stage_unit_formulas.cpp \
	ai/game_info.cpp \
	ai/gamestate_observer.cpp \
	ai/interface.cpp \
	ai/manager.cpp \
	ai/registry.cpp \
	ai/testing/aspect_attacks.cpp \
	ai/testing/ca.cpp \
	ai/testing/ca_default_move_to_targets.cpp \
	ai/testing/ca_testing_move_to_targets.cpp \
	ai/testing/stage_rca.cpp \
	ai/testing/stage_fallback.cpp \
	ai/testing.cpp \
	animated_game.cpp \
	attack_prediction.cpp \
	attack_prediction_display.cpp \
	callable_objects.cpp \
	config_cache.cpp \
	controller_base.cpp \
	dialogs.cpp \
	floating_textbox.cpp \
	formula.cpp \
	formula_debugger.cpp \
	formula_debugger_fwd.cpp \
	formula_function.cpp \
	formula_tokenizer.cpp \
	formula_string_utils.cpp \
	game_display.cpp \
	game_events.cpp \
	game_preferences.cpp \
	game_preferences_display.cpp \
	gamestatus.cpp \
	generate_report.cpp \
	generic_event.cpp \
	gui/auxiliary/canvas.cpp \
	gui/auxiliary/event/dispatcher.cpp \
	gui/auxiliary/event/distributor.cpp \
	gui/auxiliary/event/handler.cpp \
	gui/auxiliary/log.cpp \
	gui/auxiliary/timer.cpp \
	gui/auxiliary/widget_definition.cpp \
	gui/auxiliary/widget_definition/button.cpp \
	gui/auxiliary/widget_definition/horizontal_scrollbar.cpp \
	gui/auxiliary/widget_definition/image.cpp \
	gui/auxiliary/widget_definition/label.cpp \
	gui/auxiliary/widget_definition/listbox.cpp \
	gui/auxiliary/widget_definition/menubar.cpp \
	gui/auxiliary/widget_definition/minimap.cpp \
	gui/auxiliary/widget_definition/multi_page.cpp \
	gui/auxiliary/widget_definition/panel.cpp \
	gui/auxiliary/widget_definition/repeating_button.cpp \
	gui/auxiliary/widget_definition/scroll_label.cpp \
	gui/auxiliary/widget_definition/scrollbar_panel.cpp \
	gui/auxiliary/widget_definition/slider.cpp \
	gui/auxiliary/widget_definition/spacer.cpp \
	gui/auxiliary/widget_definition/stacked_widget.cpp \
	gui/auxiliary/widget_definition/text_box.cpp \
	gui/auxiliary/widget_definition/toggle_button.cpp \
	gui/auxiliary/widget_definition/toggle_panel.cpp \
	gui/auxiliary/widget_definition/tooltip.cpp \
	gui/auxiliary/widget_definition/tree_view.cpp \
	gui/auxiliary/widget_definition/vertical_scrollbar.cpp \
	gui/auxiliary/widget_definition/window.cpp \
	gui/auxiliary/window_builder/button.cpp \
	gui/auxiliary/window_builder/control.cpp \
	gui/auxiliary/window_builder/helper.cpp \
	gui/auxiliary/window_builder/horizontal_listbox.cpp \
	gui/auxiliary/window_builder/horizontal_scrollbar.cpp \
	gui/auxiliary/window_builder/image.cpp \
	gui/auxiliary/window_builder/label.cpp \
	gui/auxiliary/window_builder/listbox.cpp \
	gui/auxiliary/window_builder/minimap.cpp \
	gui/auxiliary/window_builder/menubar.cpp \
	gui/auxiliary/window_builder/multi_page.cpp \
	gui/auxiliary/window_builder/repeating_button.cpp \
	gui/auxiliary/window_builder/scroll_label.cpp \
	gui/auxiliary/window_builder/scrollbar_panel.cpp \
	gui/auxiliary/window_builder/slider.cpp \
	gui/auxiliary/window_builder/spacer.cpp \
	gui/auxiliary/window_builder/stacked_widget.cpp \
	gui/auxiliary/window_builder/text_box.cpp \
	gui/auxiliary/window_builder/toggle_button.cpp \
	gui/auxiliary/window_builder/vertical_scrollbar.cpp \
	gui/auxiliary/window_builder/panel.cpp \
	gui/auxiliary/window_builder/password_box.cpp \
	gui/auxiliary/window_builder/toggle_panel.cpp \
	gui/auxiliary/window_builder/tree_view.cpp \
	gui/auxiliary/window_builder.cpp \
	gui/dialogs/addon_connect.cpp \
	gui/dialogs/addon_list.cpp \
	gui/dialogs/campaign_selection.cpp \
	gui/dialogs/chat_log.cpp \
	gui/dialogs/dialog.cpp \
	gui/dialogs/formula_debugger.cpp \
	gui/dialogs/game_load.cpp \
	gui/dialogs/game_delete.cpp \
	gui/dialogs/game_save.cpp \
	gui/dialogs/gamestate_inspector.cpp \
	gui/dialogs/language_selection.cpp \
	gui/dialogs/lobby/lobby_data.cpp \
	gui/dialogs/lobby/lobby_info.cpp \
	gui/dialogs/lobby_main.cpp \
	gui/dialogs/lobby_player_info.cpp \
	gui/dialogs/message.cpp \
	gui/dialogs/mp_connect.cpp \
	gui/dialogs/mp_create_game.cpp \
	gui/dialogs/mp_method_selection.cpp \
	gui/dialogs/mp_cmd_wrapper.cpp \
	gui/dialogs/title_screen.cpp \
	gui/dialogs/transient_message.cpp \
	gui/dialogs/unit_create.cpp \
	gui/dialogs/wml_message.cpp \
	gui/widgets/button.cpp \
	gui/widgets/control.cpp \
	gui/widgets/container.cpp \
	gui/widgets/debug.cpp \
	gui/widgets/generator.cpp \
	gui/widgets/grid.cpp \
	gui/widgets/helper.cpp \
	gui/widgets/horizontal_scrollbar.cpp \
	gui/widgets/image.cpp \
	gui/widgets/label.cpp \
	gui/widgets/listbox.cpp \
	gui/widgets/menubar.cpp \
	gui/widgets/minimap.cpp \
	gui/widgets/multi_page.cpp \
	gui/widgets/panel.cpp \
	gui/widgets/password_box.cpp \
	gui/widgets/repeating_button.cpp \
	gui/widgets/settings.cpp \
	gui/widgets/scroll_label.cpp \
	gui/widgets/scrollbar.cpp \
	gui/widgets/scrollbar_container.cpp \
	gui/widgets/scrollbar_panel.cpp \
	gui/widgets/slider.cpp \
	gui/widgets/spacer.cpp \
	gui/widgets/stacked_widget.cpp \
	gui/widgets/text.cpp \
	gui/widgets/text_box.cpp \
	gui/widgets/toggle_button.cpp \
	gui/widgets/toggle_panel.cpp \
	gui/widgets/tooltip.cpp \
	gui/widgets/tree_view.cpp \
	gui/widgets/tree_view_node.cpp \
	gui/widgets/vertical_scrollbar.cpp \
	gui/widgets/widget.cpp \
	gui/widgets/window.cpp \
	halo.cpp \
	help.cpp \
	intro.cpp \
	lobby_preferences.cpp \
	leader_list.cpp \
	menu_events.cpp \
	mouse_events.cpp \
	mouse_handler_base.cpp \
	mp_game_settings.cpp \
	multiplayer.cpp \
	multiplayer_ui.cpp \
	multiplayer_wait.cpp \
	multiplayer_connect.cpp \
	multiplayer_create.cpp \
	multiplayer_lobby.cpp \
	network.cpp \
	network_worker.cpp \
	pathfind/pathfind.cpp \
	playcampaign.cpp \
	play_controller.cpp \
	playmp_controller.cpp \
	playsingle_controller.cpp \
	playturn.cpp \
	portrait.cpp \
	replay.cpp \
	replay_controller.cpp \
	resources.cpp \
	save_blocker.cpp \
	savegame.cpp \
	savegame_config.cpp \
	scripting/lua.cpp \
	sha1.cpp \
	settings.cpp \
	statistics.cpp \
	statistics_dialog.cpp \
	storyscreen/interface.cpp \
	storyscreen/controller.cpp \
	storyscreen/part.cpp \
	storyscreen/render.cpp \
	team.cpp \
	terrain_filter.cpp \
	text.cpp \
	time_of_day.cpp \
	titlescreen.cpp \
	tod_manager.cpp \
	tooltips.cpp \
	unit.cpp \
	unit_abilities.cpp \
	unit_animation.cpp \
	unit_display.cpp \
	unit_frame.cpp \
	unit_id.cpp \
	unit_map.cpp \
	unit_types.cpp \
	variable.cpp \
	variant.cpp \
	widgets/combo.cpp \
	widgets/combo_drag.cpp \
	widgets/drop_target.cpp \
	widgets/scrollpane.cpp


# used with editor option in the wesnoth target
wesnoth_editor_SOURCES = \
	gui/dialogs/editor_generate_map.cpp \
	gui/dialogs/editor_new_map.cpp \
	gui/dialogs/editor_resize_map.cpp \
	gui/dialogs/editor_settings.cpp \
	editor/action.cpp \
	editor/brush.cpp \
	editor/editor_main.cpp \
	editor/editor_controller.cpp \
	editor/editor_display.cpp \
	editor/editor_layout.cpp \
	editor/editor_map.cpp \
	editor/editor_palettes.cpp \
	editor/editor_preferences.cpp \
	editor/map_context.cpp \
	editor/map_fragment.cpp \
	editor/mouse_action.cpp

if EDITOR
wesnoth_source += $(wesnoth_editor_SOURCES)
endif

wesnoth_pool_alloc_SOURCES = \
	malloc.c \
	poolalloc.c

if POOLALLOC
wesnoth_source += $(wesnoth_pool_alloc_SOURCES)
endif

#############################################################################
#    Wesnoth                                                                #
#############################################################################

wesnoth_SOURCES = \
	game.cpp \
	$(wesnoth_source)

wesnoth_LDADD = $(INTERNALLIBS) $(THELIBS) $(PANGO_LIBS) $(FONTCONFIG_LIBS)
wesnoth_DEPENDENCIES=libwesnoth-core.a libwesnoth.a


#############################################################################
#    Wesnothd Server                                                        #
#############################################################################

wesnothd_SOURCES = \
	server/ban.cpp \
	server/game.cpp \
	server/input_stream.cpp \
	server/metrics.cpp \
	server/player.cpp \
	server/player_network.cpp \
	server/proxy.cpp \
	server/room.cpp \
	server/room_manager.cpp \
	server/server.cpp \
	server/simple_wml.cpp \
	server/user_handler.cpp \
	server/forum_user_handler.cpp \
	server/sample_user_handler.cpp \
	network.cpp \
	network_worker.cpp \
	loadscreen_empty.cpp


wesnothd_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBINTL)
wesnothd_DEPENDENCIES=libwesnoth-core.a

#############################################################################
#    Campaign Server                                                        #
#############################################################################

campaignd_SOURCES = \
	addon_checks.cpp \
	campaign_server/campaign_server.cpp \
	server/input_stream.cpp \
	network.cpp \
	network_worker.cpp \
	loadscreen_empty.cpp 

campaignd_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBINTL)
campaignd_DEPENDENCIES=libwesnoth-core.a

#############################################################################
#    Castle building helpers                                                #
#############################################################################

exploder_SOURCES = \
	tools/exploder.cpp \
	tools/exploder_utils.cpp \
	tools/exploder_cutter.cpp \
	tools/exploder_composer.cpp \
	tools/dummy_video.cpp \
	sdl_utils.cpp \
	loadscreen_empty.cpp 

cutter_SOURCES = \
	tools/cutter.cpp \
	tools/exploder_utils.cpp \
	tools/exploder_cutter.cpp \
	tools/dummy_video.cpp \
	sdl_utils.cpp \
	loadscreen_empty.cpp 

exploder_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBINTL) $(PNG_LIBS)
exploder_DEPENDENCIES=libwesnoth-core.a
cutter_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBINTL) $(PNG_LIBS)
cutter_DEPENDENCIES=libwesnoth-core.a

#############################################################################
#    Unit tests                                                             #
#############################################################################

test_SOURCES =  \
	tests/main.cpp \
	tests/utils/fake_display.cpp \
	tests/utils/fake_event_source.cpp \
	tests/utils/game_config_manager.cpp \
	tests/utils/play_scenario.cpp \
	tests/test_config_cache.cpp \
	tests/test_formula_ai.cpp \
	tests/test_lexical_cast.cpp	\
    tests/test_network_worker.cpp \
	tests/test_policy.cpp \
	tests/test_team.cpp \
	tests/test_util.cpp \
	tests/test_serialization.cpp \
	tests/test_version.cpp \
	tests/gui/test_drop_target.cpp \
	tests/gui/test_gui2.cpp \
	tests/gui/test_save_dialog.cpp \
	$(wesnoth_source)

test_LDADD = $(INTERNALLIBS) $(THELIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(PANGO_LIBS) $(FONTCONFIG_LIBS)
test_DEPENDENCIES=libwesnoth-core.a libwesnoth.a

#############################################################################
#    Headers                                                                #
#############################################################################

noinst_HEADERS = \
	wesconfig.h \
	animated.i \
	filesystem_win32.ii

#############################################################################
#    Headers
#############################################################################

#color_range.cpp should be removed, but game_config depends on it.
#game_config has very few things that are needed elsewhere, it should be
#removed.  Requires moving path and version at least to other files.

libwesnoth_core_a_SOURCES = \
	color_range.cpp \
	config.cpp \
	filesystem.cpp \
	game_config.cpp \
	gettext.cpp \
	hash.cpp \
	log.cpp \
	map.cpp \
	map_location.cpp \
	md5.cpp \
	thread.cpp \
	tstring.cpp \
	util.cpp \
	version.cpp \
	serialization/binary_or_text.cpp \
	serialization/binary_wml.cpp \
	serialization/parser.cpp \
	serialization/preprocessor.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp

libwesnoth_a_SOURCES = \
	builder.cpp \
	cavegen.cpp \
	clipboard.cpp \
	construct_dialog.cpp \
	cursor.cpp \
	display.cpp \
	events.cpp \
	filechooser.cpp \
	font.cpp \
	hotkeys.cpp \
	image.cpp \
	image_function.cpp \
	key.cpp \
	language.cpp \
	loadscreen.cpp \
	map_create.cpp \
	map_label.cpp \
	mapgen.cpp \
	mapgen_dialog.cpp \
	marked-up_text.cpp \
	minimap.cpp \
	pathfind/astarsearch.cpp \
	pathutils.cpp \
	preferences.cpp \
	preferences_display.cpp \
	race.cpp \
	random.cpp \
	reports.cpp \
	sdl_utils.cpp \
	show_dialog.cpp \
	sound.cpp \
	soundsource.cpp \
	sound_music_track.cpp \
	terrain.cpp \
	terrain_translation.cpp \
	time_of_day.cpp \
	video.cpp \
	theme.cpp \
	widgets/button.cpp \
	widgets/file_menu.cpp \
	widgets/label.cpp \
	widgets/menu.cpp \
	widgets/menu_style.cpp \
	widgets/progressbar.cpp \
	widgets/scrollarea.cpp \
	widgets/scrollbar.cpp \
	widgets/slider.cpp \
	widgets/textbox.cpp \
	widgets/widget.cpp \
	wml_exception.cpp 

# Until game_config.cpp is removed, this will be needed to ensure that the
# REVISION constant gets updated correctly.  If and when that definition is
# moved elsewhere, this dependency must follow it.
game_config.o: revision.hpp 

# Should be touched each time the SVN revision number goes up 
# Inclusion of this file should be guarded by HAVE_REVISION so checkouts 
# into a Windows build environment won't break.
REVISION = $(shell LC_ALL=C svnversion -n $(topdir) 2>/dev/null)
.PRECIOUS: revision.hpp
revision.hpp: FORCE
	if [ "$(REVISION)" = "" ] || [ "$(REVISION)" = "exported" ]; then echo '' >/tmp/westemp$$$$; \
	else echo '#define REVISION    "$(REVISION)"' >/tmp/westemp$$$$; fi;\
	if cmp -s revision.hpp /tmp/westemp$$$$ 2>/dev/null; then :; else cp /tmp/westemp$$$$ revision.hpp; fi; \
	if [ -e /tmp/westemp$$$$ ]; then rm /tmp/westemp$$$$; fi

FORCE:

AM_CXXFLAGS = -DHAVE_REVISION -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@  -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR) -DFIFODIR=\"$(fifodir)\" \
	-DWESNOTH_PREFIX=\"$(prefix)\"

AM_CFLAGS = -DHAVE_REVISION -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR)

if FRIBIDI
if OLD_FRIBIDI
    CXXFLAGS += -DOLD_FRIBIDI @FRIBIDI_CFLAGS@
    CFLAGS += -DOLD_FRIBIDI @FRIBIDI_CFLAGS@
    THELIBS += @FRIBIDI_LIBS@
else
    CXXFLAGS += -DHAVE_FRIBIDI @FRIBIDI2_CFLAGS@
    CFLAGS += -DHAVE_FRIBIDI @FRIBIDI2_CFLAGS@
    THELIBS += @FRIBIDI2_LIBS@
endif
endif

if X11
    CXXFLAGS += -D_X11 @X_CFLAGS@
    CFLAGS += -D_X11 @X_CFLAGS@
endif
if GCC
    CXXFLAGS += -fno-omit-frame-pointer
    CFLAGS += -fno-omit-frame-pointer
endif
if PREFSDIR
    CXXFLAGS += -DPREFERENCES_DIR=\"$(prefsdir)\"
endif

if BOOST_TEST_DYN_LINK
    CXXFLAGS += -DBOOST_TEST_DYN_LINK
endif

CXXLD = $(LDPREFIX) $(CXX)
CXXFLAGS += $(BOOST_CPPFLAGS)


if STATIC
LDFLAGS += -all-static
endif

LDFLAGS += $(BOOST_IOSTREAMS_LDFLAGS)  $(BOOST_REGEX_LDFLAGS)


#if STATIC
#LIBS += -static -L/usr/lib -Wl,-rpath,/usr/lib -L/usr/X11R6/lib -lSDL_image \
#-lSDL_mixer -lSDL_net -lSDL_ttf -lSDL -lpthread -ldl -laudio -lXt -lX11 -lXext \
#-lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lstdc++ -lm -lSM -lICE -ljpeg \
#-lpng -lz -lvorbisfile -lvorbis -logg -lsmpeg -lfreetype
#endif

if SERVER
install-exec-hook:
	mkdir -p -m 0700 $(DESTDIR)$(fifodir)
	-chown $(serveruid):$(servergid) $(DESTDIR)$(fifodir)

uninstall-hook:
	rm -rf $(DESTDIR)$(fifodir)
else
install-exec-hook:
uninstall-hook:
endif

%.tpp: %.hpp
	$(CXXCOMPILE) -fsyntax-only $<

#############################################################################
#    Tag support
#############################################################################

CLEANFILES += TAGS

TAGS:
	etags -l c++ $(wesnoth_SOURCES) $(wesnothd_SOURCES) $(campaignd_SOURCES) $(exploder_SOURCES) $(cutter_SOURCES) $(libwesnoth_core_a_SOURCES) $(libwesnoth__a_SOURCES) 

#############################################################################
#    Scripts
#############################################################################

$(bin_SCRIPTS):
	cp ../data/tools/$@ . 
