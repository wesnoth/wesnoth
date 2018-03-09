file(STRINGS ${PROJECT_SOURCE_DIR}/po/LINGUAS LANGUAGES)
separate_arguments(LANGUAGES)

find_program(PO4A "po4a")

set(PO4A_options --rm-backups --copyright-holder "Wesnoth Development Team")

foreach(LANG ${LANGUAGES})
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${LANG}/wesnoth.6)
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${LANG}/wesnothd.6)
endforeach(LANG ${LANGUAGES})


execute_process(COMMAND ${PO4A}
		${PO4A_options}
		wesnoth-manpages.cfg
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po/wesnoth-manpages)
