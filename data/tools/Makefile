# Recipes for various sanity checks.

utils-unused:
	@echo "Report on unused utility macros"
	cd ../..; data/tools/macroscope --crossreference --from data/utils --refcount 0 data

utils-macros:
	@echo "Report on usage of utility macros"
	cd ../..; data/tools/macroscope --crossreference --from data/utils data

all-macros:
	@echo "Report on usage of all macros"
	cd ../..; data/tools/macroscope --crossreference data

unresolved-macros:
	@echo "Report on unresolved macro calls"
	cd ../..; data/tools/macroscope --unresolved data
