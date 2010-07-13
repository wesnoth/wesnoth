lexer grammar InternalWML;
@header {
package org.wesnoth.ui.contentassist.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.ui.editor.contentassist.antlr.internal.Lexer;
}

T9 : '_' ;
T10 : ':' ;
T11 : '-' ;
T12 : '.' ;
T13 : '(' ;
T14 : ')' ;
T15 : '=' ;
T16 : '/' ;
T17 : 'n' ;
T18 : 's' ;
T19 : 'w' ;
T20 : 'e' ;
T21 : 'sw' ;
T22 : 'se' ;
T23 : 'ne' ;
T24 : 'nw' ;
T25 : '{' ;
T26 : '}' ;
T27 : '[' ;
T28 : ']' ;
T29 : '[/' ;
T30 : ' ' ;
T31 : ',' ;
T32 : '~' ;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2608
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2610
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2612
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9') ('a'..'z'|'A'..'Z'|'_'|' '|'0'..'9')*;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2614
RULE_IINT : ('-'|'+')? ('0'..'9')+;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2616
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';


