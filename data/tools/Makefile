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

sanity-check: unresolved lint images-unused

lint:
	# Check for obsolete WML, broken references, consistency.
	./wmllint --dryrun --nolift $(MAINLINE) $(UMC)

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
# no references.  2nd option covers stuff known to be used in UMC.
FORCE_USED = --force-used "terrain/" \
	--force-used "portraits/|images/|attacks/|projectiles/|windmill"
images-unused:
	@echo "# Report on unused images"
	@./wmlscope --crossreference --from images --refcount 0 $(FORCE_USED) $(MAINLINE) $(UMC)

all-unused:
	@echo "# Report on unused resource files and macros"
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
	@./wmlscope --extracthelp --exclude data/scenarios --exclude data/tutorial --exclude data/campaigns $(DATA)/core/macros/ >>macro-reference.xhtml
	@cat helptrailer.xhtml >>macro-reference.xhtml

# Report on unchecked macro calls
unchecked:
	@./wmlscope --unchecked $(MAINLINE) $(UMC)

# Reindent the mainline content
reindent:
	@./wmlindent --exclude=../../data/languages $(DATA)

# Extract a table of editor keybindings from the editor theme definition
editor_bindings:
	@./extractbindings <../themes/editor.cfg 

# For performing test lifts of mainline
lift:
	./wmllint --oldversion 1.3.1 --verbose $(MAINLINE) $(UMC)
trylift:
	./wmllint --oldversion 1.3.1 --dryrun --verbose $(MAINLINE) $(UMC)
diffs:
	./wmllint --diffs $(MAINLINE) $(UMC)
revert:
	./wmllint --revert $(MAINLINE) $(UMC)
clean:
	./wmllint --clean $(MAINLINE) $(UMC)

