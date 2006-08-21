#
# This makefile was created with the sole purpose of emulating the 
# makefiles of the rest of domains (and thus make updates easily
# scriptable).
#

DOMAIN= wesnoth-man
MSGMERGE = msgmerge

.SUFFIXES: .gmo .po .po-update

.po.po-update:
	@lang=`echo $@ | sed -e 's/\.po-update$$//'`; \
	tmpdir=`pwd`; \
	echo "$$lang:"; \
	echo "$${cdcmd}$(MSGMERGE) $$lang.po $(DOMAIN).pot -o $$lang.new.po"; \
	if $(MSGMERGE) $$lang.po $(DOMAIN).pot -o $$tmpdir/$$lang.new.po; then \
	  mv -f $$tmpdir/$$lang.new.po $$lang.po; \
	else \
	  echo "msgmerge for $$lang.po failed!" 1>&2; \
	  rm -f $$tmpdir/$$lang.new.po; \
	fi

.po.gmo:
	# dummy rule
