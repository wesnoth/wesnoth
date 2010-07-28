lexer grammar InternalWML;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T10 : '[' ;
T11 : ']' ;
T12 : '[/' ;
T13 : '=' ;
T14 : '.' ;
T15 : ' ' ;
T16 : '_' ;
T17 : '-' ;
T18 : '/' ;
T19 : 'n' ;
T20 : 's' ;
T21 : 'w' ;
T22 : 'e' ;
T23 : 'sw' ;
T24 : 'se' ;
T25 : 'ne' ;
T26 : 'nw' ;
T27 : ',' ;
T28 : '~' ;
T29 : ':' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1108
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1110
RULE_MACRO : '{' ( options {greedy=false;} : . )*'}';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1112
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1114
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9') ('a'..'z'|'A'..'Z'|'_'|' '|'0'..'9')*;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1116
RULE_IINT : ('-'|'+')? ('0'..'9')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1118
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';


