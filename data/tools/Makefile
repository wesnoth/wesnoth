# Recipes for various sanity checks.
#
# Hack this if our location in the Wesnoth source tree changes
TOPDIR = ../..
# References to terrain graphics go through so many layers of nasty
# opaque macros that trying to reference-check them is hopeless.
# So suppress reporting them unused even if they seem to have
# no references.
FAKE = --force-used "(terrain|buttons|cursors|cursors-bw|menu|icons)/"
# Suppress meaningless errors about castle graphics and about resources 
# used only by the GUI code in C++
EXCLUDE = --exclude "(exploder|castle-cutter|UI)"

unresolved:
	@echo "# Report on unresolved macro calls and resource references"
	@macroscope --unresolved $(EXCLUDE) $(TOPDIR)

all:
	@echo "# Report on usage of all macros and resources"
	@macroscope --crossreference $(EXCLUDE) $(TOPDIR)

utils-unused:
	@echo "# Report on unused utility macros"
	@macroscope --crossreference --from data/utils --refcount 0 $(FAKE) $(EXCLUDE) $(TOPDIR)

all-unused:
	@echo "# Report on unused resource files"
	@macroscope --crossreference --refcount 0 $(FAKE) $(EXCLUDE) $(TOPDIR)

utils-macros:
	@echo "# Report on usage of utility macros"
	@macroscope --crossreference --from data/utils $(EXCLUDE) $(TOPDIR)

definitions:
	@macroscope --definitions --exclude data/scenarios --exclude data/campaigns $(EXCLUDE) $(TOPDIR)


