JAVADOC_BINARY=javadoc
JAVADOC_DIR=javadoc
JAVADOC_FLAGS=-d $(JAVADOC_DIR) -sourcepath org.wesnoth/src -public -source 1.6 -use -version -author -splitindex -link http://java.sun.com/javase/6/docs/api/ -link http://help.eclipse.org/helios/topic/org.eclipse.platform.doc.isv/reference/api/ -subpackages org.wesnoth

.PHONY: readme, javadoc, build
readme:
	cd org.wesnoth/readme; pdflatex README.tex
	# cleanup non-needed files
	cd org.wesnoth/readme; rm README.log; rm README.aux;
	cd org.wesnoth/readme; cp README.pdf ..;

javadoc:
	$(JAVADOC_BINARY) $(JAVADOC_FLAGS)

build:
	echo 'not implemented'