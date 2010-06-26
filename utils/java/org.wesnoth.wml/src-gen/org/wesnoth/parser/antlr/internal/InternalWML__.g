lexer grammar InternalWML;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T11 : '#textdomain ' ;
T12 : '{' ;
T13 : '}' ;
T14 : '[' ;
T15 : ']' ;
T16 : '/' ;
T17 : '+' ;
T18 : '=' ;
T19 : '_' ;
T20 : '~' ;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1091
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1093
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1095
RULE_ID : ('a'..'z'|'A'..'Z'|'_'|'-'|'0'..'9')+;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1097
RULE_IDLIST : (RULE_ID ',')* RULE_ID;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1099
RULE_STRING : ('"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"'|'\'' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'\'')))* '\'');

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1101
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g" 1103
RULE_PATH : (('a'..'z'|'A'..'Z'|'_'|'.'|'-'|'0'..'9')+ '/')* ('a'..'z'|'A'..'Z'|'_'|'.'|'-'|'0'..'9')+ '/'?;


