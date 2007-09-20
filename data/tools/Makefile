# Recipes for various sanity checks.
#
# Hack this if our location in the Wesnoth source tree changes.
# It needs to point to the top-level directory, e.g. with data/images/sounds
# directly beneath it. 
TOPDIR = ../../data/core ../../data/{multiplayer,themes} ../../data/campaigns/*
# Define this to point to a local copy of UMC to run checks on.
# Note, this needs to expand to a list with one subdirectory per UMC campaign,
# otherwise the visibility checks aren't going to work right.
# UMC = ../../../umc/*

sanity-check: unresolved check-obsolete

check-obsolete:
	# Check for obsolete WML
	./wmllint --dryrun $(TOPDIR) $(UMC)

unresolved:
	@echo "# Report on unresolved macro calls and resource references"
	@./wmlscope --unresolved $(TOPDIR) $(UMC)

all:
	@echo "# Report on usage of all macros and resources"
	@./wmlscope --crossreference $(TOPDIR) $(UMC)

utils-unused:
	@echo "# Report on unused utility macros"
	@./wmlscope --crossreference --from $(TOPDIR)data/utils --refcount 0 $(FAKE) $(TOPDIR) $(UMC)

# References to terrain graphics go through so many layers of nasty
# opaque macros that trying to reference-check them is hopeless.
# So suppress reporting them unused even if they seem to have
# no references.
all-unused:
	@echo "# Report on unused resource files"
	@./wmlscope --crossreference --refcount 0 --force-used "terrain/" $(TOPDIR) $(UMC)

utils-macros:
	@echo "# Report on usage of utility macros"
	@./wmlscope --crossreference --from $(TOPDIR)data/utils $(TOPDIR) $(UMC)

collisions:
	@echo "# Report on duplicate resource files."
	@./wmlscope --collisions $(TOPDIR) $(UMC)

# Report a list of symbols and resources defined in mainline.
definitions:
	@./wmlscope --definitions --exclude data/scenarios --exclude data/campaigns $(TOPDIR)

# Generate a reference page for the utility macros.
macro-reference.xhtml:
	@cat helpheader.xhtml >macro-reference.xhtml
	@./wmlscope --extracthelp --exclude data/scenarios --exclude data/tutorial --exclude data/campaigns $(TOPDIR) >>macro-reference.xhtml
	@cat helptrailer.xhtml >>macro-reference.xhtml

