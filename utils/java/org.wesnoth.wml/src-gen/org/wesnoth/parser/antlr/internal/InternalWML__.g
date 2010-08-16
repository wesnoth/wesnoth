lexer grammar InternalWML;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T20 : '[' ;
T21 : '+' ;
T22 : ']' ;
T23 : '[/' ;
T24 : '=' ;
T25 : '{' ;
T26 : './' ;
T27 : '~' ;
T28 : '}' ;
T29 : '.' ;
T30 : '$' ;
T31 : '/' ;
T32 : '(' ;
T33 : ')' ;
T34 : '_' ;
T35 : '_$' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2015
RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2017
RULE_IFHAVE : '#ifhave' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2019
RULE_IFNHAVE : '#ifnhave' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2021
RULE_IFDEF : '#ifdef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2023
RULE_IFNDEF : '#ifndef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2025
RULE_ELSE : '#else' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2027
RULE_ENDIF : '#endif' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2029
RULE_DEFINE : '#define' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2031
RULE_ENDDEF : '#enddef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2033
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2035
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2037
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|'-'|',')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2039
RULE_EOL : ('\r'|'\n');

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2041
RULE_WS : (' '|'\t')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2043
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 2045
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


