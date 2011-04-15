JAVADOC_BINARY=javadoc
JAVADOC_DIR=javadoc
JAVADOC_FLAGS=-d $(JAVADOC_DIR) -sourcepath src -public -source 1.6 -use -version -author -splitindex -link http://java.sun.com/javase/6/docs/api/ -subpackages org.wesnoth

.PHONY: readme, javadoc
readme:
	cd readme; pdflatex README.tex
	# cleanup non-needed files
	cd readme; rm README.log; rm README.aux;
	cd readme; cp README.pdf ..;

javadoc:
	$(JAVADOC_BINARY) $(JAVADOC_FLAGS)