lexer grammar InternalWML;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T9 : '{' ;
T10 : '_' ;
T11 : ':' ;
T12 : '-' ;
T13 : '.' ;
T14 : '(' ;
T15 : ')' ;
T16 : '=' ;
T17 : '/' ;
T18 : '}' ;
T19 : '[' ;
T20 : ']' ;
T21 : '[/' ;
T22 : ' ' ;
T23 : 'n' ;
T24 : 's' ;
T25 : 'w' ;
T26 : 'e' ;
T27 : 'sw' ;
T28 : 'se' ;
T29 : 'ne' ;
T30 : 'nw' ;
T31 : ',' ;
T32 : '~' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1355
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1357
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1359
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9') ('a'..'z'|'A'..'Z'|'_'|' '|'0'..'9')*;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1361
RULE_IINT : ('-'|'+')? ('0'..'9')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1363
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';


