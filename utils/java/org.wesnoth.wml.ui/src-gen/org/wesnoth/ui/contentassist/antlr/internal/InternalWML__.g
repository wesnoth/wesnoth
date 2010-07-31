lexer grammar InternalWML;
@header {
package org.wesnoth.ui.contentassist.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.ui.editor.contentassist.antlr.internal.Lexer;
}

T13 : '+' ;
T14 : '[' ;
T15 : ']' ;
T16 : '[/' ;
T17 : '=' ;
T18 : '{' ;
T19 : '}' ;
T20 : '~' ;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1556
RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1558
RULE_DEFINE : '#define' ( options {greedy=false;} : . )*'#enddef';

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1560
RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1562
RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1564
RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|',')+;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1566
RULE_EOL : ('\r'|'\n')+;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1568
RULE_WS : (' '|'\t')+;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1570
RULE_ANY_OTHER : .;

// $ANTLR src "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g" 1572
RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


