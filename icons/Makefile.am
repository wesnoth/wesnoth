EXTRA_DIST = wesnoth.desktop wesnoth-icon.png wesnoth_editor.desktop wesnoth_editor-icon.png

if GAME_DESKTOP_ENTRY
if EDITOR
appentrydir = @APP_ENTRY@
dist_appentry_DATA = wesnoth.desktop wesnoth_editor.desktop
appicondir = @APP_ICON@
appicon_DATA = wesnoth-icon.png wesnoth_editor-icon.png
else
appentrydir = @APP_ENTRY@
dist_appentry_DATA = wesnoth.desktop
appicondir = @APP_ICON@
appicon_DATA = wesnoth-icon.png
endif
endif

