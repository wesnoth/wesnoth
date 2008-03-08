LANGUAGES = ca:Català cs:Čeština da:Dansk de:Deutsch en:English \
            es:Español fr:Français hu:Magyar it:Italiano ja:日本語 \
            lt:Lietuvių nl:Nederlands pl:Polski ru:Russkij \
            sk:Slovenčina tl:Filipino tur:Türkçe

PO4A_OPTIONS = --rm-backups --copyright-holder "Wesnoth Development Team"

default: translations

index.html.en: template.html
	cp template.html index.html.en

translations: index.html.en
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
