# Recipes for various sanity checks.
#
# Hack this if our location in the Wesnoth source tree changes.
# It needs to point to the top-level directory, e.g. with data/images/sounds
# directly beneath it. 
TOPDIR = ../../
# Define this to point to a local copy of UMC to run checks on.
# Note, this needs to expand to a list with one subdirectory per UMC campaign,
# otherwise the visibility checks aren't going to work right.
# UMC = ../../../umc/*
# References to terrain graphics go through so many layers of nasty
# opaque macros that trying to reference-check them is hopeless.
# So suppress reporting them unused even if they seem to have
# no references.  Likewise for resources used only by the C++ GUI.
FAKE = --force-used "(terrain|buttons|cursors|cursors-bw|menu|icons)/"
# Suppress meaningless errors about castle graphics and about resources 
# used only by the GUI code in C++
# FIXME: This also suppresses all checking of the tutorial files.
# Otherwise we get warnings about duplicated resource files that are 
# copies of stuff in HttT.  It's not clear what the right permanent fix is.
EXCLUDE = --exclude ".dirstamp|po/|translations/|exploder/|castle-cutter/|UI/|tutorial/"

unresolved:
	@echo "# Report on unresolved macro calls and resource references"
	@./macroscope --unresolved $(EXCLUDE) $(TOPDIR) $(UMC)

all:
	@echo "# Report on usage of all macros and resources"
	@./macroscope --crossreference $(EXCLUDE) $(TOPDIR) $(UMC)

utils-unused:
	@echo "# Report on unused utility macros"
	@./macroscope --crossreference --from $(TOPDIR)data/utils --refcount 0 $(FAKE) $(EXCLUDE) $(TOPDIR) $(UMC)

all-unused:
	@echo "# Report on unused resource files"
	@./macroscope --crossreference --refcount 0 $(FAKE) $(EXCLUDE) $(TOPDIR) $(UMC)

utils-macros:
	@echo "# Report on usage of utility macros"
	@./macroscope --crossreference --from $(TOPDIR)data/utils $(EXCLUDE) $(TOPDIR) $(UMC)

collisions:
	@echo "# Report on duplicate resource files."
	@./macroscope --collisions $(EXCLUDE) $(TOPDIR) $(UMC)

# Report a list of symbols and resources defined in mainline.
definitions:
	@./macroscope --definitions --exclude data/scenarios --exclude data/campaigns $(EXCLUDE) $(TOPDIR)

# Generate a reference page for the utility macros.
macro-reference.xhtml:
	@cat helpheader.xhtml >macro-reference.xhtml
	@./macroscope --extracthelp --exclude data/scenarios --exclude data/tutorial --exclude data/campaigns $(EXCLUDE) $(TOPDIR) >>macro-reference.xhtml
	@cat helptrailer.xhtml >>macro-reference.xhtml

