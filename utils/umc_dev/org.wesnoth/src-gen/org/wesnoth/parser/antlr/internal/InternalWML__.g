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
T29 : '_' ;
T30 : '.' ;
T31 : '$' ;
T32 : '/' ;
T33 : '(' ;
T34 : ')' ;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1443
RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1445
RULE_IFHAVE : '#ifhave' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1447
RULE_IFNHAVE : '#ifnhave' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1449
RULE_IFDEF : '#ifdef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1451
RULE_IFNDEF : '#ifndef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1453
RULE_ELSE : '#else' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1455
RULE_ENDIF : '#endif' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1457
RULE_DEFINE : '#define' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1459
RULE_ENDDEF : '#enddef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1461
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1463
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1465
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|'-'|',')+;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1467
RULE_EOL : ('\r'|'\n');

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1469
RULE_WS : (' '|'\t')+;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1471
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1473
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


