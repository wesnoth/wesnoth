#
# Generates the documents using pdflatex.
#

if(ENABLE_DESIGN_DOCUMENTS)
	include(UseLATEX)
	# Abort compilation on errors.
	# Failing to do so might lead to infinite loops and hard to diagnose
	# problems.
	set(LATEX_COMPILER_FLAGS "")
	set(PDFLATEX_COMPILER_FLAGS "")

	ADD_LATEX_DOCUMENT(
		gui2.tex
	INPUTS
		gui2/progress_bar.cpp
		gui2/unit_attack.cpp
		gui2/widget_definition.cpp
		gui2/window_builder.cpp
		gui2/progress_bar.cfg
		gui2/progress_bar.hpp
		gui2/unit_attack.cfg
		gui2/unit_attack.hpp
		gui2/widget_definition.hpp
		gui2/window_builder.hpp
	DEFAULT_PDF
	MANGLE_TARGET_NAMES
	)
endif(ENABLE_DESIGN_DOCUMENTS)

