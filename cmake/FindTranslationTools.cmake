# - Find the tools needed for updating the potfiles and translations

set(TRANSLATION_TOOLS_FOUND true)

find_program(GETTEXT_MSGINIT_EXECUTABLE msginit)
if(NOT GETTEXT_MSGINIT_EXECUTABLE)
	message("msginit not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_MSGINIT_EXECUTABLE)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE)
	message("xgettext not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE)
set(GETTEXT_XGETTEXT_OPTIONS
	--force-po
	--add-comments=TRANSLATORS 
	--copyright-holder=\"Wesnoth development team\"
	--msgid-bugs-address=\"http://bugs.wesnoth.org/\"
	--from-code=UTF-8
	--sort-by-file
	--keyword=_
	--keyword=N_
	--keyword=sgettext
	--keyword=vgettext
	--keyword=VGETTEXT
	--keyword=_n:1,2
	--keyword=sngettext:1,2
	--keyword=vngettext:1,2
)

find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTABLE )
	message("msgcat not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_MSGCAT_EXECUTABLE )

find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
	message("msgattrib not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT GETTEXT_MSGATTRIB_EXECUTABLE)

find_program(ASCIIDOC_EXECUTABLE asciidoc)
set(ASCIIDOC_OPTIONS
	-b docbook
	-d book
	-n
	-a toc
)
if(NOT ASCIIDOC_EXECUTABLE)
	message("asciidoc not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT ASCIIDOC_EXECUTABLE)

find_program(DOS2UNIX_EXECUTABLE dos2unix)
if(NOT DOS2UNIX_EXECUTABLE)
	message("dos2unix not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT DOS2UNIX_EXECUTABLE)

find_program(PO4A-TRANSLATE_EXECUTABLE po4a-translate)
set(PO4A-TRANSLATE_OPTIONS
	-f docbook
	-k 80
	-M utf-8
	-L utf-8
)
if(NOT PO4A-TRANSLATE_EXECUTABLE)
	message("po4a-translate not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT PO4A-TRANSLATE_EXECUTABLE)

find_program(PO4A-UPDATEPO_EXECUTABLE po4a-updatepo)
set(PO4A-UPDATEPO_OPTIONS
	-M utf-8
)
if(NOT PO4A-UPDATEPO_EXECUTABLE)
	message("po4a-updatepo not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT PO4A-UPDATEPO_EXECUTABLE)

find_program(PO4A-GETTEXTIZE_EXECUTABLE po4a-gettextize)
set(PO4A-GETTEXTIZE_OPTIONS
	--copyright-holder "Wesnoth Development Team"
	-f docbook
	-M utf-8
	-L utf-8
)
if(NOT PO4A-GETTEXTIZE_EXECUTABLE)
	message("po4a-gettextize not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT PO4A-GETTEXTIZE_EXECUTABLE)

find_program(XSLTPROC_EXECUTABLE xsltproc)
set(XSLTPROC_OPTIONS
	--nonet
	--stringparam callout.graphics 0
	--stringparam navig.graphics 0
	--stringparam admon.textlabel 1
	--stringparam admon.graphics 0
	--stringparam html.stylesheet "./styles/manual.css"
)
if(NOT XSLTPROC_EXECUTABLE)
	message("xsltproc not found")
	set(TRANSLATION_TOOLS_FOUND false)
endif(NOT XSLTPROC_EXECUTABLE)

if(NOT TRANSLATION_TOOLS_FOUND)
	if(TranslationTools_FIND_REQUIRED)
		message(FATAL_ERROR "Not all translation tools are found")
	endif(TranslationTools_FIND_REQUIRED)
endif(NOT TRANSLATION_TOOLS_FOUND)

