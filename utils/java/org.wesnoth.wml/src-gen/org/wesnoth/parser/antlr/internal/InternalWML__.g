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
T17 : '=' ;
T18 : '{' ;
T19 : '~' ;
T20 : '}' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1019
RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1021
RULE_DEFINE : '#define' ( options {greedy=false;} : . )*'#enddef';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1023
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1025
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1027
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|',')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1029
RULE_EOL : ('\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1031
RULE_WS : (' '|'\t')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1033
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1035
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


