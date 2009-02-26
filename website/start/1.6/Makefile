DOMAIN = wesnoth-start-1.6

PODIR = po
STAMPDIR = .stamp

LANGUAGES = $(shell cd $(PODIR) && ls *.po | sed 's/\.po//')

POT = $(PODIR)/$(DOMAIN).pot
PO = $(addprefix $(PODIR)/, $(LANGUAGES:=.po))

TEMPLATE = template.html
PAGE = index.html
PAGE_STAMP = $(STAMPDIR)/$(PAGE)
HTML_STAMP = $(addprefix $(PAGE_STAMP)., $(LANGUAGES))

LIST = languages.conf
LIST_STAMP = $(STAMPDIR)/$(LIST)

PO4A_OPTIONS = -f xhtml -M utf-8
PO4A_THRESHOLD = 85

# ----------------------------------------------------------------------

all: update-pot update-po update-html update-list

update-pot: $(POT)

update-po: $(PO)

update-html: $(HTML_STAMP) $(PAGE).en

update-list: $(LIST_STAMP)

clean:
	rm -rf $(PODIR)/*~ $(PAGE).* $(STAMPDIR)

# ----------------------------------------------------------------------

$(POT): $(TEMPLATE)
	po4a-gettextize $(PO4A_OPTIONS) -L utf-8 -m $< -p $@

$(PO): $(TEMPLATE)
	po4a-updatepo $(PO4A_OPTIONS) -m $< -p $@
	@touch $@

$(PAGE).en:
	cp $(TEMPLATE) $@

$(PAGE_STAMP).%: $(TEMPLATE) $(PODIR)/%.po
	@html=`basename $@` && \
	lang=`echo $$html | sed 's/$(PAGE).//'` && \
	echo $$html && \
	po4a-translate $(PO4A_OPTIONS) -L utf-8 -m $(TEMPLATE) \
	               -k $(PO4A_THRESHOLD) -p $(PODIR)/$$lang.po -l $$html
	@mkdir -p $(STAMPDIR) && touch $@

$(LIST_STAMP): $(LIST) $(HTML_STAMP)
	@list=$$(for i in $(PAGE).*; do \
		code=`echo $$i | cut -d. -f 3`; \
		name=`grep "^$$code:" $(LIST) | cut -d: -f 2`; \
		echo -n "<li><a href='$$i'>$$name</a></li>"; \
	done) && \
	for i in $(PAGE).*; do \
		sed -i "/<!--LANGS-->./d; \
		        s,<!--LANGS-->,<!--LANGS-->$$list\n<!--LANGS-->," $$i; \
	done
	@mkdir -p $(STAMPDIR) && touch $@

# ----------------------------------------------------------------------

stats:
	@cd $(PODIR) && \
	for i in *.po; do \
		echo -n "$$i: "; \
		msgfmt --statistics -o /dev/null $$i; \
	done

.PHONY: all update-pot update-po update-html update-list clean stats
