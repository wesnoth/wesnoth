INCLUDE(TranslationTargets)
set(XGETTEXT_OPTIONS "--from-code=UTF-8 --sort-by-file --keyword=sgettext --keyword=vgettext --keyword=_n:1,2 --keyword=sngettext:1,2 --keyword=vngettext:1,2")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pot-update.sh.in ${CMAKE_CURRENT_BINARY_DIR}/pot-update.sh)

if(ENABLE_POT_UPDATE_TARGET)
	add_custom_target(update-po4a)
endif(ENABLE_POT_UPDATE_TARGET)
add_custom_target(update-pot)
add_custom_target(update-po)
add_custom_target(update-gmo)

set(TRANSLATION_DIRS
    wesnoth
    wesnoth-anl
    wesnoth-aoi
    wesnoth-did
    wesnoth-editor
    wesnoth-ei
    wesnoth-httt
    wesnoth-l
    wesnoth-lib
    wesnoth-low
    wesnoth-manpages
    wesnoth-manual
    wesnoth-multiplayer
    wesnoth-nr
    wesnoth-sof
    wesnoth-sotbe
    wesnoth-tb
    wesnoth-test
    wesnoth-thot
    wesnoth-trow
    wesnoth-tsg
    wesnoth-tutorial
    wesnoth-units
    wesnoth-utbs
)

foreach(DIR ${TRANSLATION_DIRS})
  add_subdirectory(${DIR})
endforeach(DIR ${TRANSLATION_DIRS})
