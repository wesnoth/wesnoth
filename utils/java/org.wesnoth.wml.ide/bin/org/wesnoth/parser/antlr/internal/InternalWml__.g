lexer grammar InternalWml;
@header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

T11 : 'import' ;
T12 : 'type' ;
T13 : '[campaign]' ;
T14 : '[/campaign]' ;
T15 : 'entity' ;
T16 : 'extends' ;
T17 : '{' ;
T18 : '}' ;
T19 : 'property' ;
T20 : ':' ;
T21 : '[]' ;

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 503
RULE_ID : '^'? ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')*;

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 505
RULE_INT : ('0'..'9')+;

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 507
RULE_STRING : ('"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"'|'\'' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'\'')))* '\'');

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 509
RULE_ML_COMMENT : '/*' ( options {greedy=false;} : . )*'*/';

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 511
RULE_SL_COMMENT : '//' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 513
RULE_WS : (' '|'\t'|'\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g" 515
RULE_ANY_OTHER : .;


