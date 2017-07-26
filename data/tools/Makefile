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
#UMC = ../../../../wesnoth-umc-dev/trunk/*

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

validate:
	# Check WML for meaning and references.
	# No UMC testing until $$(UMC) is fixed
	./wmlvalidator -p $(DATA) #-u $(UMC)

validate-gui:
	# Check GUI WML for meaning and references.
	./wmlvalidator -p $(DATA) -s $(DATA)/schema-gui.cfg $(DATA)/gui/default.cfg

# Beware of removing images listed in this report unless you know
# UMC is not relying on them!
#
# References to terrain graphics go through so many layers of nasty
# opaque macros that trying to reference-check them is hopeless.
# So suppress reporting them unused even if they seem to have
# no references.  
FORCE_USED = --force-used "terrain/" 
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
macro-reference.html: helpheader.html helptrailer.html wmlscope
	@cat helpheader.html >macro-reference.html
	@./wmlscope --extracthelp --exclude data/scenarios --exclude data/tutorial --exclude data/campaigns $(DATA)/core/macros/ >>macro-reference.html
	@cat helptrailer.html >>macro-reference.html

# Report on unchecked macro calls
unchecked:
	@./wmlscope --unchecked $(MAINLINE) $(UMC)

# Reindent the mainline content
reindent:
	@./wmlindent --exclude=../../data/languages --exclude=../../data/gui --exclude=../../data/test $(DATA)

# Extract a table of editor keybindings from the editor theme definition
editor_bindings:
	@./extractbindings <../themes/editor.cfg 

# Look at the difficulty-rank order of the campaigns
rank:
	 @grep rank ../campaigns/*/_main.cfg | sed -n \
		-e '/rank=/s///' \
		-e '/..\/campaigns\//s///' \
		-e '/\/_main.cfg:/s///p' \
		| sort -n -k 2 \
		| awk '{print $$2, "\t", $$1}'

# For performing test lifts of mainline
lift:
	./wmllint --verbose $(MAINLINE) $(UMC)
trylift:
	./wmllint --dryrun --verbose $(MAINLINE) $(UMC)
diffs:
	./wmllint --diffs $(MAINLINE) $(UMC)
revert:
	./wmllint --revert $(MAINLINE) $(UMC)
clean:
	./wmllint --clean $(MAINLINE) $(UMC)
	rm -f macro-reference.html

