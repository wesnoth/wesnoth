LANGUAGES = en:English fr:Français lt:Lietuvių nl:Nederlands pl:Polski ru:Russkij sk:Slovenčina tl:Filipino tr:Türkçe

PO4A_OPTIONS = --rm-backups --copyright-holder "Wesnoth Development Team"

default: translations

index.en.html: template.html
	cp template.html index.html.en

translations: index.en.html
	@( cd po && po4a $(PO4A_OPTIONS) wesnoth-start-1.4.cfg || exit $? )
	@LANGS=$$(for lang in $(LANGUAGES); do \
		code=`echo $$lang | cut -d: -f 1`; \
		name=`echo $$lang | cut -d: -f 2`; \
		echo -n "<li><a href='index.html.$$code'>$$name</a></li>"; \
	done) && \
	for i in index.html.*; do \
		sed -i -e "/<!--LANGS-->./d;s,<!--LANGS-->,<!--LANGS-->$$LANGS\n<!--LANGS-->," $$i; \
	done

update-po:
	@( cd po && po4a $(PO4A_OPTIONS) --no-translations wesnoth-start-1.4.cfg || \
	   exit $? )

clean:
	rm -f index.html.*
