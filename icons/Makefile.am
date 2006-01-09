EXTRA_DIST = wesnoth.desktop

if GNOME1
gnome1desktopdir = @GNOME1_DESKTOP@
dist_gnome1desktop_DATA = wesnoth.desktop
gnome1icondir = $(datadir)/icons
gnome1icon_DATA = wesnoth-icon.png
endif

if GNOME2
gnome2desktopdir = $(datadir)/applications
dist_gnome2desktop_DATA = wesnoth.desktop 
gnome2icondir = $(datadir)/icons
gnome2icon_DATA = wesnoth-icon.png
endif

if KDE
kdedesktopdir = @KDE_DESKTOP@
dist_kdedesktop_DATA = wesnoth.desktop
kdeicondir = @KDE_ICON@
kdeicon_DATA = wesnoth-icon.png
endif
