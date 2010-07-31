package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;


import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

@SuppressWarnings("all")
public class InternalWMLLexer extends Lexer {
    public static final int RULE_LUA_CODE=8;
    public static final int RULE_ID=4;
    public static final int RULE_ANY_OTHER=12;
    public static final int RULE_EOL=5;
    public static final int RULE_TEXTDOMAIN=10;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=6;
    public static final int Tokens=19;
    public static final int RULE_MACRO=7;
    public static final int RULE_STRING=11;
    public static final int RULE_DEFINE=9;
    public static final int T14=14;
    public static final int RULE_WS=13;
    public static final int T15=15;
    public static final int T16=16;
    public static final int T17=17;
    public static final int T18=18;
    public InternalWMLLexer() {;} 
    public InternalWMLLexer(CharStream input) {
        super(input);
    }
    public String getGrammarFileName() { return "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g"; }

    // $ANTLR start T14
    public final void mT14() throws RecognitionException {
        try {
            int _type = T14;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:10:5: ( '[' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:10:7: '['
            {
            match('['); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T14

    // $ANTLR start T15
    public final void mT15() throws RecognitionException {
        try {
            int _type = T15;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:11:5: ( '+' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:11:7: '+'
            {
            match('+'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T15

    // $ANTLR start T16
    public final void mT16() throws RecognitionException {
        try {
            int _type = T16;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:12:5: ( ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:12:7: ']'
            {
            match(']'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T16

    // $ANTLR start T17
    public final void mT17() throws RecognitionException {
        try {
            int _type = T17;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:13:5: ( '[/' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:13:7: '[/'
            {
            match("[/"); 


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T17

    // $ANTLR start T18
    public final void mT18() throws RecognitionException {
        try {
            int _type = T18;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:14:5: ( '=' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:14:7: '='
            {
            match('='); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T18

    // $ANTLR start RULE_LUA_CODE
    public final void mRULE_LUA_CODE() throws RecognitionException {
        try {
            int _type = RULE_LUA_CODE;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:939:15: ( '<<' ( options {greedy=false; } : . )* '>>' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:939:17: '<<' ( options {greedy=false; } : . )* '>>'
            {
            match("<<"); 

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:939:22: ( options {greedy=false; } : . )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0=='>') ) {
                    int LA1_1 = input.LA(2);

                    if ( (LA1_1=='>') ) {
                        alt1=2;
                    }
                    else if ( ((LA1_1>='\u0000' && LA1_1<='=')||(LA1_1>='?' && LA1_1<='\uFFFE')) ) {
                        alt1=1;
                    }


                }
                else if ( ((LA1_0>='\u0000' && LA1_0<='=')||(LA1_0>='?' && LA1_0<='\uFFFE')) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:939:50: .
            	    {
            	    matchAny(); 

            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);

            match(">>"); 


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_LUA_CODE

    // $ANTLR start RULE_MACRO
    public final void mRULE_MACRO() throws RecognitionException {
        try {
            int _type = RULE_MACRO;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:941:12: ( '{' ( options {greedy=false; } : . )* '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:941:14: '{' ( options {greedy=false; } : . )* '}'
            {
            match('{'); 
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:941:18: ( options {greedy=false; } : . )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( (LA2_0=='}') ) {
                    alt2=2;
                }
                else if ( ((LA2_0>='\u0000' && LA2_0<='|')||(LA2_0>='~' && LA2_0<='\uFFFE')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:941:46: .
            	    {
            	    matchAny(); 

            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            match('}'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_MACRO

    // $ANTLR start RULE_DEFINE
    public final void mRULE_DEFINE() throws RecognitionException {
        try {
            int _type = RULE_DEFINE;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:13: ( '#define' ( options {greedy=false; } : . )* '#enddef' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:15: '#define' ( options {greedy=false; } : . )* '#enddef'
            {
            match("#define"); 

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:25: ( options {greedy=false; } : . )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0=='#') ) {
                    int LA3_1 = input.LA(2);

                    if ( (LA3_1=='e') ) {
                        int LA3_3 = input.LA(3);

                        if ( (LA3_3=='n') ) {
                            int LA3_4 = input.LA(4);

                            if ( (LA3_4=='d') ) {
                                int LA3_5 = input.LA(5);

                                if ( (LA3_5=='d') ) {
                                    int LA3_6 = input.LA(6);

                                    if ( (LA3_6=='e') ) {
                                        int LA3_7 = input.LA(7);

                                        if ( (LA3_7=='f') ) {
                                            alt3=2;
                                        }
                                        else if ( ((LA3_7>='\u0000' && LA3_7<='e')||(LA3_7>='g' && LA3_7<='\uFFFE')) ) {
                                            alt3=1;
                                        }


                                    }
                                    else if ( ((LA3_6>='\u0000' && LA3_6<='d')||(LA3_6>='f' && LA3_6<='\uFFFE')) ) {
                                        alt3=1;
                                    }


                                }
                                else if ( ((LA3_5>='\u0000' && LA3_5<='c')||(LA3_5>='e' && LA3_5<='\uFFFE')) ) {
                                    alt3=1;
                                }


                            }
                            else if ( ((LA3_4>='\u0000' && LA3_4<='c')||(LA3_4>='e' && LA3_4<='\uFFFE')) ) {
                                alt3=1;
                            }


                        }
                        else if ( ((LA3_3>='\u0000' && LA3_3<='m')||(LA3_3>='o' && LA3_3<='\uFFFE')) ) {
                            alt3=1;
                        }


                    }
                    else if ( ((LA3_1>='\u0000' && LA3_1<='d')||(LA3_1>='f' && LA3_1<='\uFFFE')) ) {
                        alt3=1;
                    }


                }
                else if ( ((LA3_0>='\u0000' && LA3_0<='\"')||(LA3_0>='$' && LA3_0<='\uFFFE')) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:53: .
            	    {
            	    matchAny(); 

            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);

            match("#enddef"); 


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_DEFINE

    // $ANTLR start RULE_TEXTDOMAIN
    public final void mRULE_TEXTDOMAIN() throws RecognitionException {
        try {
            int _type = RULE_TEXTDOMAIN;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:17: ( '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:19: '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#textdomain"); 

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:33: (~ ( ( '\\n' | '\\r' ) ) )*
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( ((LA4_0>='\u0000' && LA4_0<='\t')||(LA4_0>='\u000B' && LA4_0<='\f')||(LA4_0>='\u000E' && LA4_0<='\uFFFE')) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:33: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFE') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop4;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:49: ( ( '\\r' )? '\\n' )?
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( (LA6_0=='\n'||LA6_0=='\r') ) {
                alt6=1;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:50: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:50: ( '\\r' )?
                    int alt5=2;
                    int LA5_0 = input.LA(1);

                    if ( (LA5_0=='\r') ) {
                        alt5=1;
                    }
                    switch (alt5) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:50: '\\r'
                            {
                            match('\r'); 

                            }
                            break;

                    }

                    match('\n'); 

                    }
                    break;

            }


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_TEXTDOMAIN

    // $ANTLR start RULE_STRING
    public final void mRULE_STRING() throws RecognitionException {
        try {
            int _type = RULE_STRING;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:13: ( '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:15: '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"'
            {
            match('\"'); 
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:19: ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )*
            loop7:
            do {
                int alt7=3;
                int LA7_0 = input.LA(1);

                if ( (LA7_0=='\\') ) {
                    alt7=1;
                }
                else if ( ((LA7_0>='\u0000' && LA7_0<='!')||(LA7_0>='#' && LA7_0<='[')||(LA7_0>=']' && LA7_0<='\uFFFE')) ) {
                    alt7=2;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:20: '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' )
            	    {
            	    match('\\'); 
            	    if ( input.LA(1)=='\"'||input.LA(1)=='\''||input.LA(1)=='\\'||input.LA(1)=='b'||input.LA(1)=='f'||input.LA(1)=='n'||input.LA(1)=='r'||input.LA(1)=='t' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:61: ~ ( ( '\\\\' | '\"' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='!')||(input.LA(1)>='#' && input.LA(1)<='[')||(input.LA(1)>=']' && input.LA(1)<='\uFFFE') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop7;
                }
            } while (true);

            match('\"'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_STRING

    // $ANTLR start RULE_ID
    public final void mRULE_ID() throws RecognitionException {
        try {
            int _type = RULE_ID;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:9: ( ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | ',' )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | ',' )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | ',' )+
            int cnt8=0;
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0==','||(LA8_0>='0' && LA8_0<='9')||(LA8_0>='A' && LA8_0<='Z')||LA8_0=='_'||(LA8_0>='a' && LA8_0<='z')) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)==','||(input.LA(1)>='0' && input.LA(1)<='9')||(input.LA(1)>='A' && input.LA(1)<='Z')||input.LA(1)=='_'||(input.LA(1)>='a' && input.LA(1)<='z') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt8 >= 1 ) break loop8;
                        EarlyExitException eee =
                            new EarlyExitException(8, input);
                        throw eee;
                }
                cnt8++;
            } while (true);


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_ID

    // $ANTLR start RULE_EOL
    public final void mRULE_EOL() throws RecognitionException {
        try {
            int _type = RULE_EOL;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:951:10: ( ( '\\r' | '\\n' )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:951:12: ( '\\r' | '\\n' )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:951:12: ( '\\r' | '\\n' )+
            int cnt9=0;
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0=='\n'||LA9_0=='\r') ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)=='\n'||input.LA(1)=='\r' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt9 >= 1 ) break loop9;
                        EarlyExitException eee =
                            new EarlyExitException(9, input);
                        throw eee;
                }
                cnt9++;
            } while (true);


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_EOL

    // $ANTLR start RULE_WS
    public final void mRULE_WS() throws RecognitionException {
        try {
            int _type = RULE_WS;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:953:9: ( ( ' ' | '\\t' )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:953:11: ( ' ' | '\\t' )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:953:11: ( ' ' | '\\t' )+
            int cnt10=0;
            loop10:
            do {
                int alt10=2;
                int LA10_0 = input.LA(1);

                if ( (LA10_0=='\t'||LA10_0==' ') ) {
                    alt10=1;
                }


                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)=='\t'||input.LA(1)==' ' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt10 >= 1 ) break loop10;
                        EarlyExitException eee =
                            new EarlyExitException(10, input);
                        throw eee;
                }
                cnt10++;
            } while (true);


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_WS

    // $ANTLR start RULE_ANY_OTHER
    public final void mRULE_ANY_OTHER() throws RecognitionException {
        try {
            int _type = RULE_ANY_OTHER;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:955:16: ( . )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:955:18: .
            {
            matchAny(); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_ANY_OTHER

    // $ANTLR start RULE_SL_COMMENT
    public final void mRULE_SL_COMMENT() throws RecognitionException {
        try {
            int _type = RULE_SL_COMMENT;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:17: ( '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:19: '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match('#'); 
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( ((LA11_0>='\u0000' && LA11_0<='\t')||(LA11_0>='\u000B' && LA11_0<='\f')||(LA11_0>='\u000E' && LA11_0<='\uFFFE')) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:23: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFE') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop11;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:39: ( ( '\\r' )? '\\n' )?
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0=='\n'||LA13_0=='\r') ) {
                alt13=1;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:40: ( '\\r' )?
                    int alt12=2;
                    int LA12_0 = input.LA(1);

                    if ( (LA12_0=='\r') ) {
                        alt12=1;
                    }
                    switch (alt12) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:40: '\\r'
                            {
                            match('\r'); 

                            }
                            break;

                    }

                    match('\n'); 

                    }
                    break;

            }


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_SL_COMMENT

    public void mTokens() throws RecognitionException {
        // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:8: ( T14 | T15 | T16 | T17 | T18 | RULE_LUA_CODE | RULE_MACRO | RULE_DEFINE | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT )
        int alt14=15;
        alt14 = dfa14.predict(input);
        switch (alt14) {
            case 1 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:10: T14
                {
                mT14(); 

                }
                break;
            case 2 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:14: T15
                {
                mT15(); 

                }
                break;
            case 3 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:18: T16
                {
                mT16(); 

                }
                break;
            case 4 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:22: T17
                {
                mT17(); 

                }
                break;
            case 5 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:26: T18
                {
                mT18(); 

                }
                break;
            case 6 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:30: RULE_LUA_CODE
                {
                mRULE_LUA_CODE(); 

                }
                break;
            case 7 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:44: RULE_MACRO
                {
                mRULE_MACRO(); 

                }
                break;
            case 8 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:55: RULE_DEFINE
                {
                mRULE_DEFINE(); 

                }
                break;
            case 9 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:67: RULE_TEXTDOMAIN
                {
                mRULE_TEXTDOMAIN(); 

                }
                break;
            case 10 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:83: RULE_STRING
                {
                mRULE_STRING(); 

                }
                break;
            case 11 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:95: RULE_ID
                {
                mRULE_ID(); 

                }
                break;
            case 12 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:103: RULE_EOL
                {
                mRULE_EOL(); 

                }
                break;
            case 13 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:112: RULE_WS
                {
                mRULE_WS(); 

                }
                break;
            case 14 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:120: RULE_ANY_OTHER
                {
                mRULE_ANY_OTHER(); 

                }
                break;
            case 15 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:135: RULE_SL_COMMENT
                {
                mRULE_SL_COMMENT(); 

                }
                break;

        }

    }


    protected DFA14 dfa14 = new DFA14(this);
    static final String DFA14_eotS =
        "\1\uffff\1\16\3\uffff\4\14\13\uffff\2\26\5\uffff\13\26\1\uffff\4"+
        "\26\1\uffff\4\26\1\65\1\26\1\65\3\uffff\1\26\1\53";
    static final String DFA14_eofS =
        "\70\uffff";
    static final String DFA14_minS =
        "\1\0\1\57\3\uffff\1\74\3\0\13\uffff\2\145\5\uffff\1\146\1\170\1"+
        "\151\1\164\1\156\1\144\1\145\1\157\1\0\1\155\4\0\1\141\1\0\1\uffff"+
        "\1\151\1\0\1\156\4\0\1\12\2\uffff\2\0";
    static final String DFA14_maxS =
        "\1\ufffe\1\57\3\uffff\1\74\3\ufffe\13\uffff\2\145\5\uffff\1\146"+
        "\1\170\1\151\1\164\1\156\1\144\1\145\1\157\1\ufffe\1\155\4\ufffe"+
        "\1\141\1\ufffe\1\uffff\1\151\1\ufffe\1\156\4\ufffe\1\12\2\uffff"+
        "\2\ufffe";
    static final String DFA14_acceptS =
        "\2\uffff\1\2\1\3\1\5\4\uffff\1\13\1\14\1\15\1\16\1\4\1\1\1\2\1\3"+
        "\1\5\1\6\1\7\2\uffff\1\17\1\12\1\13\1\14\1\15\20\uffff\1\10\10\uffff"+
        "\2\11\2\uffff";
    static final String DFA14_specialS =
        "\70\uffff}>";
    static final String[] DFA14_transitionS = {
            "\11\14\1\13\1\12\2\14\1\12\22\14\1\13\1\14\1\10\1\7\7\14\1\2"+
            "\1\11\3\14\12\11\2\14\1\5\1\4\3\14\32\11\1\1\1\14\1\3\1\14\1"+
            "\11\1\14\32\11\1\6\uff83\14",
            "\1\15",
            "",
            "",
            "",
            "\1\22",
            "\uffff\23",
            "\144\26\1\24\17\26\1\25\uff8a\26",
            "\uffff\27",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "\1\33",
            "\1\34",
            "",
            "",
            "",
            "",
            "",
            "\1\35",
            "\1\36",
            "\1\37",
            "\1\40",
            "\1\41",
            "\1\42",
            "\1\43",
            "\1\44",
            "\12\50\1\47\2\50\1\46\25\50\1\45\uffdb\50",
            "\1\51",
            "\12\50\1\47\2\50\1\46\25\50\1\45\101\50\1\52\uff99\50",
            "\12\53\1\47\ufff4\53",
            "\uffff\53",
            "\12\50\1\47\2\50\1\46\25\50\1\45\uffdb\50",
            "\1\54",
            "\12\50\1\47\2\50\1\46\25\50\1\45\112\50\1\55\uff90\50",
            "",
            "\1\56",
            "\12\50\1\47\2\50\1\46\25\50\1\45\100\50\1\57\uff9a\50",
            "\1\60",
            "\12\50\1\47\2\50\1\46\25\50\1\45\100\50\1\61\uff9a\50",
            "\12\62\1\64\2\62\1\63\ufff1\62",
            "\12\50\1\47\2\50\1\46\25\50\1\45\101\50\1\66\uff99\50",
            "\12\62\1\64\2\62\1\63\ufff1\62",
            "\1\64",
            "",
            "",
            "\12\50\1\47\2\50\1\46\25\50\1\45\102\50\1\67\uff98\50",
            "\12\50\1\47\2\50\1\46\25\50\1\45\uffdb\50"
    };

    static final short[] DFA14_eot = DFA.unpackEncodedString(DFA14_eotS);
    static final short[] DFA14_eof = DFA.unpackEncodedString(DFA14_eofS);
    static final char[] DFA14_min = DFA.unpackEncodedStringToUnsignedChars(DFA14_minS);
    static final char[] DFA14_max = DFA.unpackEncodedStringToUnsignedChars(DFA14_maxS);
    static final short[] DFA14_accept = DFA.unpackEncodedString(DFA14_acceptS);
    static final short[] DFA14_special = DFA.unpackEncodedString(DFA14_specialS);
    static final short[][] DFA14_transition;

    static {
        int numStates = DFA14_transitionS.length;
        DFA14_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA14_transition[i] = DFA.unpackEncodedString(DFA14_transitionS[i]);
        }
    }

    class DFA14 extends DFA {

        public DFA14(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 14;
            this.eot = DFA14_eot;
            this.eof = DFA14_eof;
            this.min = DFA14_min;
            this.max = DFA14_max;
            this.accept = DFA14_accept;
            this.special = DFA14_special;
            this.transition = DFA14_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T14 | T15 | T16 | T17 | T18 | RULE_LUA_CODE | RULE_MACRO | RULE_DEFINE | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT );";
        }
    }
 

}