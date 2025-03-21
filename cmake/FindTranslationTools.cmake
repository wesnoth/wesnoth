#[=======================================================================[.rst:
FindTranslationTools
--------------------

Find the tools needed for updating the potfiles and translations.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

	GETTEXT_MSGINIT_EXECUTABLE
	GETTEXT_XGETTEXT_EXECUTABLE
	GETTEXT_MSGCAT_EXECUTABLE
	GETTEXT_MSGATTRIB_EXECUTABLE
	ASCIIDOC_EXECUTABLE
	DOS2UNIX_EXECUTABLE
	PO4A-TRANSLATE_EXECUTABLE
	PO4A-UPDATEPO_EXECUTABLE
	XSLTPROC_EXECUTABLE

#]=======================================================================]

set(TRANSLATION_TOOLS_FOUND true)

# Try to find the given EXECUTABLE_NAME and set FIND_VARIABLE to its location
# Sets TRANSLATION_TOOLS_FOUND to false if it can't find the required exe
macro(_find_translation_tool FIND_VARIABLE EXECUTABLE_NAME)
	find_program(${FIND_VARIABLE} ${EXECUTABLE_NAME})
	if(NOT ${FIND_VARIABLE})
		message("${EXECUTABLE_NAME} not found!")
		set(TRANSLATION_TOOLS_FOUND false)
	endif()
endmacro()

_find_translation_tool(GETTEXT_MSGINIT_EXECUTABLE msginit)

_find_translation_tool(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
set(GETTEXT_XGETTEXT_OPTIONS
	--force-po
	--add-comments=TRANSLATORS
	--copyright-holder=\"Wesnoth development team\"
	--msgid-bugs-address=\"https://bugs.wesnoth.org/\"
	--from-code=UTF-8
	--sort-by-file
	--keyword=_
	--keyword=N_
	--keyword=sgettext
	--keyword=vgettext
	--keyword=VGETTEXT
	--keyword=_n:1,2
	--keyword=N_n:1,2
	--keyword=sngettext:1,2
	--keyword=vngettext:1,2
	--keyword=VNGETTEXT:1,2
)

_find_translation_tool(GETTEXT_MSGCAT_EXECUTABLE msgcat)

_find_translation_tool(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)

_find_translation_tool(ASCIIDOC_EXECUTABLE asciidoc)
set(ASCIIDOC_OPTIONS
	-b docbook
	-d book
	-n
	-a toc
)

_find_translation_tool(DOS2UNIX_EXECUTABLE dos2unix)

_find_translation_tool(PO4A-TRANSLATE_EXECUTABLE po4a-translate)
set(PO4A-TRANSLATE_OPTIONS
	-f docbook
	-k 80
	-M utf-8
	-L utf-8
)

_find_translation_tool(PO4A-UPDATEPO_EXECUTABLE po4a-updatepo)
set(PO4A-UPDATEPO_OPTIONS
	-M utf-8
)

_find_translation_tool(XSLTPROC_EXECUTABLE xsltproc)
set(XSLTPROC_OPTIONS
	--nonet
	--stringparam callout.graphics 0
	--stringparam navig.graphics 0
	--stringparam admon.textlabel 1
	--stringparam admon.graphics 0
	--stringparam html.stylesheet "./styles/manual.css"
)

# added a hack to find asciidoc when using latest archlinux by searching inside the python site packages...
find_path(ASCIIDOC_DOCBOOK_XSL_PATH
	xhtml.xsl
	HINTS /usr/share/asciidoc/docbook-xsl /etc/asciidoc/docbook-xsl /opt/local/etc/asciidoc/docbook-xsl /usr/lib/*/site-packages/asciidoc/resources/docbook-xsl
	NO_DEFAULT_PATH
)
if(NOT ASCIIDOC_DOCBOOK_XSL_PATH)
	message(STATUS "asciidoc DocBook XSL path not found!")
	set(TRANSLATION_TOOLS_FOUND false)
endif()

if(NOT TRANSLATION_TOOLS_FOUND AND TranslationTools_FIND_REQUIRED)
	message(FATAL_ERROR "Some required translation tools are not found!")
endif()
