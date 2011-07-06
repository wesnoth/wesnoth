lexer grammar InternalWML;
@header {
package org.wesnoth.ui.contentassist.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.ui.editor.contentassist.antlr.internal.Lexer;
}

T20 : '_' ;
T21 : '~' ;
T22 : '.' ;
T23 : './' ;
T24 : '$' ;
T25 : '/' ;
T26 : '(' ;
T27 : ')' ;
T28 : '=' ;
T29 : '[' ;
T30 : ']' ;
T31 : '+' ;
T32 : '[/' ;
T33 : '{' ;
T34 : '}' ;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2434
RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2436
RULE_IFHAVE : '#ifhave' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2438
RULE_IFNHAVE : '#ifnhave' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2440
RULE_IFDEF : '#ifdef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2442
RULE_IFNDEF : '#ifndef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2444
RULE_ELSE : '#else' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2446
RULE_ENDIF : '#endif' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2448
RULE_DEFINE : '#define' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2450
RULE_ENDDEF : '#enddef' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2452
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2454
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2456
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|'-'|',')+;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2458
RULE_EOL : ('\r'|'\n');

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2460
RULE_WS : (' '|'\t')+;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2462
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 2464
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


