JAVADOC_BINARY=javadoc
JAVADOC_DIR=javadoc
JAVADOC_FLAGS=-d $(JAVADOC_DIR) -sourcepath src -public -source 1.6 -use -version -author -splitindex -link http://java.sun.com/javase/6/docs/api/ -link http://help.eclipse.org/helios/topic/org.eclipse.platform.doc.isv/reference/api/ -subpackages org.wesnoth

.PHONY: readme, javadoc, build
readme:
	cd readme; pdflatex README.tex
	# cleanup non-needed files
	cd readme; rm README.log; rm README.aux;
	cd readme; cp README.pdf ..;

javadoc:
	$(JAVADOC_BINARY) $(JAVADOC_FLAGS)

build:
	echo 'not implemented'