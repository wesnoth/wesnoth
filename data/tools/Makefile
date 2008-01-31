# Recipes for various sanity checks.
#
# Hack DATA if our location in the Wesnoth source tree changes.
# It needs to point to the top-level data directory.
DATA = ../../data
#
# The MAINLINE definition should expand to all directories considered  
# to be part of the mainline distribution.
MAINLINE = $(DATA)/core $(DATA)/multiplayer $(DATA)/themes $(DATA)/campaigns/*
#
# Define UMC to point to a local copy of user-made content to run checks on.
# Note, this needs to expand to a list with one subdirectory per UMC campaign,
# otherwise the visibility checks aren't going to work right.
# UMC = ../../../umc*/*

sanity-check: unresolved lint

lint:
	# Check for obsolete WML, broken references, consistency.
	./wmllint --dryrun $(MAINLINE) $(UMC)

unresolved:
	@echo "# Report on unresolved macro calls and resource references"
	@./wmlscope --unresolved $(MAINLINE) $(UMC)

all:
	@echo "# Report on usage of all macros and resources"
	@./wmlscope --crossreference $(MAINLINE) $(UMC)

utils-unused:
	@echo "# Report on unused utility macros"
	@./wmlscope --crossreference --from $(DATA)/core/macros --refcount 0 $(MAINLINE) $(UMC)

# References to terrain graphics go through so many layers of nasty
# opaque macros that trying to reference-check them is hopeless.
# So suppress reporting them unused even if they seem to have
# no references.
all-unused:
	@echo "# Report on unused resource files"
	@./wmlscope --crossreference --refcount 0 --force-used "terrain/" $(MAINLINE) $(UMC)

utils-macros:
	@echo "# Report on usage of utility macros"
	@./wmlscope --crossreference --from $(DATA)/core/macros $(MAINLINE) $(UMC)

collisions:
	@echo "# Report on duplicate resource files."
	@./wmlscope --collisions $(MAINLINE) $(UMC)

# Report a list of symbols and resources defined in mainline.
definitions:
	@./wmlscope --definitions --exclude data/scenarios --exclude data/campaigns $(MAINLINE)

# Generate a reference page for the utility macros.
macro-reference.xhtml:
	@cat helpheader.xhtml >macro-reference.xhtml
	@./wmlscope --extracthelp --exclude data/scenarios --exclude data/tutorial --exclude data/campaigns $(MAINLINE) >>macro-reference.xhtml
	@cat helptrailer.xhtml >>macro-reference.xhtml

# Reindent the mainline content
reindent:
	wmlindent --exclude=../../data/languages $(DATA)
