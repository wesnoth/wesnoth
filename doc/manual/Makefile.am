if MANUALUPDATE
dist_noinst_DATA = styles/manual.css

XSLTOPTS = --nonet \
  --stringparam callout.graphics 0 \
  --stringparam navig.graphics 0 \
  --stringparam admon.textlabel 1 \
  --stringparam admon.graphics 0 \
  --stringparam html.stylesheet "./styles/manual.css"

PO4AOPTS = --no-backups --copyright-holder "Wesnoth Development Team"

manual.en.xml: manual.txt
	$(ASCIIDOC) -b docbook -d book -n -a toc -o manual.en.xml manual.txt
	$(DOS2UNIX) manual.en.xml

update-po: manual.en.xml
	( cd ../../po/wesnoth-manual && \
	$(PO4A) $(PO4AOPTS) --no-translations wesnoth-manual.cfg || exit $? )

update-po4a: manual.en.xml
	( cd ../../po/wesnoth-manual && \
	  $(PO4A) $(PO4AOPTS) wesnoth-manual.cfg || exit $? )

html: update-po4a
	@for i in manual.*.xml; do \
		lang=$$(echo $$i | sed -e 's/manual\.\([a-zA-Z_@]*\)\.xml/\1/'); \
		echo "Generating manual from manual.$$lang.xml..."; \
		rm -f manual.$$lang.html; \
		$(XSLTPROC) $(XSLTOPTS) "/etc/asciidoc/docbook-xsl/xhtml.xsl" \
		            "manual.$$lang.xml" > "manual.$$lang.html"; \
	done

dist-hook: html

clean-local:
	rm -f *.xml
endif
