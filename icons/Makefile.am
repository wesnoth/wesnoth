EXTRA_DIST = wesnoth.desktop wesnoth-icon.png

if GNOME1
if HAS_DESKTOPDIR
gnome1desktopdir = @DESKTOPDIR@
else
gnome1desktopdir = @GNOME1_DESKTOP@
endif
dist_gnome1desktop_DATA = wesnoth.desktop
if HAS_ICONDIR
gnome1icondir = @ICONDIR@
else
gnome1icondir = $(datadir)/icons
endif
gnome1icon_DATA = wesnoth-icon.png
endif

if GNOME2
if HAS_DESKTOPDIR
gnome2desktopdir = @DESKTOPDIR@
else
gnome2desktopdir = $(datadir)/applications
endif
dist_gnome2desktop_DATA = wesnoth.desktop 
if HAS_ICONDIR
gnome2icondir = @ICONDIR@
else
gnome2icondir = $(datadir)/icons
endif
gnome2icon_DATA = wesnoth-icon.png
endif

if KDE
if HAS_DESKTOPDIR
kdedesktopdir = @DESKTOPDIR@
else
kdedesktopdir = @KDE_DESKTOP@
endif
dist_kdedesktop_DATA = wesnoth.desktop
if HAS_ICONDIR
kdeicondir = @ICONDIR@
else
kdeicondir = @KDE_ICON@
endif
kdeicon_DATA = wesnoth-icon.png
endif
