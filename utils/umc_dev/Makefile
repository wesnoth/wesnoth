ECLIPSE_LOCATION=
JAVADOC_BINARY=javadoc
JAVADOC_DIR=javadoc
JAVADOC_FLAGS=-d $(JAVADOC_DIR) -sourcepath 'org.wesnoth/src;org.wesnoth/src-gen;org.wesnoth.ui/src;org.wesnoth.ui/src-gen' -classpath '.;$(ECLIPSE_LOCATION)/plugins/*' -public -source 1.6 -use -version -author -splitindex -link http://java.sun.com/javase/6/docs/api/ -link http://help.eclipse.org/helios/topic/org.eclipse.platform.doc.isv/reference/api/ -link ftp://ftp.ulakbim.gov.tr/eclipse/eclipseMirror/modeling/tmf/xtext/javadoc/2.0.0/ -subpackages org.wesnoth

.PHONY: manual javadoc build
manual:
	# developer manual
	cd manual; pdflatex dev_manual.tex; pdflatex dev_manual.tex cp dev_manual.pdf ..;
	# user manual
	cd manual; pdflatex user_manual.tex; pdflatex user_manual.tex; cp user_manual.pdf ..;
	# cleanup non-needed files
	cd manual; rm dev_manual.log; rm dev_manual.aux; rm user_manual.log; rm user_manual.aux; rm dev_manual.out; rm dev_manual.toc; rm user_manual.out; rm user_manual.toc;

javadoc:
	$(JAVADOC_BINARY) $(JAVADOC_FLAGS)

build:
	echo 'not implemented'