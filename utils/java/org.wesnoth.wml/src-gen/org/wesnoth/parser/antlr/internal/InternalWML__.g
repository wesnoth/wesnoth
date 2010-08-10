lexer grammar InternalWML;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T13 : '[' ;
T14 : '+' ;
T15 : ']' ;
T16 : '[/' ;
T17 : '{' ;
T18 : '}' ;
T19 : '=' ;
T20 : '(' ;
T21 : ')' ;
T22 : '~' ;
T23 : '/' ;
T24 : ',' ;
T25 : '_' ;
T26 : '.' ;
T27 : '-' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1440
RULE_ID : ('a'..'z'|'A'..'Z'|'_'|'0'..'9')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1442
RULE_INT : ('0'..'9')* ('.' ('0'..'9')+)?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1444
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1446
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1448
RULE_DEFINE : '#define' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1450
RULE_ENDDEFINE : '#enddef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1452
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1454
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1456
RULE_ANY_OTHER : .;


