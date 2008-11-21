ELISP = wesnoth-mode.el wesnoth-update.el
DOC = wesnoth-mode.texi
BYTECOMPILED = wesnoth-mode.elc wesnoth-update.elc
EMACS = emacs
FLAGS = -q --eval "(add-to-list 'load-path default-directory)" \
	--batch -f batch-byte-compile
all: wesnoth-mode

wesnoth-mode :
	$(EMACS) $(FLAGS) $(ELISP)

info :
	makeinfo $(DOC)

pdf :
	texi2dvi -q --pdf -c $(DOC) 

doc : info pdf

.PHONY: clean

clean:
	rm -f $(BYTECOMPILED) wesnoth-mode.pdf wesnoth-mode.info
