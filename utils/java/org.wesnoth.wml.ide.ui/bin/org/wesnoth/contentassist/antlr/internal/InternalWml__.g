lexer grammar InternalWml;
@header {
package org.wesnoth.contentassist.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.ui.common.editor.contentassist.antlr.internal.Lexer;
}

T11 : 'import' ;
T12 : 'type' ;
T13 : '[campaign]' ;
T14 : '[/campaign]' ;
T15 : 'entity' ;
T16 : '{' ;
T17 : '}' ;
T18 : 'extends' ;
T19 : 'property' ;
T20 : ':' ;
T21 : '[]' ;

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 914
RULE_ID : '^'? ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')*;

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 916
RULE_INT : ('0'..'9')+;

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 918
RULE_STRING : ('"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"'|'\'' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'\'')))* '\'');

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 920
RULE_ML_COMMENT : '/*' ( options {greedy=false;} : . )*'*/';

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 922
RULE_SL_COMMENT : '//' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 924
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g" 926
RULE_ANY_OTHER : .;


