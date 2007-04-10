# Recipes for various sanity checks.
#
# Hack this if our location in the Wesnoth source tree changes
TOPDIR = ../..

unresolved:
	@echo "# Report on unresolved macro calls"
	@macroscope --unresolved $(TOPDIR)

all:
	@echo "# Report on usage of all macros and resources"
	@macroscope --crossreference $(TOPDIR)

utils-unused:
	@echo "# Report on unused utility macros"
	@macroscope --crossreference --from data/utils --refcount 0  $(TOPDIR)

utils-macros:
	@echo "# Report on usage of utility macros"
	@macroscope --crossreference --from data/utils $(TOPDIR)


