lexer grammar InternalWML;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T14 : '[' ;
T15 : '+' ;
T16 : ']' ;
T17 : '[/' ;
T18 : '=' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 939
RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 941
RULE_MACRO : '{' ( options {greedy=false;} : . )*'}';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 943
RULE_DEFINE : '#define' ( options {greedy=false;} : . )*'#enddef';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 945
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 947
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 949
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|',')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 951
RULE_EOL : ('\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 953
RULE_WS : (' '|'\t')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 955
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 957
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


