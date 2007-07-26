EXTRA_DIST = wesnoth.desktop wesnoth-icon.png wesnoth_editor.desktop wesnoth_editor-icon.png

if GNOME1
if EDITOR
gnome1desktopdir = @GNOME1_DESKTOP@
dist_gnome1desktop_DATA = wesnoth.desktop wesnoth_editor.desktop
gnome1icondir = @GNOME_ICON@
gnome1icon_DATA = wesnoth-icon.png wesnoth_editor-icon.png
else
gnome1desktopdir = @GNOME1_DESKTOP@
dist_gnome1desktop_DATA = wesnoth.desktop
gnome1icondir = @GNOME_ICON@
gnome1icon_DATA = wesnoth-icon.png
endif
endif

if GNOME2
if EDITOR
gnome2desktopdir = @GNOME2_DESKTOP@
dist_gnome2desktop_DATA = wesnoth.desktop wesnoth_editor.desktop
gnome2icondir = @GNOME_ICON@
gnome2icon_DATA = wesnoth-icon.png wesnoth_editor-icon.png
else
gnome2desktopdir = @GNOME2_DESKTOP@
dist_gnome2desktop_DATA = wesnoth.desktop 
gnome2icondir = @GNOME_ICON@
gnome2icon_DATA = wesnoth-icon.png
endif
endif

if KDE
if EDITOR
kdedesktopdir = @KDE_DESKTOP@
dist_kdedesktop_DATA = wesnoth.desktop wesnoth_editor.desktop
kdeicondir = @KDE_ICON@
kdeicon_DATA = wesnoth-icon.png wesnoth_editor-icon.png
else
kdedesktopdir = @KDE_DESKTOP@
dist_kdedesktop_DATA = wesnoth.desktop
kdeicondir = @KDE_ICON@
kdeicon_DATA = wesnoth-icon.png
endif
endif
