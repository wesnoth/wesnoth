#
# This makefile was created with the sole purpose of emulating the 
# makefiles of the rest of domains (and thus make updates easily
# scriptable).
#

DOMAIN= wesnoth-manpages
MSGMERGE = msgmerge
MSGFMT = msgfmt

.SUFFIXES: .gmo .po .po-update

.po.po-update:
	@lang=`echo $@ | sed -e 's/\.po-update$$//'`; \
	tmpdir=`pwd`; \
	echo "$$lang:"; \
	echo "$(MSGMERGE) $$lang.po $(DOMAIN).pot -o $$lang.new.po"; \
	if $(MSGMERGE) $$lang.po $(DOMAIN).pot -o $$tmpdir/$$lang.new.po; then \
	  mv -f $$tmpdir/$$lang.new.po $$lang.po; \
	else \
	  echo "msgmerge for $$lang.po failed!" 1>&2; \
	  rm -f $$tmpdir/$$lang.new.po; \
	fi

# this dummy rule only displays the statistics, there is no need to
# generate .gmo files for po4a manpages
.po.gmo:
	@lang=`echo $* | sed -e 's,.*/,,'`; \
	echo "$(MSGFMT) --check-domain --statistics -o /dev/null $${lang}.po"; \
	$(MSGFMT) --check-domain --statistics -o /dev/null $${lang}.po
